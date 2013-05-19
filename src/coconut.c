/*
 * coconut.c - Core functions of Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blocks.h"
#include "coconut.h"
#include "events.h"
#include "threads.h"

pthread_mutex_t output_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t watchdog_thread;
unsigned int watchdog_tick = 1;
bool running = false;

void c_output(const char *format, ...)
{
	if (!running)
		return;

	pthread_mutex_lock(&output_mutex);
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	pthread_mutex_unlock(&output_mutex);
}

static void *watchdog(void *dummy)
{
	unsigned long last_blocked_counter = blocked_counter - 1;
	bool terminate;

	while (running)
	{
		terminate = true;

		pthread_mutex_lock(&threads_list_mutex);

		if (terminate && list_empty(&threads_list.head)) // if empty - nothing happened -> ok
			terminate = false;

		if (terminate && last_blocked_counter != blocked_counter) // if counter changed, there was change in state -> ok
			terminate = false;

		if (terminate && check_liveness()) // if still uncertain, check liveness
				terminate = false;

		pthread_mutex_unlock(&threads_list_mutex);

		if (terminate) // finally -> terminate
		{
			c_output("Deadlock detected, fixed interleaving is probably impossible. Perhaps synchronization is correct or you should adjust watchdog tick with $C_WATCHDOG_TICK. Publishing all events, finishing all blocks...\n");

			pthread_mutex_lock(&events_list_mutex);
			publish_all_events();
			pthread_mutex_unlock(&events_list_mutex);

			pthread_mutex_lock(&blocks_list_mutex);
			finish_all_blocks();
			pthread_mutex_unlock(&blocks_list_mutex);
		}

		last_blocked_counter = blocked_counter;
		sleep(watchdog_tick);
	}

	return NULL;
}

void c_set_watchdog_tick(unsigned int tick)
{
	watchdog_tick = tick;
}

void c_init()
{
	char *disable_str;
	int disable_val;
	char *watchdog_tick_str;
	unsigned int new_watchdog_tick;

	disable_str = getenv("C_DISABLE");
	if (disable_str && sscanf(disable_str, "%d", &disable_val) == 1)
		if (disable_val)
			return; // disabling requested, so quitting

	// mark coconut running
	running = true;

	// init static list heads
	INIT_LIST_HEAD(&events_list.head);
	INIT_LIST_HEAD(&threads_list.head);
	INIT_LIST_HEAD(&blocks_list.head);

	// read watchdog tick duration
	watchdog_tick_str = getenv("C_WATCHDOG_TICK");
	if (watchdog_tick_str && sscanf(watchdog_tick_str, "%u", &new_watchdog_tick) == 1)
		c_set_watchdog_tick(new_watchdog_tick);

	// create watchdog
	pthread_create(&watchdog_thread, NULL, watchdog, NULL);
}

void c_free()
{
	if (!running)
		return;

	running = false; // stop running additional threads

	pthread_join(watchdog_thread, NULL); // wait for watchdog to terminate

	// memory freeing
	free_threads_list();
	free_events_list();
	free_blocks_list();
}

