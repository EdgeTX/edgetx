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

#include "model_templates.h"
#include "standalone_lua.h"
#include "str_functions.h"

char TemplatePage::path[LEN_PATH + 1];
std::function<void(void)> TemplatePage::update = nullptr;


void TemplatePage::updateInfo()
{
  VfsFile fp;
  VfsError res = VirtualFS::instance().openFile(fp, buffer, VfsOpenFlags::READ);
  size_t bytesRead = 0;
  if (res == VfsError::OK) {
    fp.read(infoText, LEN_INFO_TEXT, bytesRead);
    fp.close();
  }
  infoText[bytesRead] = '\0';
  invalidate();
}

#if defined(HARDWARE_KEYS)
void TemplatePage::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  } else {
    Page::onEvent(event);
  }
}
#endif

void TemplatePage::paint(BitmapBuffer *dc)
{
  Page::paint(dc);
  rect_t rect = body.getRect();
  coord_t x = LCD_W / 2 + PAGE_PADDING;
  coord_t y = rect.y + PAGE_PADDING;
  coord_t w = LCD_W / 2 - 2 * PAGE_PADDING;
  coord_t h = rect.h - 2 * PAGE_PADDING;
  if (count > 0) {
    if (infoText[0] == '\0')
      drawTextLines(dc, x, y, w, h, STR_NO_INFORMATION, COLOR_THEME_DISABLED);
    else
      drawTextLines(dc, x, y, w, h, infoText, COLOR_THEME_PRIMARY1);
  }
}


SelectTemplate::SelectTemplate(TemplatePage* tp)
  : templateFolderPage(tp)
{
  rect_t rect = {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT};
  new StaticText(&header, rect, STR_SELECT_TEMPLATE, 0, COLOR_THEME_PRIMARY2);

  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  std::list<std::string> files;
  VirtualFS& vfs = VirtualFS::instance();
  VfsFileInfo fno;
  VfsDir dir;
  VfsError res = vfs.openDirectory(dir, path);

  if (res == VfsError::OK) {
    // read all entries
    for (;;) {
      res = dir.read(fno);
      if (res != VfsError::OK || fno.getName()[0] == 0)
        break; // Break on error or end of dir
      const char* fName = fno.getName();
      if (strlen(fName) > STORAGE_SCREEN_FILE_LENGTH)
        continue;
      if (fName[0] == '.')
        continue;
      if (fno.getType() == VfsType::FILE) {
        const char *ext = vfs.getFileExtension(fName);
        if(ext && !strcasecmp(ext, YAML_EXT)) {
          int len = ext - fName;
          if (len < FF_MAX_LFN) {
            char name[FF_MAX_LFN] = { 0 };
            strncpy(name, fName, len);
            files.push_back(name);
          }
        }
      }
    }

    files.sort(compare_nocase);

    for (auto name: files) {
      auto tb = new TemplateButton(&body, grid.getLabelSlot(), name, [=]() -> uint8_t {
          // Read model template
          loadModelTemplate((name + YAML_EXT).c_str(), path);
          storageDirty(EE_MODEL);
          storageCheck(true);
          // Dismiss template pages
          deleteLater();
          templateFolderPage->deleteLater();
#if defined(LUA)
          // If there is a wizard Lua script, fire it up
          snprintf(buffer, LEN_BUFFER, "%s%c%s%s", path, '/', name.c_str(), SCRIPT_EXT);
          VfsFileInfo info;
          if (VirtualFS::instance().fstat(buffer, info) == VfsError::OK) {
            luaExec(buffer);
            // Need to update() the ModelCategoryPageBody before attaching StandaloneLuaWindow to not mess up focus
            update();
            update = nullptr;
            StandaloneLuaWindow::instance()->attach();
          }
#endif
          return 0;
        });
      tb->setFocusHandler([=](bool active) {
        if (active) {
          snprintf(buffer, LEN_BUFFER, "%s%c%s%s", path, '/', name.c_str(), TEXT_EXT);
          updateInfo();
        }
      });
      tb->setHeight(PAGE_LINE_HEIGHT * 2);
      grid.spacer(tb->height() + 5);
    }
  }

  dir.close();
  count = files.size();
  if (count == 0) {
    rect_t rect = body.getRect();
    rect.x = PAGE_PADDING;
    rect.y = PAGE_PADDING;
    rect.w = rect.w - (2 * PAGE_PADDING);

#if LCD_W > LCD_H
    rect.h = PAGE_LINE_HEIGHT;
    int charBreak = 60;
#else
    rect.h = PAGE_LINE_HEIGHT * 2;
    int charBreak = 40;
#endif
    new StaticText(&body, rect, wrap(STR_NO_TEMPLATES, charBreak), 0, COLOR_THEME_PRIMARY1);

    // The following button is needed because the EXIT key does not work without...
    rect = body.getRect();
    rect.x = rect.w - PAGE_PADDING - 100;
    rect.y = rect.h - PAGE_PADDING - (PAGE_LINE_HEIGHT * 2);
    rect.w = 100;
    rect.h = PAGE_LINE_HEIGHT * 2;
    new TextButton(&body, rect, STR_EXIT, [=]() -> uint8_t { deleteLater(); return 0; });
  } else {
    snprintf(buffer, LEN_BUFFER, "%s%c%s%s", path, '/', files.front().c_str(), TEXT_EXT);
    updateInfo();
  }
}


