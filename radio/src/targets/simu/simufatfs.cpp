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

#include "simulib.h"

#if !defined(SIMU_DISKIO)

#include "ff.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <string.h>

namespace fs = std::filesystem;

using sysclock = std::chrono::system_clock;
using ftime_type = fs::file_time_type;

// path to the root of the SD card image
static fs::path simuSdDirectory;

// path to the root of the models and settings
static fs::path simuSettingsDirectory;

// current simulater path
static fs::path simuCurrentPath;

static std::string to_lower(const std::string& s)
{
    std::string result(s.size(), '\0');
    std::transform(s.begin(), s.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

static sysclock::time_point ftime_to_systime(ftime_type ftime)
{
  return std::chrono::time_point_cast<sysclock::duration>(
      ftime - ftime_type::clock::now() + sysclock::now());
}

static ftime_type systime_to_ftime(sysclock::time_point systime)
{
  return std::chrono::time_point_cast<ftime_type::duration>(
      systime - sysclock::now() + ftime_type::clock::now());
}

static fs::path resolveCaseInsensitivePath(
    const fs::path& base,
    const fs::path& relativePath)
{
  fs::path current = base;
  std::error_code ec;

  for (const auto& component : relativePath) {
    std::string componentStr = component.string();

    // Skip current directory and parent directory references
    if (componentStr == "." || componentStr == "..") {
      current /= component;
      continue;
    }

    // Try exact match first
    fs::path exactPath = current / component;
    if (fs::exists(exactPath, ec) && !ec) {
      current = exactPath;
      continue;
    }

    // Look for case-insensitive match
    bool found = false;
    if (fs::is_directory(current, ec) && !ec) {
      std::string componentLower = to_lower(componentStr);

      for (const auto& entry : fs::directory_iterator(current, ec)) {
        if (ec) break;

        std::string entryName = entry.path().filename().string();
        if (to_lower(entryName) == componentLower) {
          current = entry.path();
          found = true;
          break;
        }
      }
    }

    // If not found, use original component name (file doesn't exist)
    if (!found) current /= component;
  }

  return current;
}

static fs::path makeAbsolute(const fs::path& path)
{
  return path.is_relative() ? simuCurrentPath / path : path;
}

static fs::path resolveInBase(const fs::path& base, const fs::path& virtualPath)
{
  return resolveCaseInsensitivePath(
      base, virtualPath.lexically_relative(virtualPath.root_path()));
}

// Resolve for read: try settingsPath first, fall back to sdPath
static std::string resolveForRead(const fs::path& path)
{
  fs::path p = makeAbsolute(path);

  if (!simuSettingsDirectory.empty()) {
    auto resolved = resolveInBase(simuSettingsDirectory, p);
    std::error_code ec;
    if (fs::exists(resolved, ec) && !ec) {
      return resolved.string();
    }
  }

  return resolveInBase(simuSdDirectory, p).string();
}

// Resolve for write: always use settingsPath if available
static std::string resolveForWrite(const fs::path& path)
{
  fs::path p = makeAbsolute(path);

  if (!simuSettingsDirectory.empty()) {
    return resolveInBase(simuSettingsDirectory, p).string();
  }

  return resolveInBase(simuSdDirectory, p).string();
}

void simuFatfsSetPaths(const char* sdPath, const char* settingsPath)
{
  simuCurrentPath = fs::path{"/"};

  if (sdPath && strlen(sdPath) > 0) {
    simuSdDirectory = fs::path{sdPath}.lexically_normal();
  } else if (simuSdDirectory.empty()) {
    simuSdDirectory = fs::current_path();
  }
  if (settingsPath && strlen(settingsPath) > 0) {
    simuSettingsDirectory =
        fs::path{settingsPath}.lexically_normal();
  }
}

std::string simuFatfsGetCurrentPath() { return simuCurrentPath.string(); }

std::string simuFatfsGetRealPath(const std::string &p)
{
  return resolveForRead(p);
}

FRESULT file_stat(const std::string& realPath, FILINFO* fno)
{
  std::error_code ec;
  fs::path fsPath(realPath);

  // Check if file/directory exists
  auto status = fs::status(fsPath, ec);
  if (ec || status.type() == fs::file_type::not_found) {
    return FR_INVALID_NAME;
  }

  if (fno) {
    // Set attributes
    fno->fattrib =
        (status.type() == fs::file_type::directory) ? AM_DIR : 0;

    // Get file size
    if (status.type() == fs::file_type::regular) {
      auto size = fs::file_size(fsPath, ec);
      fno->fsize = ec ? 0 : static_cast<DWORD>(size);
    } else {
      fno->fsize = 0;
    }

    // Get last write time
    auto ftime = fs::last_write_time(fsPath, ec);
    if (!ec) {
      auto systime = ftime_to_systime(ftime);
      auto time_t_val = std::chrono::system_clock::to_time_t(systime);
      struct tm* ltime = localtime(&time_t_val);

      if (ltime) {
        // Convert to FatFs format
        fno->fdate = ((ltime->tm_year - 80) << 9) | ((ltime->tm_mon + 1) << 5) |
                     ltime->tm_mday;
        fno->ftime =
            (ltime->tm_hour << 11) | (ltime->tm_min << 5) | (ltime->tm_sec / 2);
      } else {
        fno->fdate = 0;
        fno->ftime = 0;
      }
    } else {
      fno->fdate = 0;
      fno->ftime = 0;
    }
  }

  return FR_OK;
}

FRESULT f_stat(const TCHAR* name, FILINFO* fno)
{
  std::string realPath = resolveForRead(name);
  return file_stat(realPath, fno);
}

FRESULT f_mount(FATFS*, const TCHAR*, BYTE opt)
{
  return FR_OK;
}

struct _simu_FIL {
  std::unique_ptr<std::fstream> stream;
  std::string filepath;
  std::ios::openmode mode;

  _simu_FIL(const std::string& path, std::ios::openmode m) :
      filepath(path), mode(m)
  {
    stream = std::make_unique<std::fstream>(path, m);
  }
};

FRESULT f_open(FIL* fil, const TCHAR* name, BYTE flag)
{
  fil->obj.fs = nullptr;
  fil->fptr = 0;

  std::ios::openmode mode = std::ios::binary;
  std::string realPath;

  if (flag & FA_WRITE) {
    realPath = resolveForWrite(name);
    mode |= std::ios::out | std::ios::in;

    // Ensure parent directories exist in settingsPath,
    // but only if they exist in the overlay (either layer)
    std::error_code ec;
    fs::path virtualParent = makeAbsolute(fs::path{name}).parent_path();
    std::string readParent = resolveForRead(virtualParent);
    if (fs::is_directory(readParent, ec) && !ec) {
      fs::path parentPath = fs::path(realPath).parent_path();
      if (!parentPath.empty()) {
        fs::create_directories(parentPath, ec);
      }
    }

    if (flag & FA_CREATE_ALWAYS) {
      mode |= std::ios::trunc;
    } else {
      if (fs::exists(realPath, ec) && !ec) {
        mode |= std::ios::ate;  // Open at end
      } else {
        // Copy-on-write: if the file exists in sdPath, copy it to
        // settingsPath so appends start from the original content
        std::string readPath = resolveForRead(name);
        if (fs::exists(readPath, ec) && !ec) {
          fs::copy_file(readPath, realPath, ec);
          mode |= std::ios::ate;
        } else {
          mode |= std::ios::trunc;  // Create new file
        }
      }
    }
  } else {
    realPath = resolveForRead(name);
    mode |= std::ios::in;

    std::error_code ec;
    if (!fs::exists(realPath, ec) || ec) {
      return FR_INVALID_NAME;
    }

    auto size = fs::file_size(realPath, ec);
    if (!ec) {
      fil->obj.objsize = static_cast<DWORD>(size);
    }
  }

  auto simuFil = new _simu_FIL(realPath, mode);
  if (simuFil->stream->is_open()) {
    fil->obj.fs = reinterpret_cast<FATFS*>(simuFil);
    return FR_OK;
  } else {
    delete simuFil;
    return FR_INVALID_NAME;
  }
}

FRESULT f_close(FIL* fil)
{
  if (fil && fil->obj.fs) {
    delete reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    fil->obj.fs = nullptr;
  }
  return FR_OK;
}

FRESULT f_read(FIL* fil, void* data, UINT size, UINT* read)
{
  *read = 0;
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    if (sf->stream && sf->stream->is_open()) {
      sf->stream->read(static_cast<char*>(data), size);
      *read = static_cast<UINT>(sf->stream->gcount());
      fil->fptr += *read;
      sf->stream->clear();
    }
  }
  return FR_OK;
}

FRESULT f_write(FIL* fil, const void* data, UINT size, UINT* written)
{
  *written = 0;
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    if (sf->stream && sf->stream->is_open()) {
      auto pos_before = sf->stream->tellp();
      sf->stream->write(static_cast<const char*>(data), size);
      if (sf->stream->good()) {
        *written = size;
        fil->fptr += size;
      } else {
        auto pos_after = sf->stream->tellp();
        if (pos_after != std::streampos(-1) &&
            pos_before != std::streampos(-1)) {
          *written = static_cast<UINT>(pos_after - pos_before);
          fil->fptr += *written;
        }
      }
    }
  }
  return FR_OK;
}

