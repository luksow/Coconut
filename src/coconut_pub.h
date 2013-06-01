/*
 * coconut.h - Coconut - library for unit testing concurrent C/C++ programs
 * For more info see README file distributed with source code
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __COCONUT_H
#define __COCONUT_H

#ifndef NCOCONUT

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Initializing function.
 * Should be called before any other Coconut functions.
 */
void c_init();

/**
 * Resource freeing function.
 * Should be called after any other Coconut functions.
 */
void c_free();

/**
 * Sets watchdog tick duration just as environmental variable C_WATCHDOG_TICK.
 * Takes precedence over C_WATCHDOG_TICK variable.
 */
void c_set_watchdog_tick(unsigned int tick);

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
 * should be delimited with ',', sequences for sequential execution with ';'.
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
 * Helper for c_cond_block macro. Do not use. See below.
 */
bool c_begin_block_bool(const char *block, bool cond);

/**
 * Helper for c_cond_block macro. Do not use. See below.
 */
bool c_end_block_bool(bool cond);

/**
 * Allows for c_begin_block(BLOCK) to occur just before conditional check on
 * COND (for ex. in if statement) and c_end_block() just after check. Returns
 * evaluated COND.
 * Guarantees proper order and evaluating COND just once due to sequence
 * points.
 */
#define c_cond_block(COND, BLOCK) (c_begin_block_bool(BLOCK, false) || c_end_block_bool(COND))

/**
 * Function for safe outputting to stderr.
 */
void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

/**
 * c_output wrapper with beautification
 */
#define c_out(FMT, ...) do { c_output("[%s:%s:%d]: " FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } while (0)

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

#define c_init() do {} while(0)
#define c_free() do {} while(0)

#define c_set_watchdog_tick(x) do {} while (0)

#define c_output(x, ...) do {} while(0)
#define c_out(x, ...) do {} while (0)

#define c_wait_event(x) do {} while(0)
#define c_publish_event(x) do {} while(0)
#define c_is_event_published(x) do {} while(0)

#define c_set_blocks_interleaving(x) do {} while(0)
#define c_begin_block(x) do {} while(0)
#define c_end_block() do {} while(0)
#define c_cond_block(COND, BLOCK) COND

#define c_assert_true(x, y, ...) do {} while(0)
#define c_assert_false(x, y, ...) do {} while(0)
#define c_assert_before_event(EVENT, FMT, ...) do {} while (0)
#define c_assert_after_event(EVENT, FMT, ...) do {} while (0)
#define c_assert_before_block(BLOCK, FMT, ...) do {} while (0)
#define c_assert_during_block(BLOCK, FMT, ...) do {} while (0)
#define c_assert_after_block(BLOCK, FMT, ...) do {} while (0)

#endif

#ifdef __cplusplus
}
#endif

#endif
