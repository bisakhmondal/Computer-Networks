#ifndef FLOW_CONTROL_FRAMEUTILS
#define FLOW_CONTROL_FRAMEUTILS

#include"typedefs.hpp"
namespace fc{

    template<class T>
    char * CreateFrame(pid_t srcMac, pid_t destMac, T data){
        std::string frame ="";
        frame += addr_t(srcMac);
        frame += addr_t(destMac);
        
        return "a";
    }
}

#endif