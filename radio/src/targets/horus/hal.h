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

#define CPU_FREQ                        168000000
#define PERI1_FREQUENCY                 42000000
#define PERI2_FREQUENCY                 84000000
#define TIMER_MULT_APB1                 2
#define TIMER_MULT_APB2                 2

#define TELEMETRY_EXTI_PRIO             0 // required for soft serial

// Keys
#if defined(PCBX12S)
#elif defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
#elif defined(RADIO_T15)
#elif defined(PCBX10)
#endif

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
#define ROTARY_ENCODER_TIMER            TIM12
#define ROTARY_ENCODER_TIMER_IRQn       TIM8_BRK_TIM12_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM8_BRK_TIM12_IRQHandler

#if defined(RADIO_T15)
    #define ROTARY_ENCODER_INVERTED
#endif

// Switches
#if defined(RADIO_T15)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A

  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B

  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C

  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D

  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  //SW1
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  //SW2
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  //SW3
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  //SW4
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  //SW5
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  //SW6
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
#else
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H

  #if defined(PCBX12S)
  #elif defined(PCBX10)
  #endif

  #if defined(PCBX10) && !defined(RADIO_F16)
    // Gimbal switch left
    #define STORAGE_SWITCH_I
    #define HARDWARE_SWITCH_I
    // Gimbal switch right
    #define STORAGE_SWITCH_J
    #define HARDWARE_SWITCH_J
  #elif defined(PCBX12S)
    // Gimbal switch left
    #define STORAGE_SWITCH_I
    #define HARDWARE_SWITCH_I
    // Gimbal switch right
    #define STORAGE_SWITCH_J
    #define HARDWARE_SWITCH_J
  #endif
#endif

// 6POS SW
#if defined(RADIO_V16)
  #define SIXPOS_SWITCH_INDEX             5
  #define SIXPOS_LED_RED                200
  #define SIXPOS_LED_GREEN              0
  #define SIXPOS_LED_BLUE               0
#endif

// Trims
#if defined(RADIO_T15)
#elif defined(PCBX12S)
#elif defined(PCBX10)
  // Left/Right Horizontal trims (LHL/LHR)
  #if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
  #else
  #endif

  // Vertical trims - common across all PCBX10 variants

  // Right Horizontal trims - common across all PCBX10 variants
  
  // T5/T6 trim assignments
  #if defined(RADIO_V16)
    // T5/T6 trims are in exchanged positions to other handsets
  #elif defined(RADIO_FAMILY_T16)
  #else // Other PCBX10 variants
  #endif
#endif


// ADC
#if defined(PCBX12S)


  #define ADC_GPIO_MOUSE                GPIOF
  // TODO: use for SPI1_RX instead?
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_V16)
  #define ADC_GPIOA_PINS_FS             (LL_GPIO_PIN_2 | LL_GPIO_PIN_3)
  #define ADC_VREF_PREC2                330
#elif defined(PCBX10)
#if defined(RADIO_T15)
#else
#endif
#if !defined(RADIO_T15)
#endif
  #if defined(RADIO_X10) || defined(RADIO_X10E)
    #define PWM_STICKS
    #define PWM_TIMER                   TIM5
    #define PWM_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
    #define PWM_GPIO                    GPIOA
    #define PWM_GPIO_AF                 GPIO_AF2
    #define PWM_IRQHandler              TIM5_IRQHandler
    #define PWM_IRQn                    TIM5_IRQn
    #define PWM_GPIOA_PINS              (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)
  #endif
#if defined(RADIO_T15)
#else
#endif
#if !defined(RADIO_T15)
#endif
#if defined(RADIO_T15)
#else
  #define ADC_GPIOA_PINS_FS             (LL_GPIO_PIN_2 | LL_GPIO_PIN_3)
#endif
  #if defined(RADIO_V16)
    // Required DMA for more than one channel for EXT_ADC
  #else
  #endif

  // VBat divider is /4 on F42x and F43x devices
  #if defined(RADIO_TX16S) || defined(RADIO_T15) || defined(RADIO_F16) || defined(RADIO_V16) || defined(RADIO_T18)
    #define ADC_VREF_PREC2              330
  #elif defined(RADIO_T16)
    #define ADC_VREF_PREC2              300
  #else
    #define ADC_VREF_PREC2              250
  #endif
#endif

#if defined(RADIO_T15)
#elif defined(RADIO_T16)
#elif defined(RADIO_T18)
#elif defined(RADIO_TX16S) || defined(RADIO_F16)
#elif defined(RADIO_V16)
#elif defined(PCBX10)
#elif defined(PCBX12S)
#else
  #error "Missing ADC_DIRECTION array"
#endif

#if defined(RADIO_TX16S)
  #define DEFAULT_6POS_CALIB          {3, 12, 21, 30, 38}
  #define DEFAULT_6POS_IDX            5
