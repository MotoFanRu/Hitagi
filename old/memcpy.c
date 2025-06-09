/*
 * memcpy.c: copy memory regions
 *
 * Copyright (C) 2002  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * Implementation taken from the GNU textutils 2.0, which are
 * Copyright (C) 1995, 1997 Free Software Foundation, Inc.
 *
 * $Id: memcpy.c,v 1.1 2002/01/06 17:04:36 erikm Exp $
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

#ident "$Id: memcpy.c,v 1.1 2002/01/06 17:04:36 erikm Exp $"
#include "util.h"

void *memcpy(void *dest, const void *src, size_t n)
{
	const u8 *s = (const u8*)src;
	u8 *d = (u8 *)dest;

	while(n-- > 0)
		*d++ = *s++;

	return dest;
}

int memcmp(void *a, void *b, int size)
{
	unsigned char *aa=a, *bb=b;
	int i;

	for(i=0; i < size; i++) 
		if(aa[i] != bb[i]) return 1;

	return 0;
}


void memset(void *start, unsigned char val, unsigned long size) 
{
	int i;
	for(i=0; i < size; i++ ) {
		((unsigned char*)start)[i] = val;
	}
}
