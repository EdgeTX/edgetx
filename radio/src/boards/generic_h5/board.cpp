/*
 * Copyright (C) EdgeTx
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

#include "stm32_hal_ll.h"
#include "stm32_gpio.h"
#include "stm32_ws2812.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/module_port.h"
#include "hal/abnormal_reboot.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rgbleds.h"

#include "board.h"
#include "system_clock.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/intmodule_heartbeat.h"
#include "boards/generic_stm32/analog_inputs.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "debug.h"
#include "rtc.h"

#include "timers_driver.h"
#include "dataconstants.h"
#include "trainer.h"

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif

#if defined(BOOT)

#if defined(ROTARY_ENCODER_NAVIGATION)
void boardBLInit()
{
  rotaryEncoderInit();
}
#endif

#endif // BOOT

#if !defined(BOOT)
  #include "edgetx.h"
  #if defined(PXX1)
    #include "pulses/pxx1.h"
  #endif
#endif

#if !defined(USB_PD_SUPPORT) && defined(STM32H5)
void disableUsbPdPins()
{
  LL_PWR_DisableUCPDDeadBattery(); // this allows PB13/PB14 to work properly
}
#endif

#if defined(BOOT) && defined(STM32H5)
void boardBLEarlyInit()
{
  SystemClock_Config();

#if !defined(USB_PD_SUPPORT)
  disableUsbPdPins();
#endif
}
#endif

HardwareOptions hardwareOptions;

#if !defined(BOOT)

void boardInit()
{
  SystemClock_Config();

#if !defined(USB_PD_SUPPORT) && defined(STM32H5)
  disableUsbPdPins();
#endif

#if defined(AUDIO_MUTE_GPIO)
  // Mute amplifier immediately to prevent noise during boot
  gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_set(AUDIO_MUTE_GPIO);
#endif

#if defined(AUDIO) && defined(AUDIO_RCC_APB1Periph)
  LL_APB1_GRP1_EnableClock(AUDIO_RCC_APB1Periph);
#endif
#if defined(LL_APB2_GRP1_PERIPH_SYSCFG)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
#endif

  delaysInit();
  timersInit();
  __enable_irq();

  usbInit();

  board_trainer_init();

  // Sets 'hardwareOption.pcbrev' as well
  pwrInit();
  boardInitModulePorts();

#if defined(STATUS_LEDS)
  ledInit();
#if !defined(POWER_LED_BLUE)
  ledBlue();
#else
  ledGreen();
#endif
#endif

  keysInit();
  switchInit();

#if defined(ROTARY_ENCODER_NAVIGATION)
  rotaryEncoderInit();
#endif

  gimbalsDetect();

  if (!adcInit(&_adc_driver))
    TRACE("adcInit failed");

  lcdInit(); // delaysInit() must be called before
#if defined(AUDIO)
  audioInit();
#endif

#if defined(LED_STRIP_GPIO)
  rgbLedInit();
#endif

#if defined(HAPTIC)
  hapticInit();
#endif

#if defined(DEBUG)
  DBGMCU->APB1FZR1 = 0x000019FF;
#endif

#if defined(PWR_BUTTON_PRESS)
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
#endif

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

#if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif

  backlightInit();

#if defined(GUI)
  lcdSetContrast(true);
#endif
}
#endif

void boardOff()
{
#if defined(LED_STRIP_GPIO) && !defined(BOOT)
  rgbLedStop();
  rgbLedClearAll();
#endif

#if defined(STATUS_LEDS) && !defined(BOOT)
  ledOff();
#endif

  BACKLIGHT_DISABLE();

#if defined(PWR_BUTTON_PRESS)
  while (pwrPressed()) {
    WDG_RESET();
  }
#endif

  lcdOff();
  SysTick->CTRL = 0; // turn off systick
  pwrOff();

  // disable interrupts
  __disable_irq();

  while (1) {
    WDG_RESET();
  }

  // this function must not return!
}
