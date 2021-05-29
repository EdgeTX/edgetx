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
#include <stdint.h>
#include "opentx.h"
#include "diskio.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "libopenui/src/libopenui_file.h"
#endif

bool sdCardFormat()
{
  BYTE work[FF_MAX_SS];
  FRESULT res = f_mkfs("", FM_FAT32, 0, work, sizeof(work));
  switch(res) {
    case FR_OK :
      return true;
    case FR_DISK_ERR:
      POPUP_WARNING("Format error");
      return false;
    case FR_NOT_READY:
      POPUP_WARNING("SDCard not ready");
      return false;
    case FR_WRITE_PROTECTED:
      POPUP_WARNING("SDCard write protected");
      return false;
    case FR_INVALID_PARAMETER:
      POPUP_WARNING("Format param invalid");
      return false;
    case FR_INVALID_DRIVE:
      POPUP_WARNING("Invalid drive");
      return false;
    case FR_MKFS_ABORTED:
      POPUP_WARNING("Format aborted");
      return false;
    default:
      POPUP_WARNING(STR_SDCARD_ERROR);
      return false;
  }
}

const char * sdCheckAndCreateDirectory(const char * path)
{
  DIR archiveFolder;

  FRESULT result = f_opendir(&archiveFolder, path);
  if (result != FR_OK) {
    if (result == FR_NO_PATH)
      result = f_mkdir(path);
    if (result != FR_OK)
      return SDCARD_ERROR(result);
  }
  else {
    f_closedir(&archiveFolder);
  }

  return nullptr;
}

