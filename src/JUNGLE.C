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

#include "JUNGLE.H"

#include "CONFIG.H"
#include "COPYFILE.H"
#include "DESIGN.H"
#include "GAMECTRL.H"
#include "HOSTWIN.H"
#include "KEYBOARD.H"
#include "MUSIC.H"
#include "PIXWRITE.H"
#include "SHM.H"
#include "WINDOWS.H"

#include <ctype.h>
#include <fcntl.h>
#include "DOSIO.H"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define JILL_HIGH_COUNT      10
#define JILL_HIGH_STORAGE_COUNT 12
#define JILL_HIGH_NAME_LEN   10
#define JILL_SAVE_COUNT       6
#define JILL_SAVE_NAME_LEN   12
#define JILL_BOARD_BYTES (boardxs * boardys * 2)

wintype ourwin, levelwin;
vptype botvp, tempvp;
vptype *gamevp = &ourwin.inside;
vptype *cmdvp = &ourwin.topleft;
vptype *statvp = &ourwin.botleft;
word scrnxs = normxs, scrnys = normys;
uword bd[boardxs][boardys];
char newlevel[32], curlevel[32];
const char *leveltxt[32] = {
    "JILL ENTERS\rTHE\rJUNGLE MAP.\r",
    "JILL BOUNDS\rTHROUGH\rTHE BOULDERS\r",
    "JILL JOURNEYS\rINTO\rTHE FOREST\r",
    "JILL ENTERS\rTHE HUT\r",
    "\r", "\r",
    "JILL DASHES INTO\rTHE CASTLE\r",
    "JILL EXPLORES\rTHE FOREST\r",
    "JILL SNEAKS\rINTO\rARG'S DUNGEON\r",
    "JILL ENTERS\rTHE\rPHOENIX MAZE\r",
    "JILL VENTURES\rINTO THE\rKNIGHT'S PUZZLE\r",
    "JILL CREEPS INTO\rTHE DARK FOREST\r",
    "JILL SPLASHES INTO\rTHE UNDERGROUND\rRIVER\r",
    "JILL ENTERS\rYET\rANOTHER PUZZLE\r",
    "JILL ENTERS\rTHE PLATEAU\r",
    "JILL REACHES\rTHE ENDING\r\rNOW SIT BACK\rAND ENJOY\r",
    "\r", "17\r", "18\r", "19\r",
    "JILL BOUNDS INTO\rTHE BONUS LEVEL\r",
    "21\r", "22\r", "23\r", "24\r", "25\r", "26\r", "27\r", "28\r", "29\r",
    "", ""
};
char botmsg[60];
word botcol, bottime;
char oursong[32], tempname[64];
word oldlevelnum;
word facetable = 24;
word xbordercol = 1;
word xmsgdelay;
word debug, turtle, xdemoflag;
word levelmsgclock;
char xshafile[] = "jill1.sha";
word inv_shape[11] = {
    0x0026, 0x000c, 0x000d, 0x000b, 0x000e, 0x000f,
    0x0012, 0x0014, 0x0023, 0x0024, 0x0025
};

static char high_name[JILL_HIGH_STORAGE_COUNT][JILL_HIGH_NAME_LEN];
static ulongword high_score[JILL_HIGH_COUNT];
static char save_name[JILL_SAVE_COUNT][JILL_SAVE_NAME_LEN];
static int selected_save;

