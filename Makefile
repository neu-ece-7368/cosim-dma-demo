
all: hw sw

hw:
ifneq ($(OECORE_SDK_VERSION),)
  $(info Will not build SystemC HW while SDK is activated)
else
	$(MAKE) -C $@
endif

sw:
ifeq ($(OECORE_SDK_VERSION),)
  $(info Will not build SW when SDK is not activated)
else
	$(MAKE) -C $@
endif

launch_cosim: hw
	$(MAKE) -C hw launch_cosim

.PHONY: hw sw
