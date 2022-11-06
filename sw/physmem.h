/** Physical Memory abstraction*/
#ifndef PHYS_MEM_H
#define PHYS_MEM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>
using namespace std;

/// @brief physical memory access class
///        uses /dev/mem 
class PhysMemDrv {

public:
    /// @brief pointer in virtual memory that is pagemapped to physical memory 
    void *ptr; 

    /// @brief Create access to physical memory
    /// @param physAddr base address in physical memory
    /// @param len      length in bytes
    PhysMemDrv(off_t physAddrIn, size_t len) {

        int fd;       /// file descriptor to phys mem
        unsigned page_size=sysconf(_SC_PAGESIZE);  /// get page size 

        //open device file representing physical memory 
        fd=open("/dev/mem",O_RDWR);
        if(fd<1) {
            perror("failed");
            exit(-1);
        }

        // offset in /dev/mem needs to be page aligned
        off_t physAddrPageAligned = physAddr & ~(page_size-1);

        // offset to lower page boundry in phys (is 0 if physAddr is page aligned)
        off_t physAddrPageOffset = 0; 

        // if physAddress was not page aligned, then compute offset 
        // to base of page that contains the physAddress 
        if (physAddr != physAddrPageAligned) {
            physAddrPageOffset = physAddr - physAddrPageAligned; 
        }

        // if phys address was not page aligned we need to increase the length so 
        // that after adjusting the desired user lenght is still guaranteed. 
        len += physAddrPageOffset;

        // make len into page size multiple
        unsigned int lenPageMultiple = (len / page_size) * page_size;
        if ( (len  - lenPageMultiple) > 0) {
            lenPageMultiple += page_size;
        }

        /// get a pointer in process' virtual memory that points to the physcial address 
        ptr =  mmap(NULL,lenPageMultiple,PROT_READ|PROT_WRITE,MAP_SHARED,fd,physAddrPageAligned);
        if (ptr == NULL ) {
            perror("mmap faild"); 
            exit(-1);
        }

        // if physAddress was not page aligned, then virt ptr will have the same offset
        // adjust ptr to actually point to the user intended's physAddr.
        ptr = (void*)((char*)ptr + physAddrPageOffset);
    }


    /// @brief returns physical addess of virtual address paged to phyical 
    /// @param pIn   virtual address pointed to pys 
    /// @return      corresponding phys address
    void* physAddr(void *pIn) {
        // compute offset to the base (in virtual)
        unsigned int virtOffset = (off_t)  ((char*)pIn - (char*)ptr);
        // add the virtual computed offset to physAddress 
        //   don't need to deal with alignment conditions as ptr points 
        //   to actual physAddress (regardless of page alignment)
        return (void*)(physAddr + virtOffset);
    }

private: 
    // physical address to acces 
    off_t physAddr;
};

#endif