FRESULT f_lseek(FIL* fil, DWORD offset)
{
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    if (sf->stream && sf->stream->is_open()) {
      sf->stream->seekg(offset, std::ios::beg);
      sf->stream->seekp(offset, std::ios::beg);
      fil->fptr = offset;
    }
  }
  return FR_OK;
}

UINT f_size(FIL* fil)
{
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);

    // Use filesystem to get size (more efficient)
    std::error_code ec;
    auto size = fs::file_size(sf->filepath, ec);
    if (!ec) {
      return static_cast<UINT>(size);
    }
  }
  return 0;
}

FRESULT f_chdir(const TCHAR *name)
{
  fs::path p = makeAbsolute(fs::path{name});

  std::error_code ec;
  auto realPath = resolveForRead(p);

  if (fs::exists(realPath, ec) && !ec) {
    simuCurrentPath = p;
    return FR_OK;
  }

  return FR_NO_PATH;
}

struct _simu_DIR {
  std::vector<fs::path> entries;
  size_t index = 0;

  void collectEntries(const std::string& dir, std::set<std::string>& seen)
  {
    std::error_code ec;
    if (!fs::is_directory(dir, ec) || ec) return;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
      if (ec) break;
      std::string nameLower = to_lower(entry.path().filename().string());
      if (seen.insert(nameLower).second) {
        entries.push_back(entry.path());
      }
    }
  }

  bool hasNext() const { return index < entries.size(); }

  std::optional<fs::path> getNext()
  {
    if (index >= entries.size()) return std::nullopt;
    return entries[index++];
  }
};

