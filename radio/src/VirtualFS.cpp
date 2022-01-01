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

#include "opentx.h"
#include "VirtualFS.h"


#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "libopenui/src/libopenui_file.h"
#endif

VirtualFS* VirtualFS::_instance = nullptr;;

size_t flashSpiRead(size_t address, uint8_t* data, size_t size);
size_t flashSpiWrite(size_t address, const uint8_t* data, size_t size);

int flashSpiErase(size_t address);
void flashSpiEraseAll();

void flashSpiSync();


extern "C"
{
int flashRead(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size)
{
  flashSpiRead((block * c->block_size) + off, (uint8_t*)buffer, size);
  return LFS_ERR_OK;
}

int flashWrite(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size)
{
  flashSpiWrite((block * c->block_size) + off, (uint8_t*)buffer, size);
  return LFS_ERR_OK;
}

int flashErase(const struct lfs_config *c, lfs_block_t block)
{
    flashSpiErase(block * c->block_size);
    return LFS_ERR_OK;
}

int flashSync(const struct lfs_config *c)
{
  flashSpiSync();
  return LFS_ERR_OK;
}
}

uint16_t flashSpiGetPageSize();
uint16_t flashSpiGetSectorSize();
uint16_t flashSpiGetSectorCount();

VirtualFS::VirtualFS()
{
  // configuration of the filesystem is provided by this struct
  lfsCfg.read  = flashRead;
  lfsCfg.prog  = flashWrite;
  lfsCfg.erase = flashErase;
  lfsCfg.sync  = flashSync;

  // block device configuration
  lfsCfg.read_size = 256;
  lfsCfg.prog_size = flashSpiGetPageSize();
  lfsCfg.block_size = flashSpiGetSectorSize();
  lfsCfg.block_count = flashSpiGetSectorCount();
  lfsCfg.block_cycles = 500;
  lfsCfg.cache_size = 2048;
  lfsCfg.lookahead_size = 32;

//  flashSpiEraseAll();
  int err = lfs_mount(&lfs, &lfsCfg);
  if(err) {
      flashSpiEraseAll();
      err = lfs_format(&lfs, &lfsCfg);
      if(err == LFS_ERR_OK)
        err = lfs_mount(&lfs, &lfsCfg);
      if(err != LFS_ERR_OK)
        return;
  }
  lfsCfg.context = this;
  checkAndCreateDirectory("/test");
  checkAndCreateDirectory("/test/foo");
  checkAndCreateDirectory("/anotherTest");
  lfs_file_t file;
  err = lfs_file_open(&lfs,  &file, "test/testFile.txt", LFS_O_CREAT|LFS_O_TRUNC|LFS_O_WRONLY);
  if(err == LFS_ERR_OK)
  {
    lfs_file_write(&lfs, &file, "Hello World\n", sizeof("Hello World\n"));
    lfs_file_close(&lfs, &file);
  }
}

VirtualFS::~VirtualFS()
{
  lfs_unmount(&lfs);
}


#ifdef LFS_THREADSAFE
// Lock the underlying block device. Negative error codes
// are propogated to the user.
int (*lock)(const struct lfs_config *c);

// Unlock the underlying block device. Negative error codes
// are propogated to the user.
int (*unlock)(const struct lfs_config *c);
#endif

