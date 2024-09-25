#include <stdlib.h>

#ifndef STACK_NDEBUG
	#define STACK_PRINT(s)				stack_print(s, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__);
	#define STACK_INIT(s, elem_size, initial_size)	stack_ctor(s, elem_size, initial_size,  __FILE_NAME__, __LINE__, #s);
#elif
	#define STACK_PRINT(s) ;
#endif

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
	void*	buf;
	size_t	cur_index;
	size_t	allocated_size;
	size_t	elem_size;
	size_t	last_allocation_index;

#ifndef STACK_NDEBUG
	char*	instantiated_with_name;
	char*	instantiated_at_file;
	int	instantiated_at_line;
#endif

} stack_t;


#ifndef STACK_NDEBUG
stack_status_t	stack_ctor(stack_t* s, size_t elem_size, size_t initial_size, char* file, int line, char* name);
#elif
stack_status_t	stack_ctor(stack_t* s, size_t elem_size, size_t initial_size);
#endif

stack_status_t	stack_dtor(stack_t* s);
stack_status_t	stack_push(stack_t* s, const void* data);
stack_status_t	stack_pop(stack_t* s, int* resulting_data);
stack_status_t	stack_print_err(stack_status_t status);
stack_status_t	stack_print(stack_t* s, const char* file, const int line, const char* function);

