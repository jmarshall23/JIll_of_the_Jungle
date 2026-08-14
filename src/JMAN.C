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

#include <stdlib.h>
#include <string.h>

#define BREAKWALL_TILE 0x00a7

objtype objs[maxobjs + 2];
word numobjs, numscrnobjs;
word scrnobjs[maxscrnobjs + 1];
pltype pl;
word scrollxd, scrollyd, oldscrollxd, oldscrollyd, oldx0, oldy0;
word gameover, gamecount, statmodflg, designflag, peeky;

void setboard(int x, int y, int value)
{
    if (x >= 0 && x < boardxs && y >= 0 && y < boardys)
        bd[x][y] = (uword)value;
}

void modboard(int x, int y)
{
    if (x >= 0 && x < boardxs && y >= 0 && y < boardys)
        bd[x][y] |= mod_screen;
}

void initobjs(void)
{
    numobjs = 1;
    objs[0].objkind = obj_player;
    objs[0].x = 32;
    objs[0].y = 32;
    objs[0].xd = 0;
    objs[0].yd = 0;
    objs[0].state = st_stand;
    objs[0].substate = 0;
    objs[0].statecount = 0;
    objs[0].counter = 0;
    objs[0].xl = kindxl[obj_player];
    objs[0].yl = kindyl[obj_player];
    objs[0].inside = NULL;
    objs[0].info1 = 0;
    objs[0].zaphold = 0;
    pl.numinv = 0;
    pl.level = 1;
    initinv();
    memset(pl.pad, 0, sizeof(pl.pad));
}

void playerkill(int n)
{
    addscore(kindscore[(byte)objs[n].objkind], objs[n].x, objs[n].y);
    notemod(n);
    killobj(n);
}

int countobj(int objkind)
{
    int count = 0, n;
    for (n = 0; n < numobjs; ++n) count += objs[n].objkind == objkind;
    return count;
}

void notemod(int n)
{
    int x, y, startx, starty, endx, endy;
    startx = objs[n].x / 16; starty = objs[n].y / 16;
    endx = (objs[n].x + objs[n].xl + 15) / 16;
    endy = (objs[n].y + objs[n].yl + 15) / 16;
    for (y = starty; y < endy; ++y)
        for (x = startx; x < endx; ++x) modboard(x, y);
}

void setobjsize(int n)
{
    int kind, string_length = 0;
    char number[8];
    kind = (byte)objs[n].objkind;
    objs[n].xl = kindxl[kind]; objs[n].yl = kindyl[kind];
    if (objs[n].inside != NULL) string_length = (int)strlen(objs[n].inside);
    if (kind == obj_text6) objs[n].xl = (word)(string_length * 6);
    else if (kind == obj_text8) objs[n].xl = (word)(string_length * 8);
    else if (kind == obj_score) {
        _itoa(objs[n].state, number, 10);
        objs[n].xl = (word)((strlen(number) + 2) * kindxl[obj_score]);
    }
}

int findcheckpt(int level)
{
    int n;
    for (n = 0; n < numobjs; ++n)
        if (objs[n].objkind == obj_checkpt && objs[n].counter == level) return n;
    return 0;
}

void dolevelsong(void)
{
    int n = findcheckpt(pl.level);
    int c;
    int d;

    if (n > 0 && objs[n].inside != NULL &&
        (objs[n].inside[0] == '*' || objs[n].inside[0] == '#' ||
         objs[n].inside[0] == '&')) {
        strcpy(newlevel, objs[n].inside);
    } else {
        c = findcheckpt(0);
        d = objs[c].inside[0];
        if (d == '*' || d == '#' || d == '&')
            strcpy(newlevel, objs[c].inside);
    }
}

