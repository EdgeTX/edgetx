/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "debug.h"

#undef errno
extern int errno;
extern int _heap_start;
extern int _heap_end;

unsigned char * heap = (unsigned char *)&_heap_start;

extern caddr_t _sbrk(int nbytes)
{
  if (heap + nbytes < (unsigned char *)&_heap_end) {
    unsigned char * prev_heap = heap;
    heap += nbytes;
    return (caddr_t)prev_heap;
  }
  else {
    errno = ENOMEM;
    return ((void *)-1);
  }
}

#if defined(THREADSAFE_MALLOC) && !defined(BOOT)

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/task.h>

void __malloc_lock(struct _reent *r)
{
  (void)(r);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    vTaskSuspendAll();
};

void __malloc_unlock(struct _reent *r)
{
  (void)(r);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    (void)xTaskResumeAll();
};
#endif

#if !defined(SEMIHOSTING)
extern int _gettimeofday(void *p1, void *p2)
{
  return 0;
}

extern int _link(char *old, char *nw)
{
  return -1;
}

extern int _unlink(const char *path)
{
  return -1;
}

extern int _open(const char *name, int flags, int mode)
{
  return -1;
}

extern int _close(int file)
{
  return -1;
}

extern int _fstat(int file, struct stat * st)
{
  st->st_mode = S_IFCHR;
  return 0;
}

extern int _isatty(int file)
{
  return 1;
}

extern int _lseek(int file, int ptr, int dir)
{
  return 0;
}

extern int _read(int file, char *ptr, int len)
{
  return 0;
}

extern int _write(int file, char *ptr, int len)
{
  return 0;
}

extern int _getpid()
{
  return -1;
}
#endif

extern void _exit(int status)
{
  TRACE("_exit(%d)", status);
  for (;;);
}

extern void _kill(int pid, int sig)
{
  return;
}
