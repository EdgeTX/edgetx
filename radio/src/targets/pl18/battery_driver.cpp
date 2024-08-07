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

#include "edgetx.h"
#include "battery_driver.h"
#include "boards/generic_stm32/rgb_leds.h"

#define  __BATTERY_DRIVER_C__

#define BATTERY_W 140
#define BATTERY_H 200
#define BATTERY_TOP ((LCD_H - BATTERY_H)/2)
#define BATTERY_CONNECTOR_W 32
#define BATTERY_CONNECTOR_H 10
#define BATTERY_BORDER 4
#define BATTERY_W_INNER (BATTERY_W - 2*BATTERY_BORDER)
#define BATTERY_H_INNER (BATTERY_H - 2*BATTERY_BORDER)
#define BATTERY_TOP_INNER (BATTERY_TOP + BATTERY_BORDER)

#define UCHARGER_SAMPLING_CNT              10
#define UCHARGER_CHARGING_SAMPLING_CNT     10
#define WCHARGER_SAMPLING_CNT              30
#define WCHARGER_CHARGING_SAMPLING_CNT     10
#define WCHARGER_LOW_CURRENT_DELAY_CNT   6000
#define WCHARGER_HIGH_CURRENT_DELAY_CNT 24000

typedef struct
{
  bool hasCharger : 1;
  bool isChargeEnd : 1;
  bool isChargerDetectionReady : 1;
  bool isChargingDetectionReady : 1;
  bool isHighCurrent : 1;
  uint8_t chargerSamplingCount;
  uint8_t chargingSamplingCount;
  uint8_t chargeEndSamplingCount;
} STRUCT_BATTERY_CHARGER;

STRUCT_BATTERY_CHARGER uCharger; // USB charger
#if defined(WIRELESS_CHARGER)
STRUCT_BATTERY_CHARGER wCharger; // Wireless charger
uint16_t wirelessLowCurrentDelay = 0;
uint16_t wirelessHighCurrentDelay = 0;
#endif

void chargerDetection(STRUCT_BATTERY_CHARGER* charger, uint8_t chargerPinActive, uint8_t samplingCountThreshold)
{
  if ((charger->hasCharger && chargerPinActive) || (!charger->hasCharger && !chargerPinActive))
  {
    charger->chargerSamplingCount = 0;
  }
  else
  {
    charger->chargerSamplingCount++;
    if (charger->chargerSamplingCount >= samplingCountThreshold)
    {
      charger->chargerSamplingCount = 0;
      charger->hasCharger = !charger->hasCharger;
      charger->isChargerDetectionReady = true;
    }
  }
}

void resetChargeEndDetection(STRUCT_BATTERY_CHARGER* charger)
{
  charger->isChargeEnd = false;
  charger->isChargingDetectionReady = false;
  charger->chargingSamplingCount = 0;
  charger->isHighCurrent = false;
}

void chargeEndDetection(STRUCT_BATTERY_CHARGER* charger, uint8_t chargeEndPinActive, uint8_t samplingCountThreshold)
{
  if (charger->isChargeEnd)
  {
    if (chargeEndPinActive)
    {
      charger->chargingSamplingCount = 0;
      if (charger->isChargingDetectionReady)
      {
        charger->chargeEndSamplingCount = 0;
      }
      else
      {
        charger->chargeEndSamplingCount++;
        if (charger->chargeEndSamplingCount >= samplingCountThreshold)
        {
          charger->chargeEndSamplingCount = 0;
          charger->isChargingDetectionReady = true;
        }
      }
    }
    else
    {
      charger->chargeEndSamplingCount = 0;
      charger->chargingSamplingCount++;
      if (charger->chargingSamplingCount >= samplingCountThreshold)
      {
        charger->chargingSamplingCount = 0;
        charger->isChargeEnd = false;
        charger->isChargingDetectionReady = true;
      }
    }
  }
  else
  {
    if (!chargeEndPinActive)
    {
      charger->chargeEndSamplingCount = 0;
      if (charger->isChargingDetectionReady)
      {
        charger->chargingSamplingCount = 0;
      }
      else
      {
        charger->chargingSamplingCount++;
        if (charger->chargingSamplingCount >= samplingCountThreshold)
        {
          charger->chargingSamplingCount = 0;
          charger->isChargingDetectionReady = true;
        }
      }
    }
    else
    {
      charger->chargingSamplingCount = 0;
      charger->chargeEndSamplingCount++;
      if (charger->chargeEndSamplingCount >= samplingCountThreshold)
      {
        charger->chargeEndSamplingCount = 0;
        charger->isChargeEnd = true;
        charger->isChargingDetectionReady = true;
      }
    }
  }
}

