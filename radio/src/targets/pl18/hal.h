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

#define CPU_FREQ                168000000

// HSI is at 168Mhz (over-drive is not enabled!)
#define PERI1_FREQUENCY                 42000000
#define PERI2_FREQUENCY                 84000000
#define TIMER_MULT_APB1                 2
#define TIMER_MULT_APB2                 2

/* Timers Allocation:
 * TIM1 = Haptic
 * TIM4 = Trainer
 * TIM6 = Audio
 * TIM7 = 2 MHz counter
 *
 *
 * TIM14 = 5 ms counter
 */

/* DMA Allocation:
   DMA/Stream/Channel
   1/5/7 DAC/Audio
   2/4/0 ADC1
   2/0/2 ADC3
   2/3/4 SDIO
*/

#if defined(RADIO_NV14_FAMILY)
  // Monitor pin
  #define VBUS_MONITOR_GPIO               GPIO_PIN(GPIOJ, 14) // PJ.14
  // ADC
  #define ADC_VREF_PREC2                  330
#elif defined(RADIO_NB4P)
  #define ADC_GPIO_RAW1                 GPIOC
  #define ADC_VREF_PREC2                  330
#endif

// Trims
#if !defined(RADIO_PL18U)
  #define TRIMS_GPIO_REG_TR1U             GPIOH->IDR
  #define TRIMS_GPIO_PIN_TR1U             LL_GPIO_PIN_8  // PH.08
  #define TRIMS_GPIO_REG_TR1D             GPIOH->IDR
  #define TRIMS_GPIO_PIN_TR1D             LL_GPIO_PIN_9  // PH.09
  #define TRIMS_GPIO_REG_TR2U             GPIOH->IDR
  #define TRIMS_GPIO_PIN_TR2U             LL_GPIO_PIN_10 // PH.10
  #define TRIMS_GPIO_REG_TR2D             GPIOH->IDR
  #define TRIMS_GPIO_PIN_TR2D             LL_GPIO_PIN_11 // PH.11
#endif

// active 4x4 column/row based key-matrix to support up to 16 buttons with only 8 GPIOs
#if defined(RADIO_PL18U)
  #define TRIMS_GPIO_OUT1                 GPIOI
  #define TRIMS_GPIO_OUT1_PIN             LL_GPIO_PIN_2  // PI.02
#else
  #define TRIMS_GPIO_OUT1                 GPIOG
  #define TRIMS_GPIO_OUT1_PIN             LL_GPIO_PIN_2  // PG.02
#endif

#define TRIMS_GPIO_OUT2                 GPIOG
#define TRIMS_GPIO_OUT2_PIN             LL_GPIO_PIN_10 // PG.10
#define TRIMS_GPIO_OUT3                 GPIOG
#define TRIMS_GPIO_OUT3_PIN             LL_GPIO_PIN_11 // PG.11

#if defined(RADIO_PL18U)
  #define TRIMS_GPIO_OUT4                 GPIOH
  #define TRIMS_GPIO_OUT4_PIN           LL_GPIO_PIN_10  // PH.10
#endif

// OUT4 routed on MCU PCB, but not attached to any physical buttons, free to use for extensions
//#define TRIMS_GPIO_OUT4                 GPIOH
//#define TRIMS_GPIO_OUT4_PIN             LL_GPIO_PIN_7  // PH.07

#define TRIMS_GPIO_REG_IN1              GPIOB->IDR
#define TRIMS_GPIO_PIN_IN1              LL_GPIO_PIN_15 // PB.15
#define TRIMS_GPIO_REG_IN2              GPIOC->IDR
#define TRIMS_GPIO_PIN_IN2              LL_GPIO_PIN_13 // PC.13
#define TRIMS_GPIO_REG_IN3              GPIOD->IDR
#define TRIMS_GPIO_PIN_IN3              LL_GPIO_PIN_7  // PD.07
#define TRIMS_GPIO_REG_IN4              GPIOJ->IDR
#define TRIMS_GPIO_PIN_IN4              LL_GPIO_PIN_12 // PJ.12

// Index of all trims
#define KEYS_GPIOB_PINS (LL_GPIO_PIN_15)

// PC8 allocated to SDIO D0, is not required to sample SWA !
#define KEYS_GPIOC_PINS (LL_GPIO_PIN_13)

#define KEYS_GPIOD_PINS (LL_GPIO_PIN_7)

#if !defined(RADIO_PL18U)
  #define KEYS_GPIOH_PINS							\
    (LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11)
