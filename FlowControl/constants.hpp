#ifndef FLOW_CONTROL_CONTANTS
#define FLOW_CONTROL_CONTANTS

#include<iostream>

namespace fc{
    
    const int NUM_NODES = 1;// number of sender and receiver
    const int FrameSize=(7+ 8 + 2*48 + 8 + 46 + 32);
    const int ACKSIZE = FrameSize - 46;
    //fifos
    const std::string S2CFIFO = "fifos/sender2channel";
    const std::string C2SFIFO = "fifos/channel2sender";

    const std::string S2CMUTEX = "/semsender2channel";
    const std::string C2RFIFO = "fifos/channel2receiver";
    const std::string R2CFIFO = "fifos/receiver2channel";
    const std::string C2RMUTEX = "/semchannel2receiver";
    //more internal fifos intialization
    const std::string S2SSFIFO = "fifos/sender2subsender";
    const std::string R2RRFIFO = "fifos/receiver2subreceiver";
    //shared Memory
    const std::string SHMBACKINGFILE = "shmbacking";
    const std::string SHMMUTEX = "/semmutex";
    const int ALLOCATE_BYTES = 128;

    //data(for simplicity decimal representation) obtained from higher layers in OSI / tcp suite.
    const int STAICDATA[] = {15, 40, 60, 100, 75};
    const int sizeM = 4;

    const int TIMEOUT = 5; //timeout in 5 second

}

#endif