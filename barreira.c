#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/*--------------------------------------------------------------------
| Types
---------------------------------------------------------------------*/

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int n_espera[2]; //numero fatias a espera
    int n_threads;
    int acumul_flags; //acumulador da flag_desvio
    int terminar; //flag se vai terminar
} Stop;

/*--------------------------------------------------------------------
| Global variables
---------------------------------------------------------------------*/
Stop *stop;

/*--------------------------------------------------------------------
| Function: initBarreira
---------------------------------------------------------------------*/

int initBarreira(int n_threads) {
	stop = (Stop*) malloc(sizeof(Stop));

	if (stop == NULL) {
		fprintf(stderr, "Erro ao alocar struct da barreira\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_mutex_init(&(stop->mutex), NULL) != 0) {
    	fprintf(stderr, "\nErro ao inicializar mutex\n");
    	exit(EXIT_FAILURE);
  	}

  	if(pthread_cond_init(&(stop->condition), NULL) != 0) {
    	fprintf(stderr, "\nErro ao inicializar variável de condição\n");
    	exit(EXIT_FAILURE);
  	}

  	stop->n_espera[0] = 0;
  	stop->n_espera[1] = 0;
  	stop->n_threads = n_threads;
  	stop->acumul_flags = 0;
  	stop->terminar = 0;

  	return 0;
}


/*--------------------------------------------------------------------
| Function: waitBarreira
---------------------------------------------------------------------*/

int waitBarreira(int iter, int flag_desvio) {
	//flag desvio fica 1 se a tarefa puder terminar, 0 se nao
	if (pthread_mutex_lock(&(stop->mutex)) != 0) {
		fprintf(stderr, "Erro ao bloquear mutex\n");
		exit(EXIT_FAILURE);
	}

	stop->n_espera[iter%2] = stop->n_espera[iter%2] +1;

	stop->acumul_flags += flag_desvio;

	if ((stop->n_espera[iter%2]) == stop->n_threads) { //ultima tarefa a chegar a barreira verifica desvio e faz broadcast
		stop->n_espera[iter%2] = 0; //reset do contador de threads na barreira


		if (stop->acumul_flags == stop->n_threads) //se todas as tarefas puderem terminar, acumul_flags vai estar = a n_threads
			stop->terminar = 1;
		stop->acumul_flags = 0; //faz reset ao acumulador

		if (pthread_cond_broadcast(&(stop->condition)) != 0) {
			fprintf(stderr, "Erro no broadcast\n");
			exit(EXIT_FAILURE);
		}
	}
	else { //tarefas 0 a n_threads-1 ficam em espera
		while (stop->n_espera[iter%2] != 0) {
			if (pthread_cond_wait(&(stop->condition), &(stop->mutex)) != 0) {
				fprintf(stderr, "Erro no wait\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (stop->terminar) {
		if (pthread_mutex_unlock(&(stop->mutex)) != 0) {
			fprintf(stderr, "Erro ao bloquear mutex\n");
			exit(EXIT_FAILURE);
		}
		return 1; //retornar 1 para informar a tarefa que deve terminar
	}

	if (pthread_mutex_unlock(&(stop->mutex)) != 0) {
		fprintf(stderr, "Erro ao bloquear mutex\n");
		exit(EXIT_FAILURE);
	}

	return 0; //retornar 0 para continuar a iterar
}

/*--------------------------------------------------------------------
| Function: destroyBarreira
---------------------------------------------------------------------*/

void destroyBarreira() {
  if(pthread_mutex_destroy(&(stop->mutex)) != 0) {
    fprintf(stderr, "\nErro ao destruir mutex\n");
    exit(EXIT_FAILURE);
  }
  
  if(pthread_cond_destroy(&(stop->condition)) != 0) {
    fprintf(stderr, "\nErro ao destruir variável de condição\n");
    exit(EXIT_FAILURE);
  }
	free(stop);
}