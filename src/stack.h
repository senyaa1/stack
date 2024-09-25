#include <stdlib.h>

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


stack_status_t stack_ctor(stack_t* s, size_t elem_size, size_t initial_size);
stack_status_t stack_dtor(stack_t* s);
stack_status_t stack_push(stack_t* s, const void* data);
stack_status_t stack_pop(stack_t* s, int* resulting_data);
stack_status_t stack_print_err(stack_status_t status);
