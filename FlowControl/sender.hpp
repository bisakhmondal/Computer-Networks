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
            virtual void send(int)= 0;
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
                for(auto &i: STAICDATA){
                    send(i);
                    std::cout<<std::endl;
                    DEBUG("Making next frame")
                    sleep(5);
                }
                DEBUG("data transmission complete")
            }

        };
    }
}

#endif