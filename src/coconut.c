
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "coconut.h"
#include "list.h"

extern int pthread_kill(pthread_t thread, int sig); /* should be in signal.h, but buggy on some glibcs */

typedef struct list_head list_t;

typedef struct {
	list_t head;
	sem_t waiting_sem;
} waiting_t;

typedef struct {
	list_t head;
	waiting_t waitings_list;
	const char *id;
	bool published;
} event_t;

typedef struct {
	list_t head;
	pthread_t id;
	bool blocked;
} thread_t;

event_t events_list;
thread_t threads_list;

pthread_mutex_t events_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_list_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t watchdog_thread;

unsigned int watchdog_tick = 1;
unsigned long blocked_counter = 0;
bool running = false;

static event_t *find_event(const char *event)
{
	event_t *tmp;
	list_t *it;

	list_for_each(it, &events_list.head)
	{
		tmp = list_entry(it, event_t, head);
		if (strcmp(tmp->id, event) == 0)
			return tmp;
	}

	return NULL;
}

static thread_t *find_thread(const pthread_t thread)
{
	thread_t *tmp;
	list_t *it;

	list_for_each(it, &threads_list.head)
	{
		tmp = list_entry(it, thread_t, head);
		if (pthread_equal(tmp->id, thread))
			return tmp;
	}

	return NULL;
}

static bool is_thread_alive(const thread_t *thread)
{
	return pthread_kill(thread->id, 0) == 0;
}

static void mark_self_blocked()
{
	thread_t *tmp;

	__sync_fetch_and_add(&blocked_counter, 1);

	pthread_mutex_lock(&threads_list_mutex);

	tmp = find_thread(pthread_self());
	if (tmp == NULL) /* unregistered thread -> register */
	{
		tmp = malloc(sizeof(thread_t));
		tmp->id = pthread_self();
		list_add_tail(&tmp->head, &threads_list.head);
	}
	tmp->blocked = true;

	pthread_mutex_unlock(&threads_list_mutex);
}

static void mark_self_unblocked()
{
	thread_t *tmp;

	__sync_fetch_and_add(&blocked_counter, 1);

	pthread_mutex_lock(&threads_list_mutex);

	tmp = find_thread(pthread_self()); /* should not fail */
	tmp->blocked = false;

	pthread_mutex_unlock(&threads_list_mutex);
}

static void publish_existing_event(event_t *event)
{
	list_t *it;
	waiting_t *tmp;

	event->published = true;
	list_for_each(it, &event->waitings_list.head)
	{
		tmp = list_entry(it, waiting_t, head);
		sem_post(&tmp->waiting_sem);
	}
}

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

		if (terminate && list_empty(&threads_list.head)) /* if empty - nothing happened -> ok */
			terminate = false;

		if (terminate && last_blocked_counter != blocked_counter) /* if counter changed, there was change in state -> ok */
			terminate = false;

		if (terminate) /* if still uncertain, check liveness */
		{
			all_dead = true;
			list_for_each(it, &threads_list.head)
			{
				tmp_thread = list_entry(it, thread_t, head);
				if (is_thread_alive(tmp_thread))
				{
					all_dead = false;
					if (!tmp_thread->blocked) /* thread is not blocked and alive -> ok */
					{
						terminate = false;
						break;
					}
				}
			}
			if (all_dead) /* all involved threads are dead -> ok */
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
				publish_existing_event(tmp_event);
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

	/* mark coconut running */
	running = true;

	/* init static list heads */
	INIT_LIST_HEAD(&events_list.head);
	INIT_LIST_HEAD(&threads_list.head);

	/* read watchdog tick duration */
	watchdog_tick_str = getenv("C_WATCHDOG_TICK");
	if (watchdog_tick_str && sscanf(watchdog_tick_str, "%u", &new_watchdog_tick) == 1)
		watchdog_tick = new_watchdog_tick;

	/* create watchdog */
	pthread_create(&watchdog_thread, NULL, watchdog, NULL);
}

static void free_c()
{
	list_t *event_it, *event_tmp_it;
	event_t *event_tmp;
	list_t *waiting_it, *waiting_tmp_it;
	waiting_t *waiting_tmp;
	list_t *thread_it, *thread_tmp_it;
	thread_t *thread_tmp;

	running = false; /* stop running additional threads */

	pthread_join(watchdog_thread, NULL); /* wait for watchdog to terminate */

	/* memory freeing */
	list_for_each_safe(event_it, event_tmp_it, &events_list.head)
	{
		event_tmp = list_entry(event_it, event_t, head);
		list_del(event_it);
		list_for_each_safe(waiting_it, waiting_tmp_it, &event_tmp->waitings_list.head)
		{
			waiting_tmp = list_entry(waiting_it, waiting_t, head);
			list_del(waiting_it);
			sem_destroy(&waiting_tmp->waiting_sem);
			free(waiting_tmp);
		}
		free(event_tmp);
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

void c_wait_event(const char *event)
{
	event_t *tmp;
	waiting_t *waiting = NULL;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	tmp = find_event(event);

	if (tmp && !tmp->published) /* we should block */
	{
		waiting = malloc(sizeof(waiting_t));
		sem_init(&waiting->waiting_sem, 0, 0); /* initial = 0 for immediate block */
		list_add_tail(&waiting->head, &tmp->waitings_list.head);
	}
	else if (!tmp)/* not found -> create new and block */
	{
		tmp = malloc(sizeof(event_t));
		tmp->id = event;
		tmp->published = false;
		INIT_LIST_HEAD(&tmp->waitings_list.head);
		list_add_tail(&tmp->head, &events_list.head);

		waiting = malloc(sizeof(waiting_t));
		sem_init(&waiting->waiting_sem, 0, 0); /* initial = 0 for immediate block */
		list_add_tail(&waiting->head, &tmp->waitings_list.head);

	}
	/* else if (tmp && tmp->published) -> ignore */

	pthread_mutex_unlock(&events_list_mutex);

	mark_self_blocked();
	if (waiting)
		sem_wait(&waiting->waiting_sem);
	mark_self_unblocked();
}

void c_publish_event(const char *event)
{
	event_t *tmp;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	tmp = find_event(event);

	if (tmp) /* unlock all */
	{
		publish_existing_event(tmp);
	}
	else /* add new in case someone subscribe */
	{
		tmp = malloc(sizeof(event_t));
		tmp->id = event;
		tmp->published = true;
		INIT_LIST_HEAD(&tmp->waitings_list.head);

		list_add_tail(&tmp->head, &events_list.head);
	}

	pthread_mutex_unlock(&events_list_mutex);
}

