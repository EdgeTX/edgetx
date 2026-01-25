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
 
#include "stm32_adc.h"
#include "stm32_gpio.h"
#include "stm32_i2c_driver.h"
#include "stm32_hal.h"
#include "stm32_ws2812.h"
#include "stm32_spi.h"

#include "flash_driver.h"
#include "extflash_driver.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "bsp_io.h"

#include "hal/adc_driver.h"
#include "hal/flash_driver.h"
#include "hal/trainer_driver.h"
#include "hal/rotary_encoder.h"
#include "hal/switch_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rgbleds.h"

#include "globals.h"
#include "sdcard.h"
#include "debug.h"
#include "keys.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "touch_driver.h"

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

// RGB LED timer
extern const stm32_pulse_timer_t _led_timer;

static void led_strip_off()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}

void INTERNAL_MODULE_ON()
{
  gpio_set(INTMODULE_PWR_GPIO);
}

void INTERNAL_MODULE_OFF()
{
  gpio_clear(INTMODULE_PWR_GPIO);
}

void EXTERNAL_MODULE_ON()
{
  gpio_set(EXTMODULE_PWR_GPIO);
}

void EXTERNAL_MODULE_OFF()
{
  gpio_clear(EXTMODULE_PWR_GPIO);
}

void boardBLEarlyInit()
{
  // TAS2505 requires reset pin to be low on power on
  gpio_init(AUDIO_RESET_PIN, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(AUDIO_RESET_PIN);

  timersInit();
  delaysInit();
  bsp_io_init();
  usbChargerInit();
}

void boardBLPreJump()
{
  // TAS2505 requires reset pin to be high only after power on
  // https://www.ti.com/lit/ug/slau472c/slau472c.pd fig 4.2
  gpio_set(AUDIO_RESET_PIN);
  ExtFLASH_Init();
  SDRAM_Init();

  // Stop 1ms timer
  MS_TIMER->CR1 &= ~TIM_CR1_CEN;
}

void boardBLInit()
{
  ExtFLASH_Init();
  SDRAM_Init();

  // register external FLASH for DFU
  usbRegisterDFUMedia((void*)extflash_dfu_media);

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, 8 * 1024 * 1024, &extflash_driver);
}

void boardInit()
{
  // enable interrupts
  __enable_irq();

#if defined(KCX_BTAUDIO)
  btAudioInit();
#endif

  ledInit();
  boardInitModulePorts();

  pwrInit();
  delaysInit();
  timersInit();

  usbChargerInit();
  gpio_set(LED_BLUE_GPIO);

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

  // init_trainer();

#if defined(FLYSKY_GIMBAL)
  auto inittime = flysky_gimbal_init();
  if (inittime)
    TRACE("Serial gimbal detected in %d ms", inittime);
  else
    TRACE("No serial gimbal detected");
#endif

  usbInit();

  rgbLedInit();
  led_strip_off();

  keysInit();
  switchInit();
  rotaryEncoderInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

  // RTC must be initialized before rambackupRestore() is called
  rtcInit();
}

extern void rtcDisableBackupReg();

void boardOff()
{
  lcdOff();

  while (pwrPressed()) {
    WDG_RESET();
  }

  SysTick->CTRL = 0; // turn off systick

  // Shutdown the Haptic
  hapticDone();

  rtcDisableBackupReg();

  pwrOff();

  // We reach here only in forced power situations, such as hw-debugging with external power  
  // Enter STM32 stop mode / deep-sleep
  // Code snippet from ST Nucleo PWR_EnterStopMode example
#define PDMode             0x00000000U
#if defined(PWR_CR1_MRUDS) && defined(PWR_CR1_LPUDS) && defined(PWR_CR1_FPDS)
  MODIFY_REG(PWR->CR1, (PWR_CR1_PDDS | PWR_CR1_LPDS | PWR_CR1_FPDS | PWR_CR1_LPUDS | PWR_CR1_MRUDS), PDMode);
#elif defined(PWR_CR_MRLVDS) && defined(PWR_CR_LPLVDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR1, (PWR_CR1_PDDS | PWR_CR1_LPDS | PWR_CR1_FPDS | PWR_CR1_LPLVDS | PWR_CR1_MRLVDS), PDMode);
#else
//  MODIFY_REG(PWR->CR1, (PWR_CR1_P_PDDS| PWR_CR1_LPDS), PDMode);
#endif /* PWR_CR_MRUDS && PWR_CR_LPUDS && PWR_CR_FPDS */

/* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
  
  // To avoid HardFault at return address, end in an endless loop
  while (1) {

  }
}
