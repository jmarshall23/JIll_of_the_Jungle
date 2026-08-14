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

#include "DESIGN.H"

#include "GAMECTRL.H"
#include "HOSTSDL.H"
#include "JILL.H"
#include "JUNGLE.H"
#include "KEYBOARD.H"
#include "SHM.H"
#include "WINDOWS.H"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static word disy;
static word copied_object;

void infname(char *message, char *filename)
{
    clearvp(statvp);
    wprint(statvp, 0, 0, 2, message);
    winput(statvp, 0, 8, 2, filename, 12);
}

void printobjinfo(int n)
{
    char string[16];
    int kind = objs[n].objkind;

    wprint(statvp, 0, 0, 2, "kind:            ");
    wprint(statvp, 30, 0, 2, kindname[kind]);
    wprint(statvp, 0, 6, 2, "stat:            ");
    _itoa(objs[n].state, string, 10);
    wprint(statvp, 30, 6, 2, string);
    wprint(statvp, 0, 12, 2, "  xd:            ");
    _itoa(objs[n].xd, string, 10);
    wprint(statvp, 30, 12, 2, string);
    wprint(statvp, 0, 18, 2, "  yd:            ");
    _itoa(objs[n].yd, string, 10);
    wprint(statvp, 30, 18, 2, string);
    wprint(statvp, 0, 24, 2, " cnt:            ");
    _itoa(objs[n].counter, string, 10);
    wprint(statvp, 30, 24, 2, string);
    if (kindflags[kind] & f_inside)
        wprint(statvp, 0, 30, 2, "Text Inside");
}

int objdesign(int x, int y)
{
    char string[65];
    const char *name = NULL;
    int n = -1;
    int changed = 0;
    int c;
    int font;

    x *= 16;
    y = y * 16 + disy;
    for (c = 0; c < numobjs; ++c) {
        if (objs[c].x == x && objs[c].y == y) {
            n = c;
            name = kindname[objs[n].objkind];
        }
    }
    if (n == -1) name = "NONE";

    clearvp(statvp);
    wprint(statvp, 0, 0, 2, "obj:Add Oov");
    wprint(statvp, 0, 6, 2, " Del Paste");
    wprint(statvp, 0, 12, 2, " Kopy Mod");
    wprint(statvp, 30, 18, 2, name);
    key = (word)toupper(wgetkey(statvp, 0, 24, 2));
    clearvp(statvp);

    switch (key) {
    case 'A':
        n = numobjs;
        (void)addobj(obj_killme, x, y);
        changed = 1;
        break;
    case 'D':
        if (n > 0) objs[n].objkind = obj_killme;
        return 1;
    case 'P':
        (void)addobj(objs[copied_object].objkind, x, y);
        memcpy(&objs[numobjs - 1], &objs[copied_object], sizeof(objtype));
        objs[numobjs - 1].x = (word)x;
        objs[numobjs - 1].y = (word)y;
        return 1;
    case 'O':
        objs[copied_object].x = (word)x;
        objs[copied_object].y = (word)y;
        drawboard();
        return 1;
    case 'K':
        if (n >= 0) copied_object = (word)n;
        break;
    case 'M':
        if (n >= 0) changed = 1;
        break;
    default:
        break;
    }

    if (!changed) return 0;

    printobjinfo(n);
    strcpy(string, kindname[objs[n].objkind]);
    winput(statvp, 30, 0, 2, string, 12);
    _strupr(string);
    for (c = 0; c < numobjkinds; ++c) {
        if (strcmp(string, kindname[c]) == 0) {
            objs[n].objkind = (sbyte)c;
            break;
        }
    }

    printobjinfo(n);
    _itoa(objs[n].state, string, 10);
    winput(statvp, 30, 6, 2, string, 12);
    if (string[0] != '\0') objs[n].state = (word)atol(string);
    printobjinfo(n);
    _itoa(objs[n].xd, string, 10);
    winput(statvp, 30, 12, 2, string, 12);
    if (string[0] != '\0') objs[n].xd = (word)atol(string);
    printobjinfo(n);
    _itoa(objs[n].yd, string, 10);
    winput(statvp, 30, 18, 2, string, 12);
    if (string[0] != '\0') objs[n].yd = (word)atol(string);
    printobjinfo(n);
    _itoa(objs[n].counter, string, 10);
    winput(statvp, 30, 24, 2, string, 12);
    if (string[0] != '\0') objs[n].counter = (word)atol(string);

    objs[n].xl = kindxl[objs[n].objkind];
    objs[n].yl = kindyl[objs[n].objkind];
    if (kindflags[objs[n].objkind] & f_inside) {
        printobjinfo(n);
        font = objs[n].objkind == obj_text8 ? 1 : 2;
        if (objs[n].inside == NULL)
            string[0] = '\0';
        else
            strcpy(string, objs[n].inside);
        fontcolor(gamevp, objs[n].xd, objs[n].yd);
        winput(gamevp, objs[n].x, objs[n].y, font, string, 64);
        if (objs[n].inside != NULL) free(objs[n].inside);
        objs[n].inside = (char *)malloc(strlen(string) + 1);
        strcpy(objs[n].inside, string);
        setobjsize(n);
    }
    printobjinfo(n);
    shm_want[kindtable[objs[n].objkind]] = 1;
    shm_do();
    return 1;
}

