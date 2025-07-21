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

#pragma once

// DMA streams:
// 
// - LEDs       TIM2_UP: DMA1 stream 0
// - Audio      SPI1_TX: DMA1 stream 1
// - Gimbals  USART2_RX: DMA1 stream 2
// - Telemetry UART5_RX: DMA1 stream 3
// - Telemetry UART5_TX: DMA1 stream 7
// 
// - ADC3:               DMA2 stream 0
// - ADC1:               DMA2 stream 4
// - Ext. Mod. UART4_RX: DMA2 stream 5
// - Ext. Mod. UART4_TX: DMA2 stream 6
// - Ext. Mod.  TIM4_UP: DMA2 stream 6
// 


#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

// Keys
#define KEYS_GPIO_REG_ENTER           GPIOG
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_14  // PG.14
#define KEYS_GPIO_REG_PAGEDN          GPIOA
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8   // PA.08
#define KEYS_GPIO_REG_EXIT            GPIOC
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5   // PC.05

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


// function switches
#define FUNCTION_SWITCH_1               SK
#define SWITCHES_GPIO_REG_K
#define SWITCHES_GPIO_PIN_K             PCA95XX_PIN_0
#define SWITCHES_K_CFS_IDX              0
#define FUNCTION_SWITCH_2               SL
#define SWITCHES_GPIO_REG_L
#define SWITCHES_GPIO_PIN_L             PCA95XX_PIN_1
#define SWITCHES_L_CFS_IDX              1
#define FUNCTION_SWITCH_3               SM
#define SWITCHES_GPIO_REG_M
#define SWITCHES_GPIO_PIN_M             PCA95XX_PIN_2
#define SWITCHES_M_CFS_IDX              2
#define FUNCTION_SWITCH_4               SN
#define SWITCHES_GPIO_REG_N
#define SWITCHES_GPIO_PIN_N             PCA95XX_PIN_3
#define SWITCHES_N_CFS_IDX              3
#define FUNCTION_SWITCH_5               SO
#define SWITCHES_GPIO_REG_O
#define SWITCHES_GPIO_PIN_O             PCA95XX_PIN_4
#define SWITCHES_O_CFS_IDX              4
#define FUNCTION_SWITCH_6               SP
#define SWITCHES_GPIO_REG_P
#define SWITCHES_GPIO_PIN_P             PCA95XX_PIN_5
#define SWITCHES_P_CFS_IDX              5

// Direct switches

// Key 1
#define STORAGE_SWITCH_G
#define HARDWARE_SWITCH_G
#define SWITCHES_G_2POS
#define SWITCHES_GPIO_REG_G
#define SWITCHES_GPIO_PIN_G             PCA95XX_PIN_6

// Key 2
#define STORAGE_SWITCH_H
#define HARDWARE_SWITCH_H
#define SWITCHES_H_2POS
#define SWITCHES_GPIO_REG_H
#define SWITCHES_GPIO_PIN_H             PCA95XX_PIN_7


// Key 3
#define STORAGE_SWITCH_I
#define HARDWARE_SWITCH_I
#define SWITCHES_GPIO_REG_I           GPIOI
#define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_3  // PI.03

// Key 4
#define STORAGE_SWITCH_J
#define HARDWARE_SWITCH_J
#define SWITCHES_GPIO_REG_J           GPIOI
#define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_11 // PI.11


// ADC
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_0      // PA.00
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_1      // PA.01
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_2      // PA.02
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_3      // PA.03

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_6      // PA.06 VRA
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 VRB
#define ADC_GPIO_PIN_SLIDER1            LL_GPIO_PIN_0      // PB.00 VRC/LS
#define ADC_GPIO_PIN_SLIDER2            LL_GPIO_PIN_7      // PA.07 VRD/RS

// If serial gimbals are used, we can reuse the channels
#define ADC_GPIO_PIN_EXT1               LL_GPIO_PIN_0      // PA.00 EXT1
#define ADC_GPIO_PIN_EXT2               LL_GPIO_PIN_1      // PA.01 EXT2
#define ADC_GPIO_PIN_EXT3               LL_GPIO_PIN_2      // PA.02 EXT3
#define ADC_GPIO_PIN_EXT4               LL_GPIO_PIN_3      // PA.03 EXT4

