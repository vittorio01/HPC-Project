#include <stdio.h>
#include <stdlib.h>
#include <data.h>

int main(int argc, char** argv) {
    printf("--- Vector function test phase ---\n");
    printf("Testing vector initialization... ");
    Vector* vector=NULL;
    initVector(&vector,10);
    if (vector==NULL) {
        printf("Failed\n");
        return 1;
    }
    printf("Success\n");
    printf("Testing init function... ");
    initVectorData(vector,10);
    for (int i=0;i<vector->d;i++) {
        if (vector->data[i]!=10) {
            printf("Failed\n");
            return 1;
        }
    }
    
    printf("Success\n");
    printf("Testing vector print function...\n");
    printf("Entire\n");
    printVector(vector,0,10);
    printf("Lower portion\n");
    printVector(vector,0,5);
    printf("Upper portion\n");
    printVector(vector,5,12);

    printf("Testing random initialization funtion...\n");
    initVectorRandom(vector,-10,10);
    printVector(vector,0,10);
    printf("Testing vector copy function...\n");
    Vector* vector2=NULL;
    initVector(&vector2,12);
    copySubVector(vector,vector2,0,0,5);
    copySubVector(vector,vector2,4,4,10);
    copySubVector(vector,vector2,0,10,5);
    printVector(vector2,0,12);
    printf("Destroying vectors...\n");
    destroyVector(&vector);
    destroyVector(&vector2);
    printf("--- Vector function tested ---\n");

    printf("--- Matrix function test phase ---\n");
    printf("Testing matrix initialization... ");
    Matrix* matrix=NULL;
    initMatrix(&matrix,10,10);
    if (matrix==NULL) {
        printf("Failed\n");
        return 1;
    }
    printf("Success\n");
    printf("Testing init function... ");
    initMatrixData(matrix,10);
    for (int i=0;i<matrix->dx;i++) {
        for (int j=0;j<matrix->dy;j++) {
            if (matrix->data[i][j]!=10) {
                printf("Failed\n");
                return 1;
            }    
        }
    }
    printf("Success\n");
    printf("Testing matrix print function...\n");
    printf("Entire\n");
    printMatrix(matrix,0,10,0,10);
    printf("Upper left portion\n");
    printMatrix(matrix,0,5,0,5);
    printf("Lower right portion\n");
    printMatrix(matrix,5,12,5,12);
    printf("Testing random initialization funtion...\n");
    initMatrixRandom(matrix,-10,10);
    printMatrix(matrix,0,10,0,10);
    printf("Testing matrix copy function...\n");
    Matrix* matrix2=NULL;
    initMatrix(&matrix2,12,12);
    copySubMatrix(matrix,matrix2,0,0,0,0,5,5);
    copySubMatrix(matrix,matrix2,4,4,4,4,10,10);
    copySubMatrix(matrix,matrix2,0,0,10,10,5,5);
    printMatrix(matrix2,0,12,0,12);
    printf("Destroying matrices...\n");
    destroyMatrix(&matrix);
    destroyMatrix(&matrix2);
    printf("--- Matrix functions tested ---\n");
    
    printf("--- Testing matrix-vector functions---\n");
    initMatrix(&matrix,10,10);
   
    initVector(&vector,12);
    initMatrix(&matrix2,10,10);
    initVector(&vector2,10);
    if (matrix==NULL || matrix2==NULL || vector==NULL || vector2==NULL) {
        printf("Data structures not inizialized\n");
        return 1;
    }
    initVectorData(vector,1);
    initMatrixData(matrix,2);
    initVectorData(vector2,3);
    initMatrixData(matrix2,4);

    printf("Cross-copying vector1->matrix1 matrix2->vector2...");
    copyToSubMatrix(vector,matrix,2,0,0,12);
    copyToSubVector(matrix2,vector2,0,0,5,14);
    printVector(vector,0,10);
    printVector(vector2,0,12);
    printMatrix(matrix,0,12,0,12);
    printMatrix(matrix2,0,12,0,12);
    printf("Destroying structures...\n");
    destroyVector(&vector);
    destroyVector(&vector2);
    destroyMatrix(&matrix);
    destroyMatrix(&matrix2);
    printf("--- matrix-vector function tested \n");
    return 0;
}
