#ifndef DEMO_DMA_H
#define DEMO_DMA_H

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

class DemoDmaDrv {

public:

    enum {
        DEMODMA_CTRL_RUN  = 1 << 0,
        DEMODMA_CTRL_DONE = 1 << 1,
    };

    enum {
        DEMODMA_RESP_OKAY               = 0,
        DEMODMA_RESP_BUS_GENERIC_ERROR  = 1,
        DEMODMA_RESP_ADDR_DECODE_ERROR  = 2,
    };

    /* registers */
    typedef struct {
			uint32_t dst_addr;
			uint32_t src_addr;
			uint32_t len;
			uint32_t ctrl;
			uint32_t byte_en;
			uint32_t error_resp;
	} tRegs;

    // base address of device
    const unsigned int SYSTEMC_DEVICE_ADDR = 0x48001000ULL;
    // polling interval in us
    const unsigned int POLLING_INTERVAL = 500; 

    // constructor opens connection to MMRs
    DemoDmaDrv():dev(SYSTEMC_DEVICE_ADDR, 0x100) {

        // get phys pointer from phys mem class        
        pRegs= (tRegs *) dev.ptr;
    }

    /// @brief returns true if DMA is idle
    /// @return 
    bool isIdle() {
        unsigned int regVal; 
        regVal = pRegs->ctrl;
        // if run bit is not set, then the DMA is not running
        // done flag is actually for interrupts 
        return !(regVal & DEMODMA_CTRL_RUN);
    }

    /// @brief  blocks until DMA is idle 
    void waitIdle(){
        unsigned int timeout = 1000;
        while(1){
            if(isIdle()) {
                return;
            }
            timeout--;
            if(timeout <= 1) {
                cout << "DMA timeout, never turned idle" << endl;
                exit(1);
            }
            usleep(POLLING_INTERVAL);
        }
    }

    /// @brief Start DMA-based memcopy. It copies n bytes from memory area 
    ///        src to memory area dest.  The memory areas must not overlap. 
    /// @param dest pointer to where to write to
    /// @param src  pointer to where to read from 
    /// @param n    number of bytes to copy 
    /// @return  false on success, true on error (DMA not idle)
    bool memcpy_start(void  *dest, const void * src, size_t n) {

        // check that dma is idle
        if(!isIdle()) {
            return true;
        }

        //Validate word aligned access from incoming arguments
        // dest, src need to be word aligned, n multi of 4
        if (   (u_int32_t) dest % sizeof(u_int32_t) == 0 
            && (u_int32_t) src % sizeof(u_int32_t) == 0 
            && n % sizeof(u_int32_t) == 0 ) {
            
            pRegs->byte_en  = 0; // all word aligned access 
        } else {
            // WARNING UNTESTED
            pRegs->byte_en  = 1; // not word aligned access 
        }

        // write MMRs 
        pRegs->dst_addr = (u_int32_t) dest;
        pRegs->src_addr = (u_int32_t) src;
        pRegs->len      = n; 
        
        // kick off ACC set run flag
        pRegs->ctrl |= DEMODMA_CTRL_RUN;

        return false; 
    }

private:
    PhysMemDrv dev;
	tRegs *pRegs;   /// pointer to base address of device (mapped into user's virtual mem)
};

#endif