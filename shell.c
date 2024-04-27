#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"
#include <signal.h>

//global vars
pid_t *background_process_list = NULL;
pid_t list_size = 0;

void add_background_process(pid_t process){
  
  background_process_list = realloc(background_process_list,(list_size+1) * sizeof(pid_t));
  background_process_list[list_size++] = process;
}

void free_background_process(void){
  free(background_process_list);
}
//when removed val is setted to 0, ideally we should shift array_to left when remove
void remove_background_process(pid_t process){
//find pid
  for(size_t i = 0; i<list_size ; i++){
    if (background_process_list[i] == process) {
      for(size_t j = i;j<list_size-1;j++){
        background_process_list[j]=background_process_list[j+1];
      }
      //decrease size of dynamic array;
      background_process_list = realloc(background_process_list,(list_size-1) * sizeof(pid_t));
      list_size--;
      break;
    }
  }
}

shell_func shell_funcs[] = {&sh_exec_cd, &sh_exec_help, &sh_exec_exit};

char *builtin[] = {"cd","help","exit"};


static void sh_init(void){
  char *input;
  char **args; //array of char pointers
  int status;
  int background;
  do {
    printf("> ");
    input = sh_read_input();
    args = sh_tokenize(input,&background);
    // status = sh_exec(args);
    status = sh_exec_builtins(args);

    free(input);/*free input buffer */ 
    free(args);/*free arg vector */
  }while (status);

}

char *sh_read_input(void){
    char *buffer;
    size_t bufsize = 32;
    ssize_t characters;

    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    //
    characters = getline(&buffer,&bufsize,stdin);
    return buffer;
};

//this functions check if last token is & or in last token the last char is &
int isBackground(char **args,size_t count){
  char *lastToken = args[count-1] ;
  size_t size = strlen(lastToken);
  return (strcmp(lastToken,"&")==0 || lastToken[size-1]=='&') ? 1 : 0;
}
//
char **sh_tokenize(char *input,int *background){
  //save each char*
  char *token;
  size_t count = 0;
  char **argv = NULL;

// Each call to strtok() returns a pointer to a null-terminated string  contain‐
//        ing the next token.  This string does not include the delimiting byte.  If no
//        more tokens are found, strtok() returns NULL.
  token = strtok(input,TOK_DELIM);
  while(token != NULL){
    //while there is a token reallocate space in memory to save the token
    argv = realloc(argv,(count+1) * sizeof(char*));
    //en each array index allocate memory for saving the read token
    argv[count] = malloc(sizeof(char)*(strlen(token)+1));
    //put token in argv position
    strcpy(argv[count],token);
    //tracin if parse was ok 
    // printf("%s\n",token);
    //make token read a null to read next token;
    token=strtok(NULL,TOK_DELIM);
    //increment argv size
    count++;
  }
  *background = isBackground(argv, count);
  // printf("%d\n",*background);
  return  argv;
}

int sh_exec(char **argv){
  pid_t pid,pid_child;
  int status; //0 indicates succesfull termination, non zero error.
    
  pid = fork();
  
  switch (pid) {
    case -1:
      perror("fork error");
      break;
    case 0:
      /*child process */
      if(execvp(argv[0],argv)==-1){
        perror("exec error");
      }
      exit(EXIT_FAILURE);     
      break;
    default:
      /*parent process */
      status = 0;
      pid_child = waitpid(pid, &status, 0);
      break; 
  
  }
  return 1;
}


//use a function pointer to execute builtin command

int sh_exec_builtins(char **argv){
  size_t len = sizeof(builtin) / sizeof(char *);
  for (size_t i = 0; i<len;i++) {
    if(strcmp(argv[0],builtin[i])==0) return shell_funcs[i](argv); 
  }

  return sh_exec(argv);
}

int sh_exec_cd(char **argv){
  if(chdir(argv[1])!=0){
    perror("cd error");
  }
  return 1;
}

int sh_exec_help(char **argv){
  size_t len = sizeof(builtin)/sizeof(char*);
  printf("shell made by Lucas Delgado\n");
  printf("--------built-in commands--------\n");
  for(size_t i =0 ;i<len;i++){
    printf("%d_  %s\n",i+1,builtin[i]);
  }
  return 1;
}

int sh_exec_exit(char **argv){
  return 0;
}

int main(void)
{ 
  sh_init();
  
  return 0;
}
