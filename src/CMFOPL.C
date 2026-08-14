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
#include "CMFOPL.H"

#include <opl3.h>

#include <stdlib.h>
#include <string.h>

#define CMF_CHANNEL_COUNT 16
#define OPL_VOICE_COUNT    9

typedef struct opl_pair {
    uint8_t reg;
    uint8_t value;
} opl_pair;

struct cmf_opl_player {
    opl3_chip chip;
    uint8_t *data;
    size_t length;
    size_t music_offset;
    size_t instrument_offset;
    size_t position;
    uint32_t sample_rate;
    uint16_t ticks_per_second;
    uint16_t instrument_count;
    uint64_t frame_remainder;
    uint64_t frames_to_event;
    uint16_t allocation[OPL_VOICE_COUNT];
    uint8_t current_note[OPL_VOICE_COUNT];
    uint8_t carrier_level[OPL_VOICE_COUNT];
    uint8_t program[CMF_CHANNEL_COUNT];
    uint8_t transpose[CMF_CHANNEL_COUNT];
    uint8_t channel_volume[CMF_CHANNEL_COUNT];
    uint8_t running_status;
    uint8_t rhythm_register;
    uint8_t rhythm_compatibility;
    uint8_t event_pending;
    uint8_t ended;
    uint8_t loop;
};

/* CS:075E, CS:0776, and CS:077F in the recovered JVOL driver. */
static const uint16_t worx_f_numbers[12] = {
    0x016b, 0x0181, 0x0198, 0x01b0, 0x01ca, 0x01e5,
    0x0202, 0x0220, 0x0241, 0x0263, 0x0287, 0x02ae
};
static const uint8_t worx_modulator[OPL_VOICE_COUNT] = {
    0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12
};
static const uint8_t worx_carrier[OPL_VOICE_COUNT] = {
    0x03, 0x04, 0x05, 0x0b, 0x0c, 0x0d, 0x13, 0x14, 0x15
};

/* CS:06C3: General-MIDI percussion notes 35 through 68. */
static const uint8_t worx_rhythm_note_bits[34] = {
    0x10,0x10,0x01,0x08,0x01,0x08,0x10,0x01,0x10,0x01,0x04,0x01,
    0x04,0x04,0x02,0x04,0x02,0x01,0x01,0x04,0x04,0x01,0x04,0x04,
    0x04,0x04,0x04,0x04,0x01,0x01,0x01,0x01,0x04,0x04
};

/*
 * CS:06E5, consumed by sub_2EC5A when controller 103 enables rhythm.
 * These are the exact register/value pairs embedded in JILL.EXE.
 */
static const opl_pair worx_rhythm_setup[] = {
    {0xbd,0xe0},{0x08,0x00},
    {0xa6,0x38},{0xb6,0x09},{0x50,0x0b},{0xc6,0x00},{0x70,0xa8},
    {0x90,0x4c},{0x30,0x00},{0xf0,0x00},{0x53,0x04},{0x73,0xd6},
    {0x93,0x4f},{0x33,0x00},{0xf3,0x00},{0xbd,0xe0},{0x08,0x00},
    {0xbd,0xe0},{0x08,0x00},
    {0xa7,0x03},{0xb7,0x0a},{0x51,0x0d},{0xc7,0x01},{0x71,0xf9},
    {0x91,0x8c},{0x31,0x01},{0xf1,0x00},{0xbd,0xe0},{0x08,0x00},
    {0xbd,0xe0},{0x08,0x00},
    {0xa8,0x57},{0xb8,0x09},{0x52,0x00},{0xc8,0x01},{0x72,0xf7},
    {0x92,0xb5},{0x32,0x04},{0xf2,0x00},{0xbd,0xe0},{0x08,0x00},
    {0xbd,0xe0},{0x08,0x00},
    {0x54,0x07},{0x74,0xf7},{0x94,0xbf},{0x34,0x0c},{0xf4,0x00},
    {0xbd,0xe0},{0x08,0x00},{0xbd,0xe0},{0x08,0x00},
    {0x55,0x0d},{0x75,0xf5},{0x95,0xa5},{0x35,0x01},{0xf5,0x00},
    {0xbd,0xe0},{0x08,0x00}
};

static uint16_t cmf_u16(const uint8_t *data)
{
    return (uint16_t)(data[0] | ((uint16_t)data[1] << 8));
}

static void opl_write(cmf_opl_player *player, uint8_t reg, uint8_t value)
{
    OPL3_WriteRegBuffered(&player->chip, reg, value);
}

static const uint8_t *instrument(const cmf_opl_player *player,
                                 uint8_t number)
{
    if (number >= player->instrument_count) number = 0;
    return player->data + player->instrument_offset + (size_t)number * 16U;
}

