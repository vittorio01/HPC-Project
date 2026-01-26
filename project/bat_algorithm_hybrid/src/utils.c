#include "utils.h"


int intMax(int a,int b) {
    if (a>b) return a; 
    return b;
}
int intMin(int a,int b) {
    if (a<b) return a; 
    return b;
}


unsigned int intCeil(unsigned int elements, unsigned int n, unsigned id) {
    unsigned int elementsPerN=(elements + n - 1)/n; 
    unsigned int nStart = id * elementsPerN; 
    unsigned int nEnd = intMin(nStart+elementsPerN,elements);
    return nEnd-nStart;
}

unsigned int intRequiredThreads(unsigned int elements, unsigned int elementsPerThread) {
    unsigned int threadsNumber=elements/elementsPerThread; 
    if (elements%elementsPerThread>0) threadsNumber++;
    return threadsNumber;
}

