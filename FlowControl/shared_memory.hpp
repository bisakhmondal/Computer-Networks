#ifndef FLOW_CONTROL_SHAREDMEM
#define FLOW_CONTROL_SHAREDMEM

#include "utils.hpp"

#include<sys/mman.h>
#include<unistd.h>
#include <sys/stat.h>
#include<sys/types.h>

namespace fc{

    class SharedMemory{
        int fd;
        const char* backingfile;
        int bytesize;

        public:
        void* memptr;
        void*  SharedMEMInit(){
            this->fd = shm_open(backingfile, O_RDWR | O_CREAT, 0644);

            if(fd<0){
                report_and_exit("shared memory initialization failed");
            }
            
            ftruncate(fd, bytesize);

             void* memptr = mmap(NULL,       /* let system pick where to put segment */
                            bytesize,   /* how many bytes */
                            PROT_READ | PROT_WRITE, /* access protections */
                            MAP_SHARED, /* mapping visible to other processes */
                            fd,         /* file descriptor */
                            0);         /* offset: start at 1st byte */

            if (memptr == NULL) report_and_exit("Can't access segment..."); 

            return memptr;
        }  

        void* SharedMEMOpen(){
            this->fd = shm_open(backingfile, O_RDWR, 0644);  /* empty to begin */
            if (fd < 0) report_and_exit("Can't get file descriptor...");

            /* get a pointer to memory */
            void* memptr = mmap(NULL,       /* let system pick where to put segment */
                                    bytesize,   /* how many bytes */
                                    PROT_READ | PROT_WRITE, /* access protections */
                                    MAP_SHARED, /* mapping visible to other processes */
                                    fd,         /* file descriptor */
                                    0);         /* offset: start at 1st byte */
                
            if (memptr == NULL) report_and_exit("Can't access segment..."); 

            return  memptr;
        }

        SharedMemory(std::string file, int Bytes, std::string mode="init"){
            backingfile= file.c_str();
            bytesize= Bytes;
            if(mode=="init")
                this->memptr = SharedMEMInit();
            else
                this->memptr = SharedMEMOpen();
        }

        ~SharedMemory(){
            munmap(this->memptr, this->bytesize);
            close(fd);
            unlink(backingfile);
        }
    };

}


#endif
