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

#include "preview_window.h"

#include "edgetx.h"
#include "etx_lv_theme.h"
#include "quick_menu_group.h"
#include "sliders.h"
#include "textedit.h"
#include "toggleswitch.h"
#include "topbar.h"
#include "trims.h"

// class to hold a color list and apply / restore it while drawing standard
// controls
class ColorMaintainer
{
 public:
  void setColorList(std::vector<ColorEntry> colorList)
  {
    this->colorList.assign(colorList.begin(), colorList.end());
  }

  void applyColorValues()
  {
    // save old values;
    for (auto i = 0; i < THEME_COLOR_COUNT; i++) {
      oldColorVals[i] = lcdColorTable[i];
    }

    for (auto color : colorList) {
      lcdColorTable[color.colorNumber] = color.colorValue;
    }

    usePreviewStyle();
  }

  void restoreColorValues()
  {
    for (auto i = 0; i < THEME_COLOR_COUNT; i++) {
      lcdColorTable[i] = oldColorVals[i];
    }

    useMainStyle();
  }

 protected:
  uint32_t oldColorVals[THEME_COLOR_COUNT];
  std::vector<ColorEntry> colorList;
};

extern ColorMaintainer colorMaintainer;

class ThemedCheckBox : public ToggleSwitch
{
 public:
  ThemedCheckBox(Window *parent, rect_t rect, bool checked) :
      ToggleSwitch(
          parent, rect, [=]() { return checked; },
          [=](uint8_t value) { update(); }),
      checked(checked)
  {
    setWindowFlag(NO_FOCUS | NO_CLICK);
    setFocusHandler([](bool focus) {});
  }

 protected:
  bool checked;
};

ColorMaintainer colorMaintainer;

class ThemedButton : public TextButton
{
 public:
  ThemedButton(Window *window, const rect_t &rect, std::string text,
               bool isChecked) :
      TextButton(window, rect, text, nullptr)
  {
    setWindowFlag(NO_FOCUS | NO_CLICK);
    setPressHandler([=]() { return isChecked; });
  }
};

class ThemedTextEdit : public TextEdit
{
 public:
  ThemedTextEdit(Window *parent, const rect_t &rect, const char *text,
                 bool edited) :
      TextEdit(parent, rect, editText, 0)
  {
    strcpy(editText, text);
    preview(edited, editText, strlen(editText));
  }

 protected:
  char editText[50];
};

