/*
 * blocks.c - Functions for block-based testing in Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "blocks.h"
#include "coconut.h"
#include "threads.h"
#include "utils.h"

block_t blocks_list;
pthread_mutex_t blocks_list_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned long block_counter = 0;

/* should be called with blocks_list_mutex taken */
static block_t *find_block(const char *id)
{
	block_t *block;
	list_t *it;

	list_for_each(it, &blocks_list.head)
	{
		block = list_entry(it, block_t, head);
		if (strcmp(block->id, id) == 0)
			return block;
	}

	return NULL;
}

static block_t *create_add_block(const char* id, char *preds[])
{
	int i;
	block_t *tmp_block;
	waiting_t *tmp_waiting;
	block_t *block;

	block = find_block(id);
	if (block)
	{
		c_output("Duplicated blocks in interleaving definition, skipping...\n");
		return NULL;
	}

	block = malloc(sizeof(block_t));
	// standard fields
	INIT_LIST_HEAD(&block->head);
	INIT_LIST_HEAD(&block->preds.head);
	pthread_cond_init(&block->cond, NULL);
	pthread_mutex_init(&block->cond_mutex, NULL);
	block->state = CREATED;
	block->id = malloc(sizeof(char) * (strlen(id) + 1));
	strcpy(block->id, id);

	// add to list
	list_add_tail(&block->head, &blocks_list.head);

	// add preds, if any
	if (preds == NULL)
		return block;

	for (i = 0; preds[i]; ++i)
	{
		tmp_block = find_block(preds[i]);
		if (!tmp_block)
			continue;

		tmp_waiting = malloc(sizeof(waiting_t));
		tmp_waiting->block = tmp_block;
		list_add_tail(&tmp_waiting->head, &block->preds.head);
	}

	return block;
}

static void free_block(block_t *block)
{
	list_t *it, *tmp_it;
	waiting_t *tmp;

	list_for_each_safe(it, tmp_it, &block->preds.head)
	{
		tmp = list_entry(it, waiting_t, head);
		list_del(it);
		free(tmp);
	}
	pthread_cond_destroy(&block->cond);
	pthread_mutex_destroy(&block->cond_mutex);
	free(block->id);
	free(block);
}

void free_blocks_list()
{
	list_t *it, *tmp_it;
	block_t *tmp;

	list_for_each_safe(it, tmp_it, &blocks_list.head)
	{
		tmp = list_entry(it, block_t, head);
		list_del(it);
		free_block(tmp);
	}
}

static void finish_block(block_t *block)
{
	pthread_mutex_lock(&block->cond_mutex);
	block->state = FINISHED;
	pthread_cond_broadcast(&block->cond);
	pthread_mutex_unlock(&block->cond_mutex);
}


/* should be called with blocks_list_mutex taken */
void finish_all_blocks()
{
	block_t *block;
	list_t *it;

	list_for_each(it, &blocks_list.head)
	{
		block = list_entry(it, block_t, head);
		finish_block(block);
	}
}

void c_set_blocks_interleaving(const char *interleaving)
{
	int i;
	int j;
	char **groups = get_tokenized(interleaving, ";");
	char **prev_elems = NULL;
	char **elems;

	if (!running)
		return;

	free_blocks_list();

	for (i = 0; groups[i]; ++i)
	{
		elems = get_tokenized(groups[i], " ");
		for (j = 0; elems[j]; ++j)
		{
			if (strlen(elems[j]) == 0)
				continue;

			pthread_mutex_lock(&blocks_list_mutex);
			create_add_block(elems[j], prev_elems);
			pthread_mutex_unlock(&blocks_list_mutex);
		}
		free_tokenized(prev_elems);
		prev_elems = elems;
	}
	free_tokenized(prev_elems);

	free_tokenized(groups);
}

void c_begin_block(const char *id)
{
	block_t *block;
	list_t *it;
	waiting_t *tmp;

	if (!running)
		return;

	pthread_mutex_lock(&blocks_list_mutex);

	block = find_block(id);

	// basic error handling
	if (!block)
	{
		c_output("Block %s to begin not found in interleaving list. Possible malfunctions.\n", id);
		pthread_mutex_unlock(&blocks_list_mutex);
		return;
	}
	if (block->state != CREATED)
	{
		c_output("Block %s already owned by other thread. Possible malfunctions.\n", id);
		pthread_mutex_unlock(&blocks_list_mutex);
		return;
	}

	// mark owner and block visited
	block->state = ENABLED;
	block->owner = pthread_self();
	block->counter = ++block_counter;

	pthread_mutex_unlock(&blocks_list_mutex);

	mark_self_blocked();

	// wait for each pred
	list_for_each(it, &block->preds.head)
	{
		tmp = list_entry(it, waiting_t, head);
		pthread_mutex_lock(&tmp->block->cond_mutex);
		while (tmp->block->state != FINISHED)
			pthread_cond_wait(&tmp->block->cond, &tmp->block->cond_mutex);
		pthread_mutex_unlock(&tmp->block->cond_mutex);
	}
	block->state = STARTED;

	mark_self_unblocked();
}

void c_end_block()
{
	block_t *block = NULL;
	block_t *tmp;
	list_t *it;
	unsigned long min_counter = ULONG_MAX;

	if (!running)
		return;

	list_for_each(it, &blocks_list.head)
	{
		tmp = list_entry(it, block_t, head);
		if (tmp->state == STARTED && pthread_equal(tmp->owner, pthread_self()) && tmp->counter < min_counter)
		{
			min_counter = tmp->counter;
			block = tmp;
		}
	}

	if (!block)
	{
		c_output("No begin block for end block. Skipping...\n");
		return;
	}

	finish_block(block);
}

void c_one_line_block(const char *id)
{
	c_begin_block(id);
	c_end_block();
}

bool c_is_before_block(const char *id)
{
	block_t *block = find_block(id);

	return !block || block->state == CREATED || block->state == ENABLED;
}

bool c_is_during_block(const char *id)
{
	block_t *block = find_block(id);

	return block && block->state == STARTED;
}

bool c_is_after_block(const char *id)
{
	block_t *block = find_block(id);

	return block && block->state == FINISHED;
}

