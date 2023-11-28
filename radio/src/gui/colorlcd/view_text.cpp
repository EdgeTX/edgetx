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

#include "menu.h"
#include "opentx.h"
#include "sdcard.h"

constexpr int maxTxtBuffSize = 64 * 1024;

ViewTextWindow::ViewTextWindow(const std::string path, const std::string name,
                               unsigned int icon) :
      Page(icon), path(std::move(path)), name(std::move(name))
{
  fullPath = this->path + std::string(PATH_SEPARATOR) + this->name;
  extractNameSansExt();

  header.setTitle(this->name);

  lv_obj_add_event_cb(lvobj, ViewTextWindow::on_draw, LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
};

void ViewTextWindow::on_draw(lv_event_t * e)
{
  lv_obj_t* target = lv_event_get_target(e);
  auto view = (ViewTextWindow*)lv_obj_get_user_data(target);
  if (view) {
    if (view->buffer == nullptr)
      view->buildBody(&view->body);
  }
}

void ViewTextWindow::onCancel()
{
  Page::onCancel();
}

void ViewTextWindow::extractNameSansExt()
{
  uint8_t nameLength;
  uint8_t extLength;

  const char *ext =
      getFileExtension(name.c_str(), 0, 0, &nameLength, &extLength);
  extension = std::string(ext);

  openFromEnd = !strcmp(ext, LOGS_EXT);
}

bool ViewTextWindow::openFile()
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
      if (sdReadTextFileBlock(bufSize, offset) == FR_OK) {
        return true;
      }
    }
  }

  return false;
}

void ViewTextWindow::buildBody(Window *window)
{
  if (openFile()) {
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

FRESULT ViewTextWindow::sdReadTextFileBlock(const uint32_t bufSize,
                                            const uint32_t offset)
{
  FIL file;
  char escape_chars[4];
  int escape = 0;

  auto res = f_open(&file, (TCHAR *)fullPath.c_str(), FA_OPEN_EXISTING | FA_READ);
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
    if (event == EVT_KEY_BREAK(KEY_PAGEDN)) {
      offset += bufSize;
      TRACE("event=DOWN");
    }

    if (event == EVT_KEY_BREAK(KEY_PAGEUP)) {
      TRACE("event=UP");
      offset -= bufSize;
    }

    offset = std::max(offset, 0);
    offset = std::min(offset, fileLength - (int)bufSize);

    TRACE("AFTER offset=%d", offset);
    sdReadTextFileBlock(bufSize, offset);
    lv_label_set_text_static(lb, buffer);
  }

  if(event == EVT_KEY_BREAK(KEY_EXIT))
    onCancel();
#endif
}

class ViewChecklistWindow : public ViewTextWindow
{
  public:
    ViewChecklistWindow(const std::string path, const std::string name,
                        unsigned int icon) :
        ViewTextWindow(path, name, icon)
    {
      header.setTitle(g_model.header.name);
      header.setTitle2(STR_PREFLIGHT);
    }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewChecklistWindow"; };
#endif

    void onCancel() override
    {
      if (allChecked())
        ViewTextWindow::onCancel();
    }

  protected:
    TextButton* closeButton = nullptr;
    std::list<lv_obj_t*> checkBoxes;

    void updateCheckboxes()
    {
      bool lastState = true;

      for (auto it = checkBoxes.cbegin(); it != checkBoxes.cend(); ++it) {
        auto cb = *it;
        if (lastState)
        {
          lv_obj_clear_state(cb, LV_STATE_DISABLED);
          if (!(lv_obj_get_state(cb) & LV_STATE_CHECKED))
            lv_group_focus_obj(cb);
        } else {
          lv_obj_add_state(cb, LV_STATE_DISABLED);
          lv_obj_clear_state(cb, LV_STATE_CHECKED);
        }

        lastState = lv_obj_get_state(cb) & LV_STATE_CHECKED;
      }

      setCloseState();
    }

    bool allChecked()
    {
      for (auto it = checkBoxes.cbegin(); it != checkBoxes.cend(); ++it) {
        auto cb = *it;
        if (!(lv_obj_get_state(cb) & LV_STATE_CHECKED))
          return false;
      }

      return true;
    }

    void setCloseState()
    {
      if (allChecked()) {
        lv_obj_clear_state(closeButton->getLvObj(), LV_STATE_DISABLED);
        lv_group_focus_obj(closeButton->getLvObj());
      } else {
        lv_obj_add_state(closeButton->getLvObj(), LV_STATE_DISABLED);
      }
    }

    void onEvent(event_t event) override
    {
#if defined(HARDWARE_KEYS)
      if(event == EVT_KEY_BREAK(KEY_EXIT))
        onCancel();
#endif
    }

    static void checkbox_event_handler(lv_event_t* e)
    {
      lv_obj_t* target = lv_event_get_target(e);
      ViewChecklistWindow* vtw = (ViewChecklistWindow*)lv_obj_get_user_data(target);

      if (vtw) vtw->updateCheckboxes();
    }

    void buildBody(Window* window) override
    {
      if (openFile()) {
        auto obj = window->getLvObj();
        lv_obj_add_flag(
            obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_WITH_ARROW |
                     LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_AUTO);
        // prevents resetting the group's edit mode
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

        lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(obj, 3, LV_PART_MAIN);
        lv_obj_set_style_pad_row(obj, 0, LV_PART_MAIN);

        auto g = lv_group_get_default();

        checkBoxes.clear();

        size_t cur = 0;

        for(size_t i=0; i<bufSize; ++i)
        {
          if (buffer[i] == '\n' || buffer[i] == '\r') {
            buffer[i] = 0;
            if (buffer[i] == '\r' && buffer[i+1] == '\n')
              i += 1;

            lv_obj_t* row = lv_obj_create(obj);
            lv_obj_set_layout(row, LV_LAYOUT_FLEX);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_width(row, lv_pct(100));
            lv_obj_set_height(row, LV_SIZE_CONTENT);
            lv_obj_set_style_pad_all(row, 3, 0);
            lv_obj_set_style_pad_column(row, 6, 0);
            lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY);

            lv_coord_t w = lv_obj_get_content_width(obj) - 6;

            if (buffer[cur] == '=') {
              cur++;
              w -= 46;

              lv_obj_set_style_pad_left(row, 10, 0);

              auto cb = etx_checkbox_create(row);

              lv_group_add_obj(g, cb);

              lv_obj_add_event_cb(cb, ViewChecklistWindow::checkbox_event_handler, LV_EVENT_VALUE_CHANGED, this);
              lv_obj_set_user_data(cb, this);

              checkBoxes.push_back(cb);
            }

            auto lbl = lv_label_create(row);
            lv_obj_set_width(lbl, w);
            lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
            lv_label_set_text_static(lbl, &buffer[cur]);

            cur = i + 1;
          }
        }

        auto box = new FormWindow(window, rect_t{0, 0, lv_pct(100), LV_SIZE_CONTENT});
        box->padAll(8);

        closeButton = new TextButton(box, rect_t{}, STR_EXIT, [=]() -> int8_t { this->onCancel(); return 0; });
        closeButton->setWidth(lv_pct(100));

        updateCheckboxes();
      }
    }
};

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

