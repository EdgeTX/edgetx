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
Stream2:  FLYSKY_HALL_DMA_Stream_RX (only if FLYSKY_GIMBAL enabled; OFF on T22)
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  (free)
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:  ADC_EXT_DMA_STREAM (ADC3, see boards/hw_defs/t22.json)
Stream1:  AUDIO_DMA_Stream
Stream2:
Stream3:  EXTMODULE_TIMER_DMA_STREAM
Stream4:  ADC_DMA_STREAM (ADC1, see boards/hw_defs/t22.json)
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM

TIM1:     LED_STRIP_TIMER
TIM2:	  BACKLIGHT_TIMER
TIM3:     EXTMODULE_TIMER
TIM4:	  (no pins)
TIM5:     (free)
TIM6:     AUDIO_TIMER
TIM7:	  (no pins)
TIM8:	  TRAINER_TIMER
TIM12:	  MIXER_SCHEDULER_TIMER
TIM13:
TIM14:    MS_TIMER
TIM15:    HAPTIC_GPIO_TIMER
TIM16:
TIM17:	  ROTARY_ENCODER_TIMER
*/

#ifndef _HAL_H_
#define _HAL_H_

#include "hal_settings.h"

#define ADC_VREF_PREC2                  330

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
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

#define LCD_NRST_GPIO                   GPIOI
#define LCD_NRST_GPIO_PIN               LL_GPIO_PIN_15

// Backlight
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOA, 5) // TIM2_CH1
#define BACKLIGHT_TIMER                 TIM2
#define BACKLIGHT_TIMER_CHANNEL			LL_TIM_CHANNEL_CH1
#define BACKLIGHT_GPIO_AF               GPIO_AF1
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

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

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 USART3
#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOB, 10)
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOB, 11)
#define FLYSKY_HALL_SERIAL_USART_IRQn            USART3_IRQn
#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Channel                  LL_DMAMUX1_REQ_USART3_RX

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