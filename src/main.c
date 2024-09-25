#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct stack
{
	void* buf;
	size_t cur_ptr;
	size_t allocated_size;
	size_t elem_size;
} stack_t;

int stack_ctor(stack_t* s, size_t initial_size) // errors
{
	if(s->buf)
	{
		fprintf(stderr, "Stack is already initialized!\n");
		return -1;
	}

	s->elem_size = sizeof(int);

	size_t to_allocate = initial_size * s->elem_size;

	s->buf = realloc(s->buf, initial_size * s->elem_size); 
	if(!s->buf)
	{
		fprintf(stderr, "Allocation error!\n");
		return -1;
	}
	memset(s->buf, 0, to_allocate);

	s->allocated_size = to_allocate;

	return 0;
}


int stack_push(stack_t* s, int data)	
{
	if(!s->buf)
	{
		fprintf(stderr, "Stack buf is NULL!\n");
		return -1;
	}

	s->buf[cur_ptr++]
	

	return 0;
}

void stack_pop()
{

}


int main()
{
	stack_t s = {};


	return 0;
}
