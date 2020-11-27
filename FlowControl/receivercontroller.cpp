#include<iostream>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>
#include<thread>
#include "frameutils.hpp"
#include "receiver.hpp"
#include<filesystem>

//devdeps
#include<signal.h>
#include<sys/wait.h>

using namespace std;

void MakeFiFO(string fifo){
    if(!std::filesystem::exists(fifo)){
        mkfifo(fifo.c_str(), 0644);
    }
}


class ReceiverController{
    fc::descriptor_t r2clink;
    fc::pidlist_t receiveridlist;
    fc::piperoutes_t routingtable, routingtableR;// for connection with multiple receivers
    
    fc::locklist_t locks; // contains the mutexes for individual receiver process
                         // for sync read write in all threads at receivercontroller

    fc::syncedBuffer_t buffer, ackBuffer;
   
    void memory_Write(fc::SharedMemory &shm){
        using namespace fc;
        for(auto id: receiveridlist){
            *((pid_t*)shm.memptr) = id;

            shm.memptr = (void *)((pid_t*)shm.memptr +1);
        }
    }

    /*
    read from fifos by multiple worker thread to provide seamless communication between connected nodes as receiver.
     @params fifo_name  name of pipe
     
     @params mutex m & con_var c : to prevent read write on same process by two threads while sending and
     receiving to and from channel.
    */
    void workersProcess(const pid_t &recv_mac){
        
        string &fifo_name = routingtableR[recv_mac];
//        fc::Locker &locknwait = locks[recv_mac];

        //unique_lock to release while execution

        while(true){


//                std::unique_lock<std::mutex> lock(locknwait.m);
//                locknwait.c.wait(lock);
                int fd = open(fifo_name.c_str(), O_RDONLY);
            if(fd ==-1 ){
                cerr<<"fifo open error"<<fifo_name<<endl;
                sleep(100);
            }
            char ack_frame[fc::ACKSIZE +1 ];
            int status;


//                cout<<"RDONLY lock acquired"<<endl;
                //free the pipe by consuming the whole.
                while ((status = read(fd, ack_frame, fc::ACKSIZE+1)) > 0) {
//                    cout<<"ack received "<<ack_frame<<endl;
                    ackBuffer.push(string(ack_frame));
                }
//                lock.unlock();

            if(status==-1) {
                cout<<ack_frame<<endl;
                cerr << "fifo read error on -> " << fifo_name << endl;
                sleep(100);
            }
            close(fd);

//            locknwait.c.wait(lock);
        }
    }

    //the function running on a thread responsible for returning ethernet frame received through channel.
    void recepientProcess(){
        while(true){

            fc::bytestream_t eth = buffer.pop();
//            cout<<"eth from buffer "<<eth.length()<<endl;

            char ethF[fc::FrameSize+1];
            strcpy(ethF, (char*)eth.c_str());

            pid_t eth_MAC = fc::frame::getMac(eth, "dest");
            cout<<"dest mac "<<eth_MAC<<" | src mac "<<fc::frame::getMac(eth)<<endl;
            string fifopipe = routingtable[eth_MAC];
//            fc::Locker & locknwait = locks[eth_MAC];

//            cout<<"pipe to be opened WRONLY -> "<<fifopipe<<endl;

            fc::descriptor_t fd = open(fifopipe.c_str(), O_WRONLY);
//            cout<<"pipe opened WRONLY "<<endl;

            if(fd ==-1){
                cerr<<"fifo open error controller "<<fifopipe<<endl;
                sleep(100);
            }
            {
//                cout << "return frame " << endl;
//                std::unique_lock<std::mutex> lock(locknwait.m);
//                cout << "acquired lock" << endl;

                if (write(fd, ethF, fc::FrameSize + 1) < 0) {
                    cerr << "fifo write error on ->" << fifopipe << endl;
                    sleep(100);
                    buffer.push(eth);
                }
//                cout << "wrote to subR" << endl;
//                lock.unlock();

                close(fd);
//                locknwait.c.notify_all();
            }

        }
    }

