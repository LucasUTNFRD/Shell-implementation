#include "shell.h"
#include "builtin.h"
#include "job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wait.h>

Jobs_table *jobs;

void sigchld_handler(int signum) {
  pid_t pid;
  int status;

  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    // Find the job in the jobs_table and remove it
    for (size_t i = 0; i < jobs->counter; ++i) {
      if (jobs->list[i].pid == pid) {
        printf("Background job [%d] %s finished\n", jobs->list[i].job_id,
               jobs->list[i].command);
        // printf("%s\n", prompt);
        remove_job(jobs, jobs->list[i]);
        break;
      }
    }
  }
}

//[username@domain directory]$
char prompt[PROMPT_LEN] = {0};

// Var used for reading line from user
static char buffer[BUFF_LEN];

int status = 0;

// read a line from the standar input
// and prints the prompt
char *read_line(const char *prompt) {
  int i = 0;
  int c = 0;
  fprintf(stdout, "%s\n", prompt);
  fprintf(stdout, "%s", "$ ");

  memset(buffer, 0, BUFF_LEN);

  c = getchar();
  while (c != END_LINE && c != EOF) {
    buffer[i++] = c;
    c = getchar();
  }

  if (c == EOF) {
    return NULL;
  }

  buffer[i] = END_STRING;

  return buffer;
}

Command *create_command(char *name, char **args, int num_args, int background) {
  Command *cmd = malloc(sizeof(Command));
  if (cmd) {
    cmd->name = strdup(name);
    cmd->args = args;
    cmd->num_args = num_args;
    cmd->background = background;
  }

  return cmd;
}

void print_command(Command *cmd) {
  printf("Command name:%s\n", cmd->name);
  printf("Num of args: %d\n", cmd->num_args);
  printf("is background: %d\n", cmd->background);
}

void free_command(Command *cmd) {
  if (cmd) {
    free(cmd->name);
    free(cmd->args); // Free memory for args array
    free(cmd);
  }
}

// this function is using wrongly prompt
// it should be using buffer.
char **tokenize_input(char *input, int *num_args) {}

//*
// ex  ={'s','l','e','e','p','&'}
// ex  ={'s','l','e','e','p',' ',&'}
//*/
int isBackground(char **args, int argc) {
  if (argc > 0 && strcmp(args[argc - 1], "&") == 0) {
    args[argc - 1] = NULL; // remove the ampersand so it could be executed.
    return BG;
  } else {
    return FG;
  }
}

Command *parse_line(char *input) {
  int i = 0;
  char **argv = (char **)malloc((MAXARGS + 1) * sizeof(char *));

  if (argv == NULL) {
    perror("malloc failed\n");
    exit(1);
  }
  char *token = strtok(input, DELIM);
  while (token != NULL && i < MAXARGS) {
    argv[i] = token;
    // printf("token traced: %s\n", args[i]);
    i++;
    token = strtok(NULL, DELIM);
  }
  argv[i] = NULL;
}
// Command *parse_line(char *input) {
//   int num_args;
//   char **args = tokenize_input(input, &num_args);
//   char *name = args[0];
//   int background = isBackground(args, num_args);
//   char *output_file;
//   char *input_file;
//   Command *cmd = create_command(name, args, num_args, background);
//   return cmd;
// }

int run_command(char *cmd) {
  if (strlen(cmd) == 0) {
    return 0;
  }

  if (exit_shell(cmd)) {
    return EXIT_SHELL;
  }
  if (cd(cmd)) {
    return 0;
  }

  if (exec_jobs(cmd)) {
    print_jobs(jobs);
    return 0;
  }

  Command *command = parse_line(cmd);
  // print_command(command);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  } else if (pid == 0) {
    // child process
    // I/O redirection logic
    if (command->input_file != NULL) {
    }
    if (command->output_file != NULL)
      // exectute the Command
      if (execvp(command->name, command->args) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
      }
  } else {
    // parent process
    if (command->background != BG) {
      int status;
      waitpid(pid, &status, 0);
    } else {
      // logic for jobs table
      Job_t job = {pid, jobs->counter + 1, strdup(command->name)};
      add_job(jobs, job);
      printf("$ Background job [%d] %d\n", job.job_id, job.pid);
    }
  }
  free_command(command);
  return 0;
}

static void run_shell() {
  char *cmd;
  while ((cmd = read_line(prompt)) != NULL) {
    if (run_command(cmd) == EXIT_SHELL)
      return;
  }
}
// this function inits shell in $HOME directory
static void init_shell() {
  char buf[BUFF_LEN];
  char *home = getenv("HOME");

  if (chdir(home) < 0) {
    snprintf(buf, sizeof buf, "cannot cd to %s ", home);
    perror(buf);
  } else {
    snprintf(prompt, sizeof prompt, "(%s)", home);
  }
  // initalize job lists
  jobs = init_jobs();
  signal(SIGCHLD, sigchld_handler);
}

int main(void) {
  init_shell();
  run_shell();
  free_jobs(jobs);
  return 0;
}