void p_reenter(int died)
{
    int n;
    int destination_x;
    int destination_y;
    int x;
    int y;
    word saved_level;
    ulongword saved_old_score;

    statmodflg |= mod_screen;
    n = findcheckpt(pl.level);
    destination_x = objs[n].x;
    destination_y = objs[n].y;
    if (objs[0].objkind != obj_tiny) destination_y -= 16;
    if (n > 0 && died && objs[n].state == 1) {
        saved_old_score = pl.oldscore;
        saved_level = pl.level;
        loadboard(curlevel);
        pl.level = saved_level;
        pl.score = saved_old_score;
        pl.health = 6;
        n = findcheckpt(pl.level);
    }
    pl.oldscore = pl.score;
    dolevelsong();
    objs[0].x = (word)(destination_x & ~7);
    objs[0].y = (word)destination_y;
    setorigin();
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y)
            setboard(x, y, bd[x][y] | mod_screen);
    objs[0].state = st_begin;
    objs[0].statecount = 0;
}

void p_ouch(int healthtake, int diemode)
{
    if (objs[0].objkind == obj_tiny) return;
    if (objs[0].objkind == obj_player &&
        (stateinfo[objs[0].state] & sti_invincible)) return;
    healthtake -= invcount(inv_invin);
    if (healthtake <= 0) return;
    statmodflg |= mod_screen;
    pl.health = (word)(pl.health - healthtake);
    pl.ouched = 1;
    if (pl.health > 0) {
        snd_play(4, 19);
        return;
    }
    pl.health = 0;
    objs[0].objkind = obj_player;
    objs[0].xl = 16; objs[0].yl = 32;
    objs[0].state = st_die;
    objs[0].statecount = 0;
    objs[0].substate = (word)diemode;
    if (diemode == die_bird)
        objs[0].y = (word)((objs[0].y - 1) & ~15);
    objs[0].yd = -12;
    snd_play(4, 39 + diemode);
    explode1(objs[0].x, objs[0].y, 10);
}

void seekplayer(int n, int *dx, int *dy)
{
    *dx = JILL_SIGN(objs[0].x - objs[n].x);
    *dy = JILL_SIGN(objs[0].y - objs[n].y);
}

void modjunglescroll(int xd, int yd, int modcode)
{
    int x, y, new_x, new_y, viewport_end_y, end_y, end_x, viewport_end_x;

    if (xd > 0) {
        viewport_end_x = (gamevp->vpox + gamevp->vpxl - xd) / 16;
        end_x = JILL_MIN((gamevp->vpox + gamevp->vpxl - 1) / 16,
                         boardxs - 1);
        for (x = viewport_end_x; x <= end_x; ++x) {
            for (y = 0; y < scrnys + 1; ++y) {
                new_y = JILL_MIN(gamevp->vpoy / 16 + y, boardys - 1);
                setboard(x, new_y, bd[x][new_y] | modcode);
                if (modcode == mod_virtual) drawcell(x, new_y);
            }
        }
    } else if (xd < 0) {
        new_x = gamevp->vpox / 16;
        for (y = 0; y < scrnys + 1; ++y) {
            new_y = gamevp->vpoy / 16 + y;
            setboard(new_x, new_y, bd[new_x][new_y] | modcode);
            if (modcode == mod_virtual) drawcell(new_x, new_y);
        }
    }

    if (yd > 0) {
        viewport_end_y = gamevp->vpoy + gamevp->vpyl;
        end_y = JILL_MIN((viewport_end_y - 1) / 16, boardys - 1);
        for (new_y = (viewport_end_y - yd) / 16;
             new_y <= end_y; ++new_y) {
            for (x = 0; x < scrnxs + 1; ++x) {
                new_x = JILL_MIN(gamevp->vpox / 16 + x, boardxs - 1);
                setboard(new_x, new_y, bd[new_x][new_y] | modcode);
                if (modcode == mod_virtual) drawcell(new_x, new_y);
            }
        }
    } else if (yd < 0) {
        for (new_y = gamevp->vpoy / 16;
             new_y <= (gamevp->vpoy - yd - 1) / 16; ++new_y) {
            for (x = 0; x < scrnxs + 1; ++x) {
                new_x = JILL_MIN(gamevp->vpox / 16 + x, boardxs - 1);
                setboard(new_x, new_y, bd[new_x][new_y] | modcode);
                if (modcode == mod_virtual) drawcell(new_x, new_y);
            }
        }
    }
}