SelectTemplateFolder::SelectTemplateFolder(std::function<void(void)> update)
{
  TemplatePage::update = update;
  rect_t rect = {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT};
  new StaticText(&header, rect, STR_SELECT_TEMPLATE_FOLDER, 0, COLOR_THEME_PRIMARY2);

  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  auto tfb = new TemplateButton(&body, grid.getLabelSlot(), STR_BLANK_MODEL, [=]() -> uint8_t {
    deleteLater();
    return 0;
  });
  snprintf(infoText, LEN_INFO_TEXT, "%s", STR_BLANK_MODEL_INFO);
  tfb->setFocusHandler([=](bool active) {
    if (active) {
      snprintf(infoText, LEN_INFO_TEXT, "%s", STR_BLANK_MODEL_INFO);
      invalidate();
    }
  });
  tfb->setHeight(PAGE_LINE_HEIGHT * 2);
  grid.spacer(tfb->height() + 5);

  std::list<std::string> directories;
  VfsFileInfo fno;
  VfsDir dir;
  VfsError res = VirtualFS::instance().openDirectory(dir, TEMPLATES_PATH);

  if (res == VfsError::OK) {
    // read all entries
    for (;;) {
      res = dir.read(fno);
      if (res != VfsError::OK || fno.getName() == 0)
        break; // Break on error or end of dir
      const char* fName = fno.getName();
      if (strlen(fName) > STORAGE_SCREEN_FILE_LENGTH)
        continue;
      if (fName[0] == '.')
        continue;
      if (fno.getType() == VfsType::DIR)
        directories.push_back(fName);
    }

    directories.sort(compare_nocase);

    for (auto name: directories) {
#if not defined(LUA)
      // Don't show wizards dir if no lua
      if (!strcasecmp(name.c_str(), "WIZARD") == 0) {
#endif
      auto tfb = new TemplateButton(&body, grid.getLabelSlot(), name,
          [=]() -> uint8_t {
          snprintf(path, LEN_PATH, "%s%c%s", TEMPLATES_PATH, '/', name.c_str());
          new SelectTemplate(this);
          return 0;
        });
      tfb->setFocusHandler([=](bool active) {
        if (active) {
          snprintf(buffer, LEN_BUFFER, "%s%c%s%c%s%s", TEMPLATES_PATH, '/', name.c_str(), '/', "about", TEXT_EXT);
          updateInfo();
        }
      });
      tfb->setHeight(PAGE_LINE_HEIGHT * 2);
      grid.spacer(tfb->height() + 5);
    }
#if not defined(LUA)
    }
#endif
  }

  dir.close();
  count = directories.size();
  if (count == 0) {
    rect_t rect = grid.getLabelSlot();
    rect.w = body.getRect().w - 2 * PAGE_PADDING;
    new StaticText(&body, rect, STR_NO_TEMPLATES, 0, COLOR_THEME_PRIMARY1);
  }
}

SelectTemplateFolder::~SelectTemplateFolder()
{
  if (update)
    update();
}
