#ifndef FLOW_CONTROL_SENDER_SELEC_REPEAT
#define FLOW_CONTROL_SENDER_SELEC_REPEAT

#include "base.hpp"

namespace fc{

    namespace Sender{

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
                        std::thread tmut(&SELECTIVEREPEAT::Timeout, this, lambdaCapture);
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
                                std::thread tim(&SELECTIVEREPEAT::Timeout, this, i);
                                tim.detach();
                            }
                            sleep(1);
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