#ifndef BARREIRA_H
#define BARREIRA_H

extern DoubleMatrix2D *matrix[2];

int waitBarreira(int iter, int desvio);
int initBarreira(int n_threads);
void destroyBarreira();




#endif