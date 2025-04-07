#include "return_codes.h"
#include <sys/types.h>

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <HANDLERS_PID>\n", argv[0]);
		return PROGRAM_ARGS_ISSUE;
	}
	char* line = NULL;
	char* wordend;
	ssize_t bytesread = 0;
	size_t length = 0;
	pid_t handlers_pid = (pid_t)strtol(argv[1], &wordend, 10);
	if (*wordend != '\0' || handlers_pid <= 0)
	{
		fprintf(stderr, "[Generator Error]: Strange pid in argv\n");
		return PROGRAM_ARGS_ISSUE;
	}

	puts("[Generator]: Is up! Waiting for input\n");
	while (true)
	{
		printf(">>> ");
		fflush(stdout);
		bytesread = getline(&line, &length, stdin);
		if (bytesread == -1)
		{
			if (feof(stdin))
			{
				if (kill(handlers_pid, SIGTERM) == -1)
					fprintf(stderr, "[Generator Error]: Could not send signal SIGTERM\n");
				break;
			}
			else
			{
				fprintf(stderr, "[Generator]: stdin read error\n");
				break;
			}
		}
		if (!strcmp(line, "+\n"))
		{
			if (kill(handlers_pid, SIGUSR1) == -1)
			{
				fprintf(stderr, "[Generator Error]: Could not send signal USR1\n");
				continue;
			}
		}
		if (!strcmp(line, "*\n"))
		{
			if (kill(handlers_pid, SIGUSR2) == -1)
			{
				fprintf(stderr, "[Generator Error]: Could not send signal USR2\n");
				continue;
			}
		}
		if (!strcmp(line, "TERM\n"))
		{
			if (kill(handlers_pid, SIGTERM) == -1)
			{
				fprintf(stderr, "[Generator Error]: Could not send signal SIGTERM\n");
				continue;
			}
			break;
		}
		usleep(3000);
	}
	free(line);
	return SUCCESS;
}