#endif

#define KEYS_GPIOJ_PINS (LL_GPIO_PIN_12)

#if defined(RADIO_PL18U)
  #define KEYS_OUT_GPIOG_PINS (LL_GPIO_PIN_10 | LL_GPIO_PIN_11)
  #define KEYS_OUT_GPIOH_PINS (LL_GPIO_PIN_10)
  #define KEYS_OUT_GPIOI_PINS (LL_GPIO_PIN_2)
#else
  #define KEYS_OUT_GPIOG_PINS (LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11)
  #define KEYS_OUT_GPIOH_PINS (LL_GPIO_PIN_7)
#endif

// Monitor pin
// #define MONITOR_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOJ)
// #define VBUS_MONITOR_GPIO               (GPIOJ)
// #define VBUS_MONITOR_PIN                (LL_GPIO_PIN_14)

// ADC
#define ADC_VREF_PREC2                  330

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOI, 11)   // PI.11
#define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 14)   // PI.14

// Chargers (USB and wireless)
#define UCHARGER_GPIO               GPIO_PIN(GPIOB, 14)   // PB.14 input
#define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOB, 13)   // PB.13 input

#if defined(RADIO_PL18) || defined(RADIO_PL18EV) || defined(RADIO_PL18U)
  #define UCHARGER_EN_GPIO          GPIO_PIN(GPIOG, 3)    // PG.03 output
#elif defined(RADIO_NV14_FAMILY)
  #define UCHARGER_EN_GPIO          GPIO_PIN(GPIOH, 11)   // PH.11 output
//  #define UCHARGER_EN_GPIO_INV
#endif

#if defined (WIRELESS_CHARGER)
  #define WCHARGER_GPIO               GPIO_PIN(GPIOI, 9)    // PI.09 input
  #if defined(RADIO_PL18U)
    #define WCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOB, 13) // PB.13 input
  #else
    #define WCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOI, 10) // PI.10 input
  #endif
  #define WCHARGER_EN_GPIO            GPIO_PIN(GPIOH, 4)    // PH.04 output
  #if defined(RADIO_PL18) || defined(RADIO_PL18EV)
    #define WCHARGER_I_CONTROL_GPIO     GPIO_PIN(GPIOH, 13) // PH.13 output
  #endif
#endif // defined(WIRELESS_CHARGER)

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
#define SPORT_MAX_BAUDRATE              400000
#define SPORT_UPDATE_RCC_AHB1Periph     0
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

// Serial Port (DEBUG)
// We will temporarily used the PPM and the HEARTBEAT PINS
#define AUX_SERIAL_RCC_AHB1Periph       (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE)
#define AUX_SERIAL_RCC_APB1Periph       0
#define AUX_SERIAL_RCC_APB2Periph       RCC_APB2Periph_USART6
#define AUX_SERIAL_GPIO                 GPIOC
#define AUX_SERIAL_GPIO_PIN_TX          GPIO_Pin_6  // PC.06
#define AUX_SERIAL_GPIO_PIN_RX          GPIO_Pin_7  // PC.07
#define AUX_SERIAL_GPIO_PinSource_TX    GPIO_PinSource6
#define AUX_SERIAL_GPIO_PinSource_RX    GPIO_PinSource7
#define AUX_SERIAL_GPIO_AF              GPIO_AF_USART6
#define AUX_SERIAL_USART                USART6
#define AUX_SERIAL_USART_IRQHandler     USART6_IRQHandler
#define AUX_SERIAL_USART_IRQn           USART6_IRQn
#define AUX_SERIAL_TX_INVERT_GPIO       GPIOE
#define AUX_SERIAL_TX_INVERT_GPIO_PIN   GPIO_Pin_3  // PE.03
#define AUX_SERIAL_RX_INVERT_GPIO       GPIOI
#define AUX_SERIAL_RX_INVERT_GPIO_PIN   GPIO_Pin_15 // PI.15

//used in BOOTLOADER
#define SERIAL_RCC_AHB1Periph 0
#define SERIAL_RCC_APB1Periph 0
#define AUX2_SERIAL_RCC_AHB1Periph 0
#define AUX2_SERIAL_RCC_APB1Periph 0
#define AUX2_SERIAL_RCC_APB2Periph 0
#define KEYS_BACKLIGHT_RCC_AHB1Periph 0

