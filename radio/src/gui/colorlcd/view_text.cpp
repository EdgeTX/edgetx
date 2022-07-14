/*
 * Copyright (C) EdgeTX
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

#include "view_text.h"

#include "gridlayout.h"
#include "menu.h"
#include "opentx.h"
#include "sdcard.h"

void ViewTextWindow::extractNameSansExt()
{
  uint8_t nameLength;
  uint8_t extLength;

  const char *ext =
      getFileExtension(name.c_str(), 0, 0, &nameLength, &extLength);
  extension = std::string(ext);
  if (nameLength > TEXT_FILENAME_MAXLEN) nameLength = TEXT_FILENAME_MAXLEN;

  nameLength -= extLength;
  name.substr(nameLength);
  openFromEnd = !strcmp(ext, LOGS_EXT);
}

void ViewTextWindow::buildBody(Window *window)
{
  FILINFO info;

  if (buffer) {
    free(buffer);
    buffer = nullptr;
    bufSize = 0;
  }

  auto res = f_stat((TCHAR *)fullPath.c_str(), &info);
  if (res == FR_OK) {
    fileLength = int(info.fsize);
    bufSize = std::min(fileLength, maxTxtBuffSize) + 1;

    buffer = (char *)malloc(bufSize);
    if (buffer) {
      offset =
          std::max(int(openFromEnd ? int(info.fsize) - bufSize + 1 : 0), 0);
      TRACE("info.fsize=%d\tbufSize=%d\toffset=%d", info.fsize, bufSize,
            int(info.fsize) - bufSize + 1);
      if (sdReadTextFileBlock(fullPath.c_str(), bufSize, offset) == FR_OK) {
        auto obj = window->getLvObj();
        lv_obj_add_flag(
            obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_WITH_ARROW |
                     LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
        // prevents resetting the group's edit mode
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

        auto g = lv_group_get_default();
        lb = lv_label_create(obj);
        lv_obj_set_size(lb, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(lb, lv_dpx(8), 0);

        lv_group_add_obj(g, obj);
        lv_group_set_editing(g, true);
        lv_label_set_text_static(lb, buffer);

        if (openFromEnd)
          lv_obj_scroll_to_y(obj, LV_COORD_MAX, LV_ANIM_OFF);
        else
          lv_obj_scroll_to_y(obj, 0, LV_ANIM_OFF);
      }
    }
  }
}

FRESULT ViewTextWindow::sdReadTextFileBlock(const char *filename,
                                            const uint32_t bufSize,
                                            const uint32_t offset)
{
  FIL file;
  char escape_chars[4];
  int escape = 0;

  auto res = f_open(&file, (TCHAR *)filename, FA_OPEN_EXISTING | FA_READ);
  if (res == FR_OK) {
    res = f_lseek(&file, offset);
    if (res == FR_OK) {
      UINT br;
      char c;
      char *ptr = buffer;
      for (int i = 0; i < (int)bufSize; i++) {
        res = f_read(&file, &c, 1, &br);
        if (res == FR_OK && br == 1) {
          if (c == '\\' && escape == 0) {
            escape = 1;
            continue;
          } else if (c != '\\' && escape > 0 &&
                     escape < (int)sizeof(escape_chars)) {
            escape_chars[escape - 1] = c;

            if (escape == 2 && !strncmp(escape_chars, "up", 2)) {
              *ptr++ = STR_CHAR_UP[0];
              c = STR_CHAR_UP[1];
              escape = 0;
            } else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
              *ptr++ = STR_CHAR_DOWN[0];
              c = STR_CHAR_DOWN[1];
              escape = 0;
            } else if (escape == 3) {
              int val = atoi(escape_chars);
              if (val >= 200 && val < 225) {
                *ptr++ = '\302';
                c = '\200' + val - 200;
              }
            } else if (escape == 1 && c == '~') {
              c = 'z' + 1;
            } else {
              escape++;
              continue;
            }
          } else if (c == '\t') {
            c = 0x1D;  // tab
          }
          escape = 0;

          if (c == 0xA && *(ptr - 1) == 0xD) {
            *(ptr - 1) = '\n';
            continue;
          }
          *ptr++ = c;
        }
      }
      *ptr = '\0';
    }
    f_close(&file);
  }
  return res;
}

void ViewTextWindow::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (int(bufSize) < fileLength) {
    TRACE("BEFORE offset=%d", offset);
    if (event == EVT_KEY_BREAK(KEY_PGDN)) {
      offset += bufSize;
      TRACE("event=DOWN");
    }

    if (event == EVT_KEY_BREAK(KEY_PGUP)) {
      TRACE("event=UP");
      offset -= bufSize;
    }

    offset = std::max(offset, 0);
    offset = std::min(offset, fileLength - (int)bufSize);

    TRACE("AFTER offset=%d", offset);
    sdReadTextFileBlock(fullPath.c_str(), bufSize, offset);
    lv_label_set_text_static(lb, buffer);
  }
#endif
}

#include "datastructs.h"

static void replaceSpaceWithUnderscore(std::string &name)
{
  size_t index;
  do {
    index = name.find(' ');
    if (index != std::string::npos) name[index] = '_';
  } while (index != std::string::npos);
}

#if defined(SDCARD_YAML)
#define MODEL_FILE_EXT YAML_EXT
#else
#define MODEL_FILE_EXT MODELS_EXT
#endif

bool openNotes(const char buf[], std::string modelNotesName)
{
  if (isFileAvailable(modelNotesName.c_str())) {
    new ViewTextWindow(std::string(buf), modelNotesName, ICON_MODEL);
    return true;
  } else {
    return false;
  }
}
void readModelNotes()
{
  bool notesFound = false;
  LED_ERROR_BEGIN();

  std::string modelNotesName(g_model.header.name);
  modelNotesName.append(TEXT_EXT);
  const char buf[] = {MODELS_PATH};
  f_chdir((TCHAR *)buf);

  notesFound = openNotes(buf, modelNotesName);
  if (!notesFound) {
    replaceSpaceWithUnderscore(modelNotesName);
    notesFound = openNotes(buf, modelNotesName);
  }

#if !defined(EEPROM)
  if (!notesFound) {
    modelNotesName.assign(g_eeGeneral.currModelFilename);
    size_t index = modelNotesName.find(MODEL_FILE_EXT);
    if (index != std::string::npos) {
      modelNotesName.erase(index);
      modelNotesName.append(TEXT_EXT);
      notesFound = openNotes(buf, modelNotesName);
    }
    if (!notesFound) {
      replaceSpaceWithUnderscore(modelNotesName);
      notesFound = openNotes(buf, modelNotesName);
    }
  }
#endif

  LED_ERROR_END();
}
