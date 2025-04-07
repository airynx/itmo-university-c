#include "return_codes.h"
#include "utils.h"

#include <dirent.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
	pid_t pid;
	char* command;
	int64_t bytes_before;
	int64_t bytes_after;

} ProcData;

bool parse_readbytes(pid_t pid, int64_t* bytes)
{
	char path[MAX_LEN];
	snprintf(path, sizeof path, PROC_DIR "/%d/io", pid);
	FILE* ioFile = fopen(path, "r");
	if (!ioFile)
	{
		fprintf(stderr, "Could not get io information of a process with pid %d\n", pid);
		return false;
	}
	char line[MAX_LEN];
	while (fgets(line, MAX_LEN, ioFile))
	{
		if (!strncmp(line, "read_bytes:", 11))
		{
			sscanf(line + 12, "%ld", bytes);
			break;
		}
	}
	fclose(ioFile);
	return true;
}
bool parse_cmdline(pid_t pid, char* cmdline)
{
	char path[MAX_LEN];
	snprintf(path, sizeof path, PROC_DIR "/%d/cmdline", pid);
	FILE* cmdlineFile = fopen(path, "r");
	if (!cmdlineFile)
	{
		fprintf(stderr, "Could not get cmdline information of a process with pid %d\n", pid);
		return false;
	}
	char line[MAX_LEN];
	bool result = fread(cmdline, 1, sizeof(line) - 1, cmdlineFile);
	fclose(cmdlineFile);
	return result;
}
int comparator(const void* f, const void* s)
{
	ProcData* dataFirst = (ProcData*)f;
	ProcData* dataSecond = (ProcData*)s;
	int64_t diff1 = (dataSecond->bytes_before - dataSecond->bytes_after);
	int64_t diff2 = (dataFirst->bytes_before - dataFirst->bytes_after);
	if (diff2 > diff1)
		return 1;
	else if (diff2 < diff1)
		return -1;
	else
		return 0;
}
int main(void)
{
	DIR* proc_directory = opendir(PROC_DIR);
	struct dirent* entity;
	if (!proc_directory)
	{
		fprintf(stderr, "Could not open proc directory\n");
		return OPEN_ERROR;
	}
	size_t capacity = 128;
	ProcData* procData = malloc(128 * sizeof(ProcData));
	if (!procData)
	{
		fprintf(stderr, "Could not allocate memory\n");
		return MEMORY_ALLOCATION_ERROR;
	}
	size_t size = 0;
	int64_t bytes = 0;
	char cmdline[MAX_LEN];
	pid_t pid;

	while ((entity = readdir(proc_directory)))
	{
		if (!is_id(entity->d_name))
			continue;

		pid = atoi(entity->d_name);
		if (!parse_readbytes(pid, &bytes))
			continue;
		if (!parse_cmdline(pid, cmdline))
			continue;
		if (capacity <= size)
		{
			int32_t new_capacity = 2 * capacity;
			ProcData* realloced_procData = realloc(procData, sizeof(ProcData) * new_capacity);
			if (!realloced_procData)
			{
				fprintf(stderr, "Could not reallocate program struct\n");
				break;
			}
			capacity = new_capacity;
			procData = realloced_procData;
		}
		procData[size].pid = pid;
		procData[size].bytes_before = bytes;
		procData[size].bytes_after = 0;
		procData[size].command = strdup(cmdline);
		if (!procData[size].command)
		{
			fprintf(stderr, "Could not allocate memory\n");
			break;
		}
		++size;
	}
	closedir(proc_directory);
	sleep(60);
	proc_directory = opendir(PROC_DIR);
	while ((entity = readdir(proc_directory)))
	{
		pid = atoi(entity->d_name);
		if (!parse_readbytes(pid, &bytes))
			continue;

		for (size_t i = 0; i < size; i++)
		{
			if (procData[i].pid == pid)
			{
				procData[i].bytes_after = bytes;
				break;
			}
		}
	}
	closedir(proc_directory);
	if (size != 0)
	{
		qsort(procData, size, sizeof(ProcData), comparator);

		for (size_t i = 0; i < size; i++)
		{
			printf("%ld : %s : %ld" PRId64 "\n",
				   procData[i].pid,
				   procData[i].command,
				   procData[i].bytes_after - procData[i].bytes_before);
			if (i == 2)
				break;
		}
	}
	else
		puts("No information about read bytes of any process was found\n");

	for (size_t i = 0; i < size; i++)
		free(procData[i].command);
	free(procData);
	return OK;
}
