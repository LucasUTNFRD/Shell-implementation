#ifndef BUILTIN_H
#define BUILTIN_H

#include "shell.h"

extern char prompt[PROMPT_LEN];

int cd(char *cmd);

int exit_shell(char *cmd);

#endif // !BUILTIN_H
