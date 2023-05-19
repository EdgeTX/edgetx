/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#pragma once

#include <string>
#include <cstring>
#include <cinttypes>
#include <memory>

constexpr uint8_t LEN_FILE_EXTENSION_MAX = 5;  // longest used, including the dot, excluding null term.

const char * getFileExtension(const char * filename, uint8_t size = 0, uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
bool isFileExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);

enum class OUiFsError {
  OK,
  ERROR,
};

class OpenUiFileInfo
{
public:
  virtual std::string getName() = 0;
  virtual bool isDir() = 0;
  virtual bool isHidden() = 0;
  virtual bool isSystem() = 0;

protected:
  OpenUiFileInfo(){}
  virtual ~OpenUiFileInfo(){}

private:
  OpenUiFileInfo(const OpenUiFileInfo&) = delete;
};

typedef std::shared_ptr<OpenUiFileInfo> OpenUiFileInfoP;

class OpenUiFile
{
public:
  virtual size_t size() = 0;
  virtual void close() = 0;
  virtual OUiFsError read(void* buf, size_t size, size_t& readSize) = 0;
  virtual OUiFsError lseek(size_t offset)  = 0;
  virtual size_t tell() = 0;
  virtual int eof() = 0;

protected:
  OpenUiFile(){}
  virtual ~OpenUiFile(){}
private:
  OpenUiFile(const OpenUiFile&) = delete;
};

typedef std::shared_ptr<OpenUiFile> OpenUiFileP;

class OpenUiDir
{
public:
  virtual OUiFsError read(OpenUiFileInfoP& fileInfo) = 0;
protected:
  OpenUiDir(){}
  virtual ~OpenUiDir(){}
private:
  OpenUiDir(const OpenUiDir&) = delete;
};

typedef std::shared_ptr<OpenUiDir> OpenUiDirP;

enum class OUiFsOpenFlags {
  NONE          = 0x00,
  READ          = 0x01,
  OPEN_EXISTING = 0x00,
};

//OUiFsOpenFlags operator|(OUiFsOpenFlags lhs,OUiFsOpenFlags rhs)
//OUiFsOpenFlags operator&(OUiFsOpenFlags lhs,OUiFsOpenFlags rhs);
inline OUiFsOpenFlags operator|(OUiFsOpenFlags lhs,OUiFsOpenFlags rhs)
{
  typedef typename
    std::underlying_type<OUiFsOpenFlags>::type underlying;
  return static_cast<OUiFsOpenFlags>(
    static_cast<underlying>(lhs)
  | static_cast<underlying>(rhs));
}
inline OUiFsOpenFlags operator&(OUiFsOpenFlags lhs,OUiFsOpenFlags rhs)
{
  typedef typename
    std::underlying_type<OUiFsOpenFlags>::type underlying;
  return static_cast<OUiFsOpenFlags>(
    static_cast<underlying>(lhs)
  & static_cast<underlying>(rhs));
}

// the reference to the share pointer is by desgin, to allow the open function to set the pointer.
OUiFsError OUiOpenDir(OpenUiDirP& dir, const char* path);
OUiFsError OUiOpenFile(OpenUiFileP& file, const char* path, OUiFsOpenFlags flags);
const char * OUiGetFileExtension(const char * filename, uint8_t size = 0,
                                        uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
bool OUiGisFileExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);
