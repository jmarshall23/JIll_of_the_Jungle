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
#include "WINDOWS.H"

#include <stdlib.h>
#include <string.h>

object_message_fn kindmsg[numobjkinds];
word kindxl[numobjkinds], kindyl[numobjkinds];
const char *kindname[numobjkinds];
uword kindflags[numobjkinds];
word kindtable[numobjkinds], kindscore[numobjkinds];

/* Initialized data at 338B8 in JILL.EXE. */
static word first_apple = 1;
static word first_nokey = 1;
static word first_opendoor = 1;
static word first_knife = 1;
static word first_key = 1;
static word first_openmapdoor = 1;
static word first_nogem = 1;

int msg_null(int n, int msg, int z) { (void)n; (void)msg; (void)z; return 0; }
int msg_nullkind(int n, int msg, int z) { return msg_null(n, msg, z); }

int msg_apple(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_apple] * 256 + objs[n].counter / 2,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter +
                                 (objs[n].xd > 0 ? 1 : -1)) & 7);
        return (objs[n].counter & 1) == 0;
    } else if (msg == msg_touch) {
        if (objs[n].state > 0) {
            if (objs[n].zaphold == 0) {
                snd_play(6, 25);
                dotextmsg(objs[n].state);
            }
            objs[n].zaphold = 4;
            killobj(n);
        } else {
            if (z != 0) return 0;
            if (pl.health < 8) ++pl.health;
            addscore(kindscore[obj_apple], objs[n].x, objs[n].y);
            statmodflg |= mod_screen;
            snd_play(2, 11);
            killobj(n);
            if (first_apple) {
                putbotmsg("APPLES GIVE YOU HEALTH", 2);
                first_apple = 0;
            }
        }
        return 1;
    }
    return 0;
}

int msg_knife(int n, int msg, int z)
{
    int dx, dy;
    if (msg == msg_draw) {
        drawshape(gamevp, kindtable[obj_knife] * 256,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (objs[n].statecount > 0) {
            ++objs[n].statecount;
            if (objs[n].statecount > 14) {
                seekplayer(n, &dx, &dy);
                objs[n].xd = (word)(objs[n].xd + dx);
                if (objs[n].xd > 8) objs[n].xd = 8;
                else if (objs[n].xd < -8) objs[n].xd = -8;
                objs[n].yd = (word)(objs[n].yd + dy);
                if (objs[n].yd > 4) objs[n].yd = 4;
                else if (objs[n].yd < -4) objs[n].yd = -4;
            }
            if (!trymove(n, objs[n].x + objs[n].xd,
                         objs[n].y + objs[n].yd) ||
                objs[n].statecount > 64)
                objs[n].statecount = -1;
        } else if (objs[n].statecount == -1) {
            if (!trymove(n, objs[n].x, objs[n].y + 1))
                objs[n].statecount = 0;
        } else {
            return 0;
        }
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (objs[n].statecount > 0 && objs[n].statecount <= 10)
                return 1;
            if (invcount(inv_knife) >= 3) return 1;
            addinv(inv_knife);
            snd_play(2, 7);
            killobj(n);
            if (first_knife) {
                putbotmsg("YOU FOUND A KNIFE!", 2);
                first_knife = 0;
            }
        } else if ((kindflags[(byte)objs[z].objkind] & f_front) &&
                   objs[n].statecount > 0) {
            objs[n].xd = 0;
            objs[n].yd = 0;
            objs[n].statecount = 15;
            playerkill(z);
            snd_play(3, 10);
        }
        return 1;
    }
    return 0;
}
int msg_bigant(int n, int msg, int z)
{
    int shape = kindtable[obj_bigant] * 256;
    if (msg == msg_draw) {
        if (objs[n].state == 0)
            shape += (objs[n].xd > 0 ? 10 : 0) + objs[n].counter;
        else if (objs[n].xd < 0)
            shape += objs[n].state + 4;
        else
            shape += 10 - objs[n].state;
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) {
            if (++objs[n].counter > 4) objs[n].counter = 0;
            if (!crawl(n, objs[n].xd, 0)) {
                objs[n].state = 5;
                objs[n].xd = (word)-objs[n].xd;
            }
        } else {
            --objs[n].state;
        }
        return 1;
    }
    return 0;
}

