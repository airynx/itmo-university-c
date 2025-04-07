#include "return_codes.h"
#include "utils.h"
#include <linux/limits.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define CRONTAB_FORM_JOB_STRING "%d %d %d %d * %s >> %s 2>&1 %s"
#define IDENTIFIER "# next_two_min_job"
#define UPDATING_FILE "report"

static int schedule()
{
	time_t t = time(NULL);
	t += 120;
	struct tm* tm = localtime(&t);
	if (!tm)
	{
		fprintf(stderr, "Could not get local time\n");
		return TIME_ISSUE;
	}
	char jobstr[PATH_MAX];
	char command[PATH_MAX];
	snprintf(jobstr, sizeof jobstr, CRONTAB_FORM_JOB_STRING, tm->tm_min, tm->tm_hour, tm->tm_mday, tm->tm_mon + 1, OBJECT_FILE_PATH, LOG_FILE_PATH, IDENTIFIER);
	snprintf(command, sizeof command, CRONTAB_COMMAND, jobstr);
	if ((system(command)) == -1)
	{
		fprintf(stderr, "Could not execute crontab\n");
		return SYSTEM_COMMAND_ISSUE;
	}
	return SUCCESS;
}
static int check_update()
{
	FILE* file;
	char file_path[PATH_MAX];
	char* homedir;
	char* line = NULL;
	size_t length = 0;
	ssize_t read_data = -1;
	int errcode;
	off_t pos;
	bool printed = false;

	if ((errcode = get_home_directory(&homedir)))
		return errcode;
	snprintf(file_path, sizeof file_path, "%s/%s", homedir, UPDATING_FILE);
	file = fopen(file_path, "r");
	if (!file)
	{
		fprintf(stderr, "Could not open report file\n");
		return FILE_OPENING_ISSUE;
	}
	if (fseek(file, 0, SEEK_END))
	{
		fprintf(stderr, "Could not place file cursor\n");
		fclose(file);
		return CURSOR_ISSUE;
	}
	pos = ftell(file);
	if (pos == -1L)
	{
		fprintf(stderr, "Could not get cursor position\n");
		fclose(file);
		return CURSOR_ISSUE;
	}
	sleep(115);

	while (true)
	{
		if (printed)
			break;
		sleep(3);
		fclose(file);
		file = fopen(file_path, "r");
		if (!file)
		{
			fprintf(stderr, "Could not reopen report file\n");
			return FILE_OPENING_ISSUE;
		}
		fseek(file, pos, SEEK_CUR);
		if ((read_data = getline(&line, &length, file)) != EOF)
		{
			printed = true;
			do
				puts(line);
			while ((read_data = getline(&line, &length, file)) != EOF);
		}
	}
	free(line);
	fclose(file);
	return SUCCESS;
}
int main(void)
{
	int errcode;
	if ((errcode = schedule()))
		return errcode;
	if ((errcode = check_update()))
		return errcode;
	return SUCCESS;
}
