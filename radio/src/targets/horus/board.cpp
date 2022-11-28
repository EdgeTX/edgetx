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
#include "hal/adc_driver.h"
#include "hal/serial_driver.h"
#include "hal/serial_port.h"
#include "hal/trainer_driver.h"

#include "board.h"
#include "timers_driver.h"
#include "dataconstants.h"
#include "opentx_types.h"
#include "globals.h"
#include "sdcard.h"
#include "debug.h"

#include <string.h>

#if defined(AUX_SERIAL) || defined(AUX2_SERIAL)
#include "aux_serial_driver.h"
#endif

#if !defined(PCBX12S)
  #include "stm32_hal_adc.h"
  #define ADC_DRIVER stm32_hal_adc_driver
#else
  #include "x12s_adc_driver.h"
  #define ADC_DRIVER x12s_adc_driver
#endif

extern void flysky_hall_stick_check_init(void);
extern void flysky_hall_stick_init(void);
extern void flysky_hall_stick_loop( void );

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

#if HAS_SPORT_UPDATE_CONNECTOR() && !defined(BOOT)

// g_eeGeneral
#include "opentx.h"

void sportUpdateInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = SPORT_UPDATE_PWR_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(SPORT_UPDATE_PWR_GPIO, &GPIO_InitStructure);
}

void sportUpdatePowerOn()
{
  GPIO_SetBits(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
}

void sportUpdatePowerOff()
{
  GPIO_ResetBits(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
}

void sportUpdatePowerInit()
{
  if (g_eeGeneral.sportUpdatePower == 1)
    sportUpdatePowerOn();
  else
    sportUpdatePowerOff();
}
#endif

void boardInit()
{
  RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph |
                         PCBREV_RCC_AHB1Periph |
                         LED_RCC_AHB1Periph |
                         LCD_RCC_AHB1Periph |
                         BACKLIGHT_RCC_AHB1Periph |
                         KEYS_BACKLIGHT_RCC_AHB1Periph |
                         SD_RCC_AHB1Periph |
                         AUDIO_RCC_AHB1Periph |
                         KEYS_RCC_AHB1Periph |
                         ADC_RCC_AHB1Periph |
#if defined(RADIO_FAMILY_T16)
                         FLYSKY_HALL_RCC_AHB1Periph |
#endif
                         AUX_SERIAL_RCC_AHB1Periph |
                         AUX2_SERIAL_RCC_AHB1Periph |
                         TELEMETRY_RCC_AHB1Periph |
                         TRAINER_RCC_AHB1Periph |
                         AUDIO_RCC_AHB1Periph |
                         HAPTIC_RCC_AHB1Periph |
                         INTMODULE_RCC_AHB1Periph |
                         EXTMODULE_RCC_AHB1Periph |
                         SPORT_UPDATE_RCC_AHB1Periph,
                         ENABLE);

  RCC_APB1PeriphClockCmd(ROTARY_ENCODER_RCC_APB1Periph |
                         INTERRUPT_xMS_RCC_APB1Periph |
                         ADC_RCC_APB1Periph |
                         TIMER_2MHz_RCC_APB1Periph |
                         AUDIO_RCC_APB1Periph |
#if defined(RADIO_FAMILY_T16)
                         FLYSKY_HALL_RCC_APB1Periph |
#endif
                         TELEMETRY_RCC_APB1Periph |
                         AUDIO_RCC_APB1Periph |
                         MIXER_SCHEDULER_TIMER_RCC_APB1Periph |
                         BACKLIGHT_RCC_APB1Periph,
                         ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG |
                         LCD_RCC_APB2Periph |
                         ADC_RCC_APB2Periph |
                         HAPTIC_RCC_APB2Periph |
                         TELEMETRY_RCC_APB2Periph |
                         AUX_SERIAL_RCC_APB2Periph |
                         AUX2_SERIAL_RCC_APB2Periph |
                         BACKLIGHT_RCC_APB2Periph,
                         ENABLE);

#if defined(RADIO_FAMILY_T16)
  if (FLASH_OB_GetBOR() != OB_BOR_LEVEL3)
  {
    FLASH_OB_Unlock();
    FLASH_OB_BORConfig(OB_BOR_LEVEL3);
    FLASH_OB_Launch();
    FLASH_OB_Lock();
  }
#endif

  pwrInit();
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
  rotaryEncoderInit();

#if NUM_PWMSTICKS > 0
  sticksPwmInit();
  delay_ms(20);
  if (pwm_interrupt_count < 32) {
    hardwareOptions.sticksPwmDisabled = true;
  }
#endif

  globalData.flyskygimbals = false;
#if defined(RADIO_FAMILY_T16) || defined(PCBNV14)
  flysky_hall_stick_check_init();

  // Wait 70ms for FlySky gimbals to respond. According to LA trace, minimally 23ms is required
  for (uint8_t ui8 = 0; ui8 < 70; ui8++)
  {
      flysky_hall_stick_loop();
      delay_ms(1);
      if (globalData.flyskygimbals)
      {
          break;
      }
  }

#endif

  if (globalData.flyskygimbals)
  {
      flysky_hall_stick_init();
  }

  if (!adcInit(&ADC_DRIVER))
      TRACE("adcInit failed");


  init2MhzTimer();
  init1msTimer();
  usbInit();
  hapticInit();

#if defined(DEBUG)
  DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP|DBGMCU_TIM1_STOP|DBGMCU_TIM2_STOP|DBGMCU_TIM3_STOP|DBGMCU_TIM4_STOP|DBGMCU_TIM5_STOP|DBGMCU_TIM6_STOP|DBGMCU_TIM7_STOP|DBGMCU_TIM8_STOP|DBGMCU_TIM9_STOP|DBGMCU_TIM10_STOP|DBGMCU_TIM11_STOP|DBGMCU_TIM12_STOP|DBGMCU_TIM13_STOP|DBGMCU_TIM14_STOP, ENABLE);
#endif

  ledInit();

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

#if HAS_SPORT_UPDATE_CONNECTOR() && !defined(BOOT)
  sportUpdateInit();
#endif

  ledBlue();

#if defined(RTCLOCK) && !defined(COPROCESSOR)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif
}

