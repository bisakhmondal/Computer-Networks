#ifndef FLOW_CONTROL_SENDER_GOBACKN
#define FLOW_CONTROL_SENDER_GOBACKN

#include "base.hpp"

namespace fc{

    namespace Sender{

        class GOBACKN:public Base{
            // float sendingTime, receivingTime;
             std::atomic<bool> isTimeout, timerStarted;

            std::atomic<int> Sn, Sf, Sw;
            std::unordered_map<int, bytestream_t> cache;

//            std::queue<bytestream_t> tempBuffer;

        public:

            GOBACKN(pid_t mac, std::string fifo, std::string fifoW): Base( mac,fifo, fifoW){

                DEBUG("mac");
                DEBUG(srcMac);
                DEBUG(destMac)
                Sn=0;
                Sf=0;
                Sw= sizeM-1;
                isTimeout = false;
                timerStarted = false;
            }


            //Timeout function.
            inline bool Timeout(){
                timerStarted=true;
                sleep(10);
                isTimeout = true;
                timerStarted=false;
                return 1;
            }

            //running on a separate thread.
            void Recv_ACK() {
                while (Sf<=Sn) {
                    descriptor_t fd = open((char *) fifoPipe.c_str(), O_RDONLY);
//                std::cout<<std::endl<<"fifo open"<<std::endl;

                    char ack_frame[ACKSIZE + 1];
                    int status;
                    {
                        //free the pipe by consuming the whole.
                        while ((status = read(fd, ack_frame, fc::ACKSIZE + 1)) > 0) {
                            //noisy frame dropping it.
                            if (!frame::checkFrameIntegrity(bytestream_t(ack_frame))) {
                                std::cout << "Noisy acknowledgement received...Discarding..." << std::endl;
                                continue;
                            }
                            int ackNo = frame::getSEQNO(bytestream_t(ack_frame));

                            if (ackNo > Sf && ackNo <= Sn) {
                                while (Sf <= ackNo) {
//                                    cache.erase(Sf);
                                    Sf++;
                                }
                            }
                            std::cout << "Acknowledge received with seq: " << frame::getSEQNO(bytestream_t(ack_frame))
                                      << std::endl;


//                    tempBuffer.push(bytestream_t(ack_frame));
                        }
//                        if (status == -1)
//                            std::cerr << std::this_thread::get_id() << " :fifo read error on -> " << fifoPipe
//                                      << std::endl;
                    }

                    close(fd);

                }
            }

//            void GetData(){
//                Sn=sizeM-1;
//            }
            void send(vector<int> &dataArray) {
//                int offset = 0;
                bool retransmit =false;
                std::thread tt([this] { this->Recv_ACK(); });
                tt.detach();

                while (Sf < dataArray.size() ) {
                    //                DEBUG(fifoPipe)
                    while( Sn < dataArray.size()) {

                        //wait for the acknow.
                        while(Sn-Sf>=Sw && !isTimeout){
                            sleep(TIMEOUT);
                        }

                        //need retransmission
                        if(Sn-Sf >=Sw && isTimeout){
                            retransmit = true;
                            break;
                        }


                        int d = dataArray[Sn];

                        bytestream_t data = frame::CreateFrame<int>(srcMac, destMac, d, Sn);
                        cache[Sn] = data; //store frame
                        char frame[fc::FrameSize + 1];
                        strcpy(frame, (char *) data.c_str());


                        int fd = open(fifoPipeW.c_str(), O_WRONLY);
                        if (fd == -1) {
                            std::cerr << "pipe open error\n";
                        }
//writing to fifo
                        {
//                        std::lock_guard<std::mutex> lk(lock);

                            if (write(fd, frame, FrameSize + 1) < 0) {
                                std::cerr << "fifo write failed from sender node" << std::endl;
                            } else {
                                DEBUG("Packet sent | data-> " + std::to_string(d));
                            }

                        }
                        close(fd);

                        Sn++;

                        if (!timerStarted) {
                            isTimeout=false;
                            std::thread tmut([this] { this->Timeout(); });
                            tmut.detach();
                        }
                        //1 sec sleep for better result
                        sleep(1);
                    }
                    if( Sn >= dataArray.size() && isTimeout){
                        retransmit=true;
                    }
                    //retransmission
                    if(retransmit){
                        isTimeout=false;
                        std::thread tmut([this] { this->Timeout(); });
                        tmut.detach();
                        int temp = Sf;
                        DEBUG("Retransmitting Frames")

                        while(temp<Sn){
                            DEBUG(std::to_string(Sf)+ " "+ std::to_string(Sn)+ " "+ std::to_string(temp))
                            bytestream_t data = cache[temp];
                            char frame[fc::FrameSize + 1];
                            strcpy(frame, (char *) data.c_str());

                            int fd = open(fifoPipeW.c_str(), O_WRONLY);
                            if (fd == -1) {
                                std::cerr << "pipe open error\n";
                            }
//writing to fifo
                            {
//                        std::lock_guard<std::mutex> lk(lock);

                                if (write(fd, frame, FrameSize + 1) < 0) {
                                    std::cerr << "fifo write failed from sender node" << std::endl;
                                } else {
                                    DEBUG("Packet sent | data-> " + std::to_string(frame::getData(data)));
                                }

                            }
                            close(fd);
                            temp++;
                        }
                        retransmit =false;
                    }


                }
            }

            void run(){
                DEBUG("Transmission start")
                // clock_t start = clock();
                // vector<int> dataArray ={12, 45, 13, 56, 76, 67, 43, 23, 15};
                send(dataArray);
                // start = clock()-start;
                DEBUG("data transmission complete")
                // DEBUG("Time taken"+ std::to_string((float)start/CLOCKS_PER_SEC))
            }

        };
    }
}

#endif
