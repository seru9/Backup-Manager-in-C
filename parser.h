#ifndef PARSER_H
#define PARSER_H

#include "common.h"

void parse_command(char *input_buffer, CommandStruct *result);
void resolve_absolute_path(const char *input_path, char *output_buffer);
void assigner(CommandStruct *command_and_paths, char *command, char *source_path, char target_paths[][MAX_PATH_LEN], int *target_count);
int check_command(char* command, char* source_path, char target_paths[][MAX_PATH_LEN], int target_count);
#endif