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
#include "vs1053b.h"

#include "flash_driver.h"
#include "extflash_driver.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/rgb_leds.h"
#include "bsp_io.h"

#include "hal/adc_driver.h"
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

void INTERNAL_MODULE_ON()
{
  bsp_output_clear(BSP_INT_PWR);
}

void INTERNAL_MODULE_OFF()
{
  bsp_output_set(BSP_INT_PWR);
}

void EXTERNAL_MODULE_ON()
{
  bsp_output_clear(BSP_EXT_PWR);
}

void EXTERNAL_MODULE_OFF()
{
  bsp_output_set(BSP_EXT_PWR);
}

static stm32_spi_t audioSpi =
{
    .SPIx = AUDIO_SPI,
    .SCK = AUDIO_SPI_SCK_GPIO,
    .MISO = AUDIO_SPI_MISO_GPIO,
    .MOSI = AUDIO_SPI_MOSI_GPIO,
    .CS = AUDIO_CS_GPIO,
    .DMA = AUDIO_SPI_DMA,
    .txDMA_PeriphRequest = AUDIO_SPI_DMA_REQ,
    .txDMA_Stream = AUDIO_SPI_DMA_STREAM,
};

static void audio_set_rst_pin(bool set)
{
  if (set) {
    bsp_output_set(BSP_AUDIO_RST);
  } else {
    bsp_output_clear(BSP_AUDIO_RST);
  }
}

static void audio_set_mute_pin(bool set)
{
#if defined(INVERTED_MUTE_PIN)
  set = !set;  
#endif
  if (set) {
    bsp_output_set(BSP_PA_NMUTE);
  } else {
    bsp_output_clear(BSP_PA_NMUTE);
  }
}

static vs1053b_t audioConfig =
{
  .spi = &audioSpi,
  .XDCS = AUDIO_XDCS_GPIO,
  .DREQ = AUDIO_DREQ_GPIO,
  .set_rst_pin = audio_set_rst_pin,
  .set_mute_pin = audio_set_mute_pin,
  .mute_delay_ms = AUDIO_MUTE_DELAY,
  .unmute_delay_ms = AUDIO_UNMUTE_DELAY,
};

void audioInit(){
  // assume BSP IO is already init
  vs1053b_init(&audioConfig);
}

void boardBLEarlyInit()
{
  timersInit();
  bsp_io_init();
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
  bsp_output_set(BSP_PWR_LED);

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

  board_trainer_init();
  battery_charge_init();

  // init_trainer();
  flysky_gimbal_init();

  usbInit();

#if defined(LED_STRIP_GPIO)
  rgbLedInit();
#endif

  rotaryEncoderInit();

#if !defined(DEBUG_SEGGER_RTT)

  uint32_t press_start = 0;
  uint32_t press_end = 0;
  rotenc_t lastEncoderValue = 0;

  if (UNEXPECTED_SHUTDOWN()) {
    pwrOn();
  } else if (isChargerActive()) {
    while (true) {
      pwrOn();
      uint32_t now = timersGetMsTick();
      if (pwrPressed()) {
        press_end = now;
        if (press_start == 0) press_start = now;
        if ((now - press_start) > POWER_ON_DELAY) {
          break;
        }
      } else if (!isChargerActive()) {
        boardOff();
      } else {
        uint32_t press_end_touch = press_end;
        extern void rotaryEncoderCheck();
        rotaryEncoderCheck();
        rotenc_t value = rotaryEncoderGetValue();
        if (value != lastEncoderValue) {
          lastEncoderValue = value;     
          press_end_touch = timersGetMsTick();
        }
        press_start = 0;
        handle_battery_charge(press_end_touch);
        delay_ms(10);
        press_end = 0;
      }
    }
    battery_charge_end();
  }

#endif

  rgbLedClearAll();
  keysInit();
  switchInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

#if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif
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

#if !defined(BOOT)
#if defined(LED_STRIP_GPIO)
  rgbLedClearAll();
#endif
  if (isChargerActive())
  {
//    RTC->BKP0R = SOFTRESET_REQUEST;
    NVIC_SystemReset();
  }
  else
#endif

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
/*
int usbPlugged()
{
  static uint8_t debouncedState = 0;
  static uint8_t lastState = 0;

  uint8_t state = gpio_read(UCHARGER_GPIO);

  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;
  
  return debouncedState||1;
}
*/

// extern "C" void * memcpy(void* dst, const void*src, size_t count)
// {
//   uint8_t* d = (uint8_t*)dst;
//   const uint8_t* s = (const uint8_t*)src;
//   while(count--)
//     *d++ = *s++;
//   return dst;
// }
