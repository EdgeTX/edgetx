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
Stream2:  PDM_CAPTURE_DMA_STREAM
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  I2S_DMA_Stream
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:  AUX2_SERIAL_DMA_RX_STREAM
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:  ADC_EXT_DMA_STREAM (do not move)
Stream1:  AUX_SERIAL_DMA_TX_STREAM
Stream2:  AUX_SERIAL_DMA_RX_STREAM
Stream3:  EXTMODULE_TIMER_DMA_STREAM
Stream4:  ADC_DMA_STREAM (do not move)
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM
Stream7:  AUX2_SERIAL_DMA_TX_STREAM

TIM1:	BACKLIGHT_TIMER
TIM2:	LED_STRIP_TIMER
TIM3:	HAPTIC_GPIO_TIMER
TIM4:
TIM5:	EXTMODULE_TIMER
TIM7:
TIM8:	TRAINER_TIMER
TIM12:	MIXER_SCHEDULER_TIMER
TIM14:	MS_TIMER
TIM15:  PDM_CAPTURE_TIMER
TIM16:
TIM17:	ROTARY_ENCODER_TIMER

SAI1_Block_A: PDM_CLOCK (1 MHz bit-clock output on SAI1_CK1 / PE5)

USART1: TELEMETRY_USART
USART2: EXTMODULE_USART
USART3: AUX2_SERIAL_USART
USART4:
USART5: AUX_SERIAL_USART
USART6: INTMODULE_USART
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
#define KEYS_GPIO_REG_TELE            GPIOI
#define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_15 // PI.15

// Trims
#define TRIMS_GPIO_REG_LHL
#define TRIMS_GPIO_PIN_LHL

#define TRIMS_GPIO_REG_LHR
#define TRIMS_GPIO_PIN_LHR

#define TRIMS_GPIO_REG_LVD
#define TRIMS_GPIO_PIN_LVD

#define TRIMS_GPIO_REG_LVU
#define TRIMS_GPIO_PIN_LVU

#define TRIMS_GPIO_REG_RHL
#define TRIMS_GPIO_PIN_RHL

#define TRIMS_GPIO_REG_RHR
#define TRIMS_GPIO_PIN_RHR

#define TRIMS_GPIO_REG_RVD
#define TRIMS_GPIO_PIN_RVD

#define TRIMS_GPIO_REG_RVU
#define TRIMS_GPIO_PIN_RVU

#define TRIMS_GPIO_REG_RSD
#define TRIMS_GPIO_PIN_RSD

#define TRIMS_GPIO_REG_RSU
#define TRIMS_GPIO_PIN_RSU

#define TRIMS_GPIO_REG_LSD          GPIOH
#define TRIMS_GPIO_PIN_LSD          LL_GPIO_PIN_15

#define TRIMS_GPIO_REG_LSU          GPIOH
#define TRIMS_GPIO_PIN_LSU          LL_GPIO_PIN_13

// Expanders
#define IO_INT_GPIO GPIO_PIN(GPIOD, 3)
#define IO_RESET_GPIO GPIO_PIN(GPIOG, 10)
#if !defined(USE_EXTI3_IRQ)
#define USE_EXTI3_IRQ
#define EXTI3_IRQ_Priority  9
#endif

// ADC
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_6      // PA.06
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_3      // PC.03
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_4      // PC.04
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_5      // PC.05

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_1      // PC.01 POT1
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 POT2
#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_3      // PH.03

#define ADC_GPIO_PIN_SLIDER1            LL_GPIO_PIN_2      // PH.02 VRC/LS
#define ADC_GPIO_PIN_SLIDER2            LL_GPIO_PIN_7      // PA.07 VRD/RS

#define ADC_GPIO_PIN_LUX                LL_GPIO_PIN_0      // PB.00  Light Sensor

#define ADC_GPIO_PIN_EXT1               LL_GPIO_PIN_5      // PA.05,ADC1_CH19
#define ADC_GPIO_PIN_EXT2               LL_GPIO_PIN_5      // PH.05,ADC3_CH16

#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_SLIDER2|ADC_GPIO_PIN_EXT1)
#define ADC_GPIOB_PINS                  (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_LUX)
#define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_POT1)
#define ADC_GPIOH_PINS                  (ADC_GPIO_PIN_BATT | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_EXT2)


