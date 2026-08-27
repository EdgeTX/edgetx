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

/*
STM32H750
DMA1
Stream0:  LED_STRIP_TIMER_DMA_STREAM
Stream1:  INTMODULE_DMA_STREAM
Stream2:  FLYSKY_HALL_DMA_Stream_RX
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  ADC_DMA_STREAM
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:
Stream1:  AUDIO_DMA_Stream
Stream2:
Stream3:  EXTMODULE_TIMER_DMA_STREAM
Stream4:  ADC_DMA_STREAM
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM

TIM1:     LED_STRIP_TIMER
TIM2:	  BACKLIGHT_TIMER
TIM3:     EXTMODULE_TIMER
TIM4:	  (no pins)
TIM5:     TRAINER_TIMER
TIM6:     AUDIO_TIMER
TIM7:	  (no pins)
TIM8:	  TRAINER_TIMER
TIM12:	  MIXER_SCHEDULER_TIMER
TIM13:
TIM14:    MS_TIMER
TIM15:    HAPTIC_GPIO_TIMER
TIM16:
TIM17:	  ROTARY_ENCODER_TIMER

USARTS
USART1: INTMODULE_USART
USART2: TELEMETRY_USART
USART3: DGONE
USART4: BT MODULE
USART6: EXTMODULE_USART
*/

#ifndef _HAL_H_
#define _HAL_H_

#include "hal_settings.h"

// Keys
#define KEYS_GPIO_REG_PAGEDN          GPIOI
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_4
#define KEYS_GPIO_REG_SYS             GPIOI
#define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_5
#define KEYS_GPIO_REG_ENTER           GPIOI
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_2
#define KEYS_GPIO_REG_MDL             GPIOE
#define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_3
#define KEYS_GPIO_REG_EXIT            GPIOI
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_8
#define KEYS_GPIO_REG_TELE            GPIOE
#define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_4

// Trims
#define TRIMS_GPIO_REG_LHL            GPIOI
#define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_10

#define TRIMS_GPIO_REG_LHR            GPIOI
#define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_9

#define TRIMS_GPIO_REG_LVD            GPIOB
#define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_13

#define TRIMS_GPIO_REG_LVU            GPIOB
#define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_14

#define TRIMS_GPIO_REG_RHL            GPIOG
#define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_2

#define TRIMS_GPIO_REG_RHR            GPIOG
#define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_3

#define TRIMS_GPIO_REG_RVD            GPIOB
#define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_12

#define TRIMS_GPIO_REG_RVU            GPIOB
#define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_15

// ADC
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_0      // PA.00 ADC1_INP16
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_3      // PA.03 ADC12_INP15
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_1      // PA.01 ADC1_INP17
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_2      // PA.02 ADC12_INP14

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_3      // PC.03 POT2 ADC12_INP13
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_2      // PC.02 POT1 ADC123_INP12
#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_5      // PC.05 ADC12_INP8

#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV, ADC_GPIO_PIN_STICK_RH, ADC_GPIO_PIN_STICK_RV)
#define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_BATT )

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_16
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_17
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_14

#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_13
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_12
#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_8
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT  // ADC3 IMP17

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                { ADC_CHANNEL_RTC_BAT }
#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  330

#define ADC_DIRECTION {       	 \
0,-1,0,-1, 	/* gimbals */    \
1,1        	/* pots */       \
}


// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOI, 3)
#define PWR_ON_GPIO                 GPIO_PIN(GPIOD, 3)

// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)


// Telemetry
#define TELEMETRY_SET_INPUT             0
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOD, 5)
#define TELEMETRY_RX_GPIO               GPIO_UNDEF
#define TELEMETRY_USART                 USART2
#define TELEMETRY_USART_IRQn            USART2_IRQn
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_7
#define TELEMETRY_DMA_Channel_TX        LL_DMAMUX1_REQ_USART2_TX
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
#define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_3
#define TELEMETRY_DMA_Channel_RX        LL_DMAMUX1_REQ_USART2_RX
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler

// Software IRQ (Prio 5 -> FreeRTOS compatible)
//#define USE_CUSTOM_EXTI_IRQ
#define TELEMETRY_USE_CUSTOM_EXTI
#define CUSTOM_EXTI_IRQ_NAME ETH_WKUP_IRQ
#define ETH_WKUP_IRQ_Priority 5
#define CUSTOM_EXTI_IRQ_LINE 86
//#define USE_EXTI15_10_IRQ
//#define CUSTOM_EXTI_IRQ_Priority 5
#define TELEMETRY_RX_FRAME_EXTI_LINE    CUSTOM_EXTI_IRQ_LINE

// USB
#define USB_GPIO                        GPIOA
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOA, 9)
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11)
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12)
#define USB_GPIO_AF                     GPIO_AF10

