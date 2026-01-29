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

#include "button.h"
#include "edgetx.h"
#include "etx_lv_theme.h"
#include "fullscreen_dialog.h"
#include "lib_file.h"
#include "mainwindow.h"
#include "menu.h"
#include "sdcard.h"

// Used on startup to block until checklist is closed.
static bool checkListOpen = false;

// Check Box
const lv_style_const_prop_t cb_marker_checked_props[] = {
    LV_STYLE_CONST_BG_IMG_SRC(LV_SYMBOL_OK),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(cb_marker_checked, cb_marker_checked_props);

static void checkbox_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_INDICATOR, PAD_TINY);

  etx_obj_add_style(obj, cb_marker_checked,
                    LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_checkbox_set_text_static(obj, "");
}

static const lv_obj_class_t checkbox_class = {
    .base_class = &lv_checkbox_class,
    .constructor_cb = checkbox_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 25,
    .height_def = 25,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_checkbox_t),
};

static lv_obj_t* checkbox_create(lv_obj_t* parent)
{
  return etx_create(&checkbox_class, parent);
}

constexpr int maxTxtBuffSize = 64 * 1024;

class TextViewer
{
 public:
  TextViewer(const std::string path, const std::string name) : name(std::move(name))
  {
    fullPath = path + std::string(PATH_SEPARATOR) + name;
    extractNameSansExt();
  }

  ~TextViewer()
  {
    if (buffer) {
      free(buffer);
      buffer = nullptr;
    }
  }

  #if defined(HARDWARE_KEYS)
  void changePage(int direction)
  {
    if (int(bufSize) < fileLength) {
      offset = offset + bufSize * direction;

      offset = std::max(offset, 0);
      offset = std::min(offset, fileLength - (int)bufSize);

      sdReadTextFileBlock(bufSize, offset);
      lv_label_set_text_static(lb, buffer);
    }
  }
  #endif

