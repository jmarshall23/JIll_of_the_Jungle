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

#include "UNKNOWN.H"
#include "JVOL.H"
#include "HOSTAUDIO.H"

#include <stdio.h>
#include <stdlib.h>

word WORX_AX, WORX_ES, WORX_BX, WORX_DX, WORX_DI;

static FILE *element_file;
static int voc_playing;
static int sequence_playing;
static int loop_mode;
static byte master_volume;
static byte fm_volume;
static size_t sequence_length;

int WORX_CALL(byte ah, byte al, uword bx, void *memory, uword di)
{
    long length;
    size_t count;

    WORX_AX = (word)(((uword)ah << 8) | al);
    WORX_ES = 0;
    WORX_BX = (word)bx;
    WORX_DX = 0;
    WORX_DI = (word)di;

    /* INT 63h is a real-mode-only transport.  This switch is the sole host
       implementation boundary; the wrappers below retain their exact calls. */
    switch (ah) {
    case 0x01:                         /* DSPReset */
        WORX_AX = (word)host_audio_digital_available();
        break;
    case 0x04:                         /* DSPClose */
        host_audio_stop_voc();
        voc_playing = 0;
        WORX_AX = 1;
        break;
    case 0x06:                         /* PlayVOCBlock */
        voc_playing = host_audio_play_voc((const byte *)memory, bx);
        WORX_AX = voc_playing ? 0 : -1;
        break;
    case 0x08:                         /* VOCPlaying */
        voc_playing = host_audio_voc_playing();
        WORX_AX = (word)voc_playing;
        break;
    case 0x0d:                         /* StopSequence */
        host_audio_stop_music();
        sequence_playing = 0;
        WORX_AX = 1;
        break;
    case 0x17:                         /* OpenElement */
        if (element_file != NULL) fclose(element_file);
        element_file = memory != NULL ? fopen((const char *)memory, "rb") : NULL;
        if (element_file == NULL || fseek(element_file, 0, SEEK_END) != 0) {
            WORX_AX = -1;
            WORX_DX = -1;
        } else {
            length = ftell(element_file);
            rewind(element_file);
            WORX_AX = (word)(uword)length;
            WORX_DX = (word)((ulongword)length >> 16);
        }
        break;
    case 0x18:                         /* ElementRead */
        if (element_file == NULL || memory == NULL) {
            WORX_AX = -1;
        } else {
            count = fread(memory, 1, bx, element_file);
            WORX_AX = (word)count;
        }
        break;
    case 0x1d:                         /* SetLoopMode */
        loop_mode = bx != 0;
        WORX_AX = (word)loop_mode;
        break;
    case 0x1e:                         /* PlayCMFBlock */
        sequence_playing = host_audio_play_cmf((const byte *)memory,
                                               sequence_length, loop_mode);
        WORX_AX = (word)sequence_playing;
        break;
    case 0x20:                         /* SetMasterVolume */
        master_volume = (byte)bx;
        host_audio_set_master_volume((byte)(bx >> 4), (byte)(bx & 15));
        WORX_AX = master_volume;
        break;
    case 0x22:                         /* SetFMVolume */
        fm_volume = (byte)bx;
        host_audio_set_music_volume((byte)(bx >> 4), (byte)(bx & 15));
        WORX_AX = fm_volume;
        break;
    case 0x23:                         /* AdlibDetect */
        WORX_AX = (word)host_audio_music_available();
        break;
    }
    return WORX_AX;
}

void CloseWorx(void)
{
    host_audio_stop();
    CORE_CLOSEWORX();
    if (element_file != NULL) fclose(element_file);
    element_file = NULL;
}

int SetMasterVolume(byte left, byte right)
{
    return WORX_CALL(0x20, 0, (uword)(((uword)left << 4) | right), NULL, 0);
}

int SetFMVolume(byte left, byte right)
{
    return WORX_CALL(0x22, 0, (uword)(((uword)left << 4) | right), NULL, 0);
}

int AdlibDetect(void)
{
    return WORX_CALL(0x23, 0, 0, NULL, 0);
}

void DSPClose(void)
{
    (void)WORX_CALL(0x04, 0, 0, NULL, 0);
}

int DSPReset(void)
{
    return WORX_CALL(0x01, 0, 0, NULL, 0);
}

uword ElementRead(void *buffer, uword length)
{
    int result = WORX_CALL(0x18, 0, length, buffer, 0);
    return result == -1 ? 0 : (uword)WORX_AX;
}

long OpenElement(char *filename)
{
    (void)WORX_CALL(0x17, 0, 0, filename, 0);
    return (longword)(((ulongword)(uword)WORX_DX << 16) |
                      (uword)WORX_AX);
}

char *GetSequence(char *filename)
{
    long nbytes = OpenElement(filename);
    char *block = NULL;
    if (nbytes > 0) {
        block = (char *)malloc((uword)nbytes);
        if (block == NULL) return NULL;
        nbytes = (long)ElementRead(block, (uword)nbytes);
        sequence_length = (size_t)nbytes;
    } else {
        sequence_length = 0;
    }
    return block;
}

char *LoadOneShot(char *filename)
{
    long nbytes = OpenElement(filename);
    char *block = NULL;
    if (nbytes > 0) {
        block = (char *)malloc((uword)nbytes);
        if (block == NULL) return NULL;
        (void)ElementRead(block, (uword)nbytes);
    }
    return block;
}

int PlayVOCBlock(char *voc, int volume)
{
    if (voc == NULL) return -1;
    return WORX_CALL(0x06, 0, (uword)volume, voc, 0);
}

void PlayCMFBlock(char *sequence)
{
    if (sequence != NULL)
        (void)WORX_CALL(0x1e, 0, 0, sequence, 0);
}

void SetLoopMode(int mode)
{
    (void)WORX_CALL(0x1d, 0, (uword)mode, NULL, 0);
}

void StartWorx(void)
{
    CORE_STARTWORX();
    (void)host_audio_start();
}

void StopSequence(void)
{
    (void)WORX_CALL(0x0d, 0, 0, NULL, 0);
}

int VOCPlaying(void)
{
    return WORX_CALL(0x08, 0, 0, NULL, 0);
}
