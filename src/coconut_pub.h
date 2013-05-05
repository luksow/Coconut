#ifndef __COCONUT_H
#define __COCONUT_H

#ifndef NCOCONUT

#include <stdbool.h>

void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

void c_wait_event(const char *event);

void c_publish_event(const char *event);

bool c_is_event_published(const char *event);

#define c_assert_true(COND, FMT, ...) do { if (!(COND)) { c_output("[%s:%s:%d]: Assert failed: " FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } } while (0)
#define c_assert_false(COND, FMT, ...) c_assert_true(!(COND), FMT, ##__VA_ARGS__)
#define c_assert_before_event(EVENT, FMT, ...) do { if (c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)
#define c_assert_after_event(EVENT, FMT, ...) do { if (!c_is_event_published(EVENT)) c_assert_true(0, FMT, ##__VA_ARGS__); } while (0)

#else

#define c_wait_event(x) do {} while(0)
#define c_publish_event(x) do {} while(0)
#define c_assert_true(x, y, ...) do {} while(0)
#define c_assert_false(x, y, ...) do {} while(0)
#define c_assert_before_event(EVENT, FMT, ...) do {} while (0)
#define c_assert_after_event(EVENT, FMT, ...) do {} while (0)

#endif

#endif