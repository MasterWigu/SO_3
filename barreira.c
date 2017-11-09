#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

/*--------------------------------------------------------------------
| Types
---------------------------------------------------------------------*/

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int espera1, espera2;
    int n_threads;
    int desvio_max;
    int desvio_fatias, desvio_fatias_temp;
} Stop;

/*--------------------------------------------------------------------
| Global variables
---------------------------------------------------------------------*/
Stop *stop;




int waitBarreira(int iter, int desvio_fatia) {
	int* espera_activa;
	pthread_mutex_lock(&stop->mutex);

	if (iter%2==0) {
		stop->espera1++;
		espera_activa = &stop->espera1;
		printf("AAA %d\n", stop->espera1);
	}
	else {
		stop->espera2++;
		espera_activa = &stop->espera2;
		printf("BBB %d\n", stop->espera2);
	}

	printf("CCC %d\n", stop->n_threads);
	fflush(stdout);

	if (desvio_fatia > stop->desvio_fatias)
		stop->desvio_fatias = desvio_fatia;

	if ((*espera_activa) == (stop->n_threads)) {
		stop->desvio_fatias_temp = stop->desvio_fatias;
		stop->desvio_fatias = -1;
		*espera_activa = 0;
		printf("AAASSS\n\n");
		pthread_cond_broadcast(&stop->condition);
		printf("AAAABB\n");
	}

	else {
		printf("AASDSDSD\n");
		while (*espera_activa != 0) {
			pthread_cond_wait(&stop->condition, &stop->mutex);
		}
	}
	pthread_mutex_unlock(&stop->mutex);
	if (stop->desvio_fatias_temp < stop->desvio_max)
		return 1;
	return 0;
}


int initBarreira(int n_threads, int desvio) {
	stop = (Stop*) malloc(sizeof(Stop));

	if (stop == NULL) {
		fprintf(stderr, "Erro ao alocar struct barreira\n");
		return -1;
	}

	if (pthread_mutex_init(&stop->mutex, NULL) == -1) {
		fprintf(stderr, "Erro ao inicializar mutex\n");
		return -1;
	}

	if (pthread_cond_init(&stop->condition, NULL) == -1) {
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
	pthread_mutex_destroy(&stop->mutex);
	pthread_cond_destroy(&stop->condition);
	free(stop);
}