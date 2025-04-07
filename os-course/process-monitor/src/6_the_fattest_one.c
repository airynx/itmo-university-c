#include "return_codes.h"
#include "utils.h"
#include <sys/types.h>

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VMRSS_TITLE_LEN 6

int main(void)
{
	DIR* directory;
	FILE* status;
	struct dirent* entity;
	const char vmrss_title[VMRSS_TITLE_LEN * 8] = "VmRSS:";
	char path[MAX_LEN], line[MAX_LEN];
	pid_t max_pid = -1;
	uint64_t vmrss = 0, max_vmrss = 0;
	directory = opendir(PROC_DIR);
	if (!directory)
	{
		fprintf(stderr, "Could not open process directory\n");
		return OPEN_ERROR;
	}
	while ((entity = readdir(directory)))
	{
		if (!is_id(entity->d_name))
			continue;

		sprintf(path, PROC_DIR "/%s/" STATUS_DIR, entity->d_name);
		status = fopen(path, "r");
		if (!status)
		{
			fprintf(stderr, "Could not get status information of a process with pid: %s\n", entity->d_name);
			continue;
		}
		while (fgets(line, MAX_LEN, status))
		{
			if (!strncmp(line, vmrss_title, VMRSS_TITLE_LEN))
			{
				sscanf(line + VMRSS_TITLE_LEN, "%lu", &vmrss);
				break;
			}
		}
		fclose(status);
		if (vmrss > max_vmrss)
		{
			max_vmrss = vmrss;
			max_pid = (pid_t)atoi(entity->d_name);
		}
	}
	closedir(directory);
	if (max_pid != -1)
	{
		printf("Max RAM allocated is: %lu to a process with PID: %d\n", max_vmrss, max_pid);
	}
	else
	{
		fprintf(stderr, "Could not find a process that allocated max RAM\n");
	}
	return OK;
}
