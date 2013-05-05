#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "blocks.h"
#include "coconut.h"
#include "events.h"
#include "threads.h"

pthread_t watchdog_thread;
unsigned int watchdog_tick = 1;
bool running = false;

void c_output(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

static void *watchdog(void *dummy)
{
	thread_t *tmp_thread;
	event_t *tmp_event;
	list_t *it;
	unsigned long last_blocked_counter = blocked_counter - 1;
	bool terminate;
	bool all_dead;

	while (running)
	{
		sleep(watchdog_tick);
		terminate = true;
		pthread_mutex_lock(&threads_list_mutex);

		if (terminate && list_empty(&threads_list.head)) // if empty - nothing happened -> ok
			terminate = false;

		if (terminate && last_blocked_counter != blocked_counter) // if counter changed, there was change in state -> ok
			terminate = false;

		if (terminate) // if still uncertain, check liveness
		{
			all_dead = true;
			list_for_each(it, &threads_list.head)
			{
				tmp_thread = list_entry(it, thread_t, head);
				if (is_thread_alive(tmp_thread))
				{
					all_dead = false;
					if (!tmp_thread->blocked) // thread is not blocked and alive -> ok
					{
						terminate = false;
						break;
					}
				}
			}
			if (all_dead) // all involved threads are dead -> ok
				terminate = false;
		}

		pthread_mutex_unlock(&threads_list_mutex);

		if (terminate)
		{
			c_output("Deadlock detected, fixed interleaving is probably impossible. Perhaps synchronization is correct or you should adjust watchdog tick with $C_WATCHDOG_TICK. Publishing all events...\n");
			pthread_mutex_lock(&events_list_mutex);
			list_for_each(it, &events_list.head)
			{
				tmp_event = list_entry(it, event_t, head);
				publish_event(tmp_event);
			}
			pthread_mutex_unlock(&events_list_mutex);
		}

		last_blocked_counter = blocked_counter;
	}

	return NULL;
}

static void init_c(int argc, char **argv, char **envp)
{
	char *watchdog_tick_str;
	unsigned int new_watchdog_tick;

	// mark coconut running
	running = true;

	// init static list heads
	INIT_LIST_HEAD(&events_list.head);
	INIT_LIST_HEAD(&threads_list.head);

	// read watchdog tick duration
	watchdog_tick_str = getenv("C_WATCHDOG_TICK");
	if (watchdog_tick_str && sscanf(watchdog_tick_str, "%u", &new_watchdog_tick) == 1)
		watchdog_tick = new_watchdog_tick;

	// create watchdog
	pthread_create(&watchdog_thread, NULL, watchdog, NULL);
}

static void free_c()
{
	list_t *event_it, *event_tmp_it;
	event_t *event_tmp;
	list_t *thread_it, *thread_tmp_it;
	thread_t *thread_tmp;

	running = false; // stop running additional threads

	pthread_join(watchdog_thread, NULL); // wait for watchdog to terminate

	// memory freeing
	list_for_each_safe(event_it, event_tmp_it, &events_list.head)
	{
		event_tmp = list_entry(event_it, event_t, head);
		list_del(event_it);
		free_event(event_tmp);
	}

	list_for_each_safe(thread_it, thread_tmp_it, &threads_list.head)
	{
		thread_tmp = list_entry(thread_it, thread_t, head);
		list_del(thread_it);
		free(thread_tmp);
	}
}

extern int c_main(int argc, char **argv, char **envp);

int main(int argc, char **argv, char **envp)
{
	int ret;

	init_c(argc, argv, envp);
	ret = c_main(argc, argv, envp);
	free_c();

	return ret;
}