FRESULT f_opendir(DIR* rep, const TCHAR* name)
{
  fs::path p = makeAbsolute(fs::path{name});
  fs::path relative = p.lexically_relative(p.root_path());

  std::error_code ec;
  std::string settingsDir, sdDir;
  bool settingsExists = false, sdExists = false;

  if (!simuSettingsDirectory.empty()) {
    settingsDir = resolveCaseInsensitivePath(simuSettingsDirectory, relative).string();
    settingsExists = fs::is_directory(settingsDir, ec) && !ec;
  }
  sdDir = resolveCaseInsensitivePath(simuSdDirectory, relative).string();
  sdExists = fs::is_directory(sdDir, ec) && !ec;

  if (!settingsExists && !sdExists) {
    rep->obj.fs = nullptr;
    return FR_NO_PATH;
  }

  auto sd = new _simu_DIR();
  std::set<std::string> seen;
  // Settings entries take priority (added first)
  if (settingsExists) sd->collectEntries(settingsDir, seen);
  if (sdExists) sd->collectEntries(sdDir, seen);

  rep->obj.fs = reinterpret_cast<FATFS*>(sd);
  return FR_OK;
}

FRESULT f_closedir(DIR* rep)
{
  if (rep->obj.fs) {
    _simu_DIR* sd = reinterpret_cast<_simu_DIR*>(rep->obj.fs);
    rep->obj.fs = nullptr;
    delete sd;
  }
  return FR_OK;
}