/* Direct translation of sub_2EB06. */
static void load_instrument(cmf_opl_player *player, uint8_t voice,
                            uint8_t number)
{
    static const uint8_t bases[5] = { 0x20, 0x40, 0x60, 0x80, 0xe0 };
    const uint8_t *patch = instrument(player, number);
    unsigned pair;

    for (pair = 0; pair < 5; ++pair) {
        opl_write(player, (uint8_t)(bases[pair] + worx_modulator[voice]),
                  patch[pair * 2U]);
        opl_write(player, (uint8_t)(bases[pair] + worx_carrier[voice]),
                  patch[pair * 2U + 1U]);
    }
    player->carrier_level[voice] = patch[3];
    opl_write(player, (uint8_t)(0xc0U + voice), patch[10]);
}

/* Direct translation of sub_2F096. */
static void reset_worx_state(cmf_opl_player *player)
{
    unsigned voice;

    player->rhythm_register = 0xc0;
    opl_write(player, 0xbd, player->rhythm_register);
    for (voice = 0; voice < OPL_VOICE_COUNT; ++voice)
        load_instrument(player, (uint8_t)voice, 0);
    memset(player->program, 0, sizeof(player->program));
    memset(player->transpose, 0, sizeof(player->transpose));
    memset(player->current_note, 0, sizeof(player->current_note));
    for (voice = 0; voice < OPL_VOICE_COUNT; ++voice)
        player->allocation[voice] = 0xffffU;
}

/* Direct translation of sub_2EBF1 for the OPL path. */
static void all_notes_off(cmf_opl_player *player, uint8_t channel)
{
    unsigned voice;

    for (voice = 0; voice < OPL_VOICE_COUNT; ++voice) {
        if ((uint8_t)(player->allocation[voice] >> 8) == channel) {
            opl_write(player, (uint8_t)(0xa0U + voice), 0);
            opl_write(player, (uint8_t)(0xb0U + voice), 0);
            player->current_note[voice] = 0;
        }
    }
}

/* Direct translation of the OPL branch in sub_2EC5A. */
static void controller(cmf_opl_player *player, uint8_t channel,
                       uint8_t number, uint8_t value)
{
    unsigned index;

    if (number == 0x7b) {
        all_notes_off(player, channel);
        return;
    }
    if (number == 7) {
        player->channel_volume[channel] = value;
        return;
    }
    if (number == 0x67) {
        if (value == 0) {
            player->rhythm_register = 0xc0;
            player->rhythm_compatibility = 0;
            opl_write(player, 0xbd, player->rhythm_register);
            player->allocation[6] = 0xffffU;
            player->allocation[7] = 0xffffU;
            player->allocation[8] = 0xffffU;
        } else if (value == 1) {
            player->rhythm_register = 0xe0;
            player->rhythm_compatibility = 1;
            opl_write(player, 0xbd, player->rhythm_register);
            player->allocation[6] = 0x10ffU;
            player->allocation[7] = 0x10ffU;
            player->allocation[8] = 0x10ffU;
            for (index = 0;
                 index < sizeof(worx_rhythm_setup) / sizeof(worx_rhythm_setup[0]);
                 ++index) {
                opl_write(player, worx_rhythm_setup[index].reg,
                          worx_rhythm_setup[index].value);
            }
            opl_write(player, 0xbd, 0xe0);
            opl_write(player, 0x08, 0);
            player->rhythm_register = 0xe0;
        }
        return;
    }
    if (number == 0x68 || number == 0x69) {
        /*
         * The shipped driver clears BH before comparing it with 0x68 at
         * 2ED07, so both controllers store the unmodified seven-bit value.
         */
        player->transpose[channel] = value;
    }
}

