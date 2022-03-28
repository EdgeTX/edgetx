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
#include <sys/reent.h>
#include <reent.h>
#include <errno.h>
#include <fcntl.h>

#include "VirtualFS.h"

// TODO: places this somewhere else
//       and check if we really need that many files
#define MAX_OPEN_FILES 4

struct open_files_t
{
  int handle;
  int pos;
  int flags;
  VfsFile * vfs_file;
};

static open_files_t open_files[MAX_OPEN_FILES];

static int findslot(int fh)
{
  static int slot;
  static int lastfh = -1;

  if ((fh != -1) && (fh == lastfh))
    return slot;

  for (slot = 0; slot < MAX_OPEN_FILES; slot++)
    if (open_files[slot].handle == fh)
      break;
  
  lastfh = fh;

  return slot;
}

static int remap_vfs_errors(VfsError e)
{
  switch (e) {
  // case OK: // No error

  case VfsError::IO:      // Error during device operation
  case VfsError::CORRUPT: // Corrupted
  case VfsError::NOT_READY: // storage not ready
    errno = EIO;
    break;

  case VfsError::NOENT: // No directory entry
    errno = ENOENT;
    break;

  case VfsError::EXIST: // Entry already exists
    errno = EEXIST;
    break;

  case VfsError::NOTDIR:      // Entry is not a dir
  case VfsError::ISDIR:       // Entry is a dir
  case VfsError::NOTEMPTY:    // Dir is not empty
  case VfsError::INVAL:       // Invalid parameter
  case VfsError::NOATTR:      // No data/attr available
  case VfsError::NAMETOOLONG: // File name too long
    errno = EINVAL;
    break;

  case VfsError::BADF: // Bad file number
    errno = EBADF;
    break;

  case VfsError::FBIG:  // File too large
  case VfsError::NOSPC: // No space left on device
  case VfsError::NOMEM: // No more memory available
  default:
    errno = EIO;
    break;
  }

  return -1;
}

#define FILE_HANDLE_OFFSET (0x20)

static int remap_handle(int fh)
{
  return fh - FILE_HANDLE_OFFSET;
}

static int set_errno(int errval)
{
  errno = errval;
  return -1;
}

