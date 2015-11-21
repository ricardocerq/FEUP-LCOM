#include "char_queue.h"

#define INITIAL_CAPACITY_QUEUE 16

struct char_queue
{
	char* buffer;
	int capacity, count;
	size_t front, back;
};

void initialize_char_queue_t(char_queue_t* this)
{
	this->buffer = NULL;
	this->count = 0;
	this->capacity = 0;
	this->front = 0;
	this->back = 0;
}

char_queue_t* new_char_queue_t()
{
	char_queue_t* out = (char_queue_t*) malloc(sizeof(char_queue_t));
	if (out != NULL)
		initialize_char_queue_t(out);
	return (char_queue_t *) out;
}

int push(char_queue_t* this, unsigned char elem)
{
	if (this->capacity == 0)
	{
		this->buffer = malloc(sizeof(unsigned char)*INITIAL_CAPACITY_QUEUE);
		if (this->buffer == NULL)
			return 1;
		else
		{
			this->capacity = INITIAL_CAPACITY_QUEUE;
			this->buffer[this->back] = elem;
			this->count++;
			this->back = (this->back+1)%this->capacity;
			return 0;
		}
	}
	else
	{
		if (this->count == this->capacity)
		{
			char* newbuffer = malloc(sizeof(unsigned char) * this->capacity * 2);
			//this->buffer = realloc(this->buffer, sizeof(unsigned char) * this->capacity * 2);
			if (newbuffer == NULL)
				return 1;
			else
			{
				size_t access;
				size_t i;
				for(i=0, access = this->front; i < this->count; i++, access = (access+1) % this->count)
				{
					newbuffer[i] = this->buffer[access];
				}
				free(this->buffer);
				this->buffer = newbuffer;
				this->front=0;
				this->back=this->count;
				this->capacity *= 2;
				this->buffer[this->back] = elem;
				this->back= (this->back+1)%this->capacity;
				this->count++;
				return 0;

			}
		}
		else
		{
			this->buffer[this->back] = elem;
			this->back = (this->back+1) %this->capacity;
			this->count++;
			return 0;
		}
	}
}
int pop(char_queue_t* this)
{
	assert(this->count > 0);
	this->count--;
	this->front = (this->front+1)%this->capacity;
	return 0;
}

unsigned char front(char_queue_t* this)
{
	assert(this->count > 0);
	return this->buffer[this->front];
}

//int push_m(char_queue_t* this, void * elem, size_t size)
//{
//	if (this->capacity == 0)
//		{
//			this->buffer = malloc(sizeof(unsigned char)*size);
//			if (this->buffer == NULL)
//				return 1;
//			else
//			{
//				this->capacity = size;
//				this->buffer[this->back] = elem;
//				memcpy(this->buffer, elem, size);
//				this->count+= size;
//				this->back = size;
//				return 0;
//			}
//		}
//		else
//		{
//			if (this->count + size > this->capacity)
//			{
//				size_t newbuffersize = sizeof(unsigned char) * this->capacity * 2;
//				if(this->count + size > newbuffersize)
//					buffersize += size;
//				char* newbuffer = malloc(newbuffersize);
//				//this->buffer = realloc(this->buffer, sizeof(unsigned char) * this->capacity * 2);
//				if (newbuffer == NULL)
//					return 1;
//				else
//				{
//					size_t access;
//					size_t i;
//					for(i=0, access = this->front; i < this->count; i++, access = (access+1) % this->count)
//					{
//						newbuffer[i] = this->buffer[access];
//					}
//					free(this->buffer);
//					this->buffer = newbuffer;
//					this->front=0;
//					memcpy(this->buffer + this->back, elem, size);
//					this->back=this->count + size;
//					this->capacity = newbuffersize;
//					this->count += size;
//					return 0;
//
//				}
//			}
//			else
//			{
//				if(size > this->capacity - this->back)
//				{
//					memcpy(this->buffer + this->back, elem )
//				}
//				else
//				{
//
//				}
//			}
//		}
//	return 1;
//}
int push_m(char_queue_t* this, void * elem, size_t size)
{
	size_t i = 0;
	for(i=0; i < size; i++)
	{
		if(push(this, *(unsigned char*)(elem + i) ))
			return 1;
	}
	return 0;
}
int pop_m(char_queue_t* this, size_t size)
{
	assert(this->count >= size);
	size_t i = 0;
	for(i=0; i < size; i++)
	{
		if(pop(this))
			return 1;
	}
	return 0;
}

int front_m(char_queue_t* this, void* returnvalue, size_t size)
{
	assert(this->count >= size);
	size_t i = 0;
	for(i=0; i < size; i++)
	{
		*(unsigned char*)(returnvalue+i)=this->buffer[(this->front+i)%this->capacity];
	}
	return 0;
}

int size_q(char_queue_t* this)
{
	return this->count;
}

int is_empty_q(char_queue_t* this)
{
	return this->count == 0;
}

void clear_q(char_queue_t * this)
{
	this->count = 0;
	this->front = 0;
	this->back = 0;
}

void delete_char_queue_t(char_queue_t* this)
{
	free(this->buffer);
	free(this);
}

