#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include "return_codes.h"
#include "utils.h"
#include <linux/limits.h>
#include <sys/stat.h>

#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define TEST_DIRNAME "test"
#define ARCHIVE_DIRNAME "archived"
#define LOG_FILENAME "report"
#define DATE_FORMATSTR "%Y-%m-%d"
#define TIME_FORMATSTR "%H-%M-%S"
#define VALID_DATE_TEMPLATE "%Y-%m-%d_%H-%M-%S"
#define LOG_BANNER "test was created successfully"
#define CREATE_ARCHIVE_BASH_COMMAND                                                                                    \
	"[ -f \"%s.tar\" ] && tar -C \"%s\" --remove-files -rf \"%s.tar\" \"%s\" || tar -C \"%s\" --remove-files -cf "     \
	"\"%s.tar\" \"%s\""

static int make_directory(char* dirpath, const char* homepath, const char* name)
{
	snprintf(dirpath, PATH_MAX, "%s/%s", homepath, name);
	struct stat dirstat;
	if (stat(dirpath, &dirstat) == -1 && (mkdir(dirpath, 0700)))
	{
		fprintf(stderr, "Could not create directory\n");
		return DIR_CREATION_ISSUE;
	}
	return SUCCESS;
}

static int put_datefile(struct tm* timeinfo, char* path)
{
	char current_date[PATH_MAX];
	char name[PATH_MAX];
	FILE* file_entity;

	strftime(current_date, sizeof current_date, DATE_FORMATSTR "_" TIME_FORMATSTR, timeinfo);
	snprintf(name, sizeof name, "%s/%s", path, current_date);
	file_entity = fopen(name, "w");
	if (!file_entity)
	{
		fprintf(stderr, "Could not create testfile\n");
		return FILE_CREATION_ISSUE;
	}
	fclose(file_entity);
	return SUCCESS;
}
static int create_log(char* dirpath, struct tm* timeinfo)
{
	char log_string[PATH_MAX];
	char log_path[PATH_MAX];
	char current_date[PATH_MAX];
	FILE* file_entity;

	snprintf(log_path, strlen(dirpath) + strlen(LOG_FILENAME) + 2, "%s/%s", dirpath, LOG_FILENAME);
	strftime(current_date, sizeof current_date, DATE_FORMATSTR ":" TIME_FORMATSTR, timeinfo);
	snprintf(log_string, sizeof log_string, "%s " LOG_BANNER "\n", current_date);
	file_entity = fopen(log_path, "a");
	if (!file_entity)
	{
		fprintf(stderr, "Could not open file to log\n");
		return FILE_OPENING_ISSUE;
	}
	if (fputs(log_string, file_entity) == EOF)
	{
		fprintf(stderr, "Could not write log info in a file\n");
		fclose(file_entity);
		return FILE_WRITING_ISSUE;
	}
	fclose(file_entity);
	return SUCCESS;
}
static int get_timeinfo(struct tm** timeinfo)
{
	time_t timer;

	if ((time(&timer) == -1))
	{
		fprintf(stderr, "Could not get current time\n");
		return TIME_ISSUE;
	}
	*timeinfo = localtime(&timer);
	if (!*timeinfo)
	{
		fprintf(stderr, "Could not convert time\n");
		return TIME_ISSUE;
	}
	return SUCCESS;
}

static bool is_well_formed_filename(const char* string)
{
	struct tm timestrc = { 0 };
	char* str_end = strptime(string, VALID_DATE_TEMPLATE, &timestrc);

	if (!str_end || *str_end != '\0' || (mktime(&timestrc) == -1))
		return false;
	return true;
}
static int archive(char* archive_name, char* file_destination, char* filename)
{
	char command[PATH_MAX];

	snprintf(command, PATH_MAX, CREATE_ARCHIVE_BASH_COMMAND, archive_name, file_destination, archive_name, filename, file_destination, archive_name, filename);
	if (system(command) == -1)
	{
		fprintf(stderr, "Could not archive previous data\n");
		return SYSTEM_COMMAND_ISSUE;
	}
	return SUCCESS;
}
static int archive_all_previous(char* dirname, char* archive_pathname, struct tm* timeinfo)
{
	struct dirent* entity;
	char prefix[PATH_MAX];
	DIR* directory = opendir(dirname);

	if (!directory)
	{
		fprintf(stderr, "Could not open ~/test directory\n");
		return DIR_OPENING_ISSUE;
	}
	strftime(prefix, sizeof prefix, DATE_FORMATSTR, timeinfo);
	size_t prefix_len = strlen(prefix);
	while ((entity = readdir(directory)))
	{
		if (entity->d_type != DT_REG || (!strncmp(entity->d_name, prefix, prefix_len)))
			continue;
		if (is_well_formed_filename(entity->d_name))
		{
			char archive_put_path[PATH_MAX];
			char current_prefix[PATH_MAX];
			strncpy(current_prefix, entity->d_name, prefix_len);
			current_prefix[prefix_len] = '\0';
			snprintf(archive_put_path, sizeof archive_put_path, "%s/%s", archive_pathname, current_prefix);
			archive(archive_put_path, dirname, entity->d_name);
		}
	}
	closedir(directory);
	return SUCCESS;
}
int main(void)
{
	struct tm* timeinfo;
	char* homedir;
	char testdir[PATH_MAX];
	char archivedir[PATH_MAX];
	int errcode;

	if ((errcode = get_home_directory(&homedir)))
		return errcode;

	if ((errcode = make_directory(testdir, homedir, TEST_DIRNAME)))
		return errcode;

	if ((errcode = get_timeinfo(&timeinfo)))
		return errcode;

	if ((errcode = make_directory(archivedir, homedir, TEST_DIRNAME "/" ARCHIVE_DIRNAME)))
		return errcode;

	if ((errcode = archive_all_previous(testdir, archivedir, timeinfo)) == 10)
		return errcode;

	if ((errcode = put_datefile(timeinfo, testdir)))
		return errcode;

	if ((errcode = create_log(homedir, timeinfo)))
		return errcode;
	return SUCCESS;
}
