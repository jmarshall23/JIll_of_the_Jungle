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
#include "EPISODE.H"
#include "GAMECTRL.H"
#include "MUSIC.H"

#include <stdlib.h>

/* Initialized data at 33BC4 in JILL.EXE. */
static word inv_first[11] = {
    1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1
};
static word first_switch = 1;
static word first_elev = 1;
static word first_touchgem = 1;
static word first_knight = 1;

int msg_token(int n, int msg, int z)
{
    int inventory = objs[n].counter;
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e00 + inv_shape[inventory],
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
#if defined(JILL_EP1)
        if (inventory == inv_coins) {
            if (trymove(n, objs[n].x, objs[n].y + 1)) return 1;
        }
#endif
        return 0;
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (inv_xfm[inventory]) {
                playerxfm(inventory);
            } else {
                if (inv_first[inventory]) {
                    --inv_first[inventory];
                    putbotmsg(inv_getmsg[inventory], 7);
                }
                snd_play(3, 25);
#if defined(JILL_EP1)
                if (inventory != inv_coins || !invcount(inv_coins)) {
                    addinv(inventory);
                    killobj(n);
                }
#elif defined(JILL_EP2)
                addinv(inventory);
                killobj(n);
#endif
            }
            statmodflg |= mod_screen;
#if defined(JILL_EP1)
        } else if (objs[z].objkind == obj_mapdemo ||
                   objs[z].objkind == obj_demon) {
            killobj(n);
            objs[z].info1 = 1;
#endif
        }
    }
    return 0;
}

int msg_fire(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_fire] * 256 + objs[n].counter / 2 +
                  (objs[n].yd > 0 ? 6 : 0),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        ++objs[n].counter;
        if (objs[n].counter >= 12 || objs[n].counter < 0) killobj(n);
        return (objs[n].counter & 1) == 0;
    } else if (msg == msg_touch && z == 0 && objs[n].state != 1) {
        p_ouch(2, die_ash);
        explode2(0);
        objs[n].state = 1;
    }
    return 0;
}

int msg_switch(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_switch] * 256 + objs[n].state,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (dy1 != 0) objs[z].yd = 0;
            if (first_switch) {
                first_switch = 0;
                putbotmsg("Press UP/DOWN to toggle switch", 2);
            }
            if (dy1 < 0 && objs[n].state == 1) {
                objs[n].state = 0;
                snd_play(2, 23);
                sendtrig(objs[n].counter,
                         objs[n].xd == 1 ? msg_trigger : msg_trigon, n);
            } else if (dy1 > 0 && objs[n].state == 0) {
                objs[n].state = 1;
                snd_play(2, 24);
                sendtrig(objs[n].counter,
                         objs[n].xd == 1 ? msg_trigger : msg_trigoff, n);
            }
        }
        return 1;
    } else if (msg == msg_update) {
        return 0;
    }
    return 0;
}

int msg_gem(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_gem] * 256 + objs[n].counter / 2 + 4,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        return (objs[n].counter & 1) == 0;
    } else if (msg == msg_touch && z == 0) {
        if (first_touchgem) {
            first_touchgem = 0;
            putbotmsg("USE GEMS TO OPEN DOORS ON THE MAP", 2);
        }
        addinv(inv_crystal);
        snd_play(3, 16);
        addscore(kindscore[obj_gem], objs[n].x, objs[n].y);
        killobj(n);
    }
    return 0;
}

int msg_boulder(int n, int msg, int z)
{
    int dx, dy;
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e18 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    }
    if (msg == msg_update) {
        if (cando(n, objs[n].x, objs[n].y + 1,
                  f_playerthru | f_notstair) ==
            (f_playerthru | f_notstair)) {
            objs[n].yd = (word)(objs[n].yd + 2);
            if (objs[n].yd > 12) objs[n].yd = 12;
            if (trymove(n, objs[n].x + objs[n].xd,
                        objs[n].y + objs[n].yd) == 1)
                return 1;
            trymove(n, objs[n].x,
                    ((objs[n].y + objs[n].yd - 1) & ~15) +
                    16 - kindyl[obj_boulder]);
            objs[n].xd = 0;
        } else {
            if (objs[n].yd != 0) snd_play(2, 45);
            objs[n].yd = 0;
            if (objs[n].xd == 0) {
                seekplayer(n, &dx, &dy);
                objs[n].xd = (word)(dx * 4);
            }
            objs[n].counter = (word)((objs[n].counter +
                                     (objs[n].xd > 0 ? 1 : -1)) & 3);
            objs[n].yd = 0;
            if (trymove(n, objs[n].x + objs[n].xd, objs[n].y) != 1)
                objs[n].xd = (word)-objs[n].xd;
        }
        return 1;
    }
    return 0;
}

