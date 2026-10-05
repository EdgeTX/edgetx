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

#include "numberedit.h"

#include "audio.h"
#include "debug.h"
#include "hal/rotary_encoder.h"
#include "keyboard_number.h"
#include "keys.h"
#include "strhelpers.h"
#include "etx_lv_theme.h"

class NumberArea : public FormField
{
 public:
  NumberArea(NumberEdit* parent, const rect_t& rect, std::function<void()> changeHandler) :
      FormField(parent, rect, etx_textarea_create),
      numEdit(parent),
      changeHandler(changeHandler)
  {
    if (parent->getTextFlags() & CENTERED)
      etx_obj_add_style(lvobj, styles->text_align_center, LV_PART_MAIN);
    else
      etx_obj_add_style(lvobj, styles->text_align_right, LV_PART_MAIN);

    // Hide cursor
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_CURSOR | LV_STATE_EDITED);
  
    setWindowFlag(NO_FOCUS | IS_EDIT_WINDOW);

    // Cancel edit if this field loses focus (another field was selected)
    setFocusHandler([=](bool focus) {
      if (!focus && editMode)
        Keyboard::hideKeyboard();
    });

    update();
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    if(numEdit)
       return "NumberArea(" + numEdit->getName() + ")";
    else
       return "NumberArea(unknown)";
  }
#endif

  void changeEnd() override
  {
    setEditMode(false);
    hide();
    if (changeHandler) changeHandler();
  }

  void openKeyboard()
  {
    update();
    show();
    lv_group_focus_obj(lvobj);

    NumberKeyboard::open(this, numEdit->step, numEdit->fastStep * numEdit->step, parent->getTextFlags(),
                         (numEdit->vmin < 0) && (numEdit->vmax > 0),
                         [=](uint16_t n) { numEdit->handleKBEvent(n); });

    setEditMode(true);
  }

  void update()
  {
    lv_textarea_set_text(lvobj, numEdit->getDisplayVal().c_str());
  }

 protected:
  NumberEdit* numEdit = nullptr;
  std::function<void()> changeHandler = nullptr;

  void onCancel() override
  {
    changeEnd();
  }
};

/*
  The lv_textarea object is slow. To avoid too much overhead on views with multiple
  edit fields, the text area is initially displayed as a button. When the button
  is pressed, a text area object is created over the top of the button in order
  to edit the value.

  Number edit supports 'direct' edit mode by pressing the ENTER key while the
  field has focus. This enables the rotary encoder to be user to change the value.
  The lv_textarea is not used in this case.
*/
NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue, LcdFlags textFlags) :
    TextButton(parent, rect, ""),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue)),
    vmin(vmin),
    vmax(vmax)
{
  // Prevent NumberArea focus outline from clipping
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

  if (rect.w == 0 || rect.w == LV_SIZE_CONTENT) setWidth(EdgeTxStyles::EDIT_FLD_WIDTH);

  setTextFlag(textFlags);

  padLeft(PAD_MEDIUM);
  padRight(PAD_SMALL);

  lv_obj_set_width(label, LV_PCT(100));

  if (textFlags & CENTERED)
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
  else
    etx_obj_add_style(label, styles->text_align_right, LV_PART_MAIN);

  etx_bg_color(lvobj, COLOR_THEME_EDIT_INDEX, LV_PART_MAIN | LV_STATE_EDITED);
  etx_txt_color(lvobj, COLOR_THEME_PRIMARY2_INDEX, LV_PART_MAIN | LV_STATE_EDITED);

  update();

  // Cancel edit if this field loses focus (another field was selected)
  setFocusHandler([=](bool focus) {
    if (!focus && editMode && directEdit)
      onCancel();
  });
}

bool NumberEdit::customEventHandler(lv_event_code_t code, lv_event_t *e)
{
  // Rotary encoder events in direct mode
  if (directEdit && code == LV_EVENT_KEY) {
    uint32_t key = lv_event_get_key(e);
    auto inc = step + (rotaryEncoderGetAccel() * accelFactor) / 8;

    switch (key) {
      case LV_KEY_LEFT:
        changeValue(-inc);
        return true;

      case LV_KEY_RIGHT:
        changeValue(inc);
        return true;
    }
  }

  return false;
}

