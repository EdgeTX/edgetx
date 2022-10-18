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

#include "board.h"
#include "boards/generic_stm32/module_ports.h"

#include "hal/adc_driver.h"
#include "hal/trainer_driver.h"
#include "hal/switch_driver.h"

#include "globals.h"
#include "sdcard.h"
#include "touch.h"
#include "debug.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "lcd_driver.h"
#include "lcd_driver.h"
#include "battery_driver.h"
#include "touch_driver.h"
#include "watchdog_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"

#include <string.h>

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

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

void watchdogInit(unsigned int duration)
{
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->PR = 3;           // Divide by 32 => 1kHz clock
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->RLR = duration;   // 1.5 seconds nominal
  IWDG->KR = 0xAAAA;      // reload
  IWDG->KR = 0xCCCC;      // start
}

#if defined(SEMIHOSTING)
extern "C" void initialise_monitor_handles();
#endif

void delay_self(int count)
{
   for (int i = 50000; i > 0; i--)
   {
       for (; count > 0; count--);
   }
}
#define RCC_AHB1PeriphMinimum (PWR_RCC_AHB1Periph |\
                               LCD_RCC_AHB1Periph |\
                               BACKLIGHT_RCC_AHB1Periph |\
                               SDRAM_RCC_AHB1Periph \
                              )
#define RCC_AHB1PeriphOther   (SD_RCC_AHB1Periph |\
                               AUDIO_RCC_AHB1Periph |\
                               MONITOR_RCC_AHB1Periph |\
                               KEYS_RCC_AHB1Periph |\
                               ADC_RCC_AHB1Periph |\
                               AUX_SERIAL_RCC_AHB1Periph |\
                               TELEMETRY_RCC_AHB1Periph |\
                               TRAINER_RCC_AHB1Periph |\
                               AUDIO_RCC_AHB1Periph |\
                               HAPTIC_RCC_AHB1Periph |\
                               INTMODULE_RCC_AHB1Periph |\
                               FLYSKY_HALL_RCC_AHB1Periph |\
                               EXTMODULE_RCC_AHB1Periph\
                              )
#define RCC_AHB3PeriphMinimum (SDRAM_RCC_AHB3Periph)

#define RCC_APB1PeriphMinimum (INTERRUPT_xMS_RCC_APB1Periph |\
                               TIMER_2MHz_RCC_APB1Periph |\
                               BACKLIGHT_RCC_APB1Periph \
                              )

#define RCC_APB1PeriphOther   (TELEMETRY_RCC_APB1Periph |\
                               FLYSKY_HALL_RCC_APB1Periph |\
                               MIXER_SCHEDULER_TIMER_RCC_APB1Periph \
                              )
#define RCC_APB2PeriphMinimum (LCD_RCC_APB2Periph)

#define RCC_APB2PeriphOther   (ADC_RCC_APB2Periph |\
                               HAPTIC_RCC_APB2Periph |\
                               AUDIO_RCC_APB2Periph \
                              )

static uint8_t boardGetPcbRev()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(INTMODULE_RCC_AHB1Periph, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = INTMODULE_PWR_GPIO_PIN;
  GPIO_Init(INTMODULE_PWR_GPIO, &GPIO_InitStructure);
  delay_ms(1); // delay to let the input settle, else it does not work properly

  // detect NV14 vs EL18
  if (GPIO_ReadInputDataBit(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN) == Bit_SET) {
    // pull-up connected: EL18
    return PCBREV_EL18;
  } else {
    // pull-down connected: NV14
    return PCBREV_NV14;
  }
}

void boardBootloaderInit()
{
#if defined(USB_SW_PIN)
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = USB_SW_PIN;
  GPIO_Init(USB_SW_GPOIO, &GPIO_InitStructure);
  RCC_AHB1PeriphClockCmd(USB_SW_AHB1Periph_GPIO, ENABLE);
#endif

  // detect NV14 vs EL18
  hardwareOptions.pcbrev = boardGetPcbRev();
}

