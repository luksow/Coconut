/*
 * coconut.h - Core functions of Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __COCONUT_H
#define __COCONUT_H

#include <stdbool.h>

/**
 * Global variable indicating if Coconut is setup and running.
 */
extern bool running;

/**
 * Client function for outputting info on stderr.
 */
void c_output(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#endif
