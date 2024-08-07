#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

#define EXIT_SHELL 1
#define PROMPT_LEN 1024
#define BUFF_LEN 1024
#define END_LINE '\n'
#define END_STRING '\0'
#define MAXARGS 20
#define DELIM " \n"
#define SPACE " "
#define DEBUG false // set for debugging

typedef enum { FG, BG } exec_t;

typedef struct {
  char *name;   // Command name
  char **args;  // Command arguments (including command name itself)
  int num_args; // Number of arguments
  // char *input_redirect;  // Input file redirection (if any)
  // char *output_redirect; // Output file redirection (if any)
  exec_t
      background; // Flag indicating if the command should run in the background
  char *input_redirect, *output_redirect;
  int append_output;

} Command;

Command *create_command(char *name, char **args, int num_args,
                        exec_t background, char *input_file,
                        char *input_redirect, int append_output);
Command *parse_line(char *prompt);
char **tokenize_input(char *prompt, int *num_args);
void print_command(Command *cmd);
void free_command(Command *cmd);
void sigchld_handler(int signum);
char *read_line(const char *prompt);
static void run_shell();
int run_command(char *cmd);
#endif // !SHELL_H
