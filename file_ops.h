#ifndef FILE_OPS_H
#define FILE_OPS_H

#include "common.h"
#include "parser.h"

#define IS_TARGET_PATH 0
#define IS_SOURCE_PATH 1
#define IS_BACKUP_PATH 2

void make_copy(char source_path[], char target_path[]);
void copy_reg_file(const char *src_path, const char *dst_path);
void handle_symlink(char *src_path, char *dst_path, char *root_src, char *root_dst);
void recursive_copy_worker(char *current_src, char *current_dst, char *root_src, char *root_dst);
ssize_t bulk_write(int fd, char *buf, size_t count);
ssize_t bulk_read(int fd, char *buf, size_t count);
void createDir(const char *path);
int check_path(char* input_path, int is_source_path); //sprawdza własności poszczególnych ścieżek
int check_source_target(const char* source_path, const char* target_path, pid_source_target pid_archived[], int pid_archived_size);
//sprawdza czy są zapewniony edge case'y z polcenie wypunktowane!
#endif