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

#include "WINDOWS.H"
#include "KEYBOARD.H"

#include <stdlib.h>
#include <string.h>

byte cursorchar;
word curhi, curlo, curback;

void initvp(vptype *vp, int bkgnd)
{
    vp->vpox = 0;
    vp->vpoy = 0;
    vp->vphi = 1;
    vp->vpback = (word)bkgnd;
}

void defwin(wintype *win, int x8, int y, int xl16, int yl16,
            int h16, int v16, int flags)
{
    win->winflags = (word)flags;

    initvp(&win->border, 0);
    win->border.vpx = (word)(x8 * 8);
    win->border.vpy = (word)y;
    win->border.vpxl = (word)(xl16 * 16 + 16);
    win->border.vpyl = (word)(yl16 * 16 + ((flags & textbox) ? 16 : 28));

    initvp(&win->inside, 0);
    win->inside.vpx = (word)(x8 * 8 + 8);
    win->inside.vpy = (word)(y + ((flags & textbox) ? 8 : 16));
    win->inside.vpxl = (word)(xl16 * 16);
    win->inside.vpyl = (word)(yl16 * 16);
    if (h16 != 0) {
        win->inside.vpxl = (word)(win->inside.vpxl - (h16 * 16 + 8));
        win->inside.vpx = (word)(win->inside.vpx + h16 * 16 + 8);
    }

    initvp(&win->topleft, 8);
    win->topleft.vpx = (word)(x8 * 8 + 8);
    win->topleft.vpy = (word)(y + 16);
    win->topleft.vpxl = (word)(h16 * 16);
    win->topleft.vpyl = (word)(v16 * 16 + 5);

    initvp(&win->botleft, 8);
    win->botleft.vpx = (word)(x8 * 8 + 8);
    win->botleft.vpy = (word)(y + v16 * 16 + 27);
    win->botleft.vpxl = (word)(h16 * 16);
    win->botleft.vpyl = (word)(yl16 * 16 - v16 * 16 - 11);

    win->winx8 = (word)x8;
    win->winx = (word)(x8 * 8);
    win->winy = (word)y;
    win->winxl16 = (word)xl16;
    win->winxl = (word)(xl16 * 16);
    win->winyl16 = (word)yl16;
    win->winyl = (word)(yl16 * 16);
    win->winh16 = (word)h16;
    win->winh = (word)(h16 * 16);
    win->winv16 = (word)v16;
    win->winv = (word)(v16 * 16);
}

void drawwin(wintype *win)
{
    int c, d;

    clearvp(&win->border);
    if (win->winflags & textbox) {
        drawshape(&win->border, 0x4701, 0, 0);
        drawshape(&win->border, 0x4703, win->winxl + 8, 0);
        drawshape(&win->border, 0x4706, 0, win->winyl + 8);
        drawshape(&win->border, 0x4708, win->winxl + 8, win->winyl + 8);
        for (c = 1; c < win->winxl16 * 2 + 1; ++c) {
            drawshape(&win->border, 0x4702, c * 8, 0);
            drawshape(&win->border, 0x4707, c * 8, win->winyl + 8);
        }
        for (c = 1; c < win->winyl16 * 2 + 1; ++c) {
            drawshape(&win->border, 0x4704, 0, c * 8);
            drawshape(&win->border, 0x4705, win->winxl + 8, c * 8);
        }
        for (c = 0; c < win->winxl16 * 2; ++c)
            for (d = 0; d < win->winyl16 * 2; ++d)
                drawshape(&win->inside, 0x4709, c * 8, d * 8);
        return;
    }

    drawshape(&win->border, 0x4302, 0, 0);
    drawshape(&win->border, 0x4307, win->winxl + 8, win->winyl + 16);
    drawshape(&win->border, 0x4305, 0, win->winyl + 16);
    for (c = 0; c < win->winxl16; ++c) {
        if (c == 0 || c != win->winh16) {
            drawshape(&win->border, 0x4304, c * 16 + 8, 0);
            drawshape(&win->border, 0x4306, c * 16 + 8, win->winyl + 16);
        }
    }
    drawshape(&win->border, 0x4303, win->winxl + 8, 0);
    for (c = 0; c < win->winyl16; ++c) {
        drawshape(&win->border, 0x4301, win->winxl + 8, c * 16 + 16);
        if (c == 0 || c != win->winv16)
            drawshape(&win->border, 0x4300, 0, c * 16 + 16);
    }
    if (win->winflags & dialog) {
        drawshape(&win->border, 0x430e, win->winxl + 8, 32);
        drawshape(&win->border, 0x430f, win->winxl + 8, win->winyl - 16);
    }
    if (win->winh16 <= 0) return;

    drawshape(&win->border, 0x430a, win->winh + 8, 0);
    for (c = 0; c < win->winyl16; ++c) {
        if (c == 0 || c != win->winv16)
            drawshape(&win->border, 0x4309, win->winh + 8, c * 16 + 16);
    }
    drawshape(&win->border, 0x4308, win->winh + 8, win->winyl + 16);
    if (win->winv <= 0) return;

    drawshape(&win->border, 0x430d, 0, win->winv + 16);
    for (c = 0; c < win->winh16; ++c)
        drawshape(&win->border, 0x430c, c * 16 + 8, win->winv + 16);
    drawshape(&win->border, 0x430b, win->winh + 8, win->winv + 16);
}

