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

#ifndef JILL_MUSIC_H
#define JILL_MUSIC_H

#include "RECOVERY.H"

extern word musicflag, vocflag;
extern word soundf;
extern word soundoff, makesound;
extern char *textmsg;
extern word textmsglen;

/* Shared with the recovered MUSINTR.ASM interrupt body. */
extern word *freq, *dur;
extern word oldfreq, clockrate, soundlen, soundptr;
extern word clockcount, soundcount;

void getvoc(int sample);
void snd_init(char *library_name);
void snd_play(int priority, int sound);
void snd_do(void);
void text_get(int index);
void snd_exit(void);
void sb_update(void);
int sb_playing(void);
void sb_shutup(void);
void sb_playtune(char *filename);
void sampadd1(int instrument, int length, int duration, int note);
void soundadd1(int priority, char *sequence);
void soundstop(void);
void timerset(int timer, int mode, unsigned divisor);
void sampadd(int instrument, int length, int duration, int note);
void soundadd2(int priority, char *sequence);
void soundadd(int priority, char *sequence);
void testintr(void);
void spkr_intr(void);
void WorxBugInt8(void);

#endif
