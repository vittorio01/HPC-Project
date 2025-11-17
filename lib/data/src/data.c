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
    (*vector)=NULL;
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
    (*matrix)=NULL;
}

void printVector(Vector* vector, unsigned int start, unsigned int end) {
    if (vector==NULL) return;
    end=min(end,vector->d);
    printf("(%d->%d)\n",start,end);
    printf("[");
    for (unsigned int i=start;i<end;i++) {
        printf("%f",vector->data[i]);
        if (i<end-1) printf(",\t");
    }
    printf("]\n");
}

void printMatrix(Matrix* matrix,unsigned int rowStart,unsigned int rowEnd, unsigned int columnStart, unsigned int columnEnd) {
    if (matrix==NULL) return;
    rowEnd=min(rowEnd, matrix->dx);
    columnEnd=min(columnEnd,matrix->dy);
    printf("{(%d->%d,%d->%d)\n",rowStart,rowEnd,columnStart,columnEnd);
    for (unsigned int i=rowStart;i<rowEnd;i++) {
        printf("[");
        for (unsigned int j=columnStart;j<columnEnd;j++) {
            printf("%f",matrix->data[i][j]);
            if (j<columnEnd-1) printf(",\t");
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

void copySubVector(Vector* source, Vector* dest, unsigned int sourcePoint, unsigned int destPoint, unsigned int n) {
   if (dest==NULL || source==NULL) return;
   unsigned int limit=min((source->d)-sourcePoint,(dest->d)-destPoint);
   limit=min(limit,n);
   for (int i=0;i<limit;i++) {
       dest->data[destPoint+i]=source->data[sourcePoint+i];
   }

}

void copyVector(Vector* source, Vector* dest) {
    unsigned int nlim=min(source->d,dest->d);
    copySubVector(source,dest,0,0,nlim);
}

void copySubMatrix(Matrix* source, Matrix* dest, unsigned int sourceRow, unsigned int destRow, unsigned int sourceCol, unsigned int destCol,unsigned int rowN, unsigned int colN) {
    if (dest==NULL || source==NULL) return;
    unsigned int limitRow=min((source->dx)-sourceRow,(dest->dx)-destRow);
    limitRow=min(rowN,limitRow);

    unsigned int limitCol=min((source->dy)-sourceCol,(dest->dy)-destCol);
    limitCol=min(colN,limitCol);

    for (int i=0;i<limitRow;i++) {
        for (int j=0;j<limitCol;j++) {
            dest->data[destRow+i][destCol+j]=source->data[sourceRow+i][sourceCol+j];
        }
    }
}

void copyMatrix(Matrix* source, Matrix* dest) {
    unsigned int nlimx=min(source->dx,dest->dx);
    unsigned int nlimy=min(source->dy,dest->dy);
    copySubMatrix(source,dest,0,0,0,0,nlimx,nlimy);
}

void copyToSubMatrix(Vector* source, Matrix* dest, unsigned int sourcePos, unsigned int destRow, unsigned int destCol, unsigned int n) {
    if (source==NULL || dest==NULL || destRow>=dest->dx) return;

    unsigned int lim=min((dest->dy)-destCol,(source->d)-sourcePos);
    n=min(n,lim);
    for (unsigned int i=0;i<n;i++) {
        dest->data[destRow][destCol+i]=source->data[sourcePos+i];
    }

}

void copyToMatrix(Vector* source, Matrix* dest,unsigned int row) {
    unsigned int lim=min(dest->dy,source->d);
    copyToSubMatrix(source,dest,0,row,0,lim);
}

void copyToSubVector(Matrix* source, Vector* dest, unsigned int sourceRow, unsigned int sourceCol, unsigned int destPos,unsigned int n) {
    if (source==NULL || dest==NULL || sourceRow>=source->dx) return;

    unsigned int lim=min((dest->d)-destPos,(source->dy)-sourceCol);
    n=min(n,lim);
    for (unsigned int i=0;i<n;i++) {
        dest->data[destPos+i]=source->data[sourceRow][sourceCol+i];
    }
}

void copyToVector(Matrix* source, Vector* dest,unsigned int row) {
    unsigned int lim=min(dest->d,source->dy);
    copyToSubVector(source,dest,row,0,0,lim);
}

