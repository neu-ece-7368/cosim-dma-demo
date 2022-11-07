## Overview 

This repo demonstrates the use of Direct Memory Access (DMA) as a demo derived from 
the demo (courtesy of Xilinx's [Xilinx CoSimulation Demo](https://github.com/Xilinx/systemctlm-cosim-demo)). 

## Interrupts in user mode

To use interrupts in user mode, we need to configure the device using [UIO](https://www.kernel.org/doc/html/latest/driver-api/uio-howto.html).

We will use a device tree overlay to configure UIO after boot. The source of the device tree fragments for this ddemo is located at [hw/demo.dts](hw/demo.dts). 
It defines the interrupts for the demodma as `interrupts = <0 30 4>;` meaning the first (and only) interrupt group, line 30 at rising edge. 
Compile the device tree overlay blob by running make in the hw folder (or make demo.dtbo).

To enable the device tree overlay:

1. Copy the demo.dtbo file into QEMU/ZedBoard
2. Enable the overlay with: ```mkdir -p /sys/kernel/config/device-tree/overlays/demo``` and ```cat demo.dtbo > /sys/kernel/config/device-tree/overlays/demo/dtbo```. 
3. If the generic platform driver uio module is built into the kernel, see step 4. Otherwise, see step 5.
4. Builtin module: append "uio_pdrv_genirq.of_id=generic-uio" to the kernel command line.
5. Load the generic driver with ```modprobe uio_pdrv_genirq of_id=generic-uio```.

To change the kernel command line when using the ```qemu-esl``` QEMU startup script, simply edit line 120:

```
-append "root=/dev/mmcblk0p2 panic=1 rootwait rootfstype=ext4 rw <ADD HERE FROM 4.>" ${cosim_args}
```

## UIO overview and using interrupts

The generic UIO driver will create a ```/dev/uioX``` file for each node that has the ```compatible=generic-uio``` line in the device tree overlay file. This files allow accessing the register memory region and interrupt as described in each node of the overlay.

### Blocking wait for interrupt with UIO

The example below will block until the interrupt for the debug device is triggered. In the next subsection there is an example that triggers that interrupt.

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
int main(void) {
  int fd = 0;
  unsigned int ret = 0;
  ssize_t _read = 0;
  fd = open("/dev/uio0", O_RDWR);
  if (fd < 0) {
    printf("error\n");
    return 1;
  }
  _read = read(fd, (void*)&ret, 4);
  printf("read %d bytes, value is 0x%x\n", _read, ret);
  return 0;
}
```

Note the `_read = read(fd, (void*)&ret, 4);` call. The read from the character device blocks until the next interrupt has occured. 
Only reads of size 4 are supported (other sizes fail). 

### Using mmap on UIO and triggering interrupt in debug device
See below for example code that triggers the interrupt for testing in the debug device

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
int main(void) {
  int fd = 0;
  volatile unsigned int* dev;
  ssize_t _read = 0;

  fd = open("/dev/uio0", O_RDWR);

  if (fd < 0) {
    printf("error\n");
    return 1;
  }
  dev = (unsigned int*)mmap(NULL, 0x100, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if (!dev) {
    printf("error in mmap\n");
    return 1;
  }
  // trigger interrupt
  dev[3] = 1;
  return 0;
}
```

### Zynq PS-PL interrupts

Zynq interrupts from PL have the following numbers in linux (the first interrupt is #0 in SystemC):
29, 30, 31, 32, 33, 34, 35, 36, 52, 53, 54, 55, 56, 57, 58, 59.
