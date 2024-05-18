#include "shell.h"
#include "builtin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

//[username@domain directory]$
char prompt[PROMPT_LEN] = {0};

// Var used for reading line from user
static char buffer[BUFF_LEN];

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
char **tokenize_input(char *input, int *num_args) {
  int i = 0;
  char **args = (char **)malloc((MAXARGS + 1) * sizeof(char *));
  if (args == NULL) {
    perror("malloc failed\n");
    exit(1);
  }
  char *token = strtok(input, DELIM);
  while (token != NULL && i < MAXARGS) {
    args[i] = token;
    // printf("token traced: %s\n", args[i]);
    i++;
    token = strtok(NULL, DELIM);
  }
  args[i] = NULL;
  *num_args = i;
  return args;
}

Command *parse_line(char *input) {
  int num_args;
  char **args = tokenize_input(input, &num_args);
  char *name = args[0];
  Command *cmd = create_command(name, args, num_args, 0);
  return cmd;
}

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

  Command *command = parse_line(cmd);

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return -1;
  } else if (pid == 0) {
    // child process
    // exectute the Command
    if (execvp(command->name, command->args) == -1) {
      perror("execvp");
      exit(EXIT_FAILURE);
    }
  } else {
    if (!command->background) {
      int status;
      waitpid(pid, &status, 0);
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
}

int main(void) {
  init_shell();
  run_shell();
  return 0;
}
