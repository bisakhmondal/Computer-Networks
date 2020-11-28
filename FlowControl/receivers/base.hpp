#ifndef FLOW_CONTROL_RECEIVER_BASE
#define FLOW_CONTROL_RECEIVER_BASE

#include "../typedefs.hpp"
#include<thread>
#include"../utils.hpp"
#include"../constants.hpp"
#include "../frameutils.hpp"
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

    }
}

#endif