/*
 * Top level of testbench for SW Emulation (Host Compiled Simulation)
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "systemc.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/tlm_quantumkeeper.h"

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "trace.h"
#include "iconnect.h"
#include "debugdev.h"
#include "swEmu.h"
#include "demo-dma.h"

// how many initiators (masters) are on the bus iconnect
#define NR_MASTERS 1
// how many targets (slaves) are on the bus iconnect
#define NR_DEVICES 2

SC_MODULE(Top)
{
	iconnect<NR_MASTERS, NR_DEVICES> bus;
	debugdev debug;
	SWEmu swEmu;
	demodma dma;
	sc_signal<bool> irqDbg, irqVecProc;


	Top(sc_module_name name, const char *sk_descr, sc_time quantum) : bus("bus"),
																	  debug("debug"),
																	  swEmu("swEmu"),
																	  dma("dma")
	{
		m_qk.set_global_quantum(quantum);

		// map debug target (slave) device with base address 
		bus.memmap(0x48000000ULL, 0x100 - 1,
				   ADDRMODE_RELATIVE, -1, debug.socket);

		// map DMA target (slave) device with base address 
		bus.memmap(0x48001000ULL, 0x100 - 1,
					ADDRMODE_RELATIVE, -1, dma.tgt_socket);

		// connect the swEmu as a target to the bus port 
		swEmu.socket.bind(*(bus.t_sk[0]));

		// connect interrupt wires 
		debug.irq(irqDbg);
		swEmu.irq1(irqDbg);

		dma.irq(irqVecProc);
		swEmu.irq2(irqVecProc);
	}

private:
	tlm_utils::tlm_quantumkeeper m_qk;
};

void usage(void)
{
	cout << "sync-quantum-ns" << endl;
}

int sc_main(int argc, char *argv[])
{
	Top *top;
	uint64_t sync_quantum;
	sc_trace_file *trace_fp = NULL;

	// parse time quantum in ns if given as argument
	if (argc < 2) 	{
		sync_quantum = 100000;
	} else 	{
		sync_quantum = strtoull(argv[1], NULL, 10);
	}

	// set the meaning of sc_time tracking
	sc_set_time_resolution(1, SC_PS);

	// allocate top 
	top = new Top("top", argv[1], sc_time((double)sync_quantum, SC_NS));

	// create trace
	trace_fp = sc_create_vcd_trace_file("trace");
	trace(trace_fp, *top, top->name());

	// start SystemC. Will not run forever, swEmu will stop simulation
	sc_start();
	if (trace_fp)
	{
		sc_close_vcd_trace_file(trace_fp);
	}
	return top->swEmu.retVal_get();
}
