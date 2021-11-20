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
#include "../common/arm/stm32/timers_driver.h"

#if !defined(PCBX12S)
  #include "../common/arm/stm32/stm32_hal_adc.h"
  #define ADC_DRIVER stm32_hal_adc_driver
#else
  #include "x12s_adc_driver.h"
  #define ADC_DRIVER x12s_adc_driver
#endif

#if defined(__cplusplus)
extern "C" {
#endif
#include "usb_dcd_int.h"
#include "usb_bsp.h"
#if defined(__cplusplus)
}
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

#if defined(AUX_SERIAL_PWR_GPIO)
void auxSerialPowerOn()
{
  GPIO_SetBits(AUX_SERIAL_PWR_GPIO, AUX_SERIAL_PWR_GPIO_PIN);
}

void auxSerialPowerOff()
{
  GPIO_ResetBits(AUX_SERIAL_PWR_GPIO, AUX_SERIAL_PWR_GPIO_PIN);
}
#endif
#if defined(AUX2_SERIAL_PWR_GPIO)
void aux2SerialPowerOn()
{
  GPIO_SetBits(AUX2_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO_PIN);
}

void aux2SerialPowerOff()
{
  GPIO_ResetBits(AUX2_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO_PIN);
}
#endif


#if HAS_SPORT_UPDATE_CONNECTOR()
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
#if defined(IMU_LSM6DS33)
                         I2C_B2_RCC_AHB1Periph |
#else
                         AUX_SERIAL_RCC_AHB1Periph |
#endif
                         AUX2_SERIAL_RCC_AHB1Periph |
                         TELEMETRY_RCC_AHB1Periph |
                         TRAINER_RCC_AHB1Periph |
                         BT_RCC_AHB1Periph |
                         AUDIO_RCC_AHB1Periph |
                         HAPTIC_RCC_AHB1Periph |
                         INTMODULE_RCC_AHB1Periph |
                         EXTMODULE_RCC_AHB1Periph |
                         I2C_B1_RCC_AHB1Periph |
                         GPS_RCC_AHB1Periph |
                         SPORT_UPDATE_RCC_AHB1Periph |
                         TOUCH_INT_RCC_AHB1Periph |
                         TOUCH_RST_RCC_AHB1Periph,
                         ENABLE);

  RCC_APB1PeriphClockCmd(ROTARY_ENCODER_RCC_APB1Periph |
                         INTERRUPT_xMS_RCC_APB1Periph |
                         ADC_RCC_APB1Periph |
                         TIMER_2MHz_RCC_APB1Periph |
                         AUDIO_RCC_APB1Periph |
#if defined(RADIO_FAMILY_T16)
                         FLYSKY_HALL_RCC_APB1Periph |
#endif
#if defined(IMU_LSM6DS33)
                         I2C_B2_RCC_APB1Periph |
#else
                         AUX_SERIAL_RCC_APB1Periph |
#endif
                         AUX2_SERIAL_RCC_APB1Periph |
                         TELEMETRY_RCC_APB1Periph |
                         TRAINER_RCC_APB1Periph |
                         AUDIO_RCC_APB1Periph |
                         INTMODULE_RCC_APB1Periph |
                         EXTMODULE_RCC_APB1Periph |
                         I2C_B1_RCC_APB1Periph |
                         MIXER_SCHEDULER_TIMER_RCC_APB1Periph |
                         GPS_RCC_APB1Periph |
                         BACKLIGHT_RCC_APB1Periph,
                         ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG |
                         LCD_RCC_APB2Periph |
                         ADC_RCC_APB2Periph |
                         HAPTIC_RCC_APB2Periph |
                         INTMODULE_RCC_APB2Periph |
                         EXTMODULE_RCC_APB2Periph |
                         TELEMETRY_RCC_APB2Periph |
                         BT_RCC_APB2Periph |
                         AUX_SERIAL_RCC_APB2Periph |
                         AUX2_SERIAL_RCC_APB2Periph |
                         GPS_RCC_APB2Periph |
                         BACKLIGHT_RCC_APB2Periph,
                         ENABLE);

  pwrInit();
  pwrOn();
  delaysInit();

  __enable_irq();

#if defined(DEBUG) && defined(AUX_SERIAL)
  auxSerialInit(UART_MODE_DEBUG, 0); // default serial mode (None if DEBUG not defined)
#endif
#if defined(DEBUG) && defined(AUX2_SERIAL)
  aux2SerialInit(UART_MODE_DEBUG, 0); // default serial mode (None if DEBUG not defined)
#endif

  TRACE("\nHorus board started :)");
  TRACE("RCC->CSR = %08x", RCC->CSR);

  audioInit();

  // we need to initialize g_FATFS_Obj here, because it is in .ram section (because of DMA access)
  // and this section is un-initialized
  memset(&g_FATFS_Obj, 0, sizeof(g_FATFS_Obj));

  keysInit();
  rotaryEncoderInit();

#if NUM_PWMSTICKS > 0
  sticksPwmInit();
  delay_ms(20);
  if (pwm_interrupt_count < 32) {
    hardwareOptions.sticksPwmDisabled = true;
  }
#endif

  lcdInit();
  backlightInit();

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

#if defined(IMU_LSM6DS33)
  imu_lsm6ds33_init();
#endif

  init2MhzTimer();
  init1msTimer();
  usbInit();
  hapticInit();

#if defined(BLUETOOTH)
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, true);
#endif

#if defined(INTERNAL_GPS)
  gpsInit(GPS_USART_BAUDRATE);
#endif

#if defined(DEBUG)
  DBGMCU_APB1PeriphConfig(DBGMCU_IWDG_STOP|DBGMCU_TIM1_STOP|DBGMCU_TIM2_STOP|DBGMCU_TIM3_STOP|DBGMCU_TIM4_STOP|DBGMCU_TIM5_STOP|DBGMCU_TIM6_STOP|DBGMCU_TIM7_STOP|DBGMCU_TIM8_STOP|DBGMCU_TIM9_STOP|DBGMCU_TIM10_STOP|DBGMCU_TIM11_STOP|DBGMCU_TIM12_STOP|DBGMCU_TIM13_STOP|DBGMCU_TIM14_STOP, ENABLE);
#endif

  ledInit();

#if defined(USB_CHARGER)
  usbChargerInit();
#endif

#if HAS_SPORT_UPDATE_CONNECTOR()
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

#if defined (RADIO_TX16S)
  #define BATTERY_DIVIDER 1495
#else
  #define BATTERY_DIVIDER 1629
#endif 

uint16_t getBatteryVoltage()
{
  int32_t instant_vbat = anaIn(TX_VOLTAGE);  // using filtered ADC value on purpose
  return (uint16_t)((instant_vbat * (1000 + g_eeGeneral.txVoltageCalibration)) / BATTERY_DIVIDER);
}

bool isBacklightEnabled()
{
  if(globalData.unexpectedShutdown)
    return true;
  return boardBacklightOn;
}
