#include "swEmu.h"

/// @brief Interrupt Service Routine for debug
void SWEmu::isr1() {
    // there is a startup intertrupt for no good reason, 
    // ignore it.
    if (sc_time_stamp() < sc_time(1,SC_PS)){
        return;
    }
    cout << "ISR1 called at " << sc_time_stamp() << endl;
}


/// @brief Interrupt Service Routine for debug
void SWEmu::isr2() {
    // there is a startup intertrupt for no good reason, 
    // ignore it.
    if (sc_time_stamp() < sc_time(1,SC_PS)){
        return;
    }
    cout << "ISR2  called at " << sc_time_stamp() << endl;
}


void SWEmu::thread_process()
{

    // set base address of device to communicate to
    dev_addr_set(0x48000000ULL); 
    // simulate some startup  delay
    usleep(10);
    word_write(0x0, 1);

    usleep(10);
    
    unsigned int readVal = word_read(0x0);
    cout << "READ res [0x00]: " << dec << readVal << endl; 
    
    usleep(10);
    
    readVal = word_read(0x10);
    cout << "READ res [0x10]: " << dec << readVal << endl; 

    // stop simulation and exit, done with the testbench
    retVal = 0; // all good (set process exit value)
    sc_stop();

}
