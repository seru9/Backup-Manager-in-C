#ifndef FILE_OPS_RESTORE_H
#define FILE_OPS_RESTORE_H
#include "common.h"
#include "file_ops.h"

void perform_restore(char *source_path, char *target_path);

// // Funkcje pomocnicze
void delete_extras(char *source_root, char *target_root);
void recursive_restore(char *curr_source, char *curr_backup, char *source_root, char *backup_root);
int remove_recursive(char *path);

#endif