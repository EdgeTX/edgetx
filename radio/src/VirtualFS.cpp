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
#include <stdint.h>
#include <vector>
#include <stdarg.h>
#include <string.h>

#include "strhelpers.h"
#if !defined(BOOT)
#include "opentx.h"
#endif
#include "edgetx_assert.h"
#include "VirtualFS.h"
#include "board.h"
#include "audio.h"
#include "disk_cache.h"
#include "debug.h"

#if defined(LIBOPENUI) && 0
  #include "libopenui.h"
#endif

VirtualFS* VirtualFS::_instance = nullptr;;

#if defined(SDCARD)
static FATFS sdFatFs __DMA;    // initialized in boardInit()

#if defined(LOG_TELEMETRY)
VfsFile g_telemetryFile = {};
#endif

#if defined(LOG_BLUETOOTH)
VfsFile g_bluetoothFile = {};
#endif
#endif

#if defined(SPI_FLASH)
static FATFS spiFatFs __DMA;
#endif


#if !defined(CLIPBOARD_PATH_LEN)
#define CLIPBOARD_PATH_LEN 32
#endif

static VfsError convertResult(FRESULT err)
{
  switch(err)
  {
  case FR_OK:                  return VfsError::OK;
  case FR_DISK_ERR:            return VfsError::IO;
  case FR_INT_ERR:             return VfsError::INVAL;
  case FR_NOT_READY:           return VfsError::NOT_READY;
  case FR_NO_FILE:             return VfsError::NOENT;
  case FR_NO_PATH:             return VfsError::NOENT;
  case FR_INVALID_NAME:        return VfsError::INVAL;
  case FR_DENIED:              return VfsError::INVAL;
  case FR_EXIST:               return VfsError::EXIST;
  case FR_INVALID_OBJECT:      return VfsError::BADF;
  case FR_WRITE_PROTECTED:     return VfsError::INVAL;
  case FR_INVALID_DRIVE:       return VfsError::INVAL;
  case FR_NOT_ENABLED:         return VfsError::INVAL;
  case FR_NO_FILESYSTEM:       return VfsError::INVAL;
  case FR_MKFS_ABORTED:        return VfsError::INVAL;
  case FR_TIMEOUT:             return VfsError::INVAL;
  case FR_LOCKED:              return VfsError::INVAL;
  case FR_NOT_ENOUGH_CORE:     return VfsError::INVAL;
  case FR_TOO_MANY_OPEN_FILES: return VfsError::INVAL;
  case FR_INVALID_PARAMETER:   return VfsError::INVAL;
  }
  return VfsError::INVAL;
}

static int convertOpenFlagsToFat(VfsOpenFlags flags)
{
  return (int)flags;
}

VfsOpenFlags operator|(VfsOpenFlags lhs,VfsOpenFlags rhs)
{
  typedef typename
    std::underlying_type<VfsOpenFlags>::type underlying;
  return static_cast<VfsOpenFlags>(
    static_cast<underlying>(lhs)
  | static_cast<underlying>(rhs));
}

VfsOpenFlags operator|=(VfsOpenFlags lhs,VfsOpenFlags rhs)
{
  lhs = lhs|rhs;
  return lhs;
}

VfsOpenFlags operator&(VfsOpenFlags lhs,VfsOpenFlags rhs)
{
  typedef typename
    std::underlying_type<VfsOpenFlags>::type underlying;
  return static_cast<VfsOpenFlags>(
    static_cast<underlying>(lhs)
  & static_cast<underlying>(rhs));
}

VfsFileAttributes operator|(VfsFileAttributes lhs,VfsFileAttributes rhs)
{
  typedef typename
    std::underlying_type<VfsFileAttributes>::type underlying;
  return static_cast<VfsFileAttributes>(
    static_cast<underlying>(lhs)
  | static_cast<underlying>(rhs));
}

VfsFileAttributes operator&(VfsFileAttributes lhs,VfsFileAttributes rhs)
{
  typedef typename
    std::underlying_type<VfsFileAttributes>::type underlying;
  return static_cast<VfsFileAttributes>(
    static_cast<underlying>(lhs)
  & static_cast<underlying>(rhs));
}

const char* VfsFileInfo::getName() const
{
  switch(type)
  {
  case VfsFileType::ROOT: return name;
  case VfsFileType::FAT:  return(name != nullptr)?name:fatInfo.fname;
  default: break;
  }
  return "";
};

size_t VfsFileInfo::getSize() const
{
  switch(type)
  {
  case VfsFileType::ROOT: return 0;
  case VfsFileType::FAT:  return fatInfo.fsize;
  default: break;
  }
  return 0;
}

VfsType VfsFileInfo::getType() const
{
  switch(type)
  {
  case VfsFileType::ROOT:
    return VfsType::DIR;
  case VfsFileType::FAT:
    if (name != nullptr)
      return VfsType::DIR;
    if(fatInfo.fattrib & AM_DIR)
      return VfsType::DIR;
    else
      return VfsType::FILE;
  default: break;
  }
  return VfsType::UNKOWN;
};

VfsFileAttributes VfsFileInfo::getAttrib()
{
  switch(type)
  {
  case VfsFileType::ROOT:
    return VfsFileAttributes::DIR;
  case VfsFileType::FAT:
    return (VfsFileAttributes)fatInfo.fattrib;
  default: break;
  }
  return VfsFileAttributes::NONE;
}

