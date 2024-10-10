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
#include "etx_lv_theme.h"

#define ETX_STATE_NO_INFO_COLOR LV_STATE_USER_1

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

TemplatePage::TemplatePage() : Page(ICON_MODEL_SELECT, PAD_ZERO)
{
  body->setFlexLayout();

  FlexGridLayout grid(col_dsc, row_dsc, PAD_SMALL);

  auto line = body->newLine(grid);

  listWindow = new Window(line, rect_t{});
  etx_scrollbar(listWindow->getLvObj());
  listWindow->padAll(PAD_TINY);
  listWindow->padRight(PAD_SMALL);
  listWindow->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_SMALL, LV_PCT(100), body->height() - PAD_SMALL * 2);
  lv_obj_set_flex_align(listWindow->getLvObj(), LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_BETWEEN);
  lv_obj_set_grid_cell(listWindow->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  infoLabel = lv_label_create(line->getLvObj());
  lv_obj_set_height(infoLabel, body->height() - PAD_SMALL * 2);
  etx_obj_add_style(infoLabel, styles->text_align_left, LV_PART_MAIN);
  etx_txt_color(infoLabel, COLOR_THEME_PRIMARY1_INDEX);
  etx_txt_color(infoLabel, COLOR_THEME_DISABLED_INDEX, ETX_STATE_NO_INFO_COLOR);
  lv_obj_set_grid_cell(infoLabel, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
}

void TemplatePage::updateInfo()
{
  if (buffer[0]) {
    FIL fp;
    FRESULT res = f_open(&fp, buffer, FA_READ);
    unsigned int bytesRead = 0;
    if (res == FR_OK) {
      f_read(&fp, infoText, LEN_INFO_TEXT, &bytesRead);
      f_close(&fp);
    }
    infoText[bytesRead] = '\0';
  }

  if (infoText[0] == 0) {
    lv_label_set_text(infoLabel, STR_NO_INFORMATION);
    lv_obj_add_state(infoLabel, ETX_STATE_NO_INFO_COLOR);
  } else {
    lv_label_set_text(infoLabel, infoText);
    lv_obj_clear_state(infoLabel, ETX_STATE_NO_INFO_COLOR);
  }
}

#if defined(HARDWARE_KEYS)
void TemplatePage::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    deleteLater();
  } else {
    Page::onEvent(event);
  }
}
#endif

class SelectTemplate : public TemplatePage
{
 public:
  SelectTemplate(SelectTemplateFolder* tp, std::string folder) :
      templateFolderPage(tp)
  {
    header->setTitle(STR_MANAGE_MODELS);
    header->setTitle2(STR_NEW_MODEL);

    char path[LEN_PATH + 1];
    snprintf(path, LEN_PATH, "%s/%s", TEMPLATES_PATH, folder.c_str());

    std::list<std::string> files;
    FILINFO fno;
    DIR dir;
    FRESULT res = f_opendir(&dir, path);

    ButtonBase* firstButton = nullptr;

    if (res == FR_OK) {
      // read all entries
      for (;;) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
          break;  // Break on error or end of dir
        if (strlen((const char*)fno.fname) > SD_SCREEN_FILE_LENGTH) continue;
        if (fno.fattrib & (AM_DIR | AM_HID | AM_SYS))
          continue; /* Ignore folders, hidden and system files */
        if (fno.fname[0] == '.') continue; /* Ignore UNIX hidden files */

        const char* ext = getFileExtension(fno.fname);
        if (ext && !strcasecmp(ext, YAML_EXT)) {
          int len = ext - fno.fname;
          if (len < FF_MAX_LFN) {
            char name[FF_MAX_LFN] = {0};
            strncpy(name, fno.fname, len);
            files.push_back(name);
          }
        }
      }

      files.sort(compare_nocase);

      for (auto name : files) {
        auto tb = new TextButton(
            listWindow, rect_t{0, 0, lv_pct(100), EdgeTxStyles::PAGE_LINE_HEIGHT * 2}, name,
            [=]() -> uint8_t {
              deleteLater();
              templateFolderPage->doUpdate(folder, name);
              return 0;
            });
        tb->setFocusHandler([=](bool active) {
          if (active) {
            snprintf(buffer, LEN_BUFFER, "%s/%s%s", path, name.c_str(),
                     TEXT_EXT);
            updateInfo();
          }
        });

        if (!firstButton) firstButton = tb;
      }
    }

    f_closedir(&dir);

    if (files.size() == 0) {
      new StaticText(listWindow, rect_t{0, 0, lv_pct(100), lv_pct(50)},
                     STR_NO_TEMPLATES);
    } else {
      lv_group_focus_obj(firstButton->getLvObj());
    }
  }

 protected:
  SelectTemplateFolder* templateFolderPage;
};

SelectTemplateFolder::SelectTemplateFolder(
    std::function<void(std::string folder, std::string)> update)
{
  this->update = update;

  header->setTitle(STR_MANAGE_MODELS);
  header->setTitle2(STR_NEW_MODEL);

  auto tfb = new TextButton(listWindow,
                            rect_t{0, 0, lv_pct(100), EdgeTxStyles::PAGE_LINE_HEIGHT * 2},
                            STR_BLANK_MODEL, [=]() -> uint8_t {
                              doUpdate("", "");
                              return 0;
                            });
  tfb->setFocusHandler([=](bool active) {
    if (active) {
      buffer[0] = 0;
      strcpy(infoText, STR_BLANK_MODEL_INFO);
      updateInfo();
    }
  });

  std::list<std::string> directories;
  FILINFO fno;
  DIR dir;
  FRESULT res = f_opendir(&dir, TEMPLATES_PATH);

  if (res == FR_OK) {
    // read all entries
    for (;;) {
      res = f_readdir(&dir, &fno);
      if (res != FR_OK || fno.fname[0] == 0)
        break;  // Break on error or end of dir
      if (strlen((const char*)fno.fname) > SD_SCREEN_FILE_LENGTH) continue;
      if (fno.fattrib & (AM_HID | AM_SYS))
        continue;                        /* Ignore hidden and system files */
      if (fno.fname[0] == '.') continue; /* Ignore UNIX hidden files */
      if (fno.fattrib & AM_DIR) directories.push_back((char*)fno.fname);
    }

    directories.sort(compare_nocase);

    for (auto name : directories) {
#if not defined(LUA)
      // Don't show wizards dir if no lua
      if (!strcasecmp(name.c_str(), "WIZARD") == 0) {
#endif
        auto tfb = new TextButton(
            listWindow, rect_t{0, 0, lv_pct(100), EdgeTxStyles::PAGE_LINE_HEIGHT * 2}, name,
            [=]() -> uint8_t {
              new SelectTemplate(this, name);
              return 0;
            });
        tfb->setFocusHandler([=](bool active) {
          if (active) {
            snprintf(buffer, LEN_BUFFER, "%s/%s/about%s", TEMPLATES_PATH,
                     name.c_str(), TEXT_EXT);
            updateInfo();
          }
        });
#if not defined(LUA)
      }
#endif
    }
  }

  f_closedir(&dir);

  if (directories.size() == 0) {
    new StaticText(listWindow, rect_t{0, 0, lv_pct(100), lv_pct(50)},
                   STR_NO_TEMPLATES);
  }

  lv_group_focus_obj(tfb->getLvObj());
}
