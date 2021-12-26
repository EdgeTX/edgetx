/*
 * Copyright (C) EdgeTX
 *
 * Source:
 *  https://github.com/EdgeTX/libopenui
 *
 * Based on code named
 *   opentx - https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#if !defined(_OPENUI_CONF_H)
#define _OPENUI_CONF_H

#include "VirtualFS.h"

#define OpenUiFile      VfsFile
#define OpenUiDir       VfsDir
#define OpenUiFileInfo  VfsFileInfo
#define OpenUiFsRetType VfsError

#define OPENUI_FS_OPEN_FLAG_READ         VfsOpenFlags::READ
#define OPENUI_FS_OPEN_FLAG_OPENEXISTING VfsOpenFlags::OPEN_EXISTING

#define openUiOpenFile(fileHandle, path, flags)       (VirtualFS::instance().openFile(*(fileHandle), path, flags))
#define openUiCloseFile(fileHandle)                   ((fileHandle)->close())
#define openUiReadFile(fileHandle, buf, bytes, read)  ((fileHandle)->read(buf, bytes, read))
#define openUiFileGetSize(fileHandle)                 ((fileHandle)->size())
#define openUiFileLSeek(fileHandle, offset)           ((fileHandle)->lseek(offset))

#define openUiOpenDir(dirHandle, path)     (VirtualFS::instance().openDirectory(*(dirHandle), path))
#define openUiCloseDir(dirHandle)          ((dirHandle)->close())
#define openUiReadDir(dirHandle, fileInfo) ((dirHandle)->read(*(fileInfo)))

#define openUiFsIsDir(fileInfo)          ((fileInfo)->getType() == VfsType::DIR)
#define openUiFsIsHiddenFile(fileHandle) (false)
#define openUiFsIsSystemFile(fileHandle) (false)
#define openUiFsGetName(fileInfo)        ((fileInfo)->getName())

#define openUiGetFileExtension(filename, size, extMaxLen, fnlen, extlen) (VirtualFS::getFileExtension(filename, size, extMaxLen, fnlen, extlen))
#define openUiIsFileExtensionMatching(extension, pattern, match) (VirtualFS::isFileExtensionMatching(extension, pattern, match))

#define OPENUI_FS_OK VfsError::OK

#endif // _OPENUI_CONF_H
