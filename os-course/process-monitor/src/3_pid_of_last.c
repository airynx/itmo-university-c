#include "return_codes.h"
#include "utils.h"
#include <sys/types.h>

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SPACES_TO_TIME 22

int main(void)
{
	struct dirent* dir_entity;
	DIR* dir;
	pid_t latest_pid = -1;
	uint64_t latest_time = 0;
	dir = opendir(PROC_DIR);
	if (!dir)
	{
		fprintf(stderr, "Could not open process directory\n");
		return OPEN_ERROR;
	}
	while ((dir_entity = readdir(dir)))
	{
		if (!is_id(dir_entity->d_name))
			continue;
		char path[MAX_LEN];
		snprintf(path, sizeof(path), PROC_DIR "/%s/stat", dir_entity->d_name);
		FILE* stat = fopen(path, "r");
		if (!stat)
		{
			fprintf(stderr, "Could not open stat file for a process with pid: %s", dir_entity->d_name);
			continue;
		}
		char line[MAX_LEN];
		if (!fgets(line, sizeof(line), stat))
		{
			fprintf(stderr, "Could not read a line from a stat file of a process with pid: %s\n", dir_entity->d_name);
			fclose(stat);
			continue;
		}
		int32_t spaces = 0;
		int32_t start = 0;
		int32_t index = 0;
		while (line[index] != '\0')
		{
			if (line[index] == '(')
			{
				while (line[index] != ')')
					++index;
			}
			start = index;
			while (line[index] != '\0' && line[index] != ' ')
				++index;
			if (line[index] == '\0')
				break;
			++spaces;
			if (spaces == SPACES_TO_TIME)
			{
				line[index] = '\0';
				break;
			}
			++index;
		}
		if (spaces == SPACES_TO_TIME)
		{
			uint64_t current_time = strtoul(&line[start], NULL, 10);
			if (latest_time < current_time)
			{
				latest_time = current_time;
				latest_pid = (pid_t)atoi(dir_entity->d_name);
			}
		}
	}
	closedir(dir);
	if (latest_pid != -1)
	{
		printf("PID of the latest started process: %d\n", latest_pid);
	}
	else
	{
		printf("Could not find the latest started process\n");
	}
	return OK;
}