char *demoboard[10] = {
    "intro.jn1", "", "", "", "", "", "", "", "", ""
};
byte demolvl[10] = { 100, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char *demoname[10] = {
    "jn1demo.mac", "", "", "", "", "", "", "", "", ""
};
word demonum;

static void set_game_layout(void);

static void data_path(char *destination, size_t capacity, const char *name)
{
    size_t length;
    if (destination == NULL || capacity == 0) return;
    destination[0] = '\0';
    if (cfg_path[0] != '\0') {
        strncpy(destination, cfg_path, capacity - 1);
        destination[capacity - 1] = '\0';
        length = strlen(destination);
        if (length != 0 && destination[length - 1] != '\\' && destination[length - 1] != '/' &&
            length + 1 < capacity) {
            destination[length++] = '\\';
            destination[length] = '\0';
        }
    }
    if (strlen(destination) + strlen(name) + 1 < capacity) strcat(destination, name);
}

static int read_exact(FILE *file, void *destination, size_t length)
{
    return file != NULL && fread(destination, 1, length, file) == length;
}

static int write_exact(FILE *file, const void *source, size_t length)
{
    return file != NULL && fwrite(source, 1, length, file) == length;
}

static int read_word_file(FILE *file, uword *value)
{
    byte encoded[2];
    if (!read_exact(file, encoded, sizeof(encoded))) return 0;
    *value = jill_read_u16_le(encoded);
    return 1;
}

static int write_word_file(FILE *file, uword value)
{
    byte encoded[2];
    jill_write_u16_le(encoded, value);
    return write_exact(file, encoded, sizeof(encoded));
}

static void decode_object(objtype *object, const byte encoded[31])
{
    object->objkind = (sbyte)encoded[0];
    object->x = (word)jill_read_u16_le(encoded + 1);
    object->y = (word)jill_read_u16_le(encoded + 3);
    object->xd = (word)jill_read_u16_le(encoded + 5);
    object->yd = (word)jill_read_u16_le(encoded + 7);
    object->xl = (word)jill_read_u16_le(encoded + 9);
    object->yl = (word)jill_read_u16_le(encoded + 11);
    object->state = (word)jill_read_u16_le(encoded + 13);
    object->substate = (word)jill_read_u16_le(encoded + 15);
    object->statecount = (word)jill_read_u16_le(encoded + 17);
    object->counter = (word)jill_read_u16_le(encoded + 19);
    object->objflags = jill_read_u16_le(encoded + 21);
    object->inside = jill_read_u32_le(encoded + 23) != 0 ? (char *)(uintptr_t)1 : NULL;
    object->info1 = (word)jill_read_u16_le(encoded + 27);
    object->zaphold = (word)jill_read_u16_le(encoded + 29);
}

static void encode_object(byte encoded[31], const objtype *object)
{
    memset(encoded, 0, 31);
    encoded[0] = (byte)object->objkind;
    jill_write_u16_le(encoded + 1, (uword)object->x);
    jill_write_u16_le(encoded + 3, (uword)object->y);
    jill_write_u16_le(encoded + 5, (uword)object->xd);
    jill_write_u16_le(encoded + 7, (uword)object->yd);
    jill_write_u16_le(encoded + 9, (uword)object->xl);
    jill_write_u16_le(encoded + 11, (uword)object->yl);
    jill_write_u16_le(encoded + 13, (uword)object->state);
    jill_write_u16_le(encoded + 15, (uword)object->substate);
    jill_write_u16_le(encoded + 17, (uword)object->statecount);
    jill_write_u16_le(encoded + 19, (uword)object->counter);
    jill_write_u16_le(encoded + 21, object->objflags);
    jill_write_u32_le(encoded + 23, object->inside != NULL ? 1U : 0U);
    jill_write_u16_le(encoded + 27, (uword)object->info1);
    jill_write_u16_le(encoded + 29, (uword)object->zaphold);
}

static void decode_player(const byte encoded[70])
{
    int index;
    pl.level = (word)jill_read_u16_le(encoded);
    pl.health = (word)jill_read_u16_le(encoded + 2);
    pl.numinv = (word)jill_read_u16_le(encoded + 4);
    for (index = 0; index < maxinventory; ++index)
        pl.inv[index] = (word)jill_read_u16_le(encoded + 6 + index * 2);
    pl.score = jill_read_u32_le(encoded + 38);
    pl.ouched = (word)jill_read_u16_le(encoded + 42);
    pl.oldscore = jill_read_u32_le(encoded + 44);
    memcpy(pl.pad, encoded + 48, sizeof(pl.pad));
}

static void encode_player(byte encoded[70])
{
    int index;
    memset(encoded, 0, 70);
    jill_write_u16_le(encoded, (uword)pl.level);
    jill_write_u16_le(encoded + 2, (uword)pl.health);
    jill_write_u16_le(encoded + 4, (uword)pl.numinv);
    for (index = 0; index < maxinventory; ++index)
        jill_write_u16_le(encoded + 6 + index * 2, (uword)pl.inv[index]);
    jill_write_u32_le(encoded + 38, pl.score);
    jill_write_u16_le(encoded + 42, (uword)pl.ouched);
    jill_write_u32_le(encoded + 44, pl.oldscore);
    memcpy(encoded + 48, pl.pad, sizeof(pl.pad));
}

void fin(void) { snd_play(1, 22); fadein(); }
void fout(void) { snd_play(1, 21); fadeout(); }

void drawkeys(void)
{
    const char *first;
    const char *second;

    switch (objs[0].objkind) {
    case obj_player:
        fontcolor(cmdvp, 7, 8);
        first = "JUMP   ";
        if (invcount(inv_blade)) second = "BLADE  ";
        else if (invcount(inv_knife)) second = "KNIFE  ";
        else second = "       ";
        break;
    case obj_tiny:
        fontcolor(cmdvp, 7, 8);
        first = "       ";
        second = "       ";
        break;
    case obj_jillbird:
        fontcolor(cmdvp, 7 - pagedraw * 4, 8);
        first = "FLAP   ";
        second = "FIRE   ";
        break;
    case obj_jillfrog:
        fontcolor(cmdvp, 7 - pagedraw * 4, 8);
        first = "HOP    ";
        second = "LEAP   ";
        break;
    case obj_jillfish:
        fontcolor(cmdvp, 7 - pagedraw * 6, 8);
        first = "SWIM   ";
        second = "SHOOT  ";
        break;
    case 55:
        fontcolor(cmdvp, 7, 8);
        first = "       ";
        second = "       ";
        break;
    default:
        return;
    }

    wprint(cmdvp, 37, 10, 2, first);
    wprint(cmdvp, 33, 19, 2, second);
}

void drawcmds(void)
{
    fontcolor(cmdvp, 4, 8);
    clearvp(cmdvp);
    wprint(cmdvp, 0, 33, 1, "____________");
    wprint(cmdvp, 5, 2, 2, "Move Jill");
    fontcolor(cmdvp, 5, 8);
    drawshape(cmdvp, 0x0600, 2, 9);
    drawshape(cmdvp, 0x0601, 2, 18);
    drawshape(cmdvp, 0x0609, 2, 27);
    fontcolor(cmdvp, 7, 8);
    wprint(cmdvp, 30, 28, 2, "Help");
    fontcolor(cmdvp, 3, 8);
    wprint(cmdvp, 1, 43, 1, "N");
    wprint(cmdvp, 1, 51, 1, "Q");
    wprint(cmdvp, 1, 59, 1, "S");
    wprint(cmdvp, 1, 67, 1, "R");
    wprint(cmdvp, 1, 75, 1, "T");
    fontcolor(cmdvp, 2, 8);
    wprint(cmdvp, 14, 44, 2, "NOISE");
    wprint(cmdvp, 14, 52, 2, "QUIT");
    wprint(cmdvp, 14, 60, 2, "SAVE");
    wprint(cmdvp, 14, 68, 2, "RESTORE");
    wprint(cmdvp, 14, 76, 2, "TURTLE");
    drawkeys();
    drawstats();
    statmodflg |= mod_screen;
}

void putbotmsg(const char *message, int color)
{
    strcpy(botmsg, message);
    botcol = (word)color;
    bottime = 80;
    statmodflg |= mod_screen;
}

void drawstats(void)
{
    char message[32];
    int index;

    fontcolor(cmdvp, -7, 8);
    drawshape(cmdvp, 0x060a + soundf, 53, 43);
    drawshape(cmdvp, 0x060a + turtle, 53, 75);
    fontcolor(statvp, -5, pl.ouched ? 4 : 8);
    clearvp(statvp);
    wprint(statvp, 2, 2, 2, "HEALTH");
    fontcolor(statvp, -4, 8);
    for (index = 0; index < pl.health - 1; ++index)
        drawshape(statvp, 0xe2a, index * 3 + 42, 2);
    drawshape(statvp, 0xe2b, (pl.health - 1) * 3 + 40, 2);
    wprint(statvp, 33, 10, 2, "SCORE");
    _ltoa((longword)pl.score, message, 10);
    wprint(statvp, 64 - ((int)strlen(message) * 6 + 1), 16, 2, message);
    fontcolor(statvp, -2, 8);
    wprint(statvp, 1, 10, 2, "LEVEL");
    if (pl.level == 127) strcpy(message, "MAP");
    else _ltoa((longword)pl.level, message, 10);
    wprint(statvp, 1, 16, 2, message);
    if (debug && !swrite) {
        _ltoa(host_coreleft(), message, 10);
        strcat(message, "     ");
        wprint(statvp, 28, 64, 2, message);
    }
    for (index = 0; index < pl.numinv; ++index)
        drawshape(statvp, 0xe00 + inv_shape[pl.inv[index]],
                  (index / 3) * 14 + 1, (index % 3) * 14 + 26);
    drawkeys();
    clearvp(&botvp);
    fontcolor(&botvp, botcol, 0);
    wprint(&botvp, 160 - (int)strlen(botmsg) * 3, 2, 2, botmsg);
}

void zapobjs(void)
{
    int index;
    for (index = 0; index < numobjs; ++index) {
        if (objs[index].inside != NULL && objs[index].inside != (char *)(uintptr_t)1)
            free(objs[index].inside);
        objs[index].inside = NULL;
    }
    initobjs();
}

void loadcfg(void)
{
    char path[64];
    int handle;
    int index;

    data_path(path, sizeof(path), "JILL1.CFG");
    handle = _open(path, _O_BINARY | _O_RDONLY);
    if (handle < 0 || _filelength(handle) <= 0) {
        for (index = 0; index < JILL_HIGH_COUNT; ++index) {
            high_name[index][0] = '\0';
            high_score[index] = 0;
        }
        for (index = 0; index < JILL_SAVE_COUNT; ++index)
            save_name[index][0] = '\0';
        cf.firstthru = 0;
        cf.joyflag0 = 0;
        cf.video_mode = x_vga;
        cf.musicflag0 = 1;
        cf.vocflag0 = 1;
    } else {
        (void)_read(handle, high_name, 120);
        (void)_read(handle, high_score, 40);
        (void)_read(handle, save_name, 72);
        if (_read(handle, &cf, 22) < 0) cf.firstthru = 1;
    }
    if (handle >= 0) _close(handle);
}

void savecfg(void)
{
    char path[64];
    int handle;

    data_path(path, sizeof(path), "JILL1.CFG");
    handle = _creat(path, 0);
    if (handle >= 0) {
        (void)_write(handle, high_name, 120);
        (void)_write(handle, high_score, 40);
        (void)_write(handle, save_name, 72);
        (void)_write(handle, &cf, 22);
        _close(handle);
    }
}

void loadboard(const char *filename)
{
    byte board_image[JILL_BOARD_BYTES];
    byte player_image[70];
    byte object_image[31];
    byte string_marker[maxobjs];
    FILE *file;
    uword disk_object_count;
    int index, x, y;

    for (index = 9; index < SHM_MAX_TABLES; ++index) shm_want[index] = 0;
    shm_want[14] = 1;
    shm_want[46] = 1;
    strcpy(curlevel, filename);
    zapobjs();
    file = fopen(filename, "rb");
    if (file == NULL || fread(board_image, 1, sizeof(board_image), file) == 0) rexit(1);
    if (fread(&disk_object_count, 1, 2, file) == 0) rexit(2);
    if (disk_object_count > maxobjs) rexit(3);

    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y)
            bd[x][y] = jill_read_u16_le(board_image + (x * boardys + y) * 2);

    memset(string_marker, 0, sizeof(string_marker));
    numobjs = (word)disk_object_count;
    for (index = 0; index < numobjs; ++index) {
        if (fread(object_image, 1, sizeof(object_image), file) == 0) rexit(3);
        decode_object(&objs[index], object_image);
        string_marker[index] = objs[index].inside != NULL;
        objs[index].inside = NULL;
    }
    if (fread(player_image, 1, sizeof(player_image), file) == 0) rexit(4);
    decode_player(player_image);

    for (index = 0; index < numobjs; ++index) {
        uword length;
        if (!string_marker[index]) continue;
        (void)fread(&length, 1, 2, file);
        objs[index].inside = (char *)malloc((size_t)length + 1);
        (void)fread(objs[index].inside, 1, (size_t)length + 1, file);
    }
    fclose(file);
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y) {
            unsigned cell = board(x, y);
            shm_want[(info[cell].sh >> 8) & 0x3f] = 1;
        }
    for (index = 0; index < numobjs; ++index) {
        int kind = objs[index].objkind;
        shm_want[kindtable[kind]] = 1;
    }
    shm_do();
}