/* Direct translation of sub_2EF82. */
static void write_note(cmf_opl_player *player, uint8_t event_type,
                       uint8_t channel, uint8_t note, uint8_t velocity,
                       uint8_t voice)
{
    uint8_t note_minus_one = (uint8_t)(note - 1U);
    uint16_t octave = (uint16_t)(note_minus_one / 12U);
    uint8_t pitch_class = (uint8_t)(note_minus_one % 12U);
    uint16_t frequency = worx_f_numbers[pitch_class];
    uint8_t bend = player->transpose[channel];
    uint8_t b_value;
    uint8_t level;
    uint8_t loudness;
    uint8_t volume;
    uint16_t product;

    player->current_note[voice] = note;
    if (bend != 0) {
        uint16_t shifted = (uint16_t)((uint16_t)(bend * 0x16U) << 1);
        uint8_t delta = (uint8_t)(shifted >> 8);
        if (bend == 0x80U)
            frequency = (uint16_t)(frequency + delta);
        else
            frequency = (uint16_t)(frequency - delta);
    }

    opl_write(player, (uint8_t)(0xa0U + voice), (uint8_t)frequency);
    b_value = (uint8_t)((octave << 2) | ((frequency >> 8) & 3U));
    if (event_type != 0x80U && velocity != 0 &&
        ((player->rhythm_register & 0x20U) == 0 || channel < 0x0bU))
        b_value |= 0x20U;
    opl_write(player, (uint8_t)(0xb0U + voice), b_value);

    level = player->carrier_level[voice];
    loudness = (uint8_t)(0x3fU - (level & 0x3fU));
    volume = player->channel_volume[channel];
    if (volume >= 0x60U) volume = 0x5fU;
    volume = (uint8_t)((uint8_t)(volume + 0x20U) << 1);
    product = (uint16_t)(loudness * volume);
    level = (uint8_t)((level & 0xc0U) |
        (uint8_t)(0x3fU - (uint8_t)((product >> 8) + 1U)));
    opl_write(player, (uint8_t)(0x40U + worx_carrier[voice]), level);
}

/* Direct translation of sub_2EE24. */
static void note_event(cmf_opl_player *player, uint8_t event_type,
                       uint8_t channel, uint8_t note, uint8_t velocity)
{
    unsigned voice_count = (player->rhythm_register & 0x20U) ? 6U : 9U;
    unsigned voice;
    int selected = -1;

    if ((player->rhythm_register & 0x20U) != 0) {
        if (player->rhythm_compatibility && channel == 9U) {
            if (note >= 35U && note <= 68U) {
                player->rhythm_register ^= worx_rhythm_note_bits[note - 35U];
                opl_write(player, 0xbd, player->rhythm_register);
            }
            return;
        }
        if (channel >= 0x0bU) {
            if (channel <= 0x0fU) {
                player->rhythm_register ^= (uint8_t)(0x10U >> (channel - 0x0bU));
                opl_write(player, 0xbd, player->rhythm_register);
            }
            return;
        }
    }

    if (event_type == 0x80U || velocity == 0) {
        for (voice = 0; voice < voice_count; ++voice) {
            if ((uint8_t)(player->allocation[voice] >> 8) == channel &&
                player->current_note[voice] == note) {
                write_note(player, event_type, channel, note, velocity,
                           (uint8_t)voice);
                player->current_note[voice] = 0;
                return;
            }
        }
        return;
    }

    for (voice = 0; voice < voice_count; ++voice) {
        if ((uint8_t)(player->allocation[voice] >> 8) == channel &&
            player->current_note[voice] == 0) {
            selected = (int)voice;
            break;
        }
    }
    if (selected < 0) {
        for (voice = 0; voice < voice_count; ++voice) {
            if ((uint8_t)(player->allocation[voice] >> 8) == 0xffU &&
                player->current_note[voice] == 0) {
                selected = (int)voice;
                break;
            }
        }
    }
    if (selected < 0) {
        for (voice = 0; voice < voice_count; ++voice) {
            if (player->current_note[voice] == 0) {
                selected = (int)voice;
                break;
            }
        }
    }
    if (selected < 0) return;

    voice = (unsigned)selected;
    if ((uint8_t)player->allocation[voice] != player->program[channel]) {
        player->allocation[voice] =
            (uint16_t)(((uint16_t)channel << 8) | player->program[channel]);
        load_instrument(player, (uint8_t)voice, player->program[channel]);
    }
    write_note(player, event_type, channel, note, velocity, (uint8_t)voice);
}

static int read_varlen(cmf_opl_player *player, uint32_t *value)
{
    uint32_t result = 0;
    unsigned count = 0;
    uint8_t current;

    do {
        if (player->position >= player->length || count == 4) return 0;
        current = player->data[player->position++];
        result = (result << 7) | (current & 0x7fU);
        ++count;
    } while ((current & 0x80U) != 0);
    *value = result;
    return 1;
}

static void restart_sequence(cmf_opl_player *player)
{
    reset_worx_state(player);
    player->position = player->music_offset;
    player->running_status = 0;
    player->event_pending = 0;
    player->ended = 0;
}

