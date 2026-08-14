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

#include "GAMECTRL.H"
#include "KEYBOARD.H"

#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

extern word gamecount;

longword systime;
word dx1, dy1, fire1, fire2, fire1off, fire2off;
word joyflag;
word key;
word dx1hold, dy1hold, flow1;
word dx1old, dy1old;
word joyxsense, joyysense;

word macplay, macrecord, macabort, macaborted, mactime;
static char *macptr;
static char macfname[32];
static uword macofs, maclen;

word joyxl, joyxc, joyxr, joyyu, joyyc, joyyd;
static char keybuf[256];

static void game_cputs(const char *text)
{
    if (text != NULL) fputs(text, stdout);
}

int buttona1(void)
{
    /* Original: (inportb(0x201) & 0x10) == 0. */
    return 0;
}

int buttona2(void)
{
    /* Original: (inportb(0x201) & 0x20) == 0. */
    return 0;
}

void readspeed(void)
{
    int oldclock;
    systime = 0;
    oldclock = *(volatile sbyte *)myclock;
    do { } while (*(volatile sbyte *)myclock == oldclock);
    do { ++systime; } while ((*(volatile sbyte *)myclock - oldclock) < 5);
    systime /= 4L;
}

void readjoy(word *x, word *y)
{
    /* A DOS game-port timeout produced exactly this result. */
    *x = -1;
    *y = -1;
}

int caldir(char *text, word *jx, word *jy)
{
    int result = 0;
    int input = 0;
    game_cputs(text);
    do {
        readjoy(jx, jy);
        if (k_pressed()) input = k_read();
    } while (input != escape && !buttona1());
    jill_delay(25);
    if (input != escape) {
        result = 1;
        do {
            if (k_pressed()) input = k_read();
        } while (buttona1() && input != escape);
    }
    jill_delay(25);
    game_cputs("\r\n");
    return result;
}

int joypresent(void)
{
    word x, y;
    readjoy(&x, &y);
    if (x > 0 && y > 0) {
        joyxsense = x;
        joyysense = y;
        return 1;
    }
    return 0;
}

int calibratejoy(void)
{
    int input;
redo:
    joyflag = 0;
    game_cputs("\r\nJoystick calibration:  Press ESCAPE to abort.\r\n");
    if (caldir("  Center joystick and press button: ", &joyxc, &joyyc) &&
        caldir("  Move joystick to UPPER LEFT corner and press button: ",
               &joyxl, &joyyu) &&
        caldir("  Move joystick to LOWER RIGHT corner and press button: ",
               &joyxr, &joyyd)) {
        joyxl -= joyxc;
        joyxr -= joyxc;
        joyyu -= joyyc;
        joyyd -= joyyc;
        if (joyxl < -1 && joyxr > 1 && joyyu < -1 && joyyd > 1) return 1;
        game_cputs("  Calibration failed - try again (y/N)? ");
        do { } while (!k_pressed());
        game_cputs("\r\n");
        input = k_read();
        if (toupper(input) == 'Y') goto redo;
    }
    return 0;
}

void checkctrl(int pollflag)
{
    word x1, y1, xs, ys;

    if (macplay) {
        getmac();
        return;
    }

    dx1 = 0;
    dy1 = 0;
    fire1 = 0;
    flow1 = 0;
reloop:
    key = 0;
    if (k_pressed()) {
        key = (word)k_read();
        if (key == 0 || key == 1 || key == 2) key = (word)k_read();
    }
    if (key != 0) {
        switch (key) {
        case k_up:
        case '8':
            if (pollflag) goto reloop;
            dx1 = 0;
            dy1 = -1;
            break;
        case k_left:
        case '4':
            if (pollflag) goto reloop;
            dx1 = -1;
            dy1 = 0;
            break;
        case k_right:
        case '6':
            if (pollflag) goto reloop;
            dx1 = 1;
            dy1 = 0;
            break;
        case k_down:
        case '2':
            if (pollflag) goto reloop;
            dx1 = 0;
            dy1 = 1;
            break;
        }
    }
    k_status();
    fire1 = k_shift;
    fire2 = k_alt;
    if (dx1 == 0 && dy1 == 0 && joyflag) {
        readjoy(&x1, &y1);
        xs = (word)(x1 - joyxc);
        ys = (word)(y1 - joyyc);
        dx1 = (word)(((2 * xs) > joyxr) - ((2 * xs) < joyxl));
        dy1 = (word)(((2 * ys) > joyyd) - ((2 * ys) < joyyu));
        if (buttona1()) fire1 = 1;
        if (buttona2()) fire2 = 1;
    }
    if (dx1 == 0 && dy1 == 0 && pollflag) {
        if (keydown[0][scan_cursorleft] || keydown[1][scan_cursorleft]) --dx1;
        if (keydown[0][scan_cursorright] || keydown[1][scan_cursorright]) ++dx1;
        if (keydown[0][scan_cursorup] || keydown[1][scan_cursorup]) --dy1;
        if (keydown[0][scan_cursordown] || keydown[1][scan_cursordown]) ++dy1;
    }

    if (fire1) fire1 ^= fire1off;
    else fire1off = 0;
    if (fire2) fire2 ^= fire2off;
    else fire2off = 0;
    dx1old = dx1;
    dy1old = dy1;
    if (macrecord) recmac();
}

