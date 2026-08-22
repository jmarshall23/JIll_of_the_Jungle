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
#include "UNKNOWN.H"
#include "HOSTCOMPAT.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOC_COUNT       50
#define TEXT_COUNT      40
#define SOUNDMAC_COUNT 128
#define VOC_CACHE_COUNT 4
#define VOC_BLOCK_SIZE  0x1800
#define VOC_HEADER_SIZE 0x20
#define SOUND_CAPACITY  0x1000

word soundoff = 1;
word soundf = 1;
word makesound;
word SetDSP;
word SetWORX;
word vocuse;
word *freq;
word *dur;
word headersize = 640;
word vocflag = 1;
word musicflag = 1;
int vocfilehandle = -1;
char *song;

static const char vochdr[VOC_HEADER_SIZE] = {
    0x43, 0x72, 0x65, 0x61, 0x74, 0x69, 0x76, 0x65,
    0x20, 0x56, 0x6f, 0x69, 0x63, 0x65, 0x20, 0x46,
    0x69, 0x6c, 0x65, 0x1a, 0x1a, 0x00, 0x0a, 0x01,
    0x29, 0x11, 0x01, 0x5d, 0x2d, 0x00, 0xaa, 0x00
};

static const word notetable[144] = {
      64,    67,    71,    76,    80,    85,    90,    95,
     101,   107,   114,   121,     0,     0,     0,     0,
     128,   135,   143,   152,   161,   170,   181,   191,
     203,   215,   228,   242,     0,     0,     0,     0,
     256,   271,   287,   304,   322,   341,   362,   383,
     406,   430,   456,   483,     0,     0,     0,     0,
     512,   542,   574,   608,   645,   683,   724,   767,
     812,   861,   912,   967,     0,     0,     0,     0,
    1024,  1084,  1149,  1217,  1290,  1366,  1448,  1534,
    1625,  1722,  1825,  1933,     0,     0,     0,     0,
    2048,  2169,  2298,  2435,  2580,  2733,  2896,  3068,
    3250,  3444,  3649,  3866,     0,     0,     0,     0,
    4096,  4339,  4597,  4870,  5160,  5467,  5792,  6137,
    6501,  6888,  7298,  7732,     0,     0,     0,     0,
    8192,  8679,  9195,  9741, 10321, 10935, 11585, 12274,
   13003, 13777, 14596, 15646,     0,     0,     0,     0,
   16384, 17358, 18390, 19483, 20642, 21870, 23170, 24548,
   26007, 27554, 29192, 30928,     0,     0,     0,     0
};

/* The first 50 bytes of mirrortab are the VOC aliases used by snd_play. */
static const byte mirrortab[VOC_COUNT] = {
     0,  0,  0,  0,  0,  0,  0, 23,  0, 28,
     0,  0,  0, 24, 28,  0,  0,  0,  0,  0,
    48,  0,  0,  0,  0,  0,  5, 48,  0, 23,
    24, 18, 16,  0,  3,  0, 12,  0,  8,  0,
    41,  0, 32,  8, 24, 10,  0, 35,  0, 48
};

word *SOUNDS;
char *memvoc;
word oldpri;
word vocpri;
word vocused[VOC_COUNT];
word vocrate[VOC_COUNT];
sbyte vocnum[VOC_COUNT];
word voclen[VOC_COUNT];
char *textmsg;
char *soundmac[SOUNDMAC_COUNT];
word textlen[TEXT_COUNT];
longword vocposn[VOC_COUNT];
word oldfreq;
word clockrate;
word soundlen;
longword textposn[TEXT_COUNT];
word soundptr;
word textmsglen;
word clockcount;
word soundcount;
word notepriority;
word samppriority;

extern word nosnd;
extern void rexit(int result);

void testintr(void)
{
    spkr_intr();
    /* oldint8(): chaining the saved real-mode IRQ vector is a DOS boundary. */
}

