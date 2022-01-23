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

#include "opentx.h"

#include "hal/adc_driver.h"
#include "stm32_hal_adc.h"

#if defined(__cplusplus)
extern "C" {
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus)
}
#endif

HardwareOptions hardwareOptions;

void watchdogInit(unsigned int duration)
{
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->PR = 3;           // Divide by 32 => 1kHz clock
  IWDG->KR = 0x5555;      // Unlock registers
  IWDG->RLR = duration;
  IWDG->KR = 0xAAAA;      // reload
  IWDG->KR = 0xCCCC;      // start
}

#if defined(SPORT_UPDATE_PWR_GPIO)
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
  GPIO_SPORT_UPDATE_PWR_GPIO_ON(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
}

void sportUpdatePowerOff()
{
  GPIO_SPORT_UPDATE_PWR_GPIO_OFF(SPORT_UPDATE_PWR_GPIO, SPORT_UPDATE_PWR_GPIO_PIN);
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
                         KEYS_RCC_AHB1Periph |
                         LCD_RCC_AHB1Periph |
                         AUDIO_RCC_AHB1Periph |
                         BACKLIGHT_RCC_AHB1Periph |
                         ADC_RCC_AHB1Periph |
                         I2C_B1_RCC_AHB1Periph |
                         SD_RCC_AHB1Periph |
                         HAPTIC_RCC_AHB1Periph |
                         INTMODULE_RCC_AHB1Periph |
                         EXTMODULE_RCC_AHB1Periph |
                         TELEMETRY_RCC_AHB1Periph |
                         SPORT_UPDATE_RCC_AHB1Periph |
                         AUX_SERIAL_RCC_AHB1Periph |
                         TRAINER_RCC_AHB1Periph |
                         TRAINER_MODULE_RCC_AHB1Periph |
                         BT_RCC_AHB1Periph |
                         I2C_B2_RCC_AHB1Periph |
                         USB_CHARGER_RCC_AHB1Periph,
                         ENABLE);

  RCC_APB1PeriphClockCmd(ROTARY_ENCODER_RCC_APB1Periph |
                         LCD_RCC_APB1Periph |
                         AUDIO_RCC_APB1Periph |
                         ADC_RCC_APB1Periph |
                         BACKLIGHT_RCC_APB1Periph |
                         HAPTIC_RCC_APB1Periph |
                         INTERRUPT_xMS_RCC_APB1Periph |
                         TIMER_2MHz_RCC_APB1Periph |
                         I2C_B1_RCC_APB1Periph |
                         SD_RCC_APB1Periph |
                         TRAINER_RCC_APB1Periph |
                         TELEMETRY_RCC_APB1Periph |
                         AUX_SERIAL_RCC_APB1Periph |
                         INTMODULE_RCC_APB1Periph |
                         TRAINER_MODULE_RCC_APB1Periph |
                         MIXER_SCHEDULER_TIMER_RCC_APB1Periph |
                         BT_RCC_APB1Periph |
                         I2C_B2_RCC_APB1Periph,
                         ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG |
                         BACKLIGHT_RCC_APB2Periph |
                         ADC_RCC_APB2Periph |
                         HAPTIC_RCC_APB2Periph |
                         INTMODULE_RCC_APB2Periph |
                         EXTMODULE_RCC_APB2Periph |
                         TRAINER_MODULE_RCC_APB2Periph |
                         BT_RCC_APB2Periph |
                         TELEMETRY_RCC_APB2Periph,
                         ENABLE);

#if defined(BLUETOOTH) && !defined(PCBX9E)
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
#endif

  pwrInit();

#if defined(AUTOUPDATE)
  telemetryPortInit(FRSKY_SPORT_BAUDRATE, TELEMETRY_SERIAL_WITHOUT_DMA);
  sportSendByteLoop(0x7E);
#endif

#if defined(STATUS_LEDS)
  ledInit();
  ledGreen();
#endif

  keysInit();

#if defined(ROTARY_ENCODER_NAVIGATION)
  rotaryEncoderInit();
#endif

  delaysInit();

#if NUM_PWMSTICKS > 0
  sticksPwmInit();
  delay_ms(20);
  if (pwm_interrupt_count < 32) {
    hardwareOptions.sticksPwmDisabled = true;
  }
#endif

  if (!adcInit(&stm32_hal_adc_driver))
      TRACE("adcInit failed");
  lcdInit(); // delaysInit() must be called before
  audioInit();
  init2MhzTimer();
  init1msTimer();
  __enable_irq();
  i2cInit();
  usbInit();

#if defined(DEBUG) && defined(AUX_SERIAL_GPIO)
  auxSerialInit(0, 0); // default serial mode (None if DEBUG not defined)
  TRACE("\nTaranis board started :)");
#endif

#if defined(HAPTIC)
  hapticInit();
#endif

#if defined(PXX2_PROBE)
  intmodulePxx2Probe();
#endif

