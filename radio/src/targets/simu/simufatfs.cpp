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

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include "edgetx.h"

#if defined(SIMU_USE_SDCARD)  // rest of file is excluded otherwise

#if defined(_MSC_VER) || !defined (__GNUC__)
  #define MSVC_BUILD    1
#else
  #define MSVC_BUILD    0
#endif

// NOTE: the #include order is important here, sensitive on different platforms.
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>

#if MSVC_BUILD
  #include <direct.h>
  #include <stdlib.h>
  #include <sys/utime.h>
  #define mkdir(s, f) _mkdir(s)
#else
  #include <sys/time.h>
  #include <utime.h>
#endif

#include "ff.h"

namespace simu {
#include <dirent.h>
#if !defined(_MSC_VER)
  #include <libgen.h>
#endif
}

std::string simuSdDirectory;          // path to the root of the SD card image
std::string simuSettingsDirectory;    // path to the root of the models and settings (only for the radios that use SD for model storage)

bool isPathDelimiter(char delimiter)
{
  return delimiter == '/';
}

std::string removeTrailingPathDelimiter(const std::string & path)
{
  std::string result = path;
  while (!result.empty() && isPathDelimiter(result.back())) {
    result.pop_back();
  }
  return result;
}

std::string fixPathDelimiters(const char * path)
{
  // replace all '\' characters with '/'
  std::string result(path);
  std::replace(result.begin(), result.end(), '\\', '/');
  // TRACE_SIMPGMSPACE("fixPathDelimiters(): %s -> %s", path, result.c_str());
  return result;
}

void simuFatfsSetPaths(const char * sdPath, const char * settingsPath)
{
  if (sdPath) {
    simuSdDirectory = removeTrailingPathDelimiter(fixPathDelimiters(sdPath));
  }
  else {
    char buff[1024];
    f_getcwd(buff, sizeof(buff)-1);
    simuSdDirectory = removeTrailingPathDelimiter(fixPathDelimiters(buff));
  }
  if (settingsPath) {
    simuSettingsDirectory = removeTrailingPathDelimiter(fixPathDelimiters(settingsPath));
  }
  TRACE_SIMPGMSPACE("simuFatfsSetPaths(): simuSdDirectory: \"%s\"", simuSdDirectory.c_str());
  TRACE_SIMPGMSPACE("simuFatfsSetPaths(): simuSettingsDirectory: \"%s\"", simuSettingsDirectory.c_str());
}

