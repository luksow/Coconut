/*
 * events.h - Functions for event-based testing in Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __EVENTS_H_
#define __EVENTS_H_

#include <pthread.h>
#include <stdbool.h>

#include "list.h"

/**
 * Event representation in Coconut.
 * head - list head
 * cond_mutex - mutex for cond conditional variable
 * cond - conditional variable for signalling that event was published
 * id - id string
 * published - true if event was published, false otherwise
 */
typedef struct
{
	list_t head;
	pthread_mutex_t cond_mutex;
	pthread_cond_t cond;
	const char *id;
	bool published;
} event_t;

/**
 * List of all registered events.
 */
extern event_t events_list;

/**
 * Mutex for events_list.
 */
extern pthread_mutex_t events_list_mutex;

/**
 * Memory freeing function for events in events_list.
 */
void free_events_list();

/**
 * Publishes all registered events.
 * Should be called with events_list_mutex taken.
 */
void publish_all_events();

/**
 * Client function to check if event was already published.
 */
bool c_is_event_published(const char *event);

/**
 * Client function to wait for specified event.
 */
void c_wait_event(const char *event);

/**
 * Client function to publish event.
 */
void c_publish_event(const char *event);

#endif
