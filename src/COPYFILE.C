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

#include "COPYFILE.H"
#include "RECOVERY.H"

#include <fcntl.h>
#include "DOSIO.H"
#include <stdlib.h>
#include <sys/stat.h>

void copyfile(char *source, char *destination)
{
    byte *buffer;
    int input;
    int output;
    int count;

    buffer = (byte *)malloc(4096);
    if (buffer == NULL) return;
    input = _open(source, _O_BINARY | _O_RDONLY);
    if (input >= 0) {
        output = _creat(destination, 0);
        if (output >= 0) {
            do {
                count = _read(input, buffer, 4096);
                if (count > 0) (void)_write(output, buffer, count);
            } while (count > 0);
            _close(output);
        }
        _close(input);
    }
    free(buffer);
}
