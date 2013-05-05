#include <pthread.h>
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

typedef struct
{
	list_t head;
	pthread_t id;
	bool blocked;
} thread_t;

typedef struct
{
	list_t head;
	pthread_mutex_t cond_mutex;
	pthread_cond_t cond;
	const char *id;
	bool published;
} event_t;

typedef enum
{
	CREATED,
	NOT_STARTED,
	STARTED,
	FINISHED,
} BLOCK_STATE;

typedef struct
{
	list_t head;
	thread_t *owner;
	const char *id;
	BLOCK_STATE state;
} block_t;

event_t events_list;
block_t blocks_list; /* should be generally used as queue */
thread_t threads_list;

pthread_mutex_t events_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocks_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t threads_list_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t watchdog_thread;

unsigned int watchdog_tick = 1;
unsigned long blocked_counter = 0;
bool running = false;

event_t *create_add_event(const char *id)
{
	event_t *event = malloc(sizeof(event_t));
	event->id = id;
	event->published = false;
	pthread_cond_init(&event->cond, NULL);
	pthread_mutex_init(&event->cond_mutex, NULL);
	list_add_tail(&event->head, &events_list.head);

	return event;
}

void free_event(event_t *event)
{
	pthread_mutex_destroy(&event->cond_mutex);
	pthread_cond_destroy(&event->cond);
	free(event);
}

/* should be called with events_list_mutex taken */
static event_t *find_event(const char *id)
{
	event_t *event;
	list_t *it;

	list_for_each(it, &events_list.head)
	{
		event = list_entry(it, event_t, head);
		if (strcmp(event->id, id) == 0)
			return event;
	}

	return NULL;
}

static void publish_event(event_t *event)
{
	pthread_mutex_lock(&event->cond_mutex);
	event->published = true;
	pthread_cond_broadcast(&event->cond);
	pthread_mutex_unlock(&event->cond_mutex);
}

/* should be called with blocks_list_mutex taken */
static block_t *find_block(const char *block)
{
	block_t *tmp;
	list_t *it;

	list_for_each(it, &blocks_list.head)
	{
		tmp = list_entry(it, block_t, head);
		if (strcmp(tmp->id, block) == 0)
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

void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

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
	list_t *thread_it, *thread_tmp_it;
	thread_t *thread_tmp;

	running = false; /* stop running additional threads */

	pthread_join(watchdog_thread, NULL); /* wait for watchdog to terminate */

	/* memory freeing */
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

void c_begin_block(const char *block)
{
//	block_t *tmp_block;

	if (!running)
		return;

	pthread_mutex_lock(&blocks_list_mutex);

	if (find_block(block))
	{
		c_output("Duplicated block id, library malfunctions possible\n");
		return;
	}

	pthread_mutex_unlock(&blocks_list_mutex);
}

void c_end_block()
{
}

void c_one_line_block(const char *block)
{
	c_begin_block(block);
	c_end_block();
}

bool c_is_event_published(const char *event)
{
	event_t *tmp_event;

	pthread_mutex_lock(&events_list_mutex);

	tmp_event = find_event(event);

	pthread_mutex_unlock(&events_list_mutex);

	if (!tmp_event)
		return false;

	return tmp_event->published == true;
}

void c_wait_event(const char *event)
{
	event_t *tmp;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	tmp = find_event(event);
	if (!tmp) /* not found -> create new and block after */
		tmp = create_add_event(event);

	pthread_mutex_unlock(&events_list_mutex);

	mark_self_blocked();

	pthread_mutex_lock(&tmp->cond_mutex);
	while (!tmp->published) /* wait on conditional */
		pthread_cond_wait(&tmp->cond, &tmp->cond_mutex);
	pthread_mutex_unlock(&tmp->cond_mutex);

	mark_self_unblocked();
}

void c_publish_event(const char *event)
{
	event_t *tmp;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	tmp = find_event(event);
	if (!tmp)
		tmp = create_add_event(event);

	if (!tmp->published)
		publish_event(tmp);

	pthread_mutex_unlock(&events_list_mutex);
}

