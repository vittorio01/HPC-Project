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
    double* data;
    unsigned int d;
} Vector;

/*
definition of matrix structure:
dx -> number of rows
dy -> number of elements per row
data -> array of pointers that contins the rows
*/
typedef struct Matrix {
    double** data;
    unsigned int dx;
    unsigned int dy;
} Matrix;

//initVector() generates a vector structure and assigns it at the given vector pointer. Requires:
// IMPORTANT: vector needs to be set to NULL before calling this function, otherwise undefined behaviour will occur
//d -> dimension of the vector to create
//vector -> a double NULL pointer of a vector structue
//
void initVector(Vector** vector,unsigned int d);

//initVectorData() initialized the given vector with a given number. Requires:
//vector -> a double NULL pointer of a vector structue
//value  -> initaliation value
void initVectorData(Vector* vector,double value);

//initVectorRandom() initialize the given vector with a given number. Requires:
//vector -> an already initialized vector
//min,max  -> range for the random values
void initVectorRandom(Vector* vector, double min, double max);

//copySubVector copies the content between two vectors with custom coordinates/sizes. Requires:
//source -> the pointer of the source vector
//dest -> the pointer of the destination vector
//sourcePoint -> first element to copy from the source vector
//destPoint -> position in the destination vector
//n -> number of elements to copy
void copySubVector(Vector* source, Vector* dest,unsigned int sourcePoint, unsigned int destPoint, unsigned int n);

//copyVector() copy the source vector in the dest vector. Requires:
//dest, source pointers of the two vectors
void copyVector(Vector* source, Vector* dest);

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
//dy -> number of columns of the matrix
//matrix -> a NULL double pointer of a Matrix structure
void initMatrix(Matrix** matrix, unsigned int dx, unsigned int dy);

//initMatrixData() initializes the matrix with a given number. Requires:
//matrix -> the pointer of the matrix structure
//value -> initialization value;
void initMatrixData(Matrix* matrix,double value);

//initMatrixRandom() initializes the matrix with random numbers. Requires:
//matrix -> pointer to the matrix structure
//min,max -> range for the random values
void initMatrixRandom(Matrix* matrix, double min, double max);

//copySubMatrix() copies the content between two matrices with also different shapes/lengths. Requires:
//source -> the pointer of the source matrix
//dest -> the pointer of the destination matrix
//sourceRow -> Row coordinate of the first element from the source matrix
//destRow -> Row coordinate for the first element to replace in the destination matrix
//sourceCol -> Col coordinate of the first element from the source matrix
//destCol -> Col coordinate for the first element to replace in the destination matrix
//rowN -> Number of rows to copy
//colN -> Number of elements per row to copy
void copySubMatrix(Matrix* source, Matrix* dest, unsigned int sourceRow, unsigned int destRow, unsigned int sourceCol, unsigned int destCol,unsigned int rowN, unsigned int colN);

//copyMatrix() copy the content from source matrix to destination matrix. Requires:
//source, dest -> pointers of matrix structures
void copyMatrix(Matrix* source, Matrix* dest);

//printMAtrix() prints the matrix values in given start/end coordinates. Reuires:
//matrix -> pointer to the matrix structure
//rowStart,rowEns -> range of the rows to print
//colStart,colEnd -> range of the values to print in a single row
//
//The output will be a submatrix with {} braces for delimiting the set of rows and [] for the values in a single row.
void printMatrix(Matrix* matrix,unsigned int rowStart,unsigned int rowEnd, unsigned int columnStart, unsigned int columnEnd);

//destroyMatrix() deallocates a matrix structure. Requires:
//matrix -> the double pointer of the matrix structure to deallocate.
void destroyMatrix(Matrix** matrix);

//copyToSubVector() copies the content from a matrix line to a vector with custom coordinates/length. Requires:
//source, dest -> pointers to the source matrix and destination vector
//sourceCol, sourceRow -> position of the first element to copy
//n -> numer of elements to copy (in the same line specified with sourceRow)
void copyToSubVector(Matrix* source, Vector* dest, unsigned int sourceRow, unsigned int sourceCol, unsigned int destPos ,unsigned int n);

//copyToVector() extracts a line from the source matrix to the destination vector. Requires:
//source, dest -> pointers to the source matrix and destination vector
//row -> source row
void copyToVector(Matrix* source, Vector* dest, unsigned int row);

//copyToSubMatrix() copies the content from a vectorto a matrix line with custom coordinated/length. Requires:
//source, dest -> pointers ot the destinatio matrix and source vector
//destRow, destCol -> coordinates in the destination matrix
//sourcePoint -> position of the first element to copy
//n -> number of elements to copy
void copyToSubMatrix(Vector* source, Matrix* dest, unsigned int sourcePos, unsigned int destRow, unsigned int destCol, unsigned int n);

//copyToMatrix() copies the contenti from the vector to a specified matrix row. requires:
//source, dest -> pointers to the destination m:watrix and source vector
//row - selected row in the destination matrix
void copyToMatrix(Vector* source, Matrix* dest, unsigned int row);

//min() return the minimum value between a and b
unsigned int min(unsigned int a, unsigned int b);

//max() returns the maimum value between a and b;
unsigned int max(unsigned int a, unsigned int b);

// minOfVec() returns the index of the smallest item of the vector
// vec -> pointer to Vec object
int minOfVec(Vector *vec);

// maxOfVec() returns the index of the biggest item of the vector
// vec -> pointer to Vec object
int maxOfVec(Vector *vec);

#endif
