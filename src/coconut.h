#ifndef __COCONUT_H
#define __COCONUT_H

#include <stdbool.h>

extern bool running;

void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#endif