void design(void)
{
    char string[20] = "";
    char path[64] = "";
    int redraw = 0;
    int tile = 1;
    int painting = 0;
    int x;
    int y;
    int c;
    int original;

    disy = 0;
    designflag = 1;
    setorigin();
    x = objs[0].x / 16;
    y = objs[0].y / 16;
    drawboard();

    do {
        if (painting) {
            setboard(x, y, tile);
            drawcell(x, y);
            redraw = 1;
        }
        drawshape(gamevp, 0x0100, x * 16 + 4, y * 16 + 4);
        _ltoa(host_coreleft(), string, 10);
        wprint(statvp, 0, 0, 2, string);
        do checkctrl(0);
        while (host_is_open() && dx1 == 0 && dy1 == 0 && key == 0 && !redraw);
        redraw = 0;
        modboard(x, y);
        updobjs(0);
        refresh(0);
        purgeobjs();

        if (dx1 != 0 || dy1 != 0) {
            x += (((scrnxs / 2 - 1) * fire1) + 1) * dx1;
            y += (((scrnys / 2 - 1) * fire1) + 1) * dy1;
            if (x < 0) x = 0;
            if (x >= boardxs) x = boardxs - 1;
            if (y < 0) y = 0;
            if (y >= boardys) y = boardys - 1;

            if (x * 16 < gamevp->vpox) {
                gamevp->vpox -= scrnxs * 8;
                if (gamevp->vpox < 0) gamevp->vpox = 0;
                drawboard();
            }
            if (gamevp->vpox + scrnxs * 16 - 16 <= x * 16) {
                gamevp->vpox += scrnxs * 8;
                if (gamevp->vpox >= (boardxs - scrnxs) * 16 + 8)
                    gamevp->vpox = (boardxs - scrnxs) * 16 + 8;
                drawboard();
            }
            if (y * 16 < gamevp->vpoy) {
                gamevp->vpoy -= scrnys * 8;
                if (gamevp->vpoy < 0) gamevp->vpoy = 0;
                drawboard();
            }
            if (gamevp->vpoy + (scrnys - 1) * 16 <= y * 16) {
                gamevp->vpoy += scrnys * 8;
                if (gamevp->vpoy >= (boardys - scrnys + 1) * 16)
                    gamevp->vpoy = (boardys - scrnys + 1) * 16;
                drawboard();
            }
        }

        switch (toupper(key)) {
        case key_enter:
            clearvp(statvp);
            wprint(statvp, 0, 0, 2, "Put:");
            winput(statvp, 0, 8, 2, string, 16);
            _strupr(string);
            for (c = 0; c < numinfotypes; ++c) {
                if (strcmp(string, info[c].name) == 0) {
                    tile = c;
                    setboard(x, y, tile);
                    shm_want[(info[c].sh >> 8) & 0x3f] = 1;
                    shm_do();
                    break;
                }
            }
            redraw = 1;
            break;
        case k_tab:
            painting = !painting;
            break;
        case 'K':
            tile = board(x, y);
            break;
        case key_space:
            setboard(x, y, tile);
            redraw = 1;
            break;
        case 'I':
            pl.score = 100;
            printhi(1);
            break;
        case 'V':
            if (pl.numinv == 0)
                addinv(0);
            else {
                pl.numinv = 0;
                initinv();
            }
            pl.score = 0;
            pl.level = 0;
            drawstats();
            break;
        case 'H':
            original = board(x, y);
            for (c = x; board(c, y) == original; --c) {
                setboard(c, y, tile);
                drawcell(c, y);
            }
            for (c = x + 1; board(c, y) == original; ++c) {
                setboard(c, y, tile);
                drawcell(c, y);
            }
            break;
        case 'O':
            redraw = objdesign(x, y);
            break;
        case 'Z':
            infname("Clear?", path);
            if (toupper((byte)path[0]) == 'Y') {
                initboard();
                initobjs();
                drawboard();
            }
            break;
        case 'L':
            infname("Load:", path);
            if (path[0] != '\0') {
                loadboard(path);
                setorigin();
                x = objs[0].x / 16;
                y = objs[0].y / 16;
                drawboard();
            }
            break;
        case '`':
            do checkctrl(0); while (dx1 == 0 && dy1 == 0);
            scrollvp(gamevp, dx1 * 8, dy1 * 8);
            break;
        case 'Y':
            clearvp(statvp);
            wprint(statvp, 0, 0, 2, "Dis Y:");
            _itoa(disy, string, 10);
            winput(statvp, 0, 8, 2, string, 16);
            disy = (word)atol(string);
            _strupr(string);
            break;
        case 'N':
            infname("New board?", path);
            if (toupper((byte)path[0]) == 'Y') {
                zapobjs();
                initboard();
            }
            break;
        case 'S':
            infname("Save:", path);
            if (path[0] != '\0') saveboard(path);
            break;
        default:
            break;
        }
    } while (host_is_open() && key != key_escape);

    key = 0;
    designflag = 0;
}
