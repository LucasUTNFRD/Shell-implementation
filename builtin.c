#include "builtin.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to HOME)
//  it has to be executed and then return true
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']

int cd(char *cmd) {
  char *home = getenv("HOME");
  char aux_cmd[strlen(cmd)];
  strcpy(aux_cmd, cmd);
  // we know that cd is follow by space so we split and righ side is our
  // directory
  char *directory = split_line(aux_cmd, ' ');
  char *prefix = "~/";
  char *new_prompt;
  if (strcmp(aux_cmd, "cd") == 0) {
    if (strlen(directory) != 0) {
      if (chdir(directory) != 0) {
        printf("cd: %s: directory does not exist\n", directory);
      } else {
        new_prompt = (char *)malloc(strlen(directory + 2));
        strcpy(new_prompt, prefix);
        strcat(new_prompt, directory);
        strcpy(prompt, new_prompt);
        free(new_prompt);
      }
    } else {
      chdir(home);
      strcpy(home, prompt);
    }
    return true;
  }
  return 0;
}

int exit_shell(char *cmd) {
  if (strcmp(cmd, "exit") == 0) {
    return true;
  }
  return 0;
}