#if defined(DEBUG)
  DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP|DBGMCU_TIM1_STOP|DBGMCU_TIM2_STOP|DBGMCU_TIM3_STOP|DBGMCU_TIM6_STOP|DBGMCU_TIM8_STOP|DBGMCU_TIM10_STOP|DBGMCU_TIM13_STOP|DBGMCU_TIM14_STOP, ENABLE);
#endif

#if defined(PWR_BUTTON_PRESS)
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    pwrOn();
  }
#endif

#if defined(TOPLCD_GPIO)
  toplcdInit();
#endif

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

  if (HAS_SPORT_UPDATE_CONNECTOR()) {
    sportUpdateInit();
  }

#if defined(JACK_DETECT_GPIO)
  initJackDetect();
#endif

  initSpeakerEnable();
  enableSpeaker();

  initHeadphoneTrainerSwitch();

#if defined(GYRO)
  gyroInit();
#endif

#if defined(RTCLOCK) && !defined(COPROCESSOR)
  rtcInit(); // RTC must be initialized before rambackupRestore() is called
#endif

  backlightInit();

#if defined(GUI)
  lcdSetContrast(true);
#endif
}

void boardOff()
{
#if defined(STATUS_LEDS) && !defined(BOOT)
  ledOff();
#endif

  BACKLIGHT_DISABLE();

#if defined(TOPLCD_GPIO) && !defined(BOOT)
  toplcdOff();
#endif

#if defined(PWR_BUTTON_PRESS)
  while (pwrPressed()) {
    WDG_RESET();
  }
#endif

  lcdOff();
  SysTick->CTRL = 0; // turn off systick
  pwrOff();

  // disable interrupts
  __disable_irq();

  while (1) {
    WDG_RESET();
#if defined(PWR_BUTTON_PRESS)
    // X9E/X7 needs watchdog reset because CPU is still running while
    // the power key is held pressed by the user.
    // The power key should be released by now, but we must make sure
    if (!pwrPressed()) {
      // Put the CPU into sleep to reduce the consumption,
      // it might help with the RTC reset issue
      PWR->CR |= PWR_CR_CWUF;
      /* Select STANDBY mode */
      PWR->CR |= PWR_CR_PDDS;
      /* Set SLEEPDEEP bit of Cortex System Control Register */
      SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
      /* Request Wait For Event */
      __WFE();
    }
#endif
  }

  // this function must not return!
}

#if defined (RADIO_TX12)
  #define BATTERY_DIVIDER 22830
#elif defined (RADIO_T8)
  #define BATTERY_DIVIDER 50000
#elif defined (RADIO_ZORRO)
  #define BATTERY_DIVIDER 23711 // = 2047*128*BATT_SCALE/(100*(VREF*(160+499)/160))
#else
  #define BATTERY_DIVIDER 26214
#endif 

#if defined(RADIO_ZORRO)
  #define VOLTAGE_DROP 45
#else
  #define VOLTAGE_DROP 20
#endif

uint16_t getBatteryVoltage()
{
  int32_t instant_vbat = anaIn(TX_VOLTAGE); // using filtered ADC value on purpose
  instant_vbat = (instant_vbat * BATT_SCALE * (128 + g_eeGeneral.txVoltageCalibration) ) / BATTERY_DIVIDER;
  instant_vbat += VOLTAGE_DROP; // add voltage drop because of the diode TODO check if this is needed, but removal will break existing calibrations!
  return (uint16_t)instant_vbat;
}

#if defined(AUDIO_SPEAKER_ENABLE_GPIO)
void initSpeakerEnable()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = AUDIO_SPEAKER_ENABLE_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(AUDIO_SPEAKER_ENABLE_GPIO, &GPIO_InitStructure);
}

void enableSpeaker()
{
  GPIO_SetBits(AUDIO_SPEAKER_ENABLE_GPIO, AUDIO_SPEAKER_ENABLE_GPIO_PIN);
}

void disableSpeaker()
{
  GPIO_ResetBits(AUDIO_SPEAKER_ENABLE_GPIO, AUDIO_SPEAKER_ENABLE_GPIO_PIN);
}
#endif

#if defined(HEADPHONE_TRAINER_SWITCH_GPIO)
void initHeadphoneTrainerSwitch()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = HEADPHONE_TRAINER_SWITCH_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(HEADPHONE_TRAINER_SWITCH_GPIO, &GPIO_InitStructure);
}

void enableHeadphone()
{
  GPIO_ResetBits(HEADPHONE_TRAINER_SWITCH_GPIO, HEADPHONE_TRAINER_SWITCH_GPIO_PIN);
}

void enableTrainer()
{
  GPIO_SetBits(HEADPHONE_TRAINER_SWITCH_GPIO, HEADPHONE_TRAINER_SWITCH_GPIO_PIN);
}
#endif

#if defined(JACK_DETECT_GPIO)
void initJackDetect(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = JACK_DETECT_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(JACK_DETECT_GPIO, &GPIO_InitStructure);
}
#endif