int msg_ant(int n, int msg, int z)
{
    int shape = kindtable[obj_ant] * 256;
    if (msg == msg_draw) {
        if (objs[n].state == 0)
            shape += (objs[n].xd < 0 ? 5 : 0) + objs[n].counter;
        else if (objs[n].xd < 0)
            shape += 14 - objs[n].state * 5;
        else
            shape += objs[n].state * 5 - 1;
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) {
            if (++objs[n].counter > 3) objs[n].counter = 0;
            if (!crawl(n, objs[n].xd, 0)) {
                objs[n].state = 2;
                objs[n].xd = (word)-objs[n].xd;
            }
        } else {
            --objs[n].state;
        }
        return 1;
    }
    return 0;
}
int msg_fly(int n, int msg, int z)
{
    static const word draw_sequence[4] = { 0, 1, 2, 1 };
    static const word y_sequence[8] = { 1, 2, 1, 0, -1, -2, -1, 0 };
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_fly] * 256 +
                  draw_sequence[objs[n].counter & 3] +
                  (objs[n].xd > 0 ? 4 : 0),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            explode1(objs[n].x, objs[n].y, 4);
            p_ouch(1, die_ash);
            killobj(n);
        }
    } else if (msg == msg_update) {
        if (!justmove(n, objs[n].x + objs[n].xd,
                      objs[n].y + y_sequence[objs[n].counter])) {
            objs[n].xd = (word)-objs[n].xd;
            snd_play(1, 15);
        }
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        return 1;
    }
    return 0;
}

int msg_phoenix(int n, int msg, int z)
{
    static const word draw_sequence[8] = { 0, 1, 2, 3, 2, 1, 9, 9 };
    static const word reverse_sequence[8] = { 4, 4, 4, 4, 4, 4, -1, -1 };
    int new_x, old_y;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_phoenix] * 256 +
                  draw_sequence[objs[n].counter] +
                  (objs[n].xd < 0 ? reverse_sequence[objs[n].counter] : 0),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            explode1(objs[n].x, objs[n].y, 4);
            p_ouch(1, die_ash);
            explode2(0);
            killobj(n);
        }
    } else if (msg == msg_update) {
        if (!cando(n, objs[n].x, objs[n].y, f_playerthru)) return 0;
        new_x = objs[n].x + objs[n].xd;
        old_y = objs[n].y;
        ++objs[n].counter;
        if (objs[n].counter == 6 || objs[n].counter > 7)
            objs[n].counter = 0;
        if (justmove(n, new_x, old_y)) {
            if (!cando(n, objs[n].x + objs[n].xd, old_y, f_playerthru))
                objs[n].counter = 6;
        } else {
            objs[n].xd = (word)-objs[n].xd;
            snd_play(1, 15);
        }
        return 1;
    }
    return 0;
}
int msg_inchworm(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_inchworm] * 256 +
                  (objs[n].xd < 0 ? 3 : 0) + objs[n].statecount,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 3);
        if (objs[n].counter == 0) {
            objs[n].statecount ^= 1;
            if (!crawl(n, objs[n].xd, 0))
                objs[n].xd = (word)-objs[n].xd;
            return 1;
        }
        return 0;
    }
    return 0;
}
int msg_zapper(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  0x4000 + kindtable[obj_zapper] * 256 + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        p_ouch(16, die_fish);
    } else if (msg == msg_update) {
        if (++objs[n].counter > 4) objs[n].counter = 0;
        return 1;
    }
    return 0;
}
int msg_bobslug(int n, int msg, int z)
{
    int shape = kindtable[obj_bobslug] * 256;
    if (msg == msg_draw) {
        shape += objs[n].xd > 0 ? objs[n].state : 6 - objs[n].state;
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) {
            if (!crawl(n, objs[n].xd, 0)) objs[n].state = 1;
        } else {
            objs[n].statecount ^= 1;
            if (objs[n].statecount & 1) return 0;
            if (++objs[n].state >= 6) {
                objs[n].state = 0;
                objs[n].xd = (word)-objs[n].xd;
            }
        }
        return 1;
    }
    return 0;
}