// LCD SPI1
#define LCD_SPI_CS_GPIO                 GPIOG
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_10
#define LCD_SPI_GPIO                    GPIOB
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_3
#define LCD_SPI_MISO_GPIO_PIN           LL_GPIO_PIN_4
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_5

#define LCD_NRST_GPIO                   GPIOI
#define LCD_NRST_GPIO_PIN               LL_GPIO_PIN_15

// QSPI Flash
#define QSPI_CLK_GPIO                   GPIO_PIN(GPIOF, 10)
#define QSPI_CLK_GPIO_AF                GPIO_AF9
#define QSPI_CS_GPIO                    GPIO_PIN(GPIOG, 6)
#define QSPI_CS_GPIO_AF                 GPIO_AF10
#define QSPI_MISO_GPIO                  GPIO_PIN(GPIOF, 9)
#define QSPI_MISO_GPIO_AF               GPIO_AF10
#define QSPI_MOSI_GPIO                  GPIO_PIN(GPIOF, 8)
#define QSPI_MOSI_GPIO_AF               GPIO_AF10
#define QSPI_WP_GPIO                    GPIO_PIN(GPIOF, 7)
#define QSPI_WP_GPIO_AF                 GPIO_AF9
#define QSPI_HOLD_GPIO                  GPIO_PIN(GPIOF, 6)
#define QSPI_HOLD_GPIO_AF               GPIO_AF9
#define QSPI_FLASH_SIZE                 0x1000000

#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(24000000)
#define STORAGE_USE_SDIO

// Chargers (USB and wireless)
#define UCHARGER_GPIO                  GPIO_PIN(GPIOD, 6)

// Audio
#define AUDIO_MUTE_GPIO                GPIO_PIN(GPIOH, 4)
#define AUDIO_MUTE_DELAY               200  // ms
#define AUDIO_UNMUTE_DELAY             100  // ms
#define AUDIO_OUTPUT_GPIO              GPIO_PIN(GPIOA, 4)
#define AUDIO_DAC                      DAC1
#define AUDIO_DMA_Stream               LL_DMA_STREAM_1
#define AUDIO_DMA_Channel              LL_DMAMUX1_REQ_DAC1_CH1
#define AUDIO_TIM_IRQn                 TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler           TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQn          DMA2_Stream1_IRQn
#define AUDIO_DMA_Stream_IRQHandler    DMA2_Stream1_IRQHandler
#define AUDIO_TIMER                    TIM6
#define AUDIO_DMA                      DMA2

// I2C Bus
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 9)
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

#define I2C_B2                          I2C4
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOD, 13)
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOD, 12)
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000

// Internal Module
#define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOI, 1)
#define INTMODULE_BOOTCMD_GPIO          GPIO_PIN(GPIOD, 7)
#define INTMODULE_BOOTCMD_DEFAULT       0
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 6)
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 7)
#define INTMODULE_USART                 USART1
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART1_IRQn
#define INTMODULE_USART_IRQHandler      USART1_IRQHandler
#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMAMUX1_REQ_USART1_TX
#define INTMODULE_RX_DMA                DMA1
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_5
#define INTMODULE_RX_DMA_CHANNEL        LL_DMAMUX1_REQ_USART1_RX
#define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream5_IRQn
#define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream5_IRQHandler

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOI, 0)
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6) // TIM3_CH1
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)

#define EXTMODULE_TIMER                 TIM3
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
#define EXTMODULE_TIMER_IRQn            TIM3_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_2

//USART
#define EXTMODULE_USART                    USART6
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART6_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART6_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5
#define EXTMODULE_USART_IRQHandler         USART6_IRQHandler
#define EXTMODULE_USART_IRQn               USART6_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM3_UP
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_3
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream3_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream3_IRQHandler

// Trainer Port
#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOI, 6)  // TIM8_CH2
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH2

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOI, 7)  // TIM8_CH3
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH3

#define TRAINER_TIMER                   TIM8
#define TRAINER_TIMER_IRQn              TIM8_CC_IRQn
#define TRAINER_TIMER_IRQHandler        TIM8_CC_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_3
#define TRAINER_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)

// Touch
#define TOUCH_I2C_BUS                 I2C_Bus_2
#define TOUCH_RST_GPIO                GPIO_PIN(GPIOJ, 1)  // PJ.01
#define TOUCH_INT_GPIO                GPIO_PIN(GPIOD, 11)  // PD.11

#define TOUCH_INT_EXTI_Line           LL_EXTI_LINE_11
#define TOUCH_INT_EXTI_Port           LL_SYSCFG_EXTI_PORTD
#define TOUCH_INT_EXTI_SysCfgLine     LL_SYSCFG_EXTI_LINE11

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI15_10_IRQ)
#define USE_EXTI15_10_IRQ
#define EXTI15_10_IRQ_Priority 9
#endif

#define LSE_DRIVE_STRENGTH  RCC_LSEDRIVE_HIGH

#endif // _HAL_H_
