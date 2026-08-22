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
#include "SHM.H"
#include "GR.H"

#include "HOSTCOMPAT.H"
#include <stdlib.h>
#include <string.h>

word shm_want[SHM_MAX_TABLES];
byte *shm_tbladdr[SHM_MAX_TABLES];
word shm_tbllen[SHM_MAX_TABLES];
word shm_flags[SHM_MAX_TABLES];

static int shafile = -1;
static char shm_filename[80];
static ulongword shm_offset[128];
static uword shm_length[128];
static byte color_table[256];
static const byte ega_color_table[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x00, 0x08, 0x08, 0x07, 0x07, 0x07, 0x0F, 0x0F, 0x00, 0x04, 0x0C, 0x0C, 0x08, 0x08, 0x02, 0x06,
    0x06, 0x0C, 0x02, 0x02, 0x02, 0x06, 0x06, 0x0E, 0x02, 0x02, 0x02, 0x02, 0x06, 0x0E, 0x0A, 0x0A,
    0x0A, 0x0E, 0x0E, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0x04, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x04, 0x0C, 0x0C, 0x08, 0x08, 0x07, 0x06, 0x04, 0x0C, 0x02, 0x02, 0x02, 0x06, 0x06, 0x06,
    0x02, 0x02, 0x02, 0x02, 0x06, 0x06, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E,
    0x00, 0x00, 0x04, 0x04, 0x0C, 0x0C, 0x00, 0x00, 0x04, 0x04, 0x0C, 0x0C, 0x08, 0x08, 0x08, 0x06,
    0x06, 0x0C, 0x02, 0x02, 0x08, 0x08, 0x06, 0x06, 0x02, 0x02, 0x02, 0x02, 0x06, 0x06, 0x0A, 0x0A,
    0x0A, 0x0E, 0x0E, 0x09, 0x09, 0x09, 0x0E, 0x0E, 0x01, 0x01, 0x05, 0x05, 0x04, 0x02, 0x01, 0x01,
    0x05, 0x05, 0x0D, 0x0D, 0x01, 0x01, 0x08, 0x05, 0x05, 0x0C, 0x03, 0x03, 0x07, 0x07, 0x07, 0x0C,
    0x03, 0x03, 0x03, 0x07, 0x07, 0x0E, 0x02, 0x02, 0x03, 0x07, 0x07, 0x0A, 0x0A, 0x0A, 0x0E, 0x0E,
    0x01, 0x01, 0x01, 0x05, 0x0D, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x0D, 0x0D, 0x01, 0x01, 0x05, 0x05,
    0x0D, 0x0D, 0x09, 0x09, 0x09, 0x05, 0x05, 0x0E, 0x03, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x04, 0x04,
    0x0B, 0x07, 0x07, 0x0B, 0x0B, 0x0B, 0x0F, 0x0F, 0x09, 0x09, 0x09, 0x05, 0x0D, 0x0D, 0x09, 0x09,
    0x09, 0x05, 0x0D, 0x0D, 0x09, 0x09, 0x09, 0x05, 0x05, 0x0D, 0x09, 0x09, 0x09, 0x09, 0x05, 0x0D,
    0x0B, 0x0B, 0x03, 0x03, 0x0D, 0x0D, 0x0B, 0x0B, 0x0D, 0x0F, 0x0D, 0x0D, 0x0D, 0x0F, 0x0F, 0x00,
};

extern void rexit(int result);

void shm_init(char *filename)
{
    int table;
    strcpy(shm_filename, filename);
    for (table = 0; table < SHM_MAX_TABLES; ++table) {
        shm_want[table] = 0;
        shm_tbladdr[table] = NULL;
    }
    shafile = _open(shm_filename, _O_BINARY | _O_RDONLY);
    if (shafile < 0) rexit(115);
    if (_read(shafile, shm_offset, sizeof(shm_offset)) == 0) rexit(102);
    if (_read(shafile, shm_length, sizeof(shm_length)) == 0) rexit(102);
}

void init8bit(void)
{
    int color;

    switch (x_ourmode & 0xfe) {
    case x_cga:
        for (color = 0; color < 256; ++color)
            color_table[color] = (byte)(color & 3);
        break;
    case x_ega:
        memcpy(color_table, ega_color_table, sizeof(color_table));
        break;
    case x_vga:
        for (color = 0; color < 256; ++color)
            color_table[color] = (byte)color;
        break;
    }
    for (color = 0; color < 256; ++color) color_table[color] = (byte)color;
}

