/* SDL2 video, input, timing, and thread transport for the recovered game. */
#include "HOSTSDL.H"
#include "GR.H"
#include "JILL_ICON_DATA.H"
#include "KEYBOARD.H"

#include <SDL.h>

#include <stddef.h>
#include <string.h>

#define HOST_KEY_CAPACITY 512
#define HOST_QUEUE_CAPACITY 64

static SDL_Window *host_window;
static SDL_Renderer *host_renderer;
static SDL_Texture *host_texture;
static SDL_Thread *host_clock_thread;
static SDL_atomic_t host_clock_stop;
static int host_sdl_initialized;
static int host_opened;
static Uint32 host_next_present;
static byte host_keys[HOST_KEY_CAPACITY];
static int host_queue[HOST_QUEUE_CAPACITY];
static unsigned host_queue_read, host_queue_write;
static uint32_t host_pixels[JILL_SCREEN_WIDTH * JILL_SCREEN_HEIGHT];

static int SDLCALL clock_thread_proc(void *parameter)
{
    (void)parameter;
    while (!SDL_AtomicGet(&host_clock_stop)) {
        SDL_Delay(55);
        ++longclock;
        *myclock = (uword)longclock;
    }
    return 0;
}

static void stop_clock_thread(void)
{
    if (host_clock_thread == NULL) return;
    SDL_AtomicSet(&host_clock_stop, 1);
    SDL_WaitThread(host_clock_thread, NULL);
    host_clock_thread = NULL;
}

void host_start_clock(void)
{
    if (host_clock_thread != NULL) return;
    longclock = 0;
    *myclock = 0;
    SDL_AtomicSet(&host_clock_stop, 0);
    host_clock_thread = SDL_CreateThread(clock_thread_proc, "Jill clock", NULL);
}

void host_stop_clock(void)
{
    stop_clock_thread();
}

static int map_sdl_key(SDL_Keycode key)
{
    switch (key) {
    case SDLK_ESCAPE: return key_escape;
    case SDLK_RETURN:
    case SDLK_KP_ENTER: return key_enter;
    case SDLK_SPACE: return key_space;
    case SDLK_LEFT: return key_left;
    case SDLK_RIGHT: return key_right;
    case SDLK_UP: return key_up;
    case SDLK_DOWN: return key_down;
    case SDLK_LCTRL:
    case SDLK_RCTRL: return key_ctrl;
    case SDLK_LALT:
    case SDLK_RALT: return key_alt;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: return key_shift;
    case SDLK_F1: return key_f1;
    case SDLK_F2: return key_f2;
    case SDLK_F3: return key_f3;
    case SDLK_F4: return key_f4;
    case SDLK_F5: return key_f5;
    case SDLK_F9: return key_f9;
    default:
        if (key >= SDLK_a && key <= SDLK_z)
            return 'A' + (int)(key - SDLK_a);
        if (key >= SDLK_0 && key <= SDLK_9)
            return '0' + (int)(key - SDLK_0);
        return 0;
    }
}

static int is_modifier_key(int key_code)
{
    return key_code == key_ctrl || key_code == key_alt || key_code == key_shift;
}

static void queue_key(int key_code)
{
    unsigned next;
    if (key_code == 0) return;
    next = (host_queue_write + 1U) % HOST_QUEUE_CAPACITY;
    if (next == host_queue_read) return;
    host_queue[host_queue_write] = key_code;
    host_queue_write = next;
}