uint16_t get_battery_charge_state()
{
  uint16_t state = CHARGE_UNKNOWN;

  chargerDetection(&uCharger, IS_UCHARGER_ACTIVE(), UCHARGER_SAMPLING_CNT);
  if (uCharger.isChargerDetectionReady)
  {
    if (uCharger.hasCharger)  // USB charger can be detected properly no matter it is enabled or not
    {
      ENABLE_UCHARGER();
      chargeEndDetection(&uCharger, IS_UCHARGER_CHARGE_END_ACTIVE(), UCHARGER_CHARGING_SAMPLING_CNT);
      if (uCharger.isChargingDetectionReady)
      {
        if (uCharger.isChargeEnd)
        {
          state = CHARGE_FINISHED;
        }
        else
        {
          state = CHARGE_STARTED;
        }
      }
    }
    else
    {
      resetChargeEndDetection(&uCharger);

      // Disable USB charger if it is not present, so that wireless charger can be detected properly
      DISABLE_UCHARGER();
    }
  }

#if defined(WIRELESS_CHARGER)
  chargerDetection(&wCharger, IS_WCHARGER_ACTIVE(), WCHARGER_SAMPLING_CNT);
  if (wCharger.isChargerDetectionReady)
  {
    if (wCharger.hasCharger)  // Wireless charger can only be detected when USB charger is disabled
    {
      chargeEndDetection(&wCharger, IS_WCHARGER_CHARGE_END_ACTIVE(), WCHARGER_CHARGING_SAMPLING_CNT);
      if (wCharger.isChargingDetectionReady)
      {
        if (wCharger.isChargeEnd)
        {
          state = CHARGE_FINISHED;
        }
        else
        {
          state = CHARGE_STARTED;
        }
      }

      // Charge current control
      wirelessLowCurrentDelay = 0;
      if (wirelessHighCurrentDelay >= WCHARGER_HIGH_CURRENT_DELAY_CNT)
      {
        wCharger.isHighCurrent = true;
        WCHARGER_CURRENT_HIGH();
      }
      else
      {
        wirelessHighCurrentDelay++;
      }
    }
    else
    {
      resetChargeEndDetection(&wCharger);

      // Charge current control
      wirelessHighCurrentDelay = 0;
      if (wirelessLowCurrentDelay >= WCHARGER_LOW_CURRENT_DELAY_CNT)
      {
        wCharger.isHighCurrent = false;
        WCHARGER_CURRENT_LOW();
      }
      else
      {
        wirelessLowCurrentDelay++;
      }
    }
  }

#endif

  return state;
}

bool isChargerActive()
{  
#if defined(WIRELESS_CHARGER)
  while (!(uCharger.isChargerDetectionReady && wCharger.isChargerDetectionReady))
  {
    get_battery_charge_state();
    delay_ms(10);
  }
  return uCharger.hasCharger || wCharger.hasCharger;
#else
  while (!uCharger.isChargerDetectionReady)
  {
    get_battery_charge_state();
    delay_ms(10);
  }
  return uCharger.hasCharger;
#endif
}

