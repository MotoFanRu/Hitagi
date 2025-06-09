/*
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "util.h"

int strlen(const char *s)
{
	int i = 0;

	for(;*s != '\0'; s++)
		i++;

	return i;

}

int strcpy(char *d, char *s)
{
	int i=0;

	for(;;) {
		d[i] = s[i];
		if(!s[i]) break;
		i++;
	}

	return i;
}

/**
 * strcmp - Compare two strings
 * @cs: One string
 * @ct: Another string
 */
int strcmp(const char * cs,const char * ct)
{
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int strncmp(const char *s1, const char *s2, size_t maxlen) 
{
	int i;

	for(i = 0; i < maxlen; i++) {
		if(s1[i] != s2[i])
			return ((int) s1[i]) - ((int) s2[i]);
		if(s1[i] == 0)
			return 0;
	}

	return 0;
}


char *strncpy(char *dest, const char *src, size_t n)
{
	while(n > 0) {
		n--;

		if((*dest++ = *src++) == '\0')
			break;
	}

	return dest;
}




/* small variation on strncpy(): null-terminate the destination
 * string
 */
char *strlcpy(char *dest, const char *src, size_t n)
{
	strncpy(dest, src, n);

	if(n > 0)
		dest[n-1] = '\0';

	return dest;
}

/* test for a digit. return value if digit or -1 otherwise */
static int digitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	else
		return -1;
}




/* test for a hexidecimal digit. return value if digit or -1 otherwise */
static int xdigitvalue(char isdigit)
{
	if (isdigit >= '0' && isdigit <= '9' )
		return isdigit - '0';
	if (isdigit >= 'a' && isdigit <= 'f')
		return 10 + isdigit - 'a';
	return -1;
}




/* convert a string to an u32 value. if the string starts with 0x, it
 * is a hexidecimal string, otherwise we treat is as decimal. returns
 * the converted value on success, or -1 on failure. no, we don't care
 * about overflows if the string is too long.
 */
int strtou32(const char *str, u32 *value)
{
	int i;

	*value = 0;

	if(strncmp(str, "0x", 2) == 0) {
		/* hexadecimal mode */
		str += 2;
		
		while(*str != '\0') {
			if((i = xdigitvalue(*str)) < 0)
				return -1;
			
			*value = (*value << 4) | (u32)i;

			str++;
		}
	} else {
		/* decimal mode */
		while(*str != '\0') {
			if((i = digitvalue(*str)) < 0)
				return -1;
			
			*value = (*value * 10) + (u32)i;

			str++;
		}
	}

	return 0;
}
