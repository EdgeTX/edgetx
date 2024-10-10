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

#include "edgetx.h"
#include "lib_file.h"
#include "menu.h"
#include "menutoolbar.h"

class FileChoiceMenuToolbar : public MenuToolbar
{
 public:
  FileChoiceMenuToolbar(FileChoice *choice, Menu *menu) :
      MenuToolbar(choice, menu, FC_COLS)
  {
    filterButton(choice, 'a', 'd', "aA-dD");
    filterButton(choice, 'e', 'h', "eE-hH");
    filterButton(choice, 'i', 'l', "iI-lL");
    filterButton(choice, 'm', 'p', "mM-pP");
    filterButton(choice, 'q', 't', "qQ-tT");
    filterButton(choice, 'u', 'z', "uU-zZ");
    filterButton(choice, '0', '9', "0-9");

    bool found = false;
    for (int i = 0; i <= choice->getMax(); i += 1) {
      char c = choice->getString(i)[0];
      if (c && !isdigit(c) && !isalpha(c)) {
        found = true;
        break;
      }
    }

    if (found) {
      addButton(
          "._-", 0, choice->getMax(),
          [=](int16_t index) {
            char c = choice->getString(index)[0];
            return c && !isdigit(c) && !isalpha(c);
          },
          STR_MENU_OTHER);
    }

    addButton(STR_SELECT_MENU_CLR, 0, 0, nullptr, nullptr, true);
  }

  void filterButton(FileChoice *choice, char from, char to, const char* title)
  {
    bool found = false;
    for (int i = 0; i <= choice->getMax(); i += 1) {
      char c = choice->getString(i)[0];
      if (isupper(c)) c += 0x20;
      if (c >= from && c <= to) {
        found = true;
        break;
      }
    }

    if (found) {
      char s[4];
      s[0] = from; s[1] = '-'; s[2] = to; s[3] = 0;
      addButton(
          s, 0, choice->getMax(),
          [=](int16_t index) {
            char c = choice->getString(index)[0];
            if (isupper(c)) c += 0x20;
            return (c >= from && c <= to);
          },
          title);
    }
  }

  static LAYOUT_VAL(FC_COLS, 3, 2)

 protected:
};

FileChoice::FileChoice(Window *parent, const rect_t &rect, std::string folder,
                       const char *extension, int maxlen,
                       std::function<std::string()> getValue,
                       std::function<void(std::string)> setValue,
                       bool stripExtension, const char *title) :
    Choice(
        parent, rect, 0, 0, [=]() { return selectedIdx; },
        [=](int val) { setValue(getString(val)); }, title, CHOICE_TYPE_FOLDER),
    folder(std::move(folder)),
    extension(extension),
    maxlen(maxlen),
    getValue(std::move(getValue)),
    setValue(std::move(setValue)),
    stripExtension(stripExtension)
{
  update();
}

std::string FileChoice::getLabelText() { return getValue(); }

void FileChoice::loadFiles()
{
  if (loaded) return;

  loaded = true;

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

    f_closedir(&dir);
  }

  if (!files.empty()) {
    // sort files
    files.sort(compare_nocase);
    files.push_front("");

    std::string value = getValue();
    int idx = 0;
    for (const auto &file : files) {
      addValue(file.c_str());
      if (strcmp(value.c_str(), file.c_str()) == 0) selectedIdx = idx;
      idx += 1;
    }

    setMax(files.size() - 1);
  }

  fileCount = files.size();
}

void FileChoice::openMenu()
{
  loadFiles();

  if (fileCount > 0) {
    setEditMode(true);  // this needs to be done first before menu is created.

    auto menu = new Menu();
    if (menuTitle) menu->setTitle(menuTitle);

    auto tb = new FileChoiceMenuToolbar(this, menu);
    menu->setToolbar(tb);

    // fillMenu(menu); - called by MenuToolbar

    menu->setCloseHandler([=]() { setEditMode(false); });
  } else {
    new MessageDialog(STR_SDCARD, STR_NO_FILES_ON_SD);
  }
}
