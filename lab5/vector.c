#include "vector.h"

#define INITIAL_CAPACITY_VECTOR 2

struct vector
{
	void* buffer;
	int capacity, count;
	size_t size_of_elem;
};

void initialize_vector_t(vector_t* v, size_t size)
{
	v->buffer = NULL;
	v->count = 0;
	v->capacity = 0;
	v->size_of_elem = size;
}

vector_t* new_vector_t(size_t size)
{
	vector_t* out = (vector_t*) malloc(sizeof(vector_t));
	if (out != NULL)
	initialize_vector_t(out,size);
	return (vector_t *) out;
}

int push_back(vector_t* v, void* elem)
{
	if (v->capacity == 0)
	{
		v->buffer = malloc(v->size_of_elem * INITIAL_CAPACITY_VECTOR);
		if (v->buffer == NULL)
			return 1;
		else
		{
			memcpy(v->buffer, elem, v->size_of_elem);
			v->count = 1;
			v->capacity = INITIAL_CAPACITY_VECTOR;
			return 0;
		}
	}
	else
	{
		if (v->count == v->capacity)
		{
			free(v->buffer);
			v->buffer = realloc(v->buffer, v->size_of_elem * v->capacity * 2);
			if (v->buffer == NULL)
				return 1;
			else
			{
				memcpy((int) v->buffer + v->count * v->size_of_elem, elem, v->size_of_elem);
				v->count++;
				v->capacity = v->capacity * 2;
				return 0;
			}
		}
		else
		{
			memcpy((int) v->buffer + v->count * v->size_of_elem, elem, v->size_of_elem);
			v->count++;
			return 0;
		}
	}
}

int pop_back(vector_t* v)
{
	if (v->count == 0)
		return 1;
	v->count--;
	return 0;
}

int size(vector_t* v)
{
	return v->count;
}

void* at(vector_t* v, int i)
{
	if (i < 0 || i >= v->count)
		return NULL;
	else
	{
		return (void*) ((int)v->buffer + i*v->size_of_elem);
	}
}

int erase(vector_t* v, int i)
{
	if (i < 0 || i >= v->count)
			return 1;
	memcpy(v->buffer + i * v->size_of_elem, v->buffer + (i+1) * v->size_of_elem, v->size_of_elem * (v->count - i - 1));
	v->count--;
	return 0;
}
void clear(vector_t * v)
{
	v->count = 0;
}
void delete_vector_t(vector_t* v)
{
	free(v->buffer);
	free(v);
}
