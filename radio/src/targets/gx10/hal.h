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

SDMMC uses own DMA controler

DMA1
Stream0:  LED_STRIP_TIMER_DMA_STREAM
Stream1:  INTMODULE_DMA_STREAM
Stream2:  PDM_DFSDM_DMA_STREAM
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  I2S_DMA_Stream
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:  AUX_SERIAL_DMA_RX_STREAM
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:  ADC_EXT_DMA_STREAM (do not move)
Stream1:  LCD_SPI_TX_DMA_STREAM
Stream2:  LCD_SPI_RX_DMA_STREAM
Stream3:
Stream4:  ADC_DMA_STREAM (do not move)
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM / EXTMODULE_TIMER_DMA_STREAM
Stream7:  AUX_SERIAL_DMA_TX_STREAM

TIM1:	BACKLIGHT_TIMER
TIM2:	LED_STRIP_TIMER
TIM3:	HAPTIC_GPIO_TIMER
TIM4:
TIM5:	EXTMODULE_TIMER
TIM6:
TIM7:
TIM8:	TRAINER_TIMER
TIM12:	MIXER_SCHEDULER_TIMER
TIM14:	MS_TIMER
TIM15:
TIM16:
TIM17:	ROTARY_ENCODER_TIMER

DFSDM1: PDM microphone (CKOUT on PD3, channel 3 / filter 0 on PE4)
 */

#ifndef _HAL_H_
#define _HAL_H_

#include "hal_settings.h"

// Keys
#define KEYS_GPIO_REG_PAGEDN          GPIOA
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8  // PA.08
#define KEYS_GPIO_REG_PAGEUP          GPIOG
#define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_7  // PG.07
#define KEYS_GPIO_REG_SYS             GPIOB
#define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_2  // PB.02
#define KEYS_GPIO_REG_ENTER           GPIOG
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_12 // PG.12
#define KEYS_GPIO_REG_MDL             GPIOE
#define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_3  // PE.03
#define KEYS_GPIO_REG_EXIT            GPIOG
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_3  // PG.03
#define KEYS_GPIO_REG_TELE            GPIOG
#define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_2  // PG.02

// Trims
#define TRIMS_GPIO_REG_LHL            GPIOJ
#define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_6  // PJ.06

#define TRIMS_GPIO_REG_LHR            GPIOJ
#define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_7  // PJ.07

#define TRIMS_GPIO_REG_LVD            GPIOJ
#define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_9  // PJ.09

#define TRIMS_GPIO_REG_LVU            GPIOJ
#define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_10 // PJ.10

#define TRIMS_GPIO_REG_RHL            GPIOK
#define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_0  // PK.00

#define TRIMS_GPIO_REG_RHR            GPIOJ
#define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_11 // PJ.11

#define TRIMS_GPIO_REG_RVD            GPIOK
#define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_1  // PK.01

#define TRIMS_GPIO_REG_RVU            GPIOK
#define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PK.02

// Switches
#define SWITCHES_A_2POS
#define SWITCHES_GPIO_REG_A           GPIOJ
#define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_4   // PJ.04

#define SWITCHES_B_3POS
#define SWITCHES_GPIO_REG_B_H         GPIOJ
#define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_3   // PJ.03
#define SWITCHES_GPIO_REG_B_L         GPIOJ
#define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_2   // PJ.02

#define SWITCHES_C_3POS
#define SWITCHES_GPIO_REG_C_H         GPIOK
#define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_4   // PK.04
#define SWITCHES_GPIO_REG_C_L         GPIOK
#define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_5   // PK.05

#define SWITCHES_D_2POS
#define SWITCHES_GPIO_REG_D           GPIOK
#define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_3   // PK.03

#define SWITCHES_E_3POS
#define SWITCHES_GPIO_REG_E_H         GPIOJ
#define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_1   // PJ.01
#define SWITCHES_GPIO_REG_E_L         GPIOJ
#define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_0   // PJ.00

#define SWITCHES_F_3POS
#define SWITCHES_GPIO_REG_F_H         GPIOJ
#define SWITCHES_GPIO_PIN_F_H         LL_GPIO_PIN_14   // PJ.14
#define SWITCHES_GPIO_REG_F_L         GPIOJ
#define SWITCHES_GPIO_PIN_F_L         LL_GPIO_PIN_15   // PJ.15

