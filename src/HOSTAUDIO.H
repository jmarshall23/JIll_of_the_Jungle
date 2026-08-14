#ifndef JILL_HOSTAUDIO_H
#define JILL_HOSTAUDIO_H

#include "RECOVERY.H"

#include <stddef.h>

int host_audio_start(void);
void host_audio_stop(void);
int host_audio_digital_available(void);
int host_audio_music_available(void);

void host_audio_set_master_volume(byte left, byte right);
void host_audio_set_music_volume(byte left, byte right);

int host_audio_play_voc(const byte *voc, int volume);
int host_audio_voc_playing(void);
void host_audio_stop_voc(void);

int host_audio_play_cmf(const byte *cmf, size_t length, int loop);
int host_audio_music_playing(void);
void host_audio_stop_music(void);

#endif
