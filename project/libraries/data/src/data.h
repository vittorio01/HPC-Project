#ifndef __DATA_H__
#define __DATA_H__

#include <stdio.h>
#include <stdlib.h>

/*
definition of vector structure
d -> dimension of the vector
data -> array of data
*/
typedef struct Vector {
    float* data;
    unsigned int d;
} Vector;

/*
definition of matrix structure:
dx -> number of rows
dy -> number of elements per row
data -> array of pointers that contins the rows
*/
typedef struct Matrix {
    float** data;
    unsigned int dx;
    unsigned int dy;
} Matrix;

//initVector() generates a veector structure and assigns it at the given vector pointer. Requires:
//d -> dimension of the vector to create 
//vector -> a NULL pointer of a vector structue
//
void initVector(unsigned int d, Vector* vector);
void initVectorRandom(unsigned int d, Vector* vector);

//destrouyVector() deallocates the given vector structurr. Requires:
//vector -> the pointer of the vector structue to deallocate
void destroyVector(Vector* vector);

//initMatrix() generates a matrix structure and assigns it at the given matrix pointer. Requires:
//dx -> number of rows of the matrix
//dy -> dize of the rows of the matrix
//matrix -> a NULL pointer of a matrix structue
//

void initMatrix(unsigned int dx, unsigned int dy, Matrix* matrix);
void initMatrixRandom(unsigned int dx, unsigned int dy, Matrix* matrix);

//destroy() deallocates a matrix structure. Requires:
//matrix -> the pointer of the matrix structure to deallocate.
//

void destroyMatrix(Matrix* matrix);

#endif 
