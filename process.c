// CMPUT 391 Assignment #1
// Matthew Braun
// 1497171

#include "shell.h"

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

/// @brief Spawn a new process, running the specified files.
/// Optionally redirect input and output
///
/// @param cmd      C-string of command
/// @param args     NULL terminated list of arguments to give command
/// @param input    File to redirect input from (NULL for none)
/// @param output   File to redirect output to (NULL for none)
///
/// @returns < 0 on failure, 0 on success.
int process_spawn(struct process* process, char* cmd, char* args[],
                  char* input, char* output) {
    char err_buf[100];
    FILE* in_fd;
    FILE* out_fd;

    if (input) {
        in_fd = fopen(input, "r");
        if (in_fd == NULL) {
            sprintf(err_buf, "Error opening %s for input redirection", input);
            perror(err_buf);
            return -1;
        }
    }
    if (output) {
        out_fd = fopen(output, "w");
        if (out_fd == NULL) {
            sprintf(err_buf, "Error opening %s for input redirection", output);
            perror(err_buf);
            return -1;
        }
    }
    
    if (process->status != UNUSED) {
        fprintf(stderr, "Invalid process table entry used!\n");
        return -1;
    }

    printf("Spawning process...\n");
    int pid = fork();
    if (pid < 0) {
        // Error spawning process
        perror("Error spawning new job; PID < 0");
        return pid;
    } else if (pid == 0) {
        // We are in the forked process, change execution

        // Redirect stdin/stdout
        if (input) {
            dup2(fileno(in_fd), fileno(stdin));
        }
        if (output) {
            dup2(fileno(out_fd), fileno(stdout));
        }

        int rc = execvp(cmd, args);
        // exec() fns only return on failure
        perror("Failed to exec process");

        return rc;
    } else {
        printf("Parent process...\n");
        // We have returned to the parent process
        process->pid = pid;
        process->status = RUNNING;
        process->suspended = false;
        return 0;
    }
}

/// @retval 1   Process still running
/// @retval 0   Process has finished
/// @retval < 0 Error
int process_is_running(struct process* process) {
    int status = waitpid(process->pid, NULL, WNOHANG);
    if (status == 0) {
        // Still running
        process->status = RUNNING;
        return 1;
    } else if (status == process->pid) {
        // Process has finished, get time;
        process->status = UNUSED;
        return 0;
    } else {
        process->status = UNUSED;
        return status;
    }
}
