#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>

#include "stack.h"
#include "crc.h"
#include "debugger.h"
#include "colors.h"

static const size_t MIN_STACK_SIZE = 16;

#ifdef STACK_ENABLE_CANARIES
static uint64_t canaryval = 0x0;

static void init_canary(void* canaryptr, size_t sz)
{
	// srand(time(NULL));

	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, canaryptr, sz);
	close(fd);
}
#endif

static void* get_buf_ptr(stack_t* stack)
{
	return stack->buf;
}

static void set_buf_ptr(stack_t* stack, void* new_ptr)
{
	stack->buf = (void*)((size_t)new_ptr);
}

#ifdef STACK_ENABLE_CANARIES
static stack_status_t check_canary(stack_t* stack)
{
	if(canaryval == 0x0)
		init_canary(&canaryval, sizeof(canaryval));

	if(stack->canary1 != canaryval || stack->canary2 != canaryval)
		return STACK_ERR_CANARY;

	return STACK_OK;
}
static void set_canary(stack_t* stack)
{
	if(canaryval == 0x0)
		init_canary(&canaryval, sizeof(canaryval));

	stack->canary1 = canaryval;
	stack->canary2 = canaryval;
}
#endif

static stack_status_t check_hash(stack_t* stack)
{	
	uint64_t crc = crc64(stack, sizeof(stack_t));

	return STACK_OK;
}


static stack_status_t increase_alloc(stack_t *stack)
{
	if(!stack || !get_buf_ptr(stack)) return STACK_ERR_ARGNULL;

	stack->allocated_size *= 2;

	set_buf_ptr(stack, realloc(get_buf_ptr(stack), stack->allocated_size));
	if(!get_buf_ptr(stack)) return STACK_ERR_ALLOC;

	stack->last_allocation_index = stack->allocated_size / stack->elem_size;

	return STACK_OK;
}


static stack_status_t decrease_alloc(stack_t *stack)
{
	if(!stack || !get_buf_ptr(stack)) return STACK_ERR_ARGNULL;

	stack->allocated_size /= 2;
	set_buf_ptr(stack, realloc(get_buf_ptr(stack), stack->allocated_size));
	if(!get_buf_ptr(stack)) return STACK_ERR_ALLOC;

	stack->last_allocation_index = stack->allocated_size / stack->elem_size;

	return STACK_OK;
}

inline static stack_status_t maybe_decrease_alloc(stack_t *stack)
{
	if (stack->cur_index <= (stack->last_allocation_index / 2) - 1 && stack->allocated_size / stack->elem_size / 2 - 1 > MIN_STACK_SIZE)
		return decrease_alloc(stack);

	return STACK_OK;
}

stack_status_t stack_print(stack_t* stack, const char* file, const int line, const char* function)
{
	if(!stack) return STACK_ERR_ARGNULL;

#ifndef STACK_NDEBUG

	printf( "stack_t %s" BLUE " [%p]. " RESET 
		"Instantiated at " GREEN UNDERLINE "%s:%d" RESET 
		", printing from: " CYAN UNDERLINE "%s:%d (%s)\n" RESET,
		stack->instantiated_with_name, stack, stack->instantiated_at_file, stack->instantiated_at_line,
		file, line, function);

	size_t cap = stack->allocated_size / stack->elem_size;

	printf("\tcnt \t\t= " YELLOW "%lu\n" RESET, stack->cur_index);
	printf("\tcapacity \t= " YELLOW "%lu\n" RESET, cap);
	printf("\tdata" BLUE " [%p]\n" RESET, get_buf_ptr(stack));

	for(size_t i = 0; i < cap; i++)
	{
		if(i < stack->cur_index)
			printf(RED "\t\t*\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)get_buf_ptr(stack))[i]);
		else
			printf(GREEN "\t\t\t[%lu]\t" YELLOW "= %d\n" RESET, i, ((int*)get_buf_ptr(stack))[i]);
	}
#endif
	return STACK_OK;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wwrite-strings"
stack_status_t stack_print_err(stack_status_t status)
{
	if(status == STACK_OK) return status;

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
		STATSTR(STACK_ERR_UNINITIALIZED)
		STATSTR(STACK_ERR_HASH)
		STATSTR(STACK_ERR_CANARY)
		default:
			cur_status = "STACK_ERR_UNHANDLED";
		break;
	}

	if(status)
		printf("Stack status: " RED "%s" RESET "\n", cur_status);
	else
		printf("Stack status: " GREEN "%s" RESET "\n", cur_status);


	return status;

	#undef STATSTR	
}
#pragma GCC diagnostic pop


#ifndef STACK_NDEBUG
stack_status_t	stack_ctor(stack_t* stack, size_t elem_size, size_t initial_size, const char* file, const int line, const char* name)
#elif
stack_status_t	stack_ctor(stack_t* stack, size_t elem_size, size_t initial_size)
#endif
{
	setup_signals();

	if(!stack) return STACK_ERR_ARGNULL;
	if(get_buf_ptr(stack)) return STACK_ERR_INITIALIZED;

	stack->elem_size = elem_size;
	
	size_t to_allocate = (initial_size < MIN_STACK_SIZE ? MIN_STACK_SIZE : initial_size) * stack->elem_size;

	set_buf_ptr(stack, calloc(1, to_allocate));
	// fprintf(stderr, "created buf at %p\n", get_buf_ptr(stack));

	if(!get_buf_ptr(stack)) return STACK_ERR_ALLOC;

	stack->allocated_size = to_allocate;

	// memset(get_buf_ptr(stack), 0, to_allocate);


#ifdef STACK_ENABLE_CANARIES
	set_canary(stack);
#endif


#ifndef STACK_NDEBUG
	stack->instantiated_at_file = (char*)file;
	stack->instantiated_at_line = (int)line;
	stack->instantiated_with_name = (char*)name;
#endif

	return STACK_OK;
}

stack_status_t stack_dtor(stack_t* stack)
{
	if(!stack || !get_buf_ptr(stack)) return STACK_ERR_ARGNULL;

	free(get_buf_ptr(stack));
	memset(stack, 0, sizeof(stack_t));

	return STACK_OK;
}

stack_status_t stack_push(stack_t* stack, const void* data)	
{
	if(!stack || !get_buf_ptr(stack)) return STACK_ERR_ARGNULL;

	if((stack->cur_index * stack->elem_size) >= stack->allocated_size)
		if(increase_alloc(stack)) return STACK_ERR_ALLOC;

	memcpy(((char*)get_buf_ptr(stack) + (stack->cur_index++) * stack->elem_size), data, stack->elem_size);

	return STACK_OK;
}

stack_status_t stack_pop(stack_t* stack, void* resulting_data)	
{
	if(!stack || !get_buf_ptr(stack) || !resulting_data) return STACK_ERR_ARGNULL;
	if(stack->cur_index <= 0) return STACK_ERR_EMPTY;
	if(maybe_decrease_alloc(stack)) return STACK_ERR_ALLOC;

	memcpy((char*)resulting_data, ((char*)get_buf_ptr(stack) + (--stack->cur_index) * stack->elem_size), stack->elem_size);

	return STACK_OK;
}

stack_status_t stack_chk(stack_t* stack)
{
	if(check_ptr(stack, "r")) return STACK_ERR_UNINITIALIZED;
	if(check_ptr(get_buf_ptr(stack), "r")) return STACK_ERR_UNINITIALIZED;
	if(check_hash(stack)) return STACK_ERR_HASH;

#ifdef STACK_ENABLE_CANARIES
	if(check_canary(stack)) return STACK_ERR_CANARY;
#endif

	return STACK_OK;
}
