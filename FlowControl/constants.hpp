#ifndef FLOW_CONTROL_CONTANTS
#define FLOW_CONTROL_CONTANTS

#include<iostream>

namespace fc{
    
    const int NUM_NODES = 4;// number of sender and receiver
    const int FrameSize=64;
    const int ACKSIZE = 64;
    //fifos
    const std::string S2CFIFO = "fifos/sender2channel";
    const std::string S2CMUTEX = "/semsender2channel";
    const std::string C2RFIFO = "fifos/channel2receiver";
    const std::string C2RMUTEX = "/semchannel2receiver";
    //more internal fifos intialization
    const std::string S2SSFIFO = "fifos/sender2subsender";
    const std::string R2RRFIFO = "fifos/receiver2subreceiver";
    //shared Memory
    const std::string SHMBACKINGFILE = "shmbacking";
    const std::string SHMMUTEX = "/semmutex";
    const int ALLOCATE_BYTES = 128;

    //data(for simplicity decimal representation) obtained from higher layers in OSI / tcp suite.
    const int data[] = {10,20,12,18,24,27};

    const int TIMEOUT = 1; //timeout in 1 second

}

#endif