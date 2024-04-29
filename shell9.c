/*
 * shell1.c
 * 
 * Copyright 2024 osboxes <osboxes@osboxes>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * shell primitivo en foreground y background
 * soporta argumentos en los comandos y cualquier cantidad de espacios
 * intermedios entre los argumentos del comando
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define LOOP_JROMER 1
#define LOOP_GRCHERE strcmp(comando,"fin") != 0

//funciones
void ingreso_comando(const char *prompt,char *cmd,int largo_cmd);
int parsing_comando(char *cmd, char *arg[],int largo_arg);
void trace_parsing(char *arg[],int largo_arg);
void libero_parsing(char *arg[],int largo_arg);
void ejecuto(char *arg[],int largo_arg,int foreground);  // ejecuta comando y queda a la espera de la finalizacion del proceso
void chequeo_fin_proceso();
void agrego_pid(pid_t p);
void quito_pid(pid_t p);
int listar_pid();

//funciones para el manejo de se#ales  
void sig_child(int);
void sig_int(int);
void sig_alarma(int);
void bloqueo(int);
void desbloqueo(int);

//datos globales
pid_t *pids = NULL;
int npids = 0;

int main(int argc, char **args) {
	char comando[256];
	char *argv[256];
	// voy a ejecutar a sig_child() cada vez que reciba la senal SIGCHLD
	signal(SIGCHLD,sig_child);
	signal(SIGINT,sig_int);
	signal(SIGALRM,sig_alarma);
	alarm(10);
	do {
		ingreso_comando("$$>",comando,256);
		int n = parsing_comando(comando,argv,256);
		//trace_parsing(argv,n);

		//si no hay comando, vuelvo al prompt
		if ( n == 0 ) continue; // presiono ENTER
		
		if ( strcmp(argv[0],"cd") == 0 ) { // comando interno cd
			if ( argv[1] == NULL ) {
				printf("debe indicar directorio!\n");
				continue;
			}
			chdir(argv[1]);
		} else if ( strcmp(comando,"fin") != 0 ) {
			// es un comando a ejecutar!
			
			if ( n > 1 && strcmp(argv[n-1],"&") == 0 ) {
				free(argv[n-1]);
				argv[n-1]=NULL;
				n--;
				ejecuto(argv,n,0); // ejecuto background
			} else ejecuto(argv,n,1); // ejecuto foreground
		}
	} while(LOOP_GRCHERE);
	while( listar_pid() ) {
		printf("hay procesos pendientes, no puede salir!\n");
		pause();
	}
	// libero arreglo global
	free(pids);
	return 0;
}

// implemento funciones

void ingreso_comando(const char *prompt,char *comando,int largo_cmd) {
	printf("%s",prompt);
	fgets(comando,largo_cmd,stdin);
	comando[strlen(comando)-1]='\0'; // reemplaza \n por \0
}

/**
 * cmd comando a parsear, cada comando se separa por espacios
 * arg[] arreglo de punteros a char * por cada parte del comando
 * largo_arg cantidad de elementos que tiene el arreglo arg
 * Devuelve la cantidad de elementos cargados en arg[]
 */
int parsing_comando(char *comando, char *argv[],int largo_arg) {
	int i,ii=0,z=0;
	i=0;
	memset(argv,0,sizeof(char *)*largo_arg);
	//for(w=0;w<largo_arg;w++) argv[w]=NULL;

	// salteo los espacios que separan a los argumentos del comando
	while(comando[i] && comando[i] == ' ') i++;

	while(comando[i] && z < largo_arg) {
		char palabra[256];
		ii=0;
		while(comando[i] && comando[i] != ' ' && ii < 256) {
			palabra[ii] = comando[i];
			i++;ii++;
		}
		palabra[ii]='\0';
		argv[z] = strdup(palabra);
		z++;
		// salteo los espacios que separan a los argumentos del comando
		while(comando[i] && comando[i] == ' ') i++;
	}
	argv[z] = NULL;
	return z;
}

void trace_parsing(char *argv[],int largo_arg) {
	// trace, chequeo que funciona el parsing
	int w;
	for(w=0;w<largo_arg && argv[w];w++) printf("argv[%d]=[%s]\n",w,argv[w]);
}

/*
 * libero memoria apuntada por cada elemento no nulo de argv[]
 * previamente asignado por strdup()
 */
void libero_parsing(char *argv[],int largo_arg) {
	int w;
	// libero la memoria solicitada por strdup()
	for(w=0;w<largo_arg && argv[w];w++) if (argv[w]) free(argv[w]);
}

