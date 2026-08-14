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

#ifndef JILL_KEYBOARD_H
#define JILL_KEYBOARD_H

#include "RECOVERY.H"

/* Values returned by the original BIOS-backed k_read(). */
enum {
    enter = 13,
    esc = 27,
    escape = 27,
    k_tab = 9,
    k_bs = 8,
    k_up = 200,
    k_down = 208,
    k_left = 203,
    k_right = 205,
    k_pgup = 201,
    k_pgdown = 209,
    k_f1 = 187,
    k_f2 = 188,
    k_f3 = 189,
    k_f4 = 190,
    k_f5 = 191,
    k_f6 = 192,
    k_f7 = 193,
    k_f8 = 194,
    k_f9 = 195,
    k_f10 = 196
};

/* Existing recovered modules use these descriptive aliases. */
#define key_escape escape
#define key_enter enter
#define key_space 32
#define key_up k_up
#define key_down k_down
#define key_left k_left
#define key_right k_right
#define key_pgup k_pgup
#define key_pgdown k_pgdown
#define key_ctrl 0x11
#define key_alt 0x12
#define key_shift 0x10
#define key_f1 k_f1
#define key_f2 k_f2
#define key_f3 k_f3
#define key_f4 k_f4
#define key_f5 k_f5
#define key_f6 k_f6
#define key_f7 k_f7
#define key_f8 k_f8
#define key_f9 k_f9
#define key_f10 k_f10

enum {
    scan_esc = 0x01,
    scan_ctrl = 0x1d,
    scan_lshift = 0x2a,
    scan_rshift = 0x36,
    scan_alt = 0x38,
    scan_space = 0x39,
    scan_f1 = 0x3b,
    scan_cursorup = 0x48,
    scan_cursorleft = 0x4b,
    scan_cursorright = 0x4d,
    scan_cursordown = 0x50
};

extern sbyte k_rshift, k_lshift, k_shift, k_ctrl, k_alt, k_numlock;
extern volatile byte keydown[2][256];

void k_status(void);
int k_pressed(void);
int k_read(void);
void handler(void);
void installhandler(byte status);
void removehandler(void);
void disablebios(void);
void enablebios(void);
int biosstatus(void);

#endif