#define ADC_GPIO_PIN_SWA                LL_GPIO_PIN_2      // PC.02_C
#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_3      // PC.03_C
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_9      // PF.09
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_7      // PF.07
#define ADC_GPIO_PIN_SWE                LL_GPIO_PIN_8      // PF.08
#define ADC_GPIO_PIN_SWF                LL_GPIO_PIN_6      // PF.06

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_STICK_LH | \
   ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)

#define ADC_GPIOB_PINS (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER1)

#define ADC_GPIOC_PINS (ADC_GPIO_PIN_SWA | ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_BATT)

#define ADC_GPIOF_PINS \
  (ADC_GPIO_PIN_SWC | ADC_GPIO_PIN_SWD | ADC_GPIO_PIN_SWE | ADC_GPIO_PIN_SWF)

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_17    // ADC12_INP17

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_SLIDER1             LL_ADC_CHANNEL_9    // ADC12_INP9
#define ADC_CHANNEL_SLIDER2             LL_ADC_CHANNEL_7    // ADC12_INP7

// Ext inputs: analog gimbal channel re-use
#define ADC_CHANNEL_EXT1                LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_EXT2                LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_EXT3                LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_EXT4                LL_ADC_CHANNEL_17    // ADC12_INP17

// Analog switches
#define ADC_CHANNEL_SWA                 LL_ADC_CHANNEL_0    // ADC3_INP0
#define ADC_CHANNEL_SWB                 LL_ADC_CHANNEL_1    // ADC3_INP1
#define ADC_CHANNEL_SWC                 LL_ADC_CHANNEL_2    // ADC3_INP2
#define ADC_CHANNEL_SWD                 LL_ADC_CHANNEL_3    // ADC3_INP3
#define ADC_CHANNEL_SWE                 LL_ADC_CHANNEL_8    // ADC3_INP8
#define ADC_CHANNEL_SWF                 LL_ADC_CHANNEL_7    // ADC3_INP7

#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_11   // ADC123_INP11
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC12_IN16

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_64CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                                                \
  {                                                                     \
    ADC_CHANNEL_SWA, ADC_CHANNEL_SWB, ADC_CHANNEL_SWC, ADC_CHANNEL_SWD, \
        ADC_CHANNEL_SWE, ADC_CHANNEL_SWF, ADC_CHANNEL_BATT              \
  }

#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_64CYCLES_5

#define ADC_VREF_PREC2                  329

#define ADC_DIRECTION {       \
    0,0,0,0, /* gimbals */    \
    0,0,     /* pots */       \
    0,0,     /* sliders */    \
    0,	     /* vbat */       \
    0,       /* rtc_bat */    \
    0,       /* SWA */        \
    0,       /* SWB */        \
    0,       /* SWC */        \
    0,       /* SWD */        \
    0,       /* SWE */        \
    0        /* SWF */        \
  }

#define USE_EXTI9_5_IRQ // used for I2C port extender interrupt
#define EXTI9_5_IRQ_Priority 5

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOB, 3)  // PE.03
#define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 8)  // PE.04

// Chargers (USB and wireless)
#define UCHARGER_GPIO               GPIO_PIN(GPIOC, 0) // PC.00
#define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOD, 4) // PD.04

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

// Serial Port (DEBUG)
// We will temporarily used the PPM and the HEARTBEAT PINS
// #define AUX_SERIAL_RCC_AHB1Periph       (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE)
// #define AUX_SERIAL_RCC_APB1Periph       0
// #define AUX_SERIAL_RCC_APB2Periph       RCC_APB2Periph_USART6
// #define AUX_SERIAL_GPIO                 GPIOC
// #define AUX_SERIAL_GPIO_PIN_TX          GPIO_Pin_6  // PC.06
// #define AUX_SERIAL_GPIO_PIN_RX          GPIO_Pin_7  // PC.07
// #define AUX_SERIAL_GPIO_PinSource_TX    GPIO_PinSource6
// #define AUX_SERIAL_GPIO_PinSource_RX    GPIO_PinSource7
// #define AUX_SERIAL_GPIO_AF              GPIO_AF_USART6
// #define AUX_SERIAL_USART                USART6
// #define AUX_SERIAL_USART_IRQHandler     USART6_IRQHandler
// #define AUX_SERIAL_USART_IRQn           USART6_IRQn
// #define AUX_SERIAL_TX_INVERT_GPIO       GPIOE
// #define AUX_SERIAL_TX_INVERT_GPIO_PIN   GPIO_Pin_3  // PE.03
// #define AUX_SERIAL_RX_INVERT_GPIO       GPIOI
// #define AUX_SERIAL_RX_INVERT_GPIO_PIN   GPIO_Pin_15 // PI.15