int VfsFileInfo::getDate(){
  switch(type)
  {
  case VfsFileType::ROOT:
    return 0;
  case VfsFileType::FAT:
    return fatInfo.fdate;
  default: break;
  }
  return 0;
}

int VfsFileInfo::getTime()
{
  switch(type)
  {
  case VfsFileType::ROOT:
    return 0;
  case VfsFileType::FAT:
    return fatInfo.ftime;
  default: break;
  }
  return 0;
}

void VfsFileInfo::clear() {
  type = VfsFileType::UNKNOWN;
  fatInfo = {0};
  name = nullptr;
}


void VfsDir::clear()
{
  type = DIR_UNKNOWN;
  fat.dir = {0};

  readIdx = 0;
}

VfsError VfsDir::read(VfsFileInfo& info)
{
  info.clear();
  switch(type)
  {
  case VfsDir::DIR_ROOT:
    info.type = VfsFileType::ROOT;
#if defined(SPI_FLASH)
    if(readIdx == 0)
      info.name = "INTERNAL";
#if defined(SDCARD)
    else if(readIdx == 1)
      info.name = "SDCARD";
#endif
    else
      info.name = "";
#elif defined(SDCARD) // SPI_FLASH
    if(readIdx == 0)
      info.name = "SDCARD";
    else
      info.name = "";
#endif // SDCARD
    readIdx++;
    return VfsError::OK;
  case VfsDir::DIR_FAT:
  {
    info.type = VfsFileType::FAT;
    if(readIdx == 0) // emulate ".." entry
    {
      readIdx++;
      info.name = "..";
      return VfsError::OK;
    }
    VfsError ret = convertResult(f_readdir(&fat.dir, &info.fatInfo));
    return ret;
  }
  default: break;
  }
  return VfsError::INVAL;
}

VfsError VfsDir::close()
{
  VfsError ret = VfsError::INVAL;
  switch(type)
  {
  case VfsDir::DIR_ROOT:
    ret = VfsError::OK;
    break;
  case VfsDir::DIR_FAT:
    ret = convertResult(f_closedir(&fat.dir));
    break;
  default: break;
  }
  clear();
  return ret;
}

VfsError VfsDir::rewind()
{
  readIdx = 0;
  switch(type)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::OK;
  case VfsDir::DIR_FAT:
  {
    return convertResult(f_readdir(&fat.dir, nullptr));
  }
  default: break;
  }
  return VfsError::INVAL;
}

void VfsFile::clear() {
  type = VfsFileType::UNKNOWN;
  fat.file = {0};
}

VfsError VfsFile::close()
{
  VfsError ret = VfsError::INVAL;
  switch(type)
  {
  case VfsFileType::FAT:
    ret = convertResult(f_close(&fat.file));
    break;
  default: break;
  }

  clear();
  return ret;
}

int VfsFile::size()
{
  switch(type)
  {
  case VfsFileType::FAT:
    return f_size(&fat.file);
    break;
  default: break;
  }

  return -1;
}

VfsError VfsFile::read(void* buf, size_t size, size_t& readSize)
{
  switch(type)
  {
  case VfsFileType::FAT: {
    UINT rd = 0;
    VfsError res = convertResult(f_read(&fat.file, buf, size, &rd));
    readSize = rd;
    return res;
  }
  default: break;
  }

  return VfsError::INVAL;
}

char* VfsFile::gets(char* buf, size_t maxLen)
{
  switch(type)
  {
  case VfsFileType::FAT:
    return f_gets(buf, maxLen, &fat.file);
  default: break;
  }

  return 0;
}

#if !defined(BOOT)
VfsError VfsFile::write(const void* buf, size_t size, size_t& written)
{
  switch(type)
  {
  case VfsFileType::FAT: {
    UINT wrt = 0;
    VfsError res = convertResult(f_write(&fat.file, buf, size, &wrt));
    written = wrt;
    return res;
  }
  default: break;
  }

  return VfsError::INVAL;
}

VfsError VfsFile::puts(const std::string& str)
{
  size_t written;
  return this->write(str.data(), str.length(), written);
}

VfsError VfsFile::putc(char c)
{
  size_t written;
  return this->write(&c, 1, written);
}

int VfsFile::fprintf(const char* str, ...)
{
  switch(type)
  {
#if defined (USE_FATFS)
  case VfsFileType::FAT:
  {
      va_list args;
      va_start(args, str);

      int ret = f_printf(&fat.file, str, args);
      va_end(args);
      return ret;
  }
#endif
#if defined(USE_LITTLEFS)
  case VfsFileType::LFS:
  {

  default: break;
  }
#endif
  }

  return (int)VfsError::INVAL;
}
#endif

size_t VfsFile::tell()
{
  switch(type)
  {
  case VfsFileType::FAT:
    return f_tell(&fat.file);
  default: break;
  }

  return (size_t)VfsError::INVAL;
}

VfsError VfsFile::lseek(size_t offset)
{
  switch(type)
  {
  case VfsFileType::FAT:
    return convertResult(f_lseek(&fat.file, offset));
    break;
  default: break;
  }

  return VfsError::INVAL;
}

int VfsFile::eof()
{
  switch(type)
  {
  case VfsFileType::FAT:
    return f_eof(&fat.file);
  default: break;
  }

  return 0;
}

const char * VirtualFS::getBasename(const char * path)
{
  for (int8_t i = strlen(path) - 1; i >= 0; i--) {
    if (path[i] == '/') {
      return &path[i + 1];
    }
  }
  return path;
}