#endif

  
// Power
#if defined(RADIO_T18)
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOJ, 1) // PJ.01
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOJ, 0) // PJ.00
  #define PWR_EXTRA_SWITCH_GPIO         GPIO_PIN(GPIOB, 0) // PB.00
#elif defined(RADIO_V16)
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOB, 2) // PB.02
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOJ, 0) // PJ.00
#else
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOJ, 1) // PJ.01
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOJ, 0) // PJ.00
#endif

// USB Charger
#if defined(USB_CHARGER)
  #define USB_CHARGER_GPIO              GPIO_PIN(GPIOG, 11) // PG.11
  #define USB_USBDet_GPIO               GPIO_PIN(GPIOG, 13) // PG.13
#endif

// S.Port update connector
#if defined(RADIO_FAMILY_T16)
  #define SPORT_MAX_BAUDRATE            400000
#else
  #define SPORT_MAX_BAUDRATE            250000 // < 400000
#endif

#if defined(PCBX10) && !defined(RADIO_FAMILY_T16) && !defined(RADIO_T15)
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOH, 13) // PH.13
#endif

// PCBREV
#if defined(RADIO_T15)
  #define PCBREV_GPIO                   GPIO_PIN(GPIOH, 8) // PH.08
  #define PCBREV_VALUE()                (gpio_read(PCBREV_GPIO) >> 8)
#elif defined(RADIO_V16)
  #define PCBREV_VALUE()                {0}
#elif defined(PCBX10)
  #define PCBREV_GPIO_1                 GPIO_PIN(GPIOH, 7) // PH.07
  #define PCBREV_GPIO_2                 GPIO_PIN(GPIOH, 8) // PH.08
  #define PCBREV_TOUCH_GPIO             GPIO_PIN(GPIOA, 6) // PA.06
  #define PCBREV_TOUCH_PULL_TYPE        GPIO_IN_PU
  #define PCBREV_VALUE()                ((gpio_read(PCBREV_GPIO_1) ? 1 : 0) + ((gpio_read(PCBREV_GPIO_2) ? 1 : 0) << 1)) * (gpio_read(PCBREV_TOUCH_GPIO) ? 1 : 0)
#else
  #define PCBREV_GPIO                   GPIO_PIN(GPIOI, 11) // PI.11
  #define PCBREV_VALUE()                (gpio_read(PCBREV_GPIO) >> 11)
#endif

#if defined(PCBREV_TOUCH_GPIO) && !defined(PCBREV_TOUCH_PULL_TYPE)
  #define PCBREV_TOUCH_PULL_TYPE GPIO_IN_PD
  #pragma message "PCBREV_TOUCH_PULL_TYPE not defined, defaulting to GPIO_IN_PD"
#endif

// Led
#define STATUS_LEDS
#if defined(PCBX12S)
  #define LED_GPIO                      GPIO_PIN(GPIOI, 5) // PI.05
#elif defined(RADIO_T15)
  #define LED_RED_GPIO                  GPIO_PIN(GPIOI, 14)  //PI.14
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOC, 13)  //PC.13
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 3)   //PE.03
#elif defined(PCBX10)
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 2) // PE.02
  #if !defined(MANUFACTURER_FRSKY)                         // no green on X10/X10 Express
    #define LED_GREEN_GPIO              GPIO_PIN(GPIOE, 4) // PE.04
  #endif
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 5) // PE.05
#endif

// Customisable switches leds
#if defined(RADIO_T15)
#define FSLED_GPIO_1                  GPIO_PIN(GPIOA, 15)  //PA.15
#define FSLED_GPIO_2                  GPIO_PIN(GPIOC, 5)   //PC.05
#define FSLED_GPIO_3                  GPIO_PIN(GPIOH, 13)  //PH.13
#define FSLED_GPIO_4                  GPIO_PIN(GPIOG, 11)  //PG.11
#define FSLED_GPIO_5                  GPIO_PIN(GPIOC, 3)   //PC.03
#define FSLED_GPIO_6                  GPIO_PIN(GPIOC, 1)   //PC.01
#endif

