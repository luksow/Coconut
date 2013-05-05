#ifndef __EVENTS_H_
#define __EVENTS_H_

#include <pthread.h>
#include <stdbool.h>

#include "list.h"

typedef struct
{
	list_t head;
	pthread_mutex_t cond_mutex;
	pthread_cond_t cond;
	const char *id;
	bool published;
} event_t;

extern event_t events_list;
extern pthread_mutex_t events_list_mutex;

event_t *create_add_event(const char *id);

void free_event(event_t *event);

void publish_event(event_t *event);

bool c_is_event_published(const char *event);

void c_wait_event(const char *event);

void c_publish_event(const char *event);

#endif
