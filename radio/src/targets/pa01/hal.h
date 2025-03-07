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

#ifndef _HAL_H_
#define _HAL_H_

#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

/* Timers Allocation:
 * TIM3  = Haptic / Backlight
 * TIM15 = Trainer
 *
 * TIM14 = 5 ms counter
 */

/* DMA Allocation:
*/

// Keys
#define KEYS_GPIO_REG_ENTER
#define KEYS_GPIO_PIN_ENTER
#define KEYS_GPIO_ENTER               GPIO_PIN(GPIOG, 13) // PG.13

// Keys in bsp matrix
#define KEYS_GPIO_REG_PAGEUP
#define KEYS_GPIO_PIN_PAGEUP
#define KEYS_GPIO_REG_PAGEDN
#define KEYS_GPIO_PIN_PAGEDN
#define KEYS_GPIO_REG_SYS
#define KEYS_GPIO_PIN_SYS
#define KEYS_GPIO_REG_EXIT
#define KEYS_GPIO_PIN_EXIT

// Trims in bsp matrix
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

#define TRIMS_GPIO_REG_TR1U             GPIOH->IDR
#define TRIMS_GPIO_PIN_TR1U             LL_GPIO_PIN_8  // PH.08
#define TRIMS_GPIO_REG_TR1D             GPIOH->IDR
#define TRIMS_GPIO_PIN_TR1D             LL_GPIO_PIN_9  // PH.09
#define TRIMS_GPIO_REG_TR2U             GPIOH->IDR
#define TRIMS_GPIO_PIN_TR2U             LL_GPIO_PIN_10 // PH.10
#define TRIMS_GPIO_REG_TR2D             GPIOH->IDR
#define TRIMS_GPIO_PIN_TR2D             LL_GPIO_PIN_11 // PH.11

// active 4x4 column/row based key-matrix to support up to 16 buttons with only 8 GPIOs
#define TRIMS_GPIO_OUT1                 GPIOG
#define TRIMS_GPIO_OUT1_PIN             LL_GPIO_PIN_2  // PG.02
//#define TRIMS_GPIO_OUT2                 GPIOG
//#define TRIMS_GPIO_OUT2_PIN             LL_GPIO_PIN_10 // PG.10
//#define TRIMS_GPIO_OUT3                 GPIOG
//#define TRIMS_GPIO_OUT3_PIN             LL_GPIO_PIN_11 // PG.11
// OUT4 routed on MCU PCB, but not attached to any physical buttons, free to use for extensions
#define TRIMS_GPIO_OUT4                 GPIOH
#define TRIMS_GPIO_OUT4_PIN             LL_GPIO_PIN_7  // PH.07

#define TRIMS_GPIO_REG_IN1
#define TRIMS_GPIO_PIN_IN1
#define TRIMS_GPIO_REG_IN2
#define TRIMS_GPIO_PIN_IN2
#define TRIMS_GPIO_REG_IN3
#define TRIMS_GPIO_PIN_IN3
#define TRIMS_GPIO_REG_IN4
#define TRIMS_GPIO_PIN_IN4

// function switches
#define FUNCTION_SWITCH_1               I2C
#define FUNCTION_SWITCH_2               I2C
#define FUNCTION_SWITCH_3               I2C
#define FUNCTION_SWITCH_4               I2C
#define FUNCTION_SWITCH_5               I2C
#define FUNCTION_SWITCH_6               I2C

// Direct switches
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

#define KEYS_GPIOB_PINS (LL_GPIO_PIN_15)

// PC8 allocated to SDIO D0, is not required to sample SWA !
#define KEYS_GPIOC_PINS (LL_GPIO_PIN_13)

#define KEYS_GPIOD_PINS ()

#define KEYS_GPIOH_PINS							\
  (LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11)

#define KEYS_GPIOJ_PINS (LL_GPIO_PIN_12)

#define KEYS_OUT_GPIOG_PINS (LL_GPIO_PIN_2 )

#define KEYS_OUT_GPIOH_PINS (LL_GPIO_PIN_7)

// ADC
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_0      // PA.00
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_1      // PA.01
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_2      // PA.02
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_3      // PA.03

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_6      // PA.06 VRA
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 VRB
#define ADC_GPIO_PIN_SLIDER1            LL_GPIO_PIN_7      // PA.07 VRD/RS
#define ADC_GPIO_PIN_SLIDER2            LL_GPIO_PIN_0      // PB.00 VRC/LS