void saveboard(const char *filename)
{
    byte board_image[JILL_BOARD_BYTES];
    byte player_image[70];
    byte object_image[31];
    FILE *file;
    int index, x, y;

    file = fopen(filename, "wb");
    if (file == NULL) rexit(201);
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y)
            jill_write_u16_le(board_image + (x * boardys + y) * 2, bd[x][y]);
    if (!write_exact(file, board_image, sizeof(board_image))) rexit(202);
    (void)write_word_file(file, (uword)numobjs);
    for (index = 0; index < numobjs; ++index) {
        encode_object(object_image, &objs[index]);
        (void)write_exact(file, object_image, sizeof(object_image));
    }
    encode_player(player_image);
    (void)write_exact(file, player_image, sizeof(player_image));
    for (index = 0; index < numobjs; ++index) {
        size_t length;
        if (objs[index].inside == NULL) continue;
        length = strlen(objs[index].inside);
        (void)write_word_file(file, (uword)length);
        (void)write_exact(file, objs[index].inside, length + 1);
    }
    fclose(file);
}

int numlines(void)
{
    int lines = 0, index;
    for (index = 0; index < textmsglen; ++index) lines += textmsg[index] == 13;
    return lines;
}

int getline(int number, char *line, int add_spaces)
{
    int color = 7, current = 0, source = 0, output = 0;
    line[0] = '\0';
    while (source < textmsglen && current < number)
        if (textmsg[source++] == 13) ++current;
    while (source < textmsglen && (unsigned char)textmsg[source] < 32 && textmsg[source] != 13) ++source;
    if (source < textmsglen && textmsg[source] >= '0' && textmsg[source] <= '7')
        color = textmsg[source++] - '0';
    while (source < textmsglen && textmsg[source] != 13 && output < 77) {
        unsigned char character = (unsigned char)textmsg[source++];
        line[output++] = add_spaces ? (char)toupper(character) : (char)character;
    }
    line[output] = '\0';
    return color;
}

void printline(vptype *viewport, int y, int number)
{
    char line[80];
    fontcolor(viewport, getline(number, line, 1), 1);
    wprint(viewport, 0, y, 2, "                                    ");
    wprint(viewport, (viewport->vpxl - (int)strlen(line) * 6) / 2,
           y, 2, line);
}

void ourdelay(void)
{
    uword start = *myclock;
    while ((word)(*myclock - start) < xmsgdelay) { }
}

