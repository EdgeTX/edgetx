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

#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/rotary_encoder.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"

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

HardwareOptions hardwareOptions;
bool boardBacklightOn = false;

#if defined(VIDEO_SWITCH)
#include "videoswitch_driver.h"

void boardBootloaderInit()
{
  videoSwitchInit();
}
#endif

#if !defined(BOOT)
#include "opentx.h"

void boardInit()
{
  LL_AHB1_GRP1_EnableClock(LCD_RCC_AHB1Periph);
  LL_APB2_GRP1_EnableClock(LCD_RCC_APB2Periph);

#if defined(RADIO_FAMILY_T16)
  void board_set_bor_level();
  board_set_bor_level();
#endif

  pwrInit();

#if defined(FUNCTION_SWITCHES) && !defined(DEBUG_SEGGER_RTT)
#if defined(RADIO_T15)
#define LEDCHARGEON(x)   fsLedOff(x)
#define LEDCHARGEOFF(x)  fsLedOn(x)
#else
#define LEDCHARGEON(x)   fsLedOn(x)
#define LEDCHARGEOFF(x)  fsLedOff(x)
#endif
  // This is needed to prevent radio from starting when usb is plugged to charge
  usbInit();
  ledInit();
  // prime debounce state...
   usbPlugged();
   if (usbPlugged()) {
     delaysInit();
     adcInit(&_adc_driver);
     getADC();
     pwrOn(); // required to get bat adc reads
     INTERNAL_MODULE_OFF();
     EXTERNAL_MODULE_OFF();
     for (uint8_t i=0; i < NUM_FUNCTIONS_SWITCHES; i++)
       LEDCHARGEOFF(i);
     while (usbPlugged()) {
       // Let it charge ...
       getADC(); // Warning: the value read does not include VBAT calibration
                 // Also, MCU is running which create a drop vs off
       if (getBatteryVoltage() >= 660)
         LEDCHARGEON(0);
       if (getBatteryVoltage() >= 700)
         LEDCHARGEON(1);
       if (getBatteryVoltage() >= 740)
         LEDCHARGEON(2);
       if (getBatteryVoltage() >= 780)
         LEDCHARGEON(3);
       if (getBatteryVoltage() >= 810)
         LEDCHARGEON(4);
       if (getBatteryVoltage() >= 820)
         LEDCHARGEON(5);
       delay_ms(1000);
     }
     while(1) // Wait power to drain
       pwrOff();
   }
#endif

  boardInitModulePorts();

#if defined(INTMODULE_HEARTBEAT) &&                                     \
  (defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2))
  pulsesSetModuleInitCb(_intmodule_heartbeat_init);
  pulsesSetModuleDeInitCb(_intmodule_heartbeat_deinit);
  trainerSetChangeCb(_intmodule_heartbeat_trainer_hook);
#endif

  board_trainer_init();
  pwrOn();
  delaysInit();

  __enable_irq();

#if defined(DEBUG) && defined(AUX_SERIAL)
  serialSetMode(SP_AUX1, UART_MODE_DEBUG);                // indicate AUX1 is used
  serialInit(SP_AUX1, UART_MODE_DEBUG);                   // early AUX1 init
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

#if defined(VIDEO_SWITCH)
  videoSwitchInit();
#endif

#if defined(DEBUG)
  // DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP|DBGMCU_TIM1_STOP|DBGMCU_TIM2_STOP|DBGMCU_TIM3_STOP|DBGMCU_TIM4_STOP|DBGMCU_TIM5_STOP|DBGMCU_TIM6_STOP|DBGMCU_TIM7_STOP|DBGMCU_TIM8_STOP|DBGMCU_TIM9_STOP|DBGMCU_TIM10_STOP|DBGMCU_TIM11_STOP|DBGMCU_TIM12_STOP|DBGMCU_TIM13_STOP|DBGMCU_TIM14_STOP, ENABLE);
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
#if !defined(LCD_VERTICAL_INVERT)
  lcdSetInitalFrameBuffer(lcdFront->getData());
#elif defined(RADIO_F16)
  if(hardwareOptions.pcbrev > 0) {
    lcdSetInitalFrameBuffer(lcdFront->getData());
  }
#endif
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
  gpio_init(AUDIO_SHUTDOWN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(AUDIO_SHUTDOWN_GPIO);
#endif

  // Shutdown the Haptic
  hapticDone();

  rtcDisableBackupReg();
  // RTC->BKP0R = SHUTDOWN_REQUEST;

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
  return boardBacklightOn;
}