void battery_charge_init()
{
  // USB charger status pins
  gpio_init(UCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(UCHARGER_CHARGE_END_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);

#if defined(WIRELESS_CHARGER)
  // Wireless charger status pins
  gpio_init(WCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(WCHARGER_CHARGE_END_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);

#endif


  // USB charger control pins
#if defined(UCHARGER_EN_GPIO)  
  gpio_init(UCHARGER_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

  // USB charger state init
  ENABLE_UCHARGER();
  uCharger.hasCharger = !IS_UCHARGER_ACTIVE();  // Init for sampling count works
  uCharger.isChargerDetectionReady = false;
  resetChargeEndDetection(&uCharger);

#if defined(WIRELESS_CHARGER)
  // Wireless charger control pins
  gpio_init(WCHARGER_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(WCHARGER_I_CONTROL_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  // Wireless charger state init
  ENABLE_WCHARGER();
  WCHARGER_CURRENT_LOW();
  wCharger.hasCharger = !IS_WCHARGER_ACTIVE();  // Init for sampling count works
  wCharger.isChargerDetectionReady = false;
  resetChargeEndDetection(&wCharger);
#endif
}

void ledChargingInfo(uint16_t chargeState) {
#if defined(LED_STRIP_GPIO)
  static int ledIdx = 0;
  ledIdx--;
  if (ledIdx < 0) {
    ledIdx = LED_STRIP_LENGTH - 1;
  }
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    if (CHARGE_FINISHED == chargeState || ledIdx == i) {
      rgbSetLedColor(i, 0, 20, 0);
    } else {
      rgbSetLedColor(i, 0, 0, 0);
    }
  }
  rgbLedColorApply();
#endif
}

void drawChargingInfo(uint16_t chargeState) {
  static int progress = 0;
  const char* text = chargeState == CHARGE_STARTED ? STR_BATTERYCHARGING : STR_BATTERYFULL;
  int h = 0;
  LcdFlags color = 0;
  if (CHARGE_STARTED == chargeState)
  {
    if (progress >= 100)
    {
      progress = 0;
    }
    else
    {
      progress += 25;
    }
    text = STR_BATTERYCHARGING;
    h = ((BATTERY_H_INNER * progress) / 100);
    color = COLOR_THEME_EDIT;
  }
  else if (CHARGE_FINISHED == chargeState)
  {
    text = STR_BATTERYFULL;
    h = BATTERY_H_INNER;
    color = COLOR_THEME_EDIT;
  }
  else
  {
    text = STR_BATTERYNONE;
    h = BATTERY_H_INNER;
    color = COLOR_THEME_PRIMARY1;
  }

  BACKLIGHT_ENABLE();
  lcd->drawSizedText(LCD_W / 2, LCD_H - 50, text, strlen(text), CENTERED | COLOR_THEME_PRIMARY2);

  lcd->drawFilledRect((LCD_W - BATTERY_W) / 2, BATTERY_TOP, BATTERY_W, BATTERY_H, SOLID, COLOR_THEME_PRIMARY2);
  lcd->drawFilledRect((LCD_W - BATTERY_W_INNER) / 2, BATTERY_TOP_INNER, BATTERY_W_INNER, BATTERY_H_INNER, SOLID, COLOR_THEME_PRIMARY1);

  lcd->drawFilledRect((LCD_W - BATTERY_W_INNER) / 2, BATTERY_TOP_INNER + BATTERY_H_INNER - h, BATTERY_W_INNER, h, SOLID, color);
  lcd->drawFilledRect((LCD_W - BATTERY_CONNECTOR_W) / 2, BATTERY_TOP - BATTERY_CONNECTOR_H, BATTERY_CONNECTOR_W, BATTERY_CONNECTOR_H, SOLID, COLOR_THEME_PRIMARY2);
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
  if (chargeState != CHARGE_UNKNOWN) {

    if (lastState != chargeState) {
      //avoid positive check when none and unknown
      if (lastState + chargeState > 1) {
        //charge state changed - last state known
        info_until = now + (CHARGE_INFO_DURATION);
      }
    }
    //power buttons pressed
    else if (now - last_press_time < POWER_ON_DELAY) {
      info_until = now + CHARGE_INFO_DURATION;
    }
    lastState = chargeState;
  }

  if (updateTime == 0 || ((timersGetMsTick() - updateTime) >= 500))
  {
    updateTime = timersGetMsTick();
    ledChargingInfo(chargeState);

    if(now > info_until) {
      info_until = 0;
      lcd->clear();
      BACKLIGHT_DISABLE();
      if(lcdInited) {
        lcdOff();
      }
    } else {
      if (!lcdInited) {
        backlightInit();
        lcdInit();
        lcdInitDisplayDriver();
        lcdInited = true;
      } else {
        lcdOn();
      }
      lcdInitDirectDrawing();
      lcd->clear();
      drawChargingInfo(chargeState);

      // DEBUG INFO
#if 0
      char buffer[1024];

      sprintf(buffer, "%d,%d,%d,%d", uCharger.isChargerDetectionReady, uCharger.hasCharger, IS_UCHARGER_ACTIVE(), uCharger.chargerSamplingCount);
      lcd->drawSizedText(100, 10, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
    
      sprintf(buffer, "%d,%d,%d,%d,%d,", uCharger.isChargingDetectionReady, uCharger.isChargeEnd, IS_UCHARGER_CHARGE_END_ACTIVE(), uCharger.chargingSamplingCount, uCharger.chargeEndSamplingCount);
      lcd->drawSizedText(100, 40, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);

      sprintf(buffer, "%d,%d,%d,%d,%d", wCharger.isChargerDetectionReady, wCharger.hasCharger, IS_WCHARGER_ACTIVE(), wCharger.chargerSamplingCount, wCharger.isHighCurrent);
      lcd->drawSizedText(100, 70, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
    
      sprintf(buffer, "%d,%d,%d,%d,%d,", wCharger.isChargingDetectionReady, wCharger.isChargeEnd, IS_WCHARGER_CHARGE_END_ACTIVE(), wCharger.chargingSamplingCount, wCharger.chargeEndSamplingCount);
      lcd->drawSizedText(100, 100, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);

      sprintf(buffer, "%d", isChargerActive());
      lcd->drawSizedText(100, 130, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
#endif

      lcdRefresh();
    }

  }
#endif
}

