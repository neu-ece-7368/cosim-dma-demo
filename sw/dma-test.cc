#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
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
	int fd;       /// file descriptor to phys mem
	void *pDev;   /// pointer to OCM memory
	unsigned page_size=sysconf(_SC_PAGESIZE);  /// get page size 

	//open device file representing physical memory 
	fd=open("/dev/mem",O_RDWR);
	if(fd<1) {
		perror(argv[0]);
		exit(-1);
	}

	/// get a pointer in process' virtual memory that points to the physcial address of OCM
	pDev=mmap(NULL,OCM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,(OCM_ADDR & ~(page_size-1)));
	
	/// instanciate driver 
	DemoDmaDrv dma;

	cout << "copy data in" << endl;

	for(int i = 0; i < N_WORDS; i++) {
		((volatile uint32_t*)pDev)[i] = i; // write in
	}

	cout << "call DMA to copy" << endl;
	void *pSrc   = (void*) OCM_ADDR;
	void *pDest  = (void*) (OCM_ADDR+N_WORDS+sizeof(uint32_t));
	if (dma.memcpy_start(pDest,pSrc,N_WORDS+sizeof(uint32_t))) {
		cout << "DMA not idle to start with" << endl; 
		return 1;
	}

	dma.waitIdle(); 

	cout << "compare results" << endl;
	for(int i = 0; i < N_WORDS; i++) {
		uint32_t retVal = ((volatile uint32_t*)pDev)[i + N_WORDS];
		if (i != retVal) {
			cout << "Fail at << " << i << "expect: " << i << " got: " << retVal << endl;
			return 1;
		}
	}

	cout << "all passed" << endl;
		
	return 0; 
}