void explode2(int n)
{
    addobj(obj_expl2,
           objs[n].x + (objs[n].xl - kindxl[obj_expl2]) / 2,
           objs[n].y + objs[n].yl - kindyl[obj_expl2]);
}

void explode1(int x, int y, int count)
{
    int piece;
    for (piece = 0; piece < count; ++piece) {
        addobj(obj_expl1, x, y);
        objs[numobjs - 1].xd = (word)(rand() % 7 - 3);
        objs[numobjs - 1].yd = (word)(rand() % 11 - 8);
        objs[numobjs - 1].state = (word)(rand() % 3);
    }
}

int msg_expl1(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_expl1] * 256 + objs[n].state + 12 -
                  (objs[n].counter / 8) * 3,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (++objs[n].counter >= 40 || !onscreen(n)) {
            killobj(n);
        } else {
            if (++objs[n].yd > 12) objs[n].yd = 12;
            moveobj(n, objs[n].x + objs[n].xd,
                    objs[n].y + objs[n].yd);
        }
        return 1;
    }
    return 0;
}

int msg_expl2(int n, int msg, int z)
{
    static const word sequence[9] = {
        32, 31, 30, 29, 28, 29, 30, 31, 32
    };
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_expl2] * 256 + sequence[objs[n].counter],
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (++objs[n].counter >= 9 || !onscreen(n)) killobj(n);
        return 1;
    }
    return 0;
}

int msg_stalag(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e21, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) p_ouch(2, die_fish);
    } else if (msg == msg_update) {
        if (objs[n].yd == 0) return 0;
        if (!trymovey(n, objs[n].x, objs[n].y + objs[n].yd)) {
            snd_play(2, 27);
            killobj(n);
        }
        objs[n].yd = (word)(objs[n].yd + 2);
        if (objs[n].yd > 16) objs[n].yd = 16;
        return 1;
    } else if (msg == msg_trigger || msg == msg_trigon) {
        if (objs[n].yd == 0) {
            objs[n].yd = 2;
            snd_play(3, 47);
        }
        killobj(z);
    }
    return 0;
}

int msg_snake(int n, int msg, int z)
{
    static const word head_shape[4] = { 0, 1, 0, 2 };
    static const word middle_shape[4] = { 3, 5, 4, 6 };
    static const word tail_shape[4] = { 7, 7, 8, 8 };
    int shape = kindtable[obj_snake] * 256;
    int c;

    if (msg == msg_draw) {
        if (objs[n].xd > 0) {
            drawshape(gamevp, shape + head_shape[objs[n].counter],
                      objs[n].x + objs[n].xl - 16, objs[n].y);
            for (c = 1; c <= (objs[n].xl - 24) / 8; ++c)
                drawshape(gamevp, shape + middle_shape[objs[n].counter],
                          objs[n].x + c * 8, objs[n].y + 12);
            drawshape(gamevp, shape + tail_shape[objs[n].counter],
                      objs[n].x, objs[n].y + 12);
        } else {
            drawshape(gamevp, shape + head_shape[objs[n].counter] + 9,
                      objs[n].x, objs[n].y);
            for (c = 1; c <= (objs[n].xl - 24) / 8; ++c)
                drawshape(gamevp, shape + middle_shape[objs[n].counter],
                          objs[n].x + (c + 1) * 8, objs[n].y + 12);
            drawshape(gamevp, shape + tail_shape[objs[n].counter] + 5,
                      objs[n].x + objs[n].xl - 8, objs[n].y + 12);
        }
    } else if (msg == msg_update) {
        if (objs[n].state > 0)
            --objs[n].state;
        objs[n].counter = (word)((objs[n].counter + 1) & 3);
        if (!crawl(n, objs[n].xd, 0))
            objs[n].xd = (word)-objs[n].xd;
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0) {
            hitplayer(n);
            return 1;
        }
        if ((kindflags[(byte)objs[z].objkind] & f_weapon) &&
            objs[n].state == 0) {
            notemod(n);
            objs[n].state = 16;
            objs[n].xl -= 8;
            if (objs[n].xl < 24)
                playerkill(n);
            else
                snd_play(3, 31);
        }
    }
    return 0;
}