void junglescroll(int xd, int yd)
{
    int x, y;
    int cut0, cut1, cut2, cut3;
    int start_x, start_y, start_x2, start_y2;
    int end_x, end_y, end_x2, end_y2;

    start_x = oldx0 / 16;
    start_y = oldy0 / 16;
    end_x = (oldx0 + objs[0].xl + 15) / 16;
    end_y = (oldy0 + objs[0].yl + 15) / 16;
    start_x2 = objs[0].x / 16;
    start_y2 = objs[0].y / 16;
    end_x2 = (objs[0].x + objs[0].xl + 15) / 16;
    end_y2 = (objs[0].y + objs[0].yl + 15) / 16;

    if (xd == 0) {
        cut0 = 0;
        cut1 = JILL_MIN(start_x, start_x2) * 16 - gamevp->vpox;
        cut2 = JILL_MAX(end_x, end_x2) * 16 - gamevp->vpox;
        cut3 = gamevp->vpxl;
        scroll(gamevp, cut0, 0, cut1, gamevp->vpyl, 0, -yd);
    } else if (yd == 0) {
        cut0 = 0;
        cut1 = JILL_MIN(start_y, start_y2) * 16 - gamevp->vpoy;
        cut2 = JILL_MAX(end_y, end_y2) * 16 - gamevp->vpoy;
        cut3 = gamevp->vpyl;
        scroll(gamevp, 0, cut0, gamevp->vpxl, cut1, -xd, 0);
    }

    for (x = start_x; x < end_x; ++x) {
        for (y = start_y; y < end_y; ++y) {
            drawcell(x, y);
            setboard(x, y, bd[x][y] & ~mod_screenonly);
        }
    }

    if (xd == 0) {
        scroll(gamevp, cut1, 0, cut2, gamevp->vpyl, 0, -yd);
        gamevp->vpoy = (word)(gamevp->vpoy + yd);
        (void)kindmsg[(byte)objs[0].objkind](0, msg_draw, 0);
        scroll(gamevp, cut2, 0, cut3, gamevp->vpyl, 0, -yd);
    } else if (yd == 0) {
        scroll(gamevp, 0, cut1, gamevp->vpxl, cut2, -xd, 0);
        gamevp->vpox = (word)(gamevp->vpox + xd);
        (void)kindmsg[(byte)objs[0].objkind](0, msg_draw, 0);
        scroll(gamevp, 0, cut2, gamevp->vpxl, cut3, -xd, 0);
    } else {
        scrollvp(gamevp, -xd, -yd);
        gamevp->vpox = (word)(gamevp->vpox + xd);
        gamevp->vpoy = (word)(gamevp->vpoy + yd);
        (void)kindmsg[(byte)objs[0].objkind](0, msg_draw, 0);
    }
    modjunglescroll(xd, yd, mod_virtual);
}