//used in BOOTLOADER
// #define SERIAL_RCC_AHB1Periph 0
// #define SERIAL_RCC_APB1Periph 0
// #define AUX2_SERIAL_RCC_AHB1Periph 0
// #define AUX2_SERIAL_RCC_APB1Periph 0
// #define AUX2_SERIAL_RCC_APB2Periph 0
// #define KEYS_BACKLIGHT_RCC_AHB1Periph 0

// Telemetry
 #define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOB, 13)
 #define TELEMETRY_RX_GPIO               GPIO_UNDEF //GPIO_PIN(GPIOB, 12)
 #define TELEMETRY_USART                 UART5
 #define TELEMETRY_USART_IRQn            UART5_IRQn
 #define TELEMETRY_DMA                   DMA1
 #define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_7
 #define TELEMETRY_DMA_Channel_TX        LL_DMAMUX1_REQ_UART5_TX
 #define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
 #define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
 #define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_3
 #define TELEMETRY_DMA_Channel_RX        LL_DMAMUX1_REQ_UART5_RX
 #define TELEMETRY_USART_IRQHandler      UART5_IRQHandler

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
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOC, 0)  // PC.00
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#define LCD_SPI_CS_GPIO                 GPIOH
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_6  // PH.06
#define LCD_SPI_GPIO                    GPIOE
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_4  // PE.04
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_3  // PE.03
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOC, 06) // TIM3_CH1
#define BACKLIGHT_TIMER                 TIM3
#define BACKLIGHT_GPIO_AF               GPIO_AF2
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// QSPI Flash
#define QSPI_MAX_FREQ                   80000000U // 80 MHz
#define QSPI_CLK_GPIO                   GPIO_PIN(GPIOB, 2)
#define QSPI_CLK_GPIO_AF                GPIO_AF9
#define QSPI_CS_GPIO                    GPIO_PIN(GPIOB, 6)
#define QSPI_CS_GPIO_AF                 GPIO_AF10
#define QSPI_MISO_GPIO                  GPIO_PIN(GPIOD, 12)
#define QSPI_MISO_GPIO_AF               GPIO_AF9
#define QSPI_MOSI_GPIO                  GPIO_PIN(GPIOD, 11)
#define QSPI_MOSI_GPIO_AF               GPIO_AF9
#define QSPI_WP_GPIO                    GPIO_PIN(GPIOE, 2)
#define QSPI_WP_GPIO_AF                 GPIO_AF9
#define QSPI_HOLD_GPIO                  GPIO_PIN(GPIOD, 13)
#define QSPI_HOLD_GPIO_AF               GPIO_AF9
#define QSPI_FLASH_SIZE                 0x800000

// SPI NOR Flash
#define FLASH_SPI                      SPI6
#define FLASH_SPI_CS_GPIO              GPIOG
#define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_6  // PG.06
#define FLASH_SPI_GPIO                 GPIOG
#define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_13 // PG.13
#define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_12 // PG.12
#define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_14 // PG.14
// #define FLASH_SPI_DMA                  DMA2
// #define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_1
// #define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_5
// #define FLASH_SPI_DMA_TX_IRQn          DMA2_Stream5_IRQn
// #define FLASH_SPI_DMA_TX_IRQHandler    DMA2_Stream5_IRQHandler
// #define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_6
// #define FLASH_SPI_DMA_RX_IRQn          DMA2_Stream6_IRQn
// #define FLASH_SPI_DMA_RX_IRQHandler    DMA2_Stream6_IRQHandler

#define SD_PRESENT_GPIO                GPIO_PIN(GPIOD, 3) // PD.03
#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_INIT_CLK_DIV           SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(20000000)
#define STORAGE_USE_SDIO


// Audio
#define AUDIO_XDCS_GPIO               GPIO_PIN(GPIOG, 12) // PG.12
#define AUDIO_CS_GPIO                 GPIO_PIN(GPIOG, 10) // PG.10
#define AUDIO_DREQ_GPIO               GPIO_PIN(GPIOG, 13) // PG.13
#define AUDIO_SPI                     SPI1
#define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
#define AUDIO_SPI_SCK_GPIO            GPIO_PIN(GPIOA, 5)  // PA.05
#define AUDIO_SPI_MISO_GPIO           GPIO_PIN(GPIOG, 9)  // PG.09
#define AUDIO_SPI_MOSI_GPIO           GPIO_PIN(GPIOD, 7)  // PD.07
#define AUDIO_SPI_DMA                 DMA1
#define AUDIO_SPI_DMA_REQ             LL_DMAMUX1_REQ_SPI1_TX
#define AUDIO_SPI_DMA_STREAM          LL_DMA_STREAM_1
#define AUDIO_MUTE_GPIO               0
#define AUDIO_UNMUTE_DELAY            180  // ms
#define AUDIO_MUTE_DELAY              200  // ms
#define INVERTED_MUTE_PIN