int msg_checkpt(int n, int msg, int z)
{
    char string[16];
    objtype *checkpoint = &objs[n];
    if (msg == msg_update) {
        if (objs[0].x < checkpoint->x + checkpoint->xl &&
            checkpoint->x < objs[0].x + objs[0].xl &&
            objs[0].y < checkpoint->y + checkpoint->yl &&
            checkpoint->y < objs[0].y + objs[0].yl) {
            checkpoint->xd = objs[0].x;
            checkpoint->yd = objs[0].y;
        }
        return 0;
    }
    if (msg == msg_draw) {
        if (designflag) {
            fontcolor(gamevp, 5, -1);
            _itoa(checkpoint->counter, string, 10);
            wprint(gamevp, checkpoint->x + 4, checkpoint->y + 4,
                   1, string);
        }
        return 0;
    }
    if (msg == msg_touch && z == 0) {
        if (checkpoint->state == 3) {
            macrecend();
            gameover = 2;
            return 0;
        }
        if (pl.level == checkpoint->counter) return 0;
        snd_play(4, macplay ? 5 : checkpoint->counter + 50);
        if (checkpoint->counter != 0) pl.level = checkpoint->counter;
        if (objs[0].objkind == obj_tiny) moveobj(0, checkpoint->xd, checkpoint->yd);
        else {
            objs[0].x = checkpoint->x;
            objs[0].y = (word)(checkpoint->y - 16);
            objs[0].state = st_begin;
            objs[0].statecount = 0;
        }
        if (checkpoint->inside != NULL) strcpy(newlevel, checkpoint->inside);
        return 1;
    }
    return 0;
}

int msg_paul(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw)
        drawshape(gamevp,
                  kindtable[obj_paul] * 256 + objs[n].state,
                  objs[n].x, objs[n].y);
    else if (msg == msg_touch || msg == msg_update)
        return 0;
    return 0;
}
int msg_wiseman(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_wiseman] * 256 +
                  (objs[n].xd > 0 ? 5 : 0) + objs[n].state,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (objs[n].counter == 0) {
            objs[n].state = (word)((objs[n].state + 1) & 3);
            objs[n].counter = 1;
            if (!crawl(n, objs[n].xd, 0))
                objs[n].xd = (word)-objs[n].xd;
            return 1;
        }
        --objs[n].counter;
        if (z == 0) {
            if (objs[n].zaphold == 0) putbotmsg(objs[z].inside, 7);
            objs[z].zaphold = 3;
        }
    } else if (msg == msg_touch && z == 0) {
        if (objs[n].zaphold == 0) putbotmsg(objs[z].inside, 7);
        objs[z].zaphold = 3;
    }
    return 0;
}

int msg_bridger(int n, int msg, int z)
{
    int c, d;
    int xc = objs[n].x / 16;
    int yc = objs[n].y / 16;
    int oldcell = 0;
    int newcell = 0;
    int dircell = objs[n].xd == 0 ? 0x008c : 0x01b0;
    int dochange = 0;

    if (msg == msg_draw) {
        if (designflag)
            drawshape(gamevp, 0x0123, objs[n].x + 4, objs[n].y + 4);
    } else if (msg == msg_trigon) {
        dochange = 1;
        oldcell = board(xc, yc);
        newcell = dircell;
    } else if (msg == msg_trigoff) {
        dochange = 1;
        oldcell = dircell;
        newcell = 0;
    } else if (msg == msg_trigger) {
        if (board(xc, yc) == dircell) {
            newcell = 0;
            oldcell = dircell;
        } else {
            newcell = dircell;
            oldcell = board(xc, yc);
        }
        dochange = 1;
        if (objs[z].objkind == obj_pad) killobj(z);
    }

    if (dochange) {
        if (newcell == 0) {
            for (c = -1; c <= 1; c += 2) {
                d = board(xc + c * objs[n].yd,
                          yc + c * objs[n].xd);
                if ((info[d].flags & (f_playerthru | f_notstair)) ==
                    (f_playerthru | f_notstair))
                    newcell = d;
            }
        }
        while (board(xc, yc) == oldcell) {
            setboard(xc, yc, newcell);
            xc += objs[n].xd;
            yc += objs[n].yd;
        }
        return 1;
    }
    return 0;
}

