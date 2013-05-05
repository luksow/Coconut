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

typedef struct
{
	list_t head;
	thread_t *owner;
	const char *id;
	BLOCK_STATE state;
} block_t;

extern block_t blocks_list;
extern pthread_mutex_t blocks_list_mutex;

#endif
