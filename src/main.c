#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

int main()
{
	stack_t s = {};
	STACK_INIT(&s, sizeof(int), 20);

	// *(volatile int*)(0) = 0;

	for(int i = 0; i < 1000; i++)
		stack_push(&s, &i);

	// return 0;

	int data = 0;
	for(int i = 0; i < 1000; i++)
	{
		// stack_pop(&s, &data);
		stack_print_err(stack_pop(&s, &data));
		// printf("cur: %d\n", data);
		if(i > 990)
			STACK_PRINT(&s);
	}

	stack_print_err(stack_chk(&s));

	stack_print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);
	stack_print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);
	stack_print_err(stack_pop(&s, &data));
	printf("cur: %d\n", data);


	stack_dtor(&s);
	return 0;
}