void checkctrl0(int pollflag)
{
    static word oldclock;
    do { } while (oldclock == *(volatile sbyte *)myclock);
    oldclock = *(volatile sbyte *)myclock;
    checkctrl(pollflag);
}

void sensectrlmode(void)
{
    joyflag = (word)joypresent();
}

int gc_config(void)
{
    int input = ' ';
    if (joypresent()) {
        game_cputs("\r\nGame controller:  K)eyboard,  J)oystick?  ");
        do {
            do { } while (!k_pressed());
            input = toupper(k_read());
        } while (input != 'K' && input != 'J' && input != escape);
        game_cputs("\r\n");
        joyflag = 0;
        if (input == 'J') joyflag = (word)calibratejoy();
    }
    return input != escape;
}

void getkey(void)
{
    do {
        checkctrl(0);
    } while (key == 0);
}

void stopmac(void)
{
    macplay = 0;
    macrecord = 0;
    if (macptr != NULL) {
        free(macptr);
        macptr = NULL;
    }
    macofs = 0;
    mactime = 1;
    srand(12345);
}

void playmac(char *filename)
{
    int handle;
    stopmac();
    macaborted = 0;
    handle = _open(filename, _O_BINARY | _O_RDONLY);
    if (handle >= 0) {
        maclen = (uword)_filelength(handle);
        macptr = (char *)malloc(maclen);
        if (macptr == NULL) macptr = NULL;
        else if (_read(handle, macptr, maclen) >= 0) {
            macplay = 1;
            gamecount = 0;
        } else {
            free(macptr);
            macptr = NULL;
        }
        _close(handle);
    }
}

void recordmac(char *filename)
{
    stopmac();
    macptr = (char *)malloc(8000);
    if (macptr != NULL) {
        macofs = 0;
        macrecord = 1;
        strcpy(macfname, filename);
        gamecount = 0;
    }
}

void macrecend(void)
{
    int handle;
    if (!macrecord) return;
    handle = _creat(macfname, 0);
    if (handle >= 0) {
        (void)_write(handle, macptr, macofs);
        _close(handle);
    }
    stopmac();
}

void recmac(void)
{
    static word curdx1, curdy1, curfire1, curfire2, oldclock;
    word dt;
    byte bits;

    if (key == '[') { mactime = 0; key = 0; }
    if (key == ']') { mactime = 1; key = 0; }
    if (key == '}') { macrecend(); return; }
    if (macofs == 0) {
        curdx1 = curdy1 = curfire1 = curfire2 = 0;
        oldclock = gamecount;
    }
    bits = (byte)(((curdx1 != dx1) << 0) |
                  ((curdy1 != dy1) << 1) |
                  ((curfire1 != fire1) << 2) |
                  ((curfire2 != fire2) << 3) |
                  (((key > 0) && (key <= 127)) << 4));
    if (bits) {
        if (macofs != 0) {
            if (mactime == 0) dt = 1;
            else dt = (word)(gamecount - oldclock);
            if (dt < 128) macptr[macofs++] = (char)dt;
            else {
                macptr[macofs++] = (char)((dt & 127) | 128);
                macptr[macofs++] = (char)(dt >> 7);
            }
        }
        macptr[macofs++] = (char)bits;
        if (bits & 1) macptr[macofs++] = (char)dx1;
        if (bits & 2) macptr[macofs++] = (char)dy1;
        if (bits & 4) macptr[macofs++] = (char)fire1;
        if (bits & 8) macptr[macofs++] = (char)fire2;
        if (bits & 16) macptr[macofs++] = (char)key;
        curdx1 = dx1;
        curdy1 = dy1;
        curfire1 = fire1;
        curfire2 = fire2;
    }
    if (macofs >= 30000) macrecend();
}

void getmac(void)
{
    static word oldclock, nextdt;
    int tempkey;
    byte bits;

    if (k_pressed()) {
        tempkey = k_read();
        if (macabort == 0 || (macabort == 1 && tempkey == escape)) {
            stopmac();
            macaborted = 1;
        }
    }
    key = 0;
    if (macofs == 0) {
        dx1 = dy1 = fire1 = fire2 = 0;
        oldclock = gamecount;
        nextdt = 0;
    }
    if ((word)(gamecount - oldclock) >= nextdt) {
        bits = (byte)macptr[macofs++];
        if (bits & 1) dx1 = (word)(sbyte)macptr[macofs++];
        if (bits & 2) dy1 = (word)(sbyte)macptr[macofs++];
        if (bits & 4) fire1 = (word)(sbyte)macptr[macofs++];
        if (bits & 8) fire2 = (word)(sbyte)macptr[macofs++];
        if (bits & 16) key = (word)(sbyte)macptr[macofs++];
        nextdt = (word)(sbyte)macptr[macofs++];
        if (nextdt < 0) {
            nextdt = (word)((nextdt & 127) +
                            ((word)(sbyte)macptr[macofs++] << 7));
        }
    }
    if (macofs >= maclen) stopmac();
}

void gc_init(void)
{
    dx1 = dy1 = fire1 = fire1off = 0;
    dx1old = dy1old = 0;
    dx1hold = dy1hold = 0;
    keybuf[0] = 0;
    macplay = macrecord = 0;
    macabort = 1;
    joyflag = 0;
    installhandler(1);
}

void gc_exit(void)
{
    removehandler();
}
