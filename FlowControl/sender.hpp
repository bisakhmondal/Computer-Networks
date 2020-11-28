#ifndef FLOW_CONTROL_SENDER
#define FLOW_CONTROL_SENDER

#include "typedefs.hpp"
#include "frameutils.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include<thread>
#include<unistd.h>
#include<atomic>
#include <string.h>
#include <cmath>

//#include <boos
#define DEBUG(x) std::cout<<"checkpoint-> "<<x<<std::endl;

namespace fc{

    namespace Sender{

        class Base{
            protected:
            
            pid_t destMac, srcMac;
            std::string fifoPipe, fifoPipeW;
//            mutable std::mutex lock; //prevention of readwrite on same fifo on same process.

            
            public:

            Base(pid_t mac, std::string fifo, std::string fifoW): destMac(mac), fifoPipe(fifo), fifoPipeW(fifoW){
                
                srcMac = getpid();
            }
//            virtual void send(int)= 0;
            virtual void Recv_ACK() =0;
        };



        class StopNWait:public Base{
            // float sendingTime, receivingTime;
            // std::atomic<bool> isTimeout;

            int Sn;
            std::atomic<bool> hasReceived;

//            std::queue<bytestream_t> tempBuffer;

            public:

            StopNWait(pid_t mac, std::string fifo, std::string fifoW): Base( mac,fifo, fifoW){

                DEBUG("mac");
                DEBUG(srcMac);
                DEBUG(destMac)
                Sn=0;
                hasReceived = false;
            }


            //Timeout function.
            inline bool Timeout(){
//                std::cout<<"at timeout\n";
//DEBUG("at timeout function");
                    sleep(TIMEOUT);
                return hasReceived;
            }

            //running on a separate thread.
            void Recv_ACK(){
                
                descriptor_t fd = open((char*)fifoPipe.c_str(), O_RDONLY);
//                std::cout<<std::endl<<"fifo open"<<std::endl;

                char  ack_frame[ACKSIZE+1];
                int status;
                {
//                    std::lock_guard<std::mutex> lk(lock);
                    //free the pipe by consuming the whole.
                    while ((status = read(fd, ack_frame, fc::ACKSIZE + 1)) > 0) {
                        //noisy frame dropping it.
                        if (!frame::checkFrameIntegrity(bytestream_t(ack_frame))) {
                            std::cout<<"Noisy acknowledgement received...Discarding..."<<std::endl;
                            continue;
                        }

                        if (frame::getSEQNO(bytestream_t(ack_frame)) == Sn) {
                            hasReceived = true;
                        }
                        std::cout<<"Acknowledge received with seq: "<<frame::getSEQNO(bytestream_t(ack_frame))<<std::endl;


//                    tempBuffer.push(bytestream_t(ack_frame));
                    }
                    if (status == -1)
                        std::cerr << std::this_thread::get_id() << " :fifo read error on -> " << fifoPipe << std::endl;
                }

                close(fd);

            }


            void send(int d){
//                DEBUG(fifoPipe)

//                DEBUG("at send function");
                // std::thread tz([this]{this->check();});
                // tz.detach();
                bytestream_t data =  frame::CreateFrame<int>(srcMac, destMac, d, Sn);
                char frame[fc::FrameSize+1];
                strcpy(frame, (char*)data.c_str());

//                DEBUG(frame)

                int  Sent = 0;
                
                Sn = (Sn+1)%2;

                do{
                    if(Sent==2){
                        DEBUG("Retransmitting old packet")
                    }
                    int fd = open(fifoPipeW.c_str(), O_WRONLY);
                    if(fd==-1){
                        std::cerr<<"pipe open error\n";
                    }
//                    DEBUG("PIPE opened at sender");

                    hasReceived = false;

                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (write(fd, frame, FrameSize + 1) < 0) {
                            std::cerr << "fifo write failed from sender node" << std::endl;
                        }else{
                            DEBUG("Packet sent | data-> " + std::to_string(d));
                        }

                    }

                    close(fd);
                    
                    std::thread tt([this]{this->Recv_ACK();});
                    tt.detach();
                //= new std::thread
                    //timeout using unique lock.
                    if(Timeout()) {
                        DEBUG("Correct Acknowledgement Received")
                        Sent = 1;
                    }else{
                        Sent =2;
                    }

                }while(Sent!=1);

            }