// Serial Port (DEBUG)
#if defined(AUX_SERIAL)
  #if defined(RADIO_V16)
    #define AUX_SERIAL_TX_GPIO                  GPIO_PIN(GPIOG, 14) // PG.14
    #define AUX_SERIAL_RX_GPIO                  GPIO_PIN(GPIOG, 9) // PG.09
    #define AUX_SERIAL_USART                    USART6
    #define AUX_SERIAL_USART_IRQHandler         USART6_IRQHandler
    #define AUX_SERIAL_USART_IRQn               USART6_IRQn
    #define AUX_SERIAL_DMA_TX                   DMA2
    #define AUX_SERIAL_DMA_TX_STREAM            LL_DMA_STREAM_7
    #define AUX_SERIAL_DMA_TX_CHANNEL           LL_DMA_CHANNEL_5
    #define AUX_SERIAL_DMA_RX                   DMA2
    #define AUX_SERIAL_DMA_RX_STREAM            LL_DMA_STREAM_2
    #define AUX_SERIAL_DMA_RX_CHANNEL           LL_DMA_CHANNEL_5
    #define AUX_SERIAL_PWR_GPIO               GPIO_PIN(GPIOA, 15) // PA.15
  #else
    #define AUX_SERIAL_TX_GPIO                  GPIO_PIN(GPIOB, 10) // PB.10
    #define AUX_SERIAL_RX_GPIO                  GPIO_PIN(GPIOB, 11) // PB.11
    #define AUX_SERIAL_USART                    USART3
    #define AUX_SERIAL_USART_IRQHandler         USART3_IRQHandler
    #define AUX_SERIAL_USART_IRQn               USART3_IRQn
    #define AUX_SERIAL_DMA_TX                   DMA1
    #define AUX_SERIAL_DMA_TX_STREAM            LL_DMA_STREAM_3
    #define AUX_SERIAL_DMA_TX_CHANNEL           LL_DMA_CHANNEL_4
    #define AUX_SERIAL_DMA_RX                   DMA1
    #define AUX_SERIAL_DMA_RX_STREAM            LL_DMA_STREAM_1
    #define AUX_SERIAL_DMA_RX_CHANNEL           LL_DMA_CHANNEL_4
    #if defined(RADIO_TX16S) || defined(RADIO_F16)
      #define AUX_SERIAL_PWR_GPIO               GPIO_PIN(GPIOA, 15) // PA.15
    #endif
  #endif
#endif

#if defined(AUX2_SERIAL)
#if defined(PCBX12S)
  #define AUX2_SERIAL_USART                    UART4
  #define AUX2_SERIAL_USART_IRQn               UART4_IRQn
  #define AUX2_SERIAL_TX_GPIO                  GPIO_PIN(GPIOA, 0) // PA.00
  #define AUX2_SERIAL_RX_GPIO                  GPIO_PIN(GPIOA, 1) // PA.01
  #define AUX2_SERIAL_DMA_RX                   DMA1
  #define AUX2_SERIAL_DMA_RX_STREAM            LL_DMA_STREAM_2
  #define AUX2_SERIAL_DMA_RX_CHANNEL           LL_DMA_CHANNEL_4
#else
  #define AUX2_SERIAL_USART                    USART6
  #define AUX2_SERIAL_USART_IRQn               USART6_IRQn
  #define AUX2_SERIAL_GPIO                     GPIOG
  #define AUX2_SERIAL_TX_GPIO                  GPIO_PIN(GPIOG, 14) // PG.14
  #define AUX2_SERIAL_RX_GPIO                  GPIO_PIN(GPIOG, 9)  // PG.09
  #define AUX2_SERIAL_DMA_RX                   DMA2
  #define AUX2_SERIAL_DMA_RX_STREAM            LL_DMA_STREAM_1 // or stream 2
  #define AUX2_SERIAL_DMA_RX_CHANNEL           LL_DMA_CHANNEL_5
  #if !defined(RADIO_T18)
    #define AUX2_SERIAL_PWR_GPIO                 GPIO_PIN(GPIOB, 0) // PB.00
  #endif
#endif
#endif

// Telemetry
#if defined(RADIO_V16)
#define TELEMETRY_RX_REV_GPIO           GPIO_PIN(GPIOH, 7)  // PH.07
#define TELEMETRY_TX_REV_GPIO           GPIO_PIN(GPIOH, 7)  // PH.07
#define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOD, 4) // PD.04
#define TELEMETRY_SET_INPUT             0
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOD, 5) // PD.05
#define TELEMETRY_RX_GPIO               GPIO_PIN(GPIOD, 6) // PD.06
#define TELEMETRY_USART                 USART2
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_6
#define TELEMETRY_DMA_Channel_TX        LL_DMA_CHANNEL_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn
#else
#define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOD, 4) // PD.04
#define TELEMETRY_SET_INPUT             0
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOD, 5) // PD.05
#define TELEMETRY_RX_GPIO               GPIO_PIN(GPIOD, 6) // PD.06
#define TELEMETRY_USART                 USART2
#define TELEMETRY_DMA                   DMA1
#if defined(PCBX12S)
  #define TELEMETRY_DMA_Stream_RX       LL_DMA_STREAM_5
  #define TELEMETRY_DMA_Channel_RX      LL_DMA_CHANNEL_4
#endif
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_6
#define TELEMETRY_DMA_Channel_TX        LL_DMA_CHANNEL_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn
#define TELEMETRY_EXTI_PORT             LL_SYSCFG_EXTI_PORTD
#define TELEMETRY_EXTI_SYS_LINE         LL_SYSCFG_EXTI_LINE6
#define TELEMETRY_EXTI_LINE             LL_EXTI_LINE_6
#define TELEMETRY_EXTI_TRIGGER          LL_EXTI_TRIGGER_RISING
// TELEMETRY_EXTI IRQ
#if !defined(USE_EXTI9_5_IRQ)
  #define USE_EXTI9_5_IRQ
