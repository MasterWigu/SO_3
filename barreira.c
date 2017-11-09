#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

/*--------------------------------------------------------------------
| Types
---------------------------------------------------------------------*/

typedef struct {
    pthread_mutex_t mutex1;
    pthread_mutex_t mutex2;
    pthread_cond_t condition1;
    pthread_cond_t condition2;
    int espera1, espera2;
    int n_threads;
    int desvio_max;
    int desvio_fatias, desvio_fatias_temp;
} Stop;

/*--------------------------------------------------------------------
| Global variables
---------------------------------------------------------------------*/
Stop *stop;

int waitBarreira1(int iter, int desvio_fatia) {
	int* espera_activa;
	
	printf("VOU BLOQUEAR O MUTEX 1\n");
	pthread_mutex_lock(&stop->mutex1);

	stop->espera1++;

	printf("%d\n", stop->espera1);
	espera_activa = &stop->espera1;
	

	//if (desvio_fatia > stop->desvio_fatias)
		//stop->desvio_fatias = desvio_fatia;

	if ((*espera_activa) == (stop->n_threads)) {
		//stop->desvio_fatias_temp = stop->desvio_fatias;
		//stop->desvio_fatias = -1;
		
		printf("VOU FAZER BROADCAST 1\n");
		pthread_cond_broadcast(&stop->condition1);
		printf("FIZ BROADCAST 1\n");
		*espera_activa = 0;
	}

	else {
		printf(" PASSEI O ELSE 1\n");
		while (*espera_activa != 0) {
			printf("CHEGUEI A CONDICAO 1\n");
			pthread_cond_wait(&stop->condition1, &stop->mutex1);
		}
	}
	printf("VOU FAZER O UNLOCK 1\n");
	pthread_mutex_unlock(&stop->mutex1);
	printf("UNLOCK 1\n");
	//if (stop->desvio_fatias_temp < stop->desvio_max)
	//	return 1;
	return 0;
}

int waitBarreira2(int iter, int desvio_fatia) {
	int* espera_activa;
	
	printf("VOU BLOQUEAR O MUTEX 2\n");
	pthread_mutex_lock(&stop->mutex2);

	stop->espera2++;
	printf("%d\n", stop->espera2);
	espera_activa = &stop->espera2;

	//if (desvio_fatia > stop->desvio_fatias)
		//stop->desvio_fatias = desvio_fatia;

	if ((*espera_activa) == (stop->n_threads)) {
		//stop->desvio_fatias_temp = stop->desvio_fatias;
		//stop->desvio_fatias = -1;
		
		printf("VOU FAZER BROADCAST 2\n");
		pthread_cond_broadcast(&stop->condition2);
		printf("FIZ BROADCAST 2\n");
		*espera_activa = 0;
	}

	else {
		printf(" PASSEI O ELSE 2\n");
		while (*espera_activa != 0) {
			printf("CHEGUEI A CONDICAO 2\n");
			pthread_cond_wait(&stop->condition2, &stop->mutex2);
		}
	}
	printf("VOU FAZER O UNLOCK 2\n");
	pthread_mutex_unlock(&stop->mutex2);
	printf("UNLOCK 2\n");
	//if (stop->desvio_fatias_temp < stop->desvio_max)
	//	return 1;
	return 0;
}


int initBarreira(int n_threads, int desvio) {
	stop = (Stop*) malloc(sizeof(Stop));

	if (stop == NULL) {
		fprintf(stderr, "Erro ao alocar struct barreira\n");
		return -1;
	}

	if (pthread_mutex_init(&stop->mutex1, NULL) == -1) {
		fprintf(stderr, "Erro ao inicializar mutex\n");
		return -1;
	}

	if (pthread_mutex_init(&stop->mutex2, NULL) == -1) {
		fprintf(stderr, "Erro ao inicializar mutex\n");
		return -1;
	}

	if (pthread_cond_init(&stop->condition1, NULL) == -1) {
		fprintf(stderr, "Erro ao inicializar variavel de condicao\n");
		return -1;
	}

	if (pthread_cond_init(&stop->condition2, NULL) == -1) {
		fprintf(stderr, "Erro ao inicializar variavel de condicao\n");
		return -1;
	}

	stop->espera1 = 0;
	stop->espera2 = 0;
	stop->n_threads = n_threads;
	stop->desvio_max = desvio;
	return 0;
}

void destroyBarreira() {
	pthread_mutex_destroy(&stop->mutex1);
	pthread_mutex_destroy(&stop->mutex2);
	pthread_cond_destroy(&stop->condition1);
	pthread_cond_destroy(&stop->condition2);
	free(stop);
}