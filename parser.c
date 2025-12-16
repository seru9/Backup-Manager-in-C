#include "parser.h"

void parse_command(char *input_buffer, CommandStruct *result) {
    memset(result, 0, sizeof(CommandStruct));
    result->is_valid = 1;

    char *p = input_buffer;
    int token_index = 0;

    while (*p != '\0' && *p != '\n') {
        while (isspace((unsigned char)*p)) p++; //while(*p == ' ')
        if (*p == '\0' || *p == '\n') break;

        char *token_start = p;
        int in_quotes = 0;

        if (*p == '"') {
            in_quotes = 1;
            token_start++;
            p++;
        }

        while (*p != '\0' && *p != '\n') {
            if (in_quotes) {
                if (*p == '"') {
                    *p = '\0';
                    p++;
                    in_quotes = 0;
                    break;
                }
            } else {
                if (isspace((unsigned char)*p)) {
                    *p = '\0';
                    p++;
                    break;
                }
            }
            p++;
        }

        if (in_quotes) {
            printf("Error: Unclosed quotes!\n");
            result->is_valid = 0; 
            return; // Return void
        }

        if (token_index == 0) {
            strncpy(result->command, token_start, sizeof(result->command) - 1);
        } else if (token_index == 1) {
            resolve_absolute_path(token_start, result->source_path);
        } else {
            if (result->target_count < MAX_ARGS) {
                resolve_absolute_path(token_start, result->target_paths[result->target_count]);
                result->target_count++;
            }
        }
        token_index++;
    }
}

void resolve_absolute_path(const char *input_path, char *output_buffer){
    char temp_path[MAX_PATH_LEN];

    if (realpath(input_path, temp_path) != NULL) {
        strncpy(output_buffer, temp_path, MAX_PATH_LEN);
    } else {
        if (input_path[0] == '/') {
            strncpy(output_buffer, input_path, MAX_PATH_LEN);
        } else {
            if (getcwd(temp_path, sizeof(temp_path)) != NULL) {
                int written = snprintf(output_buffer, MAX_PATH_LEN, "%s/%s", temp_path, input_path);

                if (written >= MAX_PATH_LEN || written < 0) {
                    ERR("snpritnf - paths too long to fit the buffer");
                    return;
                }
            } else {
                strncpy(output_buffer, input_path, MAX_PATH_LEN);
            }
        }
    }
    output_buffer[MAX_PATH_LEN - 1] = '\0';
}
void assigner(CommandStruct *command_and_paths, char *command, char *source_path, char target_paths[][MAX_PATH_LEN], int *target_count)                 // WAŻNE: Wskaźnik, żeby zmienić wartość
{
    strncpy(command, command_and_paths->command, sizeof(command_and_paths->command) - 1);
    command[sizeof(command_and_paths->command) - 1] = '\0';

    strncpy(source_path, command_and_paths->source_path, MAX_PATH_LEN - 1);
    source_path[MAX_PATH_LEN - 1] = '\0';


    *target_count = command_and_paths->target_count;

    for(int i = 0; i < *target_count; i++) {
        strncpy(target_paths[i], command_and_paths->target_paths[i], MAX_PATH_LEN - 1);
        target_paths[i][MAX_PATH_LEN - 1] = '\0';
    }
}
int check_command(char* command, char* source_path, char target_paths[][MAX_PATH_LEN], int target_count){
    if(strcmp(command, "list") == 0 || strcmp(command, "exit") == 0){ return 1; }
    if(strcmp(command, "restore") == 0 && target_count != 1){ 
        printf("Błąd: Polecenie 'restore' wymaga ścieżki źródłowej i jednej ścieżki typu backup.\n");
        return 0; 
    }
    if (target_count < 1 && (strcmp(command, "add") == 0 || strcmp(command, "end") == 0)) {
            printf("Błąd: Polecenie 'add/end' wymaga ścieżki źródłowej i co najmniej jednej docelowej.\n");
            printf("Użycie: add/end <source path> <target path 1> [target path 2] ...\n");
            return 0;
        }
    /*Dodaj jakieś testowanie?*/
    return 1;
}