bool startsWith(const std::string & str, const std::string & prefix)
{
  return str.length() >= prefix.length() &&
         str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string & str, const std::string & suffix)
{
  return str.length() >= suffix.length() &&
         str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

bool redirectToSettingsDirectory(const std::string & path)
{
  /*
    Decide if we use special simuSettingsDirectory path or normal path

    We use special path for:
      * radio settings and models list in /RADIO directory
      * model (*.bin) files in /MODELS directory
  */
  if (!simuSettingsDirectory.empty()) {
    if (path == MODELS_PATH || path == RADIO_PATH) return true;
    if (startsWith(path, MODELS_PATH) && endsWith(path, MODELS_EXT))
      return true;
    if (path == MODELSLIST_YAML_PATH || path == RADIO_SETTINGS_YAML_PATH || path == RADIO_SETTINGS_TMPFILE_YAML_PATH || path == RADIO_SETTINGS_ERRORFILE_YAML_PATH)
      return true;
    if (startsWith(path, MODELS_PATH) && endsWith(path, YAML_EXT)) return true;
  }
  return false;
}

std::string convertToSimuPath(const char * path)
{
  std::string result;
  if (isPathDelimiter(path[0])) {
    if (redirectToSettingsDirectory(path)) {
      // TRACE("REDIRECT ********************************************");
      result = simuSettingsDirectory + std::string(path);
    }
    else {
      result = simuSdDirectory + std::string(path);
    }
  }
  else {
    result = std::string(path);
  }
  // TRACE_SIMPGMSPACE("convertToSimuPath(): %s -> %s", path, result.c_str());
  return result;
}

std::string convertFromSimuPath(const char * path)
{
  std::string result;
  if (startsWith(path, simuSdDirectory)) {
    result = std::string(path).substr(simuSdDirectory.length(), std::string::npos);
    if (result.empty()) {
      result = "/";
    }
  }
  else {
    result = std::string(path);
    if (!result.empty() && !isPathDelimiter(result[0])) {
      result = "/" + result;
    }
  }
  // TRACE_SIMPGMSPACE("convertFromSimuPath(): %s -> %s", path, result.c_str());
  return result;
}

typedef std::map<std::string, std::string> filemap_t;

filemap_t fileMap;

void splitPath(const std::string & path, std::string & dir, std::string & name)
{
#if MSVC_BUILD
  char drive[_MAX_DRIVE];
  char directory[_MAX_DIR];
  char fname[_MAX_FNAME];
  char ext[_MAX_EXT];
  _splitpath(path.c_str(), drive, directory, fname, ext);
  name = std::string(fname) + std::string(ext);
  dir = std::string(drive) + std::string(directory);
#else
  char * buff = new char[path.length()+1];
  strcpy(buff, path.c_str());
  name = simu::basename(buff);
  strcpy(buff, path.c_str());
  dir = simu::dirname(buff);
  delete[] buff;
#endif
}


#if !MSVC_BUILD
bool isFile(const std::string & fullName)
{
//#if defined(WIN32) || defined(__APPLE__) || defined(__FreeBSD__)
//  #define REGULAR_FILE DT_REG
//  #define SYMBOLIC_LINK DT_LNK
//#else
//  #define REGULAR_FILE simu::DT_REG
//  #define SYMBOLIC_LINK simu::DT_LNK
//#endif

  //if (d_type == REGULAR_FILE) return true;
  //if (d_type == SYMBOLIC_LINK) {
    struct stat tmp;
    if (stat(fullName.c_str(), &tmp) == 0) {
      // TRACE_SIMPGMSPACE("\tsymlik: %s is %s", fullName.c_str(), (tmp.st_mode & S_IFREG) ? "file" : "other");
      if (tmp.st_mode & S_IFREG) return true;
    }
  //}
  return false;
}
#endif

std::vector<std::string> listDirectoryFiles(const std::string & dirName)
{
  std::vector<std::string> result;

#if MSVC_BUILD
    std::string searchName = dirName + "*";
    // TRACE_SIMPGMSPACE("\tsearching for: %s", fileName.c_str());
    WIN32_FIND_DATA ffd;
    HANDLE hFind = FindFirstFile(searchName.c_str(), &ffd);
    if (INVALID_HANDLE_VALUE != hFind) {
      do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
          std::string fullName = dirName + std::string(ffd.cFileName);
          // TRACE_SIMPGMSPACE("listDirectoryFiles(): %s", fullName.c_str());
          result.push_back(fullName);
        }
      }
      while (FindNextFile(hFind, &ffd) != 0);
    }
#else
  simu::DIR * dir = simu::opendir(dirName.c_str());
  if (dir) {
    struct simu::dirent * res;
    while ((res = simu::readdir(dir)) != 0) {
      std::string fullName = dirName + "/" + std::string(res->d_name);
      if (isFile(fullName)) {
        // TRACE_SIMPGMSPACE("listDirectoryFiles(): %s", fullName.c_str());
        result.push_back(fullName);
      }
    }
    simu::closedir(dir);
  }
#endif
  return result;
}

std::string findTrueFileName(const std::string & path)
{
  // TRACE_SIMPGMSPACE("findTrueFileName(%s)", path.c_str());
  std::string result;
  filemap_t::iterator i = fileMap.find(path);
  if (i != fileMap.end()) {
    result = i->second;
    // TRACE_SIMPGMSPACE("\tfound in map: %s", result.c_str());
    return result;
  }
  else {
    //find file and add to map
    std::string dirName;
    std::string fileName;
    splitPath(path, dirName, fileName);
    std::vector<std::string> files = listDirectoryFiles(dirName);
    for(unsigned int i=0; i<files.size(); ++i) {
      if (!strcasecmp(files[i].c_str(), path.c_str())) {
        // TRACE_SIMPGMSPACE("\tfound: %s", files[i].c_str());
        fileMap.insert(filemap_t::value_type(path, files[i]));
        return files[i];
      }
    }
  }
  TRACE_SIMPGMSPACE("\tnot found");
  return std::string(path);
}