// If serial gimbals are used, we can reuse the channels
#define ADC_GPIO_PIN_EXT1               LL_GPIO_PIN_0      // PA.00 EXT1
#define ADC_GPIO_PIN_EXT2               LL_GPIO_PIN_1      // PA.01 EXT2
#define ADC_GPIO_PIN_EXT3               LL_GPIO_PIN_2      // PA.02 EXT3
#define ADC_GPIO_PIN_EXT4               LL_GPIO_PIN_3      // PA.03 EXT4

#define ADC_GPIO_PIN_SWA                LL_GPIO_PIN_2      // PA.02
#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_5      // PC.05
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_9      // PF.09
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_7      // PF.07
#define ADC_GPIO_PIN_SWE                LL_GPIO_PIN_8      // PF.08
#define ADC_GPIO_PIN_SWF                LL_GPIO_PIN_6      // PF.06

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_SWA|ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_STICK_LH | \
   ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)

#define ADC_GPIOB_PINS (ADC_GPIO_PIN_POT2 )

#define ADC_GPIOC_PINS ( ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_BATT)

#define ADC_GPIOF_PINS \
  (ADC_GPIO_PIN_SWC | ADC_GPIO_PIN_SWD | ADC_GPIO_PIN_SWE | ADC_GPIO_PIN_SWF)

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_17    // ADC12_INP17

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_SLIDER1             LL_ADC_CHANNEL_7    // ADC12_INP7
#define ADC_CHANNEL_SLIDER2             LL_ADC_CHANNEL_9    // ADC12_INP9

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

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOC, 13)
#define PWR_ON_GPIO                 GPIO_PIN(GPIOE,  3)

// Chargers (USB and wireless)
#define UCHARGER_GPIO               GPIO_PIN(GPIOG, 14)

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
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOB, 0)  // PB.00
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#define LCD_SPI                         SPI1
#define LCD_SPI_DMA                     DMA1
#define LCD_SPI_TX_DMA                  LL_DMAMUX1_REQ_SPI1_TX
#define LCD_SPI_RX_DMA                  LL_DMAMUX1_REQ_SPI1_RX
#define LCD_SPI_TX_DMA_STREAM           LL_DMA_STREAM_4
#define LCD_SPI_RX_DMA_STREAM           LL_DMA_STREAM_5
#define LCD_FMARK                       GPIO_PIN(GPIOB,  7)
#define LCD_NRST                        GPIO_PIN(GPIOD,  7)
#define LCD_SPI_RS                      GPIO_PIN(GPIOG, 12)
#define LCD_SPI_CS                      GPIO_PIN(GPIOG, 10)
#define LCD_SPI_CLK                     GPIO_PIN(GPIOG, 11)
#define LCD_SPI_MOSI                    GPIO_PIN(GPIOB,  5)
#define LCD_SPI_MISO                    GPIO_PIN(GPIOG,  9)
#define LCD_SPI_BAUD                    100000000

// Backlight
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOB, 03) // TIM2_CH2
#define BACKLIGHT_TIMER                 TIM2
#define BACKLIGHT_GPIO_AF               GPIO_AF1
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
//#define FLASH_SPI                      SPI6
//#define FLASH_SPI_CS_GPIO              GPIOG
//#define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_6  // PG.06
//#define FLASH_SPI_GPIO                 GPIOG
//#define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_13 // PG.13
//#define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_12 // PG.12
//#define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_14 // PG.14
// #define FLASH_SPI_DMA                  DMA2
// #define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_1
// #define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_5
// #define FLASH_SPI_DMA_TX_IRQn          DMA2_Stream5_IRQn
// #define FLASH_SPI_DMA_TX_IRQHandler    DMA2_Stream5_IRQHandler
// #define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_6
// #define FLASH_SPI_DMA_RX_IRQn          DMA2_Stream6_IRQn
// #define FLASH_SPI_DMA_RX_IRQHandler    DMA2_Stream6_IRQHandler

