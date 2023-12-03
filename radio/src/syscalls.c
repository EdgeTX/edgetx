/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "debug.h"

#if !defined(__PICOLIBC__)
extern char __heap_start[];
extern char __heap_end[];

static char* heap = __heap_start;

extern caddr_t _sbrk(int nbytes)
{
  if (heap + nbytes < __heap_end) {
    char* prev_heap = heap;
    heap += nbytes;
    return (caddr_t)prev_heap;
  }
  else {
    errno = ENOMEM;
    return ((void *)-1);
  }
}
#endif

#if defined(THREADSAFE_MALLOC) && !defined(BOOT)

#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/task.h>

#if defined(__PICOLIBC__)

#define _LOCK_T void*

_LOCK_T __lock___libc_recursive_mutex;

void __retarget_lock_init(_LOCK_T* lock) {}
void __retarget_lock_init_recursive(_LOCK_T* lock) {}

void __retarget_lock_close(_LOCK_T lock) {}
void __retarget_lock_close_recursive(_LOCK_T lock) {}

void __retarget_lock_acquire(_LOCK_T lock) {}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
  (void)(lock);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    vTaskSuspendAll();
}

void __retarget_lock_release(_LOCK_T lock) {}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
  (void)(lock);
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    (void)xTaskResumeAll();  
}

#else // __PICOLIBC__

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

#endif // __PICOLIBC__

#endif // THREADSAFE_MALLOC && !BOOT

#if !defined(SEMIHOSTING)
extern int _gettimeofday(void *p1, void *p2) { return 0; }
extern int gettimeofday(void *p1, void *p2) { return 0; }

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

extern int _getpid() { return -1; }

#if defined(__PICOLIBC__)

#include <stdio.h>

static int fake_putc(char c, FILE *file)
{
  (void) file;
  return c;
}

static const FILE __stdio = FDEV_SETUP_STREAM(fake_putc,
                                              NULL,
                                              NULL,
                                              _FDEV_SETUP_READ);

FILE *const stdin = (FILE *const)&__stdio;
__strong_reference(stdin, stdout);
__strong_reference(stdin, stderr);

#endif // USE_PICOLIBC

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