void refresh(int page_mode)
{
    byte update_table[boardxs][20];
    int x, y, c, n;
    int start_x, start_y, end_x, end_y;
    int scroll_x, scroll_y;

    if (page_mode) {
        if (statmodflg) {
            drawstats();
            statmodflg &= (pagedraw + 1) * mod_page0;
        }
        if (scrollxd + oldscrollxd != 0 || scrollyd + oldscrollyd != 0) {
            gamevp->vpox = (word)(gamevp->vpox - oldscrollxd);
            gamevp->vpoy = (word)(gamevp->vpoy - oldscrollyd);
            scroll_x = scrollxd + oldscrollxd;
            scroll_y = scrollyd + oldscrollyd;
            scrollvp(gamevp, -scroll_x, -scroll_y);
            gamevp->vpox = (word)(gamevp->vpox + scroll_x);
            gamevp->vpoy = (word)(gamevp->vpoy + scroll_y);
            modjunglescroll(scroll_x, scroll_y, mod_screen);
        }
        oldscrollxd = scrollxd;
        oldscrollyd = scrollyd;

        start_x = JILL_MIN(gamevp->vpox / 16 + scrnxs, boardxs - 1);
        start_y = JILL_MIN(gamevp->vpoy / 16 + scrnys - 1, boardys - 1);
        end_x = JILL_MAX(gamevp->vpox / 16 - 2, 0);
        end_y = JILL_MAX(gamevp->vpoy / 16 - 2, 0);
        for (x = start_x; x >= end_x; --x) {
            for (y = start_y; y >= end_y; --y) {
                if (bd[x][y] & mod_screen) {
                    drawcell(x, y);
                    setboard(x, y, bd[x][y] &
                             ~((pagedraw + 1) * mod_page0));
                }
            }
        }
        for (n = numobjs - 1; n >= 0; --n) {
            if (objs[n].objflags & mod_screen) {
                (void)kindmsg[(byte)objs[n].objkind](n, msg_draw, 0);
                objs[n].objflags &= (uword)~((pagedraw + 1) * mod_page0);
            }
        }
        pageflip();
    } else {
        if (statmodflg) {
            drawstats();
            statmodflg = 0;
        }
        for (c = 0; c < boardxs; ++c) update_table[c][0] = 255;
        if (scrollxd != 0 || scrollyd != 0)
            junglescroll(scrollxd, scrollyd);

        start_x = JILL_MIN(gamevp->vpox / 16 + scrnxs - 1, boardxs - 1);
        start_y = JILL_MIN(gamevp->vpoy / 16 + scrnys - 1, boardys - 1);
        end_x = JILL_MAX(gamevp->vpox / 16 - 2, 0);
        end_y = JILL_MAX(gamevp->vpoy / 16 - 2, 0);

        for (n = 0; n < numobjs; ++n) {
            if (objs[n].objflags & mod_screen) {
                x = objs[n].x / 16;
                if (x < end_x) x = end_x;
                c = 0;
                while (update_table[x][c] != 255) ++c;
                update_table[x][c] = (byte)n;
                update_table[x][c + 1] = 255;
                objs[n].objflags &= ~mod_screen;
            }
        }

        for (x = start_x; x >= end_x; --x) {
            for (y = start_y; y >= end_y; --y) {
                if (bd[x][y] & mod_screenonly) {
                    drawcell(x, y);
                    setboard(x, y, bd[x][y] & ~mod_screen);
                }
            }
            for (c = 0; update_table[x][c] != 255 && c < 20; ++c) {
                n = update_table[x][c];
                (void)kindmsg[(byte)objs[n].objkind](n, msg_draw, 0);
            }
        }
    }

    if (pl.ouched != 0) {
        pl.ouched = 0;
        statmodflg |= mod_screen;
    }
}

void updbkgnd(void)
{
    int x, y, tile;
    int start_x = gamevp->vpox / 16;
    int start_y = gamevp->vpoy / 16;
    int end_x = JILL_MIN(start_x + scrnxs, boardxs - 1);
    int end_y = JILL_MIN(start_y + scrnys, boardys - 1);

    for (x = start_x; x <= end_x; ++x) {
        for (y = start_y; y <= end_y; ++y) {
            tile = board(x, y);
            if (info[tile].flags & f_msgupdate)
                setboard(x, y, bd[x][y] |
                         ((msg_block(x, y, msg_update) != 0) * mod_screen));
        }
    }
}

