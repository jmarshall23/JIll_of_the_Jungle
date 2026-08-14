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
#include "MUSIC.H"

void spkr_intr(void)
{
    word frequency;

    if (makesound && soundf) {
        --soundcount;
        if (soundcount <= 0) {
            if (soundptr >= soundlen) {
                makesound = 0;
                /* Original: nosound(). */
            } else {
                frequency = freq[soundptr];
                if (frequency == -1) {
                    /* Original: nosound(). */
                } else if (frequency != oldfreq) {
                    /* Original: sound(frequency). */
                }
                soundcount = dur[soundptr];
                ++soundptr;
                oldfreq = frequency;
            }
        }
    } else {
        /* Original: nosound(). */
    }

    if (clockcount++ > 2) {
        clockcount = 0;
        /* Original: chain to oldint8. */
    } else {
        /* Original: acknowledge IRQ 0 at port 20h. */
    }
}

void WorxBugInt8(void)
{
    /* Original: preserve registers, clear direction, and chain worxint8. */
}
