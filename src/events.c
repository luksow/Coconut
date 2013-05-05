#include <stdlib.h>
#include <string.h>

#include "coconut.h"
#include "events.h"
#include "threads.h"

event_t events_list;
pthread_mutex_t events_list_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void publish_event(event_t *event)
{
	pthread_mutex_lock(&event->cond_mutex);
	event->published = true;
	pthread_cond_broadcast(&event->cond);
	pthread_mutex_unlock(&event->cond_mutex);
}

bool c_is_event_published(const char *id)
{
	event_t *event;

	pthread_mutex_lock(&events_list_mutex);

	event = find_event(id);

	pthread_mutex_unlock(&events_list_mutex);

	if (!event)
		return false;

	return event->published == true;
}

void c_wait_event(const char *id)
{
	event_t *event;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	event = find_event(id);
	if (!event) // not found -> create new and block after
		event = create_add_event(id);

	pthread_mutex_unlock(&events_list_mutex);

	mark_self_blocked();

	pthread_mutex_lock(&event->cond_mutex);
	while (!event->published) // wait on conditional
		pthread_cond_wait(&event->cond, &event->cond_mutex);
	pthread_mutex_unlock(&event->cond_mutex);

	mark_self_unblocked();
}

void c_publish_event(const char *id)
{
	event_t *event;

	if (!running)
		return;

	pthread_mutex_lock(&events_list_mutex);

	event = find_event(id);
	if (!event)
		event = create_add_event(id);

	if (!event->published)
		publish_event(event);

	pthread_mutex_unlock(&events_list_mutex);
}