#endif
// overwrite priority
#undef EXTI9_5_IRQ_Priority
#define EXTI9_5_IRQ_Priority            TELEMETRY_EXTI_PRIO

#define TELEMETRY_TIMER                 TIM11
#define TELEMETRY_TIMER_IRQn            TIM1_TRG_COM_TIM11_IRQn
#define TELEMETRY_TIMER_IRQHandler      TIM1_TRG_COM_TIM11_IRQHandler
#endif

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

// USB
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOA, 9)  // PA.09
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#if defined(RADIO_T15)
  #define LCD_NRST_GPIO                   GPIOG
  #define LCD_NRST_GPIO_PIN               LL_GPIO_PIN_10 // PG.10
  #define LCD_SPI_GPIO                    GPIOE
  #define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_4  // PE.04
  #define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_2  // PE.02
  #define LCD_SPI_MISO_GPIO_PIN           LL_GPIO_PIN_5  // PE.05
  #define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_6  // PE.06
  #define LTDC_IRQ_PRIO                   4
  #define DMA_SCREEN_IRQ_PRIO             6
#else
#if defined(PCBX12S)
  #define LCD_GPIO_NRST                 GPIO_PIN(GPIOF, 10) // PF.10
#elif defined(PCBX10)
  #define LCD_GPIO_NRST                 GPIO_PIN(GPIOI, 10) // PI.10
#endif
#if defined(PCBX10) && !defined(RADIO_T18) && !defined(RADIO_V16)
  #define LCD_VERTICAL_INVERT
#endif
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6
#endif

// Backlight
#if defined(PCBX12S)
  #if PCBREV >= 13
    #define BACKLIGHT_TIMER               TIM5
    #define BACKLIGHT_GPIO                GPIO_PIN(GPIOA, 3) // PA.03
    #define BACKLIGHT_GPIO_AF             GPIO_AF2
    #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #else
    #define BACKLIGHT_TIMER               TIM8
    #define BACKLIGHT_GPIO                GPIO_PIN(GPIOA, 5) // PA.05
    #define BACKLIGHT_GPIO_AF             GPIO_AF3
    #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #endif
#elif defined(PCBX10)
  #define BACKLIGHT_TIMER                 TIM8
  #define BACKLIGHT_GPIO                  GPIO_PIN(GPIOB, 1) // PB.01
  #define BACKLIGHT_GPIO_AF               GPIO_AF3
  #define BACKLIGHT_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

#if defined(RADIO_T18)
  #define KEYS_BACKLIGHT_GPIO             GPIO_PIN(GPIOC, 4) // PC.04
#endif

// SD
#if !defined(RADIO_T15)
#define SD_PRESENT_GPIO                   GPIO_PIN(GPIOC, 5) // PC.05
#endif
#define SD_SDIO_DMA                       DMA2
#define SD_SDIO_DMA_STREAM                DMA2_Stream3 // or Stream6
#define SD_SDIO_DMA_CHANNEL               LL_DMA_CHANNEL_4
#define SD_SDIO_DMA_IRQn                  DMA2_Stream3_IRQn
#define SD_SDIO_DMA_IRQHANDLER            DMA2_Stream3_IRQHandler
#define SD_SDIO_CLK_DIV(fq)               ((48000000 / (fq)) - 2)
#define SD_SDIO_TRANSFER_CLK_DIV          SD_SDIO_CLK_DIV(24000000)

// Uncomment only one line below to select which storage to use
#define STORAGE_USE_SDIO      // Use SD card for storage with SDIO driver
//#define STORAGE_USE_SPI_FLASH // Use SPI flash for storage instead of SD card

