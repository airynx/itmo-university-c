#include "return_codes.h"
#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>

#define stateback                                                                                                      \
	last_ppid = ppid;                                                                                                  \
	avg_accum = avg;
#define inputf "4_cpu_burst.txt"
#define tempf "4temp.txt"
#define MAX_LEN 4096
int main(void)
{
	FILE* r_file = fopen(inputf, "r");
	FILE* w_file = fopen(tempf, "w");
	if (!r_file || !w_file)
	{
		fprintf(stderr, "Could not open file!\n");
		return OPEN_ERROR;
	}
	char line[MAX_LEN];
	int32_t streak = 1;
	pid_t ppid, last_ppid = -1;
	float avg, avg_accum = 0.;
	while (fgets(line, sizeof line, r_file))
	{
		if (sscanf(line, "ProcessID=%*s : Parent_ProcessID=%d : Average_Running_Time=%f", &ppid, &avg) != 2)
		{
			fprintf(stderr, "Current line is currupted:\n%s\n", line);
			continue;
		}

		if (last_ppid != -1)
		{
			if (last_ppid == ppid)
			{
				++streak;
				avg_accum += avg;
			}
			else
			{
				fprintf(w_file, "Average_Running_Children_of_ParentID=%d is %f\n", last_ppid, avg_accum / streak);
				streak = 1;
				stateback
			}
		}
		else
		{
			stateback
		}

		fputs(line, w_file);
	}
	fprintf(w_file, "Average_Running_Children_of_ParentID=%d is %f\n", last_ppid, avg_accum / streak);
	fclose(r_file);
	fclose(w_file);
	if (remove(inputf))
	{
		fprintf(stderr,
				"This realization substitutes input file, but program can not remove original file,"
				"see output at " tempf "\n");
	}
	else
	{
		if (rename(tempf, inputf))
			fprintf(stderr, "Could not rename temp file, see outout at " tempf "\n");
	}
	return OK;
}
