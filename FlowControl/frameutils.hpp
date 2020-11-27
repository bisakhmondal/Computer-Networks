#ifndef FLOW_CONTROL_FRAMEUTILS
#define FLOW_CONTROL_FRAMEUTILS

#include"typedefs.hpp"
#include "include/library.h"
#include <string.h>
#include "crc.hpp"
#define DEBUG(x) std::cout<<"chk "<<x<<std::endl;

namespace fc{

    namespace frame{

    //IEEE 802.3 frame standard. @data :decimal or strings with only 0 & 1.
    // preamble + SFD + DMAC + SMAC + SEQNO + DATA + CRC
    template<class T>
    bytestream_t CreateFrame(pid_t srcMac, pid_t destMac, T data, int seq_number, bool ack=false){
        
        static const bytestream_t SFD = "10101011";
        std::bitset<7> preamble(0);
        for(int i=0; i<7;i+=2)
            preamble[i]=1;
        
        bytestream_t frame ="";
        frame+= preamble.to_string();
        
        frame +=SFD;

        frame += addr_t(destMac).to_string();
        frame += addr_t(srcMac).to_string();
        frame += std::bitset<8>(seq_number).to_string();
        if(!ack)
            frame += std::bitset<46>(data).to_string();
    //go error detection cross-compile builds
//         char crcip[frame.length()+1];
//         strcpy(crcip, (char *)frame.c_str());
//////         DEBUG("now crc")
//         char * crc = CRC(crcip);
//////         DEBUG(crc);
//        DEBUG(frame.length());
//         frame = bytestream_t(crc);
//        frame += std::bitset<32>(0).to_string();
        frame = CRCGEN(frame);

//         delete crc; //go cleanup
//         DEBUG(frame.length());
        return frame;
    }
    
    //go error detection cross-compile builds 
    bool checkFrameIntegrity(bytestream_t frame){
//         char  checkip[frame.length()+1];
//         strcpy(checkip, (char*)frame.c_str());
         return isCRCValid(frame);//CRCCheck(checkip);
    }

    pid_t getMac(const bytestream_t &frame, std::string node="src"){
        if(node=="src"){
            int offset = 7+8+48;
            bytestream_t srcmac = frame.substr(offset, 48);
            return std::bitset<48>(srcmac).to_ulong();
        }
        int offset = 7+8;
        bytestream_t destmac = frame.substr(offset, 48);

        return std::bitset<48>(destmac).to_ulong();
    }

    int getSEQNO(const bytestream_t &frame){
            int offset = 7+ 8+ 48*2 ;
            bytestream_t data = frame.substr(offset, 8);
            return std::bitset<8>(data).to_ulong();
    }

    int getData(const bytestream_t &frame){

            int offset = 7+ 8+ 48*2 + 8;
            bytestream_t data = frame.substr(offset, 46*8);
            return std::bitset<46>(data).to_ulong();
    }

    }
}

#endif