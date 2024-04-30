#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGUMENTS 10

// Struct to store information about background jobs
typedef struct {
    pid_t pid;
    int job_id;
    char command[MAX_COMMAND_LENGTH];
} BackgroundJob;

BackgroundJob background_jobs[MAX_ARGUMENTS];
int num_background_jobs = 0;

// Signal handler for SIGCHLD (child process termination)
void sigchld_handler(int signum) {
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        int i;
        for (i = 0; i < num_background_jobs; ++i) {
            if (background_jobs[i].pid == pid) {
                printf("[%d] Done\t%s\n", background_jobs[i].job_id, background_jobs[i].command);
                // Remove the completed job from the list
                memmove(&background_jobs[i], &background_jobs[i+1], (num_background_jobs - i - 1) * sizeof(BackgroundJob));
                num_background_jobs--;
                break;
            }
        }
    }
}

// Execute commands
void execute_command(char *args[], int background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        if (!background) {
            // Wait for foreground process to finish
            waitpid(pid, NULL, 0);
        } else {
            // Background process
            BackgroundJob job = { pid, num_background_jobs + 1, "" };
            strncpy(job.command, args[0], MAX_COMMAND_LENGTH);
            background_jobs[num_background_jobs++] = job;
            printf("[%d] %d\t%s\n", job.job_id, job.pid, job.command);
        }
    }
}

// Main loop
int main() {
    char input[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGUMENTS + 1];
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        printf("$ ");
        fflush(stdout);
        if (!fgets(input, MAX_COMMAND_LENGTH, stdin)) {
            break;
        }

        // Tokenize input
        int i = 0;
        args[i] = strtok(input, " \n");
        while (args[i] != NULL && i < MAX_ARGUMENTS) {
            i++;
            args[i] = strtok(NULL, " \n");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            // Empty command
            continue;
        }

        // Check for built-in commands
        if (strcmp(args[0], "exit") == 0) {
            break;
        } else if (strcmp(args[0], "jobs") == 0) {
            // List background jobs
            int j;
            for (j = 0; j < num_background_jobs; ++j) {
                printf("[%d] %d\t%s\n", background_jobs[j].job_id, background_jobs[j].pid, background_jobs[j].command);
            }
        } else {
            int background = 0;
            if (args[i - 1] != NULL && strcmp(args[i - 1], "&") == 0) {
                // Background job
                background = 1;
                args[i - 1] = NULL;
            }
            execute_command(args, background);
        }
    }

    return 0;
}

