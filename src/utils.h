#ifndef __UTILS_H
#define __UTILS_H

char **get_tokenized(const char *str, const char *delims);

void free_tokenized(char *array[]);

#endif
