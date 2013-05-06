/*
 * utils.h - Utils for Coconut library
 * Copyright (C) 2013 Lukasz Sowa <contact@lukaszsowa.pl>
 */

#ifndef __UTILS_H
#define __UTILS_H

/**
 * Splits str on delims. Result should be later freed with free_tokenized.
 */
char **get_tokenized(const char *str, const char *delims);

/**
 * Cleaning function for get_tokenized.
 */
void free_tokenized(char *array[]);

#endif