VirtualFS::VirtualFS()
{
#if defined (SPI_FLASH)

  spiFatFs = {0};
#endif // SPI_FLASH
#if defined (SDCARD)
  sdFatFs = {0};
#endif

  restart();
}

VirtualFS::~VirtualFS()
{
#if defined (SPI_FLASH)
  f_unmount("1:");
#endif // SPI_FLASH
}

void VirtualFS::stop()
{
  stopLogs();
  audioQueue.stopSD();
#if defined (SDCARD)
  if (sdCardMounted()) {
    f_mount(nullptr, "", 0); // unmount SD
  }
#endif

#if defined (SPI_FLASH)
  f_unmount("1:");
#endif // SPI_FLASH
}
void VirtualFS::restart()
{
  TRACE("VirtualFS::restart()");
#if defined (SDCARD)
  mountSd();
#endif
#if defined (SPI_FLASH)
#if !defined(BOOT)
  diskCache[1].clear();
#endif
  if(f_mount(&spiFatFs, "1:", 1) != FR_OK)
  {
#if !defined(BOOT)
    BYTE work[FF_MAX_SS];
    FRESULT res = f_mkfs("1:", FM_ANY, 0, work, sizeof(work));
#if !defined(BOOT)
    switch(res) {
      case FR_OK :
        break;
      case FR_DISK_ERR:
        POPUP_WARNING("Format error");
        break;
      case FR_NOT_READY:
        POPUP_WARNING("Flash not ready");
        break;
      case FR_WRITE_PROTECTED:
        POPUP_WARNING("Flash write protected");
        break;
      case FR_INVALID_PARAMETER:
        POPUP_WARNING("Format param invalid");
        break;
      case FR_INVALID_DRIVE:
        POPUP_WARNING("Invalid drive");
        break;
      case FR_MKFS_ABORTED:
        POPUP_WARNING("Format aborted");
        break;
      default:
        POPUP_WARNING(STR_SDCARD_ERROR);
        break;
    }
#endif
    if(f_mount(&spiFatFs, "1:", 1) != FR_OK)
    {
#if !defined(BOOT)
      POPUP_WARNING(STR_SDCARD_ERROR);
#endif
    }
#endif
  }
#endif // SPI_FLASH
#if !defined(BOOT)
  checkAndCreateDirectory("/DEFAULT/RADIO");
  checkAndCreateDirectory("/DEFAULT/MODELS");
  checkAndCreateDirectory("/DEFAULT/LOGS");
  checkAndCreateDirectory("/DEFAULT/SCREENSHOTS");
  checkAndCreateDirectory("/DEFAULT/BACKUP");
#endif
  startLogs();
}

void VirtualFS::mountSd()
{
  TRACE("VirtualFS::mountSd");
#if defined(SDCARD)
  if(sdCardMounted())
    return;

#if defined(DISK_CACHE) && !defined(BOOT)
  diskCache[0].clear();
#endif

  if (f_mount(&sdFatFs, "", 1) == FR_OK) {
#if(!defined(BOOT))
    // call sdGetFreeSectors() now because f_getfree() takes a long time first time it's called
    sdGetFreeSectors();
#endif
  }
  else {
    TRACE("SD Card f_mount() failed");
  }
#endif
}

bool VirtualFS::defaultStorageAvailable()
{
#if defined (SIMU)
  return true;
#endif
#if (DEFAULT_STORAGE == INTERNAL)
  return spiFatFs.fs_type != 0;
#elif (DEFAULT_STORAGE == SDCARD) // DEFAULT_STORAGE
    return sdFatFs.fs_type != 0;
#endif

}
#if !defined(BOOT)
bool VirtualFS::format()
{
// TODO format
  return false;
//  BYTE work[FF_MAX_SS];
//  FRESULT res = f_mkfs("", FM_FAT32, 0, work, sizeof(work));
//  switch(res) {
//    case FR_OK :
//      return true;
//    case FR_DISK_ERR:
//      POPUP_WARNING("Format error");
//      return false;
//    case FR_NOT_READY:
//      POPUP_WARNING("SDCard not ready");
//      return false;
//    case FR_WRITE_PROTECTED:
//      POPUP_WARNING("SDCard write protected");
//      return false;
//    case FR_INVALID_PARAMETER:
//      POPUP_WARNING("Format param invalid");
//      return false;
//    case FR_INVALID_DRIVE:
//      POPUP_WARNING("Invalid drive");
//      return false;
//    case FR_MKFS_ABORTED:
//      POPUP_WARNING("Format aborted");
//      return false;
//    default:
//      POPUP_WARNING(STR_SDCARD_ERROR);
//      return false;
//  }
}
#endif