            void run(){
                DEBUG("Transmission start")
                for(auto &i: dataArray){
                    send(i);
                    std::cout<<std::endl;
                    DEBUG("Making next frame")
                    sleep(2);
                }
                DEBUG("data transmission complete")
            }

        };
//----------------------------------GO BACK N----------------------------------------------------------------------------------

        class GOBACKN:public Base{
            // float sendingTime, receivingTime;
             std::atomic<bool> isTimeout, timerStarted;

            std::atomic<int> Sn, Sf, Sw;
            std::unordered_map<int, bytestream_t> cache;

//            std::queue<bytestream_t> tempBuffer;

        public:

            GOBACKN(pid_t mac, std::string fifo, std::string fifoW): Base( mac,fifo, fifoW){

                DEBUG("mac");
                DEBUG(srcMac);
                DEBUG(destMac)
                Sn=0;
                Sf=0;
                Sw= sizeM-1;
                isTimeout = false;
                timerStarted = false;
            }


            //Timeout function.
            inline bool Timeout(){
                timerStarted=true;
                sleep(10);
                isTimeout = true;
                timerStarted=false;
                return 1;
            }

            //running on a separate thread.
            void Recv_ACK() {
                while (Sf<=Sn) {
                    descriptor_t fd = open((char *) fifoPipe.c_str(), O_RDONLY);
//                std::cout<<std::endl<<"fifo open"<<std::endl;

                    char ack_frame[ACKSIZE + 1];
                    int status;
                    {
                        //free the pipe by consuming the whole.
                        while ((status = read(fd, ack_frame, fc::ACKSIZE + 1)) > 0) {
                            //noisy frame dropping it.
                            if (!frame::checkFrameIntegrity(bytestream_t(ack_frame))) {
                                std::cout << "Noisy acknowledgement received...Discarding..." << std::endl;
                                continue;
                            }
                            int ackNo = frame::getSEQNO(bytestream_t(ack_frame));

                            if (ackNo > Sf && ackNo <= Sn) {
                                while (Sf <= ackNo) {
//                                    cache.erase(Sf);
                                    Sf++;
                                }
                            }
                            std::cout << "Acknowledge received with seq: " << frame::getSEQNO(bytestream_t(ack_frame))
                                      << std::endl;


//                    tempBuffer.push(bytestream_t(ack_frame));
                        }
//                        if (status == -1)
//                            std::cerr << std::this_thread::get_id() << " :fifo read error on -> " << fifoPipe
//                                      << std::endl;
                    }

                    close(fd);

                }
            }

//            void GetData(){
//                Sn=sizeM-1;
//            }
            void send(vector<int> &dataArray) {
//                int offset = 0;
                bool retransmit =false;
                std::thread tt([this] { this->Recv_ACK(); });
                tt.detach();

                while (Sf < dataArray.size() ) {
                    //                DEBUG(fifoPipe)
                    while( Sn < dataArray.size()) {

                        //wait for the acknow.
                        while(Sn-Sf>=Sw && !isTimeout){
                            sleep(TIMEOUT);
                        }

                        //need retransmission
                        if(Sn-Sf >=Sw && isTimeout){
                            retransmit = true;
                            break;
                        }


                        int d = dataArray[Sn];

                        bytestream_t data = frame::CreateFrame<int>(srcMac, destMac, d, Sn);
                        cache[Sn] = data; //store frame
                        char frame[fc::FrameSize + 1];
                        strcpy(frame, (char *) data.c_str());


                        int fd = open(fifoPipeW.c_str(), O_WRONLY);
                        if (fd == -1) {
                            std::cerr << "pipe open error\n";
                        }
//writing to fifo
                        {
//                        std::lock_guard<std::mutex> lk(lock);

                            if (write(fd, frame, FrameSize + 1) < 0) {
                                std::cerr << "fifo write failed from sender node" << std::endl;
                            } else {
                                DEBUG("Packet sent | data-> " + std::to_string(d));
                            }

                        }
                        close(fd);

                        Sn++;

                        if (!timerStarted) {
                            isTimeout=false;
                            std::thread tmut([this] { this->Timeout(); });
                            tmut.detach();
                        }
                        //1 sec sleep for better result
                        sleep(1);
                    }
                    if( Sn >= dataArray.size() && isTimeout){
                        retransmit=true;
                    }
                    //retransmission
                    if(retransmit){
                        isTimeout=false;
                        std::thread tmut([this] { this->Timeout(); });
                        tmut.detach();
                        int temp = Sf;
                        DEBUG("Retransmitting Frames")

                        while(temp<Sn){
                            DEBUG(std::to_string(Sf)+ " "+ std::to_string(Sn)+ " "+ std::to_string(temp))
                            bytestream_t data = cache[temp];
                            char frame[fc::FrameSize + 1];
                            strcpy(frame, (char *) data.c_str());

                            int fd = open(fifoPipeW.c_str(), O_WRONLY);
                            if (fd == -1) {
                                std::cerr << "pipe open error\n";
                            }
//writing to fifo
                            {
//                        std::lock_guard<std::mutex> lk(lock);

                                if (write(fd, frame, FrameSize + 1) < 0) {
                                    std::cerr << "fifo write failed from sender node" << std::endl;
                                } else {
                                    DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
                                }

                            }
                            close(fd);
                            temp++;
                        }
                        retransmit =false;
                    }


                }
            }

