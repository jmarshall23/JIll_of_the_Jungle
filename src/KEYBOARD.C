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

#include "KEYBOARD.H"
#include "HOSTSDL.H"

#include <string.h>

sbyte k_rshift, k_lshift, k_shift, k_ctrl, k_alt, k_numlock;
volatile byte keydown[2][256];
static byte bioscall;

int k_pressed(void)
{
    if (host_is_open()) {
        (void)host_pump();
        return host_peek_key();
    }
    /* SDL window close is the host equivalent of cancelling a BIOS key wait. */
    return 1;
}

int k_read(void)
{
    if (host_is_open()) {
        while (host_is_open() && !host_key_pressed()) {
            (void)host_pump();
            host_sleep(1);
        }
        return host_read_key();
    }
    return key_escape;
}

void k_status(void)
{
    memset((void *)keydown, 0, sizeof(keydown));
    k_lshift = (sbyte)(host_is_open() && host_key_down(key_shift));
    k_rshift = 0;
    k_shift = (sbyte)(k_rshift | k_lshift);
    k_ctrl = (sbyte)(host_is_open() && host_key_down(key_ctrl));
    k_alt = (sbyte)(host_is_open() && host_key_down(key_alt));
    k_numlock = 0;

    keydown[0][scan_ctrl] = (byte)k_ctrl;
    keydown[0][scan_lshift] = (byte)k_lshift;
    keydown[0][scan_rshift] = (byte)k_rshift;
    keydown[0][scan_alt] = (byte)k_alt;
    keydown[0][scan_space] = (byte)(host_is_open() && host_key_down(key_space));
    keydown[1][scan_cursorup] = (byte)(host_is_open() && host_key_down(k_up));
    keydown[1][scan_cursorleft] = (byte)(host_is_open() && host_key_down(k_left));
    keydown[1][scan_cursorright] = (byte)(host_is_open() && host_key_down(k_right));
    keydown[1][scan_cursordown] = (byte)(host_is_open() && host_key_down(k_down));
}

void installhandler(byte status)
{
    memset((void *)keydown, 0, sizeof(keydown));
    bioscall = status;
}

void removehandler(void)
{
}

void disablebios(void) { bioscall = 0; }
void enablebios(void) { host_clear_keys(); bioscall = 1; }
int biosstatus(void) { return bioscall != 0; }
