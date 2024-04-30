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
#include "themes/etx_lv_theme.h"

class NumberArea : public FormField
{
 public:
  NumberArea(Window* parent, const rect_t& rect, int vmin, int vmax,
             std::function<int()> getValue,
             std::function<void(int)> setValue = nullptr,
             LcdFlags textFlags = 0) :
      FormField(parent, rect, textFlags, etx_textarea_create),
      vmin(vmin),
      vmax(vmax),
      _getValue(std::move(getValue)),
      _setValue(std::move(setValue))
  {
    if (rect.w == 0) setWidth(DEF_W);

    etx_obj_add_style(lvobj, styles->text_align_right, LV_PART_MAIN);

    // Allow encoder acceleration
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_ENCODER_ACCEL);

    lv_obj_add_event_cb(lvobj, NumberArea::numberedit_cb, LV_EVENT_KEY, this);

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
      switch (event) {
#if defined(HARDWARE_KEYS)
        case EVT_ROTARY_RIGHT: {
          int value = getValue();
          auto step = vstep;
          step += (rotaryEncoderGetAccel() * accelFactor) / 8;
          do {
#if defined(USE_HATS_AS_KEYS)
            value -= step;
#else
            value += step;
#endif
          } while (isValueAvailable && !isValueAvailable(value) &&
                   value <= vmax);
          if (value <= vmax) {
            setValue(value);
          } else {
            setValue(vmax);
            onKeyError();
          }
          return;
        }

        case EVT_ROTARY_LEFT: {
          int value = getValue();
          auto step = vstep;
          step += (rotaryEncoderGetAccel() * accelFactor) / 8;
          do {
#if defined(USE_HATS_AS_KEYS)
            value += step;
#else
            value -= step;
#endif
          } while (isValueAvailable && !isValueAvailable(value) &&
                   value >= vmin);
          if (value >= vmin) {
            setValue(value);
          } else {
            setValue(vmin);
            onKeyError();
          }
          return;
        }
#endif

        case EVT_VIRTUAL_KEY_PLUS:
          setValue(getValue() + vstep);
          break;

        case EVT_VIRTUAL_KEY_MINUS:
          setValue(getValue() - vstep);
          break;

        case EVT_VIRTUAL_KEY_FORWARD:
          setValue(getValue() + fastStep * vstep);
          break;

        case EVT_VIRTUAL_KEY_BACKWARD:
          setValue(getValue() - fastStep * vstep);
          break;

        case EVT_VIRTUAL_KEY_DEFAULT:
          setValue(vdefault);
          break;

        case EVT_VIRTUAL_KEY_MAX:
          setValue(vmax);
          break;

        case EVT_VIRTUAL_KEY_MIN:
          setValue(vmin);
          break;

        case EVT_VIRTUAL_KEY_SIGN:
          setValue(-getValue());
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
    }
  }

  void setMin(int value) { vmin = value; }
  void setMax(int value) { vmax = value; }
  void setDefault(int value) { vdefault = value; }
  void setStep(int value) { vstep = value; }
  void setFastStep(int value) { fastStep = value; }
  void setAccelFactor(int value) { accelFactor = value; }
  void setValue(int value)
  {
    auto newValue = limit(vmin, value, vmax);
    if (newValue != currentValue) {
      currentValue = newValue;
      if (_setValue != nullptr) {
        _setValue(currentValue);
      }
    }
    updateDisplay();
  }

  int32_t getValue() const { return _getValue != nullptr ? _getValue() : 0; }

  void setPrefix(std::string value) { prefix = std::move(value); }

  void setSuffix(std::string value) { suffix = std::move(value); }

  void setZeroText(std::string value) { zeroText = std::move(value); }

  void setAvailableHandler(std::function<bool(int)> handler)
  {
    isValueAvailable = std::move(handler);
  }

  void setSetValueHandler(std::function<void(int)> handler)
  {
    _setValue = std::move(handler);
  }

  void setGetValueHandler(std::function<int()> handler)
  {
    _getValue = std::move(handler);
  }

  void setDisplayHandler(std::function<std::string(int value)> function)
  {
    displayFunction = std::move(function);
  }

  void setCancelHandler(std::function<void(void)> handler)
  {
    cancelHandler = std::move(handler);
  }

  void openKeyboard() { NumberKeyboard::show(this); }
  void directEdit() { FormField::onClicked(); }

  void update()
  {
    if (_getValue == nullptr) return;
    currentValue = _getValue();
    updateDisplay();
  }

  static LAYOUT_VAL(DEF_W, 100, 100)

 protected:
  int vdefault = 0;
  int vmin;
  int vmax;
  int vstep = 1;
  int fastStep = 10;
  int accelFactor = 4;
  int currentValue;
  std::string prefix;
  std::string suffix;
  std::string zeroText;
  std::function<int()> _getValue;
  std::function<void(int)> _setValue;
  std::function<std::string(int)> displayFunction;
  std::function<bool(int)> isValueAvailable;
  std::function<void(void)> cancelHandler = nullptr;

  void updateDisplay()
  {
    if (lvobj != nullptr) {
      std::string str;
      if (displayFunction != nullptr) {
        str = displayFunction(currentValue);
      } else if (!zeroText.empty() && currentValue == 0) {
        str = zeroText;
      } else {
        str = formatNumberAsString(currentValue, textFlags, 0, prefix.c_str(),
                                  suffix.c_str());
      }
      lv_textarea_set_text(lvobj, str.c_str());
    }
  }

  void onCancel() override
  {
    if (cancelHandler)
      cancelHandler();
    else
      FormField::onCancel();
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
  if (rect.w == 0) setWidth(NumberArea::DEF_W);

  setTextFlag(textFlags);

  lv_obj_set_width(label, width() - PAD_MEDIUM * 2 - 2);
  etx_obj_add_style(label, styles->text_align_right, LV_PART_MAIN);

  update();
}

void NumberEdit::openEdit()
{
  if (edit == nullptr) {
    edit = new NumberArea(
        this,
        {-(PAD_MEDIUM + 2), -(PAD_TINY + 2),
        lv_obj_get_width(lvobj), lv_obj_get_height(lvobj)},
        this->vmin, this->vmax, _getValue, _setValue, textFlags);
    edit->setChangeHandler([=]() {
      update();
      lv_group_focus_obj(lvobj);
      edit->hide();
    });
    edit->setCancelHandler([=]() {
      lv_group_focus_obj(lvobj);
      edit->hide();
    });
  }
  edit->setTextFlag(textFlags);
  edit->setSetValueHandler(_setValue);
  edit->setGetValueHandler(_getValue);
  edit->setAvailableHandler(isValueAvailable);
  edit->setDisplayHandler(displayFunction);
  edit->setDefault(vdefault);
  edit->setMin(vmin);
  edit->setMax(vmax);
  edit->setStep(step);
  edit->setFastStep(fastStep);
  edit->setAccelFactor(accelFactor);
  edit->setPrefix(prefix);
  edit->setSuffix(suffix);
  edit->setZeroText(zeroText);
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

void NumberEdit::updateDisplay()
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
  setText(str);
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
  if (edit) edit->setValue(value);
}
