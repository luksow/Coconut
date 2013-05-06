#ifndef __BLOCKS_H
#define __BLOCKS_H

#include <pthread.h>
#include <stdbool.h>

#include "list.h"
#include "threads.h"

typedef enum
{
	CREATED,
	ENABLED,
	STARTED,
	FINISHED,
} BLOCK_STATE;

struct block;

typedef struct
{
	list_t head;
	struct block *block;
} waiting_t;

typedef struct block
{
	list_t head;
	pthread_t owner;
	waiting_t preds;
	pthread_cond_t cond;
	pthread_mutex_t cond_mutex;
	char *id;
	BLOCK_STATE state;
	unsigned long counter;
} block_t;

extern block_t blocks_list;
extern pthread_mutex_t blocks_list_mutex;

void c_set_blocks_interleaving(const char *interleaving);

void c_begin_block(const char *id);

void c_end_block();

void c_one_line_block(const char *id);

bool c_is_before_block(const char *id);

bool c_is_during_block(const char *id);

bool c_is_after_block(const char *id);

void finish_all_blocks();

void free_blocks_list();

#endif
