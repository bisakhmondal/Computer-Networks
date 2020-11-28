#include<bits/stdc++.h>

using namespace std;
#include "include/crcCPP.h"
struct c{
    int k;
    // std::mutex m;
    c(int v): k(v){}
};
typedef std::bitset<48> addr_t;
    typedef int pid_t;

   template<class T>
    string CreateFrame(pid_t srcMac, pid_t destMac, T data){
        static const std::string SFD = "10101011";
        std::bitset<56> preamble(0);
        for(int i=0; i<56;i+=2)
            preamble[i]=1;
        cout<<"p"<<preamble<<endl;
        std::string frame ="";
        frame+= preamble.to_string();
        // frame+="--";
        frame +=SFD;
        // frame+="--";
        frame += addr_t(destMac).to_string();
        // frame+="--";
        frame += addr_t(srcMac).to_string();
        // frame+="--";
        frame += std::bitset<46*8>(data).to_string();
        // cout<<frame<<endl; 

        return frame;
    }
    const char * f(){
    string b= "bisakh";
    return b.c_str();}
// #include<
int main(){
    // unordered_map<int, c> map;
    // vector<c> vcc;
    // for(int i=0;i<5;i++){
    //     vcc.push_back(c(i));
    // }
    // c a=map[0];

    // for(auto &i: vcc){
    //     cout<<i.k<<endl;
    // }
    auto z= CreateFrame<int>(10, 16, 65);
    // const char* f = z.c_str();
    char * frame = &z[0];
    auto CR = CRC(frame);
    cout<<CR<<endl;
    CR[100] = '1';
    string s = string(CR);
    int res = CRCCheck(CR);
   
    cout<<res<<endl;
    // cout<<bitset<46*8>(z.substr(56+8+48*2, 46*8)).to_ulong()<<endl;
    // cout<<z<<endl;
    // const char * a = z.c_str();
    // cout<<string(z)<<endl;
    // cout<<f()<<endl;
    return 0;
}