FRESULT f_readdir(DIR* rep, FILINFO* fil)
{
  _simu_DIR* sd = reinterpret_cast<_simu_DIR*>(rep->obj.fs);

  if (!sd || !sd->hasNext()) {
    return FR_NO_FILE;
  }

  auto maybe_path = sd->getNext();
  if (!maybe_path.has_value()) {
    return FR_NO_FILE;
  }

  auto entryPath = maybe_path.value();
  if (fil != nullptr) {
    memset(fil->fname, 0, FF_MAX_LFN);

    std::string filename = entryPath.filename().string();
    size_t copyLen = std::min(filename.length(), (size_t)(FF_MAX_LFN - 1));
    strncpy(fil->fname, filename.c_str(), copyLen);

    std::string fullPath = entryPath.string();
    return file_stat(fullPath, fil);
  }

  return FR_OK;
}

FRESULT f_mkfs(const TCHAR* path, BYTE opt, DWORD au, void* work, UINT len)
{
  return FR_OK;
}

FRESULT f_mkdir(const TCHAR* name)
{
  std::string path = resolveForWrite(name);
  std::error_code ec;

  // Create parent directories in settingsPath if they exist in the overlay
  fs::path virtualParent = makeAbsolute(fs::path{name}).parent_path();
  std::string readParent = resolveForRead(virtualParent);
  if (!fs::is_directory(readParent, ec) || ec) {
    return FR_NO_PATH;
  }
  fs::path parentPath = fs::path(path).parent_path();
  if (!parentPath.empty()) {
    fs::create_directories(parentPath, ec);
  }

  bool created = fs::create_directory(path, ec);
  if (ec) return FR_INVALID_NAME;

  return created ? FR_OK : FR_EXIST;
}

FRESULT f_unlink(const TCHAR * name)
{
  std::string path = resolveForWrite(name);
  std::error_code ec;

  bool removed = fs::remove(path, ec);
  if (ec) return FR_INVALID_NAME;

  return removed ? FR_OK : FR_INVALID_NAME;
}

FRESULT f_rename(const TCHAR *oldname, const TCHAR *newname)
{
  std::string old = resolveForWrite(oldname);
  std::string path = resolveForWrite(newname);
  std::error_code ec;

  fs::rename(old.c_str(), path.c_str(), ec);
  if (ec) return FR_INVALID_NAME;

  return FR_OK;
}

FRESULT f_utime(const TCHAR* path, const FILINFO* fno)
{
    if (fno == nullptr) {
        return FR_INVALID_PARAMETER;
    }

    std::string realPath = resolveForWrite(path);
    
    // Convert FatFs time to tm structure
    struct tm ltime = {};
    ltime.tm_year = ((fno->fdate >> 9) & 0x7F) + 80;
    ltime.tm_mon = ((fno->fdate >> 5) & 0xF) - 1;
    ltime.tm_mday = (fno->fdate & 0x1F);
    ltime.tm_hour = ((fno->ftime >> 11) & 0x1F);
    ltime.tm_min = ((fno->ftime >> 5) & 0x3F);
    ltime.tm_sec = (fno->ftime & 0x1F) * 2;
    ltime.tm_isdst = -1;

    time_t timestamp = mktime(&ltime);
    if (timestamp == -1) {
        return FR_INVALID_PARAMETER;
    }

    // Convert to file time and set it
    auto systime = std::chrono::system_clock::from_time_t(timestamp);
    auto ftime = systime_to_ftime(systime);

    std::error_code ec;
    fs::last_write_time(realPath, ftime, ec);

    return ec ? FR_DENIED : FR_OK;
}

int f_putc(TCHAR c, FIL* fil)
{
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    if (sf->stream && sf->stream->is_open()) {
      sf->stream->put(c);
      fil->fptr++;
    }
  }

  return EOF;
}

FRESULT f_getcwd(TCHAR *path, UINT sz_path)
{
  auto p_str = simuCurrentPath.generic_string();
  if (p_str.length() > sz_path) {
    return FR_NOT_ENOUGH_CORE;
  }

  strncpy(path, p_str.c_str(), sz_path + 1);
  return FR_OK;
}

FRESULT f_truncate(FIL* fil)
{
  if (fil && fil->obj.fs) {
    _simu_FIL* sf = reinterpret_cast<_simu_FIL*>(fil->obj.fs);
    if (sf->stream && sf->stream->is_open()) {
      sf->stream->flush();
      std::error_code ec;
      fs::resize_file(sf->filepath, fil->fptr, ec);
      if (ec) return FR_DISK_ERR;
    }
  }
  return FR_OK;
}

FRESULT f_getfree(const TCHAR* path, DWORD* nclst, FATFS** fatfs)
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