static bool openNotes(const char buf[], std::string modelNotesName, bool fromMenu = false)
{
  std::string fullPath = std::string(buf) + PATH_SEPARATOR + modelNotesName;

  if (isFileAvailable(fullPath.c_str())) {
    if (fromMenu || !g_model.checklistInteractive)
      new ViewTextWindow(std::string(buf), modelNotesName, ICON_MODEL);
    else
      new ViewChecklistWindow(std::string(buf), modelNotesName, ICON_MODEL);
    return true;
  } else {
    return false;
  }
}

void readModelNotes(bool fromMenu)
{
  bool notesFound = false;
  LED_ERROR_BEGIN();

  std::string modelNotesName(g_model.header.name);
  modelNotesName.append(TEXT_EXT);
  const char buf[] = {MODELS_PATH};

  notesFound = openNotes(buf, modelNotesName, fromMenu);
  if (!notesFound) {
    replaceSpaceWithUnderscore(modelNotesName);
    notesFound = openNotes(buf, modelNotesName, fromMenu);
  }

#if !defined(EEPROM)
  if (!notesFound) {
    modelNotesName.assign(g_eeGeneral.currModelFilename);
    size_t index = modelNotesName.find(MODEL_FILE_EXT);
    if (index != std::string::npos) {
      modelNotesName.erase(index);
      modelNotesName.append(TEXT_EXT);
      notesFound = openNotes(buf, modelNotesName, fromMenu);
    }
    if (!notesFound) {
      replaceSpaceWithUnderscore(modelNotesName);
      notesFound = openNotes(buf, modelNotesName, fromMenu);
    }
  }
#endif

  LED_ERROR_END();
}
