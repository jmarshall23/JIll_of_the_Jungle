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

#ifndef JILL_SHM_H
#define JILL_SHM_H

#include "RECOVERY.H"

#ifndef SHM_MAX_TABLES
#define SHM_MAX_TABLES 64
#endif

#define shm_fontf  1
#define shm_xxxxx  2
#define shm_blflag 4
#define st_byte    0
#define st_plain   1
#define st_rle     2

extern word shm_want[SHM_MAX_TABLES];
extern byte *shm_tbladdr[SHM_MAX_TABLES];
extern word shm_tbllen[SHM_MAX_TABLES];
extern word shm_flags[SHM_MAX_TABLES];

void shm_init(char *filename);
void init8bit(void);
void xlate_table(int table_number, byte *source, byte *work_buffer);
void shm_do(void);
void shm_exit(void);

#endif
