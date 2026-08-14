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

#include "JILL.H"

#include <fcntl.h>
#include <io.h>
#include <stdlib.h>

infotype info[numinfotypes];
word stateinfo[6];

void initinfo(void)
{
    static char empty_name[] = "";
    int handle;
    word number;
    word flags;
    sbyte length;
    int index;

    flags = 0x4006;
    for (index = 0; index < numinfotypes; ++index) {
        info[index].sh = 0x4700;
        info[index].name = empty_name;
        info[index].flags = flags;
    }

    handle = _open("jill.dma", _O_BINARY);
    while (_read(handle, &number, 2) > 0) {
        (void)_read(handle, &info[number].sh, 2);
        (void)_read(handle, &flags, 2);
        info[number].flags ^= flags;
        (void)_read(handle, &length, 1);
        info[number].name = (char *)malloc((size_t)length + 1);
        (void)_read(handle, info[number].name, (uword)length);
        info[number].name[length] = '\0';
    }

    for (index = 0; index < 6; ++index) stateinfo[index] = 0;
    stateinfo[st_begin] |= sti_invincible;
    stateinfo[st_stand] |= sti_canfire;
    stateinfo[st_jumping] |= sti_canfire;
    stateinfo[st_climbing] |= 0;
    stateinfo[st_die] |= sti_invincible;
}
