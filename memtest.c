#include "memtest.h"

int determinePage(long unsigned int callee, int* offset){
	int curPageOffset = (callee - (long unsigned int)pagingInfo -> baseAddr);
	long unsigned int progOffset = curPageOffset + pagingInfo -> curPage * pagingInfo -> pageSize; 
	long unsigned int newOffset = progOffset + *offset;
	int newPage = newOffset / pagingInfo -> pageSize;
	*offset = newOffset % pagingInfo -> pageSize;
	return newPage;
}

void segHandler(int errCode, siginfo_t *sigInfo, ucontext_t* uContext){
	int offset = sigInfo -> si_call_addr - pagingInfo -> baseAddr;
	int page = determinePage(uContext -> uc_mcontext.gregs[REG_RIP], &offset);
	printf("Called from address: 0x%lld\n", uContext -> uc_mcontext.gregs[REG_RIP]);
	printf("Called to offset: 0x%ld\n", sigInfo -> si_call_addr - pagingInfo -> baseAddr);
	printf("Base addr: %p\n", pagingInfo -> baseAddr);
	printf("Page: %d\n", page);
	printf("Offset: %d\n", offset);

	char filename[20];
	FILE *f;
	sprintf(filename, "/tmp/%d.bin", page);
	f = fopen(filename, "rb");
	fread((char*)pagingInfo -> baseAddr, 6108, 1, f);	
	fclose(f);
	uContext -> uc_mcontext.gregs[REG_RIP] = (long long int)offset + (long long int)pagingInfo -> baseAddr;

	exit(-1);
}

char* readBytesFromFile(char* filename){
	FILE* f = fopen(filename, "rb");
	char* mem = (char*)malloc(25000);
	fread(mem, 25000, 1, f);
	fclose(f);
	return mem;
}

int getEntryPoint(char *bytes){
	for(int i = 0; i < 0x18; i++) bytes += 1;
	return *(int*)bytes;
}

int main(){
	pagingInfo = (MemoryManagement*)malloc(sizeof(MemoryManagement));
	struct sigaction *segact = (struct sigaction*)calloc(1, sizeof(sigaction));

	segact -> sa_sigaction = (void (*)(int, siginfo_t *, void*)) segHandler;
	segact -> sa_flags = SA_SIGINFO;

	sigaction(SIGSEGV, segact, NULL);
	pagingInfo -> baseAddr = malloc(6108);
	pagingInfo -> pageSize = 6108;
	pagingInfo -> progBaseAddr = (void*)0x400000;

	char *readInBytes = readBytesFromFile("payload");
	mprotect(pagingInfo -> baseAddr, 6018, PROT_EXEC|PROT_READ|PROT_WRITE);
	char filename[2048];
	FILE* f;
	for(int i = 0; i < 4; i++){
		sprintf(filename, "/tmp/%d.bin", i);
		f = fopen(filename, "wb");
		fwrite(readInBytes+(6108*i), 6108, 1, f);
		fclose(f);
	}
	memcpy(pagingInfo -> baseAddr, readInBytes, pagingInfo -> pageSize);
	pagingInfo -> curPage = 0;

	int entrypoint = getEntryPoint(readInBytes) - 0x400000;
	int page = determinePage((long unsigned int) pagingInfo -> baseAddr, &entrypoint);
	if(0 == page){
		ucontext_t* tmp = (ucontext_t*)malloc(sizeof(ucontext_t));
		getcontext(tmp);
		tmp -> uc_mcontext.gregs[REG_RIP] = (long long int)pagingInfo -> baseAddr + entrypoint;
		setcontext(tmp);
	}
}

