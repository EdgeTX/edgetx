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
#include "stm32_ws2812.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"
#include "hal/usb_driver.h"
#include "hal/gpio.h"
#include "hal/rotary_encoder.h"

#include "board.h"
#include "boards/generic_stm32/module_ports.h"
#include "boards/generic_stm32/rgb_leds.h"

#include "globals.h"
#include "sdcard.h"
#include "touch.h"
#include "debug.h"

#if defined(FLYSKY_GIMBAL)
  #include "flysky_gimbal_driver.h"
#endif
#include "timers_driver.h"

#include "battery_driver.h"
#include "touch_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"

#include <string.h>

// Common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

#if defined(SPI_FLASH)
extern "C" void flushFTL();
#endif

#if defined(RADIO_NV14_FAMILY)
  HardwareOptions hardwareOptions;

  static uint8_t boardGetPcbRev()
  {
    gpio_init(INTMODULE_PWR_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
    delay_ms(1); // delay to let the input settle, else it does not work properly

    // detect NV14 vs EL18
    if (gpio_read(INTMODULE_PWR_GPIO)) {
      // pull-up connected: EL18
      return PCBREV_EL18;
    } else {
      // pull-down connected: NV14
      return PCBREV_NV14;
    }
  }
#endif

void delay_self(int count)
{
   for (int i = 50000; i > 0; i--)
   {
       for (; count > 0; count--);
   }
}

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

#if defined(RADIO_NB4P)
void disableVoiceChip()
{
  gpio_init(VOICE_CHIP_EN_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_clear(VOICE_CHIP_EN_GPIO);
}
#endif

void boardBLInit()
{
  // USB charger status pins
  gpio_init(UCHARGER_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);

#if defined(USB_SW_GPIO)
  gpio_init(USB_SW_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
#endif

#if defined(RADIO_NV14_FAMILY)
  // detect NV14 vs EL18
  hardwareOptions.pcbrev = boardGetPcbRev();
#endif
}

#if defined(RADIO_NB4P)
void boardBLPreJump()
{
  LL_ADC_Disable(ADC_MAIN);
}
#endif

static void monitorInit()
{
#if defined(VBUS_MONITOR_GPIO)
  gpio_init(VBUS_MONITOR_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
#endif
}

void boardInit()
{
#if defined(SEMIHOSTING)
  initialise_monitor_handles();
#endif

#if !defined(SIMU)
  // enable interrupts
  __enable_irq();
#endif

#if defined(DEBUG) && defined(AUX_SERIAL)
  serialSetMode(SP_AUX1, UART_MODE_DEBUG);                // indicate AUX1 is used
  serialInit(SP_AUX1, UART_MODE_DEBUG);                   // early AUX1 init
#endif

  TRACE("\nPL18 board started :)");
  delay_ms(10);
  TRACE("RCC->CSR = %08x", RCC->CSR);

  pwrInit();
  boardInitModulePorts();

  board_trainer_init();
  battery_charge_init();
  
  #if defined(FLYSKY_GIMBAL)
    flysky_gimbal_init();
  #endif
  
  timersInit();
  touchPanelInit();
  usbInit();

#if defined(LED_STRIP_GPIO)
  ws2812_init(&_led_timer, LED_STRIP_LENGTH, WS2812_GRB);
  ledStripOff();
#endif

  uint32_t press_start = 0;
  uint32_t press_end = 0;

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
        if (touchPanelEventOccured()) {
          touchPanelRead();
          press_end_touch = timersGetMsTick();
        }
        press_start = 0;
        handle_battery_charge(press_end_touch);
        delay_ms(10);
        press_end = 0;
      }
    }
  }

  keysInit();
  switchInit();
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  rotaryEncoderInit();
#endif
#if defined(RADIO_NB4P)
  disableVoiceChip();
#endif
  audioInit();
  monitorInit();
  adcInit(&_adc_driver);
  hapticInit();


 #if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif
 
  lcdSetInitalFrameBuffer(lcdFront->getData());
    
#if defined(DEBUG)
/*  DBGMCU_APB1PeriphConfig(
      DBGMCU_IWDG_STOP | DBGMCU_TIM1_STOP | DBGMCU_TIM2_STOP |
          DBGMCU_TIM3_STOP | DBGMCU_TIM4_STOP | DBGMCU_TIM5_STOP |
          DBGMCU_TIM6_STOP | DBGMCU_TIM7_STOP | DBGMCU_TIM8_STOP |
          DBGMCU_TIM9_STOP | DBGMCU_TIM10_STOP | DBGMCU_TIM11_STOP |
          DBGMCU_TIM12_STOP | DBGMCU_TIM13_STOP | DBGMCU_TIM14_STOP,
      ENABLE);*/
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
  ledStripOff();
#endif
  if (isChargerActive())
  {
//    RTC->BKP0R = SOFTRESET_REQUEST;
    NVIC_SystemReset();
  }
  else
#endif
  {    
//    RTC->BKP0R = SHUTDOWN_REQUEST;
    pwrOff();
  }

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

#if !defined(RADIO_NV14_FAMILY)
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
#endif
