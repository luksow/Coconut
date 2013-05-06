/*
 * threads.h - Threads management for Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __THREADS_H
#define __THREADS_H

#include <pthread.h>
#include <stdbool.h>

#include "list.h"

/**
 * Thread representation in Coconut.
 * head - list head
 * id - thread id (as in PTHREADS)
 * blocked - boolean with current execution state
 */
typedef struct
{
	list_t head;
	pthread_t id;
	bool blocked;
} thread_t;

/**
 * List of all registered threads - threads which used events or blocks.
 */
extern thread_t threads_list;

/**
 * Mutex for threads_list.
 */
extern pthread_mutex_t threads_list_mutex;

/**
 * Counter for light registering threads activities. Used to detect deadlocks.
 */
extern unsigned long blocked_counter;

/**
 * Memory freeing function for threads in threads_list.
 */
void free_threads_list();

/**
 * Finds thread based on thread id. Returns NULL if not found.
 */
thread_t *find_thread(const pthread_t id);

/**
 * Checks liveness of all registered threads. Returns true if all threads finished working or at least one is in running state.
 * Should be called with threads_list_mutex taken.
 */
bool check_liveness();

/**
 * Marks calling thread as blocked.
 * Should be called with threads_list_mutex taken.
 */
void mark_self_blocked();

/**
 * Marks calling thread as unblocked.
 */
void mark_self_unblocked();

#endif