void dotextmsg(int number)
{
    wintype textwin;
    int c, linecount, y, y0, textys;
    char line[80];
    dx1hold = 1;
    dy1hold = 1;
    text_get(number);
    if (textmsg != NULL) {
        setpagemode(1);
        defwin(&textwin,
               gamevp->vpx / 8 + 2,
               gamevp->vpy + 16,
               gamevp->vpxl / 16 - 3,
               gamevp->vpyl / 16 - 4,
               0, 0, 0);
        drawwin(&textwin);
        fontcolor(&textwin.inside, 7, 1);
        clearvp(&textwin.inside);
        fontcolor(&textwin.border, getline(0, line, 0), -1);
        titlewin(&textwin, line);
        fontcolor(&textwin.inside, 7, 0);

        textys = textwin.inside.vpyl / 6;
        linecount = numlines();
        if (linecount <= textys) {
            y = (textwin.inside.vpyl - 6 * (linecount - 1)) / 2;
            for (c = 1; c < linecount; ++c) {
                printline(&textwin.inside, y, c);
                y += 6;
            }
            pageflip();
            ourdelay();
            do checkctrl0(1);
            while (dx1 != 0 || dy1 != 0 || key != 0 || fire1 != 0);
            do checkctrl0(1);
            while (key != key_space && key != key_enter && fire1 == 0);
        } else {
            y0 = 0;
            y = 0;
            for (c = 1; c <= textys; ++c) {
                printline(&textwin.inside, y, c);
                y += 6;
            }
            pageflip();
            setpagemode(0);
            fire1off = 1;
            do checkctrl0(1);
            while (dx1 != 0 || dy1 != 0 || key != 0);
            ourdelay();
            do {
                checkctrl0(0);
                dx1 += (key == key_pgdown) - (key == key_pgup);
                if (dx1 + dy1 < 0 && y0 > 0) {
                    --y0;
                    scrollvp(&textwin.inside, 0, 6);
                    printline(&textwin.inside, 0, y0 + 1);
                } else if (dx1 + dy1 > 0 && y0 + textys < linecount) {
                    ++y0;
                    scrollvp(&textwin.inside, 0, -6);
                    printline(&textwin.inside, 6 * (textys - 1),
                              y0 + textys);
                }
            } while (key != key_enter && key != key_escape && fire1 == 0);
            setpagemode(1);
        }
        moddrawboard();
        free(textmsg);
        key = 0;
    }
}

void initboard(void)
{
    int x, y;
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y)
            setboard(x, y, 0);
    gamevp->vpox = 0;
    gamevp->vpoy = 0;
}

void putlevelmsg(int number)
{
    int c, linecount, y;
    char line[80];

    levelmsgclock = *myclock;
    if (number >= 32) return;
    textmsg = (char *)leveltxt[number];
    textmsglen = (int)strlen(textmsg);
    if (textmsg != NULL) {
        setpagemode(1);
        drawwin(&levelwin);
        fontcolor(&levelwin.inside, 7, 1);
        clearvp(&levelwin.inside);
        if (x_ourmode == x_vga && facetable != 0) {
            for (c = 0; c < 16; ++c)
                drawshape(&levelwin.topleft,
                          0x4000 + facetable * 0x100 + c,
                          16 * (c & 3), 16 * (c / 4));
        }
        linecount = numlines();
        y = (levelwin.inside.vpyl - 6 * (linecount - 1)) / 2;
        for (c = 0; c < linecount; ++c) {
            fontcolor(&levelwin.inside, getline(c, line, 0), 1);
            wprint(&levelwin.inside,
                   (levelwin.inside.vpxl - 6 * (int)strlen(line)) / 2,
                   y, 2, line);
            y += 6;
        }
        pageflip();
        moddrawboard();
    }
}

void donelevelmsg(void)
{
    int dt, done = 0;
    do checkctrl0(0); while (key != 0);
    do {
        checkctrl0(0);
        dt = (word)(*myclock - levelmsgclock) / 18;
        if (key == key_escape || key == key_enter) done = 1;
        else if (dt >= 2 && (key != 0 || fire1 != 0)) done = 1;
        else if (dt >= 4) done = 1;
    } while (!done);
}

void drawcell(int x, int y)
{
    unsigned cell;
    if (x < 0 || x >= boardxs || y < 0 || y >= boardys) return;
    cell = board(x, y);
    if ((info[cell].flags & f_msgdraw) == 0) drawshape(gamevp, info[cell].sh, x * 16, y * 16);
    else (void)msg_block(x, y, msg_draw);
}

void drawboard(void)
{
    int x, y;
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y) modboard(x, y);
    updobjs(0);
    statmodflg = 0;
    refresh(0);
}

void moddrawboard(void)
{
    int x, y;
    for (x = 0; x < boardxs; ++x)
        for (y = 0; y < boardys; ++y) modboard(x, y);
    statmodflg |= mod_screen;
}

static void set_game_layout(void)
{
    scrnxs = normxs;
    scrnys = normys;
    defwin(&ourwin, 0, 0, 19, 10, 4, 5, dialog);
    gamevp = &ourwin.inside;
    cmdvp = &ourwin.topleft;
    statvp = &ourwin.botleft;
    botvp.vpx = 0; botvp.vpy = 188;
    botvp.vpxl = 320; botvp.vpyl = 12;
    botvp.vpox = botvp.vpoy = 0;
    botvp.vphi = botvp.vpback = 0;
    if (facetable != 0 && x_ourmode == x_vga) {
        defwin(&levelwin, 12, 48, 11, 4, 4, 0, 0);
        levelwin.topleft.vpy = levelwin.inside.vpy;
        levelwin.topleft.vpyl = levelwin.inside.vpyl;
    } else {
        defwin(&levelwin, 13, 48, 8, 4, 0, 0, 0);
    }
}