void updobjs(int doflag)
{
    int count, count2;
    int n, n2;
    int flag, old_x, old_y, old_width, old_height;
    int x, y, start_x, end_x, start_y, end_y;

    numscrnobjs = 1;
    scrnobjs[0] = 0;
    start_x = gamevp->vpox - 96;
    end_x = gamevp->vpox + gamevp->vpxl + 96;
    start_y = gamevp->vpoy - 48;
    end_y = gamevp->vpoy + gamevp->vpyl + 48;

    for (n = 1; n < numobjs && numscrnobjs < maxscrnobjs; ++n) {
        if (objs[n].x + objs[n].xl >= start_x && objs[n].x <= end_x &&
            objs[n].y + objs[n].yl >= start_y && objs[n].y <= end_y) {
            scrnobjs[numscrnobjs++] = (word)n;
            objs[n].objflags &= (uword)~(pagedraw * mod_page0);
        }
    }

    scrollxd = 0;
    scrollyd = 0;
    oldx0 = objs[0].x;
    oldy0 = objs[0].y;

    for (count = 0; count < numscrnobjs; ++count) {
        n = scrnobjs[count];
        old_x = objs[n].x;
        old_y = objs[n].y;
        old_width = objs[n].xl;
        old_height = objs[n].yl;

        if (doflag) {
            if (objs[n].zaphold > 0) --objs[n].zaphold;
            if (kindmsg[(byte)objs[n].objkind](n, msg_update, 0))
                objs[n].objflags |= mod_screen;
        } else {
            objs[n].objflags |= mod_screen;
        }

        if (kindflags[(byte)objs[n].objkind] & f_msgtouch) {
            for (count2 = 0; count2 <= numscrnobjs; ++count2) {
                n2 = scrnobjs[count2];
                if (n2 != n &&
                    objs[n2].x < objs[n].x + objs[n].xl &&
                    objs[n].x < objs[n2].x + objs[n2].xl &&
                    objs[n2].y < objs[n].y + objs[n].yl &&
                    objs[n].y < objs[n2].y + objs[n2].yl) {
                    (void)kindmsg[(byte)objs[n].objkind](n, msg_touch, n2);
                    objs[n].objflags |= mod_screen;
                    (void)kindmsg[(byte)objs[n2].objkind](n2, msg_touch, n);
                    objs[n2].objflags |= mod_screen;
                }
            }
        }

        if (objs[n].objflags & mod_screen) {
            start_x = old_x / 16;
            start_y = old_y / 16;
            end_x = (old_x + old_width + 15) / 16;
            end_y = (old_y + old_height + 15) / 16;
            for (y = start_y; y < end_y; ++y)
                for (x = start_x; x < end_x; ++x)
                    setboard(x, y, bd[x][y] | mod_screen);
        }
    }

    for (count = 0; count < numscrnobjs; ++count) {
        n = scrnobjs[count];
        x = objs[n].x;
        y = objs[n].y;
        start_x = x / 16;
        start_y = y / 16;
        end_x = (x + objs[n].xl + 15) / 16;
        end_y = (y + objs[n].yl + 15) / 16;
        flag = 0;
        for (y = start_y; y < end_y; ++y)
            for (x = start_x; x < end_x; ++x)
                flag = flag || (bd[x][y] & mod_screen);
        if (flag) objs[n].objflags |= mod_screen;
    }
}

void killobj(int n)
{
    objs[n].objflags |= mod_screen;
    objs[n].objkind = obj_killme;
}

int addobj(int kind, int x, int y)
{
    objs[numobjs].objkind = (sbyte)kind;
    objs[numobjs].x = (word)x;
    objs[numobjs].y = (word)y;
    objs[numobjs].state = 0;
    objs[numobjs].xd = 0;
    objs[numobjs].yd = 0;
    objs[numobjs].xl = kindxl[kind];
    objs[numobjs].yl = kindyl[kind];
    objs[numobjs].substate = 0;
    objs[numobjs].statecount = 0;
    objs[numobjs].objflags = 0;
    objs[numobjs].inside = NULL;
    objs[numobjs].info1 = 0;
    objs[numobjs].zaphold = 0;
    objs[numobjs].counter = 0;
    if (numobjs < maxobjs) ++numobjs;
    return numobjs - 1;
}

void addinv(int invthing)
{
    if (pl.numinv >= maxinventory - 1) return;
    pl.inv[pl.numinv++] = (word)invthing;
    statmodflg |= mod_screen;
}

int takeinv(int invthing)
{
    int index, following;
    for (index = 0; index < pl.numinv; ++index) {
        if (pl.inv[index] == invthing) {
            for (following = index + 1; following < pl.numinv; ++following)
                pl.inv[following - 1] = pl.inv[following];
            --pl.numinv;
            statmodflg |= mod_screen;
            return 1;
        }
    }
    return 0;
}

int invcount(int invthing)
{
    int count = 0, index;
    for (index = 0; index < pl.numinv; ++index) count += pl.inv[index] == invthing;
    return count;
}

void initinv(void)
{
    pl.health = 6;
    while (takeinv(inv_jill)) { }
}

void moveobj(int n, int x, int y)
{
    if (y < 0) y = 0;
    else if (y > boardys * 16 - 16 - objs[n].yl)
        y = boardys * 16 - 16 - objs[n].yl;
    if (x < 0) x = 0;
    else if (x > boardxs * 16 - 16 - objs[n].xl)
        x = boardxs * 16 - 16 - objs[n].xl;
    objs[n].x = (word)x;
    objs[n].y = (word)y;
}

