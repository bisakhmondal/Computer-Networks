#include<bits/stdc++.h>
#include "semaphore.hpp"
using namespace std;

class C{
    public:
    int a=10;
};

const C c;
#include<unistd.h>

#include<thread>
#include<fcntl.h>

int k=5;
void process(){
    int fd = open("fifos/channel2receiver", 0664);
    char c;
    cout<<"reading"<<endl;
    if(read(fd, &c, sizeof(c))>0){
        cout<<c<<endl;
    }
    k=7;
    cout<<"reading done"<<endl;
}
using namespace fc;
int main(){
    // BinSemaphore sem("/sc");
    // int id;
    // if((id=fork())==0){
    //      BinSemaphore sem("/sc", 0, "open");
    //     cout<<sem.semVal()<<endl;
    //     while(!sem.semWAIT()){
    //         cout<<"inside"<<endl;
    //     }
    //     return 0;
    // }else{
    //     sleep(6);
    //     cout<<"resume"<<endl;
    //     sem.semPOST();

    // }
    cout<<k<<endl;
    std::thread t(process);

    cout<<"at main"<<endl;
    sleep(2);
    cout<<"main done"<<endl;


    t.join();
    cout<<k<<endl;
    // cout<<std::filesystem::exists("helloworld.txt")<<endl;
    return 0;
}