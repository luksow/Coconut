/*
 * blocks.h - Functions for block-based testing in Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __BLOCKS_H
#define __BLOCKS_H

#include <pthread.h>
#include <stdbool.h>

#include "list.h"
#include "threads.h"

/**
 * Enum representing state space of block in Coconut
 * CREATED - created by c_set_blocks_interleaving
 * ENABLED - visited by c_begin_block
 * STARTED - started in c_begin_block
 * FINISHED - finished in c_end_block
 */
typedef enum
{
	CREATED,
	ENABLED,
	STARTED,
	FINISHED,
} BLOCK_STATE;

struct block;

/**
 * Waiting list representation for preceding blocks
 * head - list head
 * block - proceding block
 */
typedef struct
{
	list_t head;
	struct block *block;
} waiting_t;

/**
 * Block representation in Coconut
 * head - list head
 * owner - owning thread id
 * preds - preceding blocks
 * cond - conditional variable for indicating FINISHED state
 * cond_mutex - mutex for cond conditional variable
 * id - id of block
 * state - current state
 * counter - snapshot of global counter for determining order of c_begin_block
 */
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

/**
 * List of all registered blocks byc c_set_blocks_interleaving.
 */
extern block_t blocks_list;

/**
 * Mutex for blocks_list.
 */
extern pthread_mutex_t blocks_list_mutex;

/**
 * Client function for setting desired interleaving.
 */
void c_set_blocks_interleaving(const char *interleaving);

/**
 * Client function for marking block beginning.
 */
void c_begin_block(const char *id);

/**
 * Client function for marking block ending.
 */
void c_end_block();

/**
 * Client convenience function equivalent to c_begin_block(id); c_end_block();
 */
void c_one_line_block(const char *id);

/**
 * Client function for checking if block hasn't been started yet.
 */
bool c_is_before_block(const char *id);

/**
 * Client function for checking if block is currently running.
 */
bool c_is_during_block(const char *id);

/**
 * Client function for checking if block has finished running.
 */
bool c_is_after_block(const char *id);

/**
 * Marks all registered blocks as finished.
 * Should be called with blocks_list_mutex taken.
 */
void finish_all_blocks();

/**
 * Memory freeing function for blocks in blocks_list.
 */
void free_blocks_list();

#endif
