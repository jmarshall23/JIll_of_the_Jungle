/* HOSTAUDIO.C - OpenAL transport for recovered WORX digital and music calls. */
#include "HOSTAUDIO.H"
#include "CMFOPL.H"

#include <AL/al.h>
#include <AL/alc.h>
#include <SDL.h>
#include <timidity.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MUSIC_RATE          44100
#define MUSIC_CHANNELS          2
#define MUSIC_BUFFER_FRAMES  4096
#define MUSIC_BUFFER_COUNT      4

/* snd_do allocates 0x7800 bytes for four VOC cache entries spaced 0x1800
   bytes apart.  The final 0x1800 bytes are deliberate spill capacity for
   the shipped long effects (the largest block is 0x2cfe bytes). */
#define VOC_CACHE_STRIDE       0x1800UL
#define VOC_HEADER_SIZE          0x20UL
#define VOC_BLOCK_LIMIT (VOC_CACHE_STRIDE * 2UL - VOC_HEADER_SIZE)

typedef struct music_job {
    cmf_opl_player *player;
} music_job;

static ALCdevice *audio_device;
static ALCcontext *audio_context;
static ALuint voc_source;
static ALuint voc_buffer;
static int voc_source_ready;
static int voc_play_volume = 127;

static SDL_Thread *music_thread;
static SDL_atomic_t music_stop_requested;
static SDL_atomic_t music_playing;
static SDL_atomic_t music_gain_milli;

static byte master_left = 15;
static byte master_right = 15;
static int audio_started;
static int digital_available;
static int music_available;
static int timidity_started;

static uword read_u16(const byte *data)
{
    return (uword)(data[0] | ((uword)data[1] << 8));
}

static int make_audio_context_current(void)
{
    return audio_context != NULL && alcMakeContextCurrent(audio_context) == ALC_TRUE;
}

static int timidity_config_path(char *path, size_t capacity)
{
    static const char suffix[] = "src/thirdparty/freepats/crude.cfg";
    char *base = SDL_GetBasePath();
    size_t base_length;

    if (base == NULL || capacity == 0) {
        SDL_free(base);
        return 0;
    }
    base_length = strlen(base);
    if (base_length + sizeof(suffix) > capacity) {
        SDL_free(base);
        return 0;
    }
    memcpy(path, base, base_length);
    memcpy(path + base_length, suffix, sizeof(suffix));
    SDL_free(base);
    return 1;
}

static ALfloat voc_gain(void)
{
    unsigned master = (unsigned)master_left + master_right;
    return (ALfloat)voc_play_volume / 127.0f * (ALfloat)master / 30.0f;
}

static int fill_music_buffer(music_job *job, ALuint buffer)
{
    int16_t pcm[MUSIC_BUFFER_FRAMES * MUSIC_CHANNELS];
    size_t frames = cmf_opl_read(job->player, pcm, MUSIC_BUFFER_FRAMES);

    if (frames == 0) return 0;
    alBufferData(buffer, AL_FORMAT_STEREO16, pcm,
                 (ALsizei)(frames * MUSIC_CHANNELS * sizeof(pcm[0])),
                 MUSIC_RATE);
    return alGetError() == AL_NO_ERROR;
}

static int SDLCALL music_thread_proc(void *parameter)
{
    music_job *job = (music_job *)parameter;
    ALuint source = 0;
    ALuint buffers[MUSIC_BUFFER_COUNT] = { 0, 0, 0, 0 };
    int queued = 0;
    int index;

    (void)SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);
    (void)make_audio_context_current();
    while (alGetError() != AL_NO_ERROR) { }
    alGenSources(1, &source);
    alGenBuffers(MUSIC_BUFFER_COUNT, buffers);
    if (alGetError() != AL_NO_ERROR || source == 0) goto done;

    alSourcef(source, AL_GAIN,
              (ALfloat)SDL_AtomicGet(&music_gain_milli) / 1000.0f);
    for (index = 0; index < MUSIC_BUFFER_COUNT; ++index) {
        if (!fill_music_buffer(job, buffers[index])) break;
        alSourceQueueBuffers(source, 1, &buffers[index]);
        ++queued;
    }
    if (queued == 0 || alGetError() != AL_NO_ERROR) goto done;

    alSourcePlay(source);
    SDL_AtomicSet(&music_playing, 1);
    while (!SDL_AtomicGet(&music_stop_requested)) {
        ALint processed = 0;
        ALint state = AL_STOPPED;

        alSourcef(source, AL_GAIN,
                  (ALfloat)SDL_AtomicGet(&music_gain_milli) / 1000.0f);
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        while (processed-- > 0) {
            ALuint buffer;
            alSourceUnqueueBuffers(source, 1, &buffer);
            --queued;
            if (fill_music_buffer(job, buffer)) {
                alSourceQueueBuffers(source, 1, &buffer);
                ++queued;
            }
        }
        if (queued == 0) break;
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) alSourcePlay(source);
        SDL_Delay(5);
    }

done:
    SDL_AtomicSet(&music_playing, 0);
    if (source != 0) {
        ALint count = 0;
        alSourceStop(source);
        alGetSourcei(source, AL_BUFFERS_QUEUED, &count);
        while (count-- > 0) {
            ALuint buffer;
            alSourceUnqueueBuffers(source, 1, &buffer);
        }
        alDeleteSources(1, &source);
    }
    alDeleteBuffers(MUSIC_BUFFER_COUNT, buffers);
    cmf_opl_destroy(job->player);
    free(job);
    return 0;
}

