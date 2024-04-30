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

void sigchld_handler(int signum) ;

void sigint_handler(int signum) ;
char *read_input() ;
char **tokenize_input(char *input, int *arg_size) ;
void exec_cd(char **args);
void exec_jobs(char **args);

void execute_builtin(char **argv) ;
typedef void (*shell_func)(char **args);

shell_func shell_funcs[] = {&exec_cd, &exec_jobs};

char *builtin[] = {"cd", "jobs"};

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
                memmove(&background_jobs[i], &background_jobs[i + 1], (num_background_jobs - i - 1) * sizeof(BackgroundJob));
                num_background_jobs--;
                break;
            }
        }
    }
}

void sigint_handler(int signum){
    if (num_background_jobs != 0) {
        printf("sig_int(%d):background process pending, cant exit\n", signum);
    }else{
//        free(background_jobs);
        printf("sig_int(%d): ended!\n",signum);
        exit(0);
    }

}

// Execute commands
void execute_command(char **args, int background) {
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
            BackgroundJob job = {pid, num_background_jobs + 1, ""};
            strncpy(job.command, args[0], MAX_COMMAND_LENGTH);
            background_jobs[num_background_jobs++] = job;
            printf("[%d] %d\t%s\n", job.job_id, job.pid, job.command);
        }
    }
}

char *read_input() {
    char *buffer;
    size_t bufsize = 32;
    ssize_t characters;

    buffer = (char *)malloc(bufsize * sizeof(char));
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

    printf("> ");
    characters = getline(&buffer, &bufsize, stdin);
    return buffer;
}

char **tokenize_input(char *input, int *arg_size) {
    int i = 0;
    char **args = (char **)malloc((MAX_ARGUMENTS + 1) * sizeof(char *));
    if (args == NULL) {
        perror("Unable to allocate memory for arguments");
        exit(1);
    }

    args[i] = strtok(input, " \n");
    while (args[i] != NULL && i < MAX_ARGUMENTS) {
        i++;
        args[i] = strtok(NULL, " \n");
    }
    args[i] = NULL;
    *arg_size = i;
    return args;
}

void exec_cd(char **argv) {
    if (argv[1] == NULL) {
        fprintf(stderr, "cd: missing argument\n");
        return;
    }
    if (chdir(argv[1]) != 0) {
        perror("cd error");
    }
}

void exec_jobs(char **argv) {
    // List background jobs
    int j;
    for (j = 0; j < num_background_jobs; ++j) {
        printf("[%d] %d\t%s\n", background_jobs[j].job_id, background_jobs[j].pid, background_jobs[j].command);
    }
}

void execute_builtin(char **argv) {
    size_t len = sizeof(builtin) / sizeof(char *);
    for (size_t i = 0; i < len; i++) {
        if (strcmp(argv[0], builtin[i]) == 0) {
            shell_funcs[i](argv);
            return;
        }
    }
}

int main() {
    char *input;
    char **args;
    int arg_size;
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT,sigint_handler);
    while (1) {
        input = read_input();
        args = tokenize_input(input, &arg_size);

        execute_builtin(args);

        int background = 0;

        if (arg_size > 0 && strcmp(args[arg_size - 1], "&") == 0) {
            // Background job
            background = 1;
            args[arg_size - 1] = NULL;
            arg_size--; // Update argument count
        }
        execute_command(args, background);

        free(input);
        free(args);
    }
    return 0;
}