            void run(){
                DEBUG("Transmission start")
                vector<int> dataArray ={12, 45, 13, 56, 76, 67, 43, 23, 15};
                send(dataArray);
                std::cout<<std::endl<<"Data transmission complete"<<std::endl;
            }

        };

//---------------------------------------------SELECTIVE REPEAT-----------------------------------------------


        class SELECTIVEREPEAT:public Base{

            std::atomic<int> Sn, Sf, Sw;
            int M;
            std::unordered_map<int, bytestream_t> cache;
//            fc::syncedBuffer_t tempBuffer;

            //0 -> fresh transmit 1-> transmitted 2-> need retransmission(timed out)
            std::vector<std::atomic<int>> gotResponse;

        public:

            SELECTIVEREPEAT(pid_t mac, std::string fifo, std::string fifoW): Base( mac,fifo, fifoW){

                DEBUG("mac");
                DEBUG(srcMac);
                DEBUG(destMac)
                M=3;
                Sn=0;
                Sf=0;
                Sw= std::pow(2, M-1);
            }


            //Timeout function.
            inline void Timeout(int idx){
                sleep(10);
                if(gotResponse[idx]!=1){
                    gotResponse[idx]=2;
                }
                return;
            }

            //all write of ethernet frames to controller via a single thread.
//            void DeliverData(){
//                while(true) {
//                    bytestream_t data = tempBuffer.pop();
//                    char frame[fc::FrameSize + 1];
//                    strcpy(frame, (char *) data.c_str());
//
//                    int fd = open(fifoPipeW.c_str(), O_WRONLY);
//                    if (fd == -1) {
//                        std::cerr << "pipe open error\n";
//                    }
//                    {
////                        std::lock_guard<std::mutex> lk(lock);
//
//                        if (write(fd, frame, FrameSize + 1) < 0) {
//                            std::cerr << "fifo write failed from sender node" << std::endl;
//                        } else {
//                            DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
//                        }
//
//                    }
//                    close(fd);
//                }
//            }

            int isNak(int ack){
                ack = pow(2,8)-1-ack;
                return ack<gotResponse.size()? ack:-1;
            }

