#include<iostream>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>
#include<thread>
#include "sender.hpp"

//devdeps
#include<signal.h>
#include<sys/wait.h>

using namespace std;

class SenderController{
    fc::pidlist_t senderidlist, receiveridlist;
    fc::piperoutes_t routingtable;// for connection with multiple receivers. MAC vs pipes name
    
    fc::locklist_t locks; // contains the mutexes for individual sender process 
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
        
        cout<<"WProcess"<<endl;
        string &fifo_name = routingtable[sender_mac];
        fc::Locker &locknwait = locks[sender_mac];

        //unique_lock to release while execution
        std::unique_lock<std::mutex> lock(locknwait.m);

        while(true){
            int fd = fc::openFifo(fifo_name, O_RDONLY);

            char * ethernet_frame;
            int status;

            //free the pipe by consuming the whole.
            while((status=read(fd, ethernet_frame, fc::FrameSize))>0){
                buffer.push(ethernet_frame);
               
            }

            if(status==-1)
                cerr<<this_thread::get_id()<<" :fifo read error on ->"<< fifo_name<<endl;
            
            close(fd);

            locknwait.c.wait(lock);
        }
    }

    //the function running on a thread responsible for returning acknowledgement frame received through channel.
    void recepientProcess(){
        cout<<"rProcess"<<endl;
        while(true){
            
            fc::bytestream_t ack = ackBuffer.pop();
            char * ackF = &ack[0];
            pid_t ack_MAC = 0;//fc::frame::getMac(ack, )
            string fifopipe = routingtable[ack_MAC];
            
            fc::Locker & locknwait = locks[ack_MAC];
            
            std::lock_guard<std::mutex> lock(locknwait.m);

            int fd = fc::openFifo(fifopipe, O_WRONLY);
            if(write(fd, ackF, fc::ACKSIZE)<0){
                cerr<<this_thread::get_id()<<" :fifo write error on ->"<< fifopipe<<endl;
            }
            close(fd);
            
            locknwait.c.notify_one();
            
        }
    }

    //the main process will execute this.
    void mainProcess(){
        fc::BinSemaphore sem(fc::S2CMUTEX, 1);
        
        if(!sem.semWAIT()){
            fc::descriptor_t fd = fc::openFifo(fc::S2CFIFO, O_WRONLY);

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
        cout<<"done"<<endl;
        int cnt =0;
        //initialization of sender nodes.
        for(const auto &r_mac: receiveridlist){
           pid_t sender_id;
           if((sender_id=fork())==0){
               //sender node
                
                cout<<"node init"<<endl;
                fc::Sender::StopNWait(r_mac, fc::S2SSFIFO+ "_" +to_string(cnt)).run();

                return;
           }else{
            //    main 
               senderidlist.push_back(sender_id);
               routingtable[sender_id] = fc::S2SSFIFO+ "_" +to_string(cnt);
               cnt++;
           }
        }
        cout<<"all done my part"<<endl;
        // waitpid(senderidlist[0],NULL,0);
        // //initializing threads for duplex communication.
        for(const auto &s_mac: senderidlist){
            locks[s_mac] = fc::Locker();
            
            std::thread t(&SenderController::workersProcess, this, s_mac);
            t.detach();
        }

        //recepient thread
        std::thread rec(&SenderController::recepientProcess, this);
        rec.detach();
        while(true){
            mainProcess();
        }       
        
    }
};

int main(){
    SenderController sen;
    sen.run();
    return  0;

}