void getvoc(int sample)
{
    word count;
    word oldest;
    word oldest_use;
    word index;
    word slot;
    char *block;

    /* nosound(): direct PC-speaker hardware call omitted at the host boundary. */
    if (voclen[sample] == 0 || vocnum[sample] != -1) return;
    count = 0;
    oldest = -1;
    oldest_use = -1;
    for (index = 0; index < VOC_COUNT; ++index) {
        if (vocnum[index] != -1) {
            ++count;
            if ((uword)vocused[index] < (uword)oldest_use) {
                oldest_use = vocused[index];
                oldest = index;
            }
        }
    }
    if (count >= VOC_CACHE_COUNT) {
        vocnum[sample] = vocnum[oldest];
        vocnum[oldest] = -1;
    } else {
        vocnum[sample] = (byte)count;
    }

    slot = vocnum[sample];
    block = memvoc + (uword)(slot * VOC_BLOCK_SIZE);
    memcpy(block, vochdr, VOC_HEADER_SIZE);
    block[0x1b] = (byte)voclen[sample];
    block[0x1c] = (byte)((uword)voclen[sample] >> 8);
    block[0x1e] = 0x60;
    (void)_lseek(vocfilehandle, vocposn[sample], SEEK_SET);
    (void)_read(vocfilehandle, block + VOC_HEADER_SIZE, voclen[sample]);
}

void snd_init(char *path)
{
    word index;

    clockrate = 0;
    clockcount = 0;
    textmsg = NULL;
    for (index = 0; index < VOC_COUNT; ++index) {
        vocposn[index] = -1;
        voclen[index] = 0;
        vocrate[index] = 0;
        vocnum[index] = -1;
        vocused[index] = 0;
    }
    for (index = 0; index < SOUNDMAC_COUNT; ++index)
        soundmac[index] = NULL;

    /*
    * Do not initialize the host audio backend when sound has
    * explicitly been disabled.
    */
    if (!nosnd && (musicflag || vocflag)) {
        StartWorx();

        /* getvect/setvect for WorxBugInt8 are real-mode interrupt boundaries. */
        if (musicflag)
            musicflag = (word)AdlibDetect();

        if (!musicflag)
            vocflag = 0;
    }
    if (*path == '\0') {
        vocflag = 0;
        return;
    }
    vocfilehandle = _open(path, _O_BINARY | _O_RDONLY);
    if (vocfilehandle == -1) {
        vocflag = 0;
        return;
    }
    (void)_read(vocfilehandle, vocposn, sizeof(vocposn));
    (void)_read(vocfilehandle, voclen, sizeof(voclen));
    (void)_read(vocfilehandle, vocrate, sizeof(vocrate));
    (void)_read(vocfilehandle, textposn, sizeof(textposn));
    (void)_read(vocfilehandle, textlen, sizeof(textlen));
}

void snd_play(int priority, int sound)
{
    char *block;

    if (vocflag && soundf) {
        if (!VOCPlaying() || priority >= oldpri) {
            if (mirrortab[sound] != 0) sound = mirrortab[sound];
            getvoc(sound);
            if (vocnum[sound] != -1) {
                block = memvoc + (uword)(vocnum[sound] * VOC_BLOCK_SIZE);
                (void)PlayVOCBlock(block, 0x7f);
                vocused[sound] = vocuse;
                ++vocuse;
            }
            oldpri = (word)priority;
        }
    } else if (sound < SOUNDMAC_COUNT && soundmac[sound] != NULL &&
               freq != NULL && dur != NULL) {
        soundadd(priority, soundmac[sound]);
    }
}

void snd_do(void)
{
    word index;
    word size;
    int handle;

    if (nosnd) {
        clockrate = 1;
        soundoff = 1;
        musicflag = 0;
        vocflag = 0;
        return;
    }

    /* nosound(): direct PC-speaker hardware call omitted at the host boundary. */
    if (musicflag || vocflag)
        clockrate = 0;
    else if (!vocflag)
        clockrate = 64;

    if (musicflag) (void)SetFMVolume(15, 15);
    if (vocflag) {
        SetDSP = (word)(DSPReset() != 0);
        vocflag = SetDSP;
        if (!vocflag) soundoff = 1;
        else (void)SetMasterVolume(15, 15);
    }

    if (vocflag) {
        memvoc = (char *)malloc(0x7800);
    } else {
        memvoc = NULL;
        freq = (word *)malloc(0x2080);
        dur = (word *)malloc(0x2080);
        (void)_lseek(vocfilehandle, headersize, SEEK_SET);
        for (index = 0; index < SOUNDMAC_COUNT; ++index) {
            (void)_read(vocfilehandle, &size, sizeof(size));
            if (size != 0) {
                soundmac[index] = (char *)malloc((uword)size);
                if (soundmac[index] == NULL) rexit(154);
                (void)_read(vocfilehandle, soundmac[index], (uword)size);
            } else {
                soundmac[index] = NULL;
            }
        }
        SOUNDS = (word *)malloc(0x28f0);
        handle = _open("AUDIO.EPC", _O_BINARY | _O_RDONLY);
        if (handle == -1) rexit(155);
        (void)_read(handle, SOUNDS, 0x28a0);
        _close(handle);
    }

    if (clockrate == 0) {
        clockrate = 1;
        soundoff = 1;
    } else if (clockrate > 1) {
        soundoff = 0;
        timerset(0, 2, (unsigned)(0x10000UL / (uword)clockrate));
    }
}

