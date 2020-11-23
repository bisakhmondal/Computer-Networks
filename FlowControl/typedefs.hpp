#ifndef FLOW_CONTROL_TYPEDEFS
#define FLOW_CONTROL_TYPEDEFS

#include<bitset>
#include<unordered_map>
#include<vector>
#include<queue>
#include "threadSafeQueue.hpp"

namespace fc{
    typedef std::bitset<48> addr_t; // 6 byte address types in binary
    typedef int8_t descriptor_t;
    typedef int pid_t;
    typedef std::unordered_map<int, std::string> piperoutes_t;
    typedef std::unordered_map<int, Locker> locklist_t;
    typedef std::vector<pid_t> pidlist_t;
    
    typedef fc::SyncedQueue<char*> syncedBuffer_t;

}

#endif
