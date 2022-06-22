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

#define CASE_EVT_KEY_NEXT_LINE \
  case EVT_ROTARY_RIGHT:       \
  case EVT_KEY_BREAK(KEY_PGDN)
//  case EVT_KEY_BREAK(KEY_DOWN)

#define CASE_EVT_KEY_PREVIOUS_LINE \
  case EVT_ROTARY_LEFT:            \
  case EVT_KEY_BREAK(KEY_PGUP):    \
  case EVT_KEY_BREAK(KEY_UP)

#define CASE_EVT_START           \
  case EVT_ENTRY:                \
  case EVT_KEY_BREAK(KEY_ENTER): \
  case EVT_KEY_BREAK(KEY_TELEM)

void ViewTextWindow::extractNameSansExt()
{
  uint8_t nameLength;
  uint8_t extLength;

  nameLength = name.length();
  //const char *ext =
  getFileExtension(name.c_str(), 0, 0, &nameLength, &extLength);
  if (nameLength > TEXT_FILENAME_MAXLEN) nameLength = TEXT_FILENAME_MAXLEN;

  nameLength -= extLength;
  name.substr(nameLength);
  // openFromEnd = !strcmp(ext, LOGS_EXT);
}

void ViewTextWindow::buildBody(Window *window)
{
  if (sdReadTextFile(fullpath.c_str()) != FR_OK) return;
  
  auto obj = window->getLvObj();
  lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_WITH_ARROW);
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);

  // prevents resetting the group's edit mode
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  auto g = lv_group_get_default();
  lv_group_add_obj(g, obj);
  lv_group_set_editing(g, true);

  auto label = lv_label_create(obj);
  lv_obj_set_size(label, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(label, lv_dpx(8), 0);

  lv_label_set_text_static(label, buffer);
}

int ViewTextWindow::sdReadTextFile(const char *filename)
{
  FIL file;
  FILINFO info;

  auto res = f_stat((TCHAR *)filename, &info);
  if (res != FR_OK) return res;

  // max 64KB
  if (info.fsize > 1024 * 64 - 1) return -1;

  buffer = (char*)malloc(info.fsize + 1);
  if (!buffer) return -1;

  res = f_open(&file, (TCHAR *)filename, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK) return res;

  UINT br;
  res = f_read(&file, buffer, info.fsize, &br);

  if (res != FR_OK) {
    f_close(&file);
    return -1;
  }

  buffer[br] = '\0';
  return FR_OK;
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
