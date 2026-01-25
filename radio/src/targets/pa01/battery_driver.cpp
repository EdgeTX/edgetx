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

#include "boards/generic_stm32/rgb_leds.h"
#undef UNUSED
#include "bsp_io.h"
#include "edgetx.h"
#include "mainwindow.h"
#include "static.h"
#include "stm32_ws2812.h"
#include "LvglWrapper.h"

#define  __BATTERY_DRIVER_C__

#define BATTERY_W 140
#define BATTERY_H (LCD_H - 120)
#define BATTERY_TOP ((LCD_H - BATTERY_H)/2)
#define BATTERY_CONNECTOR_W 32
#define BATTERY_CONNECTOR_H 10
#define BATTERY_BORDER 4
#define BATTERY_W_INNER (BATTERY_W - 2*BATTERY_BORDER)
#define BATTERY_H_INNER (BATTERY_H - 2*BATTERY_BORDER)
#define BATTERY_TOP_INNER (BATTERY_TOP + BATTERY_BORDER)

#define UCHARGER_SAMPLING_CNT              10
#define UCHARGER_CHARGING_SAMPLING_CNT     10

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
  //    DISABLE_UCHARGER();
    }
  }

  return state;
}

bool isChargerActive()
{  
  if (uCharger.isChargerDetectionReady) {
    // Detect the removal of the charger.
    get_battery_charge_state();
  } else {
    while (!uCharger.isChargerDetectionReady)
    {
      get_battery_charge_state();
      delay_ms(10);
    }
  }
  return uCharger.hasCharger;
}

