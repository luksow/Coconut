/*
 * coconut.h - Coconut - library for unit testing concurrent C/C++ programs
 * For more info see README file distributed with source code
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __COCONUT_H
#define __COCONUT_H

#ifndef NCOCONUT

#include <stdbool.h>

void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));


void c_wait_event(const char *event);

void c_publish_event(const char *event);

bool c_is_event_published(const char *event);


void c_set_blocks_interleaving(const char *interleaving);

void c_begin_block(const char *block);

void c_end_block();

void c_one_line_block(const char *block);

bool c_is_before_block(const char *block);

bool c_is_during_block(const char *block);

bool c_is_after_block(const char *block);

#define c_assert_true(COND, FMT, ...) do { if (!(COND)) { c_output("[%s:%s:%d]: Assert failed: " FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } } while (0)
#define c_assert_false(COND, FMT, ...) c_assert_true(!(COND), FMT, ##__VA_ARGS__)
#define c_assert_before_event(EVENT, FMT, ...) do { if (c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
#define c_assert_after_event(EVENT, FMT, ...) do { if (!c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
#define c_assert_before_block(BLOCK, FMT, ...) do { if (!c_is_before_block(BLOCK)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
#define c_assert_during_block(BLOCK, FMT, ...) do { if (!c_is_during_block(BLOCK)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
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