// SPI NOR Flash 
#if defined(PCBX12S) && PCBREV >= 13
  #define FLASH_SPI                      SPI1
  #define FLASH_SPI_CS_GPIO              GPIO_PIN(GPIOA, 15) // PA.15
  #define FLASH_SPI_GPIO                 GPIOA
  #define FLASH_SPI_SCK_GPIO             GPIO_PIN(GPIOA, 5) // PA.05
  #define FLASH_SPI_MISO_GPIO            GPIO_PIN(GPIOA, 6) // PA.06
  #define FLASH_SPI_MOSI_GPIO            GPIO_PIN(GPIOA, 7) // PA.07
  #define FLASH_SPI_DMA                  DMA2
  #define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_3
  #define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_3
  #define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_5
  // SPI1_TX: DMA2 Stream 3 (SDIO) / Stream 5 (Ext. module timer)
  // #define FLASH_SPI_TX_DMA_CHANNEL       DMA_Channel_3
  // #define FLASH_SPI_TX_DMA_STREAM        DMA2_Stream3
  // #define FLASH_SPI_TX_DMA_IRQn          DMA2_Stream3_IRQn
  // #define FLASH_SPI_TX_DMA_IRQHandler    DMA2_Stream3_IRQHandler
  // #define FLASH_SPI_TX_DMA_FLAG_TC       DMA_IT_TCIF3
  // #define FLASH_SPI_TX_DMA_STATUS_REG    HISR
  // SPI1_RX: DMA2 Stream 0 / Stream 2 
  // #define FLASH_SPI_RX_DMA_CHANNEL       DMA_Channel_3
  // #define FLASH_SPI_RX_DMA_STREAM        DMA2_Stream5
  // #define FLASH_SPI_RX_DMA_IRQn          DMA2_Stream5_IRQn
  // #define FLASH_SPI_RX_DMA_IRQHandler    DMA2_Stream5_IRQHandler
  // #define FLASH_SPI_RX_DMA_STATUS_REG    HISR
  // #define FLASH_SPI_RX_DMA_FLAG_TC       DMA_IT_TCIF5
#elif defined(PCBX10)
  #define FLASH_SPI                      SPI2
  #define FLASH_SPI_CS_GPIO              GPIO_PIN(GPIOI, 0)  // PI.00
  #define FLASH_SPI_GPIO                 GPIOI
  #define FLASH_SPI_SCK_GPIO             GPIO_PIN(GPIOI, 1)  // PI.01
  #define FLASH_SPI_MISO_GPIO            GPIO_PIN(GPIOI, 2)  // PI.02
  #define FLASH_SPI_MOSI_GPIO            GPIO_PIN(GPIOI, 3)  // PI.03
  #define FLASH_SPI_DMA                  DMA1
  #define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_0
  #define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_4
  #define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_3
  // #define FLASH_SPI_TX_DMA_CHANNEL       DMA_Channel_0
  // #define FLASH_SPI_TX_DMA_STREAM        DMA1_Stream4
  // #define FLASH_SPI_TX_DMA_IRQn          DMA1_Stream4_IRQn
  // #define FLASH_SPI_TX_DMA_IRQHandler    DMA1_Stream4_IRQHandler
  // #define FLASH_SPI_TX_DMA_FLAG_TC       DMA_IT_TCIF4
  // #define FLASH_SPI_TX_DMA_STATUS_REG    HISR
  // #define FLASH_SPI_RX_DMA_CHANNEL       DMA_Channel_0
  // #define FLASH_SPI_RX_DMA_STREAM        DMA1_Stream3
  // #define FLASH_SPI_RX_DMA_IRQn          DMA1_Stream3_IRQn
  // #define FLASH_SPI_RX_DMA_IRQHandler    DMA1_Stream3_IRQHandler
  // #define FLASH_SPI_RX_DMA_STATUS_REG    LISR
  // #define FLASH_SPI_RX_DMA_FLAG_TC       DMA_IT_TCIF3
#endif

// Audio
#if defined(PCBX12S)
  #define AUDIO_SHUTDOWN_GPIO           GPIO_PIN(GPIOI, 9)   // PI.09
  #define AUDIO_XDCS_GPIO               GPIO_PIN(GPIOI, 0)   // PI.00
  #define AUDIO_CS_GPIO                 GPIO_PIN(GPIOH, 13)  // PH.13
  #define AUDIO_DREQ_GPIO               GPIO_PIN(GPIOH, 14)  // PH.14
  #define AUDIO_RST_GPIO                GPIO_PIN(GPIOH, 15)  // PH.15
  #define AUDIO_SPI                     SPI2
  #define AUDIO_SPI_SCK_GPIO            GPIO_PIN(GPIOI, 1)  // PI.01
  #define AUDIO_SPI_MISO_GPIO           GPIO_PIN(GPIOI, 2)  // PI.02
  #define AUDIO_SPI_MOSI_GPIO           GPIO_PIN(GPIOI, 3)  // PI.03
#elif defined (PCBX10)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOA, 7) // PA.07
  #define AUDIO_OUTPUT_GPIO             GPIO_PIN(GPIOA, 4) // PA.04
  #define AUDIO_DMA_Stream              DMA1_Stream5
  #define AUDIO_DMA_Stream_IRQn         DMA1_Stream5_IRQn
  #define AUDIO_DMA_Stream_IRQHandler   DMA1_Stream5_IRQHandler
  #define AUDIO_TIMER                   TIM6
  #define AUDIO_DMA                     DMA1
#endif

#if defined(RADIO_FAMILY_T16)
#if defined(RADIO_TX16S)  || defined(RADIO_F16) || defined(RADIO_V16)
  #define AUDIO_UNMUTE_DELAY            150  // ms
#else
  #define AUDIO_UNMUTE_DELAY            120  // ms
#endif
  #define AUDIO_MUTE_DELAY              500  // ms
#endif