int msg_boll(int n, int msg, int z)
{
    static const word size[6] = { 4, 6, 8, 10, 12, 14 };
    int xd = objs[n].xd;
    int yd = 1;
    int closest = -1;
    int closest_distance = 96;
    int distance;
    int candidate;
    int c;
    int floor_y;

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_boll] * 256 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
    } else if (msg == msg_update) {
        objs[n].xl = size[objs[n].counter];
        objs[n].yl = size[objs[n].counter];

        for (c = 0; c < numscrnobjs; ++c) {
            candidate = scrnobjs[c];
            if (objs[candidate].objkind == obj_boll &&
                objs[candidate].counter > objs[n].counter) {
                distance = vectdist(n, candidate);
                if (distance < closest_distance) {
                    closest = candidate;
                    closest_distance = distance;
                }
            }
        }

        if (closest >= 0)
            pointvect(closest, n, &xd, &yd, 4);

        objs[n].xd = (word)(objs[n].xd + xd);
        if (objs[n].xd > 12)
            objs[n].xd = 12;
        if (objs[n].xd < -12)
            objs[n].xd = -12;

        objs[n].yd = (word)(objs[n].yd + yd);
        if (objs[n].yd > 12)
            objs[n].yd = 12;
        if (objs[n].yd < -12)
            objs[n].yd = -12;

        if (!justmove(n, objs[n].x + objs[n].xd, objs[n].y))
            objs[n].xd = (word)-objs[n].xd;

        if (!justmove(n, objs[n].x, objs[n].y + objs[n].yd)) {
            floor_y = ((objs[n].y + objs[n].yd) & 0xfff0) +
                      16 - objs[n].yl;
            if (!justmove(n, objs[n].x, floor_y)) {
                objs[n].yd = (word)-objs[n].yd;
                snd_play(1, 9);
            }
        }
        return 1;
    }
    return 0;
}

int msg_mega(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_mega] * 256 + objs[n].xd,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) return 0;
        objs[n].yd = (word)(objs[n].yd + 2);
        if (objs[n].yd > 16) objs[n].yd = 16;
        if (!justmove(n, objs[n].x, objs[n].y + objs[n].yd)) {
            objs[n].yd = (word)(2 - objs[n].yd);
            if (objs[n].yd == 0) return 0;
            snd_play(2, 28);
        }
        return 1;
    } else if (msg == msg_trigger) {
        objs[n].state = 1;
        return 1;
    }
    return 0;
}

int msg_bat(int n, int msg, int z)
{
    static const word y_offset[4] = { 0, 8, 0, 8 };
    static const word draw_sequence[4] = { 1, 2, 3, 2 };
    int shape = kindtable[obj_bat] * 256;
    int x_offset = 0;
    int y;
    int moved;

    if (msg == msg_draw) {
        if (objs[n].state == 0) {
            x_offset = 6;
            y = 2;
        } else {
            y = y_offset[objs[n].counter / 2];
            shape += draw_sequence[objs[n].counter / 2] +
                     (objs[n].xd < 0 ? 3 : 0);
        }
        drawshape(gamevp, shape,
                  objs[n].x + x_offset, objs[n].y + y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        if (objs[n].state == 0) {
            if (rand() % 40 != 0)
                return 0;
            objs[n].state = 1;
            objs[n].xd = (word)((rand() % 2) * 8 - 4);
            objs[n].yd = 2;
            return 1;
        }

        moved = trymove(n, objs[n].x + objs[n].xd,
                        objs[n].y + objs[n].yd);
        if (moved == 1) {
            if (rand() % 35 == 0)
                objs[n].xd = (word)-objs[n].xd;
            if (rand() % 20 != 0)
                return 1;
            objs[n].yd = (word)-objs[n].yd;
        } else if (moved == 2) {
            objs[n].xd = (word)-objs[n].xd;
        } else {
            objs[n].xd = (word)-objs[n].xd;
            if (rand() % 3 == 0 && objs[n].yd < 0) {
                objs[n].state = 0;
                trymove(n, objs[n].x, objs[n].y & 0xfff0);
            } else {
                objs[n].yd = (word)-objs[n].yd;
            }
        }
        return 1;
    }
    return 0;
}

int msg_knight(int n, int msg, int z)
{
    static const word x_offset[11] = {
        8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 8
    };
    static const word draw_sequence[11] = {
        0, 0, 1, 2, 3, 4, 4, 3, 2, 1, 0
    };

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_knight] * 256 +
                  draw_sequence[objs[n].statecount],
                  objs[n].x + x_offset[objs[n].statecount],
                  objs[n].y);
    } else if (msg == msg_update) {
        objs[n].state = 0;
        if (objs[n].statecount == 6)
            objs[n].state = 1;
        if ((gamecount & 1) == 0 || objs[n].statecount == objs[n].xd)
            return 0;
        if (++objs[n].statecount >= 10) {
            objs[n].statecount = 0;
            objs[n].xd = 0;
        }
        return 1;
    } else if (msg == msg_trigger) {
        objs[n].xd = 10;
    } else if (msg == msg_trigon) {
        objs[n].xd = 6;
    } else if (msg == msg_trigoff) {
        objs[n].xd = 0;
    } else if (msg == msg_touch) {
        if (z == 0 && objs[n].state == 1) {
            putbotmsg("The knight slices Jill in half", 7);
            p_ouch(16, 2);
        } else if ((kindflags[(byte)objs[z].objkind] & f_weapon) &&
                   first_knight != 0) {
            putbotmsg("YOUR FEEBLE ATTEMPT FAILS.", 2);
            first_knight = 0;
        }
    }
    return 0;
}

