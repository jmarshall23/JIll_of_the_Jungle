/* DOSIO.H - DOS file-creation semantics at the native host boundary. */
#ifndef JILL_DOSIO_H
#define JILL_DOSIO_H

#include <io.h>

int jill_dos_creat(const char *path, int attributes);

/* Borland DOS treated the second creat argument as DOS file attributes.
   The Microsoft CRT treats it as a permission mask instead. */
#define _creat(path, attributes) jill_dos_creat((path), (attributes))

#endif
