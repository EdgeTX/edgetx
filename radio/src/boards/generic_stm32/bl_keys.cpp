#include "board.h"
#include "hal/key_driver.h"

#if defined(BOOTLOADER_KEYS)

bool boardBLStartCondition()
{
  // Trims combo activated
  bool result = (readTrims() == BOOTLOADER_KEYS);
#if defined(SECONDARY_BOOTLOADER_KEYS)
  result |= (readTrims() == SECONDARY_BOOTLOADER_KEYS);
#endif
  return result;
}

#elif (defined(RADIO_T8) || defined(RADIO_COMMANDO8)) && !defined(RADIOMASTER_RELEASE)

bool boardBLStartCondition()
{
  // Bind button pressed
  return (KEYS_GPIO_REG_BIND->IDR & KEYS_GPIO_PIN_BIND) == 0;
}

#endif
