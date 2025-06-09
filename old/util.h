#ifndef UTIL_H
#define UTIL_H

#include "types.h"

void *memcpy(void *dest, const void *src, size_t n);
int memcmp(void *dest, void *src, int n);
void memset(void *start, unsigned char val, unsigned long size) ;

int strncmp(const char *s1, const char *s2, size_t maxlen);

int strlen(const char *s);

char *strncpy(char *dest, const char *src, size_t n);

/* same as strncpy(), but also null terminates the string */
char *strlcpy(char *dest, const char *src, size_t n);

/* convert a string to an u32 value */
int strtou32(const char *str, u32 *value);


#endif