int msg_beenest(int n, int msg, int z)
{
    int dx, dy;
    int shape = kindtable[obj_beenest] * 256;
    if (msg == msg_draw) {
        if (objs[n].counter == 1) ++shape;
        else if (objs[n].counter == 2)
            shape += objs[n].xd > 0 ? 3 : 2;
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (kindflags[(byte)objs[z].objkind] & f_weapon) {
            addscore(kindscore[obj_beenest], objs[n].x, objs[n].y);
            explode1(objs[n].x, objs[n].y, 10);
            killobj(n);
            snd_play(3, 32);
        }
    } else if (msg == msg_update) {
        if ((gamecount & 3) == 2) {
            if (objs[n].counter == 0) {
                if (rand() % 32 == 0) {
                    objs[n].counter = 1;
                    seekplayer(n, &dx, &dy);
                    objs[n].xd = (word)dx;
                    objs[n].yd = (word)dy;
                } else {
                    return 0;
                }
            } else {
                if (++objs[n].counter > 2) {
                    objs[n].counter = 0;
                    addobj(obj_beeswarm,
                           objs[n].x + objs[n].xd * 8, objs[n].y);
                }
            }
        }
        return 1;
    }
    return 0;
}

int msg_beeswarm(int n, int msg, int z)
{
    static const word move_table[32] = {
        0, 0, 0, 0, 1, 1, 1, 1,
        2, 2, 2, 2, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 2, 2, 2,
        2, 1, 1, 1, 1, 0, 0, 0
    };
    int xd;
    int yd;

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_beeswarm] * 256 + objs[n].counter +
                  (objs[n].xd > 0 ? 3 : 0) + 4,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
    } else if (msg == msg_update) {
        if (++objs[n].state > 160) {
            killobj(n);
            return 1;
        }
        if (++objs[n].counter > 2)
            objs[n].counter = 0;

        seekplayer(n, &xd, &yd);
        objs[n].xd = (word)(((rand() % 2 + xd * 2) *
                             move_table[objs[n].state & 31]) / 2);
        objs[n].yd = (word)(((rand() % 2 + yd * 2) *
                             move_table[(objs[n].state + 16) & 31]) / 2);
        justmove(n, objs[n].x + objs[n].xd,
                 objs[n].y + objs[n].yd);
        return 1;
    }
    return 0;
}
int msg_crab(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_crab] * 256 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 3);

        if (objs[n].state == 0) {
            if (objs[n].xd == 0)
                objs[n].xd = (word)((rand() % 2) * 8 - 4);
            if ((objs[n].x & 15) == 0 &&
                !cando(n, objs[n].x, objs[n].y, 4) &&
                rand() % 2 == 0)
                objs[n].state = 1;
            if (objs[n].state == 0 &&
                !crawl(n, objs[n].xd, 0))
                objs[n].xd = (word)-objs[n].xd;
        }

        if (objs[n].state == 1) {
            if (objs[n].yd == 0)
                objs[n].yd = (word)((rand() % 2) * 4 - 2);
            if (!justmove(n, objs[n].x, objs[n].y + objs[n].yd)) {
                objs[n].yd = (word)-objs[n].yd;
                justmove(n, objs[n].x, objs[n].y + objs[n].yd);
            }
            if ((objs[n].y & 15) == 0) {
                if (crawl(n, objs[n].xd, 0)) {
                    objs[n].state = 0;
                } else if (crawl(n, -objs[n].xd, 0)) {
                    objs[n].state = 0;
                    objs[n].xd = (word)-objs[n].xd;
                }
            }
        }
        return 1;
    }
    return 0;
}
int msg_croc(int n, int msg, int z)
{
    static const word head_shape[4] = { 4, 5, 6, 7 };
    static const word tail_shape[4] = { 0, 1, 2, 3 };
    int shape = kindtable[obj_croc] * 256;
    int xd;
    int yd;

    if (msg == msg_draw) {
        if (objs[n].xd > 0) {
            drawshape(gamevp, shape + head_shape[objs[n].counter],
                      objs[n].x + 32, objs[n].y);
            drawshape(gamevp, shape + tail_shape[objs[n].counter],
                      objs[n].x, objs[n].y);
        } else {
            drawshape(gamevp, shape + head_shape[objs[n].counter] + 8,
                      objs[n].x, objs[n].y);
            drawshape(gamevp, shape + tail_shape[objs[n].counter] + 8,
                      objs[n].x + 32, objs[n].y);
        }
    } else if (msg == msg_update) {
        if (objs[n].state > 0)
            --objs[n].state;
        objs[n].counter = (word)((objs[n].counter + 1) & 3);
        if (!crawl(n, objs[n].xd, 0))
            objs[n].xd = (word)-objs[n].xd;
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0) {
            hitplayer(n);
            return 1;
        }
        if ((info[(byte)objs[z].objkind].flags & f_weapon) &&
            objs[n].state == 0) {
            notemod(n);
            objs[n].state = 16;
            seekplayer(n, &xd, &yd);
            if (objs[n].xd * xd > 0)
                objs[n].xd = (word)-objs[n].xd;
            else
                playerkill(n);
        }
    }
    return 0;
}