void xlate_table(int table_number, byte *source, byte *work_buffer)
{
    byte number_of_shapes = 0;
    byte color_bits = 1;
    uword rotations, cga_length, ega_length, vga_length, flags;
    uword output_length;
    uword color_mask, color_shift;
    uword table_offset, data_offset;
    byte *output;
    byte shape;

    memcpy(&number_of_shapes, source, 1); source += 1;
    rotations = jill_read_u16_le(source); source += 2;
    cga_length = jill_read_u16_le(source); source += 2;
    ega_length = jill_read_u16_le(source); source += 2;
    vga_length = jill_read_u16_le(source); source += 2;
    memcpy(&color_bits, source, 1); source += 1;
    flags = jill_read_u16_le(source); source += 2;
    (void)rotations;

    if (x_ourmode == x_cga) {
        output_length = (uword)(cga_length + 16);
        color_mask = 3;
        color_shift = 0;
    } else if (x_ourmode == x_ega) {
        output_length = (uword)(ega_length + 16);
        color_mask = 15;
        color_shift = 8;
    } else {
        output_length = (uword)(vga_length + 16);
        color_mask = 255;
        color_shift = 16;
    }

    if ((flags & shm_fontf) != 0) {
        unsigned color;
        unsigned count = 1U << color_bits;
        for (color = 0; color < count; ++color) color_table[color] = (byte)color;
    } else if (color_bits == 8) {
        init8bit();
    } else {
        unsigned color;
        unsigned count = 1U << color_bits;
        for (color = 0; color < count; ++color) {
            ulongword packed = jill_read_u32_le(source);
            source += 4;
            color_table[color] = (byte)((packed >> color_shift) & color_mask);
        }
    }

    output = (byte *)malloc(output_length);
    if (output == NULL) rexit(100);
    shm_tbllen[table_number] = (word)output_length;
    shm_tbladdr[table_number] = output;
    shm_flags[table_number] = (word)flags;
    table_offset = 0;
    data_offset = (uword)((uword)number_of_shapes * 4U);

    for (shape = 0; shape < number_of_shapes; ++shape) {
        byte width, width_bytes, height, storage;
        int x, y, plane;
        byte packed_byte, shape_byte;

        memcpy(&width, source, 1); source += 1;
        memcpy(&height, source, 1); source += 1;
        memcpy(&storage, source, 1); source += 1;
        (void)storage;
        memcpy(work_buffer, source, (uword)width * height);
        source += (uword)width * height;

        if (color_bits == 8 && width == 64 && height == 12 &&
            x_ourmode == x_vga) {
            memmove(vgapal, work_buffer, sizeof(vgapal));
            vga_setpal();
        }

        if (x_ourmode == x_cga) width_bytes = (byte)((width + 3) / 4);
        else width_bytes = width;
        jill_write_u16_le(output + table_offset, (uword)data_offset);
        output[table_offset + 2] = width_bytes;
        output[table_offset + 3] = height;
        table_offset += 4;

        if (x_ourmode == x_cga) {
            for (y = 0; y < height; ++y) {
                packed_byte = 0;
                for (x = 0; x < width; ++x) {
                    shape_byte = color_table[work_buffer[x + y * width]];
                    packed_byte |= (byte)(shape_byte << (6 - ((x & 3) << 1)));
                    if ((x & 3) == 3 || x == width - 1) {
                        output[data_offset++] = packed_byte;
                        packed_byte = 0;
                    }
                }
            }
        } else if (x_ourmode == x_ega || x_ourmode == x_egagrey) {
            if ((flags & shm_blflag) == 0) {
                for (y = 0; y < height; ++y) {
                    for (x = 0; x < width; x += 2) {
                        packed_byte = color_table[work_buffer[x + y * width]];
                        packed_byte |= (byte)(color_table[work_buffer[x + 1 + y * width]] << 4);
                        output[data_offset++] = packed_byte;
                    }
                }
            } else {
                for (plane = 8; plane > 0; plane >>= 1) {
                    for (y = 0; y < height; ++y) {
                        packed_byte = 0;
                        for (x = 0; x < width; ++x) {
                            shape_byte = color_table[work_buffer[x + y * width]];
                            packed_byte |= (byte)(((shape_byte & plane) != 0) <<
                                                  (7 - (x & 7)));
                            if ((x & 7) == 7 || x == width - 1) {
                                output[data_offset++] = packed_byte;
                                packed_byte = 0;
                            }
                        }
                    }
                }
            }
        } else if ((flags & shm_blflag) == 0) {
            for (y = 0; y < height; ++y)
                for (x = 0; x < width; ++x)
                    output[data_offset++] = color_table[work_buffer[x + y * width]];
        } else {
            for (plane = 3; plane >= 0; --plane)
                for (y = 0; y < height; ++y)
                    for (x = 0; x < width; x += 4)
                        output[data_offset++] =
                            color_table[work_buffer[x + plane + y * width]];
        }

        if (data_offset >= output_length) exit(199);
    }
}

void shm_do(void)
{
    byte *work_buffer;
    int table;

    work_buffer = (byte *)malloc(0x1000);
    if (work_buffer == NULL) rexit(101);

    for (table = 0; table < SHM_MAX_TABLES; ++table) {
        if (!shm_want[table] && shm_tbladdr[table] != NULL) {
            free(shm_tbladdr[table]);
            shm_tbladdr[table] = NULL;
        }
    }

    for (table = 0; table < SHM_MAX_TABLES; ++table) {
        if (shm_want[table] && shm_tbladdr[table] == NULL &&
            shm_length[table] != 0) {
            byte *encoded;
            (void)_lseek(shafile, (long)shm_offset[table], 0);
            encoded = (byte *)malloc(shm_length[table]);
            if (encoded == NULL) rexit(103);
            (void)_read(shafile, encoded, shm_length[table]);
            xlate_table(table, encoded, work_buffer);
            free(encoded);
        }
    }
    free(work_buffer);
}

void shm_exit(void)
{
    int table;
    _close(shafile);
    for (table = 0; table < SHM_MAX_TABLES; ++table) {
        free(shm_tbladdr[table]);
        shm_tbladdr[table] = NULL;
    }
}
