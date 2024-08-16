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
#include "keyboard_number.h"
#include "strhelpers.h"
#include "etx_lv_theme.h"

class NumberArea : public FormField
{
 public:
  NumberArea(NumberEdit* parent, const rect_t& rect) :
      FormField(parent, rect, etx_textarea_create),
      numEdit(parent)
  {
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    if (parent->getTextFlags() & CENTERED)
      etx_obj_add_style(lvobj, styles->text_align_center, LV_PART_MAIN);
    else
      etx_obj_add_style(lvobj, styles->text_align_right, LV_PART_MAIN);

    // Allow encoder acceleration
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

    lv_obj_add_event_cb(lvobj, NumberArea::numberedit_cb, LV_EVENT_KEY, this);

    setFocusHandler([=](bool focus) {
      if (!focus && editMode) {
        setEditMode(false);
        hide();
        lv_obj_clear_state(parent->getLvObj(), LV_STATE_FOCUSED);
      }
    });

    update();
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "NumberArea(" + std::to_string(getValue()) + ")";
  }
#endif

  void onEvent(event_t event) override
  {
    TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(),
                  event);

    if (editMode) {
      int value = numEdit->getValue();
      switch (event) {
#if defined(HARDWARE_KEYS)
        case EVT_ROTARY_RIGHT: {
          auto step = numEdit->step;
          step += (rotaryEncoderGetAccel() * numEdit->accelFactor) / 8;
          do {
#if defined(USE_HATS_AS_KEYS)
            value -= step;
#else
            value += step;
#endif
          } while (numEdit->isValueAvailable && !numEdit->isValueAvailable(value) &&
                   value <= numEdit->vmax);
          if (value <= numEdit->vmax) {
            numEdit->setValue(value);
          } else {
            numEdit->setValue(numEdit->vmax);
            onKeyError();
          }
          return;
        }

        case EVT_ROTARY_LEFT: {
          auto step = numEdit->step;
          step += (rotaryEncoderGetAccel() * numEdit->accelFactor) / 8;
          do {
#if defined(USE_HATS_AS_KEYS)
            value += step;
#else
            value -= step;
#endif
          } while (numEdit->isValueAvailable && !numEdit->isValueAvailable(value) &&
                   value >= numEdit->vmin);
          if (value >= numEdit->vmin) {
            numEdit->setValue(value);
          } else {
            numEdit->setValue(numEdit->vmin);
            onKeyError();
          }
          return;
        }
#endif

        case EVT_VIRTUAL_KEY_PLUS:
          numEdit->setValue(value + numEdit->step);
          break;

        case EVT_VIRTUAL_KEY_MINUS:
          numEdit->setValue(value - numEdit->step);
          break;

        case EVT_VIRTUAL_KEY_FORWARD:
          numEdit->setValue(value + numEdit->fastStep * numEdit->step);
          break;

        case EVT_VIRTUAL_KEY_BACKWARD:
          numEdit->setValue(value - numEdit->fastStep * numEdit->step);
          break;

        case EVT_VIRTUAL_KEY_DEFAULT:
          numEdit->setValue(numEdit->vdefault);
          break;

        case EVT_VIRTUAL_KEY_MAX:
          numEdit->setValue(numEdit->vmax);
          break;

        case EVT_VIRTUAL_KEY_MIN:
          numEdit->setValue(numEdit->vmin);
          break;

        case EVT_VIRTUAL_KEY_SIGN:
          numEdit->setValue(-value);
          break;
      }
    }

    FormField::onEvent(event);
  }

  void onClicked() override
  {
    lv_indev_type_t indev_type = lv_indev_get_type(lv_indev_get_act());
    if (indev_type == LV_INDEV_TYPE_POINTER) {
      setEditMode(true);
    } else {
      FormField::onClicked();
      if (!editMode) changeEnd();
    }
  }

  void openKeyboard() { NumberKeyboard::open(this); }
  void directEdit() { FormField::onClicked(); }

  void update()
  {
    if (lvobj != nullptr)
      lv_textarea_set_text(lvobj, numEdit->getDisplayVal().c_str());
  }

 protected:
  NumberEdit* numEdit = nullptr;

  void onCancel() override
  {
    onClicked();
  }

  static void numberedit_cb(lv_event_t* e)
  {
    NumberArea* numEdit = (NumberArea*)lv_event_get_user_data(e);
    if (!numEdit || numEdit->deleted()) return;

    uint32_t key = lv_event_get_key(e);
    switch (key) {
      case LV_KEY_LEFT:
        numEdit->onEvent(EVT_ROTARY_LEFT);
        break;
      case LV_KEY_RIGHT:
        numEdit->onEvent(EVT_ROTARY_RIGHT);
        break;
    }
  }
};

/*
  The lv_textarea object is slow. To avoid too much overhead on views with multiple
  edit fields, the text area is initially displayed as a button. When the button
  is pressed, a text area object is created over the top of the button in order
  to edit the value.
*/
NumberEdit::NumberEdit(Window* parent, const rect_t& rect, int vmin, int vmax,
                       std::function<int()> getValue,
                       std::function<void(int)> setValue, LcdFlags textFlags) :
    TextButton(parent, rect, "",
               [=]() {
                 openEdit();
                 return 0;
               }),
    _getValue(std::move(getValue)),
    _setValue(std::move(setValue)),
    vmin(vmin),
    vmax(vmax)
{
  if (rect.w == 0) setWidth(DEF_W);

  setTextFlag(textFlags);

  lv_obj_set_width(label, width() - PAD_MEDIUM * 2 - 2);
  if (textFlags & CENTERED)
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
  else
    etx_obj_add_style(label, styles->text_align_right, LV_PART_MAIN);

  update();
}

void NumberEdit::openEdit()
{
  if (edit == nullptr) {
    edit = new NumberArea(
        this,
        {-(PAD_MEDIUM + 2), -(PAD_TINY + 2),
        lv_obj_get_width(lvobj), lv_obj_get_height(lvobj)});
    edit->setChangeHandler([=]() {
      update();
      if (edit->hasFocus())
        lv_group_focus_obj(lvobj);
      edit->hide();
    });
  }
  edit->update();
  edit->show();
  lv_group_focus_obj(edit->getLvObj());
  lv_indev_type_t indev_type =
      lv_indev_get_type(lv_indev_get_act());
  if (indev_type == LV_INDEV_TYPE_POINTER) {
    edit->openKeyboard();
  } else {
    edit->directEdit();
  }
  lv_obj_add_state(lvobj, LV_STATE_FOCUSED);
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
