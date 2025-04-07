#define _GNU_SOURCE

#include "return_codes.h"
#include <linux/limits.h>
#include <sys/types.h>

#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define PROC_PID_PATH "/proc/%lld/stat"
#define PROC_PATH "/proc/stat"

static volatile sig_atomic_t stop_flag = 0;
static double percentage = 0.;
static void usr1_handler(int signl)
{
	stop_flag = 1;
}
static int division_loop()
{
	struct sigaction sigact = { 0 };
	sigact.sa_handler = usr1_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	int64_t a = 212323238900290323;
	int64_t b = 112323098908232323;
	int64_t c = 1;
	int32_t current_sleeptime = 0;

	if (sigaction(SIGUSR1, &sigact, NULL) == -1)
	{
		fprintf(stderr, "Sigaction issue!\n");
		return SIGNAL_ISSUE;
	}
	while (true)
	{
		if (stop_flag)
		{
			current_sleeptime = (int)percentage / 10;
			if (current_sleeptime > 8)
				current_sleeptime = 8;
			sleep(current_sleeptime);
			stop_flag = 0;
		}
		c *= (a * (b + 1)) / (b + 2);
	}
	return SUCCESS;
}
static bool skip_char(FILE* file, char skipto)
{
	int chr;
	while ((chr = fgetc(file)) != skipto && chr != EOF)
		;
	if (chr == EOF)
		return false;
	return true;
}
static int get_cpu_time(uint64_t* cpu_time, pid_t process_pid)
{
	FILE* statfile;
	char stat_path[PATH_MAX];
	uint64_t utime, stime;

	snprintf(stat_path, sizeof stat_path, PROC_PID_PATH, process_pid);
	statfile = fopen(stat_path, "r");
	if (!statfile)
	{
		fprintf(stderr, "Could not open proc/<pid>/stat\n");
		return FILE_OPENING_ISSUE;
	}
	if (!skip_char(statfile, '('))
	{
		fclose(statfile);
		return FILE_CONTENT_ISSUE;
	}
	if (!skip_char(statfile, ')'))
	{
		fclose(statfile);
		return FILE_CONTENT_ISSUE;
	}
	for (int i = 3; i < 14; i++)
		fscanf(statfile, "%*s");

	if (fscanf(statfile, "%" SCNu64 " %" SCNu64, &utime, &stime) != 2)
	{
		fprintf(stderr, "Scanf error when reading utime, stime\n");
		fclose(statfile);
		return FILE_READING_ISSUE;
	}
	*cpu_time = utime + stime;
	fclose(statfile);
	return SUCCESS;
}
static int get_total_time(uint64_t* total_time)
{
	FILE* procstat = fopen(PROC_PATH, "r");
	char* line = NULL;
	char* readptr = NULL;
	ssize_t bytesread;
	size_t length;
	if (!procstat)
	{
		fprintf(stderr, "Could not open /proc/stat\n");
		return FILE_OPENING_ISSUE;
	}

	if ((bytesread = getline(&line, &length, procstat)) == -1)
	{
		fprintf(stderr, "Could not read /proc/stat\n");
		free(line);
		return FILE_READING_ISSUE;
	}
	if (strncmp(line, "cpu ", 4))
	{
		fprintf(stderr, "Content of /proc/stat is strange\n");
		free(line);
		return FILE_CONTENT_ISSUE;
	}

	readptr = line + 4;
	uint64_t acc = 0;
	uint64_t current_num;
	char* wordend;
	while (*readptr != '\0')
	{
		current_num = strtoull(readptr, &wordend, 10);
		if (readptr == wordend)
			break;
		acc += current_num;
		readptr = wordend;
	}
	*total_time = acc;
	free(line);
	fclose(procstat);
	return SUCCESS;
}
static int control_cpu_of_process(pid_t process_pid)
{
	int32_t processors;
	uint64_t cpu_time, total_time, prev_cpu_time, prev_total_time;

	get_cpu_time(&prev_cpu_time, process_pid);
	get_total_time(&prev_total_time);
	while (true)
	{
		sleep(1);
		processors = sysconf(_SC_NPROCESSORS_ONLN);
		if (get_cpu_time(&cpu_time, process_pid) || get_total_time(&total_time))
			continue;
		percentage = 100.0 * processors * ((double)(cpu_time - prev_cpu_time) / (double)(total_time - prev_total_time));
		printf("Percentage is %f\n", percentage);
		if (percentage > 10.)
		{
			if (kill(process_pid, SIGUSR1) == -1)
			{
				fprintf(stderr, "Issue when sending SIGUSR1 signal\n");
				return SIGNAL_ISSUE;
			}
		}
		prev_cpu_time = cpu_time;
		prev_total_time = total_time;
	}
	return SUCCESS;
}
int main(void)
{
	pid_t child_pids[3];
	for (int i = 0; i < 3; i++)
	{
		pid_t child_pid = fork();
		if (child_pid < 0)
		{
			fprintf(stderr, "Error in fork\n");
			for (int j = 0; j < i; j++)
				kill(child_pids[j], SIGTERM);
			return FORK_ISSUE;
		}
		else if (child_pid == 0)
			division_loop();
		child_pids[i] = child_pid;
	}
	pid_t monitor_process_pid = fork();
	if (monitor_process_pid < 0)
	{
		fprintf(stderr, "Error in fork\n");
		return FORK_ISSUE;
	}
	else if (monitor_process_pid == 0)
		control_cpu_of_process(child_pids[0]);

	sleep(5);
	kill(child_pids[2], SIGTERM);
	sleep(30);
	for (int i = 0; i < 2; i++)
		kill(child_pids[i], SIGTERM);
	kill(monitor_process_pid, SIGTERM);
	return SUCCESS;
}