// Telemetry
#define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOJ | RCC_AHB1Periph_DMA1)
#define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
#define TELEMETRY_RX_REV_GPIO           GPIO_PIN(GPIOJ, 8)  // PJ.08
#define TELEMETRY_TX_REV_GPIO           GPIO_PIN(GPIOJ, 7)  // PJ.07
#define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOJ, 13) // PJ.13
#define TELEMETRY_SET_INPUT             1
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
#define TELEMETRY_RX_GPIO               GPIO_PIN(GPIOD, 6)  // PD.06
#define TELEMETRY_USART                 USART2
#define TELEMETRY_USART_IRQn            USART2_IRQn
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_6
#define TELEMETRY_DMA_Channel_TX        LL_DMA_CHANNEL_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
// #define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_5
// #define TELEMETRY_DMA_Channel_RX        LL_DMA_CHANNEL_4
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler

#define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
#define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
#define TELEMETRY_TX_POL_NORM()         TELEMETRY_REV_GPIO->BSRRH = TELEMETRY_TX_REV_GPIO_PIN
#define TELEMETRY_TX_POL_INV()          TELEMETRY_REV_GPIO->BSRRL = TELEMETRY_TX_REV_GPIO_PIN
#define TELEMETRY_RX_POL_NORM()         TELEMETRY_REV_GPIO->BSRRH = TELEMETRY_RX_REV_GPIO_PIN
#define TELEMETRY_RX_POL_INV()          TELEMETRY_REV_GPIO->BSRRL = TELEMETRY_RX_REV_GPIO_PIN

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

#if defined(RADIO_NV14_FAMILY) 
  #define USB_GPIO_VBUS                 GPIO_PIN(GPIOA, 9)  // PA.09
  #define USB_SW_GPIO                   GPIO_PIN(GPIOI, 10) // PI.10
#elif defined(RADIO_PL18U)
  #define USB_SW_GPIO                   GPIO_PIN(GPIOI, 5)  // PI.05
#endif

// LCD
#define LCD_NRST_GPIO                   GPIO_PIN(GPIOG, 9)  // PG.09
#define LCD_SPI_CS_GPIO                 GPIO_PIN(GPIOE, 4)  // PE.04
#define LCD_SPI_SCK_GPIO                GPIO_PIN(GPIOE, 2)  // PE.02
#define LCD_SPI_MISO_GPIO               GPIO_PIN(GPIOE, 5)  // PE.05
#define LCD_SPI_MOSI_GPIO               GPIO_PIN(GPIOE, 6)  // PE.06
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
// TODO TIM3, TIM8, TIM14, review the channel in backlight_driver.cpp according to the chosen timer
#define BACKLIGHT_RCC_APB2Periph        0
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOA, 15) // PA.15
#define BACKLIGHT_TIMER                 TIM2
#define BACKLIGHT_GPIO_AF               GPIO_AF1
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

//used in BOOTLOADER
#define SERIAL_RCC_AHB1Periph           0
#define SERIAL_RCC_APB1Periph           0

#if defined(RADIO_NB4P)
// Rotary Encoder
#define ROTARY_ENCODER_GPIO             GPIOH
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_11 // PH.11
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_10 // PH.10
#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 10) & 0x03)
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_11
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_10
#if !defined(USE_EXTI15_10_IRQ)
  #define USE_EXTI15_10_IRQ
  #define EXTI15_10_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_SYSCFG_EXTI_PORTH
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE11
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE10
#define ROTARY_ENCODER_TIMER            TIM13
#define ROTARY_ENCODER_TIMER_IRQn       TIM8_UP_TIM13_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM8_UP_TIM13_IRQHandler
#endif

#if defined(RADIO_NV14_FAMILY) || defined(RADIO_PL18U)
  // SD card
  #if defined(RADIO_PL18U)
    #define SD_PRESENT_GPIO               GPIO_PIN(GPIOH, 11) // PH.11
  #else
    #define SD_PRESENT_GPIO               GPIO_PIN(GPIOH, 10) // PH.10
  #endif
  #define SD_SDIO_DMA                     DMA2
  #define SD_SDIO_DMA_STREAM              DMA2_Stream3
  #define SD_SDIO_DMA_CHANNEL             LL_DMA_CHANNEL_4
  #define SD_SDIO_DMA_IRQn                DMA2_Stream3_IRQn
  #define SD_SDIO_DMA_IRQHANDLER          DMA2_Stream3_IRQHandler
  #define SD_SDIO_CLK_DIV(fq)             ((48000000 / (fq)) - 2)
  #define SD_SDIO_INIT_CLK_DIV            SD_SDIO_CLK_DIV(400000)
  #if defined(RADIO_PL18U)
    #define SD_SDIO_TRANSFER_CLK_DIV      SD_SDIO_CLK_DIV(12000000)
  #else
    #define SD_SDIO_TRANSFER_CLK_DIV      SD_SDIO_CLK_DIV(24000000)
  #endif
