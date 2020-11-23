#ifndef FLOW_CONTROL_SENDER
#define FLOW_CONTROL_SENDER

#include "typedefs.hpp"
#include "frameutils.hpp"
#include "constants.hpp"
#include "utils.hpp"
#include<thread>
#include<unistd.h>
#include<atomic>

namespace fc{

    namespace Sender{

        class Base{
            protected:
            
            pid_t destMac, srcMac;
            std::string fifoPipe;
            fc::Locker lock; //prevention of readwrite on same fifo on same process.

            
            public:

            Base(pid_t mac, std::string fifo): destMac(mac), fifoPipe(fifo){
                
                srcMac = getpid();
            }
            virtual void send()= 0;
            virtual void Recv_ACK() =0;
        };

        class StopNWait:public Base{
            // float sendingTime, receivingTime;
            // std::atomic<bool> isTimeout;

            int Sn;

            std::queue<char *> tempBuffer;
            public:

            StopNWait(pid_t mac, std::string fifo): Base( mac,fifo){
                // isTimeout = false; 
                // receivingTime =-1;
                Sn=0;
            }


            //Timeout function.
            inline bool Timeout(std::unique_lock<std::mutex> &lk){
                return lock.c.wait_for(lk, std::chrono::seconds(TIMEOUT))== std::cv_status::timeout;
            }

            void Recv_ACK(){
                
                descriptor_t fd = openFifo(fifoPipe, O_RDONLY);

                char * ack_frame;
                int status;

                //free the pipe by consuming the whole.
                while((status=read(fd, ack_frame, fc::ACKSIZE))>0){
                    tempBuffer.push(ack_frame);
                }
                if(status==-1)
                    std::cerr<<std::this_thread::get_id()<<" :fifo read error on ->"<< fifoPipe<< std::endl;
                
                close(fd);

                lock.c.notify_one();
            }

            void send(const int &d){

                char * data =  CreateFrame<int>(srcMac, destMac, d);
                
                bool Sent = true;
                
                Sn++;

                do{
                    std::unique_lock<std::mutex> lk(lock.m);
                    descriptor_t fd = openFifo(fifoPipe, O_WRONLY);
                    if(write(fd, data, FrameSize)<0){
                        std::cerr<<"fifo write failed from sender node"<<std::endl;
                    }

                    close(fd);
                    std::thread *tt = new std::thread(Recv_ACK);
                
                    //timeout using unique lock.
                    if(Timeout(lk)){
                        delete tt;//not going with delete as at d
                        Sent=false;
                        continue;
                    }

                    lk.unlock();
                    /* Not corrupted and ackNum =Sn
                        bool isIn=false;
                        while(!tempBuffer.empty()){
                            char * ackFrame = tempBuffer.front();
                            tempBuffer.pop();
                            int ackNum = getAckNum(ackFrame);
                            if(ackNum==Sn)
                                isIn=true;
                        }
                        if(!isIn)
                            Sent = false;
                    */
                }while(!Sent);

            }

        };
    }
}

#endif