#define SWITCHES_G_2POS
#define SWITCHES_GPIO_REG_G           GPIOJ
#define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_5   // PJ.05

#define SWITCHES_H_2POS
#define SWITCHES_GPIO_REG_H           GPIOK
#define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_6   // PK.06

// function switches
//SW1
#define SWITCHES_GPIO_REG_I           GPIOI
#define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_15  // PI.15
#define SWITCHES_I_CFS_IDX            0
#define FUNCTION_SWITCH_1             SI
//SW2
#define SWITCHES_GPIO_REG_J           GPIOI
#define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_12  // PI.12
#define SWITCHES_J_CFS_IDX            1
#define FUNCTION_SWITCH_2             SJ
//SW3
#define SWITCHES_GPIO_REG_K           GPIOI
#define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_13  // PI.13
#define SWITCHES_K_CFS_IDX            2
#define FUNCTION_SWITCH_3             SK
//SW4
#define SWITCHES_GPIO_REG_L           GPIOI
#define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_14  // PI.14
#define SWITCHES_L_CFS_IDX            3
#define FUNCTION_SWITCH_4             SL
//SW5
#define SWITCHES_GPIO_REG_M           GPIOI
#define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_9   // PI.09
#define SWITCHES_M_CFS_IDX            4
#define FUNCTION_SWITCH_5             SM
//SW6
#define SWITCHES_GPIO_REG_N           GPIOK
#define SWITCHES_GPIO_PIN_N           LL_GPIO_PIN_7   // PK.07
#define SWITCHES_N_CFS_IDX            5
#define FUNCTION_SWITCH_6             SN

// ADC
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_6      // PA.06
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_3      // PC.03
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_4      // PC.04
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_5      // PC.05

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_1      // PC.01 POT1
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 POT2
#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_3      // PH.03

#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_RH)
#define ADC_GPIOB_PINS                  (ADC_GPIO_PIN_POT2)
#define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_POT1)
#define ADC_GPIOH_PINS                  (ADC_GPIO_PIN_BATT)

#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_13   // ADC12_INP13
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_4    // ADC12_INP4
#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_8    // ADC12_INP8

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_11   // ADC12_INP11
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_14   // ADC3_INP14
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC3_INP17

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                { ADC_CHANNEL_BATT , ADC_CHANNEL_RTC_BAT }
#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  330

#define USE_EXTI9_5_IRQ // used for I2C port extender interrupt
#define EXTI9_5_IRQ_Priority 5

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOA,  1)
#define PWR_ON_GPIO                 GPIO_PIN(GPIOH, 12)

// Chargers (USB and wireless)
#define UCHARGER_GPIO               GPIO_PIN(GPIOD, 11)
#define UCHARGER_EN_GPIO            GPIO_PIN(GPIOB, 12)
#define UCHARGER_INVERTED           (true)

// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

// Internal CRSF module defaults to 1.87M
#define DEFAULT_INT_CROSSFIRE_BR_IDX    3

// Telemetry
 #define TELEMETRY_SET_INPUT             0
 #define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOA, 9)
 #define TELEMETRY_RX_GPIO               GPIO_UNDEF
 #define TELEMETRY_USART                 USART1
 #define TELEMETRY_USART_IRQn            USART1_IRQn
 #define TELEMETRY_DMA                   DMA1
 #define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_7
 #define TELEMETRY_DMA_Channel_TX        LL_DMAMUX1_REQ_USART1_TX
 #define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
 #define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
 #define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_3
 #define TELEMETRY_DMA_Channel_RX        LL_DMAMUX1_REQ_USART1_RX
 #define TELEMETRY_USART_IRQHandler      USART1_IRQHandler

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_USE_CUSTOM_EXTI
#define CUSTOM_EXTI_IRQ_NAME ETH_WKUP_IRQ
#define ETH_WKUP_IRQ_Priority 5
#define CUSTOM_EXTI_IRQ_LINE 86
#define TELEMETRY_RX_FRAME_EXTI_LINE    CUSTOM_EXTI_IRQ_LINE

