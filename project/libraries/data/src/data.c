#include "data.h"

void initVector(unsigned int d, Vector* vector) {
    vector=malloc(sizeof(Vector));
    vector->d=d;
    vector->data=malloc(sizeof(float)*d);
}
void initVectorRandom(unsigned int d, Vector* vector);

void destroyVector(Vector* vector) {
    free(vector->data);
    free(vector);
}


void initMatrix(unsigned int dx, unsigned int dy, Matrix* matrix) {
    matrix=malloc(sizeof(Matrix));
    matrix->dx=dx;
    matrix->dy=dy;
    matrix->data=malloc(dx*sizeof(float*));
    for (unsigned int i=0;i<dy;i++) {
        matrix->data[i]=malloc(dy*sizeof(float));
    }
}

void initMatrixRandom(unsigned int dx, unsigned int dy, Matrix* matrix);

void destroyMatrix(Matrix* matrix) {
    for (unsigned int i=0;i<matrix->dx;i++) {
        free(matrix->data[i]);
    }
    free(matrix->data);
    free(matrix);
}


