
all: hw sw

hw:
ifneq ($(OECORE_SDK_VERSION),)
	$(info Will not build SystemC HW while SDK is activated)
else
	$(MAKE) -C $@
endif

# if SDK is not set source it automatically
#  - needs bash to pretend to be a login shell to get the alias
sw:
ifeq ($(OECORE_SDK_VERSION),)
	/bin/bash -li -c "setup-xarm; $(MAKE) -C $@"
else
	$(MAKE) -C $@
endif

# common targets that can be called on each directory 
clean: 
	$(MAKE) -C hw $@
ifeq ($(OECORE_SDK_VERSION),)
	/bin/bash -li -c "setup-xarm; $(MAKE) -C sw $@"
else
	$(MAKE) -C sw $@
endif

launch_cosim: hw
	$(MAKE) -C hw launch_cosim

.PHONY: hw sw
