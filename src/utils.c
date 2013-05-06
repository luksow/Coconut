#include <stdlib.h>
#include <string.h>

#include "utils.h"

static char *stptok(const char *s, char *tok, size_t toklen, const char *brk)
{
	char *lim;
	char *b;

	if (!*s)
		return NULL;

	lim = tok + toklen - 1;
	while (*s && tok < lim)
	{
		for (b = (char *) brk; *b; b++)
		{
			if (*s == *b)
			{
				*tok = 0;
				return (char *) s;
			}
		}
		*tok++ = *s++;
	}
	*tok = 0;
	return (char *) s;
}

char **get_tokenized(const char *str, const char *delims)
{
	char *stable;
	char *token;
	char **ret;
	int cnt = 0;
	int i;

	/* count tokens */
	stable = malloc(strlen(str) + 1);
	for (token = stptok(str, stable, strlen(str) + 1, delims); token; token = token[0] != '\0' ? stptok(token + 1, stable, strlen(str) + 1, delims) : NULL)
		++cnt;

	ret = malloc(sizeof(char *) * (cnt + 1));
	ret[cnt] = NULL;

	for (token = stptok(str, stable, strlen(str) + 1, delims), i = 0; token; token = token[0] != '\0' ? stptok(token + 1, stable, strlen(str) + 1, delims) : NULL, ++i)
	{
		ret[i] = malloc(strlen(stable) + 1);
		strcpy(ret[i], stable);
	}

	free(stable);

	return ret;
}

void free_tokenized(char *array[])
{
	int i;

	if (!array)
		return;

	for (i = 0; array[i]; ++i)
		free(array[i]);

	free(array);
}
