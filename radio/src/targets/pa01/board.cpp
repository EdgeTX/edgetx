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
#include "keys.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "battery_driver.h"

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

#if defined(SPI_FLASH)
extern "C" void flushFTL();
#endif

constexpr uint8_t ledMapping[] = {4, 6, 0, 2};

void INTERNAL_MODULE_BOOTCMD(uint8_t enable)
{
  if (enable)
    bsp_output_set(BSP_INTMOD_BOOTCMD);
  else
    bsp_output_clear(BSP_INTMOD_BOOTCMD);
}

void INTERNAL_MODULE_ON()
{
  bsp_output_set(BSP_INTMOD_PWR_EN);
}

void INTERNAL_MODULE_OFF()
{
  bsp_output_clear(BSP_INTMOD_PWR_EN);
}

void EXTERNAL_MODULE_ON()
{
  bsp_output_set(BSP_EXTMOD_PWR_EN);
}

void EXTERNAL_MODULE_OFF()
{
  bsp_output_clear(BSP_EXTMOD_PWR_EN);
}

void boardBLEarlyInit()
{
  timersInit();
  bsp_io_init();
  gpio_init(UCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
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

bool pwrPressedDebounced()
{
  static bool debouncedState = 0;
  static bool lastState = 0;

  bool state = pwrPressed();

  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;
  
  return debouncedState;
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

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

  board_trainer_init();
  battery_charge_init();

  adcInit(&_adc_driver);
  getADC();
  
  flysky_gimbal_init();
  usbInit();
  rgbChargeInit(); // RTOS was not running, timer_create will prevent the ADC from reading.

  rotaryEncoderInit();

#if !defined(DEBUG_SEGGER_RTT)

#if defined(SHORT_LONG_PRESS)
  uint32_t short_press = 0;
  uint32_t first_short_press = 0;
  uint32_t long_press = 0;
  uint32_t last_pulse = 0;
  uint32_t now = 0;

  pwrOn();
  hapticInit();  // Ensure the hapatic is initialized during charging

  // Init debounce
  pwrPressedDebounced();
  pwrPressedDebounced();
  
  // Handle charging state if charger is active
  if (isChargerActive()) {
    static uint32_t adc_sample_time = 0; // Hardware ADC sample tick
    bool is_charging = true;
    while (1)
    {
      now = timersGetMsTick();
      while (now - adc_sample_time > 10)
      {
        adc_sample_time = now;
        getADC();
        is_charging = isChargerActive();
      }
      // Exit conditions: button pressed or charger disconnected
      if (!pwrPressedDebounced() && is_charging) {
        updateBatteryState(RGB_STATE_CHARGE);
      } else {
        rgbLedClearAll();
        if (!is_charging) {
            boardOff();
          }
        break;
      }
    }
  }

  // First stage: Detect initial button press and handle press duration
  if (!isChargerActive()) {
    first_short_press = timersGetMsTick();
    while (pwrPressedDebounced()) {
      now = timersGetMsTick();
      short_press = now;
      last_pulse  = now;
      updateBatteryState(RGB_STATE_BREATH);

      // Shutdown after 10s continuous press
      if (now - first_short_press > FIRST_PRESS_TIMEOUT) {
        rgbLedClearAll();
        boardOff();
      }
    }
  }
  
  // Second stage: Detect secondary long-press sequence
  while (1)
  {
    now = timersGetMsTick();

    if (pwrPressedDebounced()) {
      if (long_press == 0) {
        updateBatteryState(RGB_STATE_OFF);
        long_press = now;
      }

      // Successful long-press completion
      if (now - long_press > POWER_ON_DELAY) {
        // updateBatteryState(RGB_STATE_OFF);
        break;
      }

      // Pulsing effect during long-press
      if (now - last_pulse  > POWER_ON_STEP) {
        last_pulse  = now;
        updateBatteryState(RGB_STATE_POWER_ON);
      }
    } else {
      // Breathing effect between press sequences
      updateBatteryState(RGB_STATE_BREATH);

      // Shutdown conditions:
      // 1. Incomplete secondary press (released too soon)
      // 2. Timeout between press sequences
      if ((long_press != 0) || (now - short_press > INTER_PRESS_TIMEOUT)) {
        rgbLedClearAll();
        boardOff();
      }
    }
  }

#else
  if (UNEXPECTED_SHUTDOWN()) {
    pwrOn();
  } else {
    pwrOn();
    hapticInit();
    // Init debounce
    pwrPressedDebounced();
    pwrPressedDebounced();

    uint32_t adc_sample_time = 0; // Hardware ADC sample tick
    uint32_t led_start_time = 0;
    uint32_t led_start_time1 = 0;
    uint32_t start_time = timersGetMsTick();
    uint32_t pwrPressed_start_time = 0;
    uint32_t now = 0;
    uint8_t is_charging = 0;
    uint8_t is_pwrPressed = 0;

    while(1)
    {
      now = timersGetMsTick();
      if (now - adc_sample_time > 10)
      {
        adc_sample_time = now;
        getADC();
        is_charging &= 0xFE;
        is_charging |= isChargerActive();
        is_pwrPressed &= 0xFE;
        is_pwrPressed |= pwrPressedDebounced();

        if( 0x03 == (is_pwrPressed & 0x03) )
        {
          led_start_time = now;
          led_start_time1 = now;
        }

        switch( is_charging & 0x03 )
        {
          case 0x02:
            rgbLedClearAll();
            led_start_time1 = now;
          case 0x00:
          {
            start_time = now;
            led_start_time = 0;
            if(now-led_start_time1<6000)
            {
              updateBatteryState(RGB_STATE_BAT_DIS);
            }
            else
            {
              if( 0==(is_pwrPressed & 0x03))
              {
                rgbLedClearAll();
                boardOff();
              }
            }
          }
          break;
          case 0x01: led_start_time = now;
          case 0x03:
          {
            if( (led_start_time && now-led_start_time<6000) || (!led_start_time && now-start_time<6000) )
            {
              updateBatteryState(RGB_STATE_CHARGE);
            }
            else
            {
              rgbLedClearAll();
            }
          }
          break;
          default: break;
        }

        if( !pwrPressed_start_time && 0x03 == (is_pwrPressed & 0x03) )
        {
          pwrPressed_start_time = now;
        }
        else if( 0x00 == (is_pwrPressed & 0x03) )
        {
          pwrPressed_start_time = 0;
        }
        if(pwrPressed_start_time && now-pwrPressed_start_time>1500 )
        {
          rgbLedClearAll();
          break;      //power on
        }
        is_pwrPressed <<= 1;
        is_charging <<= 1;
      }
    }
  }
#endif  
#endif
  rgbLedInit();
  rgbLedClearAll();
  keysInit();
  switchInit();
  audioInit();

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
  rgbLedClearAll();
  if (IS_UCHARGER_ACTIVE())
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

  uint8_t state = IS_UCHARGER_ACTIVE();

  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;
  
  return debouncedState;
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
