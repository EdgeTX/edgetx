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
#include "stm32_hal.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/rotary_encoder.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/intmodule_heartbeat.h"
#include "boards/generic_stm32/analog_inputs.h"

#include "timers_driver.h"
#include "dataconstants.h"
#include "opentx_types.h"
#include "globals.h"
#include "sdcard.h"
#include "debug.h"

#include <string.h>

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif

enum PowerReason {
  SHUTDOWN_REQUEST = 0xDEADBEEF,
  SOFTRESET_REQUEST = 0xCAFEDEAD,
};

constexpr uint32_t POWER_REASON_SIGNATURE = 0x0178746F;

bool UNEXPECTED_SHUTDOWN()
{
#if defined(SIMU) || defined(NO_UNEXPECTED_SHUTDOWN)
  return false;
#else
  if (WAS_RESET_BY_WATCHDOG())
    return true;
  else if (WAS_RESET_BY_SOFTWARE())
    return RTC->BKP0R != SOFTRESET_REQUEST;
  else
    return RTC->BKP1R == POWER_REASON_SIGNATURE && RTC->BKP0R != SHUTDOWN_REQUEST;
#endif
}

void SET_POWER_REASON(uint32_t value)
{
  RTC->BKP0R = value;
  RTC->BKP1R = POWER_REASON_SIGNATURE;
}

HardwareOptions hardwareOptions;
bool boardBacklightOn = false;

void watchdogInit(unsigned int duration)
{
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->PR = 3;           // Divide by 32 => 1kHz clock
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->RLR = duration;
  IWDG->KR = 0xAAAA;      // reload
  IWDG->KR = 0xCCCC;      // start
}

#if !defined(BOOT)
#include "opentx.h"

void boardInit()
{
  LL_AHB1_GRP1_EnableClock(LCD_RCC_AHB1Periph);
  LL_APB2_GRP1_EnableClock(LCD_RCC_APB2Periph);

#if defined(RADIO_FAMILY_T16)
  FLASH_OBProgramInitTypeDef OBInit;
  HAL_FLASHEx_OBGetConfig(&OBInit);
  if (OBInit.BORLevel != OB_BOR_LEVEL3) {
    OBInit.OptionType = OPTIONBYTE_BOR;
    OBInit.BORLevel = OB_BOR_LEVEL3;
    HAL_FLASH_OB_Unlock();
    HAL_FLASHEx_OBProgram(&OBInit);
    HAL_FLASH_OB_Launch();
    HAL_FLASH_OB_Lock();
  }
#endif

  pwrInit();
  boardInitModulePorts();

#if defined(INTMODULE_HEARTBEAT) &&                                     \
  (defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2))
  pulsesSetModuleInitCb(_intmodule_heartbeat_init);
  pulsesSetModuleDeInitCb(_intmodule_heartbeat_deinit);
#endif

  init_trainer();
  pwrOn();
  delaysInit();

  __enable_irq();

#if defined(DEBUG)
  serialInit(SP_AUX1, UART_MODE_DEBUG);
#endif

  TRACE("\nHorus board started :)");
  TRACE("RCC->CSR = %08x", RCC->CSR);

  audioInit();

  keysInit();
  switchInit();
  rotaryEncoderInit();

#if defined(PWM_STICKS)
  sticksPwmDetect();
#endif
  
#if defined(FLYSKY_GIMBAL)
  flysky_gimbal_init();
#endif

  if (!adcInit(&_adc_driver))
    TRACE("adcInit failed");

  timersInit();

  usbInit();
  hapticInit();

#if defined(BLUETOOTH)
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
#endif


#if defined(DEBUG)
  // Freeze timers & watchdog when core is halted
  DBGMCU->APB1FZ = 0x00E009FF;
  DBGMCU->APB2FZ = 0x00070003;
#endif

  ledInit();

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

#if defined(RTCLOCK)
  ledRed();
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif

  ledBlue();
}
#endif

extern void rtcDisableBackupReg();

void boardOff()
{
  ledOff();
  backlightEnable(0);

  while (pwrPressed()) {
    WDG_RESET();
  }

  SysTick->CTRL = 0; // turn off systick

#if defined(PCBX12S)
  // Shutdown the Audio amp
  LL_GPIO_ResetOutputPin(AUDIO_SHUTDOWN_GPIO, AUDIO_SHUTDOWN_GPIO_PIN);
#endif

  // Shutdown the Haptic
  hapticDone();

  rtcDisableBackupReg();
  RTC->BKP0R = SHUTDOWN_REQUEST;

  pwrOff();

  // We reach here only in forced power situations, such as hw-debugging with external power
  // Enter STM32 stop mode / deep-sleep
  // Code snippet from ST Nucleo PWR_EnterStopMode example
#define PDMode             0x00000000U
#if defined(PWR_CR_MRUDS) && defined(PWR_CR_LPUDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_FPDS | PWR_CR_LPUDS | PWR_CR_MRUDS), PDMode);
#elif defined(PWR_CR_MRLVDS) && defined(PWR_CR_LPLVDS) && defined(PWR_CR_FPDS)
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS | PWR_CR_LPDS | PWR_CR_FPDS | PWR_CR_LPLVDS | PWR_CR_MRLVDS), PDMode);
#else
  MODIFY_REG(PWR->CR, (PWR_CR_PDDS| PWR_CR_LPDS), PDMode);
#endif /* PWR_CR_MRUDS && PWR_CR_LPUDS && PWR_CR_FPDS */

/* Set SLEEPDEEP bit of Cortex System Control Register */
  SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

  // To avoid HardFault at return address, end in an endless loop
  while (1) {

  }
}

bool isBacklightEnabled()
{
  if (globalData.unexpectedShutdown) return true;
  return boardBacklightOn;
}