void play(int demo_flag)
{
    int c, begclock, temppage;
    int cheatchar = 0;
    int cheatcount = 0;
    word saved_score;

    putbotmsg("PRESS F1 FOR HELP!", 4);
    initinv();
    setorigin();
    drawcmds();
    drawstats();
    newlevel[0] = '\0';
    setpagemode(1);
    dolevelsong();
    gameover = 0;

    do {
        if (newlevel[0] != '\0') {
            if (newlevel[0] == '*') {
                memmove(newlevel, newlevel + 1, strlen(newlevel));
                strcpy(oursong, newlevel);
                sb_playtune(newlevel);
                newlevel[0] = '\0';
            } else if (newlevel[0] == '#') {
                memmove(newlevel, newlevel + 1, strlen(newlevel));
                strcpy(oursong, newlevel);
                if (!sb_playing()) sb_playtune(newlevel);
                newlevel[0] = '\0';
            } else if (newlevel[0] == '&') {
                memmove(newlevel, newlevel + 1, strlen(newlevel));
                macabort = 2;
                playmac(newlevel);
                strcpy(oursong, newlevel);
                if (!sb_playing()) sb_playtune(newlevel);
                newlevel[0] = '\0';
            } else if (newlevel[0] == '!') {
                int saved_count;
                putlevelmsg(0);
                c = invcount(3);
                saved_count = c;
                saved_score = (word)pl.score;
                loadboard(tempname);
                pl.score = (ulongword)(longword)(word)saved_score;
                (void)remove(tempname);
                while (c-- > 0) addinv(3);
                newlevel[0] = '\0';
                p_reenter(0);
                c = findcheckpt(pl.level);
                if (objs[c].state != 1 || saved_count > 0) {
                    killobj(c);
                } else {
                    putbotmsg("YOU LEFT WITHOUT FINDING A GEM!", 4);
                    moveobj(0, objs[c].x, objs[c].y);
                    pl.level = 0;
                }
                donelevelmsg();
            } else {
                oldlevelnum = pl.level;
                putlevelmsg(pl.level);
                saveboard(tempname);
                saved_score = (word)pl.score;
                loadboard(newlevel);
                pl.score = (ulongword)(longword)(word)saved_score;
                newlevel[0] = '\0';
                pl.level = oldlevelnum;
                p_reenter(0);
                donelevelmsg();
            }
        }

        begclock = *myclock;
        sb_update();
        ++gamecount;
        checkctrl(1);
        key = (word)toupper(key);

        if (key != 0) {
            if (cheatchar == '/') {
                if (cheatcount == 2) {
                    cheatchar = 0;
                    if (key == '0') {
                        if (macrecord) macrecend();
                    } else if (key == 'R') {
                        recordmac("temp.mac");
                    } else if (key == 'C') {
                        playmac("temp.mac");
                    }
                    key = 0;
                }
            }

            if (key == cheatchar) ++cheatcount;
            else {
                cheatcount = 1;
                cheatchar = key;
            }

            if (cheatchar == 'X' && cheatcount == 3) {
                cheatchar = 0;
                pl.health = 8;
                if (!invcount(10)) addinv(10);
                if (!invcount(1)) addinv(1);
                statmodflg |= mod_screen;
            } else if (cheatchar == 'Z' && cheatcount == 3) {
                cheatchar = 0;
                debug = !debug;
                statmodflg |= mod_screen;
            } else if (cheatchar == 'W' && cheatcount == 3) {
                getkey();
                pixwrite(key - '0');
                swrite = 1;
                cheatchar = 0;
            }
        }

        switch (key) {
        case 'N':
            soundf = !soundf;
            statmodflg |= mod_screen;
            break;
        case 'T':
            turtle = !turtle;
            statmodflg |= mod_screen;
            break;
        case 'P':
            do {
                checkctrl0(0);
                sb_update();
            } while (key == 0 && fire1 == 0 && fire2 == 0 && dx1 == 0 && dy1 == 0);
            break;
        default:
            break;
        }

        if (demo_flag && !xdemoflag && countobj(0x43) == 0)
            (void)addobj(0x43, objs[0].x, objs[0].y);

        updbkgnd();
        updobjs(1);
        updbotmsg();
        refresh(pagemode);
        purgeobjs();

        switch (toupper(key)) {
        case 'S':
            temppage = pagedraw;
            pagedraw = pageshow;
            setpages();
            savegame();
            drawcmds();
            pagedraw = (word)temppage;
            setpages();
            key = key_space;
            break;
        case 'R':
            temppage = pagedraw;
            pagedraw = pageshow;
            setpages();
            (void)loadgame();
            dolevelsong();
            drawcmds();
            pagedraw = (word)temppage;
            setpages();
            setorigin();
            moddrawboard();
            key = key_space;
            break;
        case key_f1:
            dotextmsg(1);
            break;
        case key_escape:
        case 'Q':
            setpagemode(0);
            gameover = (word)askquit();
            setpagemode(1);
            moddrawboard();
            break;
        default:
            break;
        }

        if (demo_flag && !macplay) gameover = 1;
        if (!host_is_open()) gameover = 1; /* Win32 transport boundary. */
        while ((word)(*myclock - begclock) < turtle + 1) { }
    } while (!gameover);

    key = 0;
    if (gameover == 2) pageview(200);
    setpagemode(0);
    if (!demo_flag) printhi(1);
}

void pleasewait(void)
{
    wintype waitwin;
    uword start_clock;
    int x;
    int y;

    clrpal();
    setpagemode(1);
    if (xshafile[0] != 'o' || x_ourmode != x_vga) {
        defwin(&waitwin, 6, 56, 11, 3, 0, 0, textbox);
        drawwin(&waitwin);
        fontcolor(&waitwin.inside, 15, -1);
        wprint(&waitwin.inside, 32, 3, 2, "A NEW RELEASE FROM");
        wprint(&waitwin.inside, 30, 10, 1, "Epic MegaGames");
        wprint(&waitwin.inside, 48, 32, 2, "PRODUCED BY");
        wprint(&waitwin.inside, 48, 39, 2, "Tim Sweeney");
        fontcolor(&waitwin.inside, 2, -1);
        wprint(&waitwin.inside,
               (waitwin.inside.vpxl - (int)strlen("Jill of the Jungle") * 8) / 2,
               21, 1, "Jill of the Jungle");
        fontcolor(&mainvp, 1, 0);
        wprint(&mainvp, 64, 160, 2, "NOW LOADING, PLEASE WAIT...");

        if (xshafile[0] == 'o') {
            for (x = 0; x <= 5; ++x) {
                drawshape(&mainvp, 0x0c06 + x, x * 32, 0);
                drawshape(&mainvp, 0x0c06 + x, 304 - x * 32, 0);
                drawshape(&mainvp, 0x0c00 + x, x * 32, 168);
                drawshape(&mainvp, 0x0c00 + x, 304 - x * 32, 168);
            }
        }
        pageflip();
        setpagemode(0);
        fadein();
    } else {
        shm_want[34] = 1;
        shm_do();
        clrvp(&mainvp, 0);
        for (x = 0; x < 19; ++x)
            for (y = 0; y < 12; ++y)
                drawshape(&mainvp, 0x6201 + y * 19 + x,
                          x * 16, y * 16);
        clrpal();
        pageflip();
        fadein();
        start_clock = *myclock;
        while ((word)(*myclock - start_clock) < 80 && host_is_open()) { }
        fadeout();
        clrvp(&mainvp, 0);
        pageflip();
        clrvp(&mainvp, 0);
        shm_want[47] = 1;
        shm_want[34] = 0;
        shm_do();
        for (x = 0; x < 19; ++x)
            for (y = 0; y < 12; ++y)
                drawshape(&mainvp, 0x6f01 + y * 19 + x,
                          x * 16, y * 16);
        setpagemode(0);
        clrpal();
        pageflip();
        fadein();
        start_clock = *myclock;
        while ((word)(*myclock - start_clock) < 60 && host_is_open()) { }
        shm_want[47] = 0;
        shm_do();
    }
}

