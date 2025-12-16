#ifndef SIGNALS_H
#define SIGNALS_H

#include "common.h"
#include "test.h"

// Zmienna globalna musi być extern w nagłówku
extern volatile sig_atomic_t last_signal;

void sethandler(void (*f)(int), int sigNo);
void sig_handler(int sig);
void sigchld_handler();
int should_stop();

pid_t receive_pid(pid_source_target *tablica, const char *source_path, const char *target_path);
void create_child(char source_path[], char target_path[], pid_source_target pid_archived[],  pid_source_target** cursor);
void child_work(char source_path[], char target_path[]);

#endif