void battery_charge_init()
{
  // USB charger status pins
  gpio_init(UCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(UCHARGER_CHARGE_END_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);

  // USB charger state init
  ENABLE_UCHARGER();
  uCharger.hasCharger = !IS_UCHARGER_ACTIVE();  // Init for sampling count works
  uCharger.isChargerDetectionReady = false;
  resetChargeEndDetection(&uCharger);
}

void ledChargingInfo(uint16_t chargeState) {
#if defined(LED_STRIP_GPIO)
  static int ledIdx = LED_CHARGING_END;
  if (ledIdx == LED_CHARGING_END)
    ledIdx = LED_CHARGING_START;
  else
    ledIdx += LED_CHARGING_START > LED_CHARGING_END ? -1 : 1;
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    uint8_t green = 0;
    if (CHARGE_FINISHED == chargeState) {
      if (LED_CHARGING_START > LED_CHARGING_END && i <= LED_CHARGING_START && i >= LED_CHARGING_END)
      {
        green = 20;
      }
      if (LED_CHARGING_START <= LED_CHARGING_END && i >= LED_CHARGING_START && i <= LED_CHARGING_END)
      {
        green = 20;
      }
    } else if (ledIdx == i) {
      green = 20;
    }
    rgbSetLedColor(i, 0, green, 0);
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
  chargeWindow->clear();
  delete chargeWindow;
  chargeWindow = nullptr;
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

  if (!lcdInited) {
    lcdInited = true;
    backlightInit();
    lcdInitDisplayDriver();
  }

  if (updateTime == 0 || ((timersGetMsTick() - updateTime) >= 500))
  {
    updateTime = timersGetMsTick();
    ledChargingInfo(chargeState);

    if(now > info_until) {
      info_until = 0;
      BACKLIGHT_DISABLE();
      if(lcdInited) {
        lcdOff();
      }
    } else {
      if (lcdInited) {
        lcdOn();
      }
      drawChargingInfo(chargeState);

      // DEBUG INFO - TODO delete or replace with LVGL objects
#if 0
      char buffer[1024];

      sprintf(buffer, "%d,%d,%d,%d", uCharger.isChargerDetectionReady, uCharger.hasCharger, IS_UCHARGER_ACTIVE(), uCharger.chargerSamplingCount);
      lcd->drawSizedText(100, 10, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
    
      sprintf(buffer, "%d,%d,%d,%d,%d,", uCharger.isChargingDetectionReady, uCharger.isChargeEnd, IS_UCHARGER_CHARGE_END_ACTIVE(), uCharger.chargingSamplingCount, uCharger.chargeEndSamplingCount);
      lcd->drawSizedText(100, 40, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);

      sprintf(buffer, "%d", isChargerActive());
      lcd->drawSizedText(100, 130, buffer, strlen(buffer), CENTERED | COLOR_THEME_PRIMARY2);
#endif
    }

  }
#endif
}

#define MAX_BRIGHT      255
#define CHARGING_BRIGHT  20

void rgbPowerOn(uint8_t color) {
  // Sequential RGB On
  static uint8_t power_on_step = RGB_STEP_POWER_AROUND;
  switch (power_on_step)
  {
    case RGB_STEP_POWER_AROUND:
      setLedGroupColor(4, color, MAX_BRIGHT);
      setLedGroupColor(5, color, MAX_BRIGHT);
      setLedGroupColor(6, color, MAX_BRIGHT);
      break;

    case RGB_STEP_FUNC1:
      setLedGroupColor(0, color, MAX_BRIGHT);
      break;
      
    case RGB_STEP_FUNC2:
      setLedGroupColor(1, color, MAX_BRIGHT);
      break;   

    case RGB_STEP_FUNC3:
      setLedGroupColor(2, color, MAX_BRIGHT);
      break;  

    case RGB_STEP_FUNC4:
      setLedGroupColor(3, color, MAX_BRIGHT);
      break;  

  default:
    break;
  }
  power_on_step++;
}

#define BRIGHTNESS_MAX       255
void rgbBatteryDis(uint8_t color, uint8_t power_level ) {

  switch (power_level)
  {
  case POWER_LEVEL_CRITICAL:
      setLedGroupColor(0, color, 0);
      setLedGroupColor(1, color, 0);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
      break;
  case POWER_LEVEL_LOW:
      setLedGroupColor(0, color, BRIGHTNESS_MAX);
      setLedGroupColor(1, color, 0);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
    break;
  case POWER_LEVEL_MEDIUM:
      setLedGroupColor(0, color, BRIGHTNESS_MAX);
      setLedGroupColor(1, color, BRIGHTNESS_MAX);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
    break;
  case POWER_LEVEL_HIGH:
      setLedGroupColor(0, color, BRIGHTNESS_MAX);
      setLedGroupColor(1, color, BRIGHTNESS_MAX);
      setLedGroupColor(2, color, BRIGHTNESS_MAX);
      setLedGroupColor(3, color, 0);
    break;
  case POWER_LEVEL_NEAR_FULL:
  case POWER_LEVEL_FULL:
      setLedGroupColor(0, color, BRIGHTNESS_MAX);
      setLedGroupColor(1, color, BRIGHTNESS_MAX);
      setLedGroupColor(2, color, BRIGHTNESS_MAX);
      setLedGroupColor(3, color, BRIGHTNESS_MAX);
    break;
  default:
    break;
  }
    setLedGroupColor(4, color, BRIGHTNESS_MAX);
    setLedGroupColor(5, color, BRIGHTNESS_MAX);
    setLedGroupColor(6, color, BRIGHTNESS_MAX);
}

void rgbBatteryLevelInfo(uint8_t power_level, uint8_t rgb_state) {
  uint8_t color = 0;
  uint8_t breath_index = 0;
  switch (power_level)
  {
  case POWER_LEVEL_CRITICAL:
    color = RGB_COLOR_RED;
    if (rgb_state == RGB_STATE_CHARGE) {
      color = RGB_COLOR_RED;
      breath_index |= RGB_GROUP_MASK_FUNC_1;
      setLedGroupColor(1, color, 0);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
    }
    break;

  case POWER_LEVEL_LOW:
    color = RGB_COLOR_PURPLE;
    if (rgb_state == RGB_STATE_CHARGE) {
      breath_index |= RGB_GROUP_MASK_FUNC_1;
      setLedGroupColor(1, color, 0);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
    }
    break; 

  case POWER_LEVEL_MEDIUM:
    color = RGB_COLOR_YELLOW;
    if (rgb_state == RGB_STATE_CHARGE) {
      breath_index |= RGB_GROUP_MASK_FUNC_2;
      setLedGroupColor(0, color, CHARGING_BRIGHT);
      setLedGroupColor(2, color, 0);
      setLedGroupColor(3, color, 0);
    }
    break;  

  case POWER_LEVEL_HIGH:
    color = RGB_COLOR_GREEN;
    if (rgb_state == RGB_STATE_CHARGE) {
      breath_index |= RGB_GROUP_MASK_FUNC_3;
      setLedGroupColor(0, color, CHARGING_BRIGHT);
      setLedGroupColor(1, color, CHARGING_BRIGHT);
      setLedGroupColor(3, color, 0);
    }
    break;  

  case POWER_LEVEL_NEAR_FULL:
    color = RGB_COLOR_GREEN;   
    if (rgb_state == RGB_STATE_CHARGE) {
      breath_index |= RGB_GROUP_MASK_FUNC_4;
      setLedGroupColor(0, color, CHARGING_BRIGHT);
      setLedGroupColor(1, color, CHARGING_BRIGHT);
      setLedGroupColor(2, color, CHARGING_BRIGHT);
    }
    break;  

  case POWER_LEVEL_FULL:
    color = RGB_COLOR_GREEN;
    if (rgb_state == RGB_STATE_CHARGE) {
      setLedGroupColor(0, color, CHARGING_BRIGHT);
      setLedGroupColor(1, color, CHARGING_BRIGHT);
      setLedGroupColor(2, color, CHARGING_BRIGHT);
      setLedGroupColor(3, color, CHARGING_BRIGHT);
    }
    break;  
  default:
    break;
  }

  switch (rgb_state)
  {
  case RGB_STATE_BREATH:
    ledSetGroup(RGB_GROUP_MASK_ALL);
    break;

  case RGB_STATE_CHARGE:
    breath_index |= RGB_GROUP_MASK_AROUND_L | RGB_GROUP_MASK_AROUND_R | RGB_GROUP_MASK_POWER;
    ledSetGroup(breath_index);
    break;
  
  case RGB_STATE_POWER_ON:
    rgbPowerOn(color);
    break;
  case RGB_STATE_BAT_DIS:
    rgbBatteryDis(color, power_level);
  default:
    break;
  }

  ledSetState(rgb_state);
  ledSetColor(color);
}

#define BREATH_INTERVAL 7
#define GAMMA       6
#define PI 3.14159265358979323846f
#define BREATH_STEP 0.0089759771428571  // PI / 350.0f; // step size

typedef struct 
{
  uint8_t led_color;
  uint8_t led_state;
  uint8_t led_group;
} ledInfo_t;

// used to map all led number in the rgbled chain
// function 1~4 power right left
uint8_t rgbMapping[] = {4, 6, 0, 2, 8, 12, 10};

ledInfo_t led_info = {0};

void ledSetColor(uint8_t color) {
  led_info.led_color = color;
}

void ledSetState(uint8_t state) {
  led_info.led_state = state;
}

void ledSetGroup(uint8_t group) {
  led_info.led_group = group;
}

void setLedGroupColor(uint8_t index, uint8_t color, uint8_t brightness) {

  uint8_t scaled_r = 0;
  uint8_t scaled_g = 0;
  uint8_t scaled_b = 0;

  if (color & RGB_COLOR_RED) {
    scaled_r = brightness;
  }
  if (color & RGB_COLOR_GREEN) {
    scaled_g = brightness;
  }
  if (color & RGB_COLOR_BLUE) {
    scaled_b = brightness;
  }

  ws2812_set_color(rgbMapping[index], scaled_r, scaled_g, scaled_b);
  ws2812_set_color(rgbMapping[index] + 1, scaled_r, scaled_g, scaled_b);
}

uint8_t ledBreathBright(float angle, uint8_t maxBright) {
  float brightness = sin(angle) * 0.5 + 0.5; // map range 0-1
  brightness = pow(brightness, GAMMA); // gamma correction
  uint8_t bright = (uint8_t)(brightness * maxBright);
  return bright;
}

void ledBreathUpdate(uint8_t state, uint8_t color, uint8_t group) {
  static uint32_t breath_tick = 0;
  
  if (timersGetMsTick() - breath_tick < BREATH_INTERVAL && breath_tick != 0) {
    if (state == RGB_STATE_BREATH || state == RGB_STATE_CHARGE) {
      return;
    }
  }
  breath_tick = timersGetMsTick();
  uint8_t bright = 0;
  static float breath_angle = 0;
 
  bright = ledBreathBright(breath_angle, state == RGB_STATE_CHARGE ? CHARGING_BRIGHT : MAX_BRIGHT);
  breath_angle += BREATH_STEP;
  if (breath_angle > PI) breath_angle = 0; // reset angle

  if (group & RGB_GROUP_MASK_FUNC_1)    setLedGroupColor(0, color, bright);
  if (group & RGB_GROUP_MASK_FUNC_2)    setLedGroupColor(1, color, bright);
  if (group & RGB_GROUP_MASK_FUNC_3)    setLedGroupColor(2, color, bright);
  if (group & RGB_GROUP_MASK_FUNC_4)    setLedGroupColor(3, color, bright);
  if (group & RGB_GROUP_MASK_POWER)     setLedGroupColor(4, color, bright);
  if (group & RGB_GROUP_MASK_AROUND_L)  setLedGroupColor(5, color, bright);
  if (group & RGB_GROUP_MASK_AROUND_R)  setLedGroupColor(6, color, bright);

  rgbLedColorApply();
}

void ledLoop(void) {
  if (led_info.led_state == RGB_STATE_OFF) {
    led_info.led_state = RGB_STATE_NONE;
    rgbLedClearAll();
    return;
  } else if ( led_info.led_state == RGB_STATE_POWER_ON || led_info.led_state == RGB_STATE_BAT_DIS ) {
    rgbLedColorApply();
    return;
  } else if (led_info.led_state == RGB_STATE_NONE) {
    return;
  }
  ledBreathUpdate(led_info.led_state, led_info.led_color, led_info.led_group);
}

extern const stm32_pulse_timer_t _led_timer;
static uint8_t _led_charge_colors[WS2812_BYTES_PER_LED * LED_STRIP_LENGTH];
void rgbChargeInit(void) {
  ws2812_init(&_led_timer, _led_charge_colors, LED_STRIP_LENGTH, WS2812_GRB);
  rgbLedClearAll();
}

constexpr uint16_t vbatLedTable[] = {650, 720, 760, 800, 823 };
void updateBatteryState(uint8_t rgb_state) {
  uint16_t  bat_v = getBatteryVoltage()*BAT_VOL_FACTOR;
  uint8_t power_level = POWER_LEVEL_NONE;
  static uint8_t last_power_level = POWER_LEVEL_NONE;

  if (bat_v < vbatLedTable[0]) {
    power_level = POWER_LEVEL_CRITICAL;
  } else if (bat_v < vbatLedTable[1]) {
    power_level = POWER_LEVEL_LOW;
  } else if (bat_v < vbatLedTable[2]) {
    power_level = POWER_LEVEL_MEDIUM;
  } else if (bat_v < vbatLedTable[3]) {
    power_level = POWER_LEVEL_HIGH;
  }else if (bat_v < vbatLedTable[4]) {
    power_level = POWER_LEVEL_NEAR_FULL;
  } else {
    power_level = POWER_LEVEL_FULL;
  }

  if( last_power_level != POWER_LEVEL_NONE )
  {
    if(power_level<last_power_level)
    {
      power_level=last_power_level;
    }
  }

  rgbBatteryLevelInfo(power_level, rgb_state);
  ledLoop();
  last_power_level = power_level;
}