VfsDir::DirType VirtualFS::getDirTypeAndPath(char* path)
{
  char tmpPath[2 * CLIPBOARD_PATH_LEN+1] = { 0 };
  char* tmp = &tmpPath[0];
  size_t pLen = strlen(path);

  if(strcmp(PATH_SEPARATOR, path) == 0)
  {
    return VfsDir::DIR_ROOT;
#if defined (SPI_FLASH)
  } else if(strncmp("/INTERNAL", path, 9) == 0)
  {
    tmp = strAppend(tmpPath, "1:", 2);
    if(pLen > 9)
      strAppend(tmp, path+9, pLen-9);
    else
      *tmp = '/';
    strcpy(path, tmpPath);
    return VfsDir::DIR_FAT;
#endif  // SPI_FLASH
#if defined (SDCARD)
  } else if(strncmp("/SDCARD", path, 7) == 0) {
    if(pLen > 7)
      strAppend(tmp, path+7, pLen-7);
    else
      *tmp = '/';
    strcpy(path, tmpPath);
    return VfsDir::DIR_FAT;
#endif
  } else if(strncmp("/DEFAULT", path, 8) == 0) {
#if (DEFAULT_STORAGE == INTERNAL)
    tmp = strAppend(tmpPath, "1:", 2);
    if(pLen > 8)
      strAppend(tmp, path+8, pLen-8);
    else
      *tmp = '/';
    strcpy(path, tmpPath);
    return VfsDir::DIR_FAT;
#elif (DEFAULT_STORAGE == SDCARD) // DEFAULT_STORAGE
    if(pLen > 8)
      strAppend(tmp, path+8, pLen-8);
    else
      *tmp = '/';
    strcpy(path, tmpPath);
    return VfsDir::DIR_FAT;
#else  // DEFAULT_STORAGE
  #error No valid default storage selectd
#endif
  }
  return VfsDir::DIR_UNKNOWN;
}

void VirtualFS::normalizePath(char* path)
{
  char buf[2 * CLIPBOARD_PATH_LEN+1];
  char* wPtr = buf;

  if(path[0] != '/')
  {
    wPtr = strAppend(wPtr, curWorkDir.c_str(), curWorkDir.length());
    *wPtr++ = PATH_SEPARATOR[0];
  }

  wPtr = strAppend(wPtr, path, std::min(strlen(path), sizeof(buf) - (wPtr - buf)));
  buf[sizeof(buf)-1] = '\0';
  char* tokens[20];
  size_t tokenCount = 0;

  char* cur = strtok(buf, "/" );
  while(cur)
  {
    char* old = cur;
    cur = strtok(nullptr, "/");
    if(old[0] == 0)
      continue;
    if(strcmp("..", old) == 0)
    {
      if(tokenCount > 0)
        tokenCount--;
      continue;
    }
    if(strcmp(".", old) == 0)
      continue;

    tokens[tokenCount++] = old;
    if(tokenCount >= sizeof(tokens))
      break;
  }

  wPtr = path;
  for(size_t i = 0; i < tokenCount; i++)
  {
    *wPtr++ = PATH_SEPARATOR[0];
    wPtr = strAppend(wPtr, tokens[i], strlen(tokens[i]));
  }
  *wPtr = '\0';
  if(path[0] == '\0')
  {
    path[0] = PATH_SEPARATOR[0];
    path[1] = '\0';
  }
}

#if !defined(BOOT)
VfsError VirtualFS::unlink(const char* path)
{
  if(path == nullptr)
    return VfsError::INVAL;

  char p[CLIPBOARD_PATH_LEN + 1];
  strncpy(p, path, sizeof(p));
  p[sizeof(p)-1] = '\0';

  normalizePath(p);
  VfsDir::DirType type = getDirTypeAndPath(p);

  switch(type)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::INVAL;
  case VfsDir::DIR_FAT:
    return convertResult(f_unlink(p));
  }

  return VfsError::INVAL;
}
#endif

VfsError VirtualFS::changeDirectory(const char* path)
{
  assert(path != nullptr);

  char p[CLIPBOARD_PATH_LEN + 1];
  strncpy(p, path, sizeof(p));
  p[sizeof(p)-1] = '\0';
  normalizePath(p);
  curWorkDir = p;

  return VfsError::OK;
}

VfsError VirtualFS::openDirectory(VfsDir& dir, const char * path)
{
  dir.clear();
  assert(path != nullptr);

  char dirPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(dirPath, path, sizeof(dirPath));
  dirPath[sizeof(dirPath)-1] = '\0';

  normalizePath(dirPath);

  VfsDir::DirType type = getDirTypeAndPath(dirPath);
  dir.type = type;
  switch(type)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::OK;
  case VfsDir::DIR_FAT:
    return convertResult(f_opendir(&dir.fat.dir, dirPath));
  default: break;
  }

  return VfsError::INVAL;
}
#if !defined(BOOT)
VfsError VirtualFS::makeDirectory(const char* path)
{
  assert(path != nullptr);

  char normPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(normPath, path, sizeof(normPath));
  normPath[sizeof(normPath)-1] = '\0';

  normalizePath(normPath);
  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::INVAL;
    break;
  case VfsDir::DIR_FAT:
  {
    DIR dir;
    FRESULT result = f_opendir(&dir, normPath);
    if (result == FR_OK) {
      f_closedir(&dir);
      return VfsError::OK;
    } else {
      if (result == FR_NO_PATH)
        result = f_mkdir(normPath);
      if (result != FR_OK)
        return convertResult(result);
    }
    break;
  }
  default: break;
  }
  return VfsError::INVAL;
}
VfsError VirtualFS::rename(const char* oldPath, const char* newPath)
{
  assert(oldPath != nullptr);
  assert(newPath != nullptr);

  char oldP[CLIPBOARD_PATH_LEN + 1];
  strncpy(oldP, oldPath, sizeof(oldP));
  oldP[sizeof(oldP)-1] = '\0';
  char newP[CLIPBOARD_PATH_LEN + 1];
  strncpy(newP, newPath, sizeof(newP));
  newP[sizeof(newP)-1] = '\0';

  normalizePath(oldP);
  normalizePath(newP);

  VfsDir::DirType oldType = getDirTypeAndPath(oldP);
  VfsDir::DirType newType = getDirTypeAndPath(newP);

  if(oldType == newType)
  {
    switch(oldType)
    {
    case VfsDir::DIR_ROOT:
      return VfsError::INVAL;
    case VfsDir::DIR_FAT:
      return convertResult(f_rename(oldP, newP));
    }
  } else {
    VfsError err = copyFile(oldPath, newPath);
    if(err == VfsError::OK)
      return unlink(oldPath);
    return err;
  }
  return VfsError::INVAL;
}