void NumberEdit::onClicked()
{
  lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());

  if (directEdit) {
    if (indev_type != LV_INDEV_TYPE_POINTER) {
      onCancel();
    } else {
      // User has tapped on screen outside of any control, then taps on
      // this control
      if (!lv_obj_has_state(lvobj, LV_STATE_EDITED))
        setDirectEdit(true);
    }
    return;
  }

  if (onEditStart) onEditStart();

  if (indev_type != LV_INDEV_TYPE_POINTER) {
    setDirectEdit(true);
  } else {
    if (edit == nullptr) {
      edit = new NumberArea(
          this,
          {-(PAD_MEDIUM + 2), -(PAD_BORDER * 2),
          lv_obj_get_width(lvobj), lv_obj_get_height(lvobj)},
          [=]() {
            update();
            if (onEdited) onEdited(currentValue);
          });
    }
    edit->openKeyboard();
  }
}

void NumberEdit::update()
{
  if (_getValue == nullptr) return;
  currentValue = _getValue();
  updateDisplay();
}

std::string NumberEdit::getDisplayVal()
{
  std::string str;
  if (displayFunction != nullptr) {
    str = displayFunction(currentValue);
  } else if (!zeroText.empty() && currentValue == 0) {
    str = zeroText;
  } else {
    str = formatNumberAsString(currentValue, textFlags, 0, prefix.c_str(),
                               suffix.c_str());
  }
  return str;
}

void NumberEdit::updateDisplay()
{
  setText(getDisplayVal());
}

void NumberEdit::setValue(int value)
{
  auto newValue = limit(vmin, value, vmax);
  if (newValue != currentValue) {
    currentValue = newValue;
    if (_setValue != nullptr) {
      _setValue(currentValue);
    }
  }
  updateDisplay();
  if (edit) edit->update();
}

void NumberEdit::checkEvents()
{
  if (_getValue) {
    int newValue = _getValue();
    if (newValue != currentValue) {
      currentValue = newValue;
      updateDisplay();
    }
  }
  TextButton::checkEvents();
}

void NumberEdit::changeValue(int step)
{
  // Rotary encoder change
  int value = getValue();
  do {
#if defined(USE_HATS_AS_KEYS)
    value -= step;
#else
    value += step;
#endif
  } while (isValueAvailable && !isValueAvailable(value) &&
            value >= vmin && value <= vmax);
  if (value > vmax) {
    value = vmax;
    onKeyError();
  } else if (value < vmin) {
    value = vmin;
    onKeyError();
  }
  setValue(value);
}

void NumberEdit::setDirectEdit(bool editMode)
{
  directEdit = editMode;

  // Clear 'edited' state
  if (editMode) {
    lv_obj_add_state(lvobj, LV_STATE_EDITED);
  } else {
    lv_obj_clear_state(lvobj, LV_STATE_EDITED);

    // Call final change handler (Lua scripts)
    if (onEdited) onEdited(currentValue);
  }

  setEditMode(editMode);
}

void NumberEdit::onCancel()
{
  if (directEdit)
    setDirectEdit(false);
  else
    TextButton::onCancel();
}

void NumberEdit::handleKBEvent(int n)
{
  // Handle events from the custom keyboard
  int value = getValue();
  switch (n) {
    case NumberKeyboard::DEC_FASTSTEP:
      value = value - fastStep * step;
      break;
    case NumberKeyboard::DEC_STEP:
      value = value - step;
      break;
    case NumberKeyboard::INC_STEP:
      value = value + step;
      break;
    case NumberKeyboard::INC_FASTSTEP:
      value = value + fastStep * step;
      break;
    case NumberKeyboard::SET_MIN:
      value = vmin;
      break;
    case NumberKeyboard::SET_DEFAULT:
      value = vdefault;
      break;
    case NumberKeyboard::CHANGE_SIGN:
      if (vmin < 0 && vmax > 0)
        value = -value;
      break;
    case NumberKeyboard::SET_MAX:
      value = vmax;
      break;
  }
  setValue(value);
}
