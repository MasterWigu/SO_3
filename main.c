/*
// Projeto SO - exercicio 1, version 03
// Sistemas Operativos, DEI/IST/ULisboa 2017-18
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "matrix2d.h"
#include "barreira.h"

DoubleMatrix2D *matrix[2];


/*--------------------------------------------------------------------
| Type: thread_info
| Description: Estrutura com Informação para Trabalhadoras
---------------------------------------------------------------------*/

typedef struct { 
    int id;
    int N;
    int trab;
    int tam_fatia;
    int iter;
    double maxD;
    int iter_fim;
}thread_info;


/*--------------------------------------------------------------------
| Function: absl
---------------------------------------------------------------------*/
double absl(double num) {
  if (num < 0)
    return -num;
  return num;
}

/*--------------------------------------------------------------------
| Function: simul
---------------------------------------------------------------------*/

double simul(int N, int tam_fatia, int id, int n_iter) {

  int i, j;
  double value, desvio = -1, desvio_local;

  
  for (i = (tam_fatia*(id-1)+1); i <= tam_fatia*((id-1)+1); i++)
    for (j = 1; j <= N; j++) {
      value = ( dm2dGetEntry(matrix[n_iter%2], i-1, j) + dm2dGetEntry(matrix[n_iter%2], i+1, j) +
	              dm2dGetEntry(matrix[n_iter%2], i, j-1) + dm2dGetEntry(matrix[n_iter%2], i, j+1) ) / 4.0;
                dm2dSetEntry(matrix[(n_iter+1)%2], i, j, value);

      desvio_local = absl(dm2dGetEntry(matrix[(n_iter+1)%2], i, j) - dm2dGetEntry(matrix[n_iter%2], i, j));
      if (desvio_local > desvio)
        desvio = desvio_local;
    }
  return desvio;
}


/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

void *tarefa_trabalhadora(void* args) {
  thread_info *tinfo = (thread_info *) args;
  int iter, flag_desvio, retorno;
  double desvio;

  for (iter=0; iter<= (tinfo->iter) ; iter++) {
    desvio = simul(tinfo->N, tinfo->tam_fatia, tinfo->id, iter);

    if (desvio > tinfo->maxD)
      flag_desvio = 0;
    else
      flag_desvio = 1;

    retorno = waitBarreira(iter, flag_desvio);

    if (retorno == -1) {
      fprintf(stderr, "Erro ao esperar pela barreira\n");
      exit(-1);
    }

    if (retorno == 1) { //caso a barreira retorne 1 e para terminar
      tinfo->iter_fim = iter; //guarda o numero da ultima iteracao
      return NULL;
    }
  }
 return NULL;
}


/*--------------------------------------------------------------------
| Function: parse_integer_or_exit
---------------------------------------------------------------------*/

int parse_integer_or_exit(char const *str, char const *name)
{
  int value;
 
  if(sscanf(str, "%d", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}

/*--------------------------------------------------------------------
| Function: parse_double_or_exit
---------------------------------------------------------------------*/

double parse_double_or_exit(char const *str, char const *name)
{
  double value;

  if(sscanf(str, "%lf", &value) != 1) {
    fprintf(stderr, "\nErro no argumento \"%s\".\n\n", name);
    exit(1);
  }
  return value;
}


/*--------------------------------------------------------------------
| Function: main
---------------------------------------------------------------------*/

int main (int argc, char** argv) {

  if(argc != 9) {
    fprintf(stderr, "\nNumero invalido de argumentos.\n");
    fprintf(stderr, "Uso: heatSim N tEsq tSup tDir tInf trabalhadoras iteracoes desvioMax\n\n");
    return 1;
  }

  /* argv[0] = program name */
  int N = parse_integer_or_exit(argv[1], "N");
  double tEsq = parse_double_or_exit(argv[2], "tEsq");
  double tSup = parse_double_or_exit(argv[3], "tSup");
  double tDir = parse_double_or_exit(argv[4], "tDir");
  double tInf = parse_double_or_exit(argv[5], "tInf");
  int trab = parse_integer_or_exit(argv[6], "trabalhadoras");
  int iter = parse_integer_or_exit(argv[7], "iteracoes");
  double desvio = parse_double_or_exit(argv[8], "desvio");

  fprintf(stderr, "\nArgumentos:\n"
	" N=%d tEsq=%.1f tSup=%.1f tDir=%.1f tInf=%.1f trabalhadoras=%d iteracoes=%d desvioMax=%.4f\n",
	N, tEsq, tSup, tDir, tInf, trab, iter, desvio);

  if(N < 1 || tEsq < 0 || tSup < 0 || tDir < 0 || tInf < 0 || trab < 1 || iter < 1 || desvio < 0) {
    fprintf(stderr, "\nErro: Argumentos invalidos.\n"
	" Lembrar que N >= 1, temperaturas >= 0, trabalhadoras >= 1, iteracoes >=1, desvio >= 0\n\n");
    return 1;
  }

  matrix[0] = dm2dNew(N+2, N+2);
  matrix[1]= dm2dNew(N+2, N+2);

  if (matrix[0] == NULL || matrix[1] == NULL) {
    fprintf(stderr, "\nErro: Nao foi possivel alocar memoria para as matrizes.\n\n");
    return -1;
  }

  int i, tam_fatia, res;
  pthread_t trabalhadoras[trab];
  thread_info tinfo[trab];


  for(i=0; i<N+2; i++)
    dm2dSetLineTo(matrix[0], i, 0);

  dm2dSetLineTo (matrix[0], 0, tSup);
  dm2dSetLineTo (matrix[0], N+1, tInf);
  dm2dSetColumnTo (matrix[0], 0, tEsq);
  dm2dSetColumnTo (matrix[0], N+1, tDir);

  dm2dCopy (matrix[1], matrix[0]);

  if (initBarreira(trab) == -1) {
    fprintf(stderr, "\nErro a criar barreira\n");
    return -1;
  }

  tam_fatia = N/trab;

  /* Criar Trabalhadoras */
  for (i = 0; i < trab; i++) {
    tinfo[i].id = i+1;
    tinfo[i].N = N;
    tinfo[i].trab = trab;
    tinfo[i].tam_fatia = tam_fatia;
    tinfo[i].iter = iter;
    tinfo[i].maxD = desvio;
    
    res = pthread_create(&trabalhadoras[i], NULL, tarefa_trabalhadora, &tinfo[i]);

    if(res != 0) {
      fprintf(stderr, "\nErro ao criar uma tarefa trabalhadora.\n");
      return -1;
    }
  }
  

  /* Esperar que as Trabalhadoras Terminem */
  for (i = 0; i < trab; i++) {
    res = pthread_join(trabalhadoras[i], NULL);
    
    if (res != 0) {
      fprintf(stderr, "\nErro ao esperar por uma tarefa trabalhadora.\n");    
      return -1;
    }  
  }
  
  dm2dPrint(matrix[(tinfo[0].iter_fim +1)% 2]); //vai buscar o numero da ultima iteracao para escolher a matrix a exibir

  dm2dFree(matrix[0]);
  dm2dFree(matrix[1]);
  destroyBarreira();

  return 0;
}