int standfloor(int n, int dx, int dy)
{
    int x, startx, endx, tile_y, newx, newy;
    newx = objs[n].x + dx; newy = objs[n].y + dy;
    if (((newy + objs[n].yl) & 15) != 0) return 0;
    tile_y = (newy + objs[n].yl - 1) / 16 + 1;
    startx = newx / 16; endx = (newx + objs[n].xl + 15) / 16;
    for (x = startx; x < endx; ++x) {
        int tile = board(x, tile_y);
        int flags = info[tile].flags;
        if ((flags & (f_playerthru | f_notstair)) == (f_playerthru | f_notstair)) return 0;
    }
    return 1;
}

int trymove(int n, int x, int y)
{
    int flags = f_playerthru;
    if (y > objs[n].y) flags |= f_notstair;
    if (cando(n, x, y, flags) == flags) { moveobj(n, x, y); return 1; }
    if (cando(n, objs[n].x, y, flags) == flags) { moveobj(n, objs[n].x, y); return 2; }
    if (cando(n, x, objs[n].y, flags) == flags) { moveobj(n, x, objs[n].y); return 4; }
    return 0;
}

int justmove(int n, int x, int y)
{
    if (cando(n, x, y, f_playerthru) != 0) { moveobj(n, x, y); return 1; }
    return 0;
}

int onscreen(int n)
{
    return objs[n].x + objs[n].xl >= gamevp->vpox && objs[n].y + objs[n].yl >= gamevp->vpoy &&
           objs[n].x <= gamevp->vpox + gamevp->vpxl && objs[n].y <= gamevp->vpoy + gamevp->vpyl;
}

int trymovey(int n, int x, int y)
{
    const int flags = f_playerthru | f_notstair;
    if (cando(n, x, y, flags) == flags) { moveobj(n, x, y); return 1; }
    if (cando(n, objs[n].x, y, flags) == flags) { moveobj(n, objs[n].x, y); return 1; }
    objs[n].xd = 0;
    return 0;
}

int crawl(int n, int dx, int dy)
{
    if (standfloor(n, dx, dy) &&
        cando(n, objs[n].x + dx, objs[n].y + dy, f_playerthru) == f_playerthru) {
        moveobj(n, objs[n].x + dx, objs[n].y + dy);
        return 1;
    }
    return 0;
}

void addscore(int score, int x, int y)
{
    int n = addobj(obj_score, x, y);
    if (n != 0) {
        objs[n].state = (word)score;
        objs[n].counter = 16;
        objs[n].xd = (word)(JILL_SIGN(x - objs[0].x) * 2);
        objs[n].yd = 3;
        setobjsize(n);
    }
    statmodflg |= mod_screen;
    pl.score = (ulongword)((longword)pl.score + score);
}

void addtext(const char *text, int objkind, int x, int y)
{
    int n = addobj(objkind, x, y);
    if (n != 0) {
        objs[n].counter = 64;
        objs[n].inside = jill_strdup(text);
        objs[n].xd = 2;
        objs[n].yd = -1;
        setobjsize(n);
    }
}

void sendtrig(int counter, int msg, int fromobj)
{
    int n;
    for (n = 0; n < numobjs; ++n) {
        int kind = (byte)objs[n].objkind;
        if ((kindflags[kind] & f_trigger) && objs[n].counter == counter)
            (void)kindmsg[kind](n, msg, fromobj);
    }
}

void setorigin(void)
{
    gamevp->vpox = (word)((objs[0].x - scrnxs * 8) & ~7);
    gamevp->vpox = (word)JILL_MAX(0, JILL_MIN((boardxs - scrnxs) * 16, gamevp->vpox));
    gamevp->vpoy = (word)(objs[0].y + 16 - scrnys * 8);
    gamevp->vpoy = (word)JILL_MAX(0, JILL_MIN((boardys + 1 - scrnys) * 16, gamevp->vpoy));
    oldscrollxd = oldscrollyd = 0;
}

