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

#include "JILL.H"
#include "MUSIC.H"

static word lastwater;

static const word blinkshtab[16] = {
    0x0008, 0x0008, 0x0008, 0x0008,
    0x0009, 0x000a, 0x000b, 0x000c,
    0x0026, 0x0026, 0x0026, 0x0026,
    0x0026, 0x000b, 0x000a, 0x0009
};

static const word animtab[4] = {
    -1, -2, -1, 0
};

int msg_block(int x, int y, int msg)
{
    int bk;
    int gc;
    int xc;
    int yc;
    int frame;
    int shape;
    int c;
    int d;
    int bit;
    int mask;
    int n;

    bk = board(x, y);
    gc = gamecount & 3;
    xc = x << 4;
    yc = y << 4;

    if (msg == msg_touch) {
        if (bk >= 0x186 && bk <= 0x18b) {
            p_ouch(16, 1);
            explode2(0);
        } else if (bk == 0x141 || bk == 0x142 ||
                   bk == 0x155 || bk == 0x156 || bk == 0x0a6) {
            p_ouch(16, 2);
        } else if (bk >= 0x190 && bk <= 0x197) {
            if (objs[0].objkind != obj_jillfrog &&
                objs[0].objkind != obj_jillfish)
                p_ouch(16, 1);
        } else if (bk >= 0x198 && bk <= 0x1a3) {
            if (gamecount - lastwater > 10)
                snd_play(2, 4);
            lastwater = gamecount;
        }
        return 0;
    }

    if (msg == msg_update) {
        if (bk >= 0x186 && bk <= 0x18a && gc == 0) {
            setboard(x, y, bk + 1);
            if (board(x, y) > 0x18a)
                setboard(x, y, 0x186);
            return 1;
        }

        if (bk == 0x0be)
            return gc == 1;

        if (bk == 0x06e && gc == 3)
            return 1;

        if (bk == 0x0a6 && gc == 1)
            return 1;

        if (bk == 0x081 && (gamecount & 7) == 2) {
            frame = (gamecount >> 3) & 15;
            info[0x081].flags |= f_notstair;
            info[0x081].sh = (info[0x081].sh & 0xff00) + blinkshtab[frame];
            if (frame < 8 || frame >= 13)
                info[0x081].flags ^= f_notstair;
            return 1;
        }

        if (bk == 0x15f || bk == 0x160) {
            if (bk == 0x160)
                frame = (gamecount - x) & 31;
            else
                frame = (gamecount + x) & 31;

            if (frame < 16)
                return 1;
            if (frame > 16)
                return 0;

            if (bk == 0x160) {
                n = addobj(obj_fire, xc, yc + 16);
                objs[n].yd = 1;
            } else {
                n = addobj(obj_fire, xc, yc - kindyl[obj_fire]);
                objs[n].yd = -1;
            }
            snd_play(3, 26);
            return 0;
        }

        if (bk == 0x021 && (gamecount & 0x3f) == (x & 0x3f)) {
            n = addobj(obj_fire, xc, yc - kindyl[obj_fire] + 8);
            objs[n].yd = -1;
            snd_play(3, 26);
            return 0;
        }

        if (bk >= 0x190 && bk <= 0x1a5 && (gc & 1)) {
            setboard(x, y, bk + 1);
            switch (board(x, y)) {
            case 0x194:
                setboard(x, y, 0x190);
                break;
            case 0x198:
                setboard(x, y, 0x194);
                break;
            case 0x19c:
                setboard(x, y, 0x198);
                break;
            case 0x1a0:
                setboard(x, y, 0x19c);
                break;
            case 0x1a3:
                setboard(x, y, 0x1a0);
                break;
            case 0x1a6:
                setboard(x, y, 0x1a3);
                break;
            }
            return 1;
        }

        if (bk >= 0x014 && bk <= 0x017)
            return (gamecount & 7) == 2;

        return 0;
    }

    if (msg == msg_draw) {
        if (bk == 0x15f || bk == 0x160) {
            shape = info[bk].sh;
            if (bk == 0x160)
                frame = (gamecount - x) & 31;
            else
                frame = (gamecount + x) & 31;
            if (frame < 16)
                shape += animtab[frame >> 2];
            drawshape(gamevp, shape, xc, yc);
        } else if (bk >= 0x064 && bk <= 0x066) {
            drawshape(gamevp, info[board(x - 1, y)].sh, xc, yc);
            drawshape(gamevp, info[bk].sh ^ 0x4000, xc, yc);
        } else if (bk >= 0x067 && bk <= 0x069) {
            drawshape(gamevp, info[board(x, y - 1)].sh, xc, yc);
            drawshape(gamevp, info[bk].sh ^ 0x4000, xc, yc);
        } else if (bk == 0x0be) {
            drawshape(gamevp, info[0x0be].sh + ((gamecount >> 2) & 3), xc, yc);
        } else if (bk == 0x0a6) {
            drawshape(gamevp, info[0x0a6].sh + ((gamecount >> 2) & 3), xc, yc);
        } else if (bk == 0x06e) {
            drawshape(gamevp, info[0x06e].sh + ((gamecount >> 2) & 3), xc, yc);
        } else if (bk == 0x02d) {
            drawshape(gamevp, info[bk].sh ^ 0x4000, xc, yc);
        } else if (bk >= 0x014 && bk <= 0x017) {
            frame = ((gamecount >> 3) - y) % 3;
            if (frame < 0)
                frame += 3;
            drawshape(gamevp, info[bk].sh + frame, xc, yc);

            bit = 1;
            mask = 0;
            for (d = y - 1; d <= y + 1; ++d) {
                for (c = x - 1; c <= x + 1; ++c) {
                    n = board(c, d);
                    if (n < 0x014 || n > 0x017)
                        mask += bit;
                    bit *= 2;
                }
            }

            if ((mask & 0x0aa) == 0x0aa)
                drawshape(gamevp, 0x2a05, xc, yc);
            else if ((mask & 0x00a) == 0x00a)
                drawshape(gamevp, 0x2a01, xc, yc);
            else if ((mask & 0x022) == 0x022)
                drawshape(gamevp, 0x2a02, xc, yc);
            else if ((mask & 0x088) == 0x088)
                drawshape(gamevp, 0x2a03, xc, yc);
            else if ((mask & 0x0a0) == 0x0a0)
                drawshape(gamevp, 0x2a04, xc, yc);
        }
    }

    return 0;
}
