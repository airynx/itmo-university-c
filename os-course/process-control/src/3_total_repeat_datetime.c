#include "return_codes.h"
#include "utils.h"
#include <linux/limits.h>

#include <stdio.h>
#include <stdlib.h>
#define CRONTAB_FORM_JOB_STRING "*/5 * * * %s %s >> %s 2>&1 %s"
#define RUN_DAY "5"
#define IDENTIFIER "# every_lessonday_job"
int main(void)
{
	char jobstr[PATH_MAX];
	char command[PATH_MAX];
	snprintf(jobstr, sizeof jobstr, CRONTAB_FORM_JOB_STRING, RUN_DAY, OBJECT_FILE_PATH, LOG_FILE_PATH, IDENTIFIER);
	snprintf(command, sizeof command, CRONTAB_COMMAND, jobstr);
	if ((system(command)) == -1)
	{
		fprintf(stderr, "Could not execute crontab\n");
		return SYSTEM_COMMAND_ISSUE;
	}
	return SUCCESS;
}