FRESULT f_stat (const TCHAR * name, FILINFO *fno)
{
  std::string path = convertToSimuPath(name);
  std::string realPath = findTrueFileName(path);
  struct stat tmp;
  if (stat(realPath.c_str(), &tmp)) {
    TRACE_SIMPGMSPACE("f_stat(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
    return FR_INVALID_NAME;
  }
  else {
    TRACE_SIMPGMSPACE("f_stat(%s) = OK", path.c_str());
    if (fno) {
      fno->fattrib = (tmp.st_mode & S_IFDIR) ? AM_DIR : 0;
      // convert to FatFs fdate/ftime
      struct tm *ltime = localtime(&tmp.st_mtime);
      fno->fdate = ((ltime->tm_year - 80) << 9) | ((ltime->tm_mon + 1) << 5) | ltime->tm_mday;
      fno->ftime = (ltime->tm_hour << 11) | (ltime->tm_min << 5) | (ltime->tm_sec / 2);
      fno->fsize = (DWORD)tmp.st_size;
    }
    return FR_OK;
  }
}

FRESULT f_mount (FATFS* ,const TCHAR*, BYTE opt)
{
  return FR_OK;
}

FRESULT f_open (FIL * fil, const TCHAR *name, BYTE flag)
{
  std::string path = convertToSimuPath(name);
  std::string realPath = findTrueFileName(path);
  fil->obj.fs = 0;
  if (!(flag & FA_WRITE)) {
    struct stat tmp;
    if (stat(realPath.c_str(), &tmp)) {
      TRACE_SIMPGMSPACE("f_open(%s) = INVALID_NAME (FIL %p)", path.c_str(), fil);
      return FR_INVALID_NAME;
    }
    fil->obj.objsize = tmp.st_size;
    fil->fptr = 0;
  }
  fil->obj.fs = (FATFS*)fopen(realPath.c_str(), (flag & FA_WRITE) ? ((flag & FA_CREATE_ALWAYS) ? "wb+" : "ab+") : "rb");
  fil->fptr = 0;
  if (fil->obj.fs) {
    TRACE_SIMPGMSPACE("f_open(%s, %x) = %p (FIL %p)", path.c_str(), flag, fil->obj.fs, fil);
    return FR_OK;
  }
  TRACE_SIMPGMSPACE("f_open(%s) = error %d (%s) (FIL %p)", path.c_str(), errno, strerror(errno), fil);
  return FR_INVALID_NAME;
}

FRESULT f_read (FIL* fil, void* data, UINT size, UINT* read)
{
  if (fil && fil->obj.fs) {
    *read = fread(data, 1, size, (FILE*)fil->obj.fs);
    fil->fptr += *read;
    // TRACE_SIMPGMSPACE("fread(%p) %u, %u", fil->obj.fs, size, *read);
  }
  return FR_OK;
}

FRESULT f_write (FIL* fil, const void* data, UINT size, UINT* written)
{
  if (fil && fil->obj.fs) {
    *written = fwrite(data, 1, size, (FILE*)fil->obj.fs);
    fil->fptr += size;
    // TRACE_SIMPGMSPACE("fwrite(%p) %u, %u", fil->obj.fs, size, *written);
  }
  return FR_OK;
}

TCHAR * f_gets (TCHAR* buff, int len, FIL* fil)
{
  if (fil && fil->obj.fs) {
    buff = fgets(buff, len, (FILE*)fil->obj.fs);
    if (buff != nullptr) {
      fil->fptr = *buff;
    }
    // TRACE_SIMPGMSPACE("fgets(%p) %u, %s", fil->obj.fs, len, buff);
  }
  return buff;
}

FRESULT f_lseek (FIL* fil, DWORD offset)
{
  if (fil && fil->obj.fs) {
    fseek((FILE*)fil->obj.fs, offset, SEEK_SET);
    fil->fptr = offset;
  }
  return FR_OK;
}

UINT f_size(FIL* fil)
{
  if (fil && fil->obj.fs) {
    long curr = ftell((FILE*)fil->obj.fs);
    fseek((FILE*)fil->obj.fs, 0, SEEK_END);
    long size = ftell((FILE*)fil->obj.fs);
    fseek((FILE*)fil->obj.fs, curr, SEEK_SET);
    TRACE_SIMPGMSPACE("f_size(%p) %u", fil->obj.fs, size);
    return size;
  }
  return 0;
}

FRESULT f_close (FIL * fil)
{
  TRACE_SIMPGMSPACE("f_close(%p) (FIL:%p)", fil->obj.fs, fil);
  if (fil->obj.fs) {
    fclose((FILE*)fil->obj.fs);
    fil->obj.fs = nullptr;
  }
  return FR_OK;
}

FRESULT f_chdir (const TCHAR *name)
{
  std::string path = convertToSimuPath(name);
  if (chdir(path.c_str())) {
    TRACE_SIMPGMSPACE("f_chdir(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
    return FR_NO_PATH;
  }
  TRACE_SIMPGMSPACE("f_chdir(%s)", path.c_str());
  return FR_OK;
}

struct _simu_DIR
{
    std::string name;
    simu::DIR*  dir;

    _simu_DIR(simu::DIR* dir, const TCHAR* name) :
        name(name), dir(dir)
    {}
};

FRESULT f_opendir (DIR * rep, const TCHAR * name)
{
  std::string path = convertToSimuPath(name);
  simu::DIR* dp = simu::opendir(path.c_str());
  if (dp) {
    rep->obj.fs = reinterpret_cast<FATFS*>(new _simu_DIR(dp, name));
    TRACE_SIMPGMSPACE("f_opendir(%s) = OK", path.c_str());
    return FR_OK;
  }

  rep->obj.fs = nullptr;
  TRACE_SIMPGMSPACE("f_opendir(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
  return FR_NO_PATH;
}

FRESULT f_closedir (DIR * rep)
{
  TRACE_SIMPGMSPACE("f_closedir(%p)", rep);
  if (rep->obj.fs) {
    _simu_DIR* sd = reinterpret_cast<_simu_DIR*>(rep->obj.fs);
    rep->obj.fs = nullptr;
    simu::closedir(sd->dir);
    delete sd;
  }
  return FR_OK;
}

FRESULT f_readdir (DIR * rep, FILINFO * fil)
{
  simu::dirent * ent;
  _simu_DIR* sd = reinterpret_cast<_simu_DIR*>(rep->obj.fs);

  if (!sd) return FR_NO_FILE;

  for(;;) {
    ent = simu::readdir(sd->dir);
    if (!ent) return FR_NO_FILE;
    if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") ) break;
  }

  if (fil != nullptr) {
      memset(fil->fname, 0, FF_MAX_LFN);
      strcpy(fil->fname, ent->d_name);
      // TRACE_SIMPGMSPACE("f_readdir(): %s", fil->fname);
      std::string fullName = sd->name + std::string("/") + std::string(ent->d_name);
      return f_stat(fullName.c_str(), fil);
  }

  return FR_OK;
}

FRESULT f_mkfs (const TCHAR* path, BYTE opt, DWORD au, void* work, UINT len)
{
  TRACE_SIMPGMSPACE("Format SD...");
  return FR_OK;
}

FRESULT f_mkdir (const TCHAR * name)
{
  std::string path = convertToSimuPath(name);
  if (f_stat(name, nullptr) != FR_OK) {
#if defined(WIN32) && defined(__GNUC__)
    if (mkdir(path.c_str())) {
#else
    if (mkdir(path.c_str(), 0777)) {
#endif
      TRACE_SIMPGMSPACE("mkdir(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
      return FR_INVALID_NAME;
    }
    else {
      TRACE_SIMPGMSPACE("mkdir(%s) = OK", path.c_str());
      return FR_OK;
    }
  } else {
    return FR_EXIST;
  }
  return FR_OK;
}

FRESULT f_unlink (const TCHAR * name)
{
  std::string path = convertToSimuPath(name);
  if (isFile(path)) {
    if (unlink(path.c_str())) {
      TRACE_SIMPGMSPACE("f_unlink(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
      return FR_INVALID_NAME;
    }
    else {
      TRACE_SIMPGMSPACE("f_unlink(%s) = OK", path.c_str());
      return FR_OK;
    }
  } else {
    if (rmdir(path.c_str())) {
      TRACE_SIMPGMSPACE("f_unlink(%s) = error %d (%s)", path.c_str(), errno, strerror(errno));
      return FR_INVALID_NAME;
    }
    else {
      TRACE_SIMPGMSPACE("f_unlink(%s) = OK", path.c_str());
      return FR_OK;
    }
  }
}

FRESULT f_rename(const TCHAR *oldname, const TCHAR *newname)
{
  std::string old = convertToSimuPath(oldname);
  std::string path = convertToSimuPath(newname);

  if (rename(old.c_str(), path.c_str()) < 0) {
    TRACE_SIMPGMSPACE("f_rename(%s, %s) = error %d (%s)", old.c_str(), path.c_str(), errno, strerror(errno));
    return FR_INVALID_NAME;
  }
  TRACE_SIMPGMSPACE("f_rename(%s, %s) = OK", old.c_str(), path.c_str());
  return FR_OK;
}

FRESULT f_utime(const TCHAR* path, const FILINFO* fno)
{
  if (fno == nullptr)
    return FR_INVALID_PARAMETER;

  std::string simpath = convertToSimuPath(path);
  std::string realPath = findTrueFileName(simpath);
  struct utimbuf newTimes;
  struct tm ltime;

  // convert from FatFs fdate/ftime
  ltime.tm_year = ((fno->fdate >> 9) & 0x7F) + 80;
  ltime.tm_mon = ((fno->fdate >> 5) & 0xF) - 1;
  ltime.tm_mday = (fno->fdate & 0x1F);
  ltime.tm_hour = ((fno->ftime >> 11) & 0x1F);
  ltime.tm_min = ((fno->ftime >> 5) & 0x3F);
  ltime.tm_sec = (fno->ftime & 0x1F) * 2;
  ltime.tm_isdst = -1;  // force mktime() to check dst

  newTimes.modtime = mktime(&ltime);
  newTimes.actime = newTimes.modtime;

  if (utime(realPath.c_str(), &newTimes)) {
    TRACE_SIMPGMSPACE("f_utime(%s) = error %d (%s)", simpath.c_str(), errno, strerror(errno));
    return FR_DENIED;
  }
  else {
    TRACE_SIMPGMSPACE("f_utime(%s) set mtime = %s", simpath.c_str(), ctime(&newTimes.modtime));
    return FR_OK;
  }
}

int f_putc (TCHAR c, FIL * fil)
{
  if (fil && fil->obj.fs) fwrite(&c, 1, 1, (FILE*)fil->obj.fs);
  return FR_OK;
}

int f_puts (const TCHAR * str, FIL * fil)
{
  int n;
  for (n = 0; *str; str++, n++) {
    if (f_putc(*str, fil) == EOF) return EOF;
  }
  return n;
}

int f_printf (FIL *fil, const TCHAR * format, ...)
{
  va_list arglist;
  va_start(arglist, format);
  if (fil && fil->obj.fs) vfprintf((FILE*)fil->obj.fs, format, arglist);
  va_end(arglist);
  return 0;
}

FRESULT f_getcwd (TCHAR *path, UINT sz_path)
{
  char cwd[1024];
  if (!getcwd(cwd, 1024)) {
    TRACE_SIMPGMSPACE("f_getcwd() = getcwd() error %d (%s)", errno, strerror(errno));
    strcpy(path, ".");
    return FR_NO_PATH;
  }

  std::string result = convertFromSimuPath(fixPathDelimiters(cwd).c_str());
  if (result.length() > sz_path) {
    //TRACE_SIMPGMSPACE("f_getcwd(): buffer too short");
    return FR_NOT_ENOUGH_CORE;
  }

  strcpy(path, result.c_str());
  TRACE_SIMPGMSPACE("f_getcwd() = %s", path);
  return FR_OK;
}

FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs)
{
  // just fake that we always have some clusters free
  *nclst = 10;
  return FR_OK;
}

#include "hal/storage.h"

void storageInit() {}
void storageDeInit() {}
void storagePreMountHook() {}
bool storageIsPresent() { return true; }

#endif  // #if defined(SIMU_USE_SDCARD)
