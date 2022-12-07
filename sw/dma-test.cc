#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "physmem.h"
#include "demo-dma.h"
#include <iostream>
using namespace std;

/// base address of OCM memory
#define OCM_ADDR (0xFFFC0000ULL)
// OCM memory size
#define OCM_SIZE 256*1024 

// number of words to write
#define N_WORDS 256

int main(int argc, char *argv[])
{
	// access to OCM via physical memory 
	PhysMemDrv ocm(OCM_ADDR,OCM_SIZE);
	// get pointer in data type we like
	volatile uint32_t *data = (volatile uint32_t *) ocm.ptr;
	// instanciate driver 
	DemoDmaDrv dma;

	// set everything to 0
	memset((void*)data, 0, OCM_SIZE);

	cout << "Copy data in" << endl;
	for(int i = 0; i < N_WORDS; i++) {
		data[i] = i; // write in
	}

	cout << "Start DMA copy" << endl;
	if (dma.memcpy_start(ocm.phys((void*) &data[N_WORDS]),
					     ocm.phys((void*) &data[0]),
						 N_WORDS*sizeof(uint32_t))) {
		cout << "Failed to start DMA" << endl; 
		return 1;
	} 

	cout << "Wait DMA done" << endl;
	dma.waitIdle(); 

	cout << "Compare results" << endl;
	for(int i = 0; i < N_WORDS; i++) {
		uint32_t retVal = data[i + N_WORDS];
		if (i != retVal) {
			cout << "Fail at << " << i << "expect: " << i << " got: " << retVal << endl;
			return 1;
		}
	}

	cout << "Test Passed" << endl;
	return 0; 
}
