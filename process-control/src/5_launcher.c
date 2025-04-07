#include "return_codes.h"
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define GENERATOR_OFILEDIR "./object_log_files/5_producer"
#define HANDLER_OFILEDIR "./object_log_files/5_handler"
int main(void)
{
	pid_t generator_pid, handler_pid;
	generator_pid = fork();
	if (generator_pid < 0)
	{
		fprintf(stderr, "Could not fork generator\n");
		return FORK_ISSUE;
	}
	if (generator_pid == 0)
	{
		execlp(GENERATOR_OFILEDIR, GENERATOR_OFILEDIR, (char *)NULL);
		fprintf(stderr, "Got an error in generator process\n");
		return SUBPROCESS_ISSUE;
	}
	handler_pid = fork();
	if (handler_pid < 0)
	{
		fprintf(stderr, "Could not fork handler\n");
		kill(generator_pid, SIGTERM);
		return FORK_ISSUE;
	}
	if (handler_pid == 0)
	{
		execlp(HANDLER_OFILEDIR, HANDLER_OFILEDIR, NULL);
		fprintf(stderr, "Got an error in handler process\n");
		return SUBPROCESS_ISSUE;
	}
	int status;
	if (waitpid(generator_pid, &status, 0) == -1)
	{
		fprintf(stderr, "Generator process wait error\n");
		return SUBPROCESS_ISSUE;
	}
	if (waitpid(handler_pid, &status, 0) == -1)
	{
		fprintf(stderr, "Handler process wait error\n");
		return SUBPROCESS_ISSUE;
	}
	return SUCCESS;
}
