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
    int espera[2];
    int n_threads;
    int desvio_fatias;
    int terminar;
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

  	stop->espera[0] = 0;
  	stop->espera[1] = 0;
  	stop->n_threads = n_threads;
  	stop->desvio_fatias = 0;
  	stop->terminar = 0;

  	return 0;
}


/*--------------------------------------------------------------------
| Function: waitBarreira
---------------------------------------------------------------------*/

int waitBarreira(int iter, int desvio_fat) {

	if (pthread_mutex_lock(&(stop->mutex)) != 0) {
		fprintf(stderr, "Erro ao bloquear mutex\n");
		exit(EXIT_FAILURE);
	}

	stop->espera[iter%2] = stop->espera[iter%2] +1;

	stop->desvio_fatias += desvio_fat;

	if ((stop->espera[iter%2]) == stop->n_threads) {
		stop->espera[iter%2] = 0;


		if (stop->desvio_fatias == stop->n_threads)
			stop->terminar = 1;
		stop->desvio_fatias = 0;

		if (pthread_cond_broadcast(&(stop->condition)) != 0) {
			fprintf(stderr, "Erro no broadcast\n");
			exit(EXIT_FAILURE);
		}
	}
	else {
		while (stop->espera[iter%2] != 0) {
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
		return 1;
	}

	if (pthread_mutex_unlock(&(stop->mutex)) != 0) {
		fprintf(stderr, "Erro ao bloquear mutex\n");
		exit(EXIT_FAILURE);
	}

	return 0;
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