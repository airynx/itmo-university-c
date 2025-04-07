#include "return_codes.h"
#include "utils.h"
#include <sys/types.h>

#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EX_RUNTIME_BANNER_LEN 19
#define NR_SWITCHES_BANNER_LEN 11
#define OUTPUT_FILENAME "4_cpu_burst.txt"
typedef struct
{
	pid_t pid;
	pid_t ppid;
	float avg;
} ProcData;

pid_t parse_ppid(pid_t pid)
{
	char path[MAX_LEN];
	sprintf(path, PROC_DIR "/%d/" STATUS_DIR, pid);
	FILE* status = fopen(path, "r");
	if (!status)
	{
		fprintf(stderr, "Could not get status information of a process with pid: %d\n", pid);
		return -1;
	}
	char line[MAX_LEN];
	while (fgets(line, sizeof line, status))
	{
		if (!strncmp(line, "PPid:", 5))
		{
			return atoi(line + 5);
		}
	}
	return -1;
}
float count_avg(pid_t pid)
{
	char path[MAX_LEN];
	sprintf(path, PROC_DIR "/%d/sched", pid);
	FILE* sched = fopen(path, "r");
	float sum_exec_runtime;
	int32_t nr_switches;
	if (!sched)
	{
		fprintf(stderr, "Could not get schedule information of a process with pid: %d\n", pid);
		return -1.;
	}
	char line[MAX_LEN];

	do
	{
		if (!fgets(line, sizeof line, sched))
			return -1.;
	} while (strncmp(line, "se.sum_exec_runtime", EX_RUNTIME_BANNER_LEN));
	char* colon_pos = strchr(line, ':');
	sum_exec_runtime = atof(colon_pos + 1);
	do
	{
		if (!fgets(line, sizeof line, sched))
			return -1.;
	} while (strncmp(line, "nr_switches", NR_SWITCHES_BANNER_LEN));
	colon_pos = strchr(line, ':');
	nr_switches = atoi(colon_pos + 1);

	if (nr_switches)
		return sum_exec_runtime / nr_switches;
	else
		return -1.;
}
int32_t comparator(const void* f, const void* s)
{
	ProcData* dataFirst = (ProcData*)f;
	ProcData* dataSecond = (ProcData*)s;
	return dataFirst->ppid - dataSecond->ppid;
}
int main(void)
{
	DIR* directory;
	struct dirent* entity;
	directory = opendir(PROC_DIR);
	size_t capacity = 128;
	ProcData* procData = malloc(capacity * sizeof(ProcData));
	if (!procData)
	{
		fprintf(stderr, "Could not allocate memory\n");
		return MEMORY_ALLOCATION_ERROR;
	}
	size_t size = 0;
	if (!directory)
	{
		fprintf(stderr, "Could not open process directory\n");
		free(procData);
		return OPEN_ERROR;
	}
	while ((entity = readdir(directory)))
	{
		if (!is_id(entity->d_name))
			continue;
		pid_t pid = (pid_t)atoi(entity->d_name);
		pid_t ppid = parse_ppid(pid);
		if (ppid == -1)
		{
			fprintf(stderr, "Could not find parent id for process with pid: %d\n", pid);
			continue;
		}
		float avg = count_avg(pid);
		if (avg == -1.)
		{
			fprintf(stderr, "Could not count average CPU burst for process with pid: %d\n", pid);
			continue;
		}
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
		procData[size].ppid = ppid;
		procData[size].avg = avg;
		++size;
	}
	closedir(directory);
	qsort(procData, size, sizeof(ProcData), comparator);
	FILE* out = fopen(OUTPUT_FILENAME, "w");
	if (!out)
	{
		fprintf(stderr, "Could not open file to write data\n");
		free(procData);
		return OPEN_ERROR;
	}
	for (size_t i = 0; i < size; i++)
	{
		fprintf(out,
				"ProcessID=%d : Parent_ProcessID=%d : Average_Running_Time=%f\n",
				procData[i].pid,
				procData[i].ppid,
				procData[i].avg);
	}
	free(procData);
	fclose(out);
	return OK;
}
