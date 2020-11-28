#ifndef FLOW_CONTROL_PROCESS_SEMAPHORE
#define FLOW_CONTROL_PROCESS_SEMAPHORE

#include<semaphore.h>
#include<cstdlib>
#include "utils.hpp"
namespace fc{

class BinSemaphore{
    std::string name;
    int val;

    public:
    sem_t * semptr;
    sem_t* semInit(){
        sem_t* semptr = sem_open(name.c_str(), O_CREAT, S_IRUSR|S_IWUSR, val);

        if(semptr == SEM_FAILED){
            report_and_exit("sem init failed");
        }
        return semptr;
    }
    sem_t* semOpen(){
        sem_t * semptr = sem_open(name.c_str(), O_EXCL,  0);
        if(semptr == SEM_FAILED){
            report_and_exit("sem open failed");
        }
        return semptr;
    }

    //semPOST
    int semPOST(){
        return sem_post(semptr);
    }

    //semWAIT
    int semWAIT(){
        return sem_wait(semptr);
    }

    int semVal(){
        int k;
        sem_getvalue(semptr, &k);
        return k;
    }

    //2 modes one for initialization another for openning.
    BinSemaphore(std::string n, int v=0, std::string mode="init"): name(n), val(v){
        if(mode=="init")
            this->semptr = semInit();
        else
            this->semptr = semOpen();
    }


    ~BinSemaphore(){
        // std::cout<<"des: "<<std::endl;
        sem_unlink(name.c_str());
        sem_destroy(semptr);
    }
};
}

#endif