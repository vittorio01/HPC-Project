#include "utils.h"


int intMax(int a,int b) {
    if (a>b) return a; 
    return b;
}
int intMin(int a,int b) {
    if (a<b) return a; 
    return b;
}


unsigned int intCeil(int elements, int n, int id) {
    int elementsPerN=(elements + n - 1)/n; 
    int nStart = id * elementsPerN; 
    int nEnd = intMin(nStart+elementsPerN,elements);
    return intMax(nEnd-nStart,0);
}


