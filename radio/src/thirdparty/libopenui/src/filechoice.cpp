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

#include "filechoice.h"
#include "strhelpers.h"
#include "menu.h"
#include "theme.h"
#include "message_dialog.h"
#include "libopenui_conf.h"
#include <algorithm>

#if !defined(STR_SDCARD)
  #define STR_SDCARD "SD"
#endif

#if !defined(STR_NO_FILES_ON_SD)
  #define STR_NO_FILES_ON_SD "No files on SD Card!"
#endif

FileChoice::FileChoice(Window* parent, const rect_t &rect,
                       std::string folder, const char *extension, int maxlen,
                       std::function<std::string()> getValue,
                       std::function<void(std::string)> setValue,
                       bool stripExtension) :
    ChoiceBase(parent, rect, CHOICE_TYPE_FOLDER),
    folder(std::move(folder)),
    extension(extension),
    maxlen(maxlen),
    getValue(std::move(getValue)),
    setValue(std::move(setValue)),
    stripExtension(stripExtension)
{
  lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
}

std::string FileChoice::getLabelText()
{
  return getValue();
}


bool FileChoice::openMenu()
{
  OpenUiFileInfo fno;
  OpenUiDir dir;
  std::list<std::string> files;
  const char *fnExt;
  uint8_t fnLen, extLen;

  OpenUiFsRetType res = openUiOpenDir(&dir, folder.c_str());  // Open the directory
  if (res == OPENUI_FS_OK) {
    for (;;) {
      res = openUiReadDir(&dir, &fno);
      if (res != OPENUI_FS_OK || strlen(openUiFsGetName(&fno)) == 0)
        break;                             // break on error or end of dir
      if (openUiFsIsDir(&fno))        continue;  // skip subfolders
      if (openUiFsIsHiddenFile(&fno)) continue;  // skip hidden files
      if (openUiFsIsSystemFile(&fno)) continue;  // skip system files

      fnExt = openUiGetFileExtension(openUiFsGetName(&fno), 0, 0, &fnLen, &extLen);

      if (extension && (!fnExt || !openUiIsFileExtensionMatching(fnExt, extension, nullptr)))
        continue;  // wrong extension

      if (stripExtension) fnLen -= extLen;

      if (!fnLen || fnLen > maxlen) continue;  // wrong size

      // eject duplicates
      const char* newFile = fno.getName();
      if (std::find(files.begin(), files.end(), newFile) != files.end())
        continue;

      files.emplace_back(newFile);
    }
    openUiCloseDir(&dir);

    if (!files.empty()) {
      // sort files
      files.sort(compare_nocase);
      files.push_front("");

      auto menu = new Menu(this);
      int count = 0;
      int current = -1;
      std::string value = getValue();
      for (const auto &file : files) {
        menu->addLineBuffered(file, [=]() {
            setValue(file);
            lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
        });
        // TRACE("%s %d %s %d", value.c_str(), value.size(), file.c_str(),
        // file.size());
        if (value.compare(file) == 0) {
          // TRACE("OK");
          current = count;
        }
        ++count;
      }
      menu->updateLines();

      if (current >= 0) {
        menu->select(current);
      }

      menu->setCloseHandler([=]() {
        editMode = false;
        // setFocus(SET_FOCUS_DEFAULT);
      });

      return true;
    }
  }

  new MessageDialog(this, STR_SDCARD, STR_NO_FILES_ON_SD);

  return false;
}

void FileChoice::onClicked()
{
  openMenu();
}
