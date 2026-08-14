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

#ifndef JILL_CMFOPL_H
#define JILL_CMFOPL_H

#include <stddef.h>
#include <stdint.h>

typedef struct cmf_opl_player cmf_opl_player;

cmf_opl_player *cmf_opl_create(const uint8_t *cmf, size_t length,
                               uint32_t sample_rate, int loop);
void cmf_opl_destroy(cmf_opl_player *player);
size_t cmf_opl_read(cmf_opl_player *player, int16_t *pcm, size_t frames);

#endif
