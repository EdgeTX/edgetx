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

<<<<<<<< HEAD:radio/src/targets/pa01/battery_driver.h
#include "board.h"
#include "hal.h"

#include "hal/gpio.h"
#include "stm32_gpio.h"

enum ChargeState
{
  CHARGE_UNKNOWN,
  CHARGE_NONE,
  CHARGE_STARTED,
  CHARGE_FINISHED
========
#include "form.h"
#include "bitmaps.h"
#include <vector>

class ButtonBase;

class QuickMenuGroup : public Window
{
 public:
  QuickMenuGroup(Window* parent, const rect_t &rect);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "QuickMenuGroup"; }
#endif

  // Add a new button to the carousel
  ButtonBase* addButton(EdgeTxIcon icon, const char* title,
                 std::function<uint8_t(void)> pressHandler, bool visible = true);

  void setGroup();
  void setFocus();
  void clearFocus();
  void setDisabled(bool all);
  void setEnabled();
  void setCurrent(ButtonBase* b) { curBtn = b; }
  void setCurrent(int b) { setCurrent(btns[b]); }

  static LAYOUT_VAL_SCALED(FAB_BUTTON_WIDTH, 50)
  static LAYOUT_VAL_SCALED(FAB_BUTTON_HEIGHT, 70)

  static LAYOUT_VAL_SCALED(FAB_ICON_SIZE, 30)
  static constexpr coord_t FAB_BUTTON_INNER_WIDTH = FAB_BUTTON_WIDTH;

 protected:
  std::vector<ButtonBase*> btns;
  ButtonBase* curBtn = nullptr;
  lv_group_t* group = nullptr;

  void deleteLater(bool detach = true, bool trash = true) override;
>>>>>>>> 63f64f877 (New quick menu for 3.0.):radio/src/gui/colorlcd/setup_menus/quick_menu_group.h
};

enum PowerLevel {
  POWER_LEVEL_NONE,
  POWER_LEVEL_CRITICAL,
  POWER_LEVEL_LOW,
  POWER_LEVEL_MEDIUM,
  POWER_LEVEL_HIGH,
  POWER_LEVEL_NEAR_FULL,
  POWER_LEVEL_FULL,
};

extern void battery_charge_init();
extern void handle_battery_charge(uint32_t last_press_time);
extern uint16_t get_battery_charge_state();
extern uint16_t getBatteryVoltage();   // returns current battery voltage in 10mV steps
extern bool isChargerActive();
extern void battery_charge_end();
void rgbBatteryLevelInfo(uint8_t power_level, uint8_t rgb_state);
void updateBatteryState(uint8_t rgb_state);