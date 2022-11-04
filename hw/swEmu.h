#ifndef MYMASTER_H
#define MYMASTER_H


#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace sc_core;
using namespace std;


///@brief Software Emulation
///       Host compiled simulation of SW, much faster than QEMU to test out
///       basic interactions 
SC_MODULE(SWEmu)
{
    tlm_utils::simple_initiator_socket<SWEmu> socket; /// master interface to access bus
    sc_in<bool> irq1, irq2;
 
    SC_CTOR(SWEmu) : socket("socket")
    {

        SC_THREAD(thread_process);

        // emulate interrupt service routine being called when 
        // the irq rises
        SC_METHOD(isr1);
        sensitive << irq1.pos();

        SC_METHOD(isr2);
        sensitive << irq2.pos();

        // reset timeOffset within quantum (all processes use one global quantum)
        m_qk.reset();

        // allocate payload we will use it for all transactions 
        // there is only one transaction at any given time
        trans = new tlm::tlm_generic_payload;
    }

    unsigned int retVal_get() { return retVal;}

private: 
    /// @brief Interrupt Service Routine, called on rising IRQ
    void isr1();
    /// @brief Interrupt Service Routine, called on rising IRQ
    void isr2();

    /// @brief user process (just one)
    void thread_process();

    tlm::tlm_generic_payload *trans;  /// payload for transactions 
    tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling
    sc_time timeOffset; /// local time offset to systemc time 
    int data;       // Internal data buffer used by initiator with generic payload
    unsigned long long baseAddr; // device base address
    unsigned int retVal = 0;  // return value for simulation 


    /// @brief set the base address for subsequent write read operations
    void dev_addr_set(unsigned long long baseAddrIn) {
        baseAddr = baseAddrIn;
    } 

    /// @brief write a word onto socket using fault transaction 
    /// @param offset target address offset in bytes
    /// @param value to write
    void word_write(unsigned int offset, unsigned int value) {
            timeOffset = m_qk.get_local_time();

            unsigned long long addr = baseAddr + (unsigned long long) offset;
            data = value; //GS: not sure why we cant just use value directly
            // No DMI, so use blocking transport interface
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            trans->set_address(addr);
            trans->set_data_ptr(reinterpret_cast<unsigned char *>(&data));
            trans->set_data_length(4);
            trans->set_streaming_width(4);                            // = data_length to indicate no streaming
            trans->set_byte_enable_ptr(0);                            // 0 indicates unused
            trans->set_dmi_allowed(false);                            // Mandatory initial value
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

            socket->b_transport(*trans, timeOffset);

            //cout << "WRITE     addr = " << hex << addr << ", data = " << value
            //     << " at " << sc_time_stamp() << " delay = " << timeOffset << "\n";

            // Initiator obliged to check response status
            if (trans->is_response_error())
                SC_REPORT_ERROR("TLM-2", trans->get_response_string().c_str());

            m_qk.set(timeOffset); // update the time with what came back from target
            if (m_qk.need_sync())
                m_qk.sync();
    }

    /// @brief read word from socket at addr
    /// @param addr for target
    /// @return value read
    unsigned int word_read(unsigned int offset) {
   
      unsigned long long addr = baseAddr + (unsigned long long) offset;
      trans->set_command( tlm::TLM_READ_COMMAND );
      trans->set_address(addr);
      trans->set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
      trans->set_data_length( 4 );
      trans->set_streaming_width( 4 ); // = data_length to indicate no streaming
      trans->set_byte_enable_ptr( 0 ); // 0 indicates unused
      trans->set_dmi_allowed( false ); // Mandatory initial value
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE ); // Mandatory initial value

      timeOffset = m_qk.get_local_time();

      socket->b_transport( *trans, timeOffset ); // blocking call to perform transactoin 

      // Initiator obliged to check response status
      if (trans->is_response_error())
        SC_REPORT_ERROR("TLM-2", trans->get_response_string().c_str());

      // cout << "READ     addr = " << hex << addr << ", data = " << dec << data
      //     << " at " << sc_time_stamp() << " delay = " << timeOffset << "\n";

      m_qk.set( timeOffset); // update local time offset with what came back from target 
      if (m_qk.need_sync()) m_qk.sync();

      return data;
    }

    /// @brief increase local time and sync if needed 
    /// @param incTime in micro seconds
    void usleep(unsigned int usec) {
        m_qk.inc(sc_time(usec, SC_US)); // delay for usec
        if (m_qk.need_sync())           // sync quantum if needed
            m_qk.sync();
    }

};




#endif // MYMASTER_H

