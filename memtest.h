#include <sys/mman.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>

#define REG_RIP 16

typedef struct{
	void *baseAddr;
	void *progBaseAddr;
	int pageSize;
	int curPage;
} MemoryManagement;

MemoryManagement *pagingInfo;

void segHandler(int errCode, siginfo_t* siginfo, ucontext_t* uContext);
