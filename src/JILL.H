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

#ifndef JILL_H
#define JILL_H

#include <stdio.h>
#include "RECOVERY.H"
#include "GR.H"
#include "JOBJ.H"

#define boardxs 128
#define boardys 64
#define normxs 15
#define normys 11
#define fullxs 20
#define fullys 13
#define numinfotypes 600
#define maxobjs 256
#define maxscrnobjs 192
#define maxinventory 16

#define mod_screenonly ((uword)0x8000)
#define mod_screen     ((uword)0xc000)
#define mod_virtual    ((uword)0x4000)
#define mod_page0      ((uword)0x4000)
#define mod_page1      ((uword)0x8000)

#define f_playerthru 0x0001
#define f_notstair   0x0002
#define f_notvine    0x0004
#define f_msgtouch   0x0008
#define f_msgdraw    0x0010
#define f_msgupdate  0x0020
#define f_inside     0x0040
#define f_front      0x0080
#define f_trigger    0x0100
#define f_back       0x0200
#define f_killable   0x0400
#define f_fireball   0x0800
#define f_always     0x1000
#define f_water      0x2000
#define f_weapon     0x4000

enum { msg_draw, msg_touch, msg_update, msg_trigger, msg_trigoff, msg_trigon };
enum { st_stand, st_jumping, st_climbing, st_die, st_begin };
enum { die_ash, die_bird, die_fish };
enum { sti_canfire = 1, sti_invincible = 2 };

typedef struct infotype {
    uword sh;
    uword flags;
    char *name;
} infotype;

/* Exact 31-byte DOS level record.  The far pointer is retained as an opaque
   serialized value; loadboard resolves it through the following string stack. */
#pragma pack(push, 1)
typedef struct JillDiskObject {
    byte objkind;
    word x, y, xd, yd, xl, yl;
    word state, substate, statecount, counter, objflags;
    ulongword string_pointer;
    word info1, zaphold;
} JillDiskObject;

typedef struct JillDiskPlayer {
    word level;
    word health;
    word numinv;
    word inv[maxinventory];
    ulongword score;
    word ouched;
    ulongword oldscore;
    byte pad[22];
} JillDiskPlayer;
#pragma pack(pop)

typedef struct objtype {
    sbyte objkind;
    word x, y, xd, yd, xl, yl;
    word state, substate, statecount, counter;
    uword objflags;
    char *inside;
    word info1, zaphold;
} objtype;

typedef struct pltype {
    word level;
    word health;
    word numinv;
    word inv[maxinventory];
    ulongword score;
    word ouched;
    ulongword oldscore;
    byte pad[22];
} pltype;

typedef int (*object_message_fn)(int n, int msg, int z);

extern vptype *gamevp, *statvp, *cmdvp;
extern vptype botvp, tempvp;
extern word scrnxs, scrnys;
extern uword bd[boardxs][boardys];
extern infotype info[numinfotypes];
extern object_message_fn kindmsg[numobjkinds];
extern word kindxl[numobjkinds], kindyl[numobjkinds];
extern const char *kindname[numobjkinds];
extern uword kindflags[numobjkinds];
extern word kindtable[numobjkinds], kindscore[numobjkinds];
extern word inv_shape[11];
extern word inv_xfm[11];
extern const char *inv_getmsg[11];
extern objtype objs[maxobjs + 2];
extern word numobjs, numscrnobjs;
extern word scrnobjs[maxscrnobjs + 1];
extern pltype pl;

extern word scrollxd, scrollyd, oldscrollxd, oldscrollyd, oldx0, oldy0;
extern word gameover, gamecount, statmodflg, designflag, peeky;
extern word stateinfo[6];
extern char newlevel[32], curlevel[32];
extern const char *leveltxt[32];
extern char botmsg[60];
extern word botcol, bottime;

#define board(x, y) (bd[(x)][(y)] & 0x3fffU)

void setboard(int x, int y, int value);
void modboard(int x, int y);
void initobjs(void);
void playerkill(int n);
int countobj(int objkind);
void notemod(int n);
void setobjsize(int n);
int findcheckpt(int level);
void dolevelsong(void);
void p_reenter(int died);
void p_ouch(int healthtake, int diemode);
void seekplayer(int n, int *dx, int *dy);
void modjunglescroll(int xd, int yd, int modcode);
void junglescroll(int xd, int yd);
#if defined(JILL_EP3)
void pagedjunglescroll(int xd, int yd);
#endif
void refresh(int page_mode);
void updbkgnd(void);
void updobjs(int doflag);
void killobj(int n);
int addobj(int kind, int x, int y);
void addinv(int invthing);
int takeinv(int invthing);
int invcount(int invthing);
void initinv(void);
void moveobj(int n, int x, int y);
int standfloor(int n, int dx, int dy);
int trymove(int n, int x, int y);
int justmove(int n, int x, int y);
int onscreen(int n);
int trymovey(int n, int x, int y);
int crawl(int n, int dx, int dy);
void addscore(int score, int x, int y);
void addtext(const char *text, int objkind, int x, int y);
void sendtrig(int counter, int msg, int fromobj);
void setorigin(void);
int cando(int n, int x, int y, int flags);
int objdo(int n, int x, int y, int flags);
void touchbkgnd(int n);
void purgeobjs(void);
void updbotmsg(void);
void hitplayer(int n);
int fishdo(int n, int x, int y);
void pointvect(int n1, int n2, int *x, int *y, int length);
int vectdist(int n1, int n2);
int trybreakwall(int n, int x, int y);

void initinfo(void);
void calc_scroll(int vertical_delta);
int playerxfm(int n);

void drawstats(void);
void drawcell(int x, int y);
void drawboard(void);
void moddrawboard(void);
void zapobjs(void);
void loadboard(const char *filename);
void saveboard(const char *filename);
void putbotmsg(const char *message, int color);
void dotextmsg(int number);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(JillDiskObject) == 31, "Jill object disk record must be 31 bytes");
_Static_assert(sizeof(JillDiskPlayer) == 70, "Jill player disk record must be 70 bytes");
#endif

#endif