static void render_frame(void)
{
    if (host_renderer == NULL || host_texture == NULL) return;
    SDL_SetRenderDrawColor(host_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(host_renderer);
    SDL_RenderCopy(host_renderer, host_texture, NULL, NULL);
    SDL_RenderPresent(host_renderer);
}

static void set_window_icon(void)
{
    SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormatFrom(
        (void *)jill_icon_rgba, JILL_ICON_WIDTH, JILL_ICON_HEIGHT, 32,
        JILL_ICON_WIDTH * 4, SDL_PIXELFORMAT_RGBA32);
    if (icon != NULL) {
        SDL_SetWindowIcon(host_window, icon);
        SDL_FreeSurface(icon);
    }
}

static void request_close(void)
{
    host_opened = 0;
    if (host_window != NULL) SDL_HideWindow(host_window);
}

static void handle_key_event(const SDL_KeyboardEvent *event)
{
    int key_code = map_sdl_key(event->keysym.sym);
    if (key_code <= 0 || key_code >= HOST_KEY_CAPACITY) return;
    if (event->type == SDL_KEYDOWN) {
        host_keys[key_code] = 1;
        if (!is_modifier_key(key_code) && event->repeat == 0)
            queue_key(key_code);
    } else {
        host_keys[key_code] = 0;
    }
}

static void destroy_video(void)
{
    if (host_texture != NULL) SDL_DestroyTexture(host_texture);
    if (host_renderer != NULL) SDL_DestroyRenderer(host_renderer);
    if (host_window != NULL) SDL_DestroyWindow(host_window);
    host_texture = NULL;
    host_renderer = NULL;
    host_window = NULL;
}

int host_open(const char *title, int scale)
{
    Uint32 renderer_flags = SDL_RENDERER_ACCELERATED;
    if (host_opened) return 1;
    if (scale < 1) scale = 1;
    if (scale > 5) scale = 5;

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0)
        return 0;
    host_sdl_initialized = 1;

    host_window = SDL_CreateWindow(title != NULL ? title : "Jill of the Jungle",
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   JILL_SCREEN_WIDTH * scale,
                                   JILL_SCREEN_HEIGHT * scale,
                                   SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (host_window == NULL) goto failed;
    set_window_icon();

    host_renderer = SDL_CreateRenderer(host_window, -1, renderer_flags);
    if (host_renderer == NULL)
        host_renderer = SDL_CreateRenderer(host_window, -1, SDL_RENDERER_SOFTWARE);
    if (host_renderer == NULL) goto failed;
    if (SDL_RenderSetLogicalSize(host_renderer,
                                 JILL_SCREEN_WIDTH, JILL_SCREEN_HEIGHT) != 0)
        goto failed;

    host_texture = SDL_CreateTexture(host_renderer, SDL_PIXELFORMAT_ARGB8888,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     JILL_SCREEN_WIDTH, JILL_SCREEN_HEIGHT);
    if (host_texture == NULL) goto failed;

    memset(host_pixels, 0, sizeof(host_pixels));
    if (SDL_UpdateTexture(host_texture, NULL, host_pixels,
                          JILL_SCREEN_WIDTH * (int)sizeof(host_pixels[0])) != 0)
        goto failed;
    host_clear_keys();
    host_opened = 1;
    host_next_present = 0;
    host_start_clock();
    render_frame();
    SDL_RaiseWindow(host_window);
    return 1;

failed:
    destroy_video();
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    host_sdl_initialized = 0;
    return 0;
}

void host_close(void)
{
    stop_clock_thread();
    host_opened = 0;
    destroy_video();
    host_clear_keys();
    if (host_sdl_initialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
        host_sdl_initialized = 0;
    }
}

int host_is_open(void)
{
    if (host_opened) (void)host_pump();
    return host_opened;
}

int host_pump(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            request_close();
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handle_key_event(&event.key);
            break;
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                request_close();
            else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                memset(host_keys, 0, sizeof(host_keys));
            else if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
                     event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                render_frame();
            break;
        default:
            break;
        }
    }
    if (host_opened) {
        Uint32 now = SDL_GetTicks();
        if ((Sint32)(now - host_next_present) >= 0) {
            /* DOS drew visible-page changes immediately; refresh that page here. */
            gr_present_page();
            host_next_present = now + 16;
        }
    }
    return host_opened;
}

void host_present(const byte *pixels, const byte *palette)
{
    size_t index;
    if (!host_opened || host_texture == NULL || pixels == NULL || palette == NULL)
        return;
    for (index = 0; index < JILL_SCREEN_WIDTH * JILL_SCREEN_HEIGHT; ++index) {
        unsigned color = pixels[index];
        unsigned red = (unsigned)palette[color * 3] * 255U / 63U;
        unsigned green = (unsigned)palette[color * 3 + 1] * 255U / 63U;
        unsigned blue = (unsigned)palette[color * 3 + 2] * 255U / 63U;
        host_pixels[index] = (uint32_t)(0xff000000U | (red << 16) |
                                        (green << 8) | blue);
    }
    if (SDL_UpdateTexture(host_texture, NULL, host_pixels,
                          JILL_SCREEN_WIDTH * (int)sizeof(host_pixels[0])) == 0)
        render_frame();
}

void host_set_title(const char *title)
{
    if (host_window != NULL)
        SDL_SetWindowTitle(host_window,
                           title != NULL ? title : "Jill of the Jungle");
}

int host_key_pressed(void) { return host_queue_read != host_queue_write; }

int host_peek_key(void)
{
    if (!host_key_pressed()) return 0;
    return host_queue[host_queue_read];
}

int host_read_key(void)
{
    int result;
    if (!host_key_pressed()) return 0;
    result = host_queue[host_queue_read];
    host_queue_read = (host_queue_read + 1U) % HOST_QUEUE_CAPACITY;
    return result;
}

int host_key_down(int key_code)
{
    return key_code > 0 && key_code < HOST_KEY_CAPACITY &&
           host_keys[key_code] != 0;
}

void host_clear_keys(void)
{
    memset(host_keys, 0, sizeof(host_keys));
    host_queue_read = host_queue_write = 0;
}

void host_console_clear(void)
{
    /* GUI-subsystem builds intentionally have no console to clear. */
}

void host_sleep(unsigned milliseconds) { SDL_Delay((Uint32)milliseconds); }

longword host_coreleft(void)
{
    int megabytes = SDL_GetSystemRAM();
    uint64_t bytes;
    if (megabytes <= 0) return 0;
    bytes = (uint64_t)(unsigned)megabytes * 1024U * 1024U;
    return bytes > 0x7fffffffULL ? 0x7fffffffL : (longword)bytes;
}

uint32_t host_ticks(void) { return (uint32_t)SDL_GetTicks(); }
