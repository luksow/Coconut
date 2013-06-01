# Coconut

Coconut (COmpcat CONcurrency Unit Testing) is a library for testing and debugging concurrent applications in C/C++. It allows programmer to exercise selected interleavings and check some assertions. Execution with Coconut is deterministic and repeatable due to limited concurrency.

## Interface

Coconut provides two interfaces: named events and named blocks. Named events are useful for debugging while named blocks are great for testing. Full documented interface can be found in `src/coconut_pub.h` file.

### Named events

Named events allows to block certain thread by waiting for an event to be published. Events can be published by other threads. Simple example is provided below.

```c
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
```

### Named blocks

Named blocks allows to divide the source code into blocks and then to run them in specified order. Block of code is allowed to run only if all its predecessors finished. Simple example is provided below.

```c
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
```

### Examples

For more examples go to `examples` directory.

## Compilation & usage

Compiling is as simple as issuing make command.

```bash
$ git clone git://github.com/luksow/Coconut.git
$ cd Coconut/src/
$ make
```

Make will create a static library named `libcoconut.a`. `libcoconut.a` and `coconut_pub.h` are the only files needed to start testing applications.

Using Coconut requires compiling application with `libcoconut.a`. It should be used just like a standard static library.

```bash
$ cd ../examples
$ cp ../src/libcoconut.a ./
$ cp ../src/coconut_pub.h ./coconut.h # note simplified header name
$ gcc simple_events.c libcoconut.a -lpthread -o simple_events
$ ./simple_events
some_var: 0
[simple_events.c:thread2:17]: Assert failed: some_var != 42
```

## License

See LICENSE file.

## Contact

If you have any questions, don't hesitate to contact me:

Łukasz Sowa

web: http://lukaszsowa.pl

mail: contact [at] lukaszsowa [dot] pl