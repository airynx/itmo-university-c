#include "return_codes.h"

#include <stdio.h>
#include <stdlib.h>

int get_home_directory(char** homedir)
{
	*homedir = getenv("HOME");
	if (!*homedir)
	{
		fprintf(stderr, "Could not get home directory\n");
		return NON_SPECIFIED_ISSUE;
	}
	return SUCCESS;
}
