#ifndef COMMON_H
#define COMMON_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <limits.h>
#include <sys/inotify.h>
#include <limits.h>
// Stałe
#define MAX_COMMAND_LENGTH 5120
#define MAX_ARGS 18
#define MAX_PATH_LEN 256
#define MAX_WATCHERS 128
// Makro do obsługi błędów
#define ERR(source) \
    (fprintf(stderr, "%s:%d: ", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

// Struktury
typedef struct {
    char command[32];              // np. "add", "end"
    char source_path[MAX_PATH_LEN];
    char target_paths[MAX_ARGS][MAX_PATH_LEN];
    int target_count;
    int is_valid;                  // 1 = sukces, 0 = błąd parsowania
} CommandStruct;

typedef struct {
    pid_t child;
    char source_path[MAX_PATH_LEN];
    char target_path[MAX_PATH_LEN];
} pid_source_target;

typedef struct {
    int wd;
    char path[MAX_PATH_LEN];
} WatchMap;

#endif // COMMON_H