/** @brief Vector Processor (derived from Xilinx DebugDev)
 * 
 */


/* address space definition (offsets within target) */
#define MMR_TRACE 0xC4

class vector_processor
	: public sc_core::sc_module
{
public:
	tlm_utils::simple_target_socket<vector_processor> socket;
	sc_out<bool> irq;

	vector_processor(sc_core::sc_module_name name);
	virtual void b_transport(tlm::tlm_generic_payload &trans,
							 sc_time &delay);
	virtual unsigned int transport_dbg(tlm::tlm_generic_payload &trans);
};
