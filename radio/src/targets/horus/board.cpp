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
#include "stm32_ws2812.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/rotary_encoder.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rgbleds.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/intmodule_heartbeat.h"
#include "boards/generic_stm32/analog_inputs.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "timers_driver.h"
#include "dataconstants.h"
#include "edgetx_types.h"
#include "globals.h"
#include "sdcard.h"
#include "debug.h"

#include <string.h>

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif

#if defined(CSD203_SENSOR)
  #include "csd203_sensor.h"
#endif

#if defined(LED_STRIP_GPIO)
// Common LED driver
extern const stm32_pulse_timer_t _led_timer;

void ledStripOff()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}
#endif

HardwareOptions hardwareOptions;
bool boardBacklightOn = false;

#if defined(VIDEO_SWITCH)
#include "videoswitch_driver.h"

void boardBLEarlyInit()
{
  videoSwitchInit();
}
#endif

#if !defined(BOOT)
#include "edgetx.h"

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
    ws2812_update(&_led_timer);
  }
  return (4096/5)*(sixPosState);
}
#endif

void boardInit()
{
#if defined(RADIO_FAMILY_T16)
  void board_set_bor_level();
  board_set_bor_level();
#endif

#if defined(MANUFACTURER_JUMPER) && defined(FUNCTION_SWITCHES) && !defined(DEBUG)
  // This is needed to prevent radio from starting when usb is plugged to charge
  usbInit();
  // prime debounce state...
   usbPlugged();
   if (usbPlugged()) {
     delaysInit();
 #if defined(AUDIO_MUTE_GPIO)
     // Charging can make a buzzing noise
     gpio_init(AUDIO_MUTE_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
     gpio_set(AUDIO_MUTE_GPIO);
 #endif
     while (usbPlugged()) {
       delay_ms(1000);
     }
     while(1) // Wait power to drain
       pwrOff();
   }
#endif

  pwrInit();

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

  TRACE("\nHorus board started :)");
  TRACE("RCC->CSR = %08x", RCC->CSR);

  audioInit();

  keysInit();
  switchInit();
  rotaryEncoderInit();

#if defined(HARDWARE_TOUCH)
  touchPanelInit();
#endif

#if defined(PWM_STICKS)
  sticksPwmDetect();
#endif
  
#if defined(FLYSKY_GIMBAL)
  flysky_gimbal_init();
#endif

  if (!adcInit(&_adc_driver))
    TRACE("adcInit failed");

  timersInit();

#if defined(HARDWARE_TOUCH) && !defined(SIMU)
  touchPanelInit();
#endif

#if defined(CSD203_SENSOR)
  initCSD203();
#endif

  usbInit();
  hapticInit();

#if defined(LED_STRIP_GPIO)
  ws2812_init(&_led_timer, LED_STRIP_LENGTH, WS2812_GRB);
  ledStripOff();
#endif

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