#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_13   // ADC12_INP13
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_4    // ADC12_INP4
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_8    // ADC12_INP8

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_11   // ADC12_INP11
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_14   // ADC3_INP14
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC3_INP17
#define ADC_CHANNEL_SLIDER1             LL_ADC_CHANNEL_13   // ADC3_INP13
#define ADC_CHANNEL_SLIDER2             LL_ADC_CHANNEL_7    // ADC12_INP7

#define ADC_CHANNEL_LUX                 LL_ADC_CHANNEL_9    // ADC12_INP9    -> Light Sensor

#define ADC_CHANNEL_EXT1                LL_ADC_CHANNEL_19   // ADC1_IN19
#define ADC_CHANNEL_EXT2                LL_ADC_CHANNEL_16   // ADC3_IN16

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                { ADC_CHANNEL_SLIDER1, ADC_CHANNEL_BATT, ADC_CHANNEL_RTC_BAT, ADC_CHANNEL_EXT2 }
#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  330

#define ADC_DIRECTION {       	 \
    0,-1,0,-1,  /* gimbals */    \
    -1,-1,   	/* pots */       \
    0,-1,     	/* sliders */    \
    0,0,        /* ext */        \
    0,	     	/* vbat */       \
    0,       	/* rtc_bat */    \
    0	     	/* lux sensor */ \
  }

#define USE_EXTI9_5_IRQ // used for I2C port extender interrupt
#define EXTI9_5_IRQ_Priority 5

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOA,  4)
#define PWR_ON_GPIO                 GPIO_PIN(GPIOH, 12)

// Chargers (USB and wireless)
#define UCHARGER_GPIO               GPIO_PIN(GPIOD, 11)
#define UCHARGER_EN_GPIO            GPIO_PIN(GPIOB, 12)

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

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

// USB
#define USB_GPIO                        GPIOA
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOH, 11)
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11)
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12)
#define USB_GPIO_AF                     GPIO_AF10

// LCD

#define LCD_RESET_GPIO                  GPIOJ
#define LCD_RESET_GPIO_PIN              LL_GPIO_PIN_12

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


// AUDIO
#define AUDIO_I2C                       I2C_Bus_2
#define AUDIO_SPI                       SPI2
#define AUDIO_RESET_PIN                 GPIO_PIN(GPIOH, 10)
#define AUDIO_HP_DETECT_PIN             GPIO_PIN(GPIOH, 14)
#undef VOLUME_LEVEL_MAX
#define VOLUME_LEVEL_MAX                20
#define I2S_DMA                         DMA1
#define I2S_DMA_Stream                  LL_DMA_STREAM_4
#define I2S_DMA_Stream_Request          LL_DMAMUX1_REQ_SPI2_TX
#define I2S_DMA_Stream_IRQn             DMA1_Stream4_IRQn
#define I2S_DMA_Stream_IRQHandler       DMA1_Stream4_IRQHandler

//MEMS — PDM microphone:
//   clock output driven by SAI1 block A on SAI1_CK1 / PE5 (AF6, 1 MHz)
//   data input captured by DMA from PE4 @ 1 MHz.
#define PDM_CLOCK                       GPIO_PIN(GPIOE, 5)
#define PDM_CLOCK_GPIO_AF               LL_GPIO_AF_6
#define PDM_SAI_BLOCK                   SAI1_Block_A
#define PDM_SAI_KER_FREQ                48000000  // PLL1Q (see system_clock.c)
#define PDM_CLOCK_FREQ                  1600000   // 1.6 MHz → MCKDIV=30, R=100 → 16 kHz PCM direct
#define PDM_DATA                        GPIO_PIN(GPIOE, 4)
#define PDM_DATA_GPIO_PORT              GPIOE
#define PDM_DATA_GPIO_PIN               4
#define PDM_DATA_IDR_MASK               (1U << PDM_DATA_GPIO_PIN)

// capture PE4 via GPIO->memory DMA paced by TIM15 updates -> hardware-precise,
// non-blocking PDM sampling.
#define PDM_CAPTURE_DMA                 DMA1
#define PDM_CAPTURE_DMA_STREAM          LL_DMA_STREAM_2
#define PDM_CAPTURE_DMA_REQUEST         LL_DMAMUX1_REQ_TIM15_UP
#define PDM_CAPTURE_TIMER               TIM15
#define PDM_CAPTURE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)

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