#endif

// SPI NOR Flash
#define FLASH_SPI                      SPI6
#define FLASH_SPI_CS_GPIO              GPIO_PIN(GPIOG, 6)  // PG.06
#define FLASH_SPI_SCK_GPIO             GPIO_PIN(GPIOG, 13) // PG.13
#define FLASH_SPI_MISO_GPIO            GPIO_PIN(GPIOG, 12) // PG.12
#define FLASH_SPI_MOSI_GPIO            GPIO_PIN(GPIOG, 14) // PG.14
#define FLASH_SPI_DMA                  DMA2
#define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_1
#define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_5
#define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_6

#if defined(RADIO_NV14_FAMILY) || defined(RADIO_PL18U)
  #define STORAGE_USE_SDIO
#else
  #define STORAGE_USE_SPI_FLASH
#endif

// SDRAM
#define SDRAM_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH)
#define SDRAM_RCC_AHB3Periph            RCC_AHB3Periph_FMC

// Audio
#if defined(RADIO_NV14_FAMILY) || defined(RADIO_PL18U)
  #define AUDIO_XDCS_GPIO               GPIO_PIN(GPIOH, 14) // PH.14
  #define AUDIO_CS_GPIO                 GPIO_PIN(GPIOH, 13) // PH.13
  #define AUDIO_DREQ_GPIO               GPIO_PIN(GPIOH, 15) // PH.15
  #define AUDIO_RST_GPIO                GPIO_PIN(GPIOD, 4)  // PD.04
  #define AUDIO_SPI                     SPI1
  #define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
  #define AUDIO_SPI_SCK_GPIO            GPIO_PIN(GPIOB, 3)  // PB.03
  #define AUDIO_SPI_MISO_GPIO           GPIO_PIN(GPIOB, 4)  // PB.04
  #define AUDIO_SPI_MOSI_GPIO           GPIO_PIN(GPIOB, 5)  // PB.05
#else
  #define AUDIO_OUTPUT_GPIO               GPIO_PIN(GPIOA, 4)  // PA.04
  #define AUDIO_DMA_Stream                DMA1_Stream5
  #define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
  #define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
  #define AUDIO_TIMER                     TIM6
  #define AUDIO_DMA                       DMA1
#endif

#if defined(RADIO_PL18U)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOI, 10)  // PI.10 audio amp control pin
  #define AUDIO_UNMUTE_DELAY            120  // ms
  #define AUDIO_MUTE_DELAY              500  // ms
  #define INVERTED_MUTE_PIN
#elif defined(RADIO_NV14_FAMILY)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOH, 8) // PH.08 audio amp control pin
  #define AUDIO_UNMUTE_DELAY            120  // ms
  #define AUDIO_MUTE_DELAY              500  // ms
  #define INVERTED_MUTE_PIN
#elif defined(RADIO_NB4P)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOH, 9) // PH.09 audio amp control pin
  #define AUDIO_UNMUTE_DELAY            120  // ms
  #define AUDIO_MUTE_DELAY              500  // ms
  #define INVERTED_MUTE_PIN
  #define VOICE_CHIP_EN_GPIO            GPIO_PIN(GPIOI, 5) // PI.05
#endif

// I2C Bus
#define I2C_B1                          I2C1
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 7)  // PB.07
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

// Touch
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOB, 9)   // PB.09
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOB, 12)  // PB.12

#define TOUCH_INT_EXTI_Line             LL_EXTI_LINE_9
#define TOUCH_INT_EXTI_Port             LL_SYSCFG_EXTI_PORTB
#define TOUCH_INT_EXTI_SysCfgLine       LL_SYSCFG_EXTI_LINE9

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI9_5_IRQ)
  #define USE_EXTI9_5_IRQ
  #define EXTI9_5_IRQ_Priority  9
#endif

// Haptic: TIM1_CH1
#if defined(RADIO_NB4P)
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOB, 0) // PB.00
#define HAPTIC_GPIO_TIMER               TIM1
#define HAPTIC_GPIO_AF                  GPIO_AF1
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2NE
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2
#else
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOA, 8) // PA.08
#define HAPTIC_GPIO_TIMER               TIM1
#define HAPTIC_GPIO_AF                  GPIO_AF1
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC1E | TIM_CCER_CC1NE
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR1
#endif

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 UART4
#define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOA, 0)  // PA.00
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOA, 1)  // PA.01
#define FLYSKY_HALL_SERIAL_GPIO_AF               LL_GPIO_AF_8