VfsError VirtualFS::copyFile(const char* source, const char* destination)
{
  VfsFile src;
  VfsFile dest;

  VfsError err = openFile(src, source, VfsOpenFlags::READ);
  if(err != VfsError::OK)
    return err;

  err = openFile(dest, destination, VfsOpenFlags::CREATE_NEW|VfsOpenFlags::WRITE);
  if(err != VfsError::OK)
  {
    src.close();
    return err;
  }

  err = VfsError::OK;

  char buf[256] = {0};
  size_t readBytes = 0;
  size_t written = 0;

  err = src.read(buf, sizeof(buf), readBytes);
  if(err != VfsError::OK)
    goto cleanup;

  while(readBytes)
  {
    err = dest.write(buf, readBytes, written);
    if(err != VfsError::OK)
      goto cleanup;
    err = src.read(buf, sizeof(buf), readBytes);
    if(err != VfsError::OK)
      goto cleanup;
  }

cleanup:
  src.close();
  dest.close();
  return err;
}

VfsError VirtualFS::copyFile(const char* srcFile, const char* srcDir,
           const char* destDir, const char* destFile)
{
  assert(srcFile != nullptr);
  assert(srcDir != nullptr);
  assert(destFile != nullptr);
  assert(destDir != nullptr);

  char srcPath[2*CLIPBOARD_PATH_LEN+1] = {0};
  char * tmp = strAppend(srcPath, srcDir, CLIPBOARD_PATH_LEN);
  *tmp++ = '/';
  strAppend(tmp, srcFile, CLIPBOARD_PATH_LEN);

  char destPath[2*CLIPBOARD_PATH_LEN+1] = {0};
  tmp = strAppend(destPath, destDir, CLIPBOARD_PATH_LEN);
  *tmp++ = '/';
  strAppend(tmp, destFile, CLIPBOARD_PATH_LEN);

  return copyFile(srcPath, destPath);
}

// Will overwrite if destination exists
const char * VirtualFS::moveFile(const char* srcPath, const char* destPath)
{
  assert(srcPath != nullptr);
  assert(destPath != nullptr);

  auto res = copyFile(srcPath, destPath);
  if(res != VfsError::OK) {
    return STORAGE_ERROR(res);
  }

  res = unlink(srcPath);
  if(res != VfsError::OK) {
    return STORAGE_ERROR(res);
  }
  return nullptr;
}

// Will overwrite if destination exists
const char * VirtualFS::moveFile(const char* srcFilename, const char* srcDir, const char* destFilename, const char* destDir)
{
  auto res = copyFile(srcFilename, srcDir, destFilename, destDir);
  if(res != VfsError::OK) {
    return STORAGE_ERROR(res);
  }

  char srcPath[2*CLIPBOARD_PATH_LEN+1] = { 0 };
  char * tmp = strAppend(srcPath, srcDir, CLIPBOARD_PATH_LEN);
  *tmp++ = '/';
  strAppend(tmp, srcFilename, CLIPBOARD_PATH_LEN);
  res = unlink(srcPath);
  if(res != VfsError::OK) {
    return STORAGE_ERROR(res);
  }
  return nullptr;
}
#endif // !BOOT
#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdGetNoSectors()
{
  static DWORD noSectors = 0;
  if (noSectors == 0 ) {
    disk_ioctl(0, GET_SECTOR_COUNT, &noSectors);
  }
  return noSectors;
}

#endif
VfsError VirtualFS::fstat(const char* path, VfsFileInfo& fileInfo)
{
  assert(path != nullptr);

  char normPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(normPath, path, sizeof(normPath));
  normPath[sizeof(normPath)-1] = '\0';

  normalizePath(normPath);
  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::INVAL;
  case VfsDir::DIR_FAT:
    fileInfo.type = VfsFileType::FAT;
    return convertResult(f_stat(normPath, &fileInfo.fatInfo));
  default: break;
  }
  return VfsError::INVAL;
}
#if !defined(BOOT)
VfsError VirtualFS::utime(const char* path, const VfsFileInfo& fileInfo)
{
  assert(path != nullptr);

  char normPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(normPath, path, sizeof(normPath));
  normPath[sizeof(normPath)-1] = '\0';

  normalizePath(normPath);
  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::INVAL;
  case VfsDir::DIR_FAT:
    return convertResult(f_utime(normPath, &fileInfo.fatInfo));
  default: break;
  }
  return VfsError::INVAL;
}
#endif
VfsError VirtualFS::openFile(VfsFile& file, const char* path, VfsOpenFlags flags)
{
  assert(path != nullptr);

  file.clear();
  char normPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(normPath, path, sizeof(normPath));
  normPath[sizeof(normPath)-1] = '\0';
  normalizePath(normPath);
  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  VfsError ret = VfsError::INVAL;
  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    return VfsError::INVAL;
    break;
  case VfsDir::DIR_FAT:
  {
    file.type = VfsFileType::FAT;
    ret = convertResult(f_open(&file.fat.file, normPath, convertOpenFlagsToFat(flags)));
    break;
  }
  default: break;
  }

  return ret;
}
#if !defined(BOOT)
const char* VirtualFS::checkAndCreateDirectory(const char * path)
{
  VfsError res = makeDirectory(path);

  if(res == VfsError::OK)
    return nullptr;
#if !defined(BOOT)
  return STORAGE_ERROR(res);
#else
  return "could not create directory";
#endif
}

