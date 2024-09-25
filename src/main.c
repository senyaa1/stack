#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static const size_t MIN_STACK_SIZE = 16;

typedef enum stack_status
{
	STACK_OK = 0,
	STACK_ERR_ALLOC,
	STACK_ERR_ARGNULL,
	STACK_ERR_INITIALIZED,
	STACK_ERR_EMPTY,
} stack_status_t;


typedef struct stack
{
	void* buf;
	size_t cur_index;
	size_t allocated_size;
	size_t elem_size;
	size_t last_allocation_index;
} stack_t;

void print_err(stack_status_t status)
{
	if(status == STACK_OK) return;

	#define STATSTR(stat)			\
		case stat:			\
			cur_status = #stat;	\

	char* cur_status = NULL;
	switch(status)
	{
		STATSTR(STACK_OK)
		STATSTR(STACK_ERR_ALLOC)
		STATSTR(STACK_ERR_ARGNULL)
		STATSTR(STACK_ERR_INITIALIZED)
		STATSTR(STACK_ERR_EMPTY)
	}

	printf("Stack status: %s\n", cur_status);

	#undef STATSTR
}

static inline stack_status_t increase_alloc(stack_t *s)
{
	if(!s || !s->buf) return STACK_ERR_ARGNULL;

	// printf("allocating: %ld\n", s->allocated_size);

	s->allocated_size *= 2;
	s->buf = realloc(s->buf, s->allocated_size);
	s->last_allocation_index = s->allocated_size / s->elem_size;


	if(!s->buf) return STACK_ERR_ALLOC;

	return STACK_OK;
}


inline static stack_status_t decrease_alloc(stack_t *s)
{
	if(!s || !s->buf) return STACK_ERR_ARGNULL;

	// printf("deallocating: %ld\n", s->allocated_size);

	s->allocated_size /= 2;
	s->buf = realloc(s->buf, s->allocated_size);
	s->last_allocation_index = s->allocated_size / s->elem_size;
	if(!s->buf) return STACK_ERR_ALLOC;

	return STACK_OK;
}

inline static stack_status_t maybe_decrease_alloc(stack_t *s)
{
	// printf("cur index: %ld\tlast_alloc: %ld\n", s->cur_index, s->last_allocation_index);
	if (s->cur_index <= (s->last_allocation_index / 4) && s->allocated_size / s->elem_size / 4 > MIN_STACK_SIZE)
	{
		return decrease_alloc(s);
	}

	return STACK_OK;
}

stack_status_t stack_ctor(stack_t* s, size_t elem_size, size_t initial_size) // errors
{
	if(!s) return STACK_ERR_ARGNULL;
	if(s->buf) return STACK_ERR_INITIALIZED;

	s->elem_size = elem_size;
	
	size_t to_allocate = (initial_size < MIN_STACK_SIZE ? MIN_STACK_SIZE : initial_size) * s->elem_size;
	s->buf = realloc(s->buf, to_allocate);

	if(!s->buf) return STACK_ERR_ALLOC;

	s->allocated_size = to_allocate;

	// memset(s->buf, 0, to_allocate);


	return STACK_OK;
}

stack_status_t stack_dtor(stack_t* s)
{
	if(!s || !s->buf) return STACK_ERR_ARGNULL;

	free(s->buf);
	memset(s, 0, sizeof(stack_t));

	return STACK_OK;
}

stack_status_t stack_push(stack_t* s, const void* data)	
{
	if(!s || !s->buf) return STACK_ERR_ARGNULL;

	if((s->cur_index * s->elem_size) >= s->allocated_size)
		if(increase_alloc(s)) return STACK_ERR_ALLOC;

	memcpy((s->buf + (s->cur_index++) * s->elem_size), data, s->elem_size);

	return STACK_OK;
}

stack_status_t stack_pop(stack_t* s, int* resulting_data)	
{
	if(!s || !s->buf || !resulting_data) return STACK_ERR_ARGNULL;

	if(s->cur_index <= 0) return STACK_ERR_EMPTY;

	if(maybe_decrease_alloc(s)) return STACK_ERR_ALLOC;

	memcpy(resulting_data, (s->buf + (--s->cur_index) * s->elem_size), s->elem_size);

	return STACK_OK;
}


int main()
{
	stack_t s = {};
	stack_ctor(&s, sizeof(int), 20);

	for(int i = 0; i < 1000; i++)
		stack_push(&s, &i);



	int data = 0;
	for(int i = 0; i < 1000; i++)
	{
		// stack_pop(&s, &data);
		print_err(stack_pop(&s, &data));
		printf("cur: %d\n", data);
	}


	print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);
	print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);
	print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);

	stack_dtor(&s);
	return 0;
}
