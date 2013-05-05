#ifndef __THREADS_H
#define __THREADS_H

#include <pthread.h>
#include <stdbool.h>

#include "list.h"

typedef struct
{
	list_t head;
	pthread_t id;
	bool blocked;
} thread_t;

extern thread_t threads_list;

extern pthread_mutex_t threads_list_mutex;

extern unsigned long blocked_counter;

void free_thread(thread_t *thread);

thread_t *find_thread(const pthread_t id);

bool is_thread_alive(const thread_t *thread);

void mark_self_blocked();

void mark_self_unblocked();

#endif
