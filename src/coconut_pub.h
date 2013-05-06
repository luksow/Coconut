/*
 * coconut.h - Coconut - library for unit testing concurrent C/C++ programs
 * For more info see README file distributed with source code
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __COCONUT_H
#define __COCONUT_H

#ifndef NCOCONUT

#include <stdbool.h>

/**
 * Blocks calling thread until event is published.
 */
void c_wait_event(const char *event);

/**
 * Unblocks all threads waiting for event.
 */
void c_publish_event(const char *event);

/**
 * Returns true if event was published.
 */
bool c_is_event_published(const char *event);

/**
 * Sets desired interleaving of blocks. Sequence that may run concurrently
 * should be delimited with ' ', sequences for sequential execution with ';'.
 */
void c_set_blocks_interleaving(const char *interleaving);

/**
 * Marks beginning of block.
 */
void c_begin_block(const char *block);

/**
 * Marks end of block.
 */
void c_end_block();

/**
 * Convenience function equivalent to c_begin_block(id); c_end_block(). That
 * allows for similar behaviour as event API.
 */
void c_one_line_block(const char *block);

/**
 * Checks if block has not been started yet.
 */
bool c_is_before_block(const char *block);

/**
 * Checks if block has been started and has not finished yet.
 */
bool c_is_during_block(const char *block);

/**
 * Check if block has finished.
 */
bool c_is_after_block(const char *block);

/**
 * Function for safe outputting to stderr.
 */
void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * Asserts that COND is true. Prints passed message otherwise.
 */
#define c_assert_true(COND, FMT, ...) do { if (!(COND)) { c_output("[%s:%s:%d]: Assert failed: " FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } } while (0)
/**
 * Asserts that COND is false. Prints passed message otherwise.
 */
#define c_assert_false(COND, FMT, ...) c_assert_true(!(COND), FMT, ##__VA_ARGS__)
/**
 * Asserts that EVENT has not been published yet. Prints passed message otherwise.
 */
#define c_assert_before_event(EVENT, FMT, ...) do { if (c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
/**
 * Asserts that EVENT has been published. Prints passed message otherwise.
 */
#define c_assert_after_event(EVENT, FMT, ...) do { if (!c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
/**
 * Asserts that BLOCK has not been started yet. Prints passed message otherwise.
 */
#define c_assert_before_block(BLOCK, FMT, ...) do { if (!c_is_before_block(BLOCK)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
/**
 * Asserts that BLOCK has been started but has not finished yet. Prints passed message otherwise.
 */
#define c_assert_during_block(BLOCK, FMT, ...) do { if (!c_is_during_block(BLOCK)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
/**
 * Asserts that BLOCK has finished. Prints passed message otherwise.
 */
#define c_assert_after_block(BLOCK, FMT, ...) do { if (!c_is_after_block(BLOCK)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)

#else

#define c_output(x, ...) do {} while(0)

#define c_wait_event(x) do {} while(0)
#define c_publish_event(x) do {} while(0)
#define c_is_event_published(x) do {} while(0)

#define c_set_blocks_interleaving(x) do {} while(0)
#define c_begin_block(x) do {} while(0)
#define c_end_block() do {} while(0)
#define c_one_line_block(x) do {} while(0)

#define c_assert_true(x, y, ...) do {} while(0)
#define c_assert_false(x, y, ...) do {} while(0)
#define c_assert_before_event(EVENT, FMT, ...) do {} while (0)
#define c_assert_after_event(EVENT, FMT, ...) do {} while (0)
#define c_assert_before_block(BLOCK, FMT, ...) do {} while (0)
#define c_assert_during_block(BLOCK, FMT, ...) do {} while (0)
#define c_assert_after_block(BLOCK, FMT, ...) do {} while (0)

#endif

#endif
