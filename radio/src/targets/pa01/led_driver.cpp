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
#include "hal/rgbleds.h"
#include "stm32_gpio.h"
#include "stm32_ws2812.h"

#include "boards/generic_stm32/rgb_leds.h"
#include "board.h"
#if defined(LED_STRIP_GPIO)
#include "boards/generic_stm32/rgb_leds.h"
#endif

#include "timers_driver.h"

#define GET_RED(color) (((color) & 0xF80000) >>16)
#define GET_GREEN(color) (((color) & 0x000F800) >> 8)
#define GET_BLUE(color) (((color) & 0xF8))

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
static const uint32_t fsLeds[] = {FSLED_GPIO_PIN_1, FSLED_GPIO_PIN_2,
				  FSLED_GPIO_PIN_3, FSLED_GPIO_PIN_4,
				  FSLED_GPIO_PIN_5, FSLED_GPIO_PIN_6};
#endif

void ledInit()
{
#if defined(LED_GREEN_GPIO)
  gpio_init(LED_GREEN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_RED_GPIO)
  gpio_init(LED_RED_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(LED_BLUE_GPIO)
  gpio_init(LED_BLUE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(FUNCTION_SWITCHES) && !defined(FUNCTION_SWITCHES_RGB_LEDS)
  for (size_t i = 0; i < DIM(fsLeds); i++) {
    gpio_init(fsLeds[i], GPIO_OUT, GPIO_PIN_SPEED_LOW);
  }
#endif
}

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
// used to map switch number to led number in the rgbled chain
uint8_t ledMapping[] = {4, 6, 0, 2};

void fsLedRGB(uint8_t index, uint32_t color)
{
  ws2812_set_color(ledMapping[index], GET_RED(color),
     GET_GREEN(color),GET_BLUE(color));
  ws2812_set_color(ledMapping[index]+1, GET_RED(color),
     GET_GREEN(color),GET_BLUE(color));
}

uint32_t fsGetLedRGB(uint8_t index)
{
  return rgbGetLedColor(ledMapping[index]);
}
uint8_t getRGBColorIndex(uint32_t color)
{
  for (uint8_t i = 0; i < (sizeof(colorTable) / sizeof(colorTable[0])); i++) {
    if (color == colorTable[i])
      return(i);
  }
  return 5; // Custom value set with Companion
}
#elif defined(FUNCTION_SWITCHES)
void fsLedOff(uint8_t index)
{
  gpio_clear(fsLeds[index]);
}

void fsLedOn(uint8_t index)
{
  gpio_set(fsLeds[index]);
}

bool fsLedState(uint8_t index)
{
  return gpio_read(fsLeds[index]) ? true : false;
}
#endif

void ledOff()
{
  ws2812_set_color(8, 0, 0, 0);
  ws2812_set_color(9, 0, 0, 0);
}

void ledRed()
{
  ws2812_set_color(8, 20, 0, 0);
  ws2812_set_color(9, 20, 0, 0);
}

void ledGreen()
{
  ws2812_set_color(8, 0, 20, 0);
  ws2812_set_color(9, 0, 20, 0);
}

void ledBlue()
{
  ws2812_set_color(8, 0, 0, 20);
  ws2812_set_color(9, 0, 0, 20);
}

#define BREATH_INTERVAL 7
#define BRIGHTNESS_MAX  0xF8
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

uint8_t ledBreathBright(float angle) {
  float brightness = sin(angle) * 0.5 + 0.5; // map range 0-1
  brightness = pow(brightness, GAMMA); // gamma correction
  uint8_t bright = (uint8_t)(brightness * 255);
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
 
  bright = ledBreathBright(breath_angle);
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
  } else if (led_info.led_state == RGB_STATE_POWER_ON) {
    rgbLedColorApply();
    return;
  } else if (led_info.led_state == RGB_STATE_NONE) {
    return;
  }
  ledBreathUpdate(led_info.led_state, led_info.led_color, led_info.led_group);
}