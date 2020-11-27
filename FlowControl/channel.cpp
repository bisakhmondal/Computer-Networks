#include<iostream>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>
#include<thread>
#include <string.h>
#include <filesystem>
using namespace std;

void MakeFiFO(string fifo){
    if(!std::filesystem::exists(fifo)){
        mkfifo(fifo.c_str(), 0644);
    }
}
class Channel{
    fc::syncedBuffer_t toReceiver, toSender;
    public:
    void fifoRead(string fifoname, int frameSize, bool bysender=true){
        MakeFiFO(fifoname);
        cout<<fifoname<<endl;
        while(true){
            fc::descriptor_t fd = open(fifoname.c_str(), O_RDONLY);
            char data[frameSize+1];
            int status;

            if((status=read(fd, data, frameSize+1))>0){

                close(fd);

                if(status==-1){
                    cerr<<"fifo read failed"<<endl;
                    continue;
                }

                if(bysender){
                    cout<<"read from sender"<<endl;
                }else{
                    cout<<"read from receiver"<<endl;
                }
                fc::bytestream_t d(data);
                if(bysender){
                    toReceiver.push(d);
                }else{
                    toSender.push(d);
                }
            }
        }
    }

    void fifoWrite(string fifoname, int framesize, bool bysender=true){
        MakeFiFO(fifoname);
        cout<<fifoname<<endl;
        while(true){
            fc::descriptor_t fd = open(fifoname.c_str(), O_WRONLY);

            fc::bytestream_t data;
            if(bysender){
                data = toReceiver.pop();
            }else{
                data = toSender.pop();
            }
            char d[framesize+1];
            strcpy(d, (char*)data.c_str());
            //add noises in 20%cases
            if(rand()%10>=8){
                addNoise(d, framesize);
            }
            if(write(fd, d, framesize+1)==-1){
                cerr<<"fifo write failed"<<endl;
//                if(bysender){
//                    toReceiver.push(data);
//                }else{
//                    toSender.push(data);
//                }
            }

            if(bysender){
                    cout<<"write to receiver"<<endl;
                }else{
                    cout<<"write to sender"<<endl;
                    cout<<"buffer size "<<toSender.len()<<" "<<toReceiver.len()<<endl<<endl;
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