PreviewWindow::PreviewWindow(Window *window, rect_t rect,
                             std::vector<ColorEntry> colorList) :
    Window(window, rect)
{
  setWindowFlag(NO_FOCUS);

  // reset default group to avoid focus
  lv_group_t *def_group = lv_group_get_default();
  lv_group_set_default(nullptr);

  setColorList(colorList);
  colorMaintainer.applyColorValues();

  etx_solid_bg(lvobj, COLOR_THEME_SECONDARY3_INDEX);

  auto topbar = new Window(this, {0, 0, LV_PCT(100), EdgeTxStyles::MENU_HEADER_HEIGHT});
  etx_solid_bg(topbar->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);

  new HeaderIcon(topbar, ICON_EDGETX);

  new StaticText(this, {CBT_X, CBT_Y, CBT_W, EdgeTxStyles::STD_FONT_HEIGHT}, STR_THEME_CHECKBOX);
  new ThemedCheckBox(this, {CB1_X, CB_Y, CB_W, 0}, true);
  new ThemedCheckBox(this, {CB2_X, CB_Y, CB_W, 0}, false);
  (new ThemedButton(this, {BTN_X, BTN1_Y, BTN_W, 0}, STR_THEME_ACTIVE, true))
      ->check(true);
  new ThemedButton(this, {BTN_X, BTN2_Y, BTN_W, 0}, STR_THEME_REGULAR, false);
  new MainViewTrim(this, {CBT_X, TRIM_Y, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH, EdgeTxStyles::STD_FONT_HEIGHT}, 0, false);
  new MainViewSlider(this, {CBT_X, SLIDER_Y, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH, EdgeTxStyles::STD_FONT_HEIGHT}, 0, false);
  new StaticText(this, {CBT_X, TXT1_Y, TXT_W, EdgeTxStyles::STD_FONT_HEIGHT}, STR_THEME_WARNING,
                 COLOR_THEME_WARNING_INDEX);
  new StaticText(this, {CBT_X, TXT2_Y, TXT_W, EdgeTxStyles::STD_FONT_HEIGHT}, STR_THEME_DISABLED,
                 COLOR_THEME_DISABLED_INDEX);

  new ThemedTextEdit(this, {CBT_X, EDT_Y, EDY_W, 0}, STR_THEME_EDIT, true);
  new ThemedTextEdit(this, {EDT2_X, EDT_Y, EDY_W, 0}, STR_THEME_FOCUS, false);

  new HeaderDateTime(this, width() - DATE_XO, PAD_SMALL);

  auto qm = new Window(this, {QM_X, QM_Y, QM_W, QM_H});
  etx_solid_bg(qm->getLvObj(), COLOR_THEME_QM_BG_INDEX);
  etx_obj_add_style(qm->getLvObj(), styles->bg_opacity_90, LV_PART_MAIN);
  auto sep = lv_obj_create(qm->getLvObj());
  etx_solid_bg(sep, COLOR_THEME_QM_FG_INDEX);
  lv_obj_set_size(sep, LCD_W, PAD_THREE);
  auto mask = getBuiltinIcon(ICON_TOP_LOGO);
  new StaticIcon(qm, (QM_W - mask->width) / 2, 0, ICON_TOP_LOGO, COLOR_THEME_QM_FG_INDEX);

  auto qmb = new Window(qm, {PAD_SMALL, mask->height + PAD_SMALL, QuickMenuGroup::QM_BUTTON_WIDTH, QuickMenuGroup::QM_BUTTON_HEIGHT});
  etx_obj_add_style(qmb->getLvObj(), styles->rounded, LV_PART_MAIN);
  etx_txt_color(qmb->getLvObj(), COLOR_THEME_QM_FG_INDEX, LV_PART_MAIN);
  etx_solid_bg(qmb->getLvObj(), COLOR_THEME_QM_BG_INDEX, LV_PART_MAIN);
  new StaticIcon(qmb, (QuickMenuGroup::QM_BUTTON_WIDTH - QuickMenuGroup::QM_ICON_SIZE) / 2, PAD_SMALL,
                  ICON_MODEL_SELECT, COLOR_THEME_QM_FG_INDEX);
  new StaticText(qmb, {0, QuickMenuGroup::QM_ICON_SIZE + PAD_TINY * 2, QuickMenuGroup::QM_BUTTON_WIDTH - 1, 0},
                  STR_QM_MANAGE_MODELS, COLOR_THEME_QM_FG_INDEX, CENTERED | FONT(XS));

  qmb = new Window(qm, {QuickMenuGroup::QM_BUTTON_WIDTH + PAD_SMALL * 2, mask->height + PAD_SMALL, QuickMenuGroup::QM_BUTTON_WIDTH, QuickMenuGroup::QM_BUTTON_HEIGHT});
  etx_obj_add_style(qmb->getLvObj(), styles->rounded, LV_PART_MAIN);
  etx_txt_color(qmb->getLvObj(), COLOR_THEME_QM_BG_INDEX, LV_PART_MAIN);
  etx_solid_bg(qmb->getLvObj(), COLOR_THEME_QM_FG_INDEX, LV_PART_MAIN);
  new StaticIcon(qmb, (QuickMenuGroup::QM_BUTTON_WIDTH - QuickMenuGroup::QM_ICON_SIZE) / 2, PAD_SMALL,
                  ICON_MODEL, COLOR_THEME_QM_BG_INDEX);
  new StaticText(qmb, {0, QuickMenuGroup::QM_ICON_SIZE + PAD_TINY * 2, QuickMenuGroup::QM_BUTTON_WIDTH - 1, 0},
                  STR_QM_MODEL_SETUP, COLOR_THEME_QM_BG_INDEX, CENTERED | FONT(XS));

  lv_group_set_default(def_group);

  colorMaintainer.restoreColorValues();
}

void PreviewWindow::setColorList(std::vector<ColorEntry> colorList)
{
  colorMaintainer.setColorList(colorList);
  // Force style update
  colorMaintainer.applyColorValues();
  colorMaintainer.restoreColorValues();
  // Update preview colors
  invalidate();
}
