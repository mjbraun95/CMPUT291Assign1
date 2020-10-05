// CMPUT 391 Assignment #1
// Matthew Braun
// 1497171

#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/wait.h>

struct process_info_s process_info;
struct process process_table[MAX_PT_ENTRIES];
char command_buffer[LINE_LENGTH + 1];
bool command_invalid = false;
bool ignore_prompt;


struct user_instruction {
    char* command;
    // Command + arguments + NULL termination
    char* arguments[MAX_ARGS + 2];
    char* input;
    char* output;
    bool background;
};

struct cmd {
    const char* cmdstr;
    void (*handler)(int);
};

#define CMD_HANDLER(cmd, fn) {.cmdstr = cmd, .handler = fn}
static struct cmd commands[] = {
    CMD_HANDLER("exit", exit_handler),
    CMD_HANDLER("jobs", jobs_handler),
    CMD_HANDLER("kill", kill_handler),
    CMD_HANDLER("resume", resume_handler),
    CMD_HANDLER("sleep", sleep_handler),
    CMD_HANDLER("suspend", suspend_handler),
    CMD_HANDLER("wait", wait_handler),
};

static void manage_processes(void);
bool check_for_shell_commands(struct user_instruction inst);
struct user_instruction parse_input_string_to_user_instruction(char* inputString);
int find_empty_process(void);

int main(int argc, char** argv) {
    (void) argc;
    (void) argv;

    while (1) {
        // Ensure all finished children are cleaned up and accounted for in time.
        // This lets us ignore dealing with this in the task handlers.
        char inputString [MAX_LENGTH];
        if (!ignore_prompt) {
            printf("SHELL379: ");
        }
        fgets(inputString, MAX_LENGTH, stdin);

        // Check for line too long
        int in_len = strlen(inputString);
        int buf_len = strlen(command_buffer);
        ignore_prompt = false;
        if (command_invalid) {
            if (inputString[in_len - 1] == '\n') {
                command_invalid = false;
                command_buffer[0] = '\0';
            } else {
                ignore_prompt = true;
                continue;
            }
        } else if ((buf_len + in_len) >= LINE_LENGTH) {
            printf("Line too long! (Max %d characters)\n", LINE_LENGTH);
            if (inputString[in_len - 1] != '\n') {
                command_invalid = true;
            } else {
                command_buffer[0] = '\0';
            }
            ignore_prompt = true;
            continue;
        }

        // Copy in chunks of lines
        strcpy(&command_buffer[buf_len], inputString);

        // Don't read unfinished lines
        if (command_buffer[buf_len + in_len - 1] != '\n') {
            ignore_prompt = true;
            continue;
        }
        manage_processes();

        // Ignore empty commands
        if (strlen(inputString) <= 1) {
            continue;
        }
        printf("\n");

        struct user_instruction inst = parse_input_string_to_user_instruction(command_buffer);
        bool noneOfTheAboveCommands = check_for_shell_commands(inst);
        
        if (noneOfTheAboveCommands == true) {
            int lowestUnusedProcess = find_empty_process();
            if (lowestUnusedProcess == -1) {
                printf("Process table full! Can't spawn new process");
                continue;
            }
            (void) process_spawn(&process_table[lowestUnusedProcess], inst.command, inst.arguments, inst.input, inst.output);
            // Wait for non-backgrounded tasks
            if (!inst.background) {
                waitpid(process_table[lowestUnusedProcess].pid, NULL, 0);
            }
        }
        // Reset buffer
        command_buffer[0] = '\0';
    }
}

struct user_instruction parse_input_string_to_user_instruction(char* inputString) {
    struct user_instruction inst;
    inst.command = NULL;
    for (int i=0; i<MAX_ARGS+1; i++) {
        inst.arguments[i] = NULL;
    }
    inst.input = NULL;
    inst.output = NULL;
    inst.background = false;
    char* words;
    words = strtok(inputString, "\n");
    words = strtok(words, " ");
    inst.command = words;
    inst.arguments[0] = inst.command;
    for (int i=1; i<MAX_ARGS; i++) {
        words = strtok(NULL, " ");
        if (words == NULL) {
            break;
        }
        if (words[0] != '<' && words[0] != '>' && words[0] != '&' && words[0] != '\0') {
            inst.arguments[i] = words;
        }
        else {
            if (words[0] == '<') {
                inst.input = &words[1];
                words = strtok(NULL, " ");
            } else if (words[0] == '>') {
                inst.output = &words[1];
                words = strtok(NULL, " ");
            } else if (words[0] == '&') {
                inst.background = true;
            }
        }
    }
    return inst;
}

bool check_for_shell_commands(struct user_instruction inst) {
    bool noneOfTheAboveCommands = true;
    for (int i=0; i < sizeof(commands)/sizeof(*commands); i++) {
        //if 1st argument matches a command
        if (strcmp(inst.command, commands[i].cmdstr) == 0) {
            // Attempt to parse integer argument
            int arg = 0;
            if ((inst.arguments[1]) != NULL) {
                arg = atoi(inst.arguments[1]);
            }
            // Run handler
            commands[i].handler(arg);
            noneOfTheAboveCommands = false;
            break;
        }
    }
    return noneOfTheAboveCommands;
}

int find_empty_process(void) {
    for (int i = 0; i < MAX_PT_ENTRIES; i++) {
        if (process_table[i].status == UNUSED) {
            return i;
        }
    }
    return -1;
}

static void manage_processes(void) {
    process_info.running_processes = 0;
    for (int i = 0; i < MAX_PT_ENTRIES; i++) {
        if (process_table[i].status == RUNNING) {
            if (process_is_running(&process_table[i]) == 1) {
                process_info.running_processes++;
            }
        }
    }
}