int msg_key(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e06 + objs[n].counter / 2,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        return (objs[n].counter & 1) == 0;
    } else if (msg == msg_touch) {
        if (z != 0) return 0;
        addinv(inv_redkey);
        snd_play(3, 6);
        killobj(n);
        if (first_key) {
            putbotmsg("YOU FOUND A KEY!", 2);
            first_key = 0;
        }
        return 1;
    }
    return 0;
}

int msg_pad(int n, int msg, int z)
{
    int ourmsg;
    if (msg == msg_draw) {
        if (designflag)
            drawshape(gamevp, 0x140, objs[n].x + 4, objs[n].y + 4);
    } else if (msg == msg_update) {
        return 0;
    } else if (msg == msg_touch) {
        if (z == 0) {
            if (objs[n].state == -1) ourmsg = msg_trigoff;
            else if (objs[n].state == 1) ourmsg = msg_trigon;
            else ourmsg = msg_trigger;
            sendtrig(objs[n].counter, ourmsg, n);
        }
        return 1;
    }
    return 0;
}

int msg_demon(int n, int msg, int z)
{
    static const word draw_sequence[6] = { 0, 1, 2, 3, 2, 1 };

    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_demon] * 256 +
                  draw_sequence[objs[n].counter / 2] +
                  (objs[n].xd < 0 ? 4 : 0),
                  objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        int shot;
        int dx;
        int dy;

        if (++objs[n].counter >= 12)
            objs[n].counter = 0;

        if ((!justmove(n, objs[n].x + objs[n].xd,
                       objs[n].y + objs[n].yd) || rand() % 36 == 0)
#if defined(JILL_EP1)
            && objs[n].info1 == 0
#endif
        ) {
            pointvect(0, n, &dx, &dy, 4);
            objs[n].xd = (word)dx;
            objs[n].yd = (word)dy;
            shot = addobj(obj_fireball, objs[n].x, objs[n].y);
            objs[shot].xd = objs[n].xd;
            objs[shot].yd = objs[n].yd;
            if (rand() % 2 == 0) {
                objs[n].xd = (word)(rand() % 5 - 2);
                objs[n].yd = (word)(rand() % 5 - 2);
            } else {
                objs[n].xd /= 2;
                objs[n].yd /= 2;
            }
        }
        objs[n].state -= objs[n].state > 0;
        return 1;
    } else if (msg == msg_touch) {
        if (z == 0
#if defined(JILL_EP1)
            && objs[n].info1 == 0
#endif
        ) {
            hitplayer(n);
#if defined(JILL_EP1)
        } else if (objs[z].objkind == obj_spinblad) {
#elif defined(JILL_EP2) || defined(JILL_EP3)
        } else if (kindflags[(byte)objs[z].objkind] & f_weapon) {
#endif
            if (objs[n].state == 0) {
                if (++objs[n].statecount > 5) {
                    explode2(n);
                    killobj(n);
                } else {
                    explode1(objs[n].x, objs[n].y, 4);
                }
            }
            objs[n].state = 4;
        }
        return 1;
    }
    return 0;
}
int msg_fatso(int n, int msg, int z)
{
    static const word draw_sequence[4] = { 0, 1, 2, 1 };
    int dx, dy;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_fatso] * 256 +
                  (objs[n].xd > 0 ? 3 : 0) +
                  draw_sequence[objs[n].counter / 4],
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) hitplayer(n);
    } else if (msg == msg_update) {
        if (++objs[n].counter >= 16) objs[n].counter = 0;
        if (objs[n].counter & 1) return 0;
        if (rand() % 30 == 0) {
            seekplayer(n, &dx, &dy);
            objs[n].xd = (word)dx;
            objs[n].yd = 0;
            objs[n].xd = (word)(objs[n].xd * 4);
        }
        if (!crawl(n, objs[n].xd, 0))
            objs[n].xd = (word)-objs[n].xd;
        else
            snd_play(1, 17);
        return 1;
    }
    return 0;
}