            //running on a separate thread.
            void Recv_ACK() {
                while (Sf<=Sn) {
                    descriptor_t fd = open((char *) fifoPipe.c_str(), O_RDONLY);
//                std::cout<<std::endl<<"fifo open"<<std::endl;

                    char ack_frame[ACKSIZE + 1];
                    int status;
                    {
                        //free the pipe by consuming the whole.
                        while ((status = read(fd, ack_frame, fc::ACKSIZE + 1)) > 0) {
                            //noisy frame dropping it.
                            if (!frame::checkFrameIntegrity(bytestream_t(ack_frame))) {
                                std::cout << "Noisy acknowledgement received...Discarding..." << std::endl;
                                continue;
                            }

                            int ackNo = frame::getSEQNO(bytestream_t(ack_frame));

                            //if the acknowledge is a NAK
                            if(isNak(ackNo)!=-1){
                                int nak = isNak(ackNo);

                                if(nak>=Sf && nak<Sn){
                                    bytestream_t data = cache[nak];

                                    //sending data
//                                    tempBuffer.push(data);
                                    char frame[fc::FrameSize + 1];
                                    strcpy(frame, (char *) data.c_str());

                                    int ff = open(fifoPipeW.c_str(), O_WRONLY);
                                    if (fd == -1) {
                                        std::cerr << "pipe open error\n";
                                    }
                                    {
//                        std::lock_guard<std::mutex> lk(lock);

                                        if (write(ff, frame, FrameSize + 1) < 0) {
                                            std::cerr << "fifo write failed from sender node" << std::endl;
                                        } else {
                                            DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
                                        }

                                    }
                                    close(ff);

                                    std::thread thm([this,nak]{this->Timeout(nak);});
                                    thm.detach();
                                }
                                //----------NAK handled------------------------

                            }else {
                                //-----------ACK handled-----------------------
                                if (ackNo > Sf && ackNo <= Sn) {
                                    while (Sf <= ackNo) {
                                        gotResponse[Sf] = 1;
                                        Sf++;
                                    }
                                    while (Sf != gotResponse.size() && gotResponse[Sf] == 1) {
                                        Sf++;
                                    }
                                }
                            }
                            std::cout << "Acknowledge received with seq: " << (isNak(ackNo)==-1?ackNo:isNak(ackNo)) << std::endl;


//                    tempBuffer.push(bytestream_t(ack_frame));
                        }
                        if (status == -1)
                            std::cerr << std::this_thread::get_id() << " :fifo read error on -> " << fifoPipe
                                      << std::endl;
                    }
                    close(fd);
                }
            }

//            void GetData(){
//                Sn=sizeM-1;
//            }
            void send(vector<int> &dataArray) {

                //response array initialization
                std::vector<std::atomic<int> > gotResponseTemp(dataArray.size());
                for(auto & i: gotResponseTemp){
                    i=0;
                }
                gotResponse.swap(gotResponseTemp);


                bool retransmit =false;
                std::thread tt([this] { this->Recv_ACK(); });
                tt.detach();

//                std::thread delivery([this]{this->DeliverData();});
//                delivery.detach();

                while (Sf < dataArray.size() ) {

                    while( Sn < dataArray.size()) {

//                        //wait for the acknow.
//                        while(Sn-Sf>=Sw && !isTimeout){
//                            sleep(TIMEOUT);
//                        }

                        //need retransmission
                        if(Sn-Sf >=Sw){
                            retransmit = true;
                            break;
                        }


                        int d = dataArray[Sn];

                        bytestream_t data = frame::CreateFrame<int>(srcMac, destMac, d, Sn);
                        cache[Sn] = data; //store frame

                        char frame[fc::FrameSize + 1];
                        strcpy(frame, (char *) data.c_str());

                        int fd = open(fifoPipeW.c_str(), O_WRONLY);
                        if (fd == -1) {
                            std::cerr << "pipe open error\n";
                        }
                        {
//                        std::lock_guard<std::mutex> lk(lock);

                            if (write(fd, frame, FrameSize + 1) < 0) {
                                std::cerr << "fifo write failed from sender node" << std::endl;
                            } else {
                                DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
                            }

                        }
                        close(fd);

                        Sn++;
                        int lambdaCapture = Sn-1;
                        std::thread tmut([this, lambdaCapture] { this->Timeout(lambdaCapture); });
                        tmut.detach();

                        //1 sec sleep for better result
                        sleep(1);
                    }
                    if( Sn >= dataArray.size() && Sf <=Sn && dataArray[Sf]==2){
                        retransmit=true;
                    }

                    //retransmission
                    if(retransmit){
                        int temp = Sf;
                        for(int i=temp; i<Sn;i++){
                            if(gotResponse[i]==2){

//                                tempBuffer.push(cache[i]);
                                bytestream_t data = cache[i];
                                char frame[fc::FrameSize + 1];
                                strcpy(frame, (char *) data.c_str());

                                int fd = open(fifoPipeW.c_str(), O_WRONLY);
                                if (fd == -1) {
                                    std::cerr << "pipe open error\n";
                                }
                                {
//                        std::lock_guard<std::mutex> lk(lock);

                                    if (write(fd, frame, FrameSize + 1) < 0) {
                                        std::cerr << "fifo write failed from sender node" << std::endl;
                                    } else {
                                        DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
                                    }

                                }
                                close(fd);
                                std::thread tim([this,i]{this->Timeout(i);});
                                tim.detach();
                            }
                        }
                        retransmit =false;
                    }
                }
            }

            void run(){
                DEBUG("Transmission start")
                send(dataArray);
                std::cout<<std::endl<<"Data transmission complete"<<std::endl;
            }

        };


    }
}

#endif