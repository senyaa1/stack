#define __USE_GNU

#define PRINT_DISASM

#include <signal.h>
#include <ucontext.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "debugger.h"
#include "colors.h"

static const size_t BACKTRACE_MAX_SZ = 100;
static const size_t DUMP_SZ = 15;
static const int subscribed_signals[] = { SIGSEGV, SIGILL, SIGFPE };
static bool is_setuped = false;

struct sigaction siga;


#pragma GCC diagnostic ignored "-Wunused-parameter"
static void signal_handler(int sig, siginfo_t *si, void* arg)
{
	ucontext_t *context = (ucontext_t *)arg;
	void* rip = (void*)context->uc_mcontext.gregs[REG_RIP];
	
	fprintf(stderr, RED "Caught exception! - %s\n" RESET, strsignal(sig));


	fprintf(stderr, "Crash happened at " GREEN "%p" RESET "\n\nCode: < " UNDERLINE  RED, rip);

	char code[DUMP_SZ * 3 + 2] = {0};
	char newByte[4] = {0};

	for(size_t i = 0; i < DUMP_SZ; i++)
	{
		sprintf(newByte, "%02hhx ", *(char*)((size_t)rip + i));
		strcat(code, newByte);
	}

	fprintf(stderr, "%s", code);
	fprintf(stderr, RESET "... >\n\n");


#ifdef PRINT_DISASM

	char systemcmd[DUMP_SZ * 3 + 2 + 256] = { 0 };
	sprintf(systemcmd, "echo \"%s\" | xxd -r -p | ndisasm -b 64 - 1>&2", code);
	system(systemcmd);

	fprintf(stderr, "\n");
#endif

	void *buffer[BACKTRACE_MAX_SZ] = { 0 };
	int nptrs = backtrace(buffer, BACKTRACE_MAX_SZ);
	char** strings = backtrace_symbols(buffer, nptrs);

	if (strings == NULL) {
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "Backtrace (%d):\n", nptrs);
	fprintf(stderr, "-------------------------------------------------------\n");
	for (int j = 0; j < nptrs; j++)
		fprintf(stderr, BLUE "|-%s\n" RESET, strings[j]);
	fprintf(stderr, "\n\n");
        free(strings); 

	abort();
}

bool check_ptr(void* ptr, const char* perms)
{
	FILE* fptr = fopen("/proc/self/maps", "r");
	if(!fptr)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	void* from = 0;
	void* to = 0;
	char perm[8] = { 0 };
	bool res = false;

	while(fscanf(fptr, "%p", &from) == 1)
	{
		fgetc(fptr);
		fscanf(fptr, "%p", &to);


		if(ptr >= from && ptr <= to)
		{
			res = true;

			fgetc(fptr);
			fscanf(fptr, "%4s", perm);

			for(size_t i = 0; i < strlen(perms); i++)
				res &= (bool)strchr(perm, perms[i]);

			break;
		}
		
		while(fgetc(fptr) != '\n')
			;
	}
	
	fclose(fptr);
	return res;
}

void setup_signals()
{
	if(is_setuped)
		return;

	siga.sa_sigaction = &signal_handler;
	siga.sa_flags = SA_SIGINFO;

	for(size_t i = 0; i < sizeof(subscribed_signals) / sizeof(int); i++)
	{
		if(sigaction(subscribed_signals[i], &siga, NULL))
		{
			perror("sigaction");
			exit(EXIT_FAILURE);
		}
	}
}