bool VirtualFS::format()
{
  flashSpiEraseAll();
  lfs_format(&lfs, &lfsCfg);
  return true;

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

std::vector<std::string> tokenize(const char* seps, const std::string& data)
{
    std::vector<std::string> ret;
    size_t oldpos = 0;

    while (1)
    {
        size_t newpos = data.find_first_of(seps,oldpos);
        if(newpos == std::string::npos)
        {
            ret.push_back(data.substr(oldpos));
            break;
        }
        ret.push_back(data.substr(oldpos,newpos-oldpos));
        oldpos=newpos+1;
    }
    return ret;
}

VfsDir::DirType VirtualFS::getDirTypeAndPath(std::string& path)
{
  if(path == "/")
  {
    return VfsDir::DIR_ROOT;
  } else if(path.substr(0, 6) == "/FLASH")
  {
    path = path.substr(6);
    return VfsDir::DIR_LFS;
  } else {
    path = path.substr(7);
    return VfsDir::DIR_FAT;
  }
}

void VirtualFS::normalizePath(std::string& path)
{
  std::vector<std::string> tokens;
  size_t oldpos = 0;

  if(path[0] != '/')
    path = curWorkDir + PATH_SEPARATOR + path;


  while (1)
  {
      size_t newpos = path.find_first_of(PATH_SEPARATOR, oldpos);
      if(newpos == std::string::npos)
      {
        std::string elem = path.substr(oldpos);
        if(elem == "..")
          tokens.pop_back();
        else
          tokens.push_back(path.substr(oldpos));
        break;
      }
      tokens.push_back(path.substr(oldpos,newpos-oldpos));
      oldpos=newpos+1;
  }

  if(tokens.empty())
    return;

  path = "";
  for(auto token: tokens)
  {
    if(token.length() == 0)
      continue;
    path += PATH_SEPARATOR;
    path += token;
  }
  if(path.length() == 0)
    path = "/";
}

int VirtualFS::unlink(const std::string& path)
{
  std::string p = path;
  normalizePath(p);
  VfsDir::DirType type = getDirTypeAndPath(p);

  switch(type)
  {
  case VfsDir::DIR_ROOT:
    return -1;
  case VfsDir::DIR_FAT:
    return f_unlink(p.c_str());
    break;
  case VfsDir::DIR_LFS:
    return lfs_remove(&lfs, p.c_str());
  }

  return -1;
}

int VirtualFS::changeDirectory(const std::string& path)
{
  if(path.length() == 0)
    return -1;

  std::string newWorkDir = path;
  normalizePath(newWorkDir);
  curWorkDir = newWorkDir;

  return 0;
}

int VirtualFS::openDirectory(VfsDir& dir, const char * path)
{
  dir.clear();
  if(path == nullptr)
    return -1;

  if(path[0] == 0)
    return -1;

  std::string dirPath(path);

  normalizePath(dirPath);

  VfsDir::DirType type = getDirTypeAndPath(dirPath);
  dir.type = type;
  switch(type)
  {
  case VfsDir::DIR_ROOT:
    return 0;
  case VfsDir::DIR_LFS:
    return lfs_dir_open(&lfs, &dir.lfsDir, dirPath.c_str());
  case VfsDir::DIR_FAT:
    return f_opendir(&dir.fatDir, dirPath.c_str());
  }

  return -1;
}

int VirtualFS::readDirectory(VfsDir& dir, VfsFileInfo& info, bool firstTime)
{
  info.clear();
  switch(dir.type)
  {
  case VfsDir::DIR_ROOT:
    info.type = VfsFileType::ROOT;
    if(dir.readIdx == 0)
      info.name = "FLASH";
    else if(dir.readIdx == 1)
      info.name = "SDCARD";
    else
      info.name = "";
    dir.readIdx++;
    return 0;
  case VfsDir::DIR_FAT:
    info.type = VfsFileType::FAT;
    if(dir.readIdx == 0)
    {
      dir.readIdx++;
      info.name = "..";
      return 0;
    }
    return sdReadDir(&dir.fatDir, &info.fatInfo, dir.firstTime);
  case VfsDir::DIR_LFS:
    info.type = VfsFileType::LFS;
    return lfs_dir_read(&lfs, &dir.lfsDir, &info.lfsInfo);
  }
  return -1;
}

int VirtualFS::closeDirectory(VfsDir& dir)
{
  int ret = -1;
  switch(dir.type)
  {
  case VfsDir::DIR_ROOT:
    ret = 0;
    break;
  case VfsDir::DIR_FAT:
    ret = f_closedir(&dir.fatDir);
    break;
  case VfsDir::DIR_LFS:
    ret = lfs_dir_close(&lfs, &dir.lfsDir);
    break;
  }
  dir.clear();
  return ret;
}

int VirtualFS::rename(const char* oldPath, const char* newPath)
{
  std::string oldP = oldPath;
  std::string newP = newPath;

  normalizePath(oldP);
  normalizePath(newP);

  VfsDir::DirType oldType = getDirTypeAndPath(oldP);
  VfsDir::DirType newType = getDirTypeAndPath(newP);

  if(oldType == newType)
  {
    switch(oldType)
    {
    case VfsDir::DIR_ROOT:
      return -1;
    case VfsDir::DIR_FAT:
      f_rename(oldP.c_str(), newP.c_str());
      break;
    case VfsDir::DIR_LFS:
      return lfs_rename(&lfs, oldP.c_str(), newP.c_str());
      break;
    }
  } else {
    copyFile(oldPath, newPath);
    unlink(oldPath);
  }
  return 0;
}

int VirtualFS::copyFile(const std::string& source, const std::string& destination)
{
  VfsFile src;
  VfsFile dest;

  if(openFile(src, source, LFS_O_RDONLY) != LFS_ERR_OK)
    return -1;

  if(openFile(dest, destination, LFS_O_CREAT|LFS_O_TRUNC|LFS_O_RDWR) != LFS_ERR_OK)
    return -1;

  char buf[256] = {0};
  size_t readBytes = 0;
  size_t written = 0;

  read(src, buf, sizeof(buf), readBytes);
  for(; readBytes != 0; read(src, buf, sizeof(buf), readBytes))
    write(dest, buf, readBytes, written);

  closeFile(src);
  closeFile(dest);
  return 0;
}

int VirtualFS::copyFile(const std::string& srcFile, const std::string& srcDir,
           const std::string& destDir, const std::string& destFile)
{
  return copyFile(srcDir +"/" + srcFile, destDir + "/" + destFile);
}


int VirtualFS::openFile(VfsFile& file, const std::string& path, int flags)
{
  file.clear();
  std::string normPath(path);
  normalizePath(normPath);
  VfsDir::DirType dirType = getDirTypeAndPath(normPath);

  int ret = -1;
  switch(dirType)
  {
  case VfsDir::DIR_ROOT:
    ret = -1;
    break;
  case VfsDir::DIR_FAT:
  {
    int fatFlags = FA_OPEN_EXISTING | FA_READ;
    if( flags == (LFS_O_CREAT|LFS_O_TRUNC|LFS_O_RDWR ))
      fatFlags = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
    file.type = VfsFileType::FAT;
    ret = f_open(&file.fat, normPath.c_str(), fatFlags);
    break;
  }
  case VfsDir::DIR_LFS:
  {
    int lfsFlags = LFS_O_RDONLY;
    if( flags == (LFS_O_CREAT|LFS_O_TRUNC|LFS_O_RDWR ))
      lfsFlags = LFS_O_CREAT|LFS_O_TRUNC|LFS_O_RDWR;

    file.type = VfsFileType::LFS;
    ret = lfs_file_open(&lfs, &file.lfs, normPath.c_str(), lfsFlags);
    break;
  }
  }

  return ret;

}

int VirtualFS::closeFile(VfsFile& file)
{
  int ret = -1;
  switch(file.type)
  {
  case VfsFileType::FAT:
    ret = f_close(&file.fat);
    break;
  case VfsFileType::LFS:
    ret = lfs_file_close(&lfs, &file.lfs);
    break;
  }

  file.clear();
  return ret;
}

int VirtualFS::fileSize(VfsFile& file)
{
  switch(file.type)
  {
  case VfsFileType::FAT:
    return f_size(&file.fat);
    break;
  case VfsFileType::LFS:
    return lfs_file_size(&lfs, &file.lfs);
  }

  return -1;
}

int VirtualFS::read(VfsFile& file, void* buf, size_t size, size_t& readSize)
{
  int ret = -1;

  switch(file.type)
  {
  case VfsFileType::FAT:
    ret = f_read(&file.fat, buf, size, &readSize);
    break;
  case VfsFileType::LFS:
    ret = lfs_file_read(&lfs, &file.lfs, buf, size);
    if(ret >= 0)
    {
      readSize = ret;
      ret = 0;
    } else {
      readSize = 0;
    }
    break;
  }

  return ret;
}

int VirtualFS::write(VfsFile& file, void* buf, size_t size, size_t& written)
{
  int ret = -1;

  switch(file.type)
  {
  case VfsFileType::FAT:
    ret = f_write(&file.fat, buf, size, &written);
    break;
  case VfsFileType::LFS:
    ret = lfs_file_write(&lfs, &file.lfs, buf, size);
    if(ret >= 0)
    {
      written = ret;
      ret = 0;
    } else {
      written = 0;
    }
    break;
  }

  return ret;
}

int VirtualFS::lseek(VfsFile& file, size_t offset)
{
  int ret = -1;

  switch(file.type)
  {
  case VfsFileType::FAT:
    ret = f_lseek(&file.fat, offset);
    break;
  case VfsFileType::LFS:
    ret = lfs_file_seek(&lfs, &file.lfs, offset, LFS_SEEK_SET);
    break;
  }

  return ret;
}

int VirtualFS::fileEof(VfsFile& file)
{
  switch(file.type)
  {
  case VfsFileType::FAT:
    return f_eof(&file.fat);
  case VfsFileType::LFS:
    return lfs_file_tell(&lfs, &file.lfs) == lfs_file_size(&lfs, &file.lfs);
  }

  return 0;
}

const char* VirtualFS::checkAndCreateDirectory(const char * path)
{
	lfs_dir_t dir;
	int res = lfs_dir_open(&lfs, &dir, path);
	if(res != LFS_ERR_OK)
	{
	  if(res == LFS_ERR_NOENT)
	    res = lfs_mkdir(&lfs, path);
	  if(res != LFS_ERR_OK)
	  {
	    //      return SDCARD_ERROR(result);
	    return "ERROR";
	  }
	} else {
	  lfs_dir_close(&lfs, &dir);
	}

//  DIR archiveFolder;
//
//  FRESULT result = f_opendir(&archiveFolder, path);
//  if (result != FR_OK) {
//    if (result == FR_NO_PATH)
//      result = f_mkdir(path);
//    if (result != FR_OK)
//      return SDCARD_ERROR(result);
//  }
//  else {
//    f_closedir(&archiveFolder);
//  }
//
  return nullptr;
}

bool VirtualFS::isFileAvailable(const char * path, bool exclDir)
{
  lfs_file_t file;
  int res = lfs_file_open(&lfs, &file, path, LFS_O_RDONLY);
  if(res != LFS_ERR_OK)
  {
    if(res == LFS_ERR_ISDIR)
      return(!exclDir);
    return false;
  } else {
    lfs_file_close(&lfs, &file);
  }
  return true;
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
//  uint8_t fplen;
//  char fqfp[LEN_FILE_PATH_MAX + FF_MAX_LFN + 1] = "\0";
//
//  fplen = strlen(path);
//  if (fplen > LEN_FILE_PATH_MAX) {
//    TRACE_ERROR("isFilePatternAvailable(%s) = error: path too long.\n", path);
//    return false;
//  }
//
//  strcpy(fqfp, path);
//  strcpy(fqfp + fplen, "/");
//  strncat(fqfp + (++fplen), file, FF_MAX_LFN);
//
//  if (pattern == nullptr) {
//    // no extensions list, just check the filename as-is
//    return isFileAvailable(fqfp, exclDir);
//  }
//  else {
//    // extensions list search
//    const char *ext;
//    uint16_t len;
//    uint8_t extlen, fnlen;
//    int plen;
//
//    getFileExtension(file, 0, 0, &fnlen, &extlen);
//    len = fplen + fnlen - extlen;
//    fqfp[len] = '\0';
//    ext = getFileExtension(pattern, 0, 0, &fnlen, &extlen);
//    plen = (int)fnlen;
//    while (plen > 0 && ext) {
//      strncat(fqfp + len, ext, extlen);
//      if (isFileAvailable(fqfp, exclDir)) {
//        if (match != nullptr) strncat(&(match[0]='\0'), ext, extlen);
//        return true;
//      }
//      plen -= extlen;
//      if (plen > 0) {
//        fqfp[len] = '\0';
//        ext = getFileExtension(pattern, plen, 0, nullptr, &extlen);
//      }
//    }
//  }
  return false;
}

char* VirtualFS::getFileIndex(char * filename, unsigned int & value)
{
//  value = 0;
//  char * pos = (char *)getFileExtension(filename);
//  if (!pos || pos == filename)
//    return nullptr;
//  int multiplier = 1;
//  while (pos > filename) {
//    pos--;
//    char c = *pos;
//    if (c >= '0' && c <= '9') {
//      value += multiplier * (c - '0');
//      multiplier *= 10;
//    }
//    else {
//      return pos+1;
//    }
//  }
//  return filename;
	return nullptr;
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
//  unsigned int index;
//  uint8_t extlen;
//  char * indexPos = getFileIndex(filename, index);
//  char extension[LEN_FILE_EXTENSION_MAX+1] = "\0";
//  char * p = (char *)getFileExtension(filename, 0, 0, nullptr, &extlen);
//  if (p) strncat(extension, p, sizeof(extension)-1);
//  while (true) {
//    index++;
//    if ((indexPos - filename) + _getDigitsCount(index) + extlen > size) {
//      return 0;
//    }
//    char * pos = strAppendUnsigned(indexPos, index);
//    strAppend(pos, extension);
//    if (!isFilePatternAvailable(directory, filename, nullptr, false)) {
//      return index;
//    }
//  }
	return 0;
}

//static const char * getBasename(const char * path)
//{
//  for (int8_t i = strlen(path) - 1; i >= 0; i--) {
//    if (path[i] == '/') {
//      return &path[i + 1];
//    }
//  }
//  return path;
//}

#if !defined(LIBOPENUI)
bool flashListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags)
{
//  static uint16_t lastpopupMenuOffset = 0;
//  FILINFO fno;
//  DIR dir;
//  const char * fnExt;
//  uint8_t fnLen, extLen;
//  char tmpExt[LEN_FILE_EXTENSION_MAX+1] = "\0";
//
//  popupMenuOffsetType = MENU_OFFSET_EXTERNAL;
//
//  static uint8_t s_last_flags;
//
//  if (selection) {
//    s_last_flags = flags;
//    if (!isFilePatternAvailable(path, selection, ((flags & LIST_SD_FILE_EXT) ? nullptr : extension))) selection = nullptr;
//  }
//  else {
//    flags = s_last_flags;
//  }
//
//  if (popupMenuOffset == 0) {
//    lastpopupMenuOffset = 0;
//    memset(reusableBuffer.modelsel.menu_bss, 0, sizeof(reusableBuffer.modelsel.menu_bss));
//  }
//  else if (popupMenuOffset == popupMenuItemsCount - MENU_MAX_DISPLAY_LINES) {
//    lastpopupMenuOffset = 0xffff;
//    memset(reusableBuffer.modelsel.menu_bss, 0, sizeof(reusableBuffer.modelsel.menu_bss));
//  }
//  else if (popupMenuOffset == lastpopupMenuOffset) {
//    // should not happen, only there because of Murphy's law
//    return true;
//  }
//  else if (popupMenuOffset > lastpopupMenuOffset) {
//    memmove(reusableBuffer.modelsel.menu_bss[0], reusableBuffer.modelsel.menu_bss[1], (MENU_MAX_DISPLAY_LINES-1)*MENU_LINE_LENGTH);
//    memset(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], 0xff, MENU_LINE_LENGTH);
//  }
//  else {
//    memmove(reusableBuffer.modelsel.menu_bss[1], reusableBuffer.modelsel.menu_bss[0], (MENU_MAX_DISPLAY_LINES-1)*MENU_LINE_LENGTH);
//    memset(reusableBuffer.modelsel.menu_bss[0], 0, MENU_LINE_LENGTH);
//  }
//
//  popupMenuItemsCount = 0;
//
//  FRESULT res = f_opendir(&dir, path);
//  if (res == FR_OK) {
//
//    if (flags & LIST_NONE_SD_FILE) {
//      popupMenuItemsCount++;
//      if (selection) {
//        lastpopupMenuOffset++;
//      }
//      else if (popupMenuOffset==0 || popupMenuOffset < lastpopupMenuOffset) {
//        char * line = reusableBuffer.modelsel.menu_bss[0];
//        memset(line, 0, MENU_LINE_LENGTH);
//        strcpy(line, "---");
//        popupMenuItems[0] = line;
//      }
//    }
//
//    for (;;) {
//      res = f_readdir(&dir, &fno);                   /* Read a directory item */
//      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
//      if (fno.fattrib & AM_DIR) continue;            /* Skip subfolders */
//      if (fno.fattrib & AM_HID) continue;            /* Skip hidden files */
//      if (fno.fattrib & AM_SYS) continue;            /* Skip system files */
//
//      fnExt = getFileExtension(fno.fname, 0, 0, &fnLen, &extLen);
//      fnLen -= extLen;
//
////      TRACE_DEBUG("listSdFiles(%s, %s, %u, %s, %u): fn='%s'; fnExt='%s'; match=%d\n",
////           path, extension, maxlen, (selection ? selection : "nul"), flags, fno.fname, (fnExt ? fnExt : "nul"), (fnExt && isExtensionMatching(fnExt, extension)));
//      // file validation checks
//      if (!fnLen || fnLen > maxlen || (                                              // wrong size
//            fnExt && extension && (                                                  // extension-based checks follow...
//              !isExtensionMatching(fnExt, extension) || (                            // wrong extension
//                !(flags & LIST_SD_FILE_EXT) &&                                       // only if we want unique file names...
//                strcasecmp(fnExt, getFileExtension(extension)) &&                    // possible duplicate file name...
//                isFilePatternAvailable(path, fno.fname, extension, true, tmpExt) &&  // find the first file from extensions list...
//                strncasecmp(fnExt, tmpExt, LEN_FILE_EXTENSION_MAX)                   // found file doesn't match, this is a duplicate
//              )
//            )
//          ))
//      {
//        continue;
//      }
//
//      popupMenuItemsCount++;
//
//      if (!(flags & LIST_SD_FILE_EXT)) {
//        fno.fname[fnLen] = '\0';  // strip extension
//      }
//
//      if (popupMenuOffset == 0) {
//        if (selection && strncasecmp(fno.fname, selection, maxlen) < 0) {
//          lastpopupMenuOffset++;
//        }
//        else {
//          for (uint8_t i=0; i<MENU_MAX_DISPLAY_LINES; i++) {
//            char * line = reusableBuffer.modelsel.menu_bss[i];
//            if (line[0] == '\0' || strcasecmp(fno.fname, line) < 0) {
//              if (i < MENU_MAX_DISPLAY_LINES-1) memmove(reusableBuffer.modelsel.menu_bss[i+1], line, sizeof(reusableBuffer.modelsel.menu_bss[i]) * (MENU_MAX_DISPLAY_LINES-1-i));
//              memset(line, 0, MENU_LINE_LENGTH);
//              strcpy(line, fno.fname);
//              break;
//            }
//          }
//        }
//        for (uint8_t i=0; i<min(popupMenuItemsCount, (uint16_t)MENU_MAX_DISPLAY_LINES); i++) {
//          popupMenuItems[i] = reusableBuffer.modelsel.menu_bss[i];
//        }
//      }
//      else if (lastpopupMenuOffset == 0xffff) {
//        for (int i=MENU_MAX_DISPLAY_LINES-1; i>=0; i--) {
//          char * line = reusableBuffer.modelsel.menu_bss[i];
//          if (line[0] == '\0' || strcasecmp(fno.fname, line) > 0) {
//            if (i > 0) memmove(reusableBuffer.modelsel.menu_bss[0], reusableBuffer.modelsel.menu_bss[1], sizeof(reusableBuffer.modelsel.menu_bss[i]) * i);
//            memset(line, 0, MENU_LINE_LENGTH);
//            strcpy(line, fno.fname);
//            break;
//          }
//        }
//        for (uint8_t i=0; i<min(popupMenuItemsCount, (uint16_t)MENU_MAX_DISPLAY_LINES); i++) {
//          popupMenuItems[i] = reusableBuffer.modelsel.menu_bss[i];
//        }
//      }
//      else if (popupMenuOffset > lastpopupMenuOffset) {
//        if (strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-2]) > 0 && strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1]) < 0) {
//          memset(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], 0, MENU_LINE_LENGTH);
//          strcpy(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], fno.fname);
//        }
//      }
//      else {
//        if (strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[1]) < 0 && strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[0]) > 0) {
//          memset(reusableBuffer.modelsel.menu_bss[0], 0, MENU_LINE_LENGTH);
//          strcpy(reusableBuffer.modelsel.menu_bss[0], fno.fname);
//        }
//      }
//    }
//    f_closedir(&dir);
//  }
//
//  if (popupMenuOffset > 0)
//    lastpopupMenuOffset = popupMenuOffset;
//  else
//    popupMenuOffset = lastpopupMenuOffset;
//
//  return popupMenuItemsCount;
//	return 0;
}

