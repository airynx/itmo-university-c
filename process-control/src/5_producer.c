#include "return_codes.h"
#include "utils.h"
#include <sys/stat.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool is_good_input(char* line)
{
	if ((strlen(line) == 2 && (line[0] == '+' || line[0] == '*')) || !strcmp(line, "QUIT\n"))
		return true;
	size_t i = 0;
	while (isdigit(line[i]))
		i++;
	if (line[i] == '\n')
		return true;

	return false;
}
int main(void)
{
	char* line = NULL;
	int descriptor_id = 0;
	ssize_t bytesread = 0;
	size_t bufsz = 0;

	if (mkfifo(PIPENAME, 0666) == -1)
	{
		if (errno != EEXIST)
		{
			fprintf(stderr, "[Generator Error]: Error in pipe creation\n");
			return PIPE_ISSUE;
		}
	}
	if ((descriptor_id = open(PIPENAME, O_WRONLY)) == -1)
	{
		fprintf(stderr, "[Generator Error]: Could not open pipe for writing\n");
		return FILE_OPENING_ISSUE;
	}
	puts("[Generator]: Init completed. Please enter an op or digit below");
	while (true)
	{
		printf(">>> ");
		bytesread = getline(&line, &bufsz, stdin);
		if (bytesread == -1)
		{
			if (feof(stdin))
			{
				sprintf(line, "QUIT\n");
				bytesread = strlen(line);
			}
			else
			{
				fprintf(stderr, "[Generator Error]: stdin read error\n");
				break;
			}
		}
		if (!bytesread || (bytesread == 1 && line[0] == '\n'))
			continue;
		if (write(descriptor_id, line, bytesread) == -1)
		{
			fprintf(stderr, "[Generator Error]: could not write information to the pipe\n");
			break;
		}
		if (!strcmp(line, "QUIT\n"))
			break;

		usleep(80000);
		if (!is_good_input(line))
		{
			puts("[Generator]: Invalid input!");
			break;
		}
	}
	free(line);
	close(descriptor_id);
	if (unlink(PIPENAME) == -1)
	{
		fprintf(stderr, "[Generator Error]: Could not delete pipe\n");
		return 3;
	}
	puts("[Generator]: Shutting down...");
	return SUCCESS;
}
