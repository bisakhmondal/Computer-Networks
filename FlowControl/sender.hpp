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
            virtual void send(const int &)= 0;
            virtual void Recv_ACK() =0;
        };

        class StopNWait:public Base{
            // float sendingTime, receivingTime;
            // std::atomic<bool> isTimeout;

            int Sn;

            std::queue<bytestream_t> tempBuffer;
            public:

            StopNWait(pid_t mac, std::string fifo): Base( mac,fifo){
                // isTimeout = false; 
                // receivingTime =-1;
                Sn=0;
            }


            //Timeout function.
            inline bool Timeout(std::unique_lock<std::mutex> &lk){
                std::cout<<"at timeout\n";
                return lock.c.wait_for(lk, std::chrono::seconds(10))== std::cv_status::timeout;
            }

            void Recv_ACK(){
                
                std::cout<<std::endl<<"fifo open"<<std::endl;
                descriptor_t fd = openFifo(fifoPipe, O_RDONLY);

                char * ack_frame;
                int status;
                //free the pipe by consuming the whole.
                while((status=read(fd, ack_frame, fc::ACKSIZE))>0){
                    tempBuffer.push(bytestream_t(ack_frame));
                }
                if(status==-1)
                    std::cerr<<std::this_thread::get_id()<<" :fifo read error on ->"<< fifoPipe<< std::endl;
                
                close(fd);

                lock.c.notify_one();
            }

            void send(const int &d){
                using namespace std;
                bytestream_t data =  frame::CreateFrame<int>(srcMac, destMac, d, Sn);
                cout<<"frame generated"<<endl;
                bool Sent = true;
                
                Sn = (Sn+1)%2;

                do{
                    std::unique_lock<std::mutex> lk(lock.m);
                    std::cout<<"lock acquired\n";
                    descriptor_t fd = openFifo(fifoPipe, O_WRONLY);
                    if(write(fd, data.c_str(), FrameSize)<0){
                        std::cerr<<"fifo write failed from sender node"<<std::endl;
                    }

                    close(fd);
                    cout<<"frame generated2"<<endl;
                    
                    std::thread tt([this]{this->Recv_ACK();});
                    cout<<"after rec"<<endl;
                    tt.detach();
                //= new std::thread
                    //timeout using unique lock.
                    if(Timeout(lk)){
                        cout<<"detached "<<d<<endl;
                        // delete tt;//not going with delete as at d
                        Sent=false;
                        continue;
                    }
                    lk.unlock();
                    // Not corrupted and ackNum =Sn
                        bool isIn=false;
                        
                        while(!tempBuffer.empty()){
                            bytestream_t ackFrame = tempBuffer.front();
                            tempBuffer.pop();
                            
                            if(!frame::checkFrameIntegrity(ackFrame))
                                continue;
                            
                            int ackNum = frame::getSEQNO(ackFrame);
                            
                            if(ackNum==Sn)
                                isIn=true;
                        }

                        if(!isIn)
                            Sent = false;
                    
                }while(!Sent);

            }

            void run(){

                for(auto &i: STAICDATA){
                    send(i);
                }
            }

        };
    }
}

#endif