bool VirtualFS::isFileAvailable(const char * path, bool exclDir)
{
  assert(path != nullptr);

  char normPath[CLIPBOARD_PATH_LEN + 1];
  strncpy(normPath, path, sizeof(normPath));
  normPath[sizeof(normPath)-1] = '\0';

  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    return false;
    break;
  case VfsDir::DIR_FAT:
    {
      if (exclDir) {
        FILINFO fno;
        return (f_stat(normPath, &fno) == FR_OK && !(fno.fattrib & AM_DIR));
      }
      return f_stat(normPath, nullptr) == FR_OK;
    }
  default: break;
  }

  return false;
}

const char * VirtualFS::getFileExtension(const char * filename, uint8_t size, uint8_t extMaxLen, uint8_t * fnlen, uint8_t * extlen)
{
  assert(filename != nullptr);

  int len = size;
  if (!size) {
    len = strlen(filename);
  }
  if (!extMaxLen) {
    extMaxLen = LEN_FILE_EXTENSION_MAX;
  }
  if (fnlen != nullptr) {
    *fnlen = (uint8_t)len;
  }
  for (int i=len-1; i >= 0 && len-i <= extMaxLen; --i) {
    if (filename[i] == '.') {
      if (extlen) {
        *extlen = len-i;
      }
      return &filename[i];
    }
  }
  if (extlen != nullptr) {
    *extlen = 0;
  }
  return nullptr;
}

/**
  Check if given extension exists in a list of extensions.
  @param extension The extension to search for, including leading period.
  @param pattern One or more file extensions concatenated together, including the periods.
    The list is searched backwards and the first match, if any, is returned.
    eg: ".gif.jpg.jpeg.png"
  @param match Optional container to hold the matched file extension (wide enough to hold LEN_FILE_EXTENSION_MAX + 1).
  @retval true if a extension was found in the lost, false otherwise.
*/
bool VirtualFS::isFileExtensionMatching(const char * extension, const char * pattern, char * match)
{
  assert(extension != nullptr);
  assert(pattern != nullptr);

  const char *ext;
  uint8_t extlen, fnlen;
  int plen;

  ext = getFileExtension(pattern, 0, 0, &fnlen, &extlen);
  plen = (int)fnlen;
  while (plen > 0 && ext) {
    if (!strncasecmp(extension, ext, extlen)) {
      if (match != nullptr) strncat(&(match[0]='\0'), ext, extlen);
      return true;
    }
    plen -= extlen;
    if (plen > 0) {
      ext = getFileExtension(pattern, plen, 0, nullptr, &extlen);
    }
  }
  return false;
}


/**
  Search file system path for a file. Can optionally take a list of file extensions to search through.
  Eg. find "splash.bmp", or the first occurrence of one of "splash.[bmp|jpeg|jpg|gif]".

  @param path String with path name, no trailing slash. eg; "/BITMAPS"
  @param file String containing file name to search for, with or w/out an extension.
    eg; "splash.bmp" or "splash"
  @param pattern Optional list of one or more file extensions concatenated together, including the period(s).
    The list is searched backwards and the first match, if any, is returned.  If null, then only the actual filename
    passed will be searched for.
    eg: ".gif.jpg.jpeg.bmp"
  @param exclDir true/false whether to allow directory matches (default true, excludes directories)
  @param match Optional container to hold the matched file extension (wide enough to hold LEN_FILE_EXTENSION_MAX + 1).
  @retval true if a file was found, false otherwise.
*/
bool VirtualFS::isFilePatternAvailable(const char * path, const char * file, const char * pattern, bool exclDir, char * match)
{
  uint8_t fplen;
  char fqfp[LEN_FILE_PATH_MAX + FF_MAX_LFN + 1] = "\0";

  fplen = strlen(path);
  if (fplen > LEN_FILE_PATH_MAX) {
    //TRACE_ERROR("isFilePatternAvailable(%s) = error: path too long.\n", path);
    return false;
  }

  strcpy(fqfp, path);
  strcpy(fqfp + fplen, "/");
  strncat(fqfp + (++fplen), file, FF_MAX_LFN);

  if (pattern == nullptr) {
    // no extensions list, just check the filename as-is
    return isFileAvailable(fqfp, exclDir);
  }
  else {
    // extensions list search
    const char *ext;
    uint16_t len;
    uint8_t extlen, fnlen;
    int plen;

    getFileExtension(file, 0, 0, &fnlen, &extlen);
    len = fplen + fnlen - extlen;
    fqfp[len] = '\0';
    ext = getFileExtension(pattern, 0, 0, &fnlen, &extlen);
    plen = (int)fnlen;
    while (plen > 0 && ext) {
      strncat(fqfp + len, ext, extlen);
      if (isFileAvailable(fqfp, exclDir)) {
        if (match != nullptr) strncat(&(match[0]='\0'), ext, extlen);
        return true;
      }
      plen -= extlen;
      if (plen > 0) {
        fqfp[len] = '\0';
        ext = getFileExtension(pattern, plen, 0, nullptr, &extlen);
      }
    }
  }
  return false;
}