int msg_epic(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_epic] * 256 + (gamecount & 7),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        return 1;
    } else if (msg == msg_touch && z == 0) {
        addscore(25, objs[n].x, objs[n].y);
        if (++objs[n].state > 10) {
            explode1(objs[n].x, objs[n].y, 10);
            killobj(n);
            snd_play(3, 48);
        } else {
            snd_play(2, 32);
        }
    }
    return 0;
}

int msg_spinblad(int n, int msg, int z)
{
    int xd = 0;
    int yd = 0;
    int closest;
    int closest_distance;
    int candidate;
    int distance;
    int floor_y;
    int c;

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_spinblad] * 256 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (objs[n].substate > 7)
                killobj(n);
        } else if (kindflags[(byte)objs[z].objkind] & f_killable) {
            killobj(n);
            playerkill(z);
            snd_play(3, 10);
        }
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter +
                                  (objs[n].xd > 0) -
                                  (objs[n].xd < 0)) & 3);
        if (++objs[n].substate >= 64 || !onscreen(n)) {
            killobj(n);
            return 1;
        }

        closest = -1;
        closest_distance = 0x7fff;
        for (c = 0; c < numscrnobjs; ++c) {
            candidate = scrnobjs[c];
            if ((kindflags[(byte)objs[candidate].objkind] & f_killable) &&
                candidate != 0) {
                distance = vectdist(n, candidate);
                if (distance < closest_distance && distance < 96) {
                    closest = candidate;
                    closest_distance = distance;
                }
            }
        }
        if (closest >= 0)
            pointvect(closest, n, &xd, &yd, 3);

        yd = 1;
        objs[n].xd = (word)(objs[n].xd + xd);
        if (objs[n].xd > 12)
            objs[n].xd = 12;
        if (objs[n].xd < -12)
            objs[n].xd = -12;
        objs[n].yd = (word)(objs[n].yd + yd);
        if (objs[n].yd > 12)
            objs[n].yd = 12;
        if (objs[n].yd < -12)
            objs[n].yd = -12;

        objs[n].substate = (word)(objs[n].substate +
                                  trybreakwall(n,
                                               objs[n].x + objs[n].xd,
                                               objs[n].y + objs[n].yd) * 10);

        if (!justmove(n, objs[n].x + objs[n].xd, objs[n].y))
            objs[n].xd = (word)-objs[n].xd;

        floor_y = ((objs[n].y + objs[n].yd) & 0xfff0) +
                  16 - kindyl[obj_spinblad];
        if (!justmove(n, objs[n].x, objs[n].y + objs[n].yd)) {
            snd_play(1, 35);
            if (objs[n].y == floor_y ||
                !justmove(n, objs[n].x, floor_y))
                objs[n].yd = (word)-objs[n].yd;
        }

        if (!justmove(n, objs[n].x, objs[n].y))
            killobj(n);
        return 1;
    }
    return 0;
}

