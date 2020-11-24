#ifndef FLOW_CONTROL_UTILS
#define FLOW_CONTROL_UTILS

#include<iostream>
#include<fstream>
#include<fcntl.h>
#include<sys/stat.h>
#include "typedefs.hpp"
#include<condition_variable>

namespace fc{
    
    //Locker object for threads
    class Locker{
        int k;
        public:
        mutable std::mutex m;
        std::condition_variable c;

        Locker& operator = (const Locker &o){
            return *this;
        }
    };

    void report_and_exit(const char* msg) {
    
        std::perror(msg);
        std::exit(1);

    }

    bool is_file_exist(const char *fileName)
    {
        // std::cout<<"checking fifo\n";
        std::ifstream in(fileName);
        // std::cout<<in.good()<<"\n";
        return in.good();
    }

    int openFifo(std::string name, int flag){
        if(!is_file_exist(name.c_str())){
            // std::cout<<"creating fifo"<<name<<"\n";
            int kk= mkfifo(name.c_str(), 0644);
            if(kk<0)
                report_and_exit("fifo create error");
        }

        // std::cout<<"fifo C"<<std::endl;
        int fd = open(name.c_str(), flag);
        // std::cout<<"fifo O"<<std::endl;
        if (fd==-1) report_and_exit("fifo open error");

        return fd;
    }

}

#endif