static void monitorInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

  GPIO_InitStructure.GPIO_Pin = VBUS_MONITOR_PIN;
  GPIO_Init(GPIOJ, &GPIO_InitStructure);
}

void boardInit()
{
#if defined(SEMIHOSTING)
  initialise_monitor_handles();
#endif

#if !defined(SIMU)
  RCC_AHB1PeriphClockCmd(RCC_AHB1PeriphMinimum | RCC_AHB1PeriphOther, ENABLE);
  RCC_AHB3PeriphClockCmd(RCC_AHB3PeriphMinimum, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1PeriphMinimum | RCC_APB1PeriphOther, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2PeriphMinimum | RCC_APB2PeriphOther, ENABLE);

  // enable interrupts
  __enable_irq();
#endif

  // detect NV14 vs EL18
  hardwareOptions.pcbrev = boardGetPcbRev();

#if defined(DEBUG)
  serialInit(SP_AUX1, UART_MODE_DEBUG);
#endif

  TRACE("\n%s board started :)",
        hardwareOptions.pcbrev == PCBREV_NV14 ?
        "NV14" : "EL18");

  delay_ms(10);
  TRACE("RCC->CSR = %08x", RCC->CSR);

  pwrInit();
  boardInitModulePorts();

  init_trainer();
  battery_charge_init();
  flysky_gimbal_init();
  init2MhzTimer();
  init1msTimer();
  TouchInit();
  usbInit();

  uint32_t press_start = 0;
  uint32_t press_end = 0;

  if (UNEXPECTED_SHUTDOWN()) {
    pwrOn();
  } else {
    // prime debounce state...
    uint8_t usb_state = usbPlugged();
    usb_state |= usbPlugged();
    while (usb_state) {
      pwrOn();
      uint32_t now = get_tmr10ms();
      if (pwrPressed()) {
        press_end = now;
        if (press_start == 0) press_start = now;
        if ((now - press_start) > POWER_ON_DELAY) {
          break;
        }
      } else if (!usbPlugged()){
          delay_ms(20);
          if(!usbPlugged()){
            boardOff();
          }
      } else {
        uint32_t press_end_touch = press_end;
        if (touchPanelEventOccured()) {
          touchPanelRead();
          press_end_touch = get_tmr10ms();
        }
        press_start = 0;
        handle_battery_charge(press_end_touch);
        delay_ms(20);
        press_end = 0;
      }
    }
  }

  keysInit();
  switchInit();
  audioInit();
  // we need to initialize g_FATFS_Obj here, because it is in .ram section (because of DMA access)
  // and this section is un-initialized
  memset(&g_FATFS_Obj, 0, sizeof(g_FATFS_Obj));
  monitorInit();
  adcInit(&_adc_driver);
  hapticInit();


 #if defined(RTCLOCK)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif


#if defined(DEBUG)
  DBGMCU_APB1PeriphConfig(
      DBGMCU_IWDG_STOP | DBGMCU_TIM1_STOP | DBGMCU_TIM2_STOP |
          DBGMCU_TIM3_STOP | DBGMCU_TIM4_STOP | DBGMCU_TIM5_STOP |
          DBGMCU_TIM6_STOP | DBGMCU_TIM7_STOP | DBGMCU_TIM8_STOP |
          DBGMCU_TIM9_STOP | DBGMCU_TIM10_STOP | DBGMCU_TIM11_STOP |
          DBGMCU_TIM12_STOP | DBGMCU_TIM13_STOP | DBGMCU_TIM14_STOP,
      ENABLE);
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

  // Shutdown the Audio amp
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = AUDIO_SHUTDOWN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(AUDIO_SHUTDOWN_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(AUDIO_SHUTDOWN_GPIO, AUDIO_SHUTDOWN_GPIO_PIN);

  // Shutdown the Haptic
  hapticDone();

  rtcDisableBackupReg();

  if (usbPlugged())
  {
    delay_ms(100);  // Add a delay to wait for lcdOff
    RTC->BKP0R = SOFTRESET_REQUEST;
    NVIC_SystemReset();
  }
  else
  {
    RTC->BKP0R = SHUTDOWN_REQUEST;
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
