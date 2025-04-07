#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool is_id(char* name)
{
	size_t length = strlen(name);
	for (size_t i = 0; i < length; i++)
	{
		if (!isdigit(name[i]))
			return false;
	}
	return true;
}