bool isFileAvailable(const char * path, bool exclDir)
{
  if (exclDir) {
    FILINFO fno;
    return (f_stat(path, &fno) == FR_OK && !(fno.fattrib & AM_DIR));
  }
  return f_stat(path, nullptr) == FR_OK;
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
bool isFilePatternAvailable(const char * path, const char * file, const char * pattern = nullptr, bool exclDir = true, char * match = nullptr)
{
  uint8_t fplen;
  char fqfp[LEN_FILE_PATH_MAX + FF_MAX_LFN + 1] = "\0";

  fplen = strlen(path);
  if (fplen > LEN_FILE_PATH_MAX) {
    TRACE_ERROR("isFilePatternAvailable(%s) = error: path too long.\n", path);
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

char * getFileIndex(char * filename, unsigned int & value)
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

uint8_t getDigitsCount(unsigned int value)
{
  uint8_t count = 1;
  while (value >= 10) {
    value /= 10;
    ++count;
  }
  return count;
}

unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory)
{
  unsigned int index;
  uint8_t extlen;
  char * indexPos = getFileIndex(filename, index);
  char extension[LEN_FILE_EXTENSION_MAX+1] = "\0";
  char * p = (char *)getFileExtension(filename, 0, 0, nullptr, &extlen);
  if (p) strncat(extension, p, sizeof(extension)-1);
  while (true) {
    index++;
    if ((indexPos - filename) + getDigitsCount(index) + extlen > size) {
      return 0;
    }
    char * pos = strAppendUnsigned(indexPos, index);
    strAppend(pos, extension);
    if (!isFilePatternAvailable(directory, filename, nullptr, false)) {
      return index;
    }
  }
}

const char * getBasename(const char * path)
{
  for (int8_t i = strlen(path) - 1; i >= 0; i--) {
    if (path[i] == '/') {
      return &path[i + 1];
    }
  }
  return path;
}

#if !defined(LIBOPENUI)
bool sdListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags)
{
  static uint16_t lastpopupMenuOffset = 0;
  FILINFO fno;
  DIR dir;
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

  FRESULT res = f_opendir(&dir, path);
  if (res == FR_OK) {

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
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR) continue;            /* Skip subfolders */
      if (fno.fattrib & AM_HID) continue;            /* Skip hidden files */
      if (fno.fattrib & AM_SYS) continue;            /* Skip system files */

      fnExt = getFileExtension(fno.fname, 0, 0, &fnLen, &extLen);
      fnLen -= extLen;

//      TRACE_DEBUG("listSdFiles(%s, %s, %u, %s, %u): fn='%s'; fnExt='%s'; match=%d\n",
//           path, extension, maxlen, (selection ? selection : "nul"), flags, fno.fname, (fnExt ? fnExt : "nul"), (fnExt && isExtensionMatching(fnExt, extension)));
      // file validation checks
      if (!fnLen || fnLen > maxlen || (                                              // wrong size
            fnExt && extension && (                                                  // extension-based checks follow...
              !isExtensionMatching(fnExt, extension) || (                            // wrong extension
                !(flags & LIST_SD_FILE_EXT) &&                                       // only if we want unique file names...
                strcasecmp(fnExt, getFileExtension(extension)) &&                    // possible duplicate file name...
                isFilePatternAvailable(path, fno.fname, extension, true, tmpExt) &&  // find the first file from extensions list...
                strncasecmp(fnExt, tmpExt, LEN_FILE_EXTENSION_MAX)                   // found file doesn't match, this is a duplicate
              )
            )
          ))
      {
        continue;
      }

      popupMenuItemsCount++;

      if (!(flags & LIST_SD_FILE_EXT)) {
        fno.fname[fnLen] = '\0';  // strip extension
      }

      if (popupMenuOffset == 0) {
        if (selection && strncasecmp(fno.fname, selection, maxlen) < 0) {
          lastpopupMenuOffset++;
        }
        else {
          for (uint8_t i=0; i<MENU_MAX_DISPLAY_LINES; i++) {
            char * line = reusableBuffer.modelsel.menu_bss[i];
            if (line[0] == '\0' || strcasecmp(fno.fname, line) < 0) {
              if (i < MENU_MAX_DISPLAY_LINES-1) memmove(reusableBuffer.modelsel.menu_bss[i+1], line, sizeof(reusableBuffer.modelsel.menu_bss[i]) * (MENU_MAX_DISPLAY_LINES-1-i));
              memset(line, 0, MENU_LINE_LENGTH);
              strcpy(line, fno.fname);
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
          if (line[0] == '\0' || strcasecmp(fno.fname, line) > 0) {
            if (i > 0) memmove(reusableBuffer.modelsel.menu_bss[0], reusableBuffer.modelsel.menu_bss[1], sizeof(reusableBuffer.modelsel.menu_bss[i]) * i);
            memset(line, 0, MENU_LINE_LENGTH);
            strcpy(line, fno.fname);
            break;
          }
        }
        for (uint8_t i=0; i<min(popupMenuItemsCount, (uint16_t)MENU_MAX_DISPLAY_LINES); i++) {
          popupMenuItems[i] = reusableBuffer.modelsel.menu_bss[i];
        }
      }
      else if (popupMenuOffset > lastpopupMenuOffset) {
        if (strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-2]) > 0 && strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1]) < 0) {
          memset(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], 0, MENU_LINE_LENGTH);
          strcpy(reusableBuffer.modelsel.menu_bss[MENU_MAX_DISPLAY_LINES-1], fno.fname);
        }
      }
      else {
        if (strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[1]) < 0 && strcasecmp(fno.fname, reusableBuffer.modelsel.menu_bss[0]) > 0) {
          memset(reusableBuffer.modelsel.menu_bss[0], 0, MENU_LINE_LENGTH);
          strcpy(reusableBuffer.modelsel.menu_bss[0], fno.fname);
        }
      }
    }
    f_closedir(&dir);
  }

  if (popupMenuOffset > 0)
    lastpopupMenuOffset = popupMenuOffset;
  else
    popupMenuOffset = lastpopupMenuOffset;

  return popupMenuItemsCount;
}

constexpr uint32_t TEXT_FILE_MAXSIZE = 2048;

