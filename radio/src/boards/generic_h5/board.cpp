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

#if !defined(BOOT)
  #include "edgetx.h"
  #if defined(PXX1)
    #include "pulses/pxx1.h"
  #endif
#endif

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
#endif

#if defined(CSD203_SENSOR)
  #include "csd203_sensor.h"
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

#if defined(FUNCTION_SWITCHES)
#include "storage/storage.h"
#endif

#if defined(SIXPOS_SWITCH_INDEX)
uint8_t lastADCState = 0;
uint8_t sixPosState = 0;
bool dirty = true;
uint16_t getSixPosAnalogValue(uint16_t adcValue)
{
  uint8_t currentADCState = 0;
  if (adcValue > 3800)
    currentADCState = 6;
  else if (adcValue > 3100)
    currentADCState = 5;
  else if (adcValue > 2300)
    currentADCState = 4;
  else if (adcValue > 1500)
    currentADCState = 3;
  else if (adcValue > 1000)
    currentADCState = 2;
  else if (adcValue > 400)
    currentADCState = 1;
  if (lastADCState != currentADCState) {
    lastADCState = currentADCState;
  } else if (lastADCState != 0 && lastADCState - 1 != sixPosState) {
    sixPosState = lastADCState - 1;
    dirty = true;
  }
  if (dirty) {
    for (uint8_t i = 0; i < 6; i++) {
      if (i == sixPosState)
        ws2812_set_color(i, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      else
        ws2812_set_color(i, 0, 0, 0);
    }
    rgbLedColorApply();
  }
  return (4096/5)*(sixPosState);
}
#endif

#if defined(SDLED_PWR_GPIO)
void SDLEDpwrInit()
{
  gpio_init(SDLED_PWR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  SDLED_PWR_ON();
}
#endif

void boardInit()
{
  SystemClock_Config();

#if !defined(USB_PD_SUPPORT) && defined(STM32H5)
  disableUsbPdPins();
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

#if defined(USB_CHARGE_LED) && !defined(DEBUG)
  if (usbPlugged()) {
#if defined(AUDIO_MUTE_GPIO)
     // Charging can make a buzzing noise
     gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
     gpio_set(AUDIO_MUTE_GPIO);
 #endif
    while (usbPlugged()) {
      delay_ms(1000);
    }
    pwrOff();
  }
#endif

#if defined(BLUETOOTH)
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
#endif

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

#if defined(CSD203_SENSOR)
  IICcsd203init();
  initCSD203();
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