int cando(int n, int x, int y, int flags)
{
    int cell_x, cell_y;
    int start_x = x / 16;
    int start_y = y / 16;
    int end_x = (x + objs[n].xl + 15) / 16;
    int end_y = (y + objs[n].yl + 15) / 16;
    int split_y = (objs[n].y + kindyl[(byte)objs[n].objkind] + 15) / 16;
    int stair_flags = f_notstair;
    int result = 0xffff;

    for (cell_y = start_y; cell_y < end_y; ++cell_y) {
        if (cell_y >= split_y) stair_flags = 0;
        for (cell_x = start_x; cell_x < end_x; ++cell_x)
            result &= (info[board(cell_x, cell_y)].flags | stair_flags) & flags;
    }
    return result;
}

int objdo(int n, int x, int y, int flags)
{
    int cell_x, cell_y;
    int result = 0xffff;
    int start_x = x / 16;
    int start_y = y / 16;
    int end_x = (x + objs[n].xl + 15) / 16;
    int end_y = (y + objs[n].yl + 15) / 16;

    for (cell_y = start_y; cell_y < end_y; ++cell_y)
        for (cell_x = start_x; cell_x < end_x; ++cell_x)
            result &= info[board(cell_x, cell_y)].flags & flags;
    return result;
}

void touchbkgnd(int n)
{
    int x, y, startx, starty, endx, endy;
    if (stateinfo[objs[n].state] & sti_invincible) return;
    startx = objs[n].x / 16; starty = objs[n].y / 16;
    endx = (objs[n].x + objs[n].xl + 15) / 16;
    endy = (objs[n].y + objs[n].yl + 15) / 16;
    for (y = starty; y < endy; ++y)
        for (x = startx; x < endx; ++x)
            if (info[board(x, y)].flags & f_msgtouch)
                (void)msg_block(x, y, msg_touch);
}

void purgeobjs(void)
{
    int source, destination = 0;
    for (source = 0; source < numobjs; ++source) {
        if (objs[source].objkind == obj_killme) free(objs[source].inside);
        else {
            if (source != destination) objs[destination] = objs[source];
            ++destination;
        }
    }
    numobjs = (word)destination;
}

void updbotmsg(void)
{
    if (botmsg[0] != '\0') {
        --bottime;
        if (bottime < 0) {
            botmsg[0] = '\0';
            statmodflg |= mod_screen;
        }
    }
}

void hitplayer(int n)
{
    if (objs[n].zaphold == 0 && !(stateinfo[objs[0].state] & sti_invincible))
        p_ouch(1, die_ash);
    objs[n].zaphold = 3;
}

int fishdo(int n, int x, int y)
{
    const int flags = f_playerthru | f_water;
    if (objdo(n, x, y, flags) == flags) { moveobj(n, x, y); return 1; }
    return 0;
}

void pointvect(int n1, int n2, int *xout, int *yout, int length)
{
    int x, y;
    x = objs[n1].x - objs[n2].x; y = objs[n1].y - objs[n2].y;
    if (x == 0) y = length * JILL_SIGN(y);
    else if (y == 0) x = length * JILL_SIGN(x);
    else if (abs(x) > abs(y)) { y = y * length / abs(x); x = length * JILL_SIGN(x); }
    else { x = x * length / abs(y); y = length * JILL_SIGN(y); }
    *xout = x;
    *yout = y;
}

int vectdist(int n1, int n2)
{
    return abs(objs[n1].x - objs[n2].x) + abs(objs[n1].y - objs[n2].y);
}

int trybreakwall(int n, int x, int y)
{
    int cell_x, cell_y, broken = 0;
    for (cell_x = x / 16; cell_x <= (x + objs[n].xl) / 16; ++cell_x) {
        for (cell_y = y / 16; cell_y <= (y + objs[n].yl) / 16; ++cell_y) {
            if (board(cell_x, cell_y) == BREAKWALL_TILE) {
                setboard(cell_x, cell_y, 0);
                if (broken++ == 0) {
                    explode1(cell_x * 16, cell_y * 16, 5);
                    snd_play(2, 49);
                }
            }
        }
    }
    return broken;
}
