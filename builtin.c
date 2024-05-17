#include "builtin.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int cd(Command *cmd) {
  char *home = getenv("HOME");
  if (strcmp(cmd->name, "cd") == 0) {
    if (cmd->num_args == 1) {
      chdir(home);
    } else {
      if (chdir(cmd->args[1]) != 0) {
        perror("cd error");
      }
    }
  }
  return 0;
}

int exit_shell(Command *cmd) {
  if (strcmp(cmd->name, "exit") == 0) {
    return true;
  }
  return 0;
}