void printhi(int new_high_score)
{
    char string[20];
    int background;
    int position = JILL_HIGH_COUNT;
    int index;
    int character;
    int max_length;

    background = new_high_score ? 4 : 8;
    if (new_high_score) {
        while (position > 0 && high_score[position - 1] < pl.score)
            --position;
        if (position >= JILL_HIGH_COUNT) {
            new_high_score = 0;
        } else {
            for (index = JILL_HIGH_COUNT - 2; index >= position; --index) {
                high_score[index + 1] = high_score[index];
                strcpy(high_name[index + 1], high_name[index]);
            }
            high_score[position] = pl.score;
            high_name[position][0] = '\0';
        }
    }

    fontcolor(cmdvp, 5, background);
    clearvp(cmdvp);
    wprint(cmdvp, 0, 4, 1, "____________");
    fontcolor(cmdvp, 4, background);
    wprint(cmdvp, 4, 2, 2, "HI SCORES");
    fontcolor(cmdvp, 2, background);
    for (index = 0; index < JILL_HIGH_COUNT; ++index)
        wprint(cmdvp, 2, index * 7 + 15, 2, high_name[index]);

    fontcolor(cmdvp, 6, background);
    for (index = 0; index < JILL_HIGH_COUNT; ++index) {
        _ltoa((longword)high_score[index], string, 10);
        for (character = 0; string[character] != '\0'; ++character)
            drawshape(cmdvp, 0x03d0 + (byte)string[character],
                      62 - (int)strlen(string) * 4 + character * 4,
                      index * 7 + 15);
    }

    if (new_high_score) {
        fontcolor(cmdvp, 7, background);
        _itoa((word)high_score[position], string, 10);
        max_length = (54 - (int)strlen(string) * 4) / 6;
        if (max_length >= 12) max_length = 12;
        winput(cmdvp, 2, position * 7 + 15, 2,
               high_name[position], max_length);
        if (high_name[position][0] == '\0')
            (void)loadcfg();
        else
            (void)savecfg();
        printhi(0);
    }
}

int loadsavewin(const char *message, const char *blank_message)
{
    char string[2] = { 0, 0 };
    word oldclock;
    int c;
    int cursor_frame;

    dx1hold = 1;
    dy1hold = 1;
    fire1off = 1;
    fontcolor(cmdvp, 5, 1);
    clearvp(cmdvp);
    wprint(cmdvp, 0, 4, 1, "____________");
    wprint(cmdvp, 0, 56, 1, "____________");
    fontcolor(cmdvp, 4, 1);
    wprint(cmdvp, 6, 2, 2, message);
    fontcolor(cmdvp, 3, 1);
    for (c = 0; c < JILL_SAVE_COUNT; ++c) {
        _itoa(c + 1, string, 10);
        wprint(cmdvp, 8, c * 8 + 13, 2, string);
    }
    for (c = 0; c < JILL_SAVE_COUNT; ++c)
        wprint(cmdvp, 20, c * 8 + 13, 2,
               save_name[c][0] != '\0' ? save_name[c] : blank_message);
    fontcolor(cmdvp, 2, 1);
    wprint(cmdvp, 14, 65, 2, "PRESS");
    wprint(cmdvp, 6, 77, 2, "TO ABORT");
    fontcolor(cmdvp, 4, 1);
    wprint(cmdvp, 12, 71, 2, "ESCAPE");
    fontcolor(cmdvp, 7, 1);

    cursor_frame = 6;
    do {
        string[1] = '\0';
        checkctrl0(0);
        cursor_frame = (cursor_frame & 7) + 1;
        string[0] = (char)cursor_frame;
        wprint(cmdvp, 1, selected_save * 8 + 13, 2, string);
        oldclock = *myclock;
        while (*myclock == oldclock && host_is_open()) { }
        wprint(cmdvp, 1, selected_save * 8 + 13, 2, " ");
        selected_save += dx1 + dy1;
        if (selected_save > 5) selected_save = 5;
        if (selected_save < 0) selected_save = 0;
    } while (host_is_open() && fire1 == 0 && key != key_enter && key != key_escape);

    if (!host_is_open() || key == key_escape) return -1;
    return selected_save;
}

int loadgame(void)
{
    char savefile[520];
    char mapfile[520];
    char suffix[8];
    FILE *file;
    int slot = loadsavewin("LOAD GAME", "<empty>");
    if (slot < 0 || save_name[slot][0] == '\0') return 0;

    _itoa(slot, suffix, 10);
    data_path(savefile, sizeof(savefile), "jn1save");
    strcat(savefile, ".");
    strcat(savefile, suffix);
    data_path(mapfile, sizeof(mapfile), "jn1save");
    strcat(mapfile, "m.");
    strcat(mapfile, suffix);

    file = fopen(mapfile, "rb");
    if (file != NULL) {
        fclose(file);
        (void)remove(tempname);
        (void)copyfile(mapfile, tempname);
    }
    loadboard(savefile);
    return 1;
}

void savegame(void)
{
    char savefile[520];
    char mapfile[520];
    char suffix[8];
    char name[JILL_SAVE_NAME_LEN];
    FILE *file;
    int slot = loadsavewin("SAVE GAME", "");
    if (slot < 0) return;

    strcpy(name, save_name[slot]);
    winput(cmdvp, 20, slot * 8 + 13, 2, name, 7);
    if (key == key_escape || name[0] == '\0') return;
    strcpy(save_name[slot], name);

    _itoa(slot, suffix, 10);
    data_path(savefile, sizeof(savefile), "jn1save");
    strcat(savefile, ".");
    strcat(savefile, suffix);
    data_path(mapfile, sizeof(mapfile), "jn1save");
    strcat(mapfile, "m.");
    strcat(mapfile, suffix);

    saveboard(savefile);
    file = fopen(tempname, "rb");
    if (file != NULL) {
        fclose(file);
        (void)copyfile(tempname, mapfile);
    }
    savecfg();
}

