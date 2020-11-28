#include<bits/stdc++.h>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>
#include<thread>
#include <string.h>
#include "frameutils.hpp"
#include "senders/stopNwait.hpp"
#include "senders/selectiverepeat.hpp"
#include "senders/gobackN.hpp"

//devdeps
// #include "sender.hpp"
#include<signal.h>
#include<sys/wait.h>

using namespace std;
#define DEBUG(x) cout<<"chk "<<x<<endl;


void MakeFiFO(string fifo){
    if(!std::filesystem::exists(fifo)){
        mkfifo(fifo.c_str(), 0644);
    }
}
class SenderController{

    fc::pidlist_t senderidlist, receiveridlist;
    fc::piperoutes_t routingtable, routingtableR;// for connection with multiple receivers. MAC vs pipes name
    
//    fc::locklist_t locks; // contains the mutexes for individual sender process
                         // for sync read write in all threads at sendercontroller

    fc::syncedBuffer_t buffer;
    fc::syncedBuffer_t ackBuffer;

    //read from shared memory
    void memory_Read(fc::SharedMemory &shmm){
        using namespace fc;
       
        for(int j=0;j<NUM_NODES;j++){
            pid_t k = *((int*)shmm.memptr);
            receiveridlist.push_back(k);
            cout<<k<<endl;
            shmm.memptr = (void *)((int*)shmm.memptr +1);
        }
    }

    /*
    read from fifos by multiple worker thread to provide seamless communication between connected nodes as sender.
     @params fifo_name  name of pipe
     
     @params mutex m & con_var c : to prevent read write on same process by two threads while sending and
     receiving to and from channel.
    */
    void workersProcess(const pid_t &sender_mac){
        
        string &fifo_name = routingtableR[sender_mac];

        while(true){
//            DEBUG("wprocess Acq")
            int fd = open(fifo_name.c_str(), O_RDONLY);
            if(fd ==-1){
                cerr<<fifo_name<<" fifo open error"<<endl;
            }
            char ethernet_frame[fc::FrameSize+1];
            int status;

            //free the pipe by consuming the whole.
            while((status=read(fd, ethernet_frame, fc::FrameSize+1))>0){
                buffer.push(fc::bytestream_t(ethernet_frame));
   
            }
//            lock.unlock();
            if(status==-1)
                cerr<<this_thread::get_id()<<" :fifo read error on ->"<< fifo_name<<endl;
            
            close(fd);

            // locknwait.c.wait(lock);
        }
    }

    //the function running on a thread responsible for returning acknowledgement frame received through channel.
    void recepientProcess(){
        while(true){
            
            fc::bytestream_t ack = ackBuffer.pop();

            char ackF[fc::ACKSIZE+1];
            strcpy(ackF, ack.c_str());

            pid_t ack_MAC = fc::frame::getMac(ack, "dest");//fc::frame::getMac(ack, )
            string fifopipe = routingtable[ack_MAC];
            
//            fc::Locker & locknwait = locks[ack_MAC];
            

            int fd = open(fifopipe.c_str(), O_WRONLY);
//            std::lock_guard<std::mutex> lock(locknwait.m);

            if(write(fd, ackF, fc::ACKSIZE+1)<0){
                cerr<<"fifo write error on ->"<< fifopipe<<endl;
                ackBuffer.push(ack);
            }

            close(fd);
            
//            locknwait.c.notify_one();
            
        }
    }

    //the main process will execute this. ack receiving from channel
    void mainProcessC(){
//        cout<<"mainProcessto chan"<<endl;
        MakeFiFO(fc::C2SFIFO.c_str());

        while(true){
            fc::descriptor_t fd = open(fc::C2SFIFO.c_str(), O_RDONLY);
            if(fd==-1){
                cerr<<fc::C2SFIFO<<" open error"<<endl;
            }
            char  ackFrame[fc::ACKSIZE+1];
            while(read(fd, ackFrame, fc::ACKSIZE+1)>0){
//                cout<<" ack received from channel(controller) "<<endl;
                if(!fc::frame::checkFrameIntegrity(fc::bytestream_t(ackFrame))){
                    cout<<"corrupted acknowledgement frame received...discarding..."<<endl;
                    continue;
                }
                ackBuffer.push(string(ackFrame));
            }
            close(fd);
        }
    }
    
