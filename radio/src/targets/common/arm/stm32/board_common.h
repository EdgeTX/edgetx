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

#ifndef _BOARD_COMMON_H_
#define _BOARD_COMMON_H_

#include <inttypes.h>
#include "cpu_id.h"

#if defined(LUA_EXPORT_GENERATION)
// no includes
#else

#if __clang__
  // clang is very picky about the use of "register"
  // Tell clang to ignore the warnings for the following files
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-register"
#endif

#if defined(STM32F4)
  #include "stm32f4xx.h"
  #include "stm32f4xx_rcc.h"
  #include "stm32f4xx_syscfg.h"
  #include "stm32f4xx_gpio.h"
  #include "stm32f4xx_exti.h"
  #include "stm32f4xx_tim.h"
  #include "stm32f4xx_adc.h"
  #include "stm32f4xx_spi.h"
  #include "stm32f4xx_rtc.h"
  #include "stm32f4xx_pwr.h"
  #include "stm32f4xx_dma.h"
  #include "stm32f4xx_usart.h"
  #include "stm32f4xx_flash.h"
  #include "stm32f4xx_dbgmcu.h"
  #include "misc.h"
#else
  #include "stm32f2xx.h"
  #include "stm32f2xx_rcc.h"
  #include "stm32f2xx_syscfg.h"
  #include "stm32f2xx_gpio.h"
  #include "stm32f2xx_exti.h"
  #include "stm32f2xx_tim.h"
  #include "stm32f2xx_adc.h"
  #include "stm32f2xx_spi.h"
  #include "stm32f2xx_rtc.h"
  #include "stm32f2xx_pwr.h"
  #include "stm32f2xx_dma.h"
  #include "stm32f2xx_usart.h"
  #include "stm32f2xx_flash.h"
  #include "stm32f2xx_dbgmcu.h"
  #include "misc.h"
  #include "dwt.h"    // the old ST library that we use does not define DWT register for STM32F2xx
#endif

#if __clang__
// Restore warnings about registers
#pragma clang diagnostic pop
#endif

#endif

#include "usb_driver.h"

#if defined(SIMU)
#include "../simu/simpgmspace.h"
#endif

uint16_t getBatteryVoltage();

// STM32 uses a 25K+25K voltage divider bridge to measure the battery voltage
// Measuring VBAT puts considerable drain (22 µA) on the battery instead of normal drain (~10 nA)
static inline void enableVBatBridge()
{
  ADC->CCR |= ADC_CCR_VBATE;
}

static inline void disableVBatBridge()
{
  ADC->CCR &= ~ADC_CCR_VBATE;
}

static inline bool isVBatBridgeEnabled()
{
  return ADC->CCR & ADC_CCR_VBATE;
}

// Delays driver
#define SYSTEM_TICKS_1MS  ((CPU_FREQ + 500) / 1000)
#define SYSTEM_TICKS_1US  ((CPU_FREQ + 500000)  / 1000000)
#define SYSTEM_TICKS_01US ((CPU_FREQ + 5000000) / 10000000)

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t ticksNow()
{
#if defined(SIMU)
  return 0;
#else
  return DWT->CYCCNT;
#endif
}
  
#ifdef __cplusplus
}
#endif

#include "delays_driver.h"

#define INIT_KEYS_PINS(GPIO) \
  GPIO_InitStructure.GPIO_Pin = KEYS_ ## GPIO ## _PINS; \
  GPIO_Init(GPIO, &GPIO_InitStructure)

#define SET_KEYS_PINS_HIGH(GPIO) \
  GPIO_InitStructure.GPIO_Pin = KEYS_ ## GPIO ## _PINS; \
  GPIO_Init(GPIO, &GPIO_InitStructure); \
  GPIO_SetBits(GPIO, KEYS_ ## GPIO ## _PINS)

#if defined(ROTARY_ENCODER_NAVIGATION)
  typedef int32_t rotenc_t;
  extern volatile rotenc_t rotencValue;
  #define IS_ROTARY_ENCODER_NAVIGATION_ENABLE()  true
  #define ROTARY_ENCODER_NAVIGATION_VALUE        rotencValue
  #define ROTENC_LOWSPEED              1
  #define ROTENC_MIDSPEED              5
  #define ROTENC_HIGHSPEED             50
  #define ROTENC_DELAY_MIDSPEED        32
  #define ROTENC_DELAY_HIGHSPEED       16
#elif defined(RADIO_T8) && defined(__cplusplus)
  constexpr uint8_t rotencSpeed = 1;
#endif

#define ROTARY_ENCODER_GRANULARITY (2)

#if defined(PWR_BUTTON_PRESS)
  #define pwrOffPressed()              pwrPressed()
#else
  #define pwrOffPressed()              (!pwrPressed())
#endif

#endif
