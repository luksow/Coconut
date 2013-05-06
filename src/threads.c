/*
 * threads.c - Threads management for Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#include <stdlib.h>

#include "threads.h"

thread_t threads_list;
pthread_mutex_t threads_list_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long blocked_counter = 0;

extern int pthread_kill(pthread_t thread, int sig); // should be in signal.h, but buggy on some glibcs

/* should be called with threads_list_mutex taken */
static thread_t *create_add_thread(pthread_t id)
{
	thread_t *thread = malloc(sizeof(thread_t));
	thread->id = id;
	list_add_tail(&thread->head, &threads_list.head);

	return thread;
}

static void free_thread(thread_t *thread)
{
	free(thread);
}

void free_threads_list()
{
	list_t *it, *tmp_it;
	thread_t *tmp;

	list_for_each_safe(it, tmp_it, &threads_list.head)
	{
		tmp = list_entry(it, thread_t, head);
		list_del(it);
		free_thread(tmp);
	}
}

thread_t *find_thread(const pthread_t id)
{
	thread_t *thread;
	list_t *it;

	list_for_each(it, &threads_list.head)
	{
		thread = list_entry(it, thread_t, head);
		if (pthread_equal(thread->id, id))
			return thread;
	}

	return NULL;
}

static bool is_thread_alive(const thread_t *thread)
{
	return pthread_kill(thread->id, 0) == 0;
}

bool check_liveness()
{
	bool all_dead = true;
	thread_t *thread;
	list_t *it;

	list_for_each(it, &threads_list.head)
	{
		thread = list_entry(it, thread_t, head);
		if (is_thread_alive(thread))
		{
			all_dead = false;
			if (!thread->blocked) // thread is not blocked and alive -> ok
				return true;
		}
	}

	return all_dead; // if all dead -> ok
}

void mark_self_blocked()
{
	thread_t *thread;

	__sync_fetch_and_add(&blocked_counter, 1);

	pthread_mutex_lock(&threads_list_mutex);

	thread = find_thread(pthread_self());
	if (thread == NULL) // unregistered thread -> register
		thread = create_add_thread(pthread_self());
	thread->blocked = true;

	pthread_mutex_unlock(&threads_list_mutex);
}

void mark_self_unblocked()
{
	thread_t *thread;

	__sync_fetch_and_add(&blocked_counter, 1);

	pthread_mutex_lock(&threads_list_mutex);

	thread = find_thread(pthread_self()); // should not fail
	thread->blocked = false;

	pthread_mutex_unlock(&threads_list_mutex);
}

