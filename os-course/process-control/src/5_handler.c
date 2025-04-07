#include "return_codes.h"
#include "utils.h"
#include <sys/stat.h>
#include <sys/types.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum
{
	ADD,
	MULT,
	NUM,
	BREAK,
	BAD
} Mode;

static Mode parse_op(char* line, size_t length, int64_t* lastnum)
{
	char* endptr;
	if (length == 2 && line[0] == '+')
		return ADD;
	if (length == 2 && line[0] == '*')
		return MULT;
	if (!strcmp(line, "QUIT\n"))
		return BREAK;

	*lastnum = strtol(line, &endptr, 10);
	if (endptr != line && *endptr == '\n')
		return NUM;
	else
		return BAD;
}

int main(void)
{
	// setvbuf(stdout, NULL, _IONBF, 0);
	FILE* pipe = fopen(PIPENAME, "r");
	char* line = NULL;
	ssize_t bytesread;
	size_t buflen = 0;
	int64_t accum = 1, lastnum = 0;
	Mode mode = ADD;
	bool quit_msg = false;
	while ((bytesread = getline(&line, &buflen, pipe)) != -1)
	{
		switch (parse_op(line, strlen(line), &lastnum))
		{
		case ADD:
			mode = ADD;
			puts("[Handler]: Mode switched to +");
			break;
		case MULT:
			mode = MULT;
			puts("[Handler]: Mode switched to *");
			break;
		case NUM:
			if (mode == ADD)
				accum += lastnum;
			else if (mode == MULT)
				accum *= lastnum;
			printf("[Handler]: Current result is %" PRId64 "\n", accum);
			break;
		case BREAK:
			puts("[Handler]: Quitting...");
			quit_msg = true;
			break;
		default:
		case BAD:
			puts("[Handler]: Invalid input, exiting...");
			quit_msg = true;
			break;
		}
		fflush(stdout);
		if (quit_msg)
			break;
	}
	if (bytesread == -1 && !feof(pipe))
		fprintf(stderr, "[Handler Error]: Could not read from pipe\n");
	free(line);
	fclose(pipe);
	return SUCCESS;
}