void sdReadTextFile(const char * filename, char lines[TEXT_VIEWER_LINES][LCD_COLS + 1], int & lines_count)
{
  FIL file;
  int result;
  char c;
  unsigned int sz;
  int line_length = 0;
  uint8_t escape = 0;
  char escape_chars[4] = {0};
  int current_line = 0;

  memclear(lines, TEXT_VIEWER_LINES * (LCD_COLS + 1));

  result = f_open(&file, filename, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    for (uint32_t i = 0; i < TEXT_FILE_MAXSIZE && f_read(&file, &c, 1, &sz) == FR_OK && sz == 1 && (lines_count == 0 || current_line - menuVerticalOffset < int(TEXT_VIEWER_LINES)); i++) {
      if (c == '\n') {
        ++current_line;
        line_length = 0;
        escape = 0;
      }
      else if (c != '\r' && current_line >= menuVerticalOffset && current_line - menuVerticalOffset < int(TEXT_VIEWER_LINES) && line_length < LCD_COLS) {
        if (c == '\\' && escape == 0) {
          escape = 1;
          continue;
        }
        else if (c != '\\' && escape > 0 && escape < sizeof(escape_chars)) {
          escape_chars[escape - 1] = c;
          if (escape == 2 && !strncmp(escape_chars, "up", 2)) {
            c = CHAR_UP;
          }
          else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
            c = CHAR_DOWN;
          }
          else if (escape == 3) {
            int val = atoi(escape_chars);
            if (val >= 200 && val < 225) {
              c = '\200' + val - 200;
            }
          }
          else {
            escape++;
            continue;
          }
        }
        else if (c=='~') {
          c = 'z'+1;
        }
        else if (c=='\t') {
          c = 0x1D; //tab
        }
        escape = 0;
        lines[current_line-menuVerticalOffset][line_length++] = c;
      }
    }
    if (c != '\n') {
      current_line += 1;
    }
    f_close(&file);
  }

  if (lines_count == 0) {
    lines_count = current_line;
  }
}
#endif // !LIBOPENUI

#if defined(SDCARD)
const char * sdCopyFile(const char * srcPath, const char * destPath)
{
  FIL srcFile;
  FIL destFile;
  char buf[256];
  UINT read = sizeof(buf);
  UINT written = sizeof(buf);

  FRESULT result = f_open(&srcFile, srcPath, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  result = f_open(&destFile, destPath, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) {
    f_close(&srcFile);
    return SDCARD_ERROR(result);
  }

  while (result==FR_OK && read==sizeof(buf) && written==sizeof(buf)) {
    result = f_read(&srcFile, buf, sizeof(buf), &read);
    if (result == FR_OK) {
      result = f_write(&destFile, buf, read, &written);
    }
  }

  f_close(&destFile);
  f_close(&srcFile);

  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  return nullptr;
}

const char * sdCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir)
{
  char srcPath[2*CLIPBOARD_PATH_LEN+1];
  char * tmp = strAppend(srcPath, srcDir, CLIPBOARD_PATH_LEN);
  *tmp++ = '/';
  strAppend(tmp, srcFilename, CLIPBOARD_PATH_LEN);

  char destPath[2*CLIPBOARD_PATH_LEN+1];
  tmp = strAppend(destPath, destDir, CLIPBOARD_PATH_LEN);
  *tmp++ = '/';
  strAppend(tmp, destFilename, CLIPBOARD_PATH_LEN);

  return sdCopyFile(srcPath, destPath);
}
#endif // defined(SDCARD)


#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdGetNoSectors()
{
  static DWORD noSectors = 0;
  if (noSectors == 0 ) {
    disk_ioctl(0, GET_SECTOR_COUNT, &noSectors);
  }
  return noSectors;
}

uint32_t sdGetSize()
{
  return (sdGetNoSectors() / 1000000) * BLOCK_SIZE;
}

uint32_t sdGetFreeSectors()
{
  DWORD nofree;
  FATFS * fat;
  if (f_getfree("", &nofree, &fat) != FR_OK) {
    return 0;
  }
  return nofree * fat->csize;
}

#else  // #if !defined(SIMU) || defined(SIMU_DISKIO)

uint32_t sdGetNoSectors()
{
  return 0;
}

uint32_t sdGetSize()
{
  return 0;
}

uint32_t sdGetFreeSectors()
{
  return 10;
}

#endif  // #if !defined(SIMU) || defined(SIMU_DISKIO)