// ejecuta comando y el proceso padre NO queda a la espera del proceso hijo
// debido a que implemento SIGCHLD 
void ejecuto(char *argv[],int largo_arg,int foreground) {
	pid_t pid = fork();
	if ( pid == 0 ) {
		// proceso hijo
		// exec
		signal(SIGCHLD,SIG_DFL);
		signal(SIGINT,SIG_DFL);
		signal(SIGALRM,SIG_DFL);
		
		int rc = execvp(argv[0],argv);
		printf("ejecuto(): comando [%s] no existe!!\n",argv[0]);

		// libero la memoria solicitada por strdup()
		libero_parsing(argv,largo_arg);
		exit(rc);

	} else {

		if ( foreground ) {
			bloqueo(SIGCHLD);
				int estado=0;
				pid_t pidfin = waitpid(pid,&estado,0);
				printf("ejecuto(): PID %d finalizo con estado %d\n",pidfin,WEXITSTATUS(estado));
			desbloqueo(SIGCHLD);
		} else {
			agrego_pid(pid);
			printf("ejecuto(): proceso PID %d creado!\n",pid);
		}
		// libero la memoria solicitada por strdup()
		libero_parsing(argv,largo_arg);
	}
} 

// handle de la se#al SIGCHLD
void sig_child(int signo) {
	chequeo_fin_proceso();
}

// handle de la se#al SIGINT
void sig_int(int signo) {
	printf("sig_int(%d): enviaron se#al de interrupcion\n",signo);
	chequeo_fin_proceso();
	if ( listar_pid() ) {
		printf("sig_int(%d): hay procesos pendientes, no puede salir!\n",signo);
		return;
	} else {
		free(pids);
		printf("sig_int(%d): fin!\n",signo);
		exit(0);
	}
}

// handle de la se#al SIGALRM
void sig_alarma(int signo) {
	listar_pid();
	alarm(10);
}
/*
 * funcion llamada desde handler SIGCHLD
 * no utilizar wait(), no usar funciones bloqueantes, pues se ejecuta en el contexto
 * de una interrupcion
 */
void chequeo_fin_proceso() {
	// espero por cada uno de los procesos hijos finalizados
	int estado=0;
	pid_t pid_hijo=0;
	while ((pid_hijo = waitpid(-1,&estado,WNOHANG)) > 0) {
		printf("chequeo_fin_proceso(): PID %d finalizado con retorno %d\n",pid_hijo,WEXITSTATUS(estado));
		quito_pid(pid_hijo);
		estado=0;
	}
}
// agrega p al arreglo apuntado por pids, reusa el lugar de un pid que esta en cero, sino agranda el arreglo apuntado por pids
void agrego_pid(pid_t p) {
	if ( npids == 0 ) {
		pids = (pid_t *) malloc(sizeof(pid_t));
		*pids=p;
	} else {
		pid_t *pp = pids;
		int i;
		for(i=0;i<npids;i++,pp++) {
			if ( *pp == 0 ) {
				*pp = p;
				break;
			}
		}
		if ( i < npids ) return;
		// debo ampliar el arreglo
		pids = (pid_t *) realloc(pids,sizeof(pid_t)*(npids+1));
		*(pids+npids)=p;
	}
	npids++;
}
// elimina a p (lo pone en cero) en el arreglo apuntado por pids
void quito_pid(pid_t p) {
	pid_t *pp = pids;
	int i;
	for(i=0;i<npids;i++,pp++) {
		if ( *pp == p ) {
			*pp = 0;
			break;
		}
	}
}
// recorre el arreglo apuntado por pids y muestra todo PID que no sea cero
int listar_pid() {
	pid_t *pp = pids;
	int i,n=0;
	for(i=0;i<npids;i++,pp++) {
		if ( *pp ) {
			printf("listar_pid(): PID %d en ejecucion!\n",*pp);
			n++;
		}
	}
	return n;
}

/*
 * bloqueo la senal signal
 */
void bloqueo(int signal) {
	sigset_t mascara;
	sigemptyset(&mascara);
	sigaddset(&mascara,signal);
	sigprocmask(SIG_BLOCK,&mascara,NULL);
}
/*
 * desbloqueo la senal signal
 */
void desbloqueo(int signal) {
	sigset_t mascara;
	sigemptyset(&mascara);
	sigaddset(&mascara,signal);
	sigprocmask(SIG_UNBLOCK,&mascara,NULL);
}