//#define SD_PRESENT_GPIO                GPIO_PIN(GPIOD, 3) // PD.03
#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            ((240000000 + (2*fq) -1) / (2 *fq)) /* make sure the resulting frequency is not higher than expected */
#define SD_SDIO_INIT_CLK_DIV           SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(20000000)
#define SD_SDIO_PIN_D0                 GPIO_PIN(GPIOC,  8)
#define SD_SDIO_AF_D0                  GPIO_AF12
#define SD_SDIO_PIN_D1                 GPIO_PIN(GPIOC,  9)
#define SD_SDIO_AF_D1                  GPIO_AF12
#define SD_SDIO_PIN_D2                 GPIO_PIN(GPIOC, 10)
#define SD_SDIO_AF_D2                  GPIO_AF12
#define SD_SDIO_PIN_D3                 GPIO_PIN(GPIOC, 11)
#define SD_SDIO_AF_D3                  GPIO_AF12
#define SD_SDIO_PIN_CLK                GPIO_PIN(GPIOC, 12)
#define SD_SDIO_AF_CLK                 GPIO_AF12
#define SD_SDIO_PIN_CMD                GPIO_PIN(GPIOD,  2)
#define SD_SDIO_AF_CMD                 GPIO_AF12
#define STORAGE_USE_SDIO


// Audio
#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOA, 8) // PA.08
#define AUDIO_OUTPUT_GPIO             GPIO_PIN(GPIOA, 4) // PA.04
#define AUDIO_DMA_Stream              DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn         DMA1_Stream5_IRQn
#define AUDIO_TIM_IRQn                TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler          TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQHandler   DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                   TIM6
#define AUDIO_DMA                     DMA1
#define AUDIO_UNMUTE_DELAY            180  // ms
#define AUDIO_MUTE_DELAY              200  // ms
#define INVERTED_MUTE_PIN

// I2C Bus
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 9)  // PB.09
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

#define I2C_B2                          I2C2
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOB, 10)  // PB.10
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOB, 11)  // PB.11
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000


// Haptic: TIM3_CH2
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOA, 15) // PA.15
#define HAPTIC_GPIO_TIMER               TIM4
#define HAPTIC_GPIO_AF                  GPIO_AF1
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 USART2

// #define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_UNDEF
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOD, 6)  // PD.06
#define FLYSKY_HALL_SERIAL_USART_IRQn            USART2_IRQn

#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Channel                  LL_DMAMUX1_REQ_USART2_RX
/*
// LED Strip
#define LED_STRIP_LENGTH                  24
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
*/

// Internal Module
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 15) // PB.15
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 14) // PB.14
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
// #define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream3_IRQn
// #define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream_IRQHandler

// #define INTMODULE_TIMER                 TIM3
// #define INTMODULE_TIMER_IRQn            TIM3_IRQn
// #define INTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
// #define INTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)  // PC.07
#define EXTMODULE_TX_GPIO_AF            LL_GPIO_AF_3 // TIM8_CH1
#define EXTMODULE_TIMER                 TIM8
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
#define EXTMODULE_TIMER_IRQn            TIM8_UP_TIM13_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM8_UP_TIM13_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_3


//USART
#define EXTMODULE_USART                    UART4
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOA, 0)
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOA, 1)
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6

#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_4

#define EXTMODULE_USART_IRQHandler         UART4_IRQHandler
#define EXTMODULE_USART_IRQn               UART4_IRQn

//TIMER
// TODO
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM8_CH1
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_1
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream1_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream1_IRQHandler

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


//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION
// Rotary Encoder
//#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOD
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_4 // PD.04
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_3 // PD.03
#define ROTARY_ENCODER_POSITION()       (((ROTARY_ENCODER_GPIO->IDR >> 2) & 0x02)|((ROTARY_ENCODER_GPIO->IDR >> 4) & 0x01))
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_4
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_3
#if !defined(USE_EXTI4_IRQ)
  #define USE_EXTI4_IRQ
  #define EXTI4_IRQ_Priority 5
#endif
#if !defined(USE_EXTI3_IRQ)
  #define USE_EXTI3_IRQ
  #define EXTI3_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_SYSCFG_EXTI_PORTD
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE4
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

#define LANDSCAPE_LCD true
#define PORTRAIT_LCD false

#define LCD_W                           320
#define LCD_H                           240

#define LCD_PHYS_W                      LCD_W
#define LCD_PHYS_H                      LCD_H

#define LCD_DEPTH                       16

#endif // _HAL_H_
