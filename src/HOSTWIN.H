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

#ifndef JILL_HOSTWIN_H
#define JILL_HOSTWIN_H

#include <stdint.h>
#include "RECOVERY.H"

int host_open(const char *title, int scale);
void host_close(void);
int host_is_open(void);
int host_pump(void);
void host_present(const byte *pixels, const byte *palette);
void host_set_title(const char *title);
int host_key_pressed(void);
int host_peek_key(void);
int host_read_key(void);
int host_key_down(int key_code);
void host_clear_keys(void);
void host_console_clear(void);
void host_sleep(unsigned milliseconds);
longword host_coreleft(void);
uint32_t host_ticks(void);
void host_start_clock(void);
void host_stop_clock(void);

#endif