int msg_skull(int n, int msg, int z)
{
    static const word moving_shape[8] = { 3, 4, 5, 6, 7, 6, 5, 4 };
    static const word idle_shape[4] = { 0, 1, 2, 1 };
    int shape = kindtable[obj_skull] * 256;
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  shape + idle_shape[(objs[n].statecount / 2) & 3],
                  objs[n].x, objs[n].y);
        if (objs[n].state != 0) {
            drawshape(gamevp, shape + moving_shape[objs[n].statecount],
                      objs[n].x, objs[n].y + 5);
            drawshape(gamevp,
                      shape + moving_shape[(objs[n].statecount + 4) & 7],
                      objs[n].x + 10, objs[n].y + 6);
        }
    } else if (msg == msg_update) {
        if (objs[n].state == 0) return 0;
        objs[n].statecount = (word)((objs[n].statecount + 1) & 7);
        return 1;
    } else if (msg == msg_trigger) {
        if (objs[n].state != 0) return 0;
        objs[n].state = 1;
        objs[n].statecount = 0;
        snd_play(4, 33);
        return 1;
    }
    return 0;
}

int msg_button(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_button] * 256 + 1 - objs[n].state +
                  (objs[n].xd > 0 ? 2 : 0),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (objs[n].substate == 0) {
            objs[n].state = (word)(1 - objs[n].state);
            sendtrig(objs[n].counter,
                     objs[n].state == 1 ? msg_trigoff : msg_trigon, n);
            snd_play(3, 44);
        }
        objs[n].substate = 3;
        return 1;
    } else if (msg == msg_update) {
        if (objs[n].substate > 0) --objs[n].substate;
        return 0;
    }
    return 0;
}

int msg_pac(int n, int msg, int z)
{
    int shape = kindtable[obj_pac] * 256;
    int cell_x;
    int cell_y;
    int cell;
    int xd;
    int yd = 0;
    int turn;

    if (msg == msg_draw) {
        if (objs[n].xd < 0)
            ++shape;
        else if (objs[n].yd > 0)
            shape += 3;
        else if (objs[n].yd < 0)
            shape += 2;
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
        else if (objs[z].objkind == obj_firebullet)
            killobj(n);
    } else if (msg == msg_update) {
        if ((objs[n].x & 15) == 0 && (objs[n].y & 15) == 0) {
            cell_x = objs[n].x / 16;
            cell_y = objs[n].y / 16;
            cell = board(cell_x, cell_y);
            xd = (objs[n].xd > 0) - (objs[n].xd < 0);
            yd = (objs[n].yd > 0) - (objs[n].yd < 0);

            if (xd == 0 && yd == 0) {
                if (rand() % 2 == 0)
                    xd = 1;
                else
                    yd = 1;
            }

            if (board(cell_x + xd, cell_y + yd) != cell) {
                turn = (rand() % 2) * 2 - 1;
                xd *= turn;
                yd *= turn;
                turn = xd;
                xd = yd;
                yd = turn;

                if (board(cell_x + xd, cell_y + yd) != cell) {
                    xd = -xd;
                    yd = -yd;
                    if (board(cell_x + xd, cell_y + yd) != cell) {
                        xd = -((objs[n].xd > 0) - (objs[n].xd < 0));
                        yd = -((objs[n].yd > 0) - (objs[n].yd < 0));
                        if (board(cell_x + xd, cell_y + yd) != cell) {
                            xd = 0;
                            yd = 0;
                        }
                    }
                }
            }
            objs[n].xd = (word)(objs[n].counter * xd);
            objs[n].yd = (word)(objs[n].counter * yd);
        }

        trymove(n, objs[n].x + objs[n].xd,
                objs[n].y + objs[n].yd);
        return 1;
    }
    return 0;
}

int msg_bubble(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_bubble] * 256 + objs[n].counter + 6,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (rand() % 15 == 0) ++objs[n].counter;
        if (objs[n].counter <= 2 && onscreen(n) &&
            fishdo(n, objs[n].x + rand() % 3 - 1,
                   objs[n].y - objs[n].counter - 1))
            return 1;
        killobj(n);
        return 1;
    }
    return 0;
}

