#ifndef FLOW_CONTROL_RECEIVER
#define FLOW_CONTROL_RECEIVER

#include "typedefs.hpp"
#include<thread>
#include"utils.hpp"
#include"constants.hpp"
#include<fcntl.h>
#include<cstdlib>
#include<unistd.h>
#include<string.h>
#include <atomic>
#include <cmath>

namespace fc{

    namespace Receiver{
    
        class Base{
            protected:
            int Rn;
            std::string chanlink, chanlinkW;
            pid_t srcMac;

//            mutable std::mutex lock;

            public:
            Base(std::string fifoname, std::string fifo2): chanlink(fifoname), chanlinkW(fifo2){
                srcMac = getpid();
                Rn=0;
            }
            virtual void Recv()= 0;
            virtual void Send_Ack(bytestream_t ack) =0;
            
        };

        class StopNWait: public Base{

            public:
            StopNWait(std::string fifo, std::string fifo2): Base(fifo, fifo2){}

            void Send_Ack(bytestream_t ack){
                bool status = false;
                std::cout<<"sending ACk\n";

                while(!status){
                    descriptor_t fd = open(chanlinkW.c_str(), O_WRONLY);

                    char ip[ACKSIZE+1];
                    strcpy(ip,(char*)ack.c_str());
                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (write(fd, ip, ACKSIZE + 1) > 0) {
                            std::cout<<"Acknowledgement Sent. | Seq no: "<<frame::getSEQNO(ack)<<std::endl;
                            status = true;
                        }
                    }
                    close(fd);
                }
            }

            void Recv(){

                while(true){

                    std::cout<<"receiver node\n";
                    descriptor_t fd = open(chanlink.c_str(), O_RDONLY);

                    if(fd==-1){
                        std::cerr<<"fifo open error on receiver node"<<chanlink<<std::endl;
                        sleep(100);
                    }
                    char eth_frame[FrameSize + 1];

                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (read(fd, eth_frame, FrameSize + 1) == -1) {
                            std::cerr << "fifo read error" << std::endl;
//                            continue;
                        }
//                        std::cout<<"receiver node read op\n";

                    }
//                    std::cout<<"frame to be decoded module : "<<eth_frame<<std::endl;

                    bytestream_t eth = bytestream_t(eth_frame);

                    if(!frame::checkFrameIntegrity(eth)){
                        std::cout<<"corrupted frame received...discarding..."<<std::endl;
                        continue;
                    }
                    std::cout<<"data received: "<<frame::getData(eth)<<" | sequence number "<<frame::getSEQNO(eth)<<std::endl;//frame::getData(eth)<<"seq: "<<frame::getSEQNO(eth)

                    if(frame::getSEQNO(eth)==Rn){
//                        int data = frame::getData(eth);
//                        std::cout<<"data received: "<<data<<std::endl;
                        Rn=(Rn+1)%2;
                    }

                    pid_t srcofFrame = frame::getMac(eth, "src");
                    //generating Acknowledgement Frame
                    bytestream_t ackFrame = frame::CreateFrame<int>(srcMac, srcofFrame, -1, Rn, true);
                    
                    std::thread th1(&StopNWait::Send_Ack, this, ackFrame);
                    th1.detach();
                }
            }


            void run(){
                Recv();
            }
        };

//-----------------------------------------GOBACKN-----------------------------------------------------


        class GOBACKN: public Base{

        public:
            GOBACKN(std::string fifo, std::string fifo2): Base(fifo, fifo2){}

            void Send_Ack(bytestream_t ack){
                bool status = false;
                std::cout<<"sending ACk\n";

                while(!status){
                    descriptor_t fd = open(chanlinkW.c_str(), O_WRONLY);

                    char ip[ACKSIZE+1];
                    strcpy(ip,(char*)ack.c_str());
                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (write(fd, ip, ACKSIZE + 1) > 0) {
                            std::cout<<"Acknowledgement Sent. | Seq no: "<<frame::getSEQNO(ack)<<std::endl;
                            status = true;
                        }
                    }
                    close(fd);
                }
            }

            void Recv(){

                while(true){

                    std::cout<<"receiver node\n";
                    descriptor_t fd = open(chanlink.c_str(), O_RDONLY);

                    if(fd==-1){
                        std::cerr<<"fifo open error on receiver node"<<chanlink<<std::endl;
                        sleep(100);
                    }
                    char eth_frame[FrameSize + 1];

                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (read(fd, eth_frame, FrameSize + 1) == -1) {
                            std::cerr << "fifo read error" << std::endl;
//                            continue;
                        }
//                        std::cout<<"receiver node read op\n";

                    }
//                    std::cout<<"frame to be decoded module : "<<eth_frame<<std::endl;

                    bytestream_t eth = bytestream_t(eth_frame);

                    if(!frame::checkFrameIntegrity(eth)){
                        std::cout<<"corrupted frame received...discarding..."<<std::endl;
                        continue;
                    }
                    std::cout<<"data received: "<<frame::getData(eth)<<" | sequence number "<<frame::getSEQNO(eth)<<std::endl;//frame::getData(eth)<<"seq: "<<frame::getSEQNO(eth)

                    if(frame::getSEQNO(eth)==Rn){
//                        int data = frame::getData(eth);
//                        std::cout<<"data received: "<<data<<std::endl;
                        Rn++;

                        pid_t srcofFrame = frame::getMac(eth, "src");
                        //generating Acknowledgement Frame
                        bytestream_t ackFrame = frame::CreateFrame<int>(srcMac, srcofFrame, -1, Rn, true);

                        std::thread th1(&GOBACKN::Send_Ack, this, ackFrame);
                        th1.detach();
                    }
                }
            }


            void run(){
                Recv();
            }
        };