#define FLYSKY_HALL_RCC_AHB1Periph               RCC_AHB1Periph_DMA1
#define FLYSKY_HALL_RCC_APB1Periph               RCC_APB1Periph_UART4

#define FLYSKY_HALL_SERIAL_USART_IRQHandler      UART4_IRQHandler
#define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4

// LED Strip
#if !defined(RADIO_NV14_FAMILY)
  #define LED_STRIP_LENGTH                  4
  #define BLING_LED_STRIP_START             0
  #define BLING_LED_STRIP_LENGTH            4
  #define LED_STRIP_GPIO                    GPIO_PIN(GPIOH, 12)  // PH.12 / TIM5_CH3
  #define LED_STRIP_GPIO_AF                 LL_GPIO_AF_2    // TIM3/4/5
  #define LED_STRIP_TIMER                   TIM5
  #define LED_STRIP_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH3
  #define LED_STRIP_TIMER_DMA               DMA1
  #define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMA_CHANNEL_6
  #define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_0
  #define LED_STRIP_TIMER_DMA_IRQn          DMA1_Stream0_IRQn
  #define LED_STRIP_TIMER_DMA_IRQHandler    DMA1_Stream0_IRQHandler
  #define LED_STRIP_REFRESH_PERIOD          50 //ms

  #define STATUS_LEDS
#endif

// Internal Module
#if defined(RADIO_PL18) || defined(RADIO_PL18U)
  #if defined(RADIO_PL18U)
    #define INTMODULE_PWR_GPIO            GPIO_PIN(GPIOI, 3)  // PI.03
  #else
    #define INTMODULE_PWR_GPIO            GPIO_PIN(GPIOI, 0)  // PI.00
  #endif
  #define INTMODULE_TX_GPIO               GPIO_PIN(GPIOF, 7) // PF.07
  #define INTMODULE_RX_GPIO               GPIO_PIN(GPIOF, 6) // PF.06
  #define INTMODULE_USART                 UART7
  #define INTMODULE_GPIO_AF               LL_GPIO_AF_8
  #define INTMODULE_USART_IRQn            UART7_IRQn
  #define INTMODULE_USART_IRQHandler      UART7_IRQHandler
  #define INTMODULE_DMA                   DMA1
  #define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
  #define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
  #define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
  #define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_5
  #define INTMODULE_RX_DMA                DMA1
  #define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_3
  #define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5
// #define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream3_IRQn
// #define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream_IRQHandler

  #define INTMODULE_TIMER                 TIM3
  #define INTMODULE_TIMER_IRQn            TIM3_IRQn
  #define INTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
  #define INTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#elif defined(RADIO_NB4P)
  #define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOI, 8)  // PI.08
  #define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 10) // PB.10
  #define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 11) // PB.11
  #define INTMODULE_USART                 USART3
  #define INTMODULE_GPIO_AF               LL_GPIO_AF_7
  #define INTMODULE_USART_IRQn            USART3_IRQn
  #define INTMODULE_USART_IRQHandler      USART3_IRQHandler
  #define INTMODULE_DMA                   DMA1
  #define INTMODULE_DMA_STREAM            LL_DMA_STREAM_3
  #define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream3_IRQn
  #define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
  #define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_4
  #define INTMODULE_RX_DMA                DMA1
  #define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_1
  #define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_4

  #define INTMODULE_TIMER                 TIM3
  #define INTMODULE_TIMER_IRQn            TIM3_IRQn
  #define INTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
  #define INTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#elif defined(RADIO_NV14_FAMILY)
  #define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOH, 9) // PH.09
  #define INTMODULE_TX_GPIO               GPIO_PIN(GPIOF, 7) // PF.07
  #define INTMODULE_RX_GPIO               GPIO_PIN(GPIOF, 6) // PF.06
  #define INTMODULE_USART                 UART7
  #define INTMODULE_USART_IRQn            UART7_IRQn
  #define INTMODULE_DMA                   DMA1
  #define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
  #define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
  #define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_5

  #define INTMODULE_RX_DMA                DMA1
  #define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_3
  #define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5
  // #define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream3_IRQn
  // #define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream_IRQHandler

  #define INTMODULE_TIMER                 TIM3
  #define INTMODULE_TIMER_IRQn            TIM3_IRQn
  #define INTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
  #define INTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#endif

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#if defined(RADIO_PL18U)
  #define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOI, 1) // PI.01
#else
  #define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOD, 11) // PD.11
#endif
#if defined(RADIO_NV14_FAMILY)
  #define EXTMODULE_PWR_FIX_GPIO          GPIO_PIN(GPIOA, 2)  // PA.02
#endif
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)  // PC.07
#define EXTMODULE_TX_GPIO_AF            LL_GPIO_AF_3 // TIM8_CH1
#define EXTMODULE_TX_GPIO_AF_USART      GPIO_AF_USART6
#define EXTMODULE_RX_GPIO_AF_USART      GPIO_AF_USART6
#define EXTMODULE_TIMER                 TIM8
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
#if defined(RADIO_NB4P)
#define EXTMODULE_TIMER_IRQn            TIM5_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM5_IRQHandler
#else
#define EXTMODULE_TIMER_IRQn            TIM8_UP_TIM13_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM8_UP_TIM13_IRQHandler
#endif
#define EXTMODULE_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_3
//USART
#define EXTMODULE_USART                    USART6
#define EXTMODULE_USART_GPIO               GPIOC
#define EXTMODULE_USART_GPIO_AF            GPIO_AF_USART6
#define EXTMODULE_USART_GPIO_AF_LL         LL_GPIO_AF_8
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMA_CHANNEL_5
#define EXTMODULE_USART_TX_DMA_STREAM     LL_DMA_STREAM_7

#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMA_CHANNEL_5
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_2

#define EXTMODULE_USART_IRQHandler         USART6_IRQHandler
#define EXTMODULE_USART_IRQn               USART6_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMA_CHANNEL_7
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_1
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream1_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream1_IRQHandler

#define EXTMODULE_TX_INVERT_GPIO           GPIO_PIN(GPIOE, 3)  // PE.03
#define EXTMODULE_RX_INVERT_GPIO           GPIO_PIN(GPIOI, 15) // PI.15

#define EXTMODULE_TX_NORMAL()              EXTMODULE_TX_INVERT_GPIO->BSRRH = EXTMODULE_TX_INVERT_GPIO_PIN
#define EXTMODULE_TX_INVERTED()            EXTMODULE_TX_INVERT_GPIO->BSRRL = EXTMODULE_TX_INVERT_GPIO_PIN
#define EXTMODULE_RX_NORMAL()              EXTMODULE_RX_INVERT_GPIO->BSRRH = EXTMODULE_RX_INVERT_GPIO_PIN
#define EXTMODULE_RX_INVERTED()            EXTMODULE_RX_INVERT_GPIO->BSRRL = EXTMODULE_RX_INVERT_GPIO_PIN

// Trainer Port

#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOD, 12) // PD.12
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOD, 13) // PD.13
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

#define TRAINER_TIMER                   TIM4
#define TRAINER_TIMER_IRQn              TIM4_IRQn
#define TRAINER_TIMER_IRQHandler        TIM4_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_2
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION

// Bluetooth
#if !defined(RADIO_NB4P)
  #define BLUETOOTH_ON_GPIO               GPIO_PIN(GPIOI, 8) // PI.8
  #define BT_USART                        USART3
  #define BT_USART_IRQn                   USART3_IRQn
  #define BT_GPIO_TXRX                    GPIOB
  #define BT_TX_GPIO                      GPIO_PIN(GPIOB, 10) // PB.10
  #define BT_RX_GPIO                      GPIO_PIN(GPIOB, 11) // PB.11
  #define BT_EN_GPIO                      GPIO_PIN(GPIOI, 8)  // PI.08
  #define BT_CONNECTED_GPIO               GPIO_PIN(GPIOJ, 1)  // PJ.01
  #define BT_CMD_MODE_GPIO                GPIO_PIN(GPIOH, 6)  // PH.06
#endif

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_BRK_TIM12_IRQHandler

// SDRAM
#define SDRAM_BANK1
#if defined(RADIO_PL18U)
  #define SDRAM_32MB
#endif

// LCD Settings
#if defined(RADIO_NB4P) || defined(RADIO_NV14_FAMILY)
  #define LCD_W                         320
  #define LCD_H                         480
#else
  #define LCD_W                         480
  #define LCD_H                         320
#endif

#define LCD_PHYS_W                      320
#define LCD_PHYS_H                      480

#define LCD_DEPTH                       16
#define LCD_CONTRAST_DEFAULT            20
