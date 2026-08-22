/* HOSTCOMPAT.H - small CRT/filesystem compatibility layer for recovered DOS code. */
#ifndef JILL_HOSTCOMPAT_H
#define JILL_HOSTCOMPAT_H

#include <stddef.h>
#include <stdio.h>

FILE *jill_fopen(const char *path, const char *mode);

#if defined(_WIN32)

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

#define JILL_PATH_SEPARATOR '\\'

#else

#include <ctype.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

#define JILL_PATH_SEPARATOR '/'

int jill_host_open(const char *path, int flags, ...);
long jill_host_filelength(int fd);

#define _open       jill_host_open
#define _read       read
#define _write      write
#define _close      close
#define _lseek      lseek
#define _access     access
#define _chmod      chmod
#define _filelength jill_host_filelength

#ifndef _O_BINARY
#define _O_BINARY 0
#endif
#define _O_RDONLY O_RDONLY
#define _O_WRONLY O_WRONLY
#define _O_CREAT  O_CREAT
#define _O_TRUNC  O_TRUNC

#ifndef _S_IREAD
#define _S_IREAD S_IRUSR
#endif
#ifndef _S_IWRITE
#define _S_IWRITE S_IWUSR
#endif

static inline char *jill_strupr(char *string)
{
    char *current = string;

    if (string == NULL)
        return NULL;

    while (*current != '\0') {
        *current = (char)toupper((unsigned char)*current);
        ++current;
    }

    return string;
}

#define _strupr jill_strupr
#define _stricmp  strcasecmp
#define _strnicmp strncasecmp

static inline char *jill_ltoa(long value, char *buffer, int radix)
{
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char reversed[sizeof(unsigned long) * 8 + 1];
    unsigned long magnitude;
    size_t count = 0;
    size_t output = 0;
    int negative;

    if (buffer == NULL) return NULL;
    if (radix < 2 || radix > 36) {
        buffer[0] = '\0';
        return buffer;
    }

    negative = value < 0 && radix == 10;
    magnitude = value < 0 ? 0UL - (unsigned long)value : (unsigned long)value;

    do {
        reversed[count++] = digits[magnitude % (unsigned long)radix];
        magnitude /= (unsigned long)radix;
    } while (magnitude != 0);

    if (negative) buffer[output++] = '-';
    while (count != 0) buffer[output++] = reversed[--count];
    buffer[output] = '\0';
    return buffer;
}

static inline char *jill_itoa(int value, char *buffer, int radix)
{
    return jill_ltoa((long)value, buffer, radix);
}

#define _ltoa jill_ltoa
#define _itoa jill_itoa

#endif /* !_WIN32 */

#endif /* JILL_HOSTCOMPAT_H */