void text_get(int index)
{
    textmsg = NULL;
    if (textlen[index] != 0) {
        textmsglen = textlen[index];
        textmsg = (char *)malloc((uword)textmsglen);
        if (textmsg != NULL) {
            (void)_lseek(vocfilehandle, textposn[index], SEEK_SET);
            if (_read(vocfilehandle, textmsg, (uword)textmsglen) == -1)
                textmsg = NULL;
        }
    }
}

void snd_exit(void)
{
    word index;

    timerset(0, 2, 0);
    /* nosound() and restoring the saved IRQ vectors are DOS boundaries. */
    if (freq != NULL) free(freq);
    if (dur != NULL) free(dur);
    for (index = 0; index < SOUNDMAC_COUNT; ++index)
        if (soundmac[index] != NULL) free(soundmac[index]);
    free(memvoc);
    if (vocfilehandle >= 0) _close(vocfilehandle);
    if (SetDSP) DSPClose();
    CloseWorx();
}

void sb_update(void) { }

int sb_playing(void)
{
    return 1;
}

void sb_shutup(void)
{
    if (musicflag) {
        StopSequence();
        free(song);
        song = NULL;
    }
}

void sb_playtune(char *filename)
{
    if (musicflag) {
        sb_shutup();
        song = GetSequence(filename);
        if (song != NULL) {
            SetLoopMode(1);
            PlayCMFBlock(song);
        }
    }
}

void timerset(int timer, int mode, unsigned divisor)
{
    /* Exact PIT command: ((timer << 6) + (mode << 1) + 0x30), followed by
       the divisor low and high bytes.  Privileged port writes are omitted. */
    (void)timer;
    (void)mode;
    (void)divisor;
}

void sampadd1(int instrument, int length, int duration, int note)
{
    word sample_index;
    longword multiplier;
    longword sample;

    if (soundoff) return;
    sample_index = 0;
    multiplier = notetable[note + 16];
    makesound = 1;
    do {
        sample = SOUNDS[(uword)instrument * 128U + sample_index++];
        if (sample == -1L) freq[soundlen] = -1;
        else freq[soundlen] = (word)((sample * multiplier) >> 10);
        dur[soundlen++] = (word)duration;
    } while (sample_index < length && soundlen < SOUND_CAPACITY);
}

void sampadd(int instrument, int length, int duration, int note)
{
    word sample_index;
    longword multiplier;
    longword sample;

    if (soundoff) return;
    sample_index = 0;
    multiplier = notetable[note + 16];
    makesound = 1;
    do {
        sample = SOUNDS[(uword)instrument * 128U + sample_index++];
        if (sample == -1L) freq[soundlen] = -1;
        else freq[soundlen] = (word)((sample * multiplier) >> 10);
        dur[soundlen++] = (word)duration;
    } while (sample_index < length && soundlen < SOUND_CAPACITY);
}

static word instrument_length(word instrument)
{
    word length = SOUNDS[0x1400 + (uword)instrument];
    return length < 1 ? 1 : length;
}