// LCD
#define LCD_SPI                         SPI1
#define LCD_SPI_DMA                     DMA2
#define LCD_SPI_TX_DMA                  LL_DMAMUX1_REQ_SPI1_TX
#define LCD_SPI_RX_DMA                  LL_DMAMUX1_REQ_SPI1_RX
#define LCD_SPI_TX_DMA_STREAM           LL_DMA_STREAM_1
#define LCD_SPI_RX_DMA_STREAM           LL_DMA_STREAM_2
#define LCD_FMARK                       GPIO_PIN(GPIOI,  4)//TE
#define LCD_NRST                        GPIO_PIN(GPIOJ,  12)
#define LCD_SPI_RS                      GPIO_PIN(GPIOB, 3)
#define LCD_SPI_CS                      GPIO_PIN(GPIOA, 4)
#define LCD_SPI_CLK                     GPIO_PIN(GPIOA, 5)
#define LCD_SPI_MOSI                    GPIO_PIN(GPIOA,  7)
#define LCD_SPI_BAUD                    100000000 //100MHz
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

// SD
#define SD_SDIO                         SDMMC1
#define SD_SDIO_CLK_DIV(fq)             (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_TRANSFER_CLK_DIV        SD_SDIO_CLK_DIV(24000000)    // Hz

#define SD2_PRESENT_GPIO                GPIO_PIN(GPIOH, 8) // PH.08
#define SD2_SDIO                        SDMMC2
#define SD2_SDIO_PIN_D0                 GPIO_PIN(GPIOB, 14)
#define SD2_SDIO_AF_D0                  GPIO_AF9
#define SD2_SDIO_PIN_D1                 GPIO_PIN(GPIOB, 15)
#define SD2_SDIO_AF_D1                  GPIO_AF9
#define SD2_SDIO_PIN_D2                 GPIO_PIN(GPIOG, 11)
#define SD2_SDIO_AF_D2                  GPIO_AF10
#define SD2_SDIO_PIN_D3                 GPIO_PIN(GPIOB, 4)
#define SD2_SDIO_AF_D3                  GPIO_AF9
#define SD2_SDIO_PIN_CLK                GPIO_PIN(GPIOD, 6)
#define SD2_SDIO_AF_CLK                 GPIO_AF11
#define SD2_SDIO_PIN_CMD                GPIO_PIN(GPIOD, 7)
#define SD2_SDIO_AF_CMD                 GPIO_AF11
#define SD2_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(24000000)    // Hz

#define STORAGE_USE_SDIO

// AUDIO (TAS2505 codec on I2S2, shared pinout with TX15)
#define AUDIO_I2C                     I2C_Bus_1
#define AUDIO_SPI                     SPI2
#define AUDIO_RESET_PIN               GPIO_PIN(GPIOH, 10)
#define AUDIO_HP_DETECT_PIN           GPIO_PIN(GPIOH, 14)
#define I2S_DMA                       DMA1
#define I2S_DMA_Stream                LL_DMA_STREAM_4
#define I2S_DMA_Stream_Request        LL_DMAMUX1_REQ_SPI2_TX
#define I2S_DMA_Stream_IRQn           DMA1_Stream4_IRQn
#define I2S_DMA_Stream_IRQHandler     DMA1_Stream4_IRQHandler

// MEMS — PDM microphone (LMD4737T261-04C02), captured by DFSDM:
//   PD3 = DFSDM1 CKOUT (1.6 MHz), PE4 = DFSDM1_DATIN3
#define PDM_CLOCK                       GPIO_PIN(GPIOD, 3)
#define PDM_CLOCK_GPIO_AF               LL_GPIO_AF_3
#define PDM_CLOCK_FREQ                  1600000   // PCLK2 / 75
#define PDM_DATA                        GPIO_PIN(GPIOE, 4)
#define PDM_DATA_GPIO_AF                LL_GPIO_AF_3

