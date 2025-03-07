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

#include "globals.h"
#include "sdcard.h"
#include "debug.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "battery_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"


#include "touch_driver.h"
#include "imu_icm42607p.h"

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

// RGB LED timer
extern const stm32_pulse_timer_t _led_timer;

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

#if defined(SPI_FLASH)
extern "C" void flushFTL();
#endif

static void led_strip_charge_animation(uint8_t ledOn)
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    if (i == ledOn)
      ws2812_set_color(i, 10, 10, 10);
    else
      ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}

static void led_strip_off()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}

// static void setAudioMuteHigh()
// {
//   // Is there mute ?
// }
// static void setAudioMuteLow()
// {
//   // Is there mute ?
// }
// static void setAudioResetHigh()
// {
//   gpio_set(AUDIO_RESET_PIN);
// }
// static void setAudioResetLow()
// {
//   gpio_clear(AUDIO_RESET_PIN);
// }

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
  timersInit();
  bsp_io_init();
  gpio_clear(UCHARGER_EN_GPIO);
}

void boardBLPreJump()
{
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

#if defined(DEBUG)
  serialSetMode(SP_AUX1, UART_MODE_DEBUG);                // indicate AUX1 is used
  serialInit(SP_AUX1, UART_MODE_DEBUG);                   // early AUX1 init
#endif

  boardInitModulePorts();

  pwrInit();
  delaysInit();
  timersInit();

  bsp_io_init();
  gpio_clear(UCHARGER_EN_GPIO);
  gpio_set(LED_BLUE_GPIO);
  gpio_init(HALL_SYNC, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

  // init_trainer();
  auto inittime = flysky_gimbal_init();
  if (inittime)
    TRACE("Serial gimbal detected in %d ms", inittime);
  else
    TRACE("No serial gimbal detected");

  usbInit();

#if !defined(DEBUG_SEGGER_RTT)
  // prime debounce state...
  usbPlugged();

  if (usbPlugged()) {
    delaysInit();
    ws2812_init(&_led_timer, LED_STRIP_LENGTH, WS2812_GRB);
    uint8_t ledOn = 0;
    while (usbPlugged()) {
      if(IS_UCHARGER_ACTIVE()) {
        led_strip_charge_animation(ledOn++);
        if (ledOn == LED_STRIP_LENGTH)
          ledOn = 0;
      }
      else {
        led_strip_off();
      }
      delay_ms(1000);
    }
    while(1) // Wait power to drain
      boardOff();
  }
#endif

  ws2812_init(&_led_timer, LED_STRIP_LENGTH, WS2812_GRB);
  led_strip_off();

  // uint32_t press_start = 0;
  // uint32_t press_end = 0;

  // if (UNEXPECTED_SHUTDOWN()) {
  pwrOn();
  // } else if (isChargerActive()) {
  //   while (true) {
  //     pwrOn();
  //     uint32_t now = get_tmr10ms();
  //     if (pwrPressed()) {
  //       press_end = now;
  //       if (press_start == 0) press_start = now;
  //       if ((now - press_start) > POWER_ON_DELAY) {
  //         break;
  //       }
  //     } else if (!isChargerActive()) {
  //       boardOff();
  //     } else {
  //       uint32_t press_end_touch = press_end;
  //       press_start = 0;
  //       handle_battery_charge(press_end_touch);
  //       delay_ms(10);
  //       press_end = 0;
  //     }
  //   }
  // }

  keysInit();
  switchInit();
  rotaryEncoderInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

#if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif
 
  lcdSetInitalFrameBuffer(lcdFront->getData());

  imu_icm42607p_init();
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

//    RTC->BKP0R = SHUTDOWN_REQUEST;
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


// extern "C" void * memcpy(void* dst, const void*src, size_t count)
// {
//   uint8_t* d = (uint8_t*)dst;
//   const uint8_t* s = (const uint8_t*)src;
//   while(count--)
//     *d++ = *s++;
//   return dst;
// }