#endif // !LIBOPENUI

#if defined(SDCARD)&& 0
const char * flashCopyFile(const char * srcPath, const char * destPath)
{
//  FIL srcFile;
//  FIL destFile;
//  char buf[256];
//  UINT read = sizeof(buf);
//  UINT written = sizeof(buf);
//
//  FRESULT result = f_open(&srcFile, srcPath, FA_OPEN_EXISTING | FA_READ);
//  if (result != FR_OK) {
//    return SDCARD_ERROR(result);
//  }
//
//  result = f_open(&destFile, destPath, FA_CREATE_ALWAYS | FA_WRITE);
//  if (result != FR_OK) {
//    f_close(&srcFile);
//    return SDCARD_ERROR(result);
//  }
//
//  while (result==FR_OK && read==sizeof(buf) && written==sizeof(buf)) {
//    result = f_read(&srcFile, buf, sizeof(buf), &read);
//    if (result == FR_OK) {
//      result = f_write(&destFile, buf, read, &written);
//    }
//  }
//
//  f_close(&destFile);
//  f_close(&srcFile);
//
//  if (result != FR_OK) {
//    return SDCARD_ERROR(result);
//  }
//
  return nullptr;
}

const char * flashCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir)
{
//  char srcPath[2*CLIPBOARD_PATH_LEN+1];
//  char * tmp = strAppend(srcPath, srcDir, CLIPBOARD_PATH_LEN);
//  *tmp++ = '/';
//  strAppend(tmp, srcFilename, CLIPBOARD_PATH_LEN);
//
//  char destPath[2*CLIPBOARD_PATH_LEN+1];
//  tmp = strAppend(destPath, destDir, CLIPBOARD_PATH_LEN);
//  *tmp++ = '/';
//  strAppend(tmp, destFilename, CLIPBOARD_PATH_LEN);
//
//  return sdCopyFile(srcPath, destPath);
	return nullptr;
}
#endif // defined(SDCARD)


#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t flashGetNoSectors()
{
  static DWORD noSectors = 0;
//  if (noSectors == 0 ) {
//    disk_ioctl(0, GET_SECTOR_COUNT, &noSectors);
//  }
  return noSectors;
}

//uint32_t flashGetSize()
//{
//  return (flashGetNoSectors() / 1000000) * BLOCK_SIZE;
//}
//
//uint32_t flashGetFreeSectors()
//{
////  DWORD nofree;
////  FATFS * fat;
////  if (f_getfree("", &nofree, &fat) != FR_OK) {
////    return 0;
////  }
////  return nofree * fat->csize;
//  return 10;
//}

#else  // #if !defined(SIMU) || defined(SIMU_DISKIO)

uint32_t flashGetNoSectors()
{
  return 0;
}

uint32_t flashGetSize()
{
  return 0;
}

uint32_t flashGetFreeSectors()
{
  return 10;
}

#endif  // #if !defined(SIMU) || defined(SIMU_DISKIO)
