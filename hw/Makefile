
ifneq ($(OECORE_SDK_VERSION),)
  $(error Will not build SystemC HW while SDK is activated)
endif

ifneq ($(SYSTEMC),)
# If SystemC is manually built, must match C++std
	SYSTEMC_LIBS ?= lib64
	SYSTEMC_CSTD ?= 14
	CFLAGS += -I$(SYSTEMC)/include
	CXXFLAGS += $(CFLAGS) -std=c++$(SYSTEMC_CSTD)
	LDFLAGS += -L$(SYSTEMC)/$(SYSTEMC_LIBS) -lsystemc
else
	LDFLAGS += $(shell pkg-config --libs systemc)
endif

COSIM_PATH ?= $(HOME)/.cosim
COSIM_QUANTUM ?= 100000

CFLAGS += -Wall -O2 -g
CXXFLAGS += -Wall -O2 -g

CPPFLAGS += -I .
LDLIBS   += -pthread

ZYNQ_TOP_C = zynq_demo.cc
ZYNQ_TOP_O = $(ZYNQ_TOP_C:.cc=.o)
ZYNQ_OBJS += $(ZYNQ_TOP_O)

SC_OBJS += trace.o
SC_OBJS += debugdev.o demo-dma.o

TB_OBJS := $(SC_OBJS) testbench.o swEmu.o

LIBSOC_PATH=../libsystemctlm-soc
CPPFLAGS += -I $(LIBSOC_PATH)

LIBSOC_ZYNQ_PATH=$(LIBSOC_PATH)/soc/xilinx/zynq
SC_OBJS += $(LIBSOC_ZYNQ_PATH)/xilinx-zynq.o
CPPFLAGS += -I $(LIBSOC_ZYNQ_PATH)

CPPFLAGS += -I $(LIBSOC_PATH)/tests/test-modules/
SC_OBJS += $(LIBSOC_PATH)/tests/test-modules/memory.o

LIBRP_PATH=$(LIBSOC_PATH)/libremote-port
C_OBJS += $(LIBRP_PATH)/safeio.o
C_OBJS += $(LIBRP_PATH)/remote-port-proto.o
C_OBJS += $(LIBRP_PATH)/remote-port-sk.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm-memory-master.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm-memory-slave.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm-wires.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm-ats.o
SC_OBJS += $(LIBRP_PATH)/remote-port-tlm-pci-ep.o
SC_OBJS += $(LIBSOC_PATH)/soc/pci/core/pci-device-base.o
SC_OBJS += $(LIBSOC_PATH)/soc/dma/xilinx/mcdma/mcdma.o
SC_OBJS += $(LIBSOC_PATH)/soc/net/ethernet/xilinx/mrmac/mrmac.o
CPPFLAGS += -I $(LIBRP_PATH)

OBJS = $(C_OBJS) $(SC_OBJS)

ZYNQ_OBJS += $(OBJS)

TARGET_ZYNQ_DEMO = zynq_demo
TARGET_TB        = testbench
TARGETS = $(TARGET_ZYNQ_DEMO) $(TARGET_TB)

all: $(TARGETS)

-include $(ZYNQ_OBJS:.o=.d) $(TB_OBJS:.o=.d)
CFLAGS += -MMD
CXXFLAGS += -MMD

$(TARGET_ZYNQ_DEMO): $(ZYNQ_OBJS) $(VTOP_LIB) $(VERILATED_O) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(TARGET_TB): $(TARGET_TB).o $(TB_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

launch_cosim: $(TARGET_ZYNQ_DEMO)
	./$(TARGET_ZYNQ_DEMO) unix:$(COSIM_PATH)/qemu-rport-_cosim@0 $(COSIM_QUANTUM)

launch_cosim_debug: $(TARGET_ZYNQ_DEMO)
	gdb --args ./$(TARGET_ZYNQ_DEMO) unix:$(COSIM_PATH)/qemu-rport-_cosim@0 $(COSIM_QUANTUM)

clean:
	$(RM) $(OBJS) $(OBJS:.o=.d) $(TARGETS)
	$(RM) $(TARGET_TB).o $(TB_OBJS) $(TB_OBJS:.o=.d) 
	$(RM) $(ZYNQ_OBJS) $(ZYNQ_OBJS:.o=.d) 
	$(RM) trace.vcd
