#include "boards/generic_stm32/rgb_leds.h"
#include "stm32_ws2812.h"

#include "switches.h"
#include "hal.h"

void rgbLedOnUpdate()
{
  for (uint8_t i = 0; i < 6; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }

  uint8_t pos = getXPotPosition(SIXPOS_SWITCH_INDEX);
  ws2812_set_color(pos, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
}
