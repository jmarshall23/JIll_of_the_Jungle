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
#include "PIXWRITE.H"
#include "GR.H"

#include <fcntl.h>
#include "DOSIO.H"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

word swrite;

void pixwrite(int value)
{
    char number[16];
    char name[80];
    char red[16], green[16], blue[16];
    char line[64];
    int handle;
    int x, y, color;
    word oldpage;

    _itoa(value, number, 10);
    strcpy(name, "\\screen");
    strcat(name, number);
    strcat(name, ".RAW");
    handle = _creat(name, 0);
    if (handle != -1) {
        oldpage = pagedraw;
        pagedraw = pageshow;
        for (y = 0; y < 200; ++y) {
            for (x = 0; x < 320; ++x) {
                readpix_vga(x, y);
                (void)_write(handle, &pixvalue, 1);
            }
        }
        pagedraw = oldpage;
        _close(handle);
    }

    strcpy(name, "\\screen");
    strcat(name, number);
    strcat(name, ".MAP");
    handle = _creat(name, 0);
    if (handle != -1) {
        for (color = 0; color < 256; ++color) {
            _itoa((int)vgapal[color * 3] * 4, red, 10);
            _itoa((int)vgapal[color * 3 + 1] * 4, green, 10);
            _itoa((int)vgapal[color * 3 + 2] * 4, blue, 10);
            strcpy(line, red);
            strcat(line, " ");
            strcat(line, green);
            strcat(line, " ");
            strcat(line, blue);
            strcat(line, "\r\n");
            (void)_write(handle, line, (unsigned)strlen(line));
        }
        _close(handle);
    }
}