#define PDM_DFSDM_KER_FREQ              PERI2_FREQUENCY  // DFSDM1SEL = 0 (pclk2)
#define PDM_DFSDM_CHANNEL               DFSDM1_Channel3
#define PDM_DFSDM_CHANNEL_ID            3
#define PDM_DFSDM_FILTER                DFSDM1_Filter0
#define PDM_DFSDM_FOSR                  100  // Sinc3, 1.6 MHz / 100 = 16 kHz PCM
#define PDM_DFSDM_SITP                  0    // L/R strapped low: strobe on rising edge

#define PDM_DFSDM_DMA                   DMA1
#define PDM_DFSDM_DMA_STREAM            LL_DMA_STREAM_2
#define PDM_DFSDM_DMA_REQUEST           LL_DMAMUX1_REQ_DFSDM1_FLT0

// I2C Bus
#define I2C_B1                          I2C4
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOD, 13) // PD.13
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOD, 12) // PD.12
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

#define I2C_B2                          I2C1
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOB, 9) // PB.09
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOB, 8) // PB.08
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000

#define GPIO_LED_GPIO_ON                  gpio_set
#define GPIO_LED_GPIO_OFF                 gpio_clear

// Internal Module
#define INTMODULE_PWR_GPIO                GPIO_PIN(GPIOB, 13) // PB.13
#define INTMODULE_BOOTCMD_GPIO            GPIO_PIN(GPIOH, 9)  // PH.09
#define INTMODULE_BOOTCMD_DEFAULT         0
#define INTMODULE_TX_GPIO                 GPIO_PIN(GPIOG, 14)
#define INTMODULE_RX_GPIO                 GPIO_PIN(GPIOG,  9)
#define INTMODULE_USART                   USART6
#define INTMODULE_GPIO_AF                 LL_GPIO_AF_7
#define INTMODULE_USART_IRQn              USART6_IRQn
#define INTMODULE_USART_IRQHandler        USART6_IRQHandler
#define INTMODULE_DMA                     DMA1
#define INTMODULE_DMA_STREAM              LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ          DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC             DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL             LL_DMAMUX1_REQ_USART6_TX
#define INTMODULE_RX_DMA                  DMA1
#define INTMODULE_RX_DMA_STREAM           LL_DMA_STREAM_5
#define INTMODULE_RX_DMA_CHANNEL          LL_DMAMUX1_REQ_USART6_RX
#define INTMODULE_RX_DMA_Stream_IRQn      DMA1_Stream5_IRQn
#define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream5_IRQHandler

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOD, 4) // PD.04
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOA, 2) // TIM2_CH3, TIM5_CH3,TIM15_CH1,
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOA, 3)
#define EXTMODULE_TX_GPIO_AF            LL_GPIO_AF_3 // TIM5_CH3
#define EXTMODULE_TIMER                 TIM5
#define EXTMODULE_TIMER_32BITS
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH3
#define EXTMODULE_TIMER_IRQn            TIM5_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM5_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_2

//USART
#define EXTMODULE_USART                    USART2
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART2_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART2_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5
#define EXTMODULE_USART_IRQHandler         USART2_IRQHandler
#define EXTMODULE_USART_IRQn               USART2_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM5_UP
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_6
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream6_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream6_IRQHandler

// AUX ports
#define AUX_SERIAL_TX_GPIO                 GPIO_PIN(GPIOB, 10) // PB.10
#define AUX_SERIAL_RX_GPIO                 GPIO_PIN(GPIOB, 11) // PB.11
#define AUX_SERIAL_USART                   USART3
#define AUX_SERIAL_USART_IRQHandler        USART3_IRQHandler
#define AUX_SERIAL_USART_IRQn              USART3_IRQn
#define AUX_SERIAL_DMA_TX                  DMA2
#define AUX_SERIAL_DMA_TX_STREAM           LL_DMA_STREAM_7
#define AUX_SERIAL_DMA_TX_CHANNEL          LL_DMAMUX1_REQ_USART3_TX
#define AUX_SERIAL_DMA_RX                  DMA1
#define AUX_SERIAL_DMA_RX_STREAM           LL_DMA_STREAM_6
#define AUX_SERIAL_DMA_RX_CHANNEL          LL_DMAMUX1_REQ_USART3_RX
#define AUX_SERIAL_PWR_GPIO                GPIO_PIN(GPIOE, 6)  // PE.06

#endif // _HAL_H_
