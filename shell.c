#include "shell.h"
// #include <cstddef>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define EXIT_SHELL 1

char prompt[PROMPT_LEN];

static char buffer[BUFF_LEN];

char *read_line(const char *prompt) {
  int i = 0;
  int c = 0;
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
  // for (int i = 0; i < cmd->num_args; i++) {
  //   printf("args[%d] %s\n", i, cmd->args[i]);
  // }
  printf("is background: %d\n", cmd->background);
}

void free_command(Command *cmd) {
  if (cmd) {
    free(cmd->name);
    free(cmd->args); // Free memory for args array
    free(cmd);
  }
}

char **tokenize_prompt(char *prompt, int *num_args) {
  int i = 0;
  char **args = (char **)malloc((MAXARGS + 1) * sizeof(char *));
  if (args == NULL) {
    perror("malloc failed\n");
    exit(1);
  }
  char *token = strtok(prompt, DELIM);
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

Command *parse_line(char *prompt) {
  int num_args;
  char **args = tokenize_prompt(prompt, &num_args);
  char *name = args[0];
  Command *cmd = create_command(name, args, num_args, 0);
  return cmd;
}

int run_command(char *cmd) {
  if (strlen(cmd) == 0) {
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

int main(void) {
  run_shell();
  return 0;
}
