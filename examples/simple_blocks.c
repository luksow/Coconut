#include <pthread.h>
#include <stdio.h>

#include "coconut.h"

int some_var = 0;

void *thread1(void *dummy)
{
	c_begin_block("set");
	some_var = 42;
	c_end_block();
}

void *thread2(void *dummy)
{
	c_begin_block("print");
	printf("some_var: %d\n", some_var);
	c_assert_true(some_var == 42, "some_var != 42");
	c_end_block();
}

int main()
{
	pthread_t t1;
	pthread_t t2;

	c_init();

	// test1 - should pass
	printf("test1\n");
	some_var = 0;
	c_set_blocks_interleaving("set;print");
	pthread_create(&t1, NULL, &thread1, NULL);
	pthread_create(&t2, NULL, &thread2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	// test2 - should fail
	printf("test2\n");
	some_var = 0;
	c_set_blocks_interleaving("print;set");
	pthread_create(&t1, NULL, &thread1, NULL);
	pthread_create(&t2, NULL, &thread2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	c_free();

	return 0;
}
