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

#include "hal/gpio.h"
#include "stm32_gpio.h"

#include "edgetx.h"
#include "battery_driver.h"

#define  __BATTERY_DRIVER_C__

#define BATTERY_W 140
#define BATTERY_H 320
#define BATTERY_TOP ((LCD_H - BATTERY_H)/2)
#define BATTERY_CONNECTOR_W 32
#define BATTERY_CONNECTOR_H 10
#define BATTERY_BORDER 4
#define BATTERY_W_INNER (BATTERY_W - 2*BATTERY_BORDER)
#define BATTERY_H_INNER (BATTERY_H - 2*BATTERY_BORDER)
#define BATTERY_TOP_INNER (BATTERY_TOP + BATTERY_BORDER)

void battery_charge_init()
{
  gpio_init(PWR_CHARGE_FINISHED_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
  gpio_init(PWR_CHARGING_GPIO, GPIO_IN_PU, GPIO_PIN_SPEED_LOW);
}

#define CHARGE_SAMPLES 10

uint16_t get_battery_charge_state()
{
  static uint16_t chargeSamples[CHARGE_SAMPLES] = {0};
  static uint16_t chargeSampleIndex = 0;
  uint16_t chargeState = CHARGE_UNKNOWN;
  int maxSamples = CHARGE_SAMPLES;
#if !defined(SIMU)
  bool isFinished = !READ_CHARGE_FINISHED_STATE();
  bool isCharging = !READ_CHARGING_STATE();
  //maxSamples = boardState == BOARD_POWER_OFF ? CHARGE_SAMPLES/2 : CHARGE_SAMPLES;
  if(chargeSampleIndex >= maxSamples) chargeSampleIndex = 0;
  uint16_t currentChargeState = isFinished ? CHARGE_FINISHED : isCharging ? CHARGE_STARTED : CHARGE_NONE;
  chargeSamples[chargeSampleIndex++] = currentChargeState;
  //TRACE("CHARGE sample %d value %d", chargeSampleIndex -1, currentChargeState);
#endif
    uint8_t temp1 = 0, temp2 = 0, temp3 = 0;
   for(int index = 0; index < maxSamples; index++) {
    if(chargeSamples[index] == CHARGE_FINISHED)
    {
        temp1++;
    }
    else if(chargeSamples[index] == CHARGE_STARTED)
    {
        temp2++;
    }
    else
    {
        temp3++;
    }

    if(temp1>=temp2&& temp1>temp3)
    {
        chargeState = CHARGE_FINISHED;
    }
    else if(temp2>=temp1&& temp2>temp3)
    {
        chargeState = CHARGE_STARTED;
    }
    else
    {
        chargeState = CHARGE_NONE;
    }
  }
  return chargeState;
}

void drawChargingInfo(uint16_t chargeState)
{
  static int progress = 0;
  const char* text =
      chargeState == CHARGE_STARTED ? STR_BATTERYCHARGING : STR_BATTERYFULL;
  int h = 0;
  LcdFlags color = 0;
  if (CHARGE_STARTED == chargeState) {
    if (progress >= 100) {
      progress = 0;
    } else {
      progress += 25;
    }
    text = STR_BATTERYCHARGING;
    h = ((BATTERY_H_INNER * progress) / 100);
    color = COLOR_THEME_EDIT;
  } else if (CHARGE_FINISHED == chargeState) {
    text = STR_BATTERYFULL;
    h = BATTERY_H_INNER;
    color = COLOR_THEME_EDIT;
  } else {
    text = STR_BATTERYNONE;
    h = BATTERY_H_INNER;
    color = COLOR_THEME_PRIMARY1;
  }

  BACKLIGHT_ENABLE();
  lcd->clear();
  lcd->drawSizedText(LCD_W / 2, LCD_H - 50, text, strlen(text),
                     CENTERED | COLOR_THEME_PRIMARY2);

  lcd->drawFilledRect((LCD_W - BATTERY_W) / 2, BATTERY_TOP, BATTERY_W,
                      BATTERY_H, SOLID, COLOR_THEME_PRIMARY2);
  lcd->drawFilledRect((LCD_W - BATTERY_W_INNER) / 2, BATTERY_TOP_INNER,
                      BATTERY_W_INNER, BATTERY_H_INNER, SOLID,
                      COLOR_THEME_PRIMARY1);

  lcd->drawFilledRect((LCD_W - BATTERY_W_INNER) / 2,
                      BATTERY_TOP_INNER + BATTERY_H_INNER - h, BATTERY_W_INNER,
                      h, SOLID, color);
  lcd->drawFilledRect((LCD_W - BATTERY_CONNECTOR_W) / 2,
                      BATTERY_TOP - BATTERY_CONNECTOR_H, BATTERY_CONNECTOR_W,
                      BATTERY_CONNECTOR_H, SOLID, COLOR_THEME_PRIMARY2);
}

#define CHARGE_INFO_DURATION 5000 // ms

//this method should be called by timer interrupt or by GPIO interrupt
void handle_battery_charge(uint32_t last_press_time)
{
#if !defined(SIMU)
  static uint32_t updateTime = 0;
  static uint16_t lastState = CHARGE_UNKNOWN;
  static uint32_t info_until = 0;
  static bool lcdInited = false;

  uint32_t now = timersGetMsTick();
  uint16_t chargeState = get_battery_charge_state();
  if(chargeState != CHARGE_UNKNOWN) {

    if(lastState != chargeState) {
      //avoid positive check when none and unknown
      if(lastState + chargeState > 1) {
        //charge state changed - last state known
        info_until = now + (CHARGE_INFO_DURATION);
      }
    }
    //power buttons pressed
    else if(now - last_press_time < POWER_ON_DELAY) {
      info_until = now + CHARGE_INFO_DURATION;
    }
    lastState = chargeState;
  }

  if(now > info_until) {
    info_until = 0;
    lcd->clear();
    BACKLIGHT_DISABLE();
    if(lcdInited) {
      lcdOff();
    }
    return;
  }

  if(updateTime == 0 || ((timersGetMsTick() - updateTime) >= 500))
  {
      if(!lcdInited) {
        lcdInitDisplayDriver();
        lcdInited = true;
      }
      else {
        lcdOn();
      }
      updateTime = timersGetMsTick();     
      lcdInitDirectDrawing();
      drawChargingInfo(chargeState);
      lcdRefresh();
   }
#endif
}
