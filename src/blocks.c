#include <string.h>

#include "blocks.h"

block_t blocks_list;
pthread_mutex_t blocks_list_mutex = PTHREAD_MUTEX_INITIALIZER;

block_t *find_block(const char *block)
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

void c_begin_block(const char *block)
{
}

void c_end_block()
{
}

void c_one_line_block(const char *block)
{
	c_begin_block(block);
	c_end_block();
}

