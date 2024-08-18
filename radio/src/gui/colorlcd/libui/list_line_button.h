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

#pragma once

#include "button.h"
#include "edgetx_types.h"
#include "tabsgroup.h"

class ListLineButton : public ButtonBase
{
 public:
  ListLineButton(Window* parent, uint8_t index);

  uint8_t getIndex() const { return index; }
  virtual void setIndex(uint8_t i) { index = i; }

  void checkEvents() override;

  virtual void refresh() = 0;

  static LAYOUT_VAL(BTN_H, 29, 29)
  static constexpr coord_t GRP_W = LCD_W - PAD_SMALL * 2;

 protected:
  uint8_t index;

  virtual bool isActive() const = 0;
};

class InputMixButtonBase : public ListLineButton
{
 public:
  InputMixButtonBase(Window* parent, uint8_t index);
  ~InputMixButtonBase();

  void setWeight(gvar_t value, gvar_t min, gvar_t max);
  void setSource(mixsrc_t idx);
  void setOpts(const char* s);
  void setFlightModes(uint16_t modes);

  virtual void updatePos(coord_t x, coord_t y) = 0;
  virtual void swapLvglGroup(InputMixButtonBase* line2) = 0;

  // total: 90 x 17
  static LAYOUT_VAL(FM_CANVAS_HEIGHT, 17, 17)
  static LAYOUT_VAL(FM_CANVAS_WIDTH, 90, 90)

  static LAYOUT_VAL(BTN_W, 395, 235)
  static constexpr coord_t WGT_X = PAD_TINY;
  static constexpr coord_t WGT_Y = PAD_TINY;
  static LAYOUT_VAL(WGT_W, 50, 50)
  static LAYOUT_VAL(WGT_H, 21, 21)
  static constexpr coord_t SRC_X = WGT_X + WGT_W + PAD_TINY;
  static constexpr coord_t SRC_Y = WGT_Y;
  static LAYOUT_VAL(SRC_W, 70, 69)
  static constexpr coord_t SRC_H = WGT_H;
  static constexpr coord_t OPT_X = SRC_X + SRC_W + PAD_TINY;
  static constexpr coord_t OPT_Y = WGT_Y;
  static LAYOUT_VAL(OPT_W, 164, 99)
  static constexpr coord_t OPT_H = WGT_H;
  static LAYOUT_VAL(LN_X, 73, 73)
  static LAYOUT_VAL(FM_X, (OPT_X + OPT_W + PAD_TINY), 12)
  static LAYOUT_VAL(FM_Y, (WGT_Y + PAD_TINY), (WGT_Y + WGT_H + PAD_TINY))
  static LAYOUT_VAL(FM_W, 8, 8)

 protected:

  lv_obj_t* fm_canvas = nullptr;
  void* fm_buffer = nullptr;
  uint16_t fm_modes = 0;

  lv_obj_t* weight = nullptr;
  lv_obj_t* source = nullptr;
  lv_obj_t* opts = nullptr;
};

class InputMixGroupBase : public Window
{
 public:
  InputMixGroupBase(Window* parent, mixsrc_t idx);

  mixsrc_t getMixSrc() { return idx; }
  size_t getLineCount() { return lines.size(); }

  virtual void adjustHeight();
  void addLine(InputMixButtonBase* line);
  bool removeLine(InputMixButtonBase* line);

  void refresh();

 protected:
  mixsrc_t idx;
  lv_obj_t* label;
  std::list<InputMixButtonBase*> lines;
};

class InputMixPageBase : public PageTab
{
 public:
  InputMixPageBase(const char* title, EdgeTxIcon icon) : PageTab(title, icon) {}

 protected:
  std::list<InputMixButtonBase*> lines;
  InputMixButtonBase* _copySrc = nullptr;
  Window* form = nullptr;
  uint8_t _copyMode = 0;
  std::list<InputMixGroupBase*> groups;

  virtual void addLineButton(uint8_t index) = 0;
  void addLineButton(mixsrc_t src, uint8_t index);

  InputMixButtonBase* getLineByIndex(uint8_t index);
  InputMixGroupBase* getGroupBySrc(mixsrc_t src);
  virtual InputMixGroupBase* getGroupByIndex(uint8_t index) = 0;

  virtual InputMixButtonBase* createLineButton(InputMixGroupBase* group, uint8_t index) = 0;
  virtual InputMixGroupBase* createGroup(Window* form, mixsrc_t src) = 0;

  void removeLine(InputMixButtonBase* l);
  void removeGroup(InputMixGroupBase* g);
};
