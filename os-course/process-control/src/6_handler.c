#define _GNU_SOURCE
#include "return_codes.h"

#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static uint64_t acc = 1;
static volatile sig_atomic_t is_sigtrm = 0;
static volatile sig_atomic_t usr1_sig = 0;
static volatile sig_atomic_t usr2_sig = 0;
static void usr1_handler(int signl)
{
	usr1_sig = 1;
}
static void usr2_handler(int signl)
{
	usr2_sig = 1;
}
static void sigterm_handler(int signl)
{
	is_sigtrm = 1;
}
int main(void)
{
	struct sigaction sigaction_usr1 = { 0 }, sigaction_usr2 = { 0 }, sigaction_sigterm = { 0 };
	sigaction_usr1.sa_handler = usr1_handler;
	sigemptyset(&sigaction_usr1.sa_mask);
	sigaction_usr1.sa_flags = 0;
	if (sigaction(SIGUSR1, &sigaction_usr1, NULL) == -1)
	{
		fprintf(stderr, "[Handler Error]: Could not set SIGUSR1\n");
		return SIGNAL_ISSUE;
	}
	sigaction_usr2.sa_handler = usr2_handler;
	sigemptyset(&sigaction_usr2.sa_mask);
	sigaction_usr2.sa_flags = 0;
	if (sigaction(SIGUSR2, &sigaction_usr2, NULL) == -1)
	{
		fprintf(stderr, "[Handler Error]: Could not set SIGUSR2\n");
		return SIGNAL_ISSUE;
	}
	sigaction_sigterm.sa_handler = sigterm_handler;
	sigemptyset(&sigaction_sigterm.sa_mask);
	sigaction_sigterm.sa_flags = 0;
	if (sigaction(SIGTERM, &sigaction_sigterm, NULL) == -1)
	{
		fprintf(stderr, "[Handler Error]: Could not set SIGTERM\n");
		return SIGNAL_ISSUE;
	}
	while (!is_sigtrm)
	{
		if (usr1_sig)
		{
			acc += 2;
			usr1_sig = 0;
		}
		else if (usr2_sig)
		{
			acc *= 2;
			usr2_sig = 0;
		}
		else
			continue;
		printf("[Handler]: Acc value is %" PRIu64 "\n", acc);
		fflush(stdout);
	}
	return SUCCESS;
}
