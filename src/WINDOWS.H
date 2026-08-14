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

#ifndef JILL_WINDOWS_H
#define JILL_WINDOWS_H

#include "GR.H"

#define dialog  1
#define textbox 2

/* WIN.C accesses these members at fixed offsets.  Jill added botleft to the
   three-view Xargon-era window record, making the DOS record 0x58 bytes. */
typedef struct wintype {
    word winflags;
    word winx, winx8, winy;
    word winxl, winxl16, winyl, winyl16;
    word winh, winh16, winv, winv16;
    vptype border;
    vptype inside;
    vptype topleft;
    vptype botleft;
} wintype;

void defwin(wintype *win, int x8, int y, int xl16, int yl16,
            int h16, int v16, int flags);
void drawwin(wintype *win);
void undrawwin(wintype *win);
void wprint(vptype *vp, int x, int y, int font, const char *text);
void wprintc(vptype *vp, int y, int font, const char *text);
int wgetkey(vptype *vp, int x, int y, int font);
void winput(vptype *vp, int x, int y, int font, char *text, int maxlen);
void titlewin(wintype *win, const char *text);
void titletop(wintype *win, const char *text);
void titlebot(wintype *win, const char *text);
void initvp(vptype *vp, int bkgnd);
void clearvp(vptype *vp);
void fontcolor(vptype *vp, int hi, int back);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(vptype) == 16, "Jill viewport record must be 16 bytes");
_Static_assert(sizeof(wintype) == 88, "Jill window record must be 0x58 bytes");
#endif

#endif
