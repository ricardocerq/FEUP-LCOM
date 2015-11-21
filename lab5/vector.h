#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

///vector 'class'
typedef struct vector vector_t;

///initialize vector
///@param v vector to initialize
///@param size size of each element of the vector
void initialize_vector_t(vector_t* v, size_t size);

///creates and initializes a vector
///@param size size of each element of the vector
///@return initialized vector
vector_t* new_vector_t(size_t size);

///adds an element at the end of the vector
///@param v vector
///@param element to add
///@return zero on success, no-zero otherwise
int push_back(vector_t* v, void* elem);

///removes an element from the end of the vector
///@param v vector
///@return zero on success, no-zero otherwise
int pop_back(vector_t* v);

///returns the size of the vector
///@param v vector
///@return size of vector
int size(vector_t* v);

///element access
///@param v vector
///@param i index of element
///@return pointer to element
void* at(vector_t* v, int i);

///erases element from vector, reducing its size
///@param v vector
///@param i index of element
///@return zero on success, no-zero otherwise
int erase(vector_t* v, int i);

///empties the vector
///@param v vector
void clear(vector_t * v);

///frees memory used by vector
///@param v vector
void delete_vector_t(vector_t* v);

#endif
