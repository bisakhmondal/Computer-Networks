#ifndef FLOW_CONTROL_THREADSAFEQUEUE
#define FLOW_CONTROL_THREADSAFEQUEUE

#include<queue>
#include<condition_variable>

namespace fc{

template<class T>
class SyncedQueue{
   
    std::queue<T> q;
    std::condition_variable c;
    mutable std::mutex m; //thread safe mutex.
   
    public:
        void push(T val){
            std::lock_guard<std::mutex> lock(m);
            q.push(val);
            c.notify_one();
        } 

        T pop(){
            std::unique_lock<std::mutex> lock(m);
            
            while(q.empty()){
                c.wait(m); //release the lock until it gets filled with a value by other workers.
            }
            T popped = q.front();
            q.pop();
            return popped;
        }
};

}

#endif
