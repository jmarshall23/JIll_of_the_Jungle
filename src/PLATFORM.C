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

#include "RECOVERY.H"
#include "HOSTWIN.H"

#include <stdlib.h>
#include <string.h>

static volatile uword jill_bios_clock;
volatile uword *myclock = &jill_bios_clock;
volatile longword longclock;
#include <time.h>

uword jill_read_u16_le(const byte *source)
{
    return (uword)(source[0] | ((uword)source[1] << 8));
}

ulongword jill_read_u32_le(const byte *source)
{
    return (ulongword)source[0]
        | ((ulongword)source[1] << 8)
        | ((ulongword)source[2] << 16)
        | ((ulongword)source[3] << 24);
}

void jill_write_u16_le(byte *destination, uword value)
{
    destination[0] = (byte)value;
    destination[1] = (byte)(value >> 8);
}

void jill_write_u32_le(byte *destination, ulongword value)
{
    destination[0] = (byte)value;
    destination[1] = (byte)(value >> 8);
    destination[2] = (byte)(value >> 16);
    destination[3] = (byte)(value >> 24);
}

char *jill_strdup(const char *source)
{
    size_t length;
    char *copy;
    if (source == NULL) return NULL;
    length = strlen(source) + 1;
    copy = (char *)malloc(length);
    if (copy != NULL) memcpy(copy, source, length);
    return copy;
}

int jill_random(int limit)
{
    if (limit <= 0) return 0;
    return rand() % limit;
}

void jill_delay(unsigned milliseconds)
{
    if (host_is_open()) { host_sleep(milliseconds); return; }
    const clock_t until = clock() + (clock_t)milliseconds * CLOCKS_PER_SEC / 1000;
    while (clock() < until) { }
}

uint32_t jill_ticks(void)
{
    if (host_is_open()) return host_ticks();
    return (uint32_t)((uint64_t)clock() * 1000U / CLOCKS_PER_SEC);
}