    //the main process will execute this. eth receiving from channel
    void mainProcessC(){
        MakeFiFO(fc::C2RFIFO);
        while(true){
            fc::descriptor_t fd = open(fc::C2RFIFO.c_str(), O_RDONLY);
            if(fd==-1){
                cerr<<"fifo open error"<<fc::C2RFIFO<<endl;
            }
            char ethFrame[fc::FrameSize+1];
            while(read(fd, ethFrame, fc::FrameSize+1)>0){
                if(!fc::frame::checkFrameIntegrity(fc::bytestream_t(ethFrame))){
                    cout<<"corrupted ethernet frame received...discarding..."<<endl;
                    continue;
                }
                buffer.push(fc::bytestream_t (ethFrame));
            }
            close(fd);
        }
    }
    
    void mainProcessS(){
        MakeFiFO(fc::R2CFIFO);
        while(true){
            fc::descriptor_t fd = open(fc::R2CFIFO.c_str(), O_WRONLY);
            if(fd==-1){
                cerr<<"fifo open error"<<fc::R2CFIFO<<endl;
            }

            fc::bytestream_t ackframe = ackBuffer.pop();
            char ip[fc::ACKSIZE+1];
            strcpy(ip,(char *)ackframe.c_str());
            if(write(fd, ip, fc::ACKSIZE+1)<0){
                cerr<<"frame sending to channel failed"<<endl;
                ackBuffer.push(ackframe);
            }
            cout<<"--write to channel from receiver--"<<endl;
            close(fd);
            cout<<"buffer size "<<buffer.len()<<" "<<ackBuffer.len()<<endl<<endl;

        }
    }


    public:
    void run(){

        for(int i=0; i<fc::NUM_NODES; i++){
            pid_t id;
            MakeFiFO(fc::R2RRFIFO+"__"+to_string(i));
            MakeFiFO(fc::R2RRFIFO+"__R"+to_string(i));


            if((id=fork())==0){
                //child process
//                fc::Receiver::StopNWait(fc::R2RRFIFO+"__"+ to_string(i), fc::R2RRFIFO+"__R"+to_string(i)).run();
                fc::Receiver::GOBACKN(fc::R2RRFIFO+"__"+ to_string(i), fc::R2RRFIFO+"__R"+to_string(i)).run();

                return;
            }else{
                cout<<id<<endl;
                receiveridlist.push_back(id);
                routingtable[id] = fc::R2RRFIFO+"__"+to_string(i);
                routingtableR[id] = fc::R2RRFIFO+"__R"+to_string(i);
            }
        }


        fc::SharedMemory shm(fc::SHMBACKINGFILE, fc::ALLOCATE_BYTES);
        fc::BinSemaphore sem(fc::SHMMUTEX);
        //synchronus memory write using semaphore.
        memory_Write(shm);
        if(sem.semPOST()<0) fc::report_and_exit("sempost error");
    

        // //initializing threads for duplex communication.
        for(const auto &r_mac: receiveridlist){
            locks[r_mac] = fc::Locker();

//            string frame = fc::frame::CreateFrame(11, receiveridlist[0], 10, 0);
//            cout<<"Frame to be pushed "<<frame<<endl<<routingtable[r_mac]<<endl;
            std::thread t(&ReceiverController::workersProcess, this, r_mac);
            t.detach();
        }



        //recepient thread
        std::thread rec(&ReceiverController::recepientProcess, this);
        rec.detach();

        std::thread fromchan(&ReceiverController::mainProcessC, this);
        std::thread tochan(&ReceiverController::mainProcessS, this);
        for(auto &i: receiveridlist){
            waitpid(i, NULL, 0);
        }
        fromchan.join();
        tochan.join();
    }
};

int main(){
    ReceiverController rc;
    rc.run();
    return  0;

}