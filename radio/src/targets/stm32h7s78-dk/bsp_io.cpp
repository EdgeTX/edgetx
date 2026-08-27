#include "hal.h"
#include "seesaw.h"
#include "hal/key_driver.h"
#include "stm32_i2c_driver.h"

#define BUTTON_X      (1 << 6)
#define BUTTON_Y      (1 << 2)
#define BUTTON_A      (1 << 5)
#define BUTTON_B      (1 << 1)
#define BUTTON_SELECT (1 << 0)
#define BUTTON_START  (1 << 16)

#define BUTTON_MASK \
  (BUTTON_X | BUTTON_Y | BUTTON_A | BUTTON_B | BUTTON_SELECT | BUTTON_START)

seesaw_t _seesaw = {0, 0};

void keysInit()
{
  if (seesaw_init(&_seesaw, I2C_Bus_1, 0x50) < 0) return;
  seesaw_pin_mode(&_seesaw, BUTTON_MASK, SEESAW_INPUT_PULLUP);
}

uint32_t readKeys()
{
  uint32_t buttons = 0;
  if (seesaw_digital_read(&_seesaw, BUTTON_MASK, &buttons) < 0) return 0;

  uint32_t keys = 0;
  if ((buttons & BUTTON_X) == 0) keys |= (1 << KEY_UP);
  if ((buttons & BUTTON_B) == 0) keys |= (1 << KEY_DOWN);
  if ((buttons & BUTTON_A) == 0) keys |= (1 << KEY_ENTER);
  if ((buttons & BUTTON_Y) == 0) keys |= (1 << KEY_EXIT);

  return keys;
}