extern "C" int _open(const char *name, int flags, ...)
{
  int fh = 0;
  int slot;

  if ((slot = findslot(-1)) == MAX_OPEN_FILES)
    return set_errno(ENFILE);

  if (flags & O_APPEND)
    flags &= ~O_TRUNC;

  VfsOpenFlags vfs_flags = VfsOpenFlags::NONE;
  if (((flags & (O_CREAT | O_TRUNC)) == (O_CREAT | O_TRUNC)) &&
      (flags & (O_RDWR | O_WRONLY)))
    vfs_flags = VfsOpenFlags::CREATE_ALWAYS;
  else if ((flags & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
    vfs_flags = VfsOpenFlags::OPEN_EXISTING;
  else if ((flags & O_CREAT) == O_CREAT)
    vfs_flags = VfsOpenFlags::OPEN_ALWAYS;
  else if ((flags == O_RDONLY) || (flags == O_WRONLY) || (flags == O_RDWR))
    vfs_flags = VfsOpenFlags::OPEN_EXISTING;
  else
    return set_errno(EINVAL);

  if ((flags & O_ACCMODE) == O_RDONLY)
    vfs_flags = vfs_flags | VfsOpenFlags::READ;
  else if ((flags & O_ACCMODE) == O_WRONLY)
    vfs_flags = vfs_flags | VfsOpenFlags::WRITE;
  else if ((flags & O_ACCMODE) == O_RDWR)
    vfs_flags = vfs_flags | VfsOpenFlags::READ | VfsOpenFlags::WRITE;
  else
    return set_errno(EINVAL);

  fh = -1;
  errno = EIO;

  VfsError err;
  if (!open_files[slot].vfs_file)
    if ((open_files[slot].vfs_file = new VfsFile()) != nullptr)
      if ((fh = ((err = VirtualFS::instance().openFile(
                      *open_files[slot].vfs_file, name, vfs_flags)) ==
                 VfsError::OK)
                    ? slot
                    : -1) == -1)
        remap_vfs_errors(err);

  if (fh >= 0) {
    open_files[slot].handle = fh;
    open_files[slot].pos = 0;
    open_files[slot].flags = 0;

    if (flags & O_APPEND) {
      auto f = open_files[slot].vfs_file;
      int s = f->size();
      if (f->lseek(s) != VfsError::OK)
        fh = -1;
      else
        open_files[slot].pos = f->tell();
    }
  }

  if ((fh < 0) && open_files[slot].vfs_file) {
    delete open_files[slot].vfs_file;
    open_files[slot].vfs_file = nullptr;
  }
  
  return fh >= 0 ? (fh + FILE_HANDLE_OFFSET) : -1;
}

extern "C" int _close(int fd)
{
  int slot = findslot(remap_handle(fd));
  if (slot == MAX_OPEN_FILES)
    return set_errno(EBADF);
  
  open_files[slot].handle = -1;

  if (open_files[slot].vfs_file) {

    auto f = open_files[slot].vfs_file;
    auto err = f->close();

    open_files[slot].vfs_file = nullptr;
    delete f;

    if (err != VfsError::OK)
      return remap_vfs_errors(err);
  }

  return 0;
}

extern "C" int _fstat(int fd, struct stat * st)
{
  (void)fd;
  (void)st;
  return set_errno(ENOSYS);
}

extern "C" int _lseek(int fd, int ptr, int dir)
{
  int slot = findslot(remap_handle(fd));
  if (slot == MAX_OPEN_FILES || !open_files[slot].vfs_file)
    return set_errno(EBADF);

  // We support only SEEK_SET for now
  if (dir != SEEK_SET)
    return set_errno(EINVAL);
  
  auto err = open_files[slot].vfs_file->lseek(ptr);
  if (err != VfsError::OK)
    return remap_vfs_errors(err);
  
  return open_files[slot].pos = open_files[slot].vfs_file->tell();
}

extern "C" int _read(int fd, char *ptr, int len)
{
  int slot = findslot(remap_handle(fd));
  if (slot == MAX_OPEN_FILES || !open_files[slot].vfs_file)
    return set_errno(EBADF);

  if (open_files[slot].flags & O_WRONLY)
    return set_errno(EBADF);

  auto f = open_files[slot].vfs_file;
  size_t bytesRead;
  VfsError err;

  if ((err = f->read(ptr, len, bytesRead)) != VfsError::OK)
    return remap_vfs_errors(err);
  
  int bytesUnRead = len - bytesRead;
  if (bytesUnRead < 0)
    return -1;

  open_files[slot].pos += len - bytesUnRead;
  return len - bytesUnRead;
}

extern "C" int _write(int fd, char *ptr, int len)
{
#if !defined(BOOT)
  int slot = findslot(remap_handle(fd));
  if (slot == MAX_OPEN_FILES || !open_files[slot].vfs_file)
    return set_errno(EBADF);

  if (open_files[slot].flags & O_RDONLY)
    return set_errno(EBADF);

  auto f = open_files[slot].vfs_file;
  size_t bytesWritten;
  VfsError err;

  if ((err = f->write(ptr, len, bytesWritten)) != VfsError::OK)
    return remap_vfs_errors(err);

  int bytesUnWritten = len - bytesWritten;
  if (bytesUnWritten < 0 || bytesUnWritten == len)
    return -1;

  open_files[slot].pos += len - bytesUnWritten;

  return len - bytesUnWritten;
#else
  return set_errno(ENOSYS);
#endif
}

// Not Implemented:
// int _mkdir(const char *path, mode_t mode __attribute__ ((unused)))
// int _chmod(const char *path, mode_t mode)
