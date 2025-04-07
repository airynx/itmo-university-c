#include "return_codes.h"
#include "utils.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OUTPUT_FILENAME "2_from_sbin.txt"
int main(void)
{
	DIR* process_dir = opendir(PROC_DIR);
	if (!process_dir)
	{
		fprintf(stderr, "Could not open process directory\n");
		return OPEN_ERROR;
	}

	FILE* out = fopen(OUTPUT_FILENAME, "w");
	if (!out)
	{
		fprintf(stderr, "Could not open output file\n");
		closedir(process_dir);
		return OPEN_ERROR;
	}
	char link[MAX_LEN];
	char path[MAX_LEN];
	ssize_t link_length;
	struct dirent* directory;
	while ((directory = readdir(process_dir)))
	{
		if (!is_id(directory->d_name))
			continue;
		snprintf(link, sizeof(link), PROC_DIR "/%s/exe", directory->d_name);
		link_length = readlink(link, path, sizeof(path) - 1);
		if (link_length == -1)
		{
			if (errno != ENOENT)
			{
				if (errno == EACCES)
				{
					fprintf(stderr,
							"You have no permission to access the executable of process with PID=%s\n",
							directory->d_name);
				}
				else
				{
					fprintf(stderr, "An issue with reading link of a process executable: PID:%s, ERR:%s\n", directory->d_name, strerror(errno));
				}
			}
			continue;
		}
		path[link_length] = '\0';	 // readlink does not append null terminator
		if (!strncmp(path, "/usr/sbin/", 10))
			fprintf(out, "PID: %s\n", directory->d_name);
	}
	fclose(out);
	closedir(process_dir);
	return OK;
}
