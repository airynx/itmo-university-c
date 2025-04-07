#include "return_codes.h"
#include "utils.h"
#include <sys/types.h>

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define OUTPUT_FILENAME "1_users_processes.txt"
typedef struct
{
	char* process_id;
	char* command;
} ProcData;

uid_t parse_uid(char* process_id)
{
	char path[MAX_LEN];
	uid_t status_userid = -1;
	snprintf(path, sizeof(path), PROC_DIR "/%s/" STATUS_DIR, process_id);
	FILE* status_file = fopen(path, "r");

	if (!status_file)
	{
		fprintf(stderr, "Can not open status file: %s\n", path);
		return status_userid;
	}
	char line[MAX_LEN];

	while (fgets(line, sizeof line, status_file))
	{
		if (!strncmp(line, "Uid:", 4))
		{
			if (sscanf(line, "Uid:\t%u", &status_userid) != 1)
			{
				fprintf(stderr, "Could not read USERID from status file\n");
				status_userid = -1;
			}
			break;
		}
	}
	fclose(status_file);
	return status_userid;
}
char* parse_command(char* process_id)
{
	char path[MAX_LEN];
	snprintf(path, sizeof(path), PROC_DIR "/%s/comm", process_id);
	FILE* comm_file = fopen(path, "r");
	if (!comm_file)
	{
		fprintf(stderr, "Could not open file: %s\n", path);
		return NULL;
	}
	char* command = malloc(MAX_LEN);
	if (!command)
	{
		fprintf(stderr, "Could not allocate memory for command buffer\n");
		fclose(comm_file);
		return NULL;
	}
	if (!fgets(command, 128, comm_file))
	{
		fprintf(stderr, "Could not read command from file: %s\n", path);
		free(command);
		fclose(comm_file);
		return NULL;
	}
	fclose(comm_file);
	char* index = command + strlen(command) - 1;
	while (command <= index && isspace(*index))
	{
		*(index--) = '\0';
	}
	return command;
}
int main(void)
{
	DIR* process_dir = opendir(PROC_DIR);
	if (!process_dir)
	{
		fprintf(stderr, "Can not open process directory\n");
		return OPEN_ERROR;
	}
	const uid_t USERID = getuid();

	struct dirent* directory;
	size_t capacity = 128;

	ProcData* procdata = (ProcData*)malloc(capacity * sizeof(ProcData));
	if (!procdata)
	{
		fprintf(stderr, "Can not allocate memory for program structure\n");
		return MEMORY_ALLOCATION_ERROR;
	}
	size_t counter = 0;
	while ((directory = readdir(process_dir)))
	{
		if (!is_id(directory->d_name))
			continue;
		if (parse_uid(directory->d_name) != USERID)
			continue;
		char* command = parse_command(directory->d_name);
		if (command)
		{
			if (capacity <= counter)
			{
				size_t new_capacity = 2 * capacity;
				ProcData* realloced_procdata = realloc(procdata, sizeof(ProcData) * new_capacity);
				if (!realloced_procdata)
				{
					fprintf(stderr, "Could not reallocate program struct\n");
					free(command);
					break;
				}
				capacity = new_capacity;
				procdata = realloced_procdata;
			}
			procdata[counter].process_id = strdup(directory->d_name);
			if (!procdata[counter].process_id)
			{
				fprintf(stderr, "Could not allocate memory for process id: %s\n", directory->d_name);
				free(command);
				continue;
			}
			procdata[counter].command = command;
			++counter;
		}
	}
	closedir(process_dir);
	FILE* out = fopen(OUTPUT_FILENAME, "w");
	if (!out)
	{
		fprintf(stderr, "Could not open output file!\n");
		for (size_t i = 0; i < counter; i++)
		{
			free(procdata[i].process_id);
			free(procdata[i].command);
		}
		free(procdata);
		return MEMORY_ALLOCATION_ERROR;
	}
	fprintf(out, "%zu\n", counter);
	for (size_t i = 0; i < counter; i++)
	{
		fprintf(out, "%s:%s\n", procdata[i].process_id, procdata[i].command);
	}
	for (size_t i = 0; i < counter; i++)
	{
		printf("%s:%s\n", procdata[i].process_id, procdata[i].command);
	}
	fclose(out);
	for (size_t i = 0; i < counter; i++)
	{
		free(procdata[i].process_id);
		free(procdata[i].command);
	}
	free(procdata);
	return OK;
}
