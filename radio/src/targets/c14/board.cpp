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
#include "gyro.h"
#include "hal/audio_driver.h"
#include "boards/generic_stm32/analog_inputs.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "battery_driver.h"

#if defined(IMU_ICM4207C)
#include "drivers/icm42607C.h"
#endif

#include "bitmapbuffer.h"
#include "colors.h"

// Module control needs g_model — firmware only, not bootloader
#if !defined(BOOT)
#include "myeeprom.h"
#include "pulses/modules_constants.h"
#include "dataconstants.h"
#endif

#include "tp_gt911.h"

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

// RGB LED timer
extern const stm32_pulse_timer_t _led_timer;

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

static const etx_imu_t _imu_candidates[] = {
#if defined(IMU_ICM4207C)
  { &imu_icm42607_driver, IMU_I2C_BUS, ICM426xx_I2C_BASE_ADDR },
  { &imu_icm42607_driver, IMU_I2C_BUS, ICM426xx_I2C_BASE_ADDR + 1 },
#endif
};

static void gyroInit()
{
  gyroStart(imuDetect(_imu_candidates, DIM(_imu_candidates)));
}

void INTERNAL_MODULE_ON()
{
  bsp_output_set(BSP_INT_PWR);
}

void INTERNAL_MODULE_OFF()
{
  bsp_output_clear(BSP_INT_PWR);
}

void INTERNAL_MODULE_BOOTCMD(uint8_t enable)
{
  if (enable)
    bsp_output_clear(BSP_RF_BOOT0);
  else
    bsp_output_set(BSP_RF_BOOT0);
}

void EXTERNAL_MODULE_ON()
{
  bsp_output_set(BSP_EXT_PWR);
}

void EXTERNAL_MODULE_OFF()
{
  bsp_output_clear(BSP_EXT_PWR);
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

  gpio_init(UCHARGER_CHARGE_END_GPIO, GPIO_IN_PD, GPIO_PIN_SPEED_LOW);

  usbInit();

  gimbalsDetect();

  ledInit();
  rgbLedInit();


  rgbLedClearAll();
  keysInit();
  rotaryEncoderInit();
  switchInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

#if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif

  gyroInit();
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
#endif
  pwrOff();

  // We reach here only in forced power situations, such as hw-debugging with external power
  // Enter STM32 stop mode / deep-sleep
  // Code snippet from ST Nucleo PWR_EnterStopMode example
#define PDMode             0x00000000U
#if defined(PWR_CR1_MRUDS) && defined(PWR_CR1_LPUDS) && defined(PWR_CR1_FPDS)
  MODIFY_REG(PWR->CR1, (PWR_CR1_PDDS | PWR_CR1_LPDS | PWR_CR1_FPDS | PWR_CR1_LPUDS | PWR_CR1_MRUDS), PDMode);
#elif defined(PWR_CR_MRLVDS) && defined(PWR_CR_LPLVDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR1, (PWR_CR1_PDDS | PWR_CR1_LPDS | PWR_CR1_FPDS | PWR_CR1_LPLVDS | PWR_CR1_MRLVDS), PDMode);
#endif

/* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  // To avoid HardFault at return address, end in an endless loop
  while (1) {
  }
}