// Internal Module
#define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOB, 13) // PB.13
#define INTMODULE_BOOTCMD_GPIO          GPIO_PIN(GPIOH, 9)  // PH.09
#define INTMODULE_BOOTCMD_DEFAULT       0
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOG, 14)
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOG,  9)
#define INTMODULE_USART                 USART6
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART6_IRQn
#define INTMODULE_USART_IRQHandler      USART6_IRQHandler
#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMAMUX1_REQ_USART6_TX
#define INTMODULE_RX_DMA                DMA1
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_5
#define INTMODULE_RX_DMA_CHANNEL        LL_DMAMUX1_REQ_USART6_RX
#define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream5_IRQn
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

// Trainer Port
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOA)

#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOI, 6)
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH2

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOI, 5)
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH1

#define TRAINER_TIMER                   TIM8
#define TRAINER_TIMER_IRQn              TIM8_CC_IRQn
#define TRAINER_TIMER_IRQHandler        TIM8_CC_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_3
#define TRAINER_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)

#define TRAINER_DETECT_GPIO             GPIO_PIN(GPIOH, 4)

// AUX ports
#define AUX_SERIAL_TX_GPIO                  GPIO_PIN(GPIOB, 6) // PB.06
#define AUX_SERIAL_RX_GPIO                  GPIO_PIN(GPIOB, 5) // PB.05
#define AUX_SERIAL_USART                    UART5
#define AUX_SERIAL_USART_IRQHandler         UART5_IRQHandler
#define AUX_SERIAL_USART_IRQn               UART5_IRQn
#define AUX_SERIAL_DMA_TX                   DMA2
#define AUX_SERIAL_DMA_TX_STREAM            LL_DMA_STREAM_1
#define AUX_SERIAL_DMA_TX_CHANNEL           LL_DMAMUX1_REQ_UART5_TX
#define AUX_SERIAL_DMA_RX                   DMA2
#define AUX_SERIAL_DMA_RX_STREAM            LL_DMA_STREAM_2
#define AUX_SERIAL_DMA_RX_CHANNEL           LL_DMAMUX1_REQ_UART5_RX
#define AUX_SERIAL_PWR_GPIO                 GPIO_PIN(GPIOB, 7) // PB.07

#if defined(AUX2_SERIAL)
#define AUX2_SERIAL_TX_GPIO                 GPIO_PIN(GPIOB, 10) // PB.10
#define AUX2_SERIAL_RX_GPIO                 GPIO_PIN(GPIOB, 11) // PB.11
#define AUX2_SERIAL_USART                   USART3
#define AUX2_SERIAL_USART_IRQHandler        USART3_IRQHandler
#define AUX2_SERIAL_USART_IRQn              USART3_IRQn
#define AUX2_SERIAL_DMA_TX                  DMA2
#define AUX2_SERIAL_DMA_TX_STREAM           LL_DMA_STREAM_7
#define AUX2_SERIAL_DMA_TX_CHANNEL          LL_DMAMUX1_REQ_USART3_TX
#define AUX2_SERIAL_DMA_RX                  DMA1
#define AUX2_SERIAL_DMA_RX_STREAM           LL_DMA_STREAM_6
#define AUX2_SERIAL_DMA_RX_CHANNEL          LL_DMAMUX1_REQ_USART3_RX
#define AUX2_SERIAL_PWR_GPIO                GPIO_PIN(GPIOC, 13) // PC.13
#elif defined(KCX_BTAUDIO)
#define BTAUDIO_POWER_GPIO                  GPIO_PIN(GPIOC, 13)
#define BTAUDIO_LINKED_GPIO                 GPIO_PIN(GPIOB, 11)
#define BTAUDIO_CONNECT_GPIO                GPIO_PIN(GPIOB, 10)
#elif defined(BLUETOOTH)
#define BT_USART                            USART3
#define BT_USART_IRQn                       USART3_IRQn
#define BT_TX_GPIO                          GPIO_PIN(GPIOB, 10) // PB.10
#define BT_RX_GPIO                          GPIO_PIN(GPIOB, 11) // PB.11
#define BT_EN_GPIO                          GPIO_PIN(GPIOE, 6)  // PE.06
#endif

// Touch
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOE, 2) // PE.02
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOJ, 13) // PJ.13

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI2_IRQ)
#define USE_EXTI2_IRQ
#define EXTI2_IRQ_Priority  9
#endif

#define LSE_DRIVE_STRENGTH  RCC_LSEDRIVE_HIGH

#endif // _HAL_H_
