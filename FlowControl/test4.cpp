//
// Created by bisakh on 27/11/20.
//

#include<bits/stdc++.h>

#include "crc.hpp"

using namespace  std;


class C{

public:
    int a;
    C(int k=0){
        a=k;
    }
};

int main(){
    string s ="0101";
    s = CRCGEN(s);
    cout<<s<<endl;
    cout<<isCRCValid(s)<<endl;
    //    vector<std::mutex> v(4);
//    for(auto &i: v){
//        cout<<<endl;
//    }
}