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

#include "HOSTCOMPAT.H"
#include "RECOVERY.H"
#include "HOSTSDL.H"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#if !defined(_WIN32)
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#endif

static volatile uword jill_bios_clock;
volatile uword *myclock = &jill_bios_clock;
volatile longword longclock;

#if !defined(_WIN32)
#define JILL_HOST_PATH_MAX 4096

static int jill_resolve_case_path(const char *path, char *resolved, size_t capacity)
{
    char directory[JILL_HOST_PATH_MAX];
    const char *name;
    const char *slash;
    DIR *dir;
    struct dirent *entry;
    int found = 0;

    if (path == NULL || resolved == NULL || capacity == 0) return 0;
    slash = strrchr(path, '/');
    if (slash == NULL) {
        strcpy(directory, ".");
        name = path;
    } else {
        size_t length = (size_t)(slash - path);
        if (length == 0) {
            strcpy(directory, "/");
        } else {
            if (length >= sizeof(directory)) return 0;
            memcpy(directory, path, length);
            directory[length] = '\0';
        }
        name = slash + 1;
    }

    if (*name == '\0') return 0;
    dir = opendir(directory);
    if (dir == NULL) return 0;

    while ((entry = readdir(dir)) != NULL) {
        int written;
        if (strcasecmp(entry->d_name, name) != 0) continue;
        if (strcmp(directory, ".") == 0)
            written = snprintf(resolved, capacity, "%s", entry->d_name);
        else if (strcmp(directory, "/") == 0)
            written = snprintf(resolved, capacity, "/%s", entry->d_name);
        else
            written = snprintf(resolved, capacity, "%s/%s", directory, entry->d_name);
        found = written >= 0 && (size_t)written < capacity;
        break;
    }

    closedir(dir);
    return found;
}

static int jill_posix_open(const char *path, int flags, mode_t mode, int has_mode)
{
    return has_mode ? open(path, flags, mode) : open(path, flags);
}

int jill_host_open(const char *path, int flags, ...)
{
    char resolved[JILL_HOST_PATH_MAX];
    mode_t mode = 0;
    int has_mode = (flags & O_CREAT) != 0;
    int result;
    int saved_errno;

    if (has_mode) {
        va_list arguments;
        va_start(arguments, flags);
        mode = (mode_t)va_arg(arguments, int);
        va_end(arguments);

        /* DOS/Windows would reopen an existing file regardless of case. */
        if (jill_resolve_case_path(path, resolved, sizeof(resolved)))
            return jill_posix_open(resolved, flags, mode, 1);
    }

    result = jill_posix_open(path, flags, mode, has_mode);
    if (result >= 0 || errno != ENOENT) return result;

    saved_errno = errno;
    if (!jill_resolve_case_path(path, resolved, sizeof(resolved))) {
        errno = saved_errno;
        return -1;
    }
    return jill_posix_open(resolved, flags, mode, has_mode);
}

long jill_host_filelength(int fd)
{
    struct stat status;
    if (fstat(fd, &status) != 0) return -1;
    return (long)status.st_size;
}
#endif

FILE *jill_fopen(const char *path, const char *mode)
{
#if defined(_WIN32)
    return fopen(path, mode);
#else
    char resolved[JILL_HOST_PATH_MAX];
    FILE *file;
    int saved_errno;

    /* For creating/appending files, prefer an existing case-insensitive match. */
    if (mode != NULL && mode[0] != 'r' &&
        jill_resolve_case_path(path, resolved, sizeof(resolved)))
        return fopen(resolved, mode);

    file = fopen(path, mode);
    if (file != NULL || errno != ENOENT) return file;

    saved_errno = errno;
    if (!jill_resolve_case_path(path, resolved, sizeof(resolved))) {
        errno = saved_errno;
        return NULL;
    }
    return fopen(resolved, mode);
#endif
}

uword jill_read_u16_le(const byte *source)
{
    return (uword)(source[0] | ((uword)source[1] << 8));
}

ulongword jill_read_u32_le(const byte *source)
{
    return (ulongword)source[0]
        | ((ulongword)source[1] << 8)
        | ((ulongword)source[2] << 16)
        | ((ulongword)source[3] << 24);
}

void jill_write_u16_le(byte *destination, uword value)
{
    destination[0] = (byte)value;
    destination[1] = (byte)(value >> 8);
}

void jill_write_u32_le(byte *destination, ulongword value)
{
    destination[0] = (byte)value;
    destination[1] = (byte)(value >> 8);
    destination[2] = (byte)(value >> 16);
    destination[3] = (byte)(value >> 24);
}

char *jill_strdup(const char *source)
{
    size_t length;
    char *copy;
    if (source == NULL) return NULL;
    length = strlen(source) + 1;
    copy = (char *)malloc(length);
    if (copy != NULL) memcpy(copy, source, length);
    return copy;
}

int jill_random(int limit)
{
    if (limit <= 0) return 0;
    return rand() % limit;
}

void jill_delay(unsigned milliseconds)
{
    if (host_is_open()) { host_sleep(milliseconds); return; }
    const clock_t until = clock() + (clock_t)milliseconds * CLOCKS_PER_SEC / 1000;
    while (clock() < until) { }
}

uint32_t jill_ticks(void)
{
    if (host_is_open()) return host_ticks();
    return (uint32_t)((uint64_t)clock() * 1000U / CLOCKS_PER_SEC);
}

int jill_dos_creat(const char *path, int attributes)
{
    (void)attributes;
    if (_access(path, 0) == 0)
        (void)_chmod(path, _S_IREAD | _S_IWRITE);
    return _open(path, _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY,
                 _S_IREAD | _S_IWRITE);
}
