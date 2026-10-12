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
#include "stm32_spi.h"

#include "flash_driver.h"
#include "extflash_driver.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "stm32_switch_driver.h"
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
#include "gyro.h"
#if !defined(BOOT)
#include "aux3_chat.h"
#endif

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"


#include "touch_driver.h"

#if defined(IMU_ICM4207C)
#include "drivers/icm42607C.h"
#endif
#if defined(IMU_SC7U22)
#include "drivers/sc7u22.h"
#endif

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

static const etx_imu_t _imu_candidates[] = {
#if defined(IMU_ICM4207C)
  { &imu_icm42607_driver, IMU_I2C_BUS, ICM426xx_I2C_BASE_ADDR },
  { &imu_icm42607_driver, IMU_I2C_BUS, ICM426xx_I2C_BASE_ADDR + 1 },
#endif
#if defined(IMU_SC7U22)
  { &imu_sc7u22_driver, IMU_I2C_BUS, SC7U22_I2C_BASE_ADDR },
  { &imu_sc7u22_driver, IMU_I2C_BUS, SC7U22_I2C_BASE_ADDR + 1 },
#endif
};

// The IMU is mounted upside down and rotated, so its X/Y axes are swapped
// (and both reversed, see IMU_INVERT_X/Y). Swap them here, before the generic
// inversion/calibration, so that TltX/TltY and their settings line up
static imu_read_fn _imuRead = nullptr;

static int imuReadSwapXY(etx_imu_data_t* data)
{
  int ret = _imuRead(data);
  if (ret >= 0) {
    float t = data->accel_x;
    data->accel_x = data->accel_y;
    data->accel_y = t;
    t = data->gyro_x;
    data->gyro_x = data->gyro_y;
    data->gyro_y = t;
  }
  return ret;
}

static void gyroInit()
{
  _imuRead = imuDetect(_imu_candidates, DIM(_imu_candidates));
  gyroStart(_imuRead ? imuReadSwapXY : nullptr);
}

#if defined(SIXPOS_SWITCH_INDEX)

#if defined(FUNCTION_SWITCHES)

extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);
extern uint8_t isSwitch3Pos(uint8_t idx);
struct _adckey_switches_expander {
    uint8_t state;
};
// Bitmask of all 6 function switch bits
#define FS_ALL_BITS 0x3F

// For each 6POS position (0=none, 1-6=SW1-SW6): bit of the active switch,
// inverted so active switch pin reads low (DOWN) like real PCA95xx hardware.
// position 0 (none): all bits set = all UP
// position N: all bits set except SW_N's bit = SW_N is DOWN
static uint8_t _pos_to_state(uint8_t pos)
{
  if (pos == 0) return FS_ALL_BITS;
  uint8_t active_bit = (1 << (pos - 1));
  return FS_ALL_BITS & ~active_bit;
}
static _adckey_switches_expander _adckey_switches = { FS_ALL_BITS };

void sixPosUpdateFromAdc()
{
  uint16_t* values = getAnalogValues();
  uint16_t adcValue = values[SIXPOS_SWITCH_INDEX];

  uint8_t current = 0;
  if (adcValue > 3800) current = 1;
  else if (adcValue > 3100) current = 2;
  else if (adcValue > 2300) current = 3;
  else if (adcValue > 1500) current = 4;
  else if (adcValue > 1000) current = 5;
  else if (adcValue > 400)  current = 6;

  static uint8_t lastSeen = 0;
  static uint8_t debounce = 0;
  static uint8_t sixPosState = 0;

  if (current != lastSeen) {
    lastSeen = current;
    debounce = 2;  // require this many more stable samples before committing
  } else if (debounce > 0) {
    if (--debounce == 0) {
      // Switch state always reflects the true debounced position
      _adckey_switches.state = _pos_to_state(current);
      if (current != 0) {
        // Indicator value stays sticky on the last active position
        sixPosState = current;
      }
    }
  }

  values[SIXPOS_SWITCH_INDEX] = sixPosState ? (4096 / 5) * (7 - sixPosState) : 0;
}

static SwitchHwPos _get_switch_pos(uint8_t idx)
{
  SwitchHwPos pos = SWITCH_HW_UP;

  const stm32_switch_t* def = boardGetSwitchDef(idx);
  uint8_t state = _adckey_switches.state;

  if (def->isCustomSwitch) {
    if ((state & def->Pin_high) == 0) {
      return SWITCH_HW_DOWN;
    } else {
      return SWITCH_HW_UP;
    }
  }
  else if (!def->Pin_low) {
    // 2POS switch
    if ((state & def->Pin_high) != 0) {
      pos = SWITCH_HW_DOWN;
    }
  } else {
    bool hi = state & def->Pin_high;
    bool lo = state & def->Pin_low;

    if(!isSwitch3Pos(idx))
    {
      // Switch not declared as 3POS installed in a 3POS HW
      if (!(hi && lo)) {
        pos = SWITCH_HW_DOWN;
      }
    } else if (hi && lo) {
      pos = SWITCH_HW_MID;
    } else if (!hi && lo) {
      pos = SWITCH_HW_DOWN;
    }
  }
  return pos;
}

static SwitchHwPos _get_fs_switch_pos(uint8_t idx)
{
  const stm32_switch_t* def = boardGetSwitchDef(idx);
  uint8_t state = _adckey_switches.state;
  if ((state & def->Pin_high) == 0) {
    return SWITCH_HW_DOWN;
  } else {
    return SWITCH_HW_UP;
  }
}

bool boardIsCustomSwitch(uint8_t idx);

SwitchHwPos boardSwitchGetPosition(uint8_t idx)
{
  if (boardIsCustomSwitch(idx)) {
    return _get_fs_switch_pos(idx);
  } else if (boardGetSwitchDef(idx)->GPIOx_high != nullptr) {
    return stm32_switch_get_position(boardGetSwitchDef(idx));
  } else {
    return _get_switch_pos(idx);
  }
}

#endif

#endif

static void led_strip_off()
{
  rgbLedClearAll();
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

void Ai_SOUNDMODULE_OFF()
{
  gpio_set(AUDIO_SELECT_GPIO);
}

void Ai_SOUNDMODULE_ON()
{
  gpio_clear(AUDIO_SELECT_GPIO);
}

void boardBLEarlyInit()
{
  timersInit();
  delaysInit();
  //bsp_io_init();
  usbChargerInit();
  hr_exesenserInit();
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

  ledInit();
  boardInitModulePorts();

  pwrInit();
  delaysInit();
  timersInit();

  usbChargerInit();
  hr_exesenserInit();
  gpio_set(LED_BLUE_GPIO);

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

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
  board_trainer_init();
  rotaryEncoderInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

  // RTC must be initialized before rambackupRestore() is called
  rtcInit();

#if !defined(BOOT)
  // Dedicated UART5 / AUX3 text channel for the V15 smart chat module.
  // Kept separate from generic AUX1/AUX2 serial configuration so other targets are unaffected.
  v15ChatUartInit();
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
  
#if !defined(BOOT)
  v15ChatUartDeinit();
#endif

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
