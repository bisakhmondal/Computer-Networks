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
        public:
        mutable std::mutex m;
        std::condition_variable c;
    };

    void report_and_exit(const char* msg) {
    
        std::perror(msg);
        std::exit(1);

    }

    bool is_file_exist(const char *fileName)
    {
        std::ifstream in(fileName);
        return in.good();
    }
    descriptor_t openFifo(std::string name, int flag){
        if(!is_file_exist(name.c_str()))
            mkfifo(name.c_str(), 0644);

        std::cout<<"fifo C"<<std::endl;
        descriptor_t fd = open(name.c_str(), flag);
        std::cout<<"fifo O"<<std::endl;
        if (fd==-1) report_and_exit("fifo open error");

        return fd;
    }

}

#endif