// I2C Bus
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 7)  // PB.07
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

#define I2C_B2                          I2C3
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOH, 8)  // PH.08
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOH, 7)  // PH.07
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000

// Haptic: TIM3_CH2
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOB, 5) // PB.05
#define HAPTIC_GPIO_TIMER               TIM3
#define HAPTIC_GPIO_AF                  GPIO_AF2
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 USART2

#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOD, 6)  // PD.06
#define FLYSKY_HALL_SERIAL_USART_IRQn            USART2_IRQn

#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
#define FLYSKY_HALL_DMA_Channel                  LL_DMAMUX1_REQ_USART2_RX

// LED Strip
#define LED_STRIP_LENGTH                  24
#define BLING_LED_STRIP_START             0
#define BLING_LED_STRIP_LENGTH            12
#define CFS_LED_STRIP_START               12
#define CFS_LED_STRIP_LENGTH              12
#define CFS_LEDS_PER_SWITCH               2
#define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 15)  // PA.15 / TIM2_CH1
#define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1         // TIM1/2/16/17
#define LED_STRIP_TIMER                   TIM2
#define LED_STRIP_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH1
#define LED_STRIP_TIMER_DMA               DMA1
#define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMAMUX1_REQ_TIM2_UP
#define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_0
#define LED_STRIP_TIMER_DMA_IRQn          DMA1_Stream0_IRQn
#define LED_STRIP_TIMER_DMA_IRQHandler    DMA1_Stream0_IRQHandler
#define LED_STRIP_REFRESH_PERIOD          50 //ms
#define STATUS_LEDS
#define LED_CHARGING_START                12
#define LED_CHARGING_END                  23


// Internal Module
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 11) // PB.11
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 10) // PB.10
#define INTMODULE_USART                 USART3
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART3_IRQn
#define INTMODULE_USART_IRQHandler      USART3_IRQHandler
//#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_5
//#define INTMODULE_RX_DMA                DMA1
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_3
#define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 9)  // PB.09
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOI, 9)  // PI.09
#define EXTMODULE_TIMER                 TIM4
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH4
#define EXTMODULE_TIMER_IRQn            TIM4_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM4_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_2 // TIM4_CH4

//USART
#define EXTMODULE_USART                    UART4
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOI, 9)
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOB, 9)
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6

#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5

#define EXTMODULE_USART_IRQHandler         UART4_IRQHandler
#define EXTMODULE_USART_IRQn               UART4_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM4_UP
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_6
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream6_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream6_IRQHandler

// Trainer Port
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOD)

#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOE, 5) // PE.05
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOE, 6) // PE.06
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

#define TRAINER_TIMER                   TIM15
#define TRAINER_TIMER_IRQn              TIM15_IRQn
#define TRAINER_TIMER_IRQHandler        TIM15_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_4
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_I2C_CLK_RATE              100000

#define IMU_I2C_BUS                     I2C_Bus_1
#define IMU_I2C_ADDRESS                 0x6A

//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION
// Rotary Encoder
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOG
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_2 // PG.02
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_3 // PG.03
#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 2) & 0x03)
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_2
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_3
#if !defined(USE_EXTI2_IRQ)
  #define USE_EXTI2_IRQ
  #define EXTI2_IRQ_Priority 5
#endif
#if !defined(USE_EXTI3_IRQ)
  #define USE_EXTI3_IRQ
  #define EXTI3_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_SYSCFG_EXTI_PORTG
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE2
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE3
#define ROTARY_ENCODER_TIMER            TIM17
#define ROTARY_ENCODER_TIMER_IRQn       TIM17_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM17_IRQHandler

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_BRK_TIM12_IRQHandler

#define LCD_W                           480
#define LCD_H                           320

#define LCD_PHYS_W                      LCD_H
#define LCD_PHYS_H                      LCD_W

#define LCD_DEPTH                       16
