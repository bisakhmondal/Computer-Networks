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

class ReceiverController{
    fc::descriptor_t r2clink;
    fc::pidlist_t receiveridlist;
    fc::piperoutes_t routingtable;// for connection with multiple receivers
   
    void memory_Write(fc::SharedMemory &shm){
        using namespace fc;
        for(auto id: receiveridlist){
            *((pid_t*)shm.memptr) = id;

            shm.memptr = (void *)((pid_t*)shm.memptr +1);
        }
    }

    public:
    void run(){

        for(int i=0; i<fc::NUM_NODES; i++){
            pid_t id;
            if((id=fork())==0){
                //child process
                // Receiver().init();
                return;
            }else{
                cout<<id<<endl;
                receiveridlist.push_back(id);
            }
        }
        fc::SharedMemory shm(fc::SHMBACKINGFILE, fc::ALLOCATE_BYTES);
        fc::BinSemaphore sem(fc::SHMMUTEX);
        //synchronus memory write using semaphore.
        memory_Write(shm);
        sleep(10);
        if(sem.semPOST()<0) fc::report_and_exit("sempost error");
        sleep(10);


    }
};

int main(){
    ReceiverController rc;
    rc.run();
    return  0;

}