char* VirtualFS::getFileIndex(char * filename, unsigned int & value)
{
  value = 0;
  char * pos = (char *)getFileExtension(filename);
  if (!pos || pos == filename)
    return nullptr;
  int multiplier = 1;
  while (pos > filename) {
    pos--;
    char c = *pos;
    if (c >= '0' && c <= '9') {
      value += multiplier * (c - '0');
      multiplier *= 10;
    }
    else {
      return pos+1;
    }
  }
  return filename;
}

static uint8_t _getDigitsCount(unsigned int value)
{
  uint8_t count = 1;
  while (value >= 10) {
    value /= 10;
    ++count;
  }
  return count;
}

unsigned int VirtualFS::findNextFileIndex(char * filename, uint8_t size, const char * directory)
{
  unsigned int index = 0;
  uint8_t extlen;
  char * indexPos = getFileIndex(filename, index);
  char extension[LEN_FILE_EXTENSION_MAX+1] = "\0";
  char * p = (char *)getFileExtension(filename, 0, 0, nullptr, &extlen);
  if (p) strncat(extension, p, sizeof(extension)-1);
  while (true) {
    index++;
    if ((indexPos - filename) + _getDigitsCount(index) + extlen > size) {
      return 0;
    }
    char * pos = strAppendUnsigned(indexPos, index);
    strAppend(pos, extension);
    if (!isFilePatternAvailable(directory, filename, nullptr, false)) {
      return index;
    }
  }
  return 0;
}
#endif
#if !defined(LIBOPENUI) && !defined(BOOT)
bool VirtualFS::listFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags)
{
  static uint16_t lastpopupMenuOffset = 0;
  VfsFileInfo fno;
  VfsDir dir;
  const char * fnExt;
  uint8_t fnLen, extLen;
  char tmpExt[LEN_FILE_EXTENSION_MAX+1] = "\0";

  popupMenuOffsetType = MENU_OFFSET_EXTERNAL;

  static uint8_t s_last_flags;

  if (selection) {
    s_last_flags = flags;
    if (!isFilePatternAvailable(path, selection, ((flags & LIST_SD_FILE_EXT) ? nullptr : extension))) selection = nullptr;
  }
  else {
    flags = s_last_flags;
  }

  if (popupMenuOffset == 0) {
    lastpopupMenuOffset = 0;
    memset(reusableBuffer.modelsel.menu_bss, 0, sizeof(reusableBuffer.modelsel.menu_bss));
  }
  else if (popupMenuOffset == popupMenuItemsCount - MENU_MAX_DISPLAY_LINES) {
    lastpopupMenuOffset = 0xffff;
    memset(reusableBuffer.modelsel.menu_bss, 0, sizeof(reusableBuffer.modelsel.menu_bss));
  }
  else if (popupMenuOffset == lastpopupMenuOffset) {
    // should not happen, only there because of Murphy's law
    return true;
  }
  else if (popupMenuOffset > lastpopupMenuOffset) {
    memmove(reusableBuffer.modelsel.menu_bss[0], reusableBuffer.modelsel.menu_bss[1], (MENU_MAX_DISPLAY_LINES-1)*MENU_LINE_LENGTH);
    memset(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], 0xff, MENU_LINE_LENGTH);
  }
  else {
    memmove(reusableBuffer.modelsel.menu_bss[1], reusableBuffer.modelsel.menu_bss[0], (MENU_MAX_DISPLAY_LINES-1)*MENU_LINE_LENGTH);
    memset(reusableBuffer.modelsel.menu_bss[0], 0, MENU_LINE_LENGTH);
  }

  popupMenuItemsCount = 0;

  VfsError res = openDirectory(dir, path);
  if (res == VfsError::OK) {

    if (flags & LIST_NONE_SD_FILE) {
      popupMenuItemsCount++;
      if (selection) {
        lastpopupMenuOffset++;
      }
      else if (popupMenuOffset==0 || popupMenuOffset < lastpopupMenuOffset) {
        char * line = reusableBuffer.modelsel.menu_bss[0];
        memset(line, 0, MENU_LINE_LENGTH);
        strcpy(line, "---");
        popupMenuItems[0] = line;
      }
    }

    for (;;) {
      res = dir.read(fno);                   /* Read a directory item */
      if (res != VfsError::OK || strlen(fno.getName()) == 0) break;  /* Break on error or end of dir */
      if (fno.getType() == VfsType::DIR) continue;            /* Skip subfolders */
      if ((int)(fno.getAttrib() & VfsFileAttributes::HID) != 0) continue;     /* Skip hidden files */
      if ((int)(fno.getAttrib() & VfsFileAttributes::SYS) != 0) continue;     /* Skip system files */

      fnExt = getFileExtension(fno.getName(), 0, 0, &fnLen, &extLen);
      fnLen -= extLen;

//      TRACE_DEBUG("listSdFiles(%s, %s, %u, %s, %u): fn='%s'; fnExt='%s'; match=%d\n",
//           path, extension, maxlen, (selection ? selection : "nul"), flags, fname.c_str(), (fnExt ? fnExt : "nul"), (fnExt && isExtensionMatching(fnExt, extension)));
      // file validation checks
      if (!fnLen || fnLen > maxlen || (                                              // wrong size
            fnExt && extension && (                                                  // extension-based checks follow...
              !isFileExtensionMatching(fnExt, extension) || (                            // wrong extension
                !(flags & LIST_SD_FILE_EXT) &&                                       // only if we want unique file names...
                strcasecmp(fnExt, getFileExtension(extension)) &&                    // possible duplicate file name...
                isFilePatternAvailable(path, fno.getName(), extension, true, tmpExt) &&  // find the first file from extensions list...
                strncasecmp(fnExt, tmpExt, LEN_FILE_EXTENSION_MAX)                   // found file doesn't match, this is a duplicate
              )
            )
          ))
      {
        continue;
      }

      popupMenuItemsCount++;

      std::string fname = fno.getName();
      if (!(flags & LIST_SD_FILE_EXT)) {
        fname = fname.substr(0,fnLen);  // strip extension
      }

      if (popupMenuOffset == 0) {
        if (selection && strncasecmp(fname.c_str(), selection, maxlen) < 0) {
          lastpopupMenuOffset++;
        }
        else {
          for (uint8_t i=0; i<MENU_MAX_DISPLAY_LINES; i++) {
            char * line = reusableBuffer.modelsel.menu_bss[i];
            if (line[0] == '\0' || strcasecmp(fname.c_str(), line) < 0) {
              if (i < MENU_MAX_DISPLAY_LINES-1) memmove(reusableBuffer.modelsel.menu_bss[i+1], line, sizeof(reusableBuffer.modelsel.menu_bss[i]) * (MENU_MAX_DISPLAY_LINES-1-i));
              memset(line, 0, MENU_LINE_LENGTH);
              strcpy(line, fname.c_str());
              break;
            }
          }
        }
        for (uint8_t i=0; i<min(popupMenuItemsCount, (uint16_t)MENU_MAX_DISPLAY_LINES); i++) {
          popupMenuItems[i] = reusableBuffer.modelsel.menu_bss[i];
        }
      }
      else if (lastpopupMenuOffset == 0xffff) {
        for (int i=MENU_MAX_DISPLAY_LINES-1; i>=0; i--) {
          char * line = reusableBuffer.modelsel.menu_bss[i];
          if (line[0] == '\0' || strcasecmp(fname.c_str(), line) > 0) {
            if (i > 0) memmove(reusableBuffer.modelsel.menu_bss[0], reusableBuffer.modelsel.menu_bss[1], sizeof(reusableBuffer.modelsel.menu_bss[i]) * i);
            memset(line, 0, MENU_LINE_LENGTH);
            strcpy(line, fname.c_str());
            break;
          }
        }
        for (uint8_t i=0; i<min(popupMenuItemsCount, (uint16_t)MENU_MAX_DISPLAY_LINES); i++) {
          popupMenuItems[i] = reusableBuffer.modelsel.menu_bss[i];
        }
      }
      else if (popupMenuOffset > lastpopupMenuOffset) {
        if (strcasecmp(fname.c_str(), reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-2]) > 0 && strcasecmp(fname.c_str(), reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1]) < 0) {
          memset(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], 0, MENU_LINE_LENGTH);
          strcpy(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], fname.c_str());
        }
      }
      else {
        if (strcasecmp(fname.c_str(), reusableBuffer.modelsel.menu_bss[1]) < 0 && strcasecmp(fname.c_str(), reusableBuffer.modelsel.menu_bss[0]) > 0) {
          memset(reusableBuffer.modelsel.menu_bss[0], 0, MENU_LINE_LENGTH);
          strcpy(reusableBuffer.modelsel.menu_bss[0], fname.c_str());
        }
      }
    }
    dir.close();
  }

  if (popupMenuOffset > 0)
    lastpopupMenuOffset = popupMenuOffset;
  else
    popupMenuOffset = lastpopupMenuOffset;

  return popupMenuItemsCount;
	return 0;
}

