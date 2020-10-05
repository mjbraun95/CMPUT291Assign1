// CMPUT 391 Assignment #1
// Matthew Braun
// 1497171

#ifndef SHELL_H_
#define SHELL_H_

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define LINE_LENGTH 100
#define MAX_ARGS 7
#define MAX_LENGTH 20
#define MAX_PT_ENTRIES 32

enum process_status {
    UNUSED,
    RUNNING,
};

struct process {
    enum process_status status;
    bool suspended;
    int pid;
};

struct process_info_s {
    clock_t systime;
    clock_t usertime;
    unsigned int running_processes;
};

// Process Management structure declarations
extern struct process_info_s process_info;
extern struct process process_table[MAX_PT_ENTRIES];

// System process management
int process_spawn(struct process* process, char* cmd, char* args[],
                  char* input, char* output);
int process_is_running(struct process* process);

// Shell command handler functions
void exit_handler(int);
void jobs_handler(int);
void kill_handler(int);
void resume_handler(int);
void sleep_handler(int);
void suspend_handler(int);
void wait_handler(int);


#endif // SHELL_H_