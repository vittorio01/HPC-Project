#include <stdio.h>
#include <stdlib.h>
#include <data.h>

int main(int argc, char** argv) {
    printf("Testing vector functions...\n");
    
    Vector* vector=NULL;
    initVector(&vector,100);
    printVector(vector,0,10);
    
    if (vector==NULL) {
        printf("Vector not correctly generated\n");
        return 1;
    }
    printVector(vector,0,10);
    initVectorData(vector,30);
    printVector(vector,0,10);
    initVectorRandom(vector,-10,10);
    destroyVector(&vector);

    printf("Testing matrix functions...\n");
    Matrix* matrix=NULL;
    initMatrix(&matrix,10,10);
    if (matrix==NULL) {
        printf("Matrix not correctly generated\n");
        return 1;
    }
    printMatrix(matrix,0,5,0,5);
    initMatrixData(matrix,25);
    printMatrix(matrix,12,12,8,5);
    initMatrixRandom(matrix,-125,110);
    printMatrix(matrix,0,10,0,12);
    destroyMatrix(&matrix);

    return 0;
}
