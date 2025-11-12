#include "data.h"

void initVector(Vector** vector, unsigned int d) {
    //destroy an eventual vector allocated in the given pointer
    destroyVector(vector);

    //vector allocation
    (*vector)=malloc(sizeof(Vector));
    (*vector)->d=d;
    (*vector)->data=malloc(sizeof(float)*d);
}
void initVectorData(Vector* vector, float value) {
    if (vector==NULL) return;
    for (int i=0;i<vector->d;i++) {
        vector->data[i]=value;
    }
}

void initVectorRandom(Vector* vector, float min, float max) {
    if (vector==NULL) return;
    for (int i=0;i<vector->d;i++) {
        vector->data[i]=min + ((float) rand() / (float) RAND_MAX)*(max-min);
    }
}

void destroyVector(Vector** vector) {
    if ((*vector)==NULL) return;

    //if the vector's pointer is not null, destroy it
    if ((*vector)->data!=NULL) free((*vector)->data);
    free((*vector));
}


void initMatrix(Matrix** matrix,unsigned int dx, unsigned int dy) {
    //destroy an eventual allocated matrix on the given pointer
    destroyMatrix(matrix); 

    //matrix allocation
    (*matrix)=malloc(sizeof(Matrix));
    (*matrix)->dx=dx;
    (*matrix)->dy=dy;
    (*matrix)->data=malloc(dx*sizeof(float*));
    for (unsigned int i=0;i<dy;i++) {
        (*matrix)->data[i]=malloc(dy*sizeof(float));
    }
}

void initMatrixData(Matrix* matrix,float value) {
    if (matrix==NULL) return;

    for (int i=0;i<matrix->dx;i++) {
        for(int j=0;j<matrix->dy;j++) {
            matrix->data[i][j]=value;
        }
    }
}

void initMatrixRandom(Matrix* matrix,float min,float max) {
    if (matrix==NULL) return;

    srand(time(NULL));
    for (int i=0;i<matrix->dx;i++) {
        for(int j=0;j<matrix->dy;j++) {
            matrix->data[i][j]=min + ((float) rand() / (float) RAND_MAX)*(max-min);
        }
    }
}

void destroyMatrix(Matrix** matrix) {
    if ((*matrix)==NULL) return;

    //if the matrix pointer is not null, destroy it 
    if ((*matrix)->data!=NULL) {
        for (unsigned int i=0;i<(*matrix)->dx;i++) {
            if((*matrix)->data[i]!=NULL) free((*matrix)->data[i]);
        }
        free((*matrix)->data);
    }
    free((*matrix));
}

void printVector(Vector* vector, unsigned int start, unsigned int end) {
    if (vector==NULL) return;
    end=min(end,vector->d);
    printf("[");
    for (unsigned int i=start;i<end;i++) {
        printf("%f,",vector->data[i]);
    }
    printf("]\n");
}

void printMatrix(Matrix* matrix,unsigned int rowStart,unsigned int rowEnd, unsigned int columnStart, unsigned int columnEnd) {
    if (matrix==NULL) return;
    rowEnd=min(rowEnd, matrix->dx);
    columnEnd=min(columnEnd,matrix->dy);
    printf("{\n");
    for (unsigned int i=rowStart;i<rowEnd;i++) {
        printf("[");
        for (unsigned int j=columnStart;j<columnEnd;j++) {
            printf("%f,",matrix->data[i][j]);
        }
        printf("]\n");
    }
    printf("}\n");
}

unsigned int min(unsigned int a, unsigned int b) {
    if (a<b) return a;
    return b;
}

unsigned int max(unsigned int a, unsigned int b) {
    if (a>b) return a;
    return b;
}