void undrawwin(wintype *win)
{
    clearvp(&win->border);
}

void wprint(vptype *vp, int x, int y, int font, const char *text)
{
    int fontx;
    size_t c;

    if (curhi != vp->vphi || curback != vp->vpback)
        fontcolor(vp, vp->vphi, vp->vpback);
    if (font == 1) fontx = 8;
    else if (font == 2) fontx = 6;
    else fontx = 0;
    if (fontx != 0) {
        for (c = 0; c < strlen(text); ++c)
            drawshape(vp, (font << 8) + (text[c] & 0x7f),
                      x + fontx * (int)c, y);
    }
}

void wprintc(vptype *vp, int y, int font, const char *text)
{
    int fontx;
    if (font == 1)
        fontx = 8;
    else if (font == 2)
        fontx = 6;
    else
        fontx = 0;
    wprint(vp, vp->vpxl - (int)strlen(text) * fontx / 2,
           y, font, text);
}

int wgetkey(vptype *vp, int x, int y, int font)
{
    char tempstr[2] = {0, 0};
    uword oldclock;

    while (!k_pressed()) {
        oldclock = *myclock;
        while (oldclock == *myclock) { }
        cursorchar = (byte)((cursorchar & 7) + 1);
        tempstr[0] = (char)cursorchar;
        wprint(vp, x, y, font, tempstr);
    }
    wprint(vp, x, y, font, " ");
    return k_read();
}

void winput(vptype *vp, int x, int y, int font, char *text, int maxlen)
{
    int input, fontx, templen;
    char tempstr[2] = {0, 0};
    int firstflag = 1;

    if (font == 1) fontx = 8;
    else if (font == 2) fontx = 6;
    else fontx = 0;
    wprint(vp, x, y, font, text);
    do {
        input = wgetkey(vp, x + fontx * (int)strlen(text), y, font);
        if (input >= 32 && input < 128) {
            if (firstflag) {
                firstflag = 0;
                while (text[firstflag] != '\0') text[firstflag++] = ' ';
                text[firstflag++] = ' ';
                text[firstflag] = '\0';
                wprint(vp, x, y, font, text);
                text[0] = '\0';
            }
            if ((int)strlen(text) < maxlen) {
                templen = (int)strlen(text);
                text[templen] = (char)input;
                text[templen + 1] = '\0';
                tempstr[0] = (char)input;
                wprint(vp, x + fontx * templen, y, font, tempstr);
            }
        } else if ((input == k_bs || input == key_left) && strlen(text) > 0) {
            text[strlen(text) - 1] = '\0';
        }
        firstflag = 0;
    } while (input != key_enter && input != key_escape);
}

void titlewin(wintype *win, const char *text)
{
    wprint(&win->border,
           16 + (win->winxl + win->winh) / 2 - 4 * (int)strlen(text),
           4, 1, text);
}

void titletop(wintype *win, const char *text)
{
    wprint(&win->border,
           8 + win->winh / 2 - 3 * (int)strlen(text), 5, 2, text);
}

void titlebot(wintype *win, const char *text)
{
    wprint(&win->border,
           8 + win->winh / 2 - 3 * (int)strlen(text),
           win->winyl + 19, 2, text);
}

void clearvp(vptype *vp)
{
    clrvp(vp, (byte)vp->vpback);
}

void fontcolor(vptype *vp, int hi, int back)
{
    int lo;

    /* Jill leaves lo uninitialized on the CGA/default path. */
    hi = -abs(hi);
    switch (x_ourmode & 0xfe) {
    case x_ega:
        if (hi >= 0) {
            hi = (hi & 7) + 8;
            lo = hi & 7;
        } else {
            hi = ((-hi) & 7) + 8;
            lo = hi;
        }
        if (back != -1) back &= 15;
        break;
    case x_vga:
        if (hi >= 0) {
            hi = (hi & 7) + 8;
            lo = hi & 7;
        } else {
            hi = ((-hi) & 7) + 8;
            lo = hi;
        }
        break;
    default:
        break;
    }
    fntcolor(hi, lo, back);
    vp->vphi = (word)hi;
    vp->vpback = (word)back;
    curhi = (word)hi;
    curback = (word)back;
}
