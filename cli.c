// CMPUT 391 Assignment #1
// Matthew Braun
// 1497171

#include "shell.h"

#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/times.h>

/// Find process which is running, return NULL if none found.
struct process* is_process_running(int pid) {
    printf("pid: %d\n", pid);
    for (int i = 0; i < MAX_PT_ENTRIES; i++) {
        printf("status: %d, pid: %d\n", process_table[i].status, process_table[i].pid);
        if (process_table[i].status == RUNNING && process_table[i].pid == pid) {
            return &process_table[i];
        }
    }

    return NULL;
}

void exit_handler(int arg) {
    (void) arg;
    for (int i = 0; i < MAX_PT_ENTRIES; i++) {
        if (process_table[i].status == RUNNING) {
            kill_handler(process_table[i].pid);
        }
    }
    
    exit(0);
}

void jobs_handler(int arg) {
    (void) arg;

    printf("Running Processes:\n");
    int pid = fork();
    if (pid < 0) {
        // Error forking
        perror("Could not fork for process info");
        return;
    } else if (pid == 0) {
        char* exec_args[] = {"ps", "-ro", "pid,state,etimes,command", NULL};
        execvp("ps", exec_args);
        perror("Failed to execute ps");
        return;
    } else {
        // Wait for 'ps' to finish
        waitpid(pid, NULL, 0);
    }

    // Get child process times
    struct tms tm_data;
    times(&tm_data);
    int usertime = tm_data.tms_cutime / 100;
    int systime = tm_data.tms_cstime / 100;

    printf("Processes =     %i active\n"
           "Completed Processes:\n"
           "User time =     %i seconds\n"
           "Sys  time =     %i seconds\n", 
           process_info.running_processes, usertime, systime);
}

void kill_handler(int pid) {
    if (is_process_running(pid)) {
        int rc = kill(pid, SIGKILL);
        if (rc == 0) {
            printf("Process %i killed.\n", pid);
        } else {
            perror("Failed to kill process");
        }
    } else {
        fprintf(stderr, "Process %i not found in table!\n", pid);
    }
}

void resume_handler(int pid) {
    struct process* p = is_process_running(pid);
    if (p) {
        if (p->suspended) {
            int rc = kill(pid, SIGCONT);
            if (rc == 0) {
                printf("Process %i resumed.\n", pid);
                p->suspended = false;
            } else {
                perror("Failed to resume process");
            }
        } else {
            printf("Process %i is currently running...\n", pid);
        }
    } else {
        fprintf(stderr, "Process %i not found in table!\n", pid);
    }
}

void sleep_handler(int secs) {
    sleep(secs);
}

void suspend_handler(int pid) {
    struct process* p = is_process_running(pid);
    if (p) {
        if (!p->suspended) {
            int rc = kill(pid, SIGSTOP);
            if (rc == 0) {
                printf("Process %i suspended.\n", pid);
                p->suspended = true;
            } else {
                perror("Failed to suspend process");
            }
        } else {
            printf("Process %i is currently suspended...\n", pid);
        }
    } else {
        fprintf(stderr, "Process %i not found in table!\n", pid);
    }
}

void wait_handler(int pid) {
    // Cleanup is handled by main loop
    if (is_process_running(pid)) {
        printf("Waiting on PID %i...\n", pid);
        waitpid(pid, NULL, 0);
    } else {
        printf("PID %i is not running\n", pid);
    }
}