// Touch
#if defined(HARDWARE_TOUCH)
  #define TOUCH_I2C_BUS                   I2C_Bus_1
  #define TOUCH_INT_GPIO                  GPIO_PIN(GPIOH, 2)  // PH.02
#if defined(PCBX12S)
  #define TOUCH_RST_GPIO                  GPIO_PIN(GPIOF, 7)  // PF.7
#else
  #define TOUCH_RST_GPIO                  GPIO_PIN(GPIOF, 10)  // PF.10
#endif
  #define TOUCH_INT_EXTI_Line             LL_EXTI_LINE_2
  #define TOUCH_INT_EXTI_Port             LL_SYSCFG_EXTI_PORTH
  #define TOUCH_INT_EXTI_SysCfgLine       LL_SYSCFG_EXTI_LINE2
  // TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI2_IRQ)
  #define USE_EXTI2_IRQ
  #define EXTI2_IRQ_Priority  9
#endif

#if defined(PCBX12S)
  #define TOUCH_PANEL_INVERTED
#endif
#endif // HARDWARE_TOUCH

// First I2C Bus
#if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(PCBX12S) || defined(RADIO_T15) || defined(RADIO_V16)
  #define I2C_B1                      I2C1
  #define I2C_B1_SCL_GPIO             GPIO_PIN(GPIOB, 8)  // PB.08
  #define I2C_B1_SDA_GPIO             GPIO_PIN(GPIOB, 9)  // PB.09
  #define I2C_B1_GPIO_AF              LL_GPIO_AF_4   // I2C1
  #define I2C_B1_CLK_RATE             400000
#else
  #define I2C_B1                      I2C3
  #define I2C_B1_SCL_GPIO             GPIO_PIN(GPIOH, 7)  // PH.07
  #define I2C_B1_SDA_GPIO             GPIO_PIN(GPIOH, 8)  // PH.08
  #define I2C_B1_GPIO_AF              LL_GPIO_AF_4   // I2C3
  #define I2C_B1_CLK_RATE             400000
#endif

// Second I2C Bus
#if !defined(AUX_SERIAL) && defined(IMU_LSM6DS33)
  #define I2C_B2                      I2C2
  #define I2C_B2_SCL_GPIO             GPIO_PIN(GPIOB, 10)  // PB.10
  #define I2C_B2_SDA_GPIO             GPIO_PIN(GPIOB, 11)  // PB.11
  #define I2C_B2_GPIO_AF              LL_GPIO_AF_4    // I2C2
  #define I2C_B2_CLK_RATE             400000
  #if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
    #define I2C_B2_PWR_GPIO           GPIO_PIN(GPIOA, 15)  // PA.15
  #endif
#endif

#if defined(IMU)
 #if defined(PCBX12S)
   #define IMU_I2C_BUS                   I2C_Bus_1
   #define IMU_I2C_ADDRESS               0x6A
 #elif !defined(AUX_SERIAL) && defined(IMU_LSM6DS33)
   #define IMU_I2C_BUS                   I2C_Bus_2
   #define IMU_I2C_ADDRESS               0x6A
 #endif
#endif

// Haptic
#define HAPTIC_PWM
#if defined(PCBX12S)
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOA, 2) // PA.02
  #define HAPTIC_GPIO_TIMER             TIM9
  #define HAPTIC_GPIO_AF                GPIO_AF3
  #define HAPTIC_TIMER_OUTPUT_ENABLE    TIM_CCER_CC1E
  #define HAPTIC_TIMER_MODE             TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2
  #define HAPTIC_TIMER_COMPARE_VALUE    HAPTIC_GPIO_TIMER->CCR1
#elif defined(RADIO_T15)  // TIM2_CH1
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOA, 5) // PA.05
  #define HAPTIC_GPIO_TIMER             TIM2
  #define HAPTIC_GPIO_AF                GPIO_AF1
  #define HAPTIC_TIMER_OUTPUT_ENABLE    TIM_CCER_CC1E | TIM_CCER_CC1NE;
  #define HAPTIC_TIMER_MODE             TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE
  #define HAPTIC_TIMER_COMPARE_VALUE    HAPTIC_GPIO_TIMER->CCR1
#elif defined(PCBX10)
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOE, 6) // PE.06
  #define HAPTIC_GPIO_TIMER             TIM9
  #define HAPTIC_GPIO_AF                GPIO_AF3
  #define HAPTIC_TIMER_OUTPUT_ENABLE    TIM_CCER_CC2E
  #define HAPTIC_TIMER_MODE             TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_TIMER_COMPARE_VALUE    HAPTIC_GPIO_TIMER->CCR2
  // FlySky Hall Sticks
  #define FLYSKY_HALL_SERIAL_USART                 UART4
  #define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
  #define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOA, 0) // PA.00
  #define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOA, 1) // PA.01
  #define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
  #define FLYSKY_HALL_SERIAL_DMA                   DMA1
  #define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
  #define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