void drawgamewin(void)
{
    clearvp(&mainvp);
    drawwin(&ourwin);
    fontcolor(cmdvp, 7, 8);
    clearvp(cmdvp);
    fontcolor(statvp, 7, 8);
    clearvp(statvp);
    fontcolor(gamevp, 7, 8);
    clearvp(gamevp);
    fontcolor(&ourwin.border, xbordercol, -1);
    titlewin(&ourwin, "Jill of the Jungle");
    titlebot(&ourwin, "INVENTORY");
    titletop(&ourwin, "CONTROLS");
}
void noisemaker(void)
{
    static const char noise_keys[] =
        "1234567890-=QWERTYUIOP[]ASDFGHJKL;'ZXCVBNM,./\1\2\3\4\5\6";
    int c;

    fire1off = 0;
    do {
        sb_update();
        checkctrl0(0);
        key = (word)toupper(key);
        for (c = 0; noise_keys[c] != '\0'; ++c)
            if ((byte)noise_keys[c] == key)
                snd_play(1, c + 1);
    } while (host_is_open() && key != key_enter && key != key_escape);
}

void pageview(int page)
{
    vptype saved_viewport;
    int n;
    int c;
    uword start_clock;

    scrnxs = fullxs;
    scrnys = fullys;
    do {
        n = -1;
        setpagemode(1);
        fout();
        for (c = 0; c < numobjs; ++c)
            if (objs[c].objkind == obj_checkpt &&
                objs[c].counter == page)
                n = c;

        if (n > 0) {
            saved_viewport = *gamevp;
            *gamevp = mainvp;
            gamevp->vpox = objs[n].x;
            gamevp->vpoy = objs[n].y;
            drawboard();
            pageflip();
            setpagemode(0);
            fin();
            *gamevp = saved_viewport;

            if (page == 99) {
                noisemaker();
            } else {
                start_clock = *myclock;
                fire1off = 1;
                do {
                    checkctrl0(0);
                    sb_update();
                } while (host_is_open() &&
                         ((key == 0 && fire1 == 0) ||
                          (word)(*myclock - start_clock) < 18));
            }

            if (objs[n].xd != 0)
                page = objs[n].xd;
            else
                n = -1;
        }
    } while (n > 0 && host_is_open());

    scrnxs = normxs;
    scrnys = normys;
    setpagemode(1);
    fout();
    drawgamewin();
    drawboard();
    pageflip();
    setpagemode(0);
    fin();
}
int domenu(const char *message, const char *key_table, int first_choice_line,
           int choices, int timeout, int indent, int window_x, int window_width)
{
    wintype menu;
    char line[80];
    word timeout_clock;
    word move_clock = 0;
    int accept;
    int table_index;
    int previous_choice = 1;
    int choice = 0;
    int flash = 0;
    int line_number;
    int cursor_x;

    textmsg = (char *)message;
    textmsglen = (int)strlen(textmsg);
    defwin(&menu, window_x, 64, window_width, numlines() / 2 + 1, 0, 0, 2);
    drawwin(&menu);
    setpagemode(1);
    timeout_clock = *myclock;
    for (line_number = numlines() - 1; line_number >= 0; --line_number) {
        int color = getline(line_number, line, 0);
        fontcolor(&menu.inside, color, -1);
        wprint(&menu.inside,
               4 + (line_number >= first_choice_line) * indent,
               line_number * 8 + 8, 2, line);
    }
    pageflip();
    setpagemode(0);
    cursor_x = (indent - 12) & ~7;

    for (;;) {
        if (!host_is_open()) {
            key = 'Q';
            break;
        }
        sb_update();
        if (++flash >= 12) flash = 0;
        if ((flash & 1) != 0 || previous_choice != choice) {
            drawshape(&menu.inside, 0x4709, cursor_x,
                      (first_choice_line + previous_choice) * 8 + 8);
            drawshape(&menu.inside, 0x0201 + (flash >> 1), cursor_x,
                      (first_choice_line + choice) * 8 + 8);
        }
        previous_choice = choice;
        checkctrl0(0);
        key = (word)toupper(key);

        if (dx1 + dy1 != 0 && abs((word)(*myclock - move_clock)) > 1) {
            move_clock = *myclock;
            choice += dx1 + dy1;
            if (choice < 0) choice = 0;
            if (choice > choices - 1) choice = choices - 1;
            timeout_clock = *myclock;
        }
        if ((word)(*myclock - timeout_clock) > 300 && timeout) {
            key = 'D';
            break;
        }

        accept = 0;
        if (key == key_escape) key = 'Q';
        if (key == key_enter || key == key_space || fire1) {
            key = (byte)key_table[choice];
            accept = 1;
        } else {
            for (table_index = 0; table_index < (int)strlen(key_table); ++table_index)
                if ((byte)key_table[table_index] == key) accept = 1;
        }
        if (accept) break;
    }
    return key;
}

int askquit(void)
{
    snd_play(1, 22);
    (void)domenu("7REALLY QUIT?\r4YES\r2NO\r", "YN", 1, 2, 0, 20, 13, 6);
    return key == 'Y';
}

void dodemo(void)
{
    setpagemode(1);
    gamecount = 0;
    fout();
    drawgamewin();
    drawcmds();
    demonum = 0;
    macabort = 0;
    do {
        if (demonum != 0) {
            fout();
            setpagemode(1);
        }
        if (demolvl[demonum] == 0) demonum = 0;
        loadboard(demoboard[demonum]);
        pl.level = demolvl[demonum];
        p_reenter(0);
        drawboard();
        pageflip();
        setpagemode(0);
        fin();
        playmac(demoname[demonum]);
        if (macplay) {
            play(1);
            stopmac();
            ++demonum;
        } else {
            macaborted = 1;
        }
    } while (!macaborted && host_is_open());
}

