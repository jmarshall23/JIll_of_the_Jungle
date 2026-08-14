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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>

#include "HOSTWIN.H"
#include "GR.H"
#include "KEYBOARD.H"

#include <string.h>

#define HOST_KEY_CAPACITY 512
#define HOST_QUEUE_CAPACITY 64

static HWND host_window;
static int host_opened;
static byte host_keys[HOST_KEY_CAPACITY];
static int host_queue[HOST_QUEUE_CAPACITY];
static unsigned host_queue_read, host_queue_write;
static uint32_t host_pixels[JILL_SCREEN_WIDTH * JILL_SCREEN_HEIGHT];
static BITMAPINFO host_bitmap;
static HANDLE host_clock_thread;
static volatile LONG host_clock_stop;

static DWORD WINAPI clock_thread_proc(LPVOID parameter)
{
    (void)parameter;
    while (InterlockedCompareExchange(&host_clock_stop, 0, 0) == 0) {
        Sleep(55);
        ++longclock;
        *myclock = (uword)longclock;
    }
    return 0;
}

static void stop_clock_thread(void)
{
    if (host_clock_thread == NULL) return;
    InterlockedExchange(&host_clock_stop, 1);
    (void)WaitForSingleObject(host_clock_thread, 1000);
    CloseHandle(host_clock_thread);
    host_clock_thread = NULL;
}

void host_start_clock(void)
{
    if (host_clock_thread != NULL) return;
    longclock = 0;
    *myclock = 0;
    InterlockedExchange(&host_clock_stop, 0);
    host_clock_thread = CreateThread(NULL, 0, clock_thread_proc, NULL, 0, NULL);
}

void host_stop_clock(void)
{
    stop_clock_thread();
}