  void build(Window* window)
  {
    if (openFile()) {
      auto obj = window->getLvObj();
      lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SCROLL_MOMENTUM);
      etx_scrollbar(obj);
      // prevents resetting the group's edit mode
      window->setWindowFlag(NO_FOCUS);

      auto g = lv_group_get_default();
      lb = lv_label_create(obj);
      lv_obj_set_size(lb, lv_pct(100), LV_SIZE_CONTENT);
      etx_obj_add_style(lb, styles->pad_medium, LV_PART_MAIN);

      lv_group_add_obj(g, obj);
      lv_group_set_editing(g, true);
      lv_label_set_text_static(lb, buffer);

      if (openFromEnd)
        lv_obj_scroll_to_y(obj, LV_COORD_MAX, LV_ANIM_OFF);
      else
        lv_obj_scroll_to_y(obj, 0, LV_ANIM_OFF);
    }
  }

 protected:
  std::string name;
  std::string fullPath;

  lv_obj_t* lb;

  int offset = 0;
  char* buffer = nullptr;
  size_t bufSize = 0;
  int fileLength = 0;
  bool openFromEnd;

  void extractNameSansExt()
  {
    uint8_t nameLength;
    uint8_t extLength;

    const char* ext =
        getFileExtension(name.c_str(), 0, 0, &nameLength, &extLength);

    openFromEnd = !strcmp(ext, LOGS_EXT);
  }

  bool openFile()
  {
    FILINFO info;

    if (buffer) {
      free(buffer);
      buffer = nullptr;
      bufSize = 0;
    }

    auto res = f_stat((TCHAR*)fullPath.c_str(), &info);
    if (res == FR_OK) {
      fileLength = int(info.fsize);
      bufSize = std::min(fileLength, maxTxtBuffSize) + 1;

      buffer = (char*)malloc(bufSize);
      if (buffer) {
        offset = std::max(int(openFromEnd ? int(info.fsize) - bufSize + 1 : 0), 0);
        if (sdReadTextFileBlock(bufSize, offset) == FR_OK) {
          return true;
        }
      }
    }

    return false;
  }

  FRESULT sdReadTextFileBlock(const uint32_t bufSize, const uint32_t offset)
  {
    FIL file;
    char escape_chars[4];
    int escape = 0;

    auto res =
        f_open(&file, (TCHAR*)fullPath.c_str(), FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK) {
      res = f_lseek(&file, offset);
      if (res == FR_OK) {
        UINT br;
        char c;
        char* ptr = buffer;
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
                *ptr++ = CHAR_UP[0];
                c = CHAR_UP[1];
                escape = 0;
              } else if (escape == 2 && !strncmp(escape_chars, "dn", 2)) {
                *ptr++ = CHAR_DOWN[0];
                c = CHAR_DOWN[1];
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
};

ViewTextWindow::ViewTextWindow(const std::string path, const std::string name,
                               EdgeTxIcon icon) :
    Page(icon, PAD_ZERO)
{
  textViewer = new TextViewer(path, name);

  header->setTitle(name);

  delayLoad();
};

void ViewTextWindow::delayedInit()
{
  textViewer->build(body);
}

void ViewTextWindow::onCancel()
{
  if (textViewer) delete textViewer;
  Page::onCancel();
  checkListOpen = false;
}

#if defined(HARDWARE_KEYS)
void ViewTextWindow::onPressPGUP()
{
  if (textViewer) textViewer->changePage(-1);
}

void ViewTextWindow::onPressPGDN()
{
  if (textViewer) textViewer->changePage(1);
}
#endif

class ViewChecklistWindow : public Page, public TextViewer
{
 public:
  ViewChecklistWindow(const std::string path, const std::string name,
                      EdgeTxIcon icon) :
      Page(icon, PAD_ZERO), TextViewer(path, name)
  {
    header->setTitle(g_model.header.name);
    header->setTitle2(STR_PREFLIGHT);

    fullPath = path + std::string(PATH_SEPARATOR) + name;
    extractNameSansExt();

    delayLoad();
  }

  void delayedInit() override
  {
    if (buffer == nullptr) buildBody(body);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewChecklistWindow"; };
#endif

  void onCancel() override
  {
    if (allChecked()) {
      Page::onCancel();
      checkListOpen = false;
    }
  }

 protected:
  TextButton* closeButton = nullptr;
  std::list<lv_obj_t*> checkBoxes;

  void updateCheckboxes()
  {
    bool lastState = true;

    for (auto it = checkBoxes.cbegin(); it != checkBoxes.cend(); ++it) {
      auto cb = *it;
      if (lastState) {
        lv_obj_clear_state(cb, LV_STATE_DISABLED);
        if (!(lv_obj_get_state(cb) & LV_STATE_CHECKED)) lv_group_focus_obj(cb);
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
      if (!(lv_obj_get_state(cb) & LV_STATE_CHECKED)) return false;
    }

    return true;
  }

  void setCloseState()
  {
    if (allChecked()) {
      closeButton->enable();
      lv_group_focus_obj(closeButton->getLvObj());
    } else {
      closeButton->disable();
    }
  }

  static void checkbox_event_handler(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    ViewChecklistWindow* vtw =
        (ViewChecklistWindow*)lv_obj_get_user_data(target);

    if (vtw) vtw->updateCheckboxes();
  }

  void buildBody(Window* window)
  {
    if (openFile()) {
      auto obj = window->getLvObj();
      lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW | LV_OBJ_FLAG_SCROLL_MOMENTUM);
      etx_scrollbar(obj);
      // prevents resetting the group's edit mode
      window->setWindowFlag(NO_FOCUS);

      lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
      lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
      lv_obj_set_style_pad_all(obj, PAD_THREE, LV_PART_MAIN);
      lv_obj_set_style_pad_row(obj, 0, LV_PART_MAIN);

      auto g = lv_group_get_default();

      checkBoxes.clear();

      size_t cur = 0;

      for (size_t i = 0; i <= bufSize; ++i) {
        if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == 0) {
          // Check for end of line & end of file
          if (buffer[i] == 0 && cur == i) break;
          buffer[i] = 0;
          if (buffer[i] == '\r' && buffer[i + 1] == '\n') i += 1;

          lv_obj_t* row = lv_obj_create(obj);
          lv_obj_set_layout(row, LV_LAYOUT_FLEX);
          lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
          lv_obj_set_width(row, lv_pct(100));
          lv_obj_set_height(row, LV_SIZE_CONTENT);
          lv_obj_set_style_pad_all(row, PAD_THREE, 0);
          lv_obj_set_style_pad_column(row, PAD_MEDIUM, 0);
          lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                LV_FLEX_ALIGN_SPACE_EVENLY);

          lv_coord_t w = lv_obj_get_content_width(obj) - PAD_MEDIUM;

          if (buffer[cur] == '=') {
            cur++;
            w -= 46;

            lv_obj_set_style_pad_left(row, PAD_LARGE + PAD_TINY, 0);

            auto cb = checkbox_create(row);

            lv_group_add_obj(g, cb);

            lv_obj_add_event_cb(cb, ViewChecklistWindow::checkbox_event_handler,
                                LV_EVENT_VALUE_CHANGED, this);
            lv_obj_set_user_data(cb, this);

            checkBoxes.push_back(cb);
          }

          auto lbl = etx_label_create(row);
          lv_obj_set_width(lbl, w);
          lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
          lv_label_set_text_static(lbl, &buffer[cur]);

          cur = i + 1;
        }
      }

      auto box =
          new Window(window, rect_t{0, 0, lv_pct(100), LV_SIZE_CONTENT});
      box->padAll(PAD_LARGE);

      closeButton = new TextButton(box, rect_t{}, STR_EXIT, [=]() -> int8_t {
        this->onCancel();
        return 0;
      });
      closeButton->setWidth(lv_pct(100));

      updateCheckboxes();
    }
  }
};

static void replaceSpaceWithUnderscore(std::string& name)
{
  size_t index;
  do {
    index = name.find(' ');
    if (index != std::string::npos) name[index] = '_';
  } while (index != std::string::npos);
}

#define MODEL_FILE_EXT YAML_EXT

static bool checkNotesFile(std::string modelNotesName)
{
  std::string fullPath = std::string(MODELS_PATH) + PATH_SEPARATOR + modelNotesName;
  return isFileAvailable(fullPath.c_str());
}

static std::string getModelNotesFile()
{
  std::string modelNotesName(g_model.header.name);
  modelNotesName.append(TEXT_EXT);

  if (checkNotesFile(modelNotesName))
    return modelNotesName;

  replaceSpaceWithUnderscore(modelNotesName);
  if (checkNotesFile(modelNotesName))
    return modelNotesName;

  modelNotesName.assign(g_eeGeneral.currModelFilename);
  size_t index = modelNotesName.find(MODEL_FILE_EXT);
  if (index != std::string::npos) {
    modelNotesName.erase(index);
    modelNotesName.append(TEXT_EXT);
    if (checkNotesFile(modelNotesName))
      return modelNotesName;
  }
  replaceSpaceWithUnderscore(modelNotesName);
  if (checkNotesFile(modelNotesName))
    return modelNotesName;

  return std::string("");
}

static Window* _readModelNotes(bool fromMenu)
{
  std::string modelNotesName = getModelNotesFile();
  if (!modelNotesName.empty()) {
    std::string fullPath = std::string(MODELS_PATH) + PATH_SEPARATOR + modelNotesName;

    if (isFileAvailable(fullPath.c_str())) {
      if (fromMenu || !g_model.checklistInteractive)
        return new ViewTextWindow(std::string(MODELS_PATH), modelNotesName, ICON_MODEL);
      else
        return new ViewChecklistWindow(std::string(MODELS_PATH), modelNotesName, ICON_MODEL);
    }
  }
  return nullptr;
}

void readModelNotes(bool fromMenu)
{
  _readModelNotes(fromMenu);
}

// Blocking version of readModelNotes.
void readChecklist()
{
  auto dialog = _readModelNotes(false);
  if (dialog) {
    LED_ERROR_BEGIN();
    MainWindow::instance()->blockUntilClose(true, [=]() {
      return dialog->deleted();
    });
    LED_ERROR_END();
  }
}

ModelNotesPage::ModelNotesPage(const PageDef& pageDef) : PageGroupItem(pageDef, PAD_ZERO)
{
}

void ModelNotesPage::build(Window* window)
{
  if (!textViewer) textViewer = new TextViewer(MODELS_PATH, getModelNotesFile());
  if (textViewer) textViewer->build(window);
}

void ModelNotesPage::cleanup()
{
  if (textViewer) delete textViewer;
  textViewer = nullptr;
}
