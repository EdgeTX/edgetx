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

#include "radio_diagkeys.h"

#include "hal/rotary_encoder.h"
#include "libopenui.h"
#include "edgetx.h"

#if defined(RADIO_NB4P)
static const uint8_t _trimMap[MAX_TRIMS * 2] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
#elif defined(PCBPL18)
static const uint8_t _trimMap[MAX_TRIMS * 2] = {8, 9, 10, 11, 12, 13, 14, 15,
                                                2, 3, 4,  5,  0,  1,  6,  7};
#else
static const uint8_t _trimMap[MAX_TRIMS * 2] = {6, 7, 4, 5, 2,  3,
                                                0, 1, 8, 9, 10, 11};
#endif

static EnumKeys get_ith_key(uint8_t i)
{
  auto supported_keys = keysGetSupported();
  for (uint8_t k = 0; k < MAX_KEYS; k++) {
    if (supported_keys & (1 << k)) {
      if (i-- == 0) return (EnumKeys)k;
    }
  }

  // should not get here,
  // we assume: i < keysGetMaxKeys()
  return (EnumKeys)0;
}

class RadioKeyDiagsWindow : public Window
{
 public:
  RadioKeyDiagsWindow(Window *parent, const rect_t &rect) : Window(parent, rect)
  {
    padAll(PAD_ZERO);

    coord_t colWidth = (width() - 24) / 3;
    coord_t colHeight = height() - 12;

    Window* form;
    coord_t x = 6;

    if (keysGetMaxKeys() > 0) {
      form = new Window(parent, rect_t{x, 6, colWidth, colHeight});
      etx_txt_color(form->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
      addKeys(form);
      x += colWidth + 6;
    } else {
      colWidth = (width() - 18) / 2;
    }

    form = new Window(parent, rect_t{x, 6, colWidth, colHeight});
    etx_txt_color(form->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
    addSwitches(form);
    x += colWidth + 6;

    form = new Window(parent, rect_t{x, 6, colWidth, colHeight});
    etx_txt_color(form->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
    addTrims(form);
  }

  ~RadioKeyDiagsWindow()
  {
    delete keyValues;
    delete switchValues;
    delete trimValues;
  }

  void addKeys(Window *form)
  {
    keyValues = new lv_obj_t *[keysGetMaxKeys()];
    lv_obj_t *obj = form->getLvObj();
    uint8_t i;

    // KEYS
    for (i = 0; i < keysGetMaxKeys(); i++) {
      auto k = get_ith_key(i);

      auto lbl = lv_label_create(obj);
      lv_label_set_text(lbl, keysGetLabel(k));
      lv_obj_set_pos(lbl, 0, i * EdgeTxStyles::PAGE_LINE_HEIGHT);

      lbl = lv_label_create(obj);
      lv_label_set_text(lbl, "");
      lv_obj_set_pos(lbl, 70, i * EdgeTxStyles::PAGE_LINE_HEIGHT);
      keyValues[i] = lbl;
    }

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
    auto lbl = lv_label_create(obj);
    lv_label_set_text(lbl, STR_ROTARY_ENCODER);
    lv_obj_set_pos(lbl, 0, (i + 1) * EdgeTxStyles::PAGE_LINE_HEIGHT);

    reValue = lv_label_create(obj);
    lv_label_set_text(reValue, "");
    lv_obj_set_pos(reValue, 70, (i + 1) * EdgeTxStyles::PAGE_LINE_HEIGHT);
#endif
  }

  void addSwitches(Window *form)
  {
    switchValues = new lv_obj_t *[switchGetMaxSwitches()];
    lv_obj_t *obj = form->getLvObj();
    uint8_t i;
    uint8_t row = 0;

    // SWITCHES
    for (i = 0; i < switchGetMaxSwitches(); i++) {
      if (SWITCH_EXISTS(i)) {
        auto lbl = lv_label_create(obj);
        lv_label_set_text(lbl, "");
        lv_obj_set_pos(lbl, 0, row * EdgeTxStyles::PAGE_LINE_HEIGHT);
        switchValues[i] = lbl;
        row += 1;
      }
    }
  }

  void addTrims(Window *form)
  {
    trimValues = new lv_obj_t *[keysGetMaxTrims() * 2];
    lv_obj_t *obj = form->getLvObj();
    char s[10];

    auto lbl = lv_label_create(obj);
    lv_label_set_text(lbl, STR_TRIMS);
    lv_obj_set_pos(lbl, 0, 0);
    lbl = lv_label_create(obj);
    lv_label_set_text(lbl, "-");
    lv_obj_set_pos(lbl, 62, 0);
    lbl = lv_label_create(obj);
    lv_label_set_text(lbl, "+");
    lv_obj_set_pos(lbl, 75, 0);

    // TRIMS
    for (uint8_t i = 0; i < keysGetMaxTrims(); i++) {
      lbl = lv_label_create(obj);
      formatNumberAsString(s, 10, i + 1, 0, 10, "T");
      lv_label_set_text(lbl, s);
      lv_obj_set_pos(lbl, 4, i * EdgeTxStyles::PAGE_LINE_HEIGHT + EdgeTxStyles::PAGE_LINE_HEIGHT);

      lbl = lv_label_create(obj);
      lv_label_set_text(lbl, "");
      lv_obj_set_pos(lbl, 60, i * EdgeTxStyles::PAGE_LINE_HEIGHT + EdgeTxStyles::PAGE_LINE_HEIGHT);
      trimValues[i * 2] = lbl;

      lbl = lv_label_create(obj);
      lv_label_set_text(lbl, "");
      lv_obj_set_pos(lbl, 75, i * EdgeTxStyles::PAGE_LINE_HEIGHT + EdgeTxStyles::PAGE_LINE_HEIGHT);
      trimValues[i * 2 + 1] = lbl;
    }
  }

  void setKeyState()
  {
    char s[10] = "0";

    for (uint8_t i = 0; i < keysGetMaxKeys(); i++) {
      auto k = get_ith_key(i);
      s[0] = keysGetState(k) + '0';
      lv_label_set_text(keyValues[i], s);
    }

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
    formatNumberAsString(s, 10, rotaryEncoderGetValue());
    lv_label_set_text(reValue, s);
#endif
  }

  void setSwitchState()
  {
    uint8_t i;

    for (i = 0; i < switchGetMaxSwitches(); i++) {
      if (SWITCH_EXISTS(i)) {
        getvalue_t val = getValue(MIXSRC_FIRST_SWITCH + i);
        getvalue_t sw =
            ((val < 0) ? 3 * i + 1 : ((val == 0) ? 3 * i + 2 : 3 * i + 3));
        lv_label_set_text(switchValues[i], getSwitchPositionName(sw));
      }
    }
  }

  void setTrimState()
  {
    char s[10] = "0";

    for (uint8_t i = 0; i < keysGetMaxTrims() * 2; i++) {
      s[0] = keysGetTrimState(_trimMap[i]) + '0';
      lv_label_set_text(trimValues[i], s);
    }
  }

  void checkEvents() override
  {
    setKeyState();
    setSwitchState();
    setTrimState();
  }

 protected:
  lv_obj_t **keyValues = nullptr;
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  lv_obj_t *reValue = nullptr;
#endif
  lv_obj_t **switchValues = nullptr;
  lv_obj_t **trimValues = nullptr;
};

void RadioKeyDiagsPage::buildHeader(Window *window)
{
  header->setTitle(STR_RADIO_SETUP);
  header->setTitle2(STR_MENU_RADIO_SWITCHES);
}

void RadioKeyDiagsPage::buildBody(Window *window)
{
  body->padAll(PAD_ZERO);
  new RadioKeyDiagsWindow(window, {0, 0, window->width(), window->height()});
}

RadioKeyDiagsPage::RadioKeyDiagsPage() : Page(ICON_MODEL_SETUP)
{
  buildHeader(header);
  buildBody(body);
}