static int map_virtual_key(WPARAM virtual_key)
{
    switch (virtual_key) {
    case VK_ESCAPE: return key_escape;
    case VK_RETURN: return key_enter;
    case VK_SPACE: return key_space;
    case VK_LEFT: return key_left;
    case VK_RIGHT: return key_right;
    case VK_UP: return key_up;
    case VK_DOWN: return key_down;
    case VK_CONTROL: return key_ctrl;
    case VK_MENU: return key_alt;
    case VK_SHIFT: return key_shift;
    case VK_F1: return key_f1;
    case VK_F2: return key_f2;
    case VK_F3: return key_f3;
    case VK_F4: return key_f4;
    case VK_F5: return key_f5;
    case VK_F9: return key_f9;
    default:
        if (virtual_key >= 'A' && virtual_key <= 'Z') return (int)virtual_key;
        if (virtual_key >= '0' && virtual_key <= '9') return (int)virtual_key;
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

static void destination_rect(const RECT *client, RECT *destination)
{
    int width = client->right - client->left;
    int height = client->bottom - client->top;
    int wanted_width = height * JILL_SCREEN_WIDTH / JILL_SCREEN_HEIGHT;
    int wanted_height = width * JILL_SCREEN_HEIGHT / JILL_SCREEN_WIDTH;
    if (wanted_width <= width) {
        destination->left = (width - wanted_width) / 2;
        destination->top = 0;
        destination->right = destination->left + wanted_width;
        destination->bottom = height;
    } else {
        destination->left = 0;
        destination->top = (height - wanted_height) / 2;
        destination->right = width;
        destination->bottom = destination->top + wanted_height;
    }
}

static void paint_frame(HDC dc)
{
    RECT client, destination;
    HBRUSH black = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (host_window == NULL) return;
    GetClientRect(host_window, &client);
    FillRect(dc, &client, black);
    destination_rect(&client, &destination);
    SetStretchBltMode(dc, COLORONCOLOR);
    StretchDIBits(dc,
                  destination.left, destination.top,
                  destination.right - destination.left,
                  destination.bottom - destination.top,
                  0, 0, JILL_SCREEN_WIDTH, JILL_SCREEN_HEIGHT,
                  host_pixels, &host_bitmap, DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT CALLBACK host_window_proc(HWND window, UINT message,
                                         WPARAM wparam, LPARAM lparam)
{
    int key_code;
    switch (message) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        paint_frame(dc);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        key_code = map_virtual_key(wparam);
        if (key_code > 0 && key_code < HOST_KEY_CAPACITY) {
            host_keys[key_code] = 1;
            if (!is_modifier_key(key_code) && (lparam & (1L << 30)) == 0)
                queue_key(key_code);
        }
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        key_code = map_virtual_key(wparam);
        if (key_code > 0 && key_code < HOST_KEY_CAPACITY) host_keys[key_code] = 0;
        return 0;
    case WM_KILLFOCUS:
        memset(host_keys, 0, sizeof(host_keys));
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        host_window = NULL;
        host_opened = 0;
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcA(window, message, wparam, lparam);
    }
}

int host_open(const char *title, int scale)
{
    static const char class_name[] = "JillRecoveryWindow";
    WNDCLASSA window_class;
    RECT rectangle;
    HINSTANCE instance = GetModuleHandleA(NULL);
    if (host_opened) return 1;
    if (scale < 1) scale = 1;
    if (scale > 5) scale = 5;

    memset(&window_class, 0, sizeof(window_class));
    window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = host_window_proc;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    window_class.lpszClassName = class_name;
    if (!RegisterClassA(&window_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return 0;

    rectangle.left = rectangle.top = 0;
    rectangle.right = JILL_SCREEN_WIDTH * scale;
    rectangle.bottom = JILL_SCREEN_HEIGHT * scale;
    AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE);
    host_window = CreateWindowExA(0, class_name, title != NULL ? title : "Jill of the Jungle",
                                  WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                  rectangle.right - rectangle.left,
                                  rectangle.bottom - rectangle.top,
                                  NULL, NULL, instance, NULL);
    if (host_window == NULL) return 0;

    memset(&host_bitmap, 0, sizeof(host_bitmap));
    host_bitmap.bmiHeader.biSize = sizeof(host_bitmap.bmiHeader);
    host_bitmap.bmiHeader.biWidth = JILL_SCREEN_WIDTH;
    host_bitmap.bmiHeader.biHeight = -JILL_SCREEN_HEIGHT;
    host_bitmap.bmiHeader.biPlanes = 1;
    host_bitmap.bmiHeader.biBitCount = 32;
    host_bitmap.bmiHeader.biCompression = BI_RGB;
    memset(host_pixels, 0, sizeof(host_pixels));
    host_clear_keys();
    host_opened = 1;
    host_start_clock();
    ShowWindow(host_window, SW_SHOW);
    UpdateWindow(host_window);
    SetFocus(host_window);
    return 1;
}

void host_close(void)
{
    stop_clock_thread();
    if (host_window != NULL) DestroyWindow(host_window);
    host_window = NULL;
    host_opened = 0;
    host_clear_keys();
}

int host_is_open(void) { return host_opened; }

int host_pump(void)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) host_opened = 0;
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
    return host_opened;
}

void host_present(const byte *pixels, const byte *palette)
{
    size_t index;
    HDC dc;
    if (!host_opened || host_window == NULL || pixels == NULL || palette == NULL) return;
    for (index = 0; index < JILL_SCREEN_WIDTH * JILL_SCREEN_HEIGHT; ++index) {
        unsigned color = pixels[index];
        unsigned red = (unsigned)palette[color * 3] * 255U / 63U;
        unsigned green = (unsigned)palette[color * 3 + 1] * 255U / 63U;
        unsigned blue = (unsigned)palette[color * 3 + 2] * 255U / 63U;
        host_pixels[index] = (uint32_t)(blue | (green << 8) | (red << 16));
    }
    dc = GetDC(host_window);
    if (dc != NULL) { paint_frame(dc); ReleaseDC(host_window, dc); }
}

void host_set_title(const char *title)
{
    if (host_window != NULL) SetWindowTextA(host_window, title != NULL ? title : "Jill of the Jungle");
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
    return key_code > 0 && key_code < HOST_KEY_CAPACITY && host_keys[key_code] != 0;
}

void host_clear_keys(void)
{
    memset(host_keys, 0, sizeof(host_keys));
    host_queue_read = host_queue_write = 0;
}

void host_console_clear(void)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD origin = { 0, 0 };
    DWORD cells, written;

    if (output == INVALID_HANDLE_VALUE || !GetConsoleScreenBufferInfo(output, &info)) return;
    cells = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
    FillConsoleOutputCharacterA(output, ' ', cells, origin, &written);
    FillConsoleOutputAttribute(output, info.wAttributes, cells, origin, &written);
    SetConsoleCursorPosition(output, origin);
}

void host_sleep(unsigned milliseconds) { Sleep(milliseconds); }

longword host_coreleft(void)
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) return 0;
    return status.ullAvailVirtual > 0x7fffffffULL
        ? 0x7fffffffL : (longword)status.ullAvailVirtual;
}
uint32_t host_ticks(void) { return GetTickCount(); }
