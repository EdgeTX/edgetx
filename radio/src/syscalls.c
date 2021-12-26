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
extern int _end;
extern int _heap_end;

unsigned char * heap = (unsigned char *)&_end;

static int set_errno(int errval)
{
  errno = errval;
  return -1;
}

extern caddr_t _sbrk(int nbytes)
{
  if (heap + nbytes >= (unsigned char *)&_heap_end)
    return (void*)set_errno(ENOMEM);
  
  unsigned char * prev_heap = heap;
  heap += nbytes;
  return (caddr_t)prev_heap;
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
  (void)old;
  (void)nw;
  return set_errno(ENOSYS);
}

extern int _unlink(const char *path)
{
  (void)path;
  return set_errno(ENOSYS);
}

extern int _isatty(int file)
{
  (void)file;
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

int _open(const char *name, int flags, ...)
{
  return 0;
}

int _close(int fd)
{
  return 0;
}

int _fstat(int fd, struct stat * st)
{
  return 0;
}

int _lseek(int fd, int ptr, int dir)
{
  return 0;
}

int _read(int fd, char *ptr, int len)
{
  return 0;
}

int _write(int fd, char *ptr, int len)
{
  return 0;
}

