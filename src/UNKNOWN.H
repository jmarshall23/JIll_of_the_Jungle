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

#ifndef JILL_UNKNOWN_H
#define JILL_UNKNOWN_H

#include "RECOVERY.H"

extern word WORX_AX, WORX_ES, WORX_BX, WORX_DX, WORX_DI;

int WORX_CALL(byte ah, byte al, uword bx, void *memory, uword di);
void CloseWorx(void);
void StartWorx(void);
int SetMasterVolume(byte left, byte right);
int SetFMVolume(byte left, byte right);
int AdlibDetect(void);
void DSPClose(void);
int DSPReset(void);
uword ElementRead(void *buffer, uword length);
char *GetSequence(char *filename);
char *LoadOneShot(char *filename);
long OpenElement(char *filename);
int PlayVOCBlock(char *voc, int volume);
void PlayCMFBlock(char *sequence);
void SetLoopMode(int mode);
void StopSequence(void);
int VOCPlaying(void);

#endif
