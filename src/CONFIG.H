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

#ifndef JILL_CONFIG_H
#define JILL_CONFIG_H

#include "RECOVERY.H"

typedef struct ConfigState {
    word firstthru;
    word joyflag0, joyxl0, joyxc0, joyxr0, joyyu0, joyyc0, joyyd0;
    word video_mode;
    word musicflag0;
    word vocflag0;
} ConfigState;

void cfg_getpath(int argc, char **argv);
void cfg_init(int argc, char **argv);
int doconfig(void);

extern char cfg_path[260];
extern word nosnd, cfgdemo;
extern ConfigState cf;

#endif
