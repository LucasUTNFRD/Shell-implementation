#ifndef SHELL_H
#define SHELL_H



#define  BUFFER_SIZE 1024
#define TOK_DELIM " \t\r\n\a"
//define a buffer for reading user input

//handle user input

//read 
char *sh_read_input(void);
//parse
char **sh_tokenize(char *input);
//execute
int sh_exec(char **argv);

int sh_exec_builtins(char **argv);

int sh_exec_cd(char **args);
int sh_exec_help(char **args);
int sh_exec_exit(char **args);

//definig a pointer function;
typedef int (*shell_func)(char **args);




#endif // !SHELL_H
