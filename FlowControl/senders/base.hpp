#ifndef FLOW_CONTROL_SENDER_BASE
#define FLOW_CONTROL_SENDER_BASE

#include "../typedefs.hpp"
#include "../frameutils.hpp"
#include "../constants.hpp"
#include "../utils.hpp"
#include<thread>
#include<unistd.h>
#include<atomic>
#include <string.h>
#include <cmath>
#include<ctime>

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

    }
}

#endif