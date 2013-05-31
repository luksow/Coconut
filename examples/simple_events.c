#include <pthread.h>
#include <stdio.h>

#include "coconut.h"

int some_var = 0;

void *thread1(void *dummy)
{
	c_wait_event("printed");
	some_var = 42;
}

void *thread2(void *dummy)
{
	printf("some_var: %d\n", some_var);
	c_assert_true(some_var == 42, "some_var != 42");
	c_publish_event("printed");
}

int main()
{
	pthread_t t1;
	pthread_t t2;

	c_init();

	pthread_create(&t1, NULL, &thread1, NULL);
	pthread_create(&t2, NULL, &thread2, NULL);
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);

	c_free();

	return 0;
}
