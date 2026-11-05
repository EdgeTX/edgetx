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

#include "battery_driver.h"
#include "hal/adc_driver.h"

#include "boards/generic_stm32/rgb_leds.h"
#include "edgetx.h"
#include "mainwindow.h"
#include "static.h"
#include "LvglWrapper.h"

#define  __BATTERY_DRIVER_C__

// Debug driver
//#define BATTERY_DRIVER_DEBUG

#define BATTERY_W 140
#define BATTERY_H (LCD_H - 120)
#define BATTERY_TOP ((LCD_H - BATTERY_H)/2)
#define BATTERY_CONNECTOR_W 32
#define BATTERY_CONNECTOR_H 10
#define BATTERY_BORDER 4
#define BATTERY_W_INNER (BATTERY_W - 2*BATTERY_BORDER)
#define BATTERY_H_INNER (BATTERY_H - 2*BATTERY_BORDER)
#define BATTERY_TOP_INNER (BATTERY_TOP + BATTERY_BORDER)

#define UCHARGER_SAMPLING_CNT                10
#if defined(RADIO_PL18U)
  #define UCHARGER_CHARGING_SAMPLING_CNT     1
  #define UCHARGER_CHARGE_END_SAMPLING_CNT   250
#else
  #define UCHARGER_CHARGING_SAMPLING_CNT     10
  #define UCHARGER_CHARGE_END_SAMPLING_CNT   10
#endif

#define WCHARGER_SAMPLING_CNT                30
#if defined(RADIO_PL18U)
  #define WCHARGER_CHARGING_SAMPLING_CNT     1
  #define WCHARGER_CHARGE_END_SAMPLING_CNT   250
#else
  #define WCHARGER_CHARGING_SAMPLING_CNT     10
  #define WCHARGER_CHARGE_END_SAMPLING_CNT   10
#endif
#define WCHARGER_LOW_CURRENT_DELAY_CNT       6000
#define WCHARGER_HIGH_CURRENT_DELAY_CNT      24000
#define CHARGER_DETECTION_TIMEOUT            40    // 400ms

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

static STRUCT_BATTERY_CHARGER uCharger; // USB charger

#if defined(WIRELESS_CHARGER)
  static STRUCT_BATTERY_CHARGER wCharger; // Wireless charger
  static uint16_t wirelessLowCurrentDelay = 0;
  static uint16_t wirelessHighCurrentDelay = 0;
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

void chargeEndDetection(STRUCT_BATTERY_CHARGER* charger, uint8_t chargeEndPinActive,
                        uint8_t chargingSamplingCountThreshold, uint8_t chargeEndSamplingCountThreshold)
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
        if (charger->chargeEndSamplingCount >= chargeEndSamplingCountThreshold)
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
      if (charger->chargingSamplingCount >= chargingSamplingCountThreshold)
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
        if (charger->chargingSamplingCount >= chargingSamplingCountThreshold)
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
      if (charger->chargeEndSamplingCount >= chargeEndSamplingCountThreshold)
      {
        charger->chargeEndSamplingCount = 0;
        charger->isChargeEnd = true;
        charger->isChargingDetectionReady = true;
      }
    }
  }
}

uint16_t get_uCharger_state(uint16_t state)
{
  chargerDetection(&uCharger, IS_UCHARGER_ACTIVE(), UCHARGER_SAMPLING_CNT);
  if (uCharger.isChargerDetectionReady)
  {
    if (uCharger.hasCharger)  // USB charger can be detected properly no matter it is enabled or not
    {
#if !defined(RADIO_PL18U)
      // Enable USB charger when USB power is detected
      ENABLE_UCHARGER();
#endif
      chargeEndDetection(&uCharger, IS_UCHARGER_CHARGE_END_ACTIVE(), UCHARGER_CHARGING_SAMPLING_CNT, UCHARGER_CHARGE_END_SAMPLING_CNT);
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

#if !defined(RADIO_PL18U)
      // Disable USB charger if it is not present, so that wireless charger can be detected properly
      DISABLE_UCHARGER();
#endif
    }
  }

  return state;
}

