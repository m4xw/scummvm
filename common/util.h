/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "scummsys.h"

#ifndef ABS
#define ABS(x) ((x)>=0?(x):-(x))
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define SWAP(a,b) do{int tmp=a; a=b; b=tmp; } while(0)
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


int RGBMatch(byte *palette, int r, int g, int b);
int Blend(int src, int dst, byte *palette);
void ClearBlendCache(byte *palette, int weight);

/*
 * Print hexdump of the data passed in, 8 bytes a row
 */
void hexdump(const byte * data, int len);

#endif
