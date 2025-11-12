#ifndef __DATA_H__
#define __DATA_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
//vector -> a double NULL pointer of a vector structue
//
void initVector(Vector** vector,unsigned int d);

//initVectorData() initialized the given vector with a given number. Requires:
//vector -> a double NULL pointer of a vector structue
//value  -> initaliation value

void initVectorData(Vector* vector,float value);

//initVectorRandom() initialized the given vector with a given number. Requires:
//vector -> a double NULL pointer of a vector structue
//min,max  -> range for the random values

void initVectorRandom(Vector* vector, float min, float max);

//destroyVector() deallocates the given vector structure. Requires:
//vector -> the double pointer of the vector structue to deallocate

void destroyVector(Vector** vector);

//printVector() prints the desired range of values of a vector. Requires:
//vector -> the vector structure to print
//start,end -> start and end points 
//
//The output will be a subvector with [] braces

void printVector(Vector* vector, unsigned int start, unsigned int end);

//initMatrix() generates a matrix structure and assigns it at the given matrix pointer. Requires:
//dx -> number of rows of the matrix
//dy -> dize of the rows of the matrix
//matrix -> a NULL double pointer of a matrix structue
//

void initMatrix(Matrix** matrix, unsigned int dx, unsigned int dy);

//initMatrixData() initializes the matrix with a given number. Requires:
//matrix -> the pointer of the matrix structure
//value -> initialization value;

void initMatrixData(Matrix* matrix,float value);

//initMatrixRandom() initializes the matrix with random numbers. Requires:
//matrix -> pointer to the matrix structure
//min,max -> range for the random values

void initMatrixRandom(Matrix* matrix, float min, float max);

//printMAtrix() prints the matrix values in given start/end coordinates. Reuires:
//matrix -> pointer to the matrix structure
//rowStart,rowEns -> range of the rows to print
//colStart,colEnd -> range of the values to print in a single row
//
//The output will be a submatrix with {} braces for delimiting the set of rows and [] for the values in a single row.
void printMatrix(Matrix* matrix,unsigned int rowStart,unsigned int rowEnd, unsigned int columnStart, unsigned int columnEnd);

//destroy() deallocates a matrix structure. Requires:
//matrix -> the double pointer of the matrix structure to deallocate.

void destroyMatrix(Matrix** matrix);

//min() return the minimum value between a and b
unsigned int min(unsigned int a, unsigned int b);

//max() returns the maimum value between a and b;
unsigned int max(unsigned int a, unsigned int b);

#endif 
