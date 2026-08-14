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

#include "UNCRUNCH.H"

void uncrunch(byte *source, byte *destination, word source_length)
{
    uword emitted = 0;
    uword row_start = 0;
    byte attribute = 0;

    while (source_length != 0) {
        byte command = *source++;
        uword repeat;
        byte character;

        --source_length;

        if (command >= 0x20) {
            destination[emitted++] = command;
            destination[emitted++] = attribute;
            continue;
        }
        if (command < 0x10) {
            attribute = (byte)((attribute & 0xf0) | command);
            continue;
        }
        if (command < 0x18) {
            attribute = (byte)((attribute & 0x8f) | ((command - 0x10) << 4));
            continue;
        }
        if (command == 0x18) {
            row_start += 160;
            emitted = row_start;
            continue;
        }
        if (command == 0x1b) {
            attribute ^= 0x80;
            continue;
        }
        if (command > 0x1b) continue;

        repeat = (uword)*source++ + 1;
        --source_length;
        character = 0x20;
        if (command != 0x19) {
            character = *source++;
            --source_length;
        }
        while (repeat-- != 0) {
            destination[emitted++] = character;
            destination[emitted++] = attribute;
        }
    }
}
