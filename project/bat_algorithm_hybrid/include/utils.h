#ifndef __UTILS_H__ 
#define __UTILS_H__ 

/*
 * This library contains useful functions for the hybrid bat algorithm
 */

//definition for bool values
typedef enum {false,true} bool;

/* intMax() returns the max value between a and b:
 * a -> first value 
 * b -> second value
 */
int intMax(int a,int b);

/* intMin() returns the min value between a and b: 
 * a -> first value 
 * b -> second balue
 */
int intMin(int a,int b);

/* intCeil() divides the number of elements between multiple threads with a specific Id and returns the number of elements that should be processed for that specific thread:
 * elements -> numer of elements to divide 
 * n        -> number of threads 
 * id       -> id of the thread 
 */
unsigned int intCeil(unsigned int elements, unsigned int n, unsigned id);

/* intRequiredThreadsl() finds how many threads are necessary in base of the number of elements and the elements per thread:
 * elements             -> numer of elements to divide 
 * elementsPErThread    -> trget number of elements per thread 
 */

unsigned int intRequiredThreads(unsigned int elements, unsigned int elementsPerThread);

#endif 
