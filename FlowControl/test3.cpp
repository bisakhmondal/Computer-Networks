#include<bits/stdc++.h>
#include<unistd.h>
using namespace std;
#include "sender.hpp"
#include "frameutils.hpp"
#include "constants.hpp"
#include "typedefs.hpp"
#include "fcntl.h"
#include "sys/wait.h"

void workersProcess(string fifo_name){
//
//    cout<<"WProcess"<<endl;
//    string &fifo_name = routingtable[sender_mac];
//    fc::Locker &locknwait = locks[sender_mac];

    //unique_lock to release while execution

//    while(true){
//        DEBUG("wprocess Acq")
//        std::unique_lock<std::mutex> lock(locknwait.m);
        int fd = open((char*)fifo_name.c_str(), O_RDONLY);
        DEBUG("pipe openned at receiver");
        if(fd<0){
            cerr<<"openning error "<<endl;
        }
        char ethernet_frame[fc::FrameSize+1] ;//= new char[10];
        int status;

        //free the pipe by consuming the whole.
        if(read(fd, ethernet_frame, fc::FrameSize+1)>0){
//            buffer.push(ethernet_frame);
    cout<<"data reading"<<endl;
            int data = fc::frame::getData(fc::bytestream_t(ethernet_frame));
            cout<<"data received:::::::::::: "<<data<<endl;

        }
//    if(status==-1)
//        cerr<<"fifo read error on -"<<endl;

//    cout<<"status: "<<status<<endl;
//        lock.unlock();

        close(fd);

        // locknwait.c.wait(lock);
//    }
}
int main(){
    string eth_frame = fc::frame::CreateFrame(100, 120, 130, 1);
    if(fork()==0){
        fc::Sender::StopNWait(100, fc::S2SSFIFO+ "_" +to_string(10)).run();
//            cout<<"opening pipe WR"<<endl;
//            int fd = open((char*)(fc::S2SSFIFO+ "_" +to_string(10)).c_str(), O_WRONLY);
//            if(fd<0){
//                cerr<<"manual error fifo open "<<endl;
//            }
////            string eth_frame="101010101";
//            char eth[eth_frame.length()+1];
//            strcpy(eth,(char*)eth_frame.c_str());
////            eth[eth_frame.length()] = '\0';
////            if(eth[strlen]){
////                cout<<"10 length str"<<endl;
////            }
////             if(eth(eth))
//            if(write(fd, eth, fc::FrameSize+1)>0){
//                cout<<"frame sent"<<endl;
//            }
//
//            close(fd);
        return 0;
    }else{
        thread t(workersProcess,fc::S2SSFIFO+ "_" +to_string(10) );

        t.join();
    }

    cout<<"Done"<<endl;
    wait(NULL);
    sleep(2);
    return 0;
}