static int process_event(cmf_opl_player *player)
{
    uint8_t status;

    if (player->position >= player->length) return 0;
    status = player->data[player->position];
    if ((status & 0x80U) != 0) {
        ++player->position;
        if (status < 0xf0U) player->running_status = status;
    } else {
        if (player->running_status == 0) return 0;
        status = player->running_status;
    }

    if (status < 0xf0U) {
        uint8_t event_type = status & 0xf0U;
        uint8_t channel = status & 0x0fU;
        unsigned data_count =
            (event_type == 0xc0U || event_type == 0xd0U) ? 1U : 2U;
        uint8_t first;
        uint8_t second = 0;

        if (player->position + data_count > player->length) return 0;
        first = player->data[player->position++];
        if (data_count == 2) second = player->data[player->position++];
        if (first >= 0x80U || second >= 0x80U) return 0;

        if (event_type <= 0x90U) {
            if ((player->rhythm_register & 0x20U) == 0 ||
                (player->rhythm_compatibility ? channel != 9U : channel < 0x0bU))
                first = (uint8_t)(first - 12U); /* CS:0996 is F4h. */
            note_event(player, event_type, channel, first, second);
        } else if (event_type <= 0xb0U) {
            controller(player, channel, first, second);
        } else if (event_type == 0xc0U) {
            player->program[channel] = first;
        }
        return 1;
    }

    if (status == 0xffU) {
        uint8_t type;
        uint32_t meta_length;

        if (player->position >= player->length) return 0;
        type = player->data[player->position++];
        if (!read_varlen(player, &meta_length) ||
            meta_length > player->length - player->position) return 0;
        player->position += meta_length;
        if (type == 0x2fU) {
            if (player->loop) {
                restart_sequence(player);
            } else {
                player->ended = 1;
            }
        }
        return 1;
    }
    if (status == 0xf0U || status == 0xf7U) {
        uint32_t sysex_length;
        if (!read_varlen(player, &sysex_length) ||
            sysex_length > player->length - player->position) return 0;
        player->position += sysex_length;
        return 1;
    }
    return 0;
}

static int prepare_audio(cmf_opl_player *player)
{
    unsigned zero_time_events = 0;

    while (!player->ended && player->frames_to_event == 0) {
        uint32_t delta;

        if (player->event_pending) {
            player->event_pending = 0;
            if (!process_event(player)) {
                player->ended = 1;
                break;
            }
            if (++zero_time_events == 10000U) {
                player->ended = 1;
                break;
            }
            continue;
        }
        if (!read_varlen(player, &delta)) {
            player->ended = 1;
            break;
        }
        player->event_pending = 1;
        player->frame_remainder += (uint64_t)delta * player->sample_rate;
        player->frames_to_event =
            player->frame_remainder / player->ticks_per_second;
        player->frame_remainder %= player->ticks_per_second;
    }
    return !player->ended || player->frames_to_event != 0;
}

cmf_opl_player *cmf_opl_create(const uint8_t *cmf, size_t length,
                               uint32_t sample_rate, int loop)
{
    cmf_opl_player *player;
    uint16_t instrument_offset;
    uint16_t music_offset;
    uint16_t instrument_count;
    uint16_t ticks_per_second;

    if (cmf == NULL || length < 40 || memcmp(cmf, "CTMF", 4) != 0 ||
        sample_rate == 0) return NULL;
    instrument_offset = cmf_u16(cmf + 6);
    music_offset = cmf_u16(cmf + 8);
    ticks_per_second = cmf_u16(cmf + 12);
    instrument_count = cmf_u16(cmf + 36);
    if (ticks_per_second == 0 || music_offset >= length ||
        instrument_count == 0 || instrument_offset > length ||
        instrument_count > (length - instrument_offset) / 16U)
        return NULL;

    player = (cmf_opl_player *)calloc(1, sizeof(*player));
    if (player == NULL) return NULL;
    player->data = (uint8_t *)malloc(length);
    if (player->data == NULL) {
        free(player);
        return NULL;
    }
    memcpy(player->data, cmf, length);
    player->length = length;
    player->music_offset = music_offset;
    player->instrument_offset = instrument_offset;
    player->instrument_count = instrument_count;
    player->ticks_per_second = ticks_per_second;
    player->sample_rate = sample_rate;
    player->loop = loop != 0;
    memset(player->channel_volume, 0x7f, sizeof(player->channel_volume));

    OPL3_Reset(&player->chip, sample_rate);
    restart_sequence(player);
    return player;
}

void cmf_opl_destroy(cmf_opl_player *player)
{
    if (player == NULL) return;
    free(player->data);
    free(player);
}

size_t cmf_opl_read(cmf_opl_player *player, int16_t *pcm, size_t frames)
{
    size_t generated = 0;

    if (player == NULL || pcm == NULL) return 0;
    while (generated < frames && prepare_audio(player)) {
        size_t count = frames - generated;
        if ((uint64_t)count > player->frames_to_event)
            count = (size_t)player->frames_to_event;
        if (count == 0) continue;
        OPL3_GenerateStream(&player->chip, pcm + generated * 2U,
                            (uint32_t)count);
        generated += count;
        player->frames_to_event -= count;
    }
    return generated;
}