int msg_roman(int n, int msg, int z)
{
    int dx, dy;
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_roman] * 256 +
                  (objs[n].xd > 0 ? 8 : 0) + objs[n].counter,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0
#if defined(JILL_EP1)
            && objs[n].info1 == 0
#endif
        ) hitplayer(n);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 7);
        if (rand() % 30 == 0) {
            seekplayer(n, &dx, &dy);
            objs[n].yd = 0;
            objs[n].xd = (word)(abs(objs[n].xd) * dx);
        }
        if (!crawl(n, objs[n].xd, 0))
            objs[n].xd = (word)-objs[n].xd;
        else
            snd_play(1, 17);
        return 1;
    }
    return 0;
}
int msg_fireball(int n, int msg, int z)
{
    if (msg == msg_draw) {
        drawshape(gamevp,
                  kindtable[obj_fireball] * 256 + objs[n].counter / 2,
                  objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0) {
            hitplayer(n);
            explode2(0);
        }
    } else if (msg == msg_update) {
        if (!onscreen(n)) {
            killobj(n);
        } else {
            if (++objs[n].counter >= 8) objs[n].counter = 0;
            if (!justmove(n, objs[n].x + objs[n].xd,
                          objs[n].y + objs[n].yd))
                killobj(n);
        }
        return 1;
    }
    return 0;
}

int msg_cloud(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e0a, objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        objs[n].counter = (word)((objs[n].counter + 1) & 15);
        if (objs[n].counter & 1) {
            if (!justmove(n, objs[n].x + objs[n].xd, objs[n].y))
                objs[n].xd = (word)-objs[n].xd;
            return 1;
        }
        return 0;
    }
    return 0;
}

int msg_text6(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        if (x_ourmode == 0) fontcolor(gamevp, objs[n].xd, 0);
        else fontcolor(gamevp, objs[n].xd, objs[n].yd);
        wprint(gamevp, objs[n].x, objs[n].y + 1, 2, objs[n].inside);
    } else if (msg == msg_update) {
        if (objs[n].counter > 0 && --objs[n].counter <= 0) {
            killobj(n);
            return 1;
        }
        return 0;
    }
    return 0;
}

int msg_text8(int n, int msg, int z)
{
    (void)z;
    if (msg == msg_draw) {
        if (x_ourmode == 0) fontcolor(gamevp, objs[n].xd, 0);
        else fontcolor(gamevp, objs[n].xd, objs[n].yd);
        wprint(gamevp, objs[n].x, objs[n].y, 1, objs[n].inside);
    } else if (msg == msg_update) {
        if (objs[n].counter > 0 && --objs[n].counter <= 0) {
            killobj(n);
            return 1;
        }
        return 0;
    }
    return 0;
}

int msg_score(int n, int msg, int z)
{
    char string[10];
    int c;
    (void)z;
    if (msg == msg_draw) {
        fontcolor(gamevp, 1 + (objs[n].counter & 3), -1);
        _itoa(objs[n].state, string, 10);
        for (c = 0; string[c] != 0; ++c)
            drawshape(gamevp, 0x3d0 + string[c],
                      objs[n].x + c * 4, objs[n].y);
    } else if (msg == msg_update) {
        if (--objs[n].counter < 0 || !onscreen(n)) {
            killobj(n);
            return 0;
        }
        objs[n].x = (word)(objs[n].x + objs[n].xd);
        objs[n].y = (word)(objs[n].y + objs[n].yd);
        --objs[n].yd;
        return 1;
    }
    return 0;
}

