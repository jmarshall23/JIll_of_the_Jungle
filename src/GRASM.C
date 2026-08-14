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

#include "GR.H"

#include <stdlib.h>

static void put_pixel(int x, int y, int color)
{
    if (x < 0 || y < 0 || x >= JILL_SCREEN_WIDTH || y >= JILL_SCREEN_HEIGHT) return;
    jill_video[pagedraw != 0][y][x] = (byte)color;
}

void plot_cga(int x, int y, byte color) { put_pixel(x, y, color & 3); }
void plot_ega(int x, int y, byte color) { put_pixel(x, y, color & 15); }
void plot_vga(int x, int y, byte color) { put_pixel(x, y, color); }

void readpix_vga(int x, int y)
{
    if (x < 0 || y < 0 || x >= JILL_SCREEN_WIDTH || y >= JILL_SCREEN_HEIGHT) {
        pixvalue = 0;
        return;
    }
    pixvalue = jill_video[pagedraw != 0][y][x];
}

void line_cga(int x0, int y0, int x1, int y1, byte color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        if (2 * error >= dy) { error += dy; x0 += sx; }
        if (2 * error <= dx) { error += dx; y0 += sy; }
    }
}