//---------------------------------------SELECTIVE REPEAT-------------------------------------------------------


        class SELECTIVEREPEAT: public Base{

            std::vector<std::atomic<int>> gotResponse;
            std::atomic<bool> nakSent, ackNeeded;
        public:
            SELECTIVEREPEAT(std::string fifo, std::string fifo2): Base(fifo, fifo2){
                nakSent=false;
                ackNeeded=false;
            }

            void Send_Ack(bytestream_t ack){
                bool status = false;
                std::cout<<"sending ACk\n";

                while(!status){
                    descriptor_t fd = open(chanlinkW.c_str(), O_WRONLY);

                    char ip[ACKSIZE+1];
                    strcpy(ip,(char*)ack.c_str());
                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (write(fd, ip, ACKSIZE + 1) > 0) {
                            std::cout<<"Acknowledgement Sent. | Seq no: "<<frame::getSEQNO(ack)<<std::endl;
                            status = true;
                        }
                    }
                    close(fd);
                }
            }

            void Recv(){

                //response array initialization
                std::vector<std::atomic<int> > gotResponseTemp(dataArray.size());
                for(auto & i: gotResponseTemp){
                    i=0;
                }
                gotResponse.swap(gotResponseTemp);

                while(true) {

                    std::cout << "receiver node\n";
                    descriptor_t fd = open(chanlink.c_str(), O_RDONLY);

                    if (fd == -1) {
                        std::cerr << "fifo open error on receiver node" << chanlink << std::endl;
                        sleep(100);
                    }
                    char eth_frame[FrameSize + 1];

                    {
//                        std::lock_guard<std::mutex> lk(lock);

                        if (read(fd, eth_frame, FrameSize + 1) == -1) {
                            std::cerr << "fifo read error" << std::endl;
//                            continue;
                        }
//                        std::cout<<"receiver node read op\n";
                    }

                    bytestream_t eth = bytestream_t(eth_frame);

                    if (!frame::checkFrameIntegrity(eth)) {
                        std::cout << "corrupted frame received...discarding..." << std::endl;
                        continue;
                    }
                    std::cout << "data received: " << frame::getData(eth) << " | sequence number "
                              << frame::getSEQNO(eth) << std::endl;//frame::getData(eth)<<"seq: "<<frame::getSEQNO(eth)

                    int curSeq = frame::getSEQNO(eth);
                    pid_t srcofFrame = frame::getMac(eth, "src");

                    bytestream_t ackFrame;

                    if (curSeq != Rn && !nakSent) {
                        //nakFrame has seq 2^8 -Rn -1
                        ackFrame = frame::CreateFrame<int>(srcMac, srcofFrame, -1, std::pow(2, 8) - 1 - Rn, true);
                        nakSent = true;
                        std::thread th1(&SELECTIVEREPEAT::Send_Ack, this, ackFrame);
                        th1.detach();
                    }

                    if (curSeq < gotResponse.size() && gotResponse[curSeq] != 1) {
                        gotResponse[curSeq] = 1;
                        while (gotResponse[Rn] == 1) {
                            Rn++;
                            ackNeeded = true;
                        }
                    }
                    if (ackNeeded) {
                        ackFrame = frame::CreateFrame<int>(srcMac, srcofFrame, -1, Rn, true);
                        ackNeeded = false;
                        nakSent = false;
                        std::thread th1(&SELECTIVEREPEAT::Send_Ack, this, ackFrame);
                        th1.detach();
                    }
                }

            }


            void run(){
                Recv();
            }
        };

    }

}

#endif