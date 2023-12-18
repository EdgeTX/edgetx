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

#include <algorithm>

#include "libopenui_file.h"
#include "menu.h"
#include "theme.h"
#include "message_dialog.h"

#if !defined(STR_SDCARD)
#define STR_SDCARD "SD"
#endif

#if !defined(STR_NO_FILES_ON_SD)
#define STR_NO_FILES_ON_SD "No files on SD Card!"
#endif

FileChoice::FileChoice(Window *parent, const rect_t &rect, std::string folder,
                       const char *extension, int maxlen,
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

std::string FileChoice::getLabelText() { return getValue(); }

bool FileChoice::openMenu()
{
  FILINFO fno;
  DIR dir;
  std::list<std::string> files;
  const char *fnExt;
  uint8_t fnLen, extLen;

  FRESULT res = f_opendir(&dir, folder.c_str());  // Open the directory
  if (res == FR_OK) {
    bool firstTime = true;
    for (;;) {
      res = sdReadDir(&dir, &fno, firstTime);
      if (res != FR_OK || fno.fname[0] == 0)
        break;  // break on error or end of dir
      if (fno.fattrib & (AM_HID | AM_SYS | AM_DIR))
        continue;  // Ignore subfolders, hidden files and system files
      if (fno.fname[0] == '.' && fno.fname[1] != '.')
        continue;  // Ignore hidden files under UNIX, but not ..

      fnExt = getFileExtension(fno.fname, 0, 0, &fnLen, &extLen);

      if (extension && (!fnExt || !isExtensionMatching(fnExt, extension)))
        continue;  // wrong extension

      if (stripExtension) fnLen -= extLen;

      if (!fnLen || fnLen > maxlen) continue;  // wrong size

      // eject duplicates
      std::string newFile(fno.fname, fnLen);
      if (std::find(files.begin(), files.end(), newFile) != files.end())
        continue;

      files.emplace_back(newFile);
    }

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

void FileChoice::onClicked() { openMenu(); }