uint16_t get_wCharger_state(uint16_t state)
{
#if defined(WIRELESS_CHARGER)
  chargerDetection(&wCharger, IS_WCHARGER_ACTIVE(), WCHARGER_SAMPLING_CNT);
  if (wCharger.isChargerDetectionReady)
  {
    if (wCharger.hasCharger)  // Wireless charger can only be detected when USB charger is disabled
    {
#if defined(RADIO_PL18U)
      // Disable USB charger when wireless charger is present, otherwise USB charger detection will be wrong
      DISABLE_UCHARGER();
      ENABLE_WCHARGER();
#endif
      chargeEndDetection(&wCharger, IS_WCHARGER_CHARGE_END_ACTIVE(), WCHARGER_CHARGING_SAMPLING_CNT, WCHARGER_CHARGE_END_SAMPLING_CNT);
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
#if defined(RADIO_PL18U)
      // Enable USB charger only if wireless charger is not present
      DISABLE_WCHARGER();
      ENABLE_UCHARGER();
#endif

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

#endif  // defined(WIRELESS_CHARGER)

  return state;
}

#if defined(RADIO_PL18EV)
/*
 * PL18EV: charge state like the FlySky firmware (charger task every 100 ms)
 * - charge pin (USB PB13 / wireless PI10): a change counts after 10 equal
 *   samples (1 s)
 * - "full" only if the pin reports charge end AND the battery voltage
 *   (5 samples without min/max) is >= 4.15 V; once full it stays full until
 *   the charger is removed. On the PL18EV the pin is high while charging
 *   too, so the pin alone cannot tell "full".
 * - the charger IC is enabled for 97 s, then switched off for 3 s (restarts
 *   the charge cycle); the pin is not evaluated during the pause
 * - only one charger is enabled: USB has priority, the wireless charger is
 *   only used without USB
 * - wireless charge current: high after 240 s on the pad, low again after
 *   60 s without
 */
#define EV_STEP_MS          100
#define EV_DEBOUNCE         10    // 1 s
#define EV_RUN_STEPS        970   // 97 s charger on
#define EV_PAUSE_STEPS      30    // 3 s charger off
#define EV_FULL_VOLTAGE     415   // 4.15 V (10 mV)
#define EV_WL_HIGH_STEPS    2400  // 240 s
#define EV_WL_LOW_STEPS     600   // 60 s

struct EvChargeEnd {
  bool end;      // charge pin reports "end"
  bool ready;    // pin state confirmed since the charger was enabled
  uint8_t cntLow;
  uint8_t cntHigh;
};

static EvChargeEnd evUsbEnd;
static EvChargeEnd evWlEnd;
static uint32_t evLastStep = 0;
static bool evRun = false;        // charger IC enabled (false = off / pause)
static uint16_t evRunCnt = EV_PAUSE_STEPS;
static bool evFull = false;
static uint16_t evVolt[5];
static uint8_t evVoltIdx = 0;
static uint8_t evVoltCnt = 0;
static uint16_t evWlOn = 0;
static uint16_t evWlOff = 0;
static bool evWlHigh = false;
static uint16_t evState = CHARGE_UNKNOWN;
static bool evAdcReady = false;   // ADC initialised for the charge screen

void battery_charge_adc_ready(bool ready) { evAdcReady = ready; }

static void evEndReset(EvChargeEnd& e)
{
  e.end = false;
  e.ready = false;
  e.cntLow = 0;
  e.cntHigh = 0;
}

static void evEndDetect(EvChargeEnd& e, bool pinHigh)
{
  uint8_t& same = pinHigh ? e.cntHigh : e.cntLow;
  (pinHigh ? e.cntLow : e.cntHigh) = 0;
  if (e.end == pinHigh) {
    // pin confirms the current state
    if (!e.ready && ++same >= EV_DEBOUNCE) {
      same = 0;
      e.ready = true;
    }
  } else if (++same >= EV_DEBOUNCE) {
    same = 0;
    e.end = pinHigh;
    e.ready = true;
  }
}

// average of 5 voltage samples without the highest and the lowest one
static uint16_t evVoltage()
{
  if (evVoltCnt < 5) return 0;
  uint32_t sum = 0;
  uint16_t vmin = 0xFFFF, vmax = 0;
  for (uint8_t i = 0; i < 5; i++) {
    sum += evVolt[i];
    if (evVolt[i] < vmin) vmin = evVolt[i];
    if (evVolt[i] > vmax) vmax = evVolt[i];
  }
  return (sum - vmin - vmax) / 3;
}

static void evStep()
{
  bool usb = uCharger.isChargerDetectionReady && uCharger.hasCharger;
  bool wl = !usb && wCharger.isChargerDetectionReady && wCharger.hasCharger;

  if (evAdcReady) {
    adcRead();
    evVolt[evVoltIdx] = getBatteryVoltage();
    evVoltIdx = (evVoltIdx + 1) % 5;
    if (evVoltCnt < 5) evVoltCnt++;
  }

  // charge pin only while the charger IC is enabled
  if (!usb || !evRun) evEndReset(evUsbEnd);
  else evEndDetect(evUsbEnd, gpio_read(UCHARGER_CHARGE_END_GPIO));
  if (!wl || !evRun) evEndReset(evWlEnd);
  else evEndDetect(evWlEnd, gpio_read(WCHARGER_CHARGE_END_GPIO));

  if (!usb && !wl) {
    evFull = false;
  } else {
    EvChargeEnd& e = usb ? evUsbEnd : evWlEnd;
    if (e.ready && e.end && evVoltage() >= EV_FULL_VOLTAGE) evFull = true;
  }

  // enable cycle: 97 s on, 3 s off
  if (usb || wl) {
    if (evRun) {
      if (++evRunCnt >= EV_RUN_STEPS) { evRunCnt = 0; evRun = false; }
    } else if (++evRunCnt >= EV_PAUSE_STEPS) {
      evRunCnt = 0;
      evRun = true;
    }
  } else {
    evRun = false;
    evRunCnt = EV_PAUSE_STEPS;  // enable at once when a charger appears
  }
  if (evRun && usb) {
    ENABLE_UCHARGER();
    DISABLE_WCHARGER();
  } else if (evRun && wl) {
    DISABLE_UCHARGER();
    ENABLE_WCHARGER();
  } else {
    DISABLE_UCHARGER();
    DISABLE_WCHARGER();
  }

  // wireless charge current
  if (wl) {
    evWlOff = 0;
    if (evWlOn < EV_WL_HIGH_STEPS) evWlOn++;
    else evWlHigh = true;
  } else if (evWlHigh) {
    if (++evWlOff >= EV_WL_LOW_STEPS) { evWlHigh = false; evWlOn = 0; }
  } else {
    evWlOn = 0;
  }
  wCharger.isHighCurrent = evWlHigh;
  if (evWlHigh) gpio_set(WCHARGER_I_CONTROL_GPIO);
  else gpio_clear(WCHARGER_I_CONTROL_GPIO);

  if (!usb && !wl)
    evState = CHARGE_NONE;
  else
    evState = evFull ? CHARGE_FINISHED : CHARGE_STARTED;
}

uint16_t get_battery_charge_state()
{
  // charger presence: every call (10 ms), 10 equal samples
  chargerDetection(&uCharger, IS_UCHARGER_ACTIVE(), UCHARGER_SAMPLING_CNT);
  if (uCharger.isChargerDetectionReady && uCharger.hasCharger) {
    // with USB the wireless charger is not used
    wCharger.hasCharger = false;
    wCharger.isChargerDetectionReady = true;
    wCharger.chargerSamplingCount = 0;
  } else {
    chargerDetection(&wCharger, IS_WCHARGER_ACTIVE(), WCHARGER_SAMPLING_CNT);
  }

  uint32_t now = timersGetMsTick();
  if (evLastStep == 0 || (uint32_t)(now - evLastStep) >= EV_STEP_MS) {
    evLastStep = now ? now : 1;
    evStep();
  }
  return evState;
}
#else
uint16_t get_battery_charge_state()
{
  uint16_t state = CHARGE_UNKNOWN;

#if defined(RADIO_PL18U)
  // PL18U wireless charger takes precedence
  state = get_wCharger_state(state);
  state = get_uCharger_state(state);
#else
  state = get_uCharger_state(state);
  state = get_wCharger_state(state);
#endif

  return state;
}
#endif  // defined(RADIO_PL18EV)

bool isChargerActive()
{  
#if defined(WIRELESS_CHARGER)
  uint16_t timeout = CHARGER_DETECTION_TIMEOUT;
  while (!(uCharger.isChargerDetectionReady && wCharger.isChargerDetectionReady) && timeout--)
  {
    get_battery_charge_state();
    delay_ms(10);
  }
  return uCharger.hasCharger || wCharger.hasCharger;
#else
  uint16_t timeout = CHARGER_DETECTION_TIMEOUT;
  while (!uCharger.isChargerDetectionReady && timeout--)
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
#endif  // defined(WIRELESS_CHARGER)


  // USB charger control pins
#if defined(UCHARGER_EN_GPIO)  
  gpio_init(UCHARGER_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

  // USB charger state init
#if defined(RADIO_PL18U) || defined(RADIO_PL18EV)
  // PL18EV: the charger is enabled once it is detected
  DISABLE_UCHARGER();
#else
  ENABLE_UCHARGER();
#endif  // defined(RADIO_PL18U)
  uCharger.hasCharger = !IS_UCHARGER_ACTIVE();  // Init for sampling count works
  uCharger.isChargerDetectionReady = false;
  resetChargeEndDetection(&uCharger);

#if defined(WIRELESS_CHARGER)
  // Wireless charger control pins
  gpio_init(WCHARGER_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);

#if defined(WCHARGER_I_CONTROL_GPIO)
  gpio_init(WCHARGER_I_CONTROL_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif  // defined(WCHARGER_I_CONTROL_GPIO)

  // Wireless charger state init
#if defined(RADIO_PL18EV)
  // the charger is enabled once it is detected, low current first
  DISABLE_WCHARGER();
  gpio_clear(WCHARGER_I_CONTROL_GPIO);
#else
  ENABLE_WCHARGER();
  WCHARGER_CURRENT_LOW();
#endif
  wCharger.hasCharger = !IS_WCHARGER_ACTIVE();  // Init for sampling count works
  wCharger.isChargerDetectionReady = false;
  resetChargeEndDetection(&wCharger);
#endif  // defined(WIRELESS_CHARGER)
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

static Window* chargeWindow = nullptr;

void drawChargingInfo(uint16_t chargeState) {
  static int progress = -1;
  static StaticText* stateText = nullptr;
  static lv_obj_t* battBox = nullptr;

  const char* text;
  int h;
  LcdColorIndex color = COLOR_THEME_EDIT_INDEX;

  switch (chargeState) {
    case CHARGE_STARTED:
      progress += 1;
      if (progress > 4) progress = 0;
      text = STR_BATTERYCHARGING;
      h = ((BATTERY_H_INNER * progress) / 4);
      break;
    case CHARGE_FINISHED:
      text = STR_BATTERYFULL;
      h = BATTERY_H_INNER;
      break;
    default:
      text = STR_BATTERYNONE;
      h = BATTERY_H_INNER;
      color = COLOR_THEME_PRIMARY1_INDEX;
      break;
  }

  if (chargeWindow == nullptr) {
    // Ensure lvgl is initialised before creating windows
    LvglWrapper::instance();
    chargeWindow = new Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H});
    etx_solid_bg(chargeWindow->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);

    stateText = new StaticText(chargeWindow, {0, LCD_H - 50, LCD_W, 50}, "", COLOR_THEME_PRIMARY2_INDEX, CENTERED);
    
    lv_obj_t* box = lv_obj_create(chargeWindow->getLvObj());
    lv_obj_set_pos(box, (LCD_W - BATTERY_W) / 2, BATTERY_TOP);
    lv_obj_set_size(box, BATTERY_W, BATTERY_H);
    lv_obj_set_style_border_opa(box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(box, 2, LV_PART_MAIN);
    etx_border_color(box, COLOR_THEME_PRIMARY2_INDEX);
    
    box = lv_obj_create(chargeWindow->getLvObj());
    lv_obj_set_pos(box, (LCD_W - BATTERY_CONNECTOR_W) / 2, BATTERY_TOP - BATTERY_CONNECTOR_H);
    lv_obj_set_size(box, BATTERY_CONNECTOR_W, BATTERY_CONNECTOR_H);
    etx_solid_bg(box, COLOR_THEME_PRIMARY2_INDEX);

    battBox = lv_obj_create(chargeWindow->getLvObj());
  }

  stateText->setText(text);

  lv_obj_set_pos(battBox, (LCD_W - BATTERY_W_INNER) / 2, BATTERY_TOP_INNER + BATTERY_H_INNER - h);
  lv_obj_set_size(battBox, BATTERY_W_INNER, h);
  etx_solid_bg(battBox, color);

  LvglWrapper::instance()->run();
  lv_refr_now(nullptr);

  BACKLIGHT_ENABLE();
}

void battery_charge_end()
{
  if (chargeWindow) {
    chargeWindow->clear();
    delete chargeWindow;
    chargeWindow = nullptr;
  }
}

#define CHARGE_INFO_DURATION 5000 // ms

// This method should be called by timer interrupt or by GPIO interrupt
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
      // Avoid positive check when none and unknown
      if (lastState + chargeState > 1) {
        // Charge state changed - last state known
        info_until = now + (CHARGE_INFO_DURATION);
      }
    }
    // Power buttons pressed
    else if (now - last_press_time < POWER_ON_DELAY) {
      info_until = now + CHARGE_INFO_DURATION;
    }
    lastState = chargeState;
  }

  if (!lcdInited) {
    lcdInited = true;
    backlightInit();
    // Ensure lvgl is initialised before creating windows
    LvglWrapper::instance();
  }

  if (updateTime == 0 || ((timersGetMsTick() - updateTime) >= 500))
  {
    updateTime = timersGetMsTick();
    ledChargingInfo(chargeState);

#if !defined(BATTERY_DRIVER_DEBUG)
    if(now > info_until) {
      info_until = 0;
      BACKLIGHT_DISABLE();
      if(lcdInited) {
        lcdOff();
      }
    } else {
#endif
      if (lcdInited) {
        lcdOn();
      }
      drawChargingInfo(chargeState);

      // DEBUG INFO - TODO delete or replace with LVGL objects
#if defined(BATTERY_DRIVER_DEBUG)
      char buffer[1024];

      sprintf(buffer, "%d,%d,%d,%d", uCharger.isChargerDetectionReady, uCharger.hasCharger, IS_UCHARGER_ACTIVE(), uCharger.chargerSamplingCount);
      lcd->drawSizedText(100, 10, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
    
      sprintf(buffer, "%d,%d,%d,%d,%d,", uCharger.isChargingDetectionReady, uCharger.isChargeEnd, IS_UCHARGER_CHARGE_END_ACTIVE(), uCharger.chargingSamplingCount, uCharger.chargeEndSamplingCount);
      lcd->drawSizedText(100, 40, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);

#if defined(WIRELESS_CHARGER)
      sprintf(buffer, "%d,%d,%d,%d,%d", wCharger.isChargerDetectionReady, wCharger.hasCharger, IS_WCHARGER_ACTIVE(), wCharger.chargerSamplingCount, wCharger.isHighCurrent);
      lcd->drawSizedText(100, 70, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
    
      sprintf(buffer, "%d,%d,%d,%d,%d,", wCharger.isChargingDetectionReady, wCharger.isChargeEnd, IS_WCHARGER_CHARGE_END_ACTIVE(), wCharger.chargingSamplingCount, wCharger.chargeEndSamplingCount);
      lcd->drawSizedText(100, 100, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
#endif

      sprintf(buffer, "%d", isChargerActive());
      lcd->drawSizedText(100, 130, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
#endif
#if !defined(BATTERY_DRIVER_DEBUG)
    }
#endif
  }
#endif
}

