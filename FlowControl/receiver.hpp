#ifndef FLOW_CONTROL_SENDER
#define FLOW_CONTROL_SENDER

#include "typedefs.hpp"
#include<thread>
#include"utils.hpp"
#include"constants.hpp"
#include<fcntl.h>
#include<cstdlib>
#include<unistd.h>
namespace fc{
    namespace Receiver{
        class Base{
            protected:
            int Rn;
            std::string chanlink;
            public:
            Base(std::string fifoname): chanlink(fifoname){ Rn=0;}
            virtual void Recv()= 0;
            
        };

        class StopNWait: public Base{

            public:
            void Recv(){
                while(true){
                    int fd = openFifo(chanlink, O_RDONLY);
                    char *eth_frame;
                    if(read(fd, eth_frame, FrameSize)==-1){
                        report_and_exit("receiver read error");
                    }
                    //checksum check to be done...
                    /*
                    if(frameseq==Rn){
                        extractFrame(eth_frame);
                        Rn++;
                     }

                    
                     */


                }
            }
        };




    }

}

#endif