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

#ifndef JILL_JOBJ_H
#define JILL_JOBJ_H

/* Recovered Jill object numbers used by the on-disk level format. */
enum jill_object_kind {
    obj_player = 0,
    obj_apple = 1,
    obj_knife = 2,
    obj_killme = 3,
    obj_bigant = 4,
    obj_unused5 = 5,
    obj_fly = 6,
    obj_demon = 7,
    obj_fatso = 8,
    obj_inchworm = 9,
    obj_zapper = 10,
    obj_bobslug = 11,
    obj_checkpt = 12,
    obj_paul = 13,
    obj_key = 14,
    obj_pad = 15,
    obj_wiseman = 16,
    obj_roman = 17,
    obj_enemyshot = 18,
    obj_cloud = 19,
    obj_text6 = 20,
    obj_text8 = 21,
    obj_frog = 22,
    obj_tiny = 23,
    obj_door = 24,
    obj_falldoor = 25,
    obj_bridger = 26,
    obj_score = 27,
    obj_token = 28,
    obj_ant = 29,
    obj_phoenix = 30,
    obj_fire = 31,
    obj_switch = 32,
    obj_gem = 33,
    obj_explode = 34,
    obj_boulder = 35,
    obj_expl1 = 36,
    obj_expl2 = 37,
    obj_stalag = 38,
    obj_snake = 39,
    obj_searock = 40,
    obj_boll = 41,
    obj_mega = 42,
    obj_bat = 43,
    obj_knight = 44,
    obj_beenest = 45,
    obj_beeswarm = 46,
    obj_crab = 47,
    obj_croc = 48,
    obj_epic = 49,
    obj_spinblad = 50,
    obj_skull = 51,
    obj_button = 52,
    obj_pac = 53,
    obj_jillfish = 54,
    obj_unused55 = 55,
    obj_jillbird = 56,
    obj_jillfrog = 57,
    obj_bubble = 58,
    obj_jellyfish = 59,
    obj_badfish = 60,
    obj_elev = 61,
    obj_firebullet = 62,
    obj_fishbullet = 63,
    obj_eyes = 64,
    obj_vineclimb = 65,
    obj_flag = 66,
    obj_txtmsg = 67,
    obj_mapdemo = 68,
    numobjkinds = 69
};

enum jill_inventory_kind {
    inv_jill = 0,
    inv_redkey = 1,
    inv_knife = 2,
    inv_crystal = 3,
    inv_frog = 4,
    inv_bird = 5,
    inv_coins = 6,
    inv_fish = 7,
    inv_blade = 8,
    inv_jump = 9,
    inv_invin = 10
};

void initobjinfo(void);
int msg_null(int n, int msg, int z);
int msg_nullkind(int n, int msg, int z);
int msg_block(int x, int y, int msg);
void explode1(int x, int y, int count);
void explode2(int n);

int msg_apple(int n, int msg, int z);
int msg_knife(int n, int msg, int z);
int msg_bigant(int n, int msg, int z);
int msg_ant(int n, int msg, int z);
int msg_fly(int n, int msg, int z);
int msg_phoenix(int n, int msg, int z);
int msg_inchworm(int n, int msg, int z);
int msg_zapper(int n, int msg, int z);
int msg_bobslug(int n, int msg, int z);
int msg_checkpt(int n, int msg, int z);
int msg_paul(int n, int msg, int z);
int msg_wiseman(int n, int msg, int z);
int msg_bridger(int n, int msg, int z);
int msg_key(int n, int msg, int z);
int msg_pad(int n, int msg, int z);
int msg_demon(int n, int msg, int z);
int msg_fatso(int n, int msg, int z);
int msg_roman(int n, int msg, int z);
int msg_fireball(int n, int msg, int z);
int msg_cloud(int n, int msg, int z);
int msg_text6(int n, int msg, int z);
int msg_score(int n, int msg, int z);
int msg_text8(int n, int msg, int z);
int msg_frog(int n, int msg, int z);
int msg_door(int n, int msg, int z);
int msg_falldoor(int n, int msg, int z);
int msg_token(int n, int msg, int z);
int msg_fire(int n, int msg, int z);
int msg_switch(int n, int msg, int z);
int msg_gem(int n, int msg, int z);
int msg_boulder(int n, int msg, int z);
int msg_expl1(int n, int msg, int z);
int msg_expl2(int n, int msg, int z);
int msg_stalag(int n, int msg, int z);
int msg_snake(int n, int msg, int z);
int msg_boll(int n, int msg, int z);
int msg_mega(int n, int msg, int z);
int msg_bat(int n, int msg, int z);
int msg_knight(int n, int msg, int z);
int msg_beenest(int n, int msg, int z);
int msg_beeswarm(int n, int msg, int z);
int msg_crab(int n, int msg, int z);
int msg_croc(int n, int msg, int z);
int msg_epic(int n, int msg, int z);
int msg_spinblad(int n, int msg, int z);
int msg_skull(int n, int msg, int z);
int msg_button(int n, int msg, int z);
int msg_pac(int n, int msg, int z);
int msg_bubble(int n, int msg, int z);
int msg_jellyfish(int n, int msg, int z);
int msg_badfish(int n, int msg, int z);
int msg_elev(int n, int msg, int z);
int msg_firebullet(int n, int msg, int z);
int msg_fishbullet(int n, int msg, int z);
int msg_searock(int n, int msg, int z);
int msg_eyes(int n, int msg, int z);
int msg_vineclimb(int n, int msg, int z);
int msg_flag(int n, int msg, int z);
int msg_macrotrig(int n, int msg, int z);
int msg_txtmsg(int n, int msg, int z);
int msg_mapdemo(int n, int msg, int z);
int msg_player(int n, int msg, int z);
int msg_tiny(int n, int msg, int z);
int msg_jillfish(int n, int msg, int z);
int msg_jillspider(int n, int msg, int z);
int msg_jillfrog(int n, int msg, int z);
int msg_jillbird(int n, int msg, int z);

#endif
