#include<iostream>
#include "constants.hpp"
#include "typedefs.hpp"
#include"utils.hpp"
#include "shared_memory.hpp"
#include "semaphore.hpp"
#include<fcntl.h>

//devdeps
#include<signal.h>
#include<sys/wait.h>

using namespace std;

class SenderController{
    fc::descriptor_t s2clink;
    fc::pidlist_t senderidlist, receiveridlist;
    fc::piperoutes_t routingtable;// for connection with multiple receivers
   
    //read from shared memory
    void memory_Read(fc::SharedMemory &shmm){
        using namespace fc;
       
        for(int j=0;j<NUM_NODES;j++){
            pid_t k = *((int*)shmm.memptr);
            receiveridlist.push_back(k);
            shmm.memptr = (void *)((int*)shmm.memptr +1);
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
        for(auto i: receiveridlist){
            cout<<i<<endl;
        }

        // for(int i=0; i<fc::NUM_NODES; i++){
        //     pid_t id;
        //     if((id=fork())==0){
        //         //child process
        //         return;
        //     }else{
        //         senderidlist.push_back(id);
        //     }
        // }
        //synchronus memory write using semaphore.
        // memory_Write(shm);
        
    }
};

int main(){
    SenderController sen;
    sen.run();
    return  0;

}