void soundadd1(int priority, char *sequence)
{
    word instrument = -1;
    word cursor = 0;
    word note, duration_value, sample_count, total;
    word remaining;

    if (soundoff) return;
    if (makesound &&
        !((priority >= notepriority && notepriority != -1) || priority == -1))
        return;
    if (priority >= 0 || !makesound) {
        makesound = 0;
        soundptr = soundlen = soundcount = 0;
    }
    notepriority = (word)priority;
    do {
        if ((byte)sequence[cursor] == 0xf0) {
            ++cursor;
            instrument = (word)(sbyte)sequence[cursor++];
        }
        note = (word)(sbyte)sequence[cursor++];
        duration_value = (word)(sbyte)sequence[cursor++];
        if (instrument == -1) {
            freq[soundlen] = notetable[note];
            dur[soundlen++] = (word)((uword)duration_value * (uword)clockrate);
            makesound = 1;
        } else {
            sample_count = instrument_length(instrument);
            total = (word)((uword)duration_value * (uword)clockrate);
            remaining = (word)(total - (word)(sample_count << 7));
            if (remaining > 0) {
                sampadd(instrument, 128, sample_count, note);
                freq[soundlen] = -1;
                dur[soundlen++] = remaining;
            } else {
                sampadd(instrument, (word)((uword)total / (uword)sample_count),
                        sample_count, note);
            }
        }
    } while (sequence[cursor] != 0 && soundlen < SOUND_CAPACITY);
}

void soundadd2(int priority, char *sequence)
{
    word instrument = -1;
    word cursor = 0;
    word note, duration_value, sample_count, total;
    word remaining;

    if (soundoff) return;
    if (makesound &&
        !((priority >= notepriority && notepriority != -1) || priority == -1))
        return;
    if (priority >= 0 || !makesound) {
        makesound = 0;
        soundptr = soundlen = soundcount = 0;
    }
    notepriority = (word)priority;
    do {
        if ((byte)sequence[cursor] == 0xf0) {
            ++cursor;
            instrument = (word)(sbyte)sequence[cursor++];
        }
        note = (word)(sbyte)sequence[cursor++];
        duration_value = (word)(sbyte)sequence[cursor++];
        if (instrument == -1) {
            freq[soundlen] = notetable[note];
            dur[soundlen++] = (word)((uword)duration_value * (uword)clockrate);
            makesound = 1;
        } else {
            sample_count = instrument_length(instrument);
            total = (word)((uword)duration_value * (uword)clockrate);
            remaining = (word)(total - (word)(sample_count << 7));
            if (remaining > 0) {
                sampadd(instrument, 128, sample_count, note);
                freq[soundlen] = -1;
                dur[soundlen++] = remaining;
            } else {
                sampadd(instrument, (word)((uword)total / (uword)sample_count),
                        sample_count, note);
            }
        }
    } while (sequence[cursor] != 0 && soundlen < SOUND_CAPACITY);
}

void soundadd(int priority, char *sequence)
{
    word instrument = -1;
    word cursor = 0;
    word note, duration_value, sample_count, total;
    word remaining;

    if (soundoff) return;
    if (makesound &&
        !((priority >= notepriority && notepriority != -1) || priority == -1))
        return;
    if (priority >= 0 || !makesound) {
        makesound = 0;
        soundptr = soundlen = soundcount = 0;
    }
    notepriority = (word)priority;
    do {
        if ((byte)sequence[cursor] == 0xf0) {
            ++cursor;
            instrument = (word)(sbyte)sequence[cursor++];
        }
        note = (word)(sbyte)sequence[cursor++];
        duration_value = (word)(sbyte)sequence[cursor++];
        if (instrument == -1) {
            freq[soundlen] = notetable[note];
            dur[soundlen++] = (word)((uword)duration_value * (uword)clockrate);
            makesound = 1;
        } else {
            sample_count = instrument_length(instrument);
            total = (word)((uword)duration_value * (uword)clockrate);
            remaining = (word)(total - (word)(sample_count << 7));
            if (remaining > 0) {
                sampadd(instrument, 128, sample_count, note);
                freq[soundlen] = -1;
                dur[soundlen++] = remaining;
            } else {
                /* Jill's original signed-remainder division bug, at 160F1. */
                sampadd(instrument, (word)(remaining / (word)sample_count),
                        sample_count, note);
            }
        }
    } while (sequence[cursor] != 0 && soundlen < SOUND_CAPACITY);
}

void soundstop(void)
{
    makesound = 0;
    /* nosound(): direct PC-speaker hardware call omitted at the host boundary. */
}