int msg_frog(int n, int msg, int z)
{
    int shape;
    int x;
    int y;
    int result = 0;

    if (msg == msg_draw) {
        shape = kindtable[(byte)objs[n].objkind] * 256 +
                (objs[n].xd < 0 ? 3 : 0);
        if (objs[n].state == 1)
            shape += (objs[n].yd > 0 ? 2 : 1);
        drawshape(gamevp, shape, objs[n].x, objs[n].y);
    } else if (msg == msg_touch) {
        if (z == 0)
            hitplayer(n);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) {
            if (++objs[n].counter <= 16)
                return 0;
            objs[n].state = 1;
            seekplayer(n, &x, &y);
            objs[n].xd = (word)(x * 4);
            objs[n].yd = -10;
            result = 1;
        } else {
            result = 1;
            if (++objs[n].yd > 12)
                objs[n].yd = 10;
            x = objs[n].x + objs[n].xd;
            y = objs[n].y + objs[n].yd;
            if (trymove(n, x, y) & 3)
                return 1;
            if (objs[n].yd < 0) {
                objs[n].yd = 0;
                return 1;
            }
            y = (y & 0xfff0) + 16 - objs[n].yl;
            trymove(n, x, y);
            seekplayer(n, &x, &y);
            objs[n].xd = (word)x;
            objs[n].state = 0;
            objs[n].counter = 0;
            snd_play(1, 18);
        }
    }
    return result;
}

int msg_door(int n, int msg, int z)
{
    int c;
    int xc = objs[n].x / 16;
    int yc = objs[n].y / 16;
    (void)z;

    if (msg == msg_draw) {
        if (designflag)
            drawshape(gamevp, 0x0e05, objs[n].x + 4, objs[n].y + 12);
        if (objs[n].state != 0) {
            drawcell(xc, yc);
            drawcell(xc, yc + 1);
            drawshape(gamevp, info[161].sh,
                      objs[n].x, objs[n].y - objs[n].state);
            drawshape(gamevp, info[162].sh,
                      objs[n].x, objs[n].y + objs[n].state + 16);
            drawcell(xc, yc - 1);
            drawcell(xc, yc + 2);
        }
    } else if (msg == msg_update) {
        if (objs[n].state == 0) return 0;
        if (++objs[n].state > 16) killobj(n);
        return 1;
    } else if (msg == msg_trigger) {
        if (objs[n].state != 0) return 0;
        if (board(xc, yc) == 0x00be) {
            if (takeinv(inv_crystal)) {
                snd_play(3, 36);
                if (first_openmapdoor) {
                    putbotmsg("THE GATE OPENS", 1);
                    first_openmapdoor = 0;
                }
                setboard(xc, yc,
                         board(xc + objs[n].xd, yc + objs[n].yd));
                killobj(n);
            } else if (first_nogem) {
                putbotmsg("YOU NEED A GEM TO PASS", 1);
                first_nogem = 0;
            }
        } else if (takeinv(inv_redkey)) {
            if (first_opendoor) {
                putbotmsg("THE DOOR OPENS", 1);
                first_opendoor = 0;
            }
            snd_play(3, 12);
            objs[n].state = 1;
            for (c = 0; c <= 1; ++c)
                setboard(xc, yc + c, board(xc - 1, yc + c));
        } else if (first_nokey) {
            putbotmsg("THE DOOR IS LOCKED", 2);
            first_nokey = 0;
        }
    }
    return 0;
}

int msg_falldoor(int n, int msg, int z)
{
    int xc, yc;
    (void)z;
    if (msg == msg_draw) {
        drawshape(gamevp, 0x0e10, objs[n].x, objs[n].y);
    } else if (msg == msg_update) {
        if (objs[n].state == 0) return 0;
        if (justmove(n, objs[n].x, objs[n].y + 4)) {
            if ((objs[n].y & 15) == 0) {
                xc = objs[n].x / 16;
                yc = objs[n].y / 16;
                setboard(xc, yc - 1, objs[n].counter);
            }
        } else {
            snd_play(3, 14);
            xc = objs[n].x / 16;
            yc = objs[n].y / 16;
            setboard(xc, yc, objs[n].counter);
            killobj(n);
        }
        return 1;
    } else if (msg == msg_trigger) {
        objs[n].state = 1;
        objs[n].counter = board(objs[n].x >> 4, (objs[n].y >> 4) - 1);
        snd_play(3, 13);
    }
    return 0;
}

