#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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

void sh_init(void){
  char *input;
  char **args; //array of char pointers
  int status;
  do {
    printf("> ");
    input = sh_read_input();
    args = sh_tokenize(input);
    status = sh_exec(args);

    free(input); 
    free(args);
  }while (1);

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

char **sh_tokenize(char *input){
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
  return  argv;
}

int sh_exec(char **argv){
  pid_t pid,pid_child;
  int status; 
  pid = fork();
  if (pid == 0) {/*child process */
    if(execvp(argv[0],argv)==-1){
      perror("exec error");
    }
    exit(EXIT_FAILURE);
  }else if (pid < 0) {/*error in fork */
    perror("fork error");
  }else { /*SHELL PROCESS */
    status = 0;
    pid_child =wait(&status);
		// printf("PID %d finalizado con retorno %d\n",pid_child,status/256);
  }
  return 1;
}



int main(int argc, char *argv[])
{ 
  sh_init();
  return 0;
}
