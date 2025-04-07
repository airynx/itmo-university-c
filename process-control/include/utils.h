#ifndef PROCESS_CONTROL_UTILS_H

#define CRONTAB_COMMAND "(crontab -l 2>/dev/null; echo \"%s\") | crontab -"
#define OBJECT_FILE_PATH "~/lab2/samurai-process-control-sakuranpetal/object_log_files/1_datetime"
#define LOG_FILE_PATH "~/lab2/samurai-process-control-sakuranpetal/object_log_files/1_datetime.log"
#define PIPENAME "aripipe"

int get_home_directory(char** homedir);

#define PROCESS_CONTROL_UTILS_H

#endif	  // PROCESS_CONTROL_UTILS_H