int msg_jellyfish(int n, int msg, int z)
{
    static const word draw_sequence[10] = {
        9, 10, 11, 12, 13, 14, 13, 12, 11, 10
    };

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_jellyfish] * 256 +
                  draw_sequence[objs[n].counter],
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (++objs[n].counter >= 10)
            objs[n].counter = 0;

        objs[n].xd = (word)(objs[n].xd +
                            (rand() % 3 - 1) *
                            ((abs(objs[n].xd) < 3) + 1));
        if (objs[n].xd > 6)
            objs[n].xd = 6;
        if (objs[n].xd < -6)
            objs[n].xd = -6;

        objs[n].yd = (word)(objs[n].yd +
                            (rand() % 3 - 1) *
                            ((objs[n].yd < 2) + 1));
        if (objs[n].yd > 6)
            objs[n].yd = 6;
        if (objs[n].yd < -6)
            objs[n].yd = -6;

        if (!fishdo(n, objs[n].x + objs[n].xd, objs[n].y))
            objs[n].xd = (word)-objs[n].xd;
        if (!fishdo(n, objs[n].x, objs[n].y + objs[n].yd))
            objs[n].yd = (word)-objs[n].yd;
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0) {
            hitplayer(n);
        } else if (kindflags[(byte)objs[z].objkind] & f_weapon) {
            explode1(objs[n].x, objs[n].y, 20);
            killobj(n);
        }
    }
    return 0;
}
int msg_badfish(int n, int msg, int z)
{
    static const word draw_sequence[4] = { 0, 1, 2, 1 };

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_badfish] * 256 +
                  draw_sequence[objs[n].counter & 3] +
                  (objs[n].xd > 0 ? 3 : 0) + 15,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        ++objs[n].counter;
        if (rand() % 20 == 0) {
            objs[n].xd = (word)((rand() % 3) * 4 - 4);
            if (objs[n].xd == 0)
                objs[n].yd = (word)((rand() % 2) * 4 - 2);
            else
                objs[n].yd = (word)((rand() % 3) * 2 - 2);
        }

        if (!fishdo(n, objs[n].x + objs[n].xd, objs[n].y))
            objs[n].xd = (word)-objs[n].xd;
        if (!fishdo(n, objs[n].x, objs[n].y + objs[n].yd))
            objs[n].yd = (word)-objs[n].yd;

        if (rand() % 4 == 0)
            addobj(obj_bubble, objs[n].x + 6, objs[n].y - 2);
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
        else if (kindflags[(byte)objs[z].objkind] & f_weapon)
            playerkill(n);
    }
    return 0;
}

int msg_elev(int n, int msg, int z)
{
    int xc = objs[n].x >> 4;
    int yc = objs[n].y >> 4;

    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e2c, objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (objs[n].state > 0) --objs[n].state;
        if (objs[n].state == 0 && board(xc, yc + 1) == 0x008a &&
            objs[n].counter != -1) {
            setboard(xc, yc + 1, board(xc, yc));
            moveobj(n, objs[n].x, objs[n].y + 16);
            return 1;
        }
        return 0;
    } else if (msg == msg_touch) {
        if (objs[z].objkind == obj_player) {
            objs[n].state = 6;
            if (first_elev) {
                first_elev = 0;
                putbotmsg("Press UP/DOWN to use elevator", 2);
            }
            if (dy1 < 0) {
                objs[z].yd = 0;
                if (objs[n].substate != dy1) snd_play(2, 29);
                if (justmove(0, objs[0].x, (yc - 2) * 16)) {
                    moveobj(n, objs[n].x, objs[n].y - 16);
                    setboard(xc, yc, 0x008a);
                }
            } else if (dy1 > 0) {
                objs[z].yd = 0;
                if (objs[n].substate != dy1) snd_play(2, 30);
                if (board(xc, yc + 1) == 0x008a) {
                    setboard(xc, yc + 1, board(xc, yc));
                    moveobj(n, objs[n].x, objs[n].y + 16);
                    justmove(0, objs[0].x, yc * 16);
                }
            }
            objs[n].substate = dy1;
        }
        return 1;
    }
    return 0;
}

