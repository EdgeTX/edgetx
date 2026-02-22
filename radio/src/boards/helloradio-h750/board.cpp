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
#include "keys.h"

#include "flysky_gimbal_driver.h"
#include "timers_driver.h"

#include "bitmapbuffer.h"
#include "colors.h"


#include "touch_driver.h"

#include <string.h>

// common ADC driver
extern const etx_hal_adc_driver_t _adc_driver;

// RGB LED timer
extern const stm32_pulse_timer_t _led_timer;

#if defined(SIXPOS_SWITCH_INDEX)

#if defined(FUNCTION_SWITCHES)

extern const stm32_switch_t* boardGetSwitchDef(uint8_t idx);
extern SwitchHwPos stm32_switch_get_position(const stm32_switch_t* sw);

extern uint8_t isSwitch3Pos(uint8_t idx);
struct _adckey_switches_expander {
    uint8_t state;
};
const uint8_t _adckeyidx[8] = {
  0, 1,2,4,8,0x10, 0x20,0
};
static _adckey_switches_expander _adckey_switches;

static SwitchHwPos _get_switch_pos(uint8_t idx)
{
  SwitchHwPos pos = SWITCH_HW_UP;

  const stm32_switch_t* def = boardGetSwitchDef(idx);
  uint8_t state = _adckey_switches.state;

  if (idx<4) {//get hardware switch position
    return stm32_switch_get_position(def);
  }
  //get adc key switch position
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
  } else {
    return _get_switch_pos(idx);
  }
}
#endif

uint8_t lastADCState = 0;
uint8_t sixPosState = 0;
uint8_t sixPosadc = 0;

#define keywaittime 8 //10ms*8
uint8_t uploadPosState = keywaittime;

bool dirty = true;

uint16_t getSixPosAnalogValue(uint16_t adcValue)
{
  uint8_t currentADCState = 0;
  if(uploadPosState){
    uploadPosState--;
    goto __retposadc__;
  }
  else if (adcValue > 3800)
    currentADCState = 1;
  else if (adcValue > 3100)
    currentADCState = 2;
  else if (adcValue > 2300)
    currentADCState = 3;
  else if (adcValue > 1500)
    currentADCState = 4;
  else if (adcValue > 1000)
    currentADCState = 5;
  else if (adcValue > 400)
    currentADCState = 6;
  if (lastADCState != currentADCState) {
    lastADCState = currentADCState;
    uploadPosState=keywaittime;
  }
  else if (lastADCState != sixPosState) {
    sixPosState = lastADCState ;
    dirty = true;
  }
  if (dirty) {
  #if defined(FUNCTION_SWITCHES)  
    _adckey_switches.state=_adckeyidx[sixPosState];
  #endif
    if (sixPosState != 0 && sixPosState - 1 != sixPosadc){
      sixPosadc = sixPosState - 1;
    }
  }
__retposadc__:  
  return (4096/5)*sixPosadc;
}
#endif

static void led_strip_off()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
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

void INTMODULE_ANTSEL_EXT()
{
  gpio_set(INTMODULE_ANTSEL_GPIO);
}

void INTMODULE_ANTSEL_INT()
{
  gpio_clear(INTMODULE_ANTSEL_GPIO);
}

void boardBLEarlyInit()
{
  timersInit();
  usbChargerInit();
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

void USBCharger(uint32_t usbchgstatus)
{
  // USB Charger init code can be added here if needed
  static uint32_t usbchgmode=0;

  if(usbchgstatus>10){
    rgbLedClearAll();
    rgbLedColorApply();
    return;
  }
  switch (++usbchgmode) {
    case 1:
      ws2812_set_color(0, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 2:
      ws2812_set_color(1, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 3:
      ws2812_set_color(2, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 4:
      ws2812_set_color(3, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 5:
      ws2812_set_color(4, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 6:
      ws2812_set_color(5, SIXPOS_LED_RED, SIXPOS_LED_GREEN, SIXPOS_LED_BLUE);
      break;
    case 7:
      usbchgmode=0;
      rgbLedClearAll();
      break;
    default:
      break;
  }
  rgbLedColorApply();
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
  gpio_set(LED_BLUE_GPIO);

  ExtFLASH_InitRuntime();

  // register internal & external FLASH for UF2
  flashRegisterDriver(FLASH_BANK1_BASE, BOOTLOADER_SIZE, &stm32_flash_driver);
  flashRegisterDriver(QSPI_BASE, QSPI_FLASH_SIZE, &extflash_driver);

  usbInit();

  rgbLedInit();
  led_strip_off();

#if !defined(DEBUG_SEGGER_RTT)

  static uint32_t usbchgstatus=0;

  // This is needed to prevent radio from starting when usb is plugged to charge
  if (usbPlugged()) {
    while (usbPlugged() && !pwrPressed()) {//charging loop
      delay_ms(500);
      USBCharger(usbchgstatus);
      if (IS_UCHARGER_ACTIVE())  {
        /* code */
        ledRed();
        usbchgstatus=0;
      }
      else{
        ledGreen();
        usbchgstatus++;
      }
    }
    if (!pwrPressed()) {
      pwrOff();
      // Wait power to drain
      while (true) {
      }
    }
  }
#endif

  rgbLedInit();
  led_strip_off();

  keysInit();
  switchInit();
  rotaryEncoderInit();
  touchPanelInit();
  audioInit();
  adcInit(&_adc_driver);
  hapticInit();

  rtcInit(); // RTC must be initialized before rambackupRestore() is called
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
  while (pwrPressed()) {
    WDG_RESET();
  }
  if (IS_UCHARGER_ACTIVE()||usbPlugged())
  {
    while (pwrPressed()) {
      WDG_RESET();
    }
//    RTC->BKP0R = SOFTRESET_REQUEST;
    NVIC_SystemReset();
  }
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
