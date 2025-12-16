#ifndef TEST_H
#define TEST_H

#include "common.h"
#include "inotify.h"
void print_command_data(int is_valid, const char *command, const char *source_path, const char target_paths[][MAX_PATH_LEN],int target_count);
void print_watches();
void inicjalizujKopiarke();
#endif