int msg_firebullet(int n, int msg, int z)
{
    static const word y_sequence[20] = {
         3,  2,  1,  1,  0, -1, -1, -2, -2, -3,
        -3, -2, -2, -1, -1,  0,  1,  1,  2,  3
    };

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_firebullet] * 256 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        for (;;) {
            if (!onscreen(n)) {
                killobj(n);
                break;
            }
            objs[n].counter = (word)((objs[n].counter + 1) & 3);
            if (++objs[n].state >= 20) objs[n].state = 0;
            trybreakwall(n, objs[n].x + objs[n].xd,
                         objs[n].y + objs[n].yd);
            justmove(n, objs[n].x,
                     objs[n].y + objs[n].yd + y_sequence[objs[n].state]);
            if (!justmove(n, objs[n].x + objs[n].xd, objs[n].y))
                continue;
            if (++objs[n].substate >= 80) continue;
            break;
        }
        return 1;
    } else if (msg == msg_touch) {
        if (kindflags[(byte)objs[z].objkind] & f_front) {
            killobj(n);
            playerkill(z);
            snd_play(3, 10);
        }
        return 1;
    }
    return 0;
}
int msg_fishbullet(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_fishbullet] * 256 +
                  (objs[n].xd > 0 ? 1 : 0) + 21,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (!onscreen(n)) {
            killobj(n);
            return 0;
        }
        if (!justmove(n, objs[n].x + objs[n].xd, objs[n].y))
            killobj(n);
        return 1;
    }
    return 0;
}

int msg_searock(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_searock] * 256 + 34,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (rand() % 12 == 0) {
            addobj(obj_bubble, objs[n].x + 2, objs[n].y + 4);
            return 1;
        }
        return 0;
    }
    return 0;
}

int msg_eyes(int n, int msg, int z)
{
    int x, y;
    int shape = kindtable[obj_eye] * 256;
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
        drawshape(gamevp, shape + 1,
                  objs[n].x + objs[n].xd + 5,
                  objs[n].y + objs[n].yd + 4);
    } else if (msg == msg_update) {
        pointvect(0, n, &x, &y, 3);
        if (y > 1) y = 1;
        else if (y != 1) y = -1;
        if (x < -2) x = -2;
        if (objs[n].xd != x || objs[n].yd != y) {
            objs[n].xd = (word)x;
            objs[n].yd = (word)y;
            return 1;
        }
        return 0;
    }
    return 0;
}

int msg_vineclimb(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_vineclimb] * 256 + objs[n].counter / 2,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (!(stateinfo[objs[0].state] & sti_invincible)) {
                if (!justmove(0, objs[0].x - 8, objs[0].y))
                    justmove(0, objs[0].x + 8, objs[0].y);
                objs[0].state = 0;
                objs[0].substate = 0;
            }
            hitplayer(n);
        }
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        if (objs[n].yd == 0) objs[n].yd = 2;
        if (!objdo(n, objs[n].x, objs[n].y + objs[n].yd, f_notvine))
            moveobj(n, objs[n].x, objs[n].y + objs[n].yd);
        else
            objs[n].yd = (word)-objs[n].yd;
        return 1;
    }
    return 0;
}

int msg_flag(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_flag] * 256 + objs[n].counter / 2 + 1,
                  objs[n].x + 2, objs[n].y);
    } else if (msg == msg_update) {
        if (++objs[n].counter >= 6) objs[n].counter = 0;
        return (objs[n].counter & 1) == 0;
    } else if (msg == msg_touch) {
        if (z == 0 && objs[n].state == 0) {
            objs[n].state = 1;
            addscore(6, objs[n].x, objs[n].y);
            snd_play(2, 32);
            explode1(objs[n].x, objs[n].y, 5);
        }
        return 1;
    }
    return 0;
}

int msg_macrotrig(int n, int msg, int z)
{
    (void)n;
    (void)z;
    return msg == msg_touch ? 1 : 0;
}

int msg_txtmsg(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_update) {
        if (objs[n].state > 0) --objs[n].state;
    } else if (msg == msg_touch) {
        if (objs[n].state == 0) {
            putbotmsg(objs[n].inside, 7);
            snd_play(3, 20);
        }
        objs[n].state = 8;
    }
    return 0;
}

int msg_mapdemo(int n, int msg, int z)
{
    static const word shape_offset[2] = { 16, 20 };
    static const word shape_length[2] = { 4, 3 };
    int c;
    (void)z;
    if (msg == msg_draw) {
        if (x_ourmode != 0) {
            for (c = 0; c < shape_length[objs[n].xd]; ++c)
                drawshape(gamevp,
                          0x4000 + kindtable[obj_mapdemo] * 256 +
                          shape_offset[objs[n].xd] + c,
                          objs[n].x + c * 16, objs[n].y);
        }
    } else if (msg == msg_update) {
        if (!designflag) {
            objs[n].x = (word)(gamevp->vpox + scrollxd + 16);
            objs[n].y = (word)(gamevp->vpoy + scrollyd + 4);
        }
        return 1;
    }
    return 0;
}