    void mainProcessS(){
//        cout<<"mainProcess from chan"<<endl;
        MakeFiFO(fc::S2CFIFO.c_str());

        while(true){
            fc::descriptor_t fd = open(fc::S2CFIFO.c_str(), O_WRONLY);
            fc::bytestream_t frame = buffer.pop();
            char  ip[fc::FrameSize+1];
            strcpy(ip, (char *)frame.c_str());
            if(write(fd, ip, fc::FrameSize+1)<0){
                cerr<<"frame sending to channel failed"<<endl;
                buffer.push(frame);
            }
            close(fd);
        }
    }

    public:
    void run(){

       {
        fc::SharedMemory shm(fc::SHMBACKINGFILE, fc::ALLOCATE_BYTES, "open");
        fc::BinSemaphore sem(fc::SHMMUTEX,0, "open");

        //getting the receiver ip addresses synchronously.
        // cout<<"sem count" <<sem.semVal()<<endl;
        if(!sem.semWAIT()){
            memory_Read(shm);
        }

        if(sem.semPOST()<0) fc::report_and_exit("sempost error");
       }
//        cout<<"done"<<endl;
//
////
//receiveridlist.push_back(1000);
//

        int cnt = 0;
        //initialization of sender nodes.
        for(const auto &r_mac: receiveridlist){
           pid_t sender_id;

MakeFiFO(fc::S2SSFIFO+ "_" +to_string(cnt));
            MakeFiFO(fc::S2SSFIFO+ "_R" +to_string(cnt));

           if((sender_id=fork())==0){
               //sender node
                
                cout<<"node init "<<cnt<<endl;

            //    fc::Sender::StopNWait(r_mac, fc::S2SSFIFO+ "_" +to_string(cnt), fc::S2SSFIFO+ "_R" +to_string(cnt)).run();
              fc::Sender::GOBACKN(r_mac, fc::S2SSFIFO+ "_" +to_string(cnt), fc::S2SSFIFO+ "_R" +to_string(cnt)).run();
            //    fc::Sender::SELECTIVEREPEAT(r_mac, fc::S2SSFIFO+ "_" +to_string(cnt), fc::S2SSFIFO+ "_R" +to_string(cnt)).run();

                return;
           }else{
            //    main 
               senderidlist.push_back(sender_id);
               routingtable[sender_id] = fc::S2SSFIFO+ "_" +to_string(cnt);
               routingtableR[sender_id] = fc::S2SSFIFO+ "_R" +to_string(cnt);
               cnt++;
           }
        }

        // //initializing threads for duplex communication.
        for(const auto &s_mac: senderidlist){
//            locks[s_mac] = fc::Locker();
            DEBUG("New thread -> Controller")
//            string frame = fc::frame::CreateFrame(11, s_mac, -1, 1, true);
//            cout<<"Frame to be pushed "<<frame<<endl<<endl;
            std::thread t(&SenderController::workersProcess, this, s_mac);
            t.detach();
        }
//        sleep(15);
        //recepient thread
        std::thread rec(&SenderController::recepientProcess, this);
        rec.detach();

        std::thread tochan(&SenderController::mainProcessS, this);
        std::thread fromchan(&SenderController::mainProcessC, this);
        for(auto &smac: senderidlist){
            waitpid(smac, NULL, 0);
        }
        tochan.join();
        fromchan.join();
    }
};

int main(){
    DEBUG("MAIN")
    SenderController sen;
    sen.run();
    return  0;

}