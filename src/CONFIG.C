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

#include "CONFIG.H"
#include "GAMECTRL.H"
#include "GR.H"
#include "HOSTSDL.H"
#include "KEYBOARD.H"
#include "MUSIC.H"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char cfg_path[260] = "";
word nosnd;
word cfgdemo;

ConfigState cf = { 0, 0, 0, 0, 0, 0, 0, 0, x_vga, 1, 1 };

_Static_assert(sizeof(ConfigState) == 22, "configuration record must be 22 bytes");

void cfg_getpath(int argc, char **argv)
{
    int index;
    for (index = 0; index < argc; ++index) {
        _strupr(argv[index]);
        if (argv[index][0] == '/' && argv[index][1] == 'P')
            strcpy(cfg_path, argv[index] + 2);
    }
}

void cfg_init(int argc, char **argv)
{
    int index;

    host_console_clear();
    fputs("\r\n\r\nDetecting your hardward...\r\n", stdout);
    fputs("\r\nIf your system locks, reboot and type:\r\n", stdout);
    fputs("   JILL1 /NOSB  (No Sound Blaster card)\r\n", stdout);
    fputs("   JILL1 /SB    (With a Sound Blaster)\r\n", stdout);
    fputs("   JILL1 /NOSND (If all else fails)\r\n", stdout);
    readspeed();
    for (index = 0; index < argc; ++index) {
        _strupr(argv[index]);
        if (strcmp(argv[index], "/TEST") == 0) {
            char string[16];
            _ltoa(systime, string, 10);
            fputs(string, stdout);
            getkey();
        } else if (strcmp(argv[index], "/NOSB") == 0) {
            vocflag = musicflag = 0;
        } else if (strcmp(argv[index], "/SB") == 0) {
            /* The original switch is accepted but makes no assignment. */
        } else if (strcmp(argv[index], "/NOSND") == 0) {
            vocflag = musicflag = 0;
            nosnd = 1;
        } else if (strcmp(argv[index], "/DEMO") == 0) {
            cfgdemo = 1;
        }
    }
}

int doconfig(void)
{
    int configure = cf.firstthru;
    char string[16];

    if (!configure) {
        x_ourmode = (byte)cf.video_mode;
        joyflag = (word)joypresent();
        if (!joyflag) {
            cf.joyflag0 = 0;
        } else if (cf.joyflag0) {
            joyxl = cf.joyxl0;
            joyxc = cf.joyxc0;
            joyxr = cf.joyxr0;
            joyyu = cf.joyyu0;
            joyyc = cf.joyyc0;
            joyyd = cf.joyyd0;
            checkctrl(0);
            configure |= dx1 != 0 || dy1 != 0;
        }
        if (!musicflag) cf.musicflag0 = 0;
        if (!vocflag) cf.vocflag0 = 0;
    }
#if 0 // jmarshall
    if (!configure) {
        host_console_clear();
        fputs("\r\n", stdout);
        fputs(" Your configuration:\r\n", stdout);
        if (cf.vocflag0)
            fputs("    Digital Sound Blaster sound effects ON\r\n", stdout);
        else
            fputs("    No digitized sound effects\r\n", stdout);
        if (cf.musicflag0)
            fputs("    Sound Blaster musical sound track ON\r\n", stdout);
        else
            fputs("    No musical sound track\r\n", stdout);
        if (cf.joyflag0)
            fputs("    A joystick\r\n", stdout);
        else
            fputs("    No joystick\r\n", stdout);
        if (x_ourmode == x_cga) {
            fputs("    CGA graphics (You're missing some\r\n", stdout);
            fputs("    hot 256-color VGA scenery!)\r\n", stdout);
        } else if (x_ourmode == x_ega) {
            fputs("    16-color EGA graphics\r\n", stdout);
        } else {
            fputs("    256-color VGA graphics\r\n", stdout);
        }
        fputs("\r\n", stdout);
        fputs("  Press ENTER if this is correct\r\n", stdout);
        fputs("      or press 'C' to configure: ", stdout);

        do {
            getkey();
            key = (word)toupper(key);
        } while (key != enter && key != 'C' && key != escape);
        if (key == 'C') configure = 1;
        if (key == escape) return 0;
    }

    if (configure) {
        host_console_clear();
        if (!vocflag && !musicflag) {
            fputs("\r\n", stdout);
            fputs(" No Sound Blaster-compatible music card has been\r\n", stdout);
            fputs(" detected.\r\n\r\n", stdout);
            fputs(" Press any key to continue...", stdout);
            getkey();
        }

        if (vocflag && systime < 4000L) {
            fputs("\r\n\r\n", stdout);
            fputs(" A Sound Blaster card was detected, but your CPU is\r\n", stdout);
            fputs(" too slow to support digitized sound.  Digital sound\r\n", stdout);
            fputs(" is now OFF.\r\n\r\n", stdout);
            fputs(" Press any key to continue...", stdout);
            getkey();
        } else if (vocflag) {
            fputs(" A Sound Blaster card has been detected.\r\n\r\n", stdout);
            fputs(" This game will play high-quality digital sound\r\n", stdout);
            fputs(" through your Sound Blaster if you wish.\r\n\r\n", stdout);
            fputs(" Warning:  There's a teeny chance this will cause\r\n", stdout);
            fputs(" problems if you have less than 640K of RAM, or\r\n", stdout);
            fputs(" if your computer is not totally compatible.\r\n\r\n", stdout);
            fputs(" Do you want digital sound? ", stdout);
            do {
                getkey();
                key = (word)toupper(key);
                if (key == '~') {
                    _ltoa(host_coreleft(), string, 10);
                    fputs(string, stdout);
                }
                if (key == escape) return 0;
            } while (key != 'Y' && key != 'N');
            cf.vocflag0 = (word)(key == 'Y');
        }

        if (musicflag) {
            host_console_clear();
            fputs("\r\n\r\n\r\n", stdout);
            fputs(" This game features a Sound Blaster-compatible\r\n", stdout);
            fputs(" musical sound track.\r\n\r\n\r\n", stdout);
            fputs(" Do you want the musical sound track? ", stdout);
            do {
                getkey();
                key = (word)toupper(key);
                if (key == escape) return 0;
            } while (key != 'Y' && key != 'N');
            cf.musicflag0 = (word)(key == 'Y');
        }

        host_console_clear();
        fputs("\r\n", stdout);
        if (!gc_config()) return 0;
        cf.joyflag0 = joyflag;

        host_console_clear();
        fputs("\r\n", stdout);
        fputs(" Please tell us about your graphics:\r\n", stdout);
        fputs("     CGA 4-color graphics\r\n", stdout);
        fputs("     EGA 16-color graphics\r\n", stdout);
        fputs("     VGA 256-color graphics\r\n", stdout);
        fputs("\r\n", stdout);
        fputs(" Note: If you have a slow old computer, CGA\r\n", stdout);
        fputs("       graphics are recommended.\r\n", stdout);
        if (!gr_config()) return 0;
    }
#endif
    if (systime < 4000L) {
        vocflag = 0;
        cf.vocflag0 = 0;
    }
    cf.firstthru = 0;
    joyflag = cf.joyflag0;
    cf.joyxl0 = joyxl;
    cf.joyxc0 = joyxc;
    cf.joyxr0 = joyxr;
    cf.joyyu0 = joyyu;
    cf.joyyc0 = joyyc;
    cf.joyyd0 = joyyd;
    cf.video_mode = x_ourmode;
    vocflag = cf.vocflag0;
    musicflag = cf.musicflag0;
    return 1;
}