#endif // !LIBOPENUI

bool VirtualFS::sdCardMounted()
{
#if defined (SDCARD)
  return sdFatFs.fs_type != 0;
#else
  return false;
#endif
}
size_t VirtualFS::sdGetFreeSectors()
{
#if defined (SDCARD)
  return ::sdGetFreeSectors();
#else
  return 0;
#endif
}

void VirtualFS::startLogs()
{
  if(!sdCardMounted())
    return;
#if defined(SDCARD)
#if defined(LOG_TELEMETRY)
  openFile(g_telemetryFile, LOGS_PATH "/telemetry.log", VfsOpenFlags::OPEN_ALWAYS | VfsOpenFlagsWRITE);
  if (g_telemetryFile.size() > 0) {
    g_telemetryFile.lseek(g_telemetryFile.size()); // append
  }
#endif

#if defined(LOG_BLUETOOTH)
  openFile(g_bluetoothFile, LOGS_PATH "/bluetooth.log", VfsOpenFlags::OPEN_ALWAYS | VfsOpenFlagsWRITE);
  if (&g_bluetoothFile.size() > 0) {
    g_bluetoothFile.lseek(g_bluetoothFile.size()); // append
  }
#endif
#endif
}

void VirtualFS::stopLogs()
{
#if defined(SDCARD)
#if defined(LOG_TELEMETRY)
  g_telemetryFile.close();
#endif

#if defined(LOG_BLUETOOTH)
  g_bluetoothFile.close();
#endif
#endif
}
