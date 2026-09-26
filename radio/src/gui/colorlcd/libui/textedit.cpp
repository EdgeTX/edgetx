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

#include "textedit.h"

#include "keyboard_text.h"
#include "myeeprom.h"
#include "storage/storage.h"
#include "etx_lv_theme.h"

#if defined(HARDWARE_KEYS)
#include "menu.h"
#endif

class TextArea : public FormField
{
 public:
  TextArea(Window* parent, const rect_t& rect, char* value, uint8_t length,
           std::function<void()> changeHandler) :
      FormField(parent, rect, etx_textarea_create),
      value(value), length(length),
      changeHandler(changeHandler)
  {
    setWindowFlag(NO_FOCUS | IS_EDIT_WINDOW);

    lv_textarea_set_max_length(lvobj, length);
    lv_textarea_set_placeholder_text(lvobj, "---");

    setFocusHandler([=](bool focus) {
      if (!focus && editMode)
        Keyboard::hideKeyboard();
    });

    update();
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextArea"; }
#endif

  void update()
  {
    // value may not be null-terminated
    std::string txt(value, length);
    lv_textarea_set_text(lvobj, txt.c_str());
  }

  void openKeyboard()
  {
    update();
    show();
    lv_group_focus_obj(lvobj);
    TextKeyboard::open(this);
    setEditMode(true);
  }

  void setExcludedChars(std::string& excludedChars) {
    if (excludedChars.size() > 0) {
      allowedChars = "";
      for (char c = ' '; c <= '~'; c += 1) {
        if (excludedChars.find(c) == std::string::npos)
          allowedChars += c;
      }
      lv_textarea_set_accepted_chars(lvobj, allowedChars.c_str());
    } else {
      lv_textarea_set_accepted_chars(lvobj, nullptr);
    }
  }

 protected:
  char* value;
  uint8_t length;
  std::function<void()> changeHandler = nullptr;
  std::string allowedChars;

  void trim()
  {
    for (int i = length - 1; i >= 0; i--) {
      if (value[i] == ' ' || value[i] == '\0')
        value[i] = '\0';
      else
        break;
    }
  }

  void changeEnd() override
  {
    setEditMode(false);
    hide();

    auto text = lv_textarea_get_text(lvobj);
    if (strncmp(value, text, length) != 0) {
      strncpy(value, text, length);
      trim();
      if (changeHandler) changeHandler();
    }
  }
};

/*
  The lv_textarea object is slow. To avoid too much overhead on views with multiple
  edit fields, the text area is initially displayed as a button. When the button
  is pressed, a text area object is created over the top of the button in order
  to edit the value.
*/
TextEdit::TextEdit(Window* parent, const rect_t& rect, char* text,
                               uint8_t length,
                               std::function<void(void)> updateHandler) :
    TextButton(parent, rect, "", [=]() {
      openEdit();
      return 0;
    }),
    updateHandler(updateHandler), text(text), length(length)
{
  // Prevent TextArea focus outline from clipping
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

  if (rect.w == 0) setWidth(EdgeTxStyles::EDIT_FLD_WIDTH);

  update();
  lv_obj_align(label, LV_ALIGN_OUT_LEFT_MID, 0, PAD_TINY);
}

void TextEdit::update()
{
  if (text[0]) {
    std::string s(text, length);
    setText(s);
  } else {
    setText("---");
  }
}

void TextEdit::openEdit()
{
  if (edit == nullptr) {
    edit = new TextArea(this,
                        {-(PAD_MEDIUM + 2), -(PAD_BORDER * 2),
                          lv_obj_get_width(lvobj), lv_obj_get_height(lvobj)},
                        text, length,
                        [=]() {
                          update();
                          if (updateHandler) updateHandler();
                        });
  }
  edit->setExcludedChars(excludedChars);
  edit->openKeyboard();
}

void TextEdit::preview(bool edited, char* text, uint8_t length)
{
  setWindowFlag(NO_FOCUS | NO_CLICK);

  edit = new TextArea(this,
                      {-(PAD_MEDIUM + 2), -(PAD_BORDER * 2), width(), height()},
                      text, length, nullptr);
  edit->setWindowFlag(NO_CLICK);
  lv_group_focus_obj(edit->getLvObj());
  lv_obj_add_state(edit->getLvObj(), LV_STATE_FOCUSED);
  if (edited) lv_obj_add_state(edit->getLvObj(), LV_STATE_EDITED);
}

ModelTextEdit::ModelTextEdit(Window* parent, const rect_t& rect, char* value,
                             uint8_t length, std::function<void(void)> updateHandler) :
    TextEdit(parent, rect, value, length,
             [=]() {
               if (updateHandler) updateHandler();
               storageDirty(EE_MODEL);
             })
{
}

ModelStringEdit::ModelStringEdit(Window* parent, const rect_t& rect, std::string value,
                                 std::function<void(const char* s)> updateHandler) :
    TextEdit(parent, rect, txt, MAX_STR_EDIT_LEN,
             [=]() {
               if (updateHandler) updateHandler(txt);
               storageDirty(EE_MODEL);
             })
{
  strncpy(txt, value.c_str(), length);
  update();
}

RadioTextEdit::RadioTextEdit(Window* parent, const rect_t& rect, char* value,
                             uint8_t length) :
    TextEdit(parent, rect, value, length,
             []() { storageDirty(EE_GENERAL); })
{
}