void boardOff()
{
  backlightEnable(0);

  while (pwrPressed()) {
    WDG_RESET();
  }

  SysTick->CTRL = 0; // turn off systick

#if defined(PCBX12S)
  // Shutdown the Audio amp
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = AUDIO_SHUTDOWN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(AUDIO_SHUTDOWN_GPIO, &GPIO_InitStructure);
  GPIO_ResetBits(AUDIO_SHUTDOWN_GPIO, AUDIO_SHUTDOWN_GPIO_PIN);
#endif

  // Shutdown the Haptic
  hapticDone();

#if defined(RTC_BACKUP_RAM)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, DISABLE);
  PWR_BackupRegulatorCmd(DISABLE);
#endif

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

#if defined(AUX_SERIAL_PWR_GPIO) || defined(AUX2_SERIAL_PWR_GPIO)
static void _aux_pwr(GPIO_TypeDef *GPIOx, uint32_t pin, uint8_t on)
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOx, &pinInit);

  if (on) {
    LL_GPIO_SetOutputPin(GPIOx, pin);
  } else {
    LL_GPIO_ResetOutputPin(GPIOx, pin);
  }
}
#endif

#if defined(AUX_SERIAL)
void set_aux_pwr(uint8_t on)
{
#if defined(AUX_SERIAL_PWR_GPIO)
  _aux_pwr(AUX_SERIAL_PWR_GPIO, AUX_SERIAL_PWR_GPIO_PIN, on);
#endif
}

const etx_serial_port_t auxSerialPort = {
  "AUX1",
  &AuxSerialDriver,
  set_aux_pwr,
};
#define AUX_SERIAL_PORT &auxSerialPort
#else
#define AUX_SERIAL_PORT nullptr
#endif

#if defined(AUX2_SERIAL)
void set_aux2_pwr(uint8_t on)
{
#if defined(AUX2_SERIAL_PWR_GPIO)
  _aux_pwr(AUX2_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO_PIN, on);
#endif
}

const etx_serial_port_t aux2SerialPort = {
#if !defined(PCBX12S)
  "AUX2",
#else
  // AUX2 is hardwired to the internal GPS
  // on the X12S, let's hide the setting
  nullptr,
#endif
  &Aux2SerialDriver,
  set_aux2_pwr,
};
#define AUX2_SERIAL_PORT &aux2SerialPort
#else
#define AUX2_SERIAL_PORT nullptr
#endif // AUX2_SERIAL

static const etx_serial_port_t* serialPorts[MAX_AUX_SERIAL] = {
  AUX_SERIAL_PORT,
  AUX2_SERIAL_PORT,
};

const etx_serial_port_t* auxSerialGetPort(int port_nr)
{
  if (port_nr >= MAX_AUX_SERIAL) return nullptr;
  return serialPorts[port_nr];
}
