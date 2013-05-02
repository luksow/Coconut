#ifndef __LIB_COCONUT_H
#define __LIB_COCONUT_H

#ifndef NCOCONUT

void c_wait_event(const char *event);

void c_publish_event(const char *event);

void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#define ASSERT_FMT "[%s:%s:%d]: Assert failed: "
#define c_assert_true(COND, FMT, ...) do { if (!(COND)) { c_output(ASSERT_FMT FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } } while (0)
#define c_assert_false(COND, FMT, ...) do { if ((COND)) { c_output(ASSERT_FMT FMT "\n", __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__); } } while (0)

#else

#define c_wait_event(x) do {} while(0)
#define c_publish_event(x) do {} while(0)
#define c_assert_true(x, y, ...) do {} while(0)
#define c_assert_false(x, y, ...) do {} while(0)

#endif

#endif