int host_audio_start(void)
{
    char config_path[1024];

    if (audio_started) return digital_available || music_available;
    audio_started = 1;
    SDL_AtomicSet(&music_gain_milli, 1000);
    audio_device = alcOpenDevice(NULL);
    if (audio_device == NULL) return 0;
    audio_context = alcCreateContext(audio_device, NULL);
    if (audio_context == NULL || !make_audio_context_current()) {
        if (audio_context != NULL) alcDestroyContext(audio_context);
        alcCloseDevice(audio_device);
        audio_context = NULL;
        audio_device = NULL;
        return 0;
    }

    while (alGetError() != AL_NO_ERROR) { }
    alGenSources(1, &voc_source);
    if (alGetError() != AL_NO_ERROR || voc_source == 0) return 0;
    voc_source_ready = 1;
    alSourcei(voc_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSource3f(voc_source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    digital_available = 1;
    music_available = 1;

    /* TiMidity remains the vendored renderer for genuine MIDI input. */
    if (timidity_config_path(config_path, sizeof(config_path)) &&
        mid_init(config_path) == 0) {
        timidity_started = 1;
    }
    return 1;
}

void host_audio_stop(void)
{
    host_audio_stop_voc();
    host_audio_stop_music();
    if (timidity_started) mid_exit();
    timidity_started = 0;
    music_available = 0;
    digital_available = 0;
    if (voc_source_ready) alDeleteSources(1, &voc_source);
    voc_source = 0;
    voc_source_ready = 0;
    if (audio_context != NULL) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(audio_context);
    }
    if (audio_device != NULL) alcCloseDevice(audio_device);
    audio_context = NULL;
    audio_device = NULL;
    audio_started = 0;
}

int host_audio_digital_available(void)
{
    if (!audio_started) (void)host_audio_start();
    return digital_available;
}

int host_audio_music_available(void)
{
    if (!audio_started) (void)host_audio_start();
    return music_available;
}

void host_audio_set_master_volume(byte left, byte right)
{
    master_left = left > 15 ? 15 : left;
    master_right = right > 15 ? 15 : right;
    if (voc_source_ready) alSourcef(voc_source, AL_GAIN, voc_gain());
}

void host_audio_set_music_volume(byte left, byte right)
{
    unsigned clipped_left = left > 15 ? 15 : left;
    unsigned clipped_right = right > 15 ? 15 : right;
    SDL_AtomicSet(&music_gain_milli,
        (int)((clipped_left + clipped_right) * 1000U / 30U));
}

void host_audio_stop_voc(void)
{
    if (!voc_source_ready) return;
    alSourceStop(voc_source);
    alSourcei(voc_source, AL_BUFFER, 0);
    if (voc_buffer != 0) alDeleteBuffers(1, &voc_buffer);
    voc_buffer = 0;
}

int host_audio_voc_playing(void)
{
    ALint state;
    if (!voc_source_ready) return 0;
    alGetSourcei(voc_source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

int host_audio_play_voc(const byte *voc, int volume)
{
    size_t block_offset;
    ulongword block_length;
    size_t sample_count;
    unsigned sample_rate;

    if (!host_audio_digital_available() || voc == NULL ||
        memcmp(voc, "Creative Voice File", 19) != 0) return 0;
    block_offset = read_u16(voc + 20);
    if (block_offset < 26 || voc[block_offset] != 1) return 0;
    block_length = (ulongword)voc[block_offset + 1]
        | ((ulongword)voc[block_offset + 2] << 8)
        | ((ulongword)voc[block_offset + 3] << 16);
    if (block_length <= 2 || block_length > VOC_BLOCK_LIMIT ||
        voc[block_offset + 5] != 0) return 0;
    sample_count = (size_t)block_length - 2;
    sample_rate = 1000000U / (256U - voc[block_offset + 4]);
    if (sample_rate < 4000 || sample_rate > 48000) return 0;

    host_audio_stop_voc();
    while (alGetError() != AL_NO_ERROR) { }
    alGenBuffers(1, &voc_buffer);
    alBufferData(voc_buffer, AL_FORMAT_MONO8, voc + block_offset + 6,
                 (ALsizei)sample_count, (ALsizei)sample_rate);
    voc_play_volume = volume < 0 ? 0 : (volume > 127 ? 127 : volume);
    alSourcef(voc_source, AL_GAIN, voc_gain());
    alSourcei(voc_source, AL_BUFFER, (ALint)voc_buffer);
    alSourcePlay(voc_source);
    if (alGetError() != AL_NO_ERROR) {
        host_audio_stop_voc();
        return 0;
    }
    return 1;
}

void host_audio_stop_music(void)
{
    SDL_AtomicSet(&music_stop_requested, 1);
    if (music_thread != NULL) {
        SDL_WaitThread(music_thread, NULL);
    }
    music_thread = NULL;
    SDL_AtomicSet(&music_playing, 0);
}

int host_audio_play_cmf(const byte *cmf, size_t length, int loop)
{
    cmf_opl_player *player;
    music_job *job;

    if (!host_audio_music_available()) return 0;
    player = cmf_opl_create(cmf, length, MUSIC_RATE, loop);
    if (player == NULL) return 0;
    host_audio_stop_music();
    job = (music_job *)malloc(sizeof(*job));
    if (job == NULL) {
        cmf_opl_destroy(player);
        return 0;
    }
    job->player = player;
    SDL_AtomicSet(&music_stop_requested, 0);
    music_thread = SDL_CreateThread(music_thread_proc, "Jill music", job);
    if (music_thread == NULL) {
        cmf_opl_destroy(player);
        free(job);
        return 0;
    }
    return 1;
}

int host_audio_music_playing(void)
{
    return SDL_AtomicGet(&music_playing) != 0;
}
