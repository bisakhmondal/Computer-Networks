#include<iostream>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>
#include<thread>

using namespace std;

class Channel{
    fc::syncedBuffer_t toReceiver, toSender;
    public:
    void fifoRead(string fifoname, int frameSize, bool bysender=true){
        while(true){
            fc::descriptor_t fd = fc::openFifo(fifoname, O_RDONLY);
            char * data;
            int status;
            if((status=read(fd, data, frameSize))>0){
                if(status==-1){
                    cerr<<"fifo read failed"<<endl;
                    continue;
                }
                string d(data);
                if(bysender){
                    toReceiver.push(d);
                }else{
                    toSender.push(d);
                }
            }
            close(fd);
        }
    }

    void fifoWrite(string fifoname, int framesize, bool bysender=true){
        while(true){
            fc::descriptor_t fd = fc::openFifo(fifoname, O_WRONLY);

            fc::bytestream_t data;
            if(bysender){
                data = toReceiver.pop();
            }else{
                data = toSender.pop();
            }
            char * d = &data[0];
            //add noises in 20%cases
            if(rand()%10>=8){
                addNoise(d, framesize);
            }
            if(write(fd, d, framesize)==-1){
                cerr<<"fifo write failed"<<endl;
            }
            close(fd);
        }
    }

    void addNoise(char * frame, int framesize){
        int numtimes = rand()%10;
        while(numtimes--){
            int pos = rand()%framesize;
            frame[pos] = frame[pos]=='1' ? '0':'1';
        }
    }
    void run(){
        thread t1(&Channel::fifoRead, this, fc::S2CFIFO, fc::FrameSize, true);
        thread t2(&Channel::fifoWrite, this, fc::C2RFIFO, fc::FrameSize, true);
        thread t3(&Channel::fifoRead, this, fc::R2CFIFO, fc::ACKSIZE, false);
        thread t4(&Channel::fifoWrite, this, fc::C2SFIFO, fc::ACKSIZE, false);

        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
};


int main(){
    Channel c;
    c.run();
    return 0;
}