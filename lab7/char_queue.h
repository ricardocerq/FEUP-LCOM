#ifndef CHAR_QUEUE_H
#define CHAR_QUEUE_H

#include <stdlib.h>
#include <assert.h>

///queue 'class'
typedef struct char_queue char_queue_t;

///initialize queue
///@param this queue to initialize
void initialize_char_queue_t(char_queue_t* this);

///creates and initializes a queue
///@return initialized queue
char_queue_t* new_char_queue_t();

///adds an element to the queue
///@param this queue
///@param element to add
///@return zero on success, no-zero otherwise
int push(char_queue_t* this, unsigned char elem);

///removes an element from the end of the vector
///@param this queue
///@return zero on success, non-zero otherwise
int pop(char_queue_t* this);

///removes an element from the end of the vector
///@param this queue
///@return front of queue
unsigned char front(char_queue_t* this);

///adds an element of arbitrary size to the queue
///@param this queue
///@param element to add
///@param size of element to add
///@return zero on success, no-zero otherwise
int push_m(char_queue_t* this, void * elem, size_t size);

///removes an element of arbitrary size from the end of the vector
///@param this queue
///@param size of element
///@return zero on success, non-zero otherwise
int pop_m(char_queue_t* this, size_t size);

///removes an element of arbitrary size from the end of the vector
///@param this queue
///@param address to fill
///@param size of element
///@return success of operation
int front_m(char_queue_t* this, void* returnvalue, size_t size);

///returns the size of the queue
///@param this queue
///@return size of queue
int size_q(char_queue_t* this);

///returns the size of the queue
///@param this queue
///@return 1 if queue is empty else 0
int is_empty_q(char_queue_t* this);

///empties the queue
///@param this char_queue_t
void clear_q(char_queue_t * this);

///frees memory used by queue
///@param this queue
void delete_char_queue_t(char_queue_t* this);

#endif