void initobjinfo(void)
{
    /* JILL2.EXE's stripped initobjinfo assigns the same 69 dimensions,
       flags, shape tables, scores, and handler relationships as Episode 1. */
    static const char *const names[numobjkinds] = {
        "PLAYER","APPLE","KNIFE","KILLME","BIGANT","FLY","MACROTRIG","DEMON","BUNNY",
        "INCHWORM","ZAPPER","BOBSLUG","CHECKPT","PAUL","KEY","PAD","WISEMAN","ROMAN",
        "FIREBALL","CLOUD","TEXT6","TEXT8","FROG","TINY","DOOR","FALLDOOR","BRIDGER",
        "SCORE","TOKEN","ANT","PHOENIX","FIRE","SWITCH","GEM","EXPLODE","BOULDER",
        "EXPL1","EXPL2","STALAG","SNAKE","SEAROCK","BOLL","MEGA","BAT","KNIGHT",
        "BEENEST","BEESWARM","CRAB","CROC","EPIC","SPINBLAD","SKULL","BUTTON","PAC",
        "JILLFISH","UNUSED55","JILLBIRD","JILLFROG","BUBBLE","JELLYFISH","BADFISH","ELEV",
        "FIREBULLET","FISHBULLET","EYE","VINECLIMB","FLAG","MAPDEMO","ROMAN"
    };
    static const word recovered_xl[numobjkinds] = {
        16,12,10,0,24,16,0,32,8,16,32,24,16,24,16,16,16,20,16,16,6,8,14,
        4,16,16,0,4,16,32,16,16,16,16,16,16,16,16,16,56,16,14,20,26,32,16,
        16,16,64,32,16,22,16,16,24,16,16,14,8,16,28,16,16,12,16,16,36,64,16
    };
    static const word recovered_yl[numobjkinds] = {
        32,12,10,0,16,14,0,32,8,8,16,24,16,32,8,16,24,28,16,16,7,8,10,10,
        24,16,0,5,16,16,16,32,16,16,16,16,32,32,16,16,16,14,24,32,32,16,
        16,16,8,16,16,26,16,16,16,16,16,10,8,24,16,16,16,5,12,8,16,16,32
    };
    static const uword recovered_flags[numobjkinds] = {
        8,0,0x4008,0,0x480,0x80,8,0,0x480,0x480,0,0x400,0x40,0,0,0,0x40,
        0x480,0,0,0x40,0x40,0x480,8,0x100,0x100,0x100,0,8,0x480,0x480,0,8,
        0,8,0,0,0,0x100,0x400,0,0,0x100,0x480,0x100,0,0,0x480,0x480,0,
        0x4008,0x100,8,0x400,8,8,8,8,0,0,0,0x100,0x4008,0x4008,0x180,0,8,0,0x480
    };
    static const word recovered_table[numobjkinds] = {
        8,9,13,0,59,60,0,43,58,22,28,52,0,57,14,0,11,44,26,10,0,0,63,16,
        0,14,0,0,0,10,11,12,60,9,14,0,46,14,0,15,14,31,33,35,36,37,37,38,
        39,40,45,47,49,50,51,0,11,63,51,51,51,0,26,51,62,61,5,3,44
    };
    static const word recovered_score[numobjkinds] = {
        0,12,0,0,8,3,0,20,2,3,0,5,0,0,0,0,0,12,0,0,0,0,15,0,0,0,0,0,0,
        6,4,0,0,23,0,0,0,0,0,35,0,0,0,4,0,11,0,2,3,35,0,0,0,0,0,0,0,0,
        0,0,7,0,0,0,3,0,0,0,12
    };
    int kind;
    for (kind = 0; kind < numobjkinds; ++kind) {
        kindmsg[kind] = msg_nullkind;
        kindxl[kind] = recovered_xl[kind]; kindyl[kind] = recovered_yl[kind];
        kindname[kind] = names[kind];
        kindflags[kind] = recovered_flags[kind];
        kindtable[kind] = recovered_table[kind];
        kindscore[kind] = recovered_score[kind];
    }

    kindmsg[obj_player] = msg_player;
    kindmsg[obj_apple] = msg_apple;
    kindmsg[obj_knife] = msg_knife;
    kindmsg[obj_killme] = msg_null;
    kindmsg[obj_bigant] = msg_bigant;
    kindmsg[obj_fly] = msg_fly;
    kindmsg[obj_macrotrig] = msg_macrotrig;
    kindmsg[obj_demon] = msg_demon;
    kindmsg[obj_bunny] = msg_frog;
    kindmsg[obj_fatso] = msg_fatso;
    kindmsg[obj_inchworm] = msg_inchworm;
    kindmsg[obj_zapper] = msg_zapper;
    kindmsg[obj_bobslug] = msg_bobslug;
    kindmsg[obj_checkpt] = msg_checkpt;
    kindmsg[obj_paul] = msg_paul; kindmsg[obj_key] = msg_key;
    kindmsg[obj_pad] = msg_pad;
    kindmsg[obj_wiseman] = msg_wiseman; kindmsg[obj_roman] = msg_roman;
    kindmsg[obj_fireball] = msg_fireball;
    kindmsg[obj_cloud] = msg_cloud;
    kindmsg[obj_text6] = msg_text6;
    kindmsg[obj_text8] = msg_text8;
    kindmsg[obj_frog] = msg_frog;
    kindmsg[obj_tiny] = msg_tiny;
    kindmsg[obj_door] = msg_door;
    kindmsg[obj_falldoor] = msg_falldoor;
    kindmsg[obj_bridger] = msg_bridger;
    kindmsg[obj_score] = msg_score;
    kindmsg[obj_token] = msg_token; kindmsg[obj_ant] = msg_ant; kindmsg[obj_phoenix] = msg_phoenix;
    kindmsg[obj_fire] = msg_fire; kindmsg[obj_switch] = msg_switch; kindmsg[obj_gem] = msg_gem;
    kindmsg[obj_txtmsg] = msg_txtmsg; kindmsg[obj_boulder] = msg_boulder;
    kindmsg[obj_expl1] = msg_expl1; kindmsg[obj_expl2] = msg_expl2;
    kindmsg[obj_stalag] = msg_stalag; kindmsg[obj_snake] = msg_snake;
    kindmsg[obj_searock] = msg_searock; kindmsg[obj_boll] = msg_boll; kindmsg[obj_mega] = msg_mega;
    kindmsg[obj_bat] = msg_bat; kindmsg[obj_knight] = msg_knight; kindmsg[obj_beenest] = msg_beenest;
    kindmsg[obj_beeswarm] = msg_beeswarm; kindmsg[obj_crab] = msg_crab; kindmsg[obj_croc] = msg_croc;
    kindmsg[obj_epic] = msg_epic; kindmsg[obj_spinblad] = msg_spinblad; kindmsg[obj_skull] = msg_skull;
    kindmsg[obj_button] = msg_button; kindmsg[obj_pac] = msg_pac;
    kindmsg[obj_jillfish] = msg_jillfish; kindmsg[obj_jillspider] = msg_jillspider;
    kindmsg[obj_jillbird] = msg_jillbird; kindmsg[obj_jillfrog] = msg_jillfrog;
    kindmsg[obj_bubble] = msg_bubble; kindmsg[obj_jellyfish] = msg_jellyfish; kindmsg[obj_badfish] = msg_badfish;
    kindmsg[obj_elev] = msg_elev; kindmsg[obj_firebullet] = msg_firebullet; kindmsg[obj_fishbullet] = msg_fishbullet;
    kindmsg[obj_eye] = msg_eyes; kindmsg[obj_vineclimb] = msg_vineclimb; kindmsg[obj_flag] = msg_flag;
    kindmsg[obj_mapdemo] = msg_mapdemo;

}