void jmenu(void)
{
    int quit = 0;
    int c;

    loadboard("intro.jn1");
    while (host_is_open() && !quit) {
        setpagemode(1);
        setorigin();
        drawboard();
        printhi(0);
        clearvp(&botvp);
        fontcolor(statvp, 7, 8);
        clearvp(statvp);
        if (facetable != 0 && x_ourmode == x_vga) {
            for (c = 0; c < 16; ++c)
                drawshape(statvp,
                          mod_virtual + facetable * 256 + c,
                          (c & 3) * 16, (c >> 2) * 16);
        } else {
            wprint(statvp, 0, 28, 2, "");
            wprint(statvp, 0, 36, 2, "");
        }
        pageflip();
        setpagemode(0);

        (void)domenu("7PICK A CHOICE:\r"
                     "2PLAY\r"
                     "2RESTORE\r"
                     "5STORY\r"
                     "5INSTRUCTIONS\r"
                     "5ORDERING INFO\r"
                     "5CREDITS\r"
                     "3DEMO\r"
                     "3NOISEMAKER\r"
                     "6EPIC'S BBS\r"
                     "4QUIT\r",
                     "PRSIOCDNEQ", 1, 10, 1, 24, 9, 8);
        setpagemode(0);

        if (key == key_escape || key == 'Q') {
            quit = 1;
        } else if (key == 'P') {
            gamecount = 0;
            setpagemode(1);
            fout();
            drawgamewin();
            drawcmds();
            loadboard("map.jn1");
            pl.level = 127;
            p_reenter(0);
            drawboard();
            pageflip();
            setpagemode(0);
            fin();
            play(0);
            sb_playtune("funky.ddt");
            loadboard("intro.jn1");
        } else if (key == 0x10) {
            setpagemode(1);
            drawgamewin();
            drawboard();
            drawcmds();
            pageflip();
            setpagemode(0);
            dolevelsong();
            curlevel[0] = '\0';
            play(0);
        } else if (key == 'R') {
            if (loadgame()) {
                setpagemode(1);
                fout();
                drawgamewin();
                drawcmds();
                setorigin();
                drawboard();
                dolevelsong();
                pageflip();
                setpagemode(0);
                fin();
                play(0);
                sb_playtune("funky.ddt");
                loadboard("intro.jn1");
            }
        } else if (key == 'S') {
            pageview(0);
        } else if (key == 'I') {
            dotextmsg(1);
        } else if (key == 'O' || key == 'H') {
            pageview(8);
        } else if (key == 'C') {
            pageview(12);
        } else if (key == 'E') {
            pageview(20);
        } else if (key == 'D') {
            dodemo();
            sb_playtune("funky.ddt");
            loadboard("intro.jn1");
        } else if (key == 'N') {
            pageview(99);
        } else if (key == 5) {
            setpagemode(1);
            drawgamewin();
            drawboard();
            pageflip();
            setpagemode(0);
            design();
        }
    }
}

void rexit(int result)
{
    savecfg();
    snd_exit();
    shm_exit();
    gc_exit();
    gr_exit();
    host_close();
    exit(result);
}

int main(int argc, char **argv)
{
    char shape_path[520], sound_path[520];
    byte old_palette[JILL_PALETTE_SIZE * 3];
    const char *level = NULL;
    int validate_only = 0;
    int window_scale = 3;
    int index;

    host_start_clock();
    for (index = 1; index < argc; ++index) {
        if (_stricmp(argv[index], "--validate") == 0) validate_only = 1;
        else if (_strnicmp(argv[index], "--scale=", 8) == 0) window_scale = atoi(argv[index] + 8);
        else if (_stricmp(argv[index], "--play") == 0) { }
        else if (argv[index][0] != '/' && argv[index][0] != '-') level = argv[index];
    }
    cfg_getpath(argc, argv);
    data_path(tempname, sizeof(tempname), "temp");
    (void)loadcfg();
    cfg_init(argc, argv);
    data_path(shape_path, sizeof(shape_path), "JILL1.SHA");
    data_path(sound_path, sizeof(sound_path), "JILL1.VCL");
    snd_init(sound_path);
    gc_init();

    /* The native window is the Win32 keyboard transport.  Open it before the
       original configuration dialog so getkey() receives the same keystrokes
       that DOS read from the BIOS.  --validate is a host-only test path. */
    if (!validate_only &&
        !host_open("Jill of the Jungle - Press Enter to accept configuration", window_scale)) {
        fprintf(stderr, "Jill recovery: unable to create the game window.\n");
        snd_exit();
        gc_exit();
        return 3;
    }
    if (!validate_only && !doconfig()) {
        snd_exit();
        gc_exit();
        host_close();
        return 0;
    }

    if (!validate_only)
        host_set_title("Jill of the Jungle - Arrows move, Shift jumps, Alt fires");

    gr_init();
    clrpal();
    if (!validate_only) (void)savecfg();
    (void)shm_init(shape_path);

    if (validate_only) {
        set_game_layout();
        initinfo();
        initobjinfo();
        initboard();
        initobjs();
        if (level == NULL) level = "intro.jn1";
        loadboard(level);
        printf("Jill recovery: loaded %s (%d objects, level %d, score %lu)\n",
               curlevel, (int)numobjs, (int)pl.level, (unsigned long)pl.score);
        snd_exit(); shm_exit(); gc_exit(); gr_exit(); host_close();
        return 0;
    }

    shm_want[1] = 1;
    shm_want[2] = 1;
    shm_want[7] = 1;
    shm_do();
    fontcolor(&mainvp, 9, 0);
    memcpy(old_palette, vgapal, sizeof(old_palette));
    pleasewait();
    snd_do();
    sb_playtune("funky.ddt");
    shm_want[3] = 1;
    shm_want[4] = 1;
    shm_want[5] = 1;
    shm_want[6] = 1;
    shm_want[8] = 1;
    shm_want[14] = 1;
    shm_do();
    fout();
    memcpy(vgapal, old_palette, sizeof(old_palette));

    set_game_layout();
    initinfo();
    initobjinfo();
    initboard();
    initobjs();
    if (level != NULL) {
        loadboard(level);
        setpagemode(1);
        drawgamewin();
        drawcmds();
        setorigin();
        drawboard();
        pageflip();
        setpagemode(0);
        fin();
        play(0);
    } else if (xdemoflag) {
        dodemo();
    } else {
        setpagemode(1);
        drawgamewin();
        pageflip();
        setpagemode(0);
        fin();
        jmenu();
    }
    fout();
    undrawwin(&ourwin);
    snd_exit(); shm_exit(); gc_exit(); gr_exit(); host_close();
    return 0;
}
