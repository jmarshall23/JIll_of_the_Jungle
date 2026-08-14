/*
===========================================================================

Jill of the Jungle Reconstructed
Copyright (C) 2026 Justin Marshall(IceColdDuke).

This file is part of the Jill of the Jungle Reconstructed Source Code (?Jill of the Jungle Reconstructed?).

Jill of the Jungle Reconstructed is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Jill of the Jungle Reconstructed is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Jill of the Jungle Reconstructed.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/

#ifndef JILL_RECOVERY_H
#define JILL_RECOVERY_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t  byte;
typedef int8_t   sbyte;
typedef int16_t  word;
typedef uint16_t uword;
typedef int32_t  longword;
typedef uint32_t ulongword;

#if defined(_MSC_VER)
#  define JILL_PACKED
#  define JILL_INLINE __inline
#else
#  define JILL_PACKED __attribute__((packed))
#  define JILL_INLINE inline
#endif

#define JILL_ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#define JILL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define JILL_MAX(a, b) ((a) > (b) ? (a) : (b))
#define JILL_SIGN(a) (((a) > 0) - ((a) < 0))

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(byte) == 1, "byte must be 8-bit");
_Static_assert(sizeof(word) == 2, "word must be 16-bit");
_Static_assert(sizeof(longword) == 4, "longword must be 32-bit");
_Static_assert(sizeof(void *) == 4, "the recovered program requires 32-bit pointers");
#endif

uword jill_read_u16_le(const byte *source);
ulongword jill_read_u32_le(const byte *source);
void jill_write_u16_le(byte *destination, uword value);
void jill_write_u32_le(byte *destination, ulongword value);
char *jill_strdup(const char *source);
int jill_random(int limit);
void jill_delay(unsigned milliseconds);
uint32_t jill_ticks(void);

/* Win32 emulation of the BIOS timer word at 0040:006c. */
extern volatile uword *myclock;
extern volatile longword longclock;

#endif
