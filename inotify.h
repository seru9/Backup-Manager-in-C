#ifndef INOTIFY_H
#define INOTIFY_H

#include "common.h"
#include "signals.h"
#include "file_ops.h"
#include "file_ops_restore.h"
#include "test.h"
extern WatchMap watches[MAX_WATCHERS]; 
extern int watches_count;
int add_watch_to_map(int wd, const char *path);
void add_watch_recursive(int fd, const char *root_path);
char* get_path_by_wd(int wd);
#endif