#endif

#if defined(RADIO_V16)
  // LED Strip
  #define LED_STRIP_LENGTH                  40
  #define BLING_LED_STRIP_START             6
  #define BLING_LED_STRIP_LENGTH            34
  #define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 10)  // PA.10 / TIM1_CH3
  #define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1    // TIM1/2
  #define LED_STRIP_TIMER                   TIM1
  #define LED_STRIP_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH3
  #define LED_STRIP_TIMER_DMA               DMA2
  #define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMA_CHANNEL_6
  #define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_5
  #define LED_STRIP_TIMER_DMA_IRQn          DMA2_Stream5_IRQn
  #define LED_STRIP_TIMER_DMA_IRQHandler    DMA2_Stream5_IRQHandler
  #define LED_STRIP_REFRESH_PERIOD          50 //ms
#endif

// Internal PXX1 Module:
//  -> let's assume all internal XJT modules used are either X10 or X12S type
#define EXTERNAL_ANTENNA

#define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOA, 8) // PA.08
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 6) // PB.06
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 7) // PB.07
#define INTMODULE_USART                 USART1
#define INTMODULE_USART_IRQn            USART1_IRQn
#define INTMODULE_DMA                   DMA2
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_7
#define INTMODULE_DMA_STREAM_IRQ        DMA2_Stream7_IRQn
#define INTMODULE_DMA_STREAM_IRQHandler DMA2_Stream7_IRQHandler
#define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_4
#define INTMODULE_RX_DMA                DMA2
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_2
#define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_4
#if defined(PCBX12S)
  #define INTMODULE_BOOTCMD_GPIO        GPIO_PIN(GPIOC, 2) // PC.02
#elif defined(RADIO_V16)
  #define INTMODULE_BOOTCMD_GPIO        GPIO_PIN(GPIOB, 0) // PB.00
#elif defined(PCBX10)
  #define INTMODULE_BOOTCMD_GPIO        GPIO_PIN(GPIOI, 9) // PI.09
#endif
#if defined(RADIO_V16)
  #define INTMODULE_BOOTCMD_DEFAULT     1 // RESET
#else
  #define INTMODULE_BOOTCMD_DEFAULT     0 // RESET
#endif

// External Module
#define EXTMODULE_PWR_GPIO                 GPIO_PIN(GPIOB, 3) // PB.03
#define EXTERNAL_MODULE_PWR_OFF()          gpio_clear(EXTMODULE_PWR_GPIO)
#if (defined(PCBX10) && defined(PCBREV_EXPRESS)) || defined(RADIO_V16)
  #define EXTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 10) // PB.10 (TIM2_CH3)
  #define EXTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 11) // PB.11
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF1
  #define EXTMODULE_TIMER                  TIM2
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define EXTMODULE_TIMER_32BITS
  #define EXTMODULE_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define EXTMODULE_TIMER_IRQn             TIM2_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM2_IRQHandler
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_3
  #define EXTMODULE_TIMER_DMA              DMA1
  #define EXTMODULE_TIMER_DMA_STREAM       LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_FLAG_TC      DMA_IT_TCIF1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA1_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA1_Stream1_IRQHandler
  #define EXTMODULE_USART                  USART3
  #define EXTMODULE_USART_IRQn             USART3_IRQn
  #define EXTMODULE_USART_IRQHandler       USART3_IRQHandler
  #define EXTMODULE_USART_TX_DMA           DMA1
  #define EXTMODULE_USART_TX_DMA_CHANNEL   LL_DMA_CHANNEL_4
  #define EXTMODULE_USART_TX_DMA_STREAM    LL_DMA_STREAM_3
  #define EXTMODULE_USART_RX_DMA_CHANNEL   LL_DMA_CHANNEL_4
  #define EXTMODULE_USART_RX_DMA_STREAM    LL_DMA_STREAM_1
  #if defined(RADIO_V16)
    #define EXTMODULE_TX_INVERT_GPIO           GPIO_PIN(GPIOI, 2) // PI.02
    #define EXTMODULE_RX_INVERT_GPIO           GPIO_PIN(GPIOI, 9) // PI.09
  #endif
#elif defined(PCBX10) || PCBREV >= 13
  #define EXTMODULE_TX_GPIO                GPIO_PIN(GPIOA, 10) // PA.10 (TIM1_CH3)
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF1
  #define EXTMODULE_TIMER                  TIM1
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define EXTMODULE_TIMER_IRQn             TIM1_UP_TIM10_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM1_UP_TIM10_IRQHandler
  #define EXTMODULE_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_6
  #define EXTMODULE_TIMER_DMA              DMA2
  #define EXTMODULE_TIMER_DMA_STREAM       LL_DMA_STREAM_5
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA2_Stream5_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA2_Stream5_IRQHandler
#else
  #define EXTMODULE_TX_GPIO                GPIO_PIN(GPIOA, 15) // PA.15 (TIM2_CH1)
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF1
  #define EXTMODULE_TIMER                  TIM2
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH1
  #define EXTMODULE_TIMER_32BITS
  #define EXTMODULE_TIMER_IRQn             TIM2_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM2_IRQHandler
  #define EXTMODULE_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_3
  #define EXTMODULE_TIMER_DMA              DMA1
  #define EXTMODULE_TIMER_DMA_STREAM       LL_DMA_STREAM_7
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA1_Stream7_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA1_Stream7_IRQHandler
#endif

// Heartbeat
#if !defined(RADIO_V16)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_GPIO                GPIO_PIN(GPIOD, 12) // PD.12 / TIM4_CH1
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTH
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE12
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_12
#endif

// INTMODULE_HEARTBEAT_EXTI IRQ
#if !defined(USE_EXTI15_10_IRQ)
  #define USE_EXTI15_10_IRQ
  #define EXTI15_10_IRQ_Priority 5
#endif
#define INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER
#if defined(INTERNAL_MODULE_PXX2)
  #define INTMODULE_HEARTBEAT_TRIGGER           GPIO_FALLING
#else
  #define INTMODULE_HEARTBEAT_TRIGGER           GPIO_RISING
#endif

// Trainer Port
#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOC, 6) // PC.06
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1
#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOC, 7) // PC.07
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

#define TRAINER_DETECT_GPIO             GPIO_PIN(GPIOB, 4) // PB.04
#if !defined(PCBX10) || defined(RADIO_FAMILY_T16)
  #define TRAINER_DETECT_INVERTED
#endif

#define TRAINER_TIMER                   TIM3
#define TRAINER_TIMER_IRQn              TIM3_IRQn
#define TRAINER_TIMER_IRQHandler        TIM3_IRQHandler
#define TRAINER_GPIO_AF                 GPIO_AF2
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Trainer CPPM input on heartbeat pin
#if !defined(RADIO_V16)
  #define TRAINER_MODULE_CPPM_TIMER            TIM4
  #define TRAINER_MODULE_CPPM_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO             INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_CPPM_TIMER_Channel    LL_TIM_CHANNEL_CH1
  #define TRAINER_MODULE_CPPM_TIMER_IRQn       TIM4_IRQn
  #define TRAINER_MODULE_CPPM_TIMER_IRQHandler TIM4_IRQHandler
  #define TRAINER_MODULE_CPPM_GPIO_AF          LL_GPIO_AF_2
#endif

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM13
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_UP_TIM13_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_UP_TIM13_IRQHandler

// Bluetooth
#define STORAGE_BLUETOOTH
#if defined(BLUETOOTH)
  #if defined(RADIO_T15)
    #define BT_USART                      USART3
    #define BT_USART_IRQn                 USART3_IRQn
    #define BT_USART_GPIO                 GPIOB
    #define BT_TX_GPIO                    GPIO_PIN(GPIOB, 10) // PB.10
    #define BT_RX_GPIO                    GPIO_PIN(GPIOB, 11) // PB.11
  #else
    #define BT_USART                      USART6
    #define BT_USART_IRQn                 USART6_IRQn
    #define BT_USART_GPIO                 GPIOG
    #define BT_TX_GPIO                    GPIO_PIN(GPIOG, 14) // PG.14
    #define BT_RX_GPIO                    GPIO_PIN(GPIOG, 9)  // PG.09
    #if defined(RADIO_TX16S)
      #define BT_PWR_GPIO                 GPIO_PIN(GPIOB, 0) // PB.00
    #endif
  #endif
#endif

#if defined(PCBX12S)
  #if PCBREV >= 13
    #define BT_EN_GPIO                  GPIO_PIN(GPIOI, 10) // PI.10
  #else
    #define BT_EN_GPIO                  GPIO_PIN(GPIOA, 6) // PA.06
  #endif
  // #define BT_BRTS_GPIO                  GPIOG
  // #define BT_BRTS_GPIO_PIN              GPIO_Pin_10 // PG.10
  // #define BT_BCTS_GPIO                  GPIOG
  // #define BT_BCTS_GPIO_PIN              GPIO_Pin_11 // PG.11
#elif defined(PCBX10) && !defined(RADIO_T15)
  #define BT_EN_GPIO                    GPIO_PIN(GPIOG, 10) // PG.10
#endif

// Video switch
#if defined(RADIO_F16)
  #define VIDEO_SWITCH_GPIO             GPIO_PIN(GPIOH, 15) // PH.15
#endif

// SDRAM
#define SDRAM_BANK2

#if defined(RADIO_T15)
#define LCD_W                          480
#define LCD_H                          320
#else
#define LCD_W                          480
#define LCD_H                          272
#endif

#define LCD_PHYS_H                     LCD_H
#define LCD_PHYS_W                     LCD_W
#define LCD_DEPTH                      16
