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

#define CPU_FREQ            250000000
#define PERI1_FREQUENCY     250000000
#define PERI2_FREQUENCY     250000000
#define TIMER_MULT_APB1     1
#define TIMER_MULT_APB2     1

#define TELEMETRY_EXTI_PRIO             0 // required for soft serial

// Keys
#define KEYS_GPIO_REG_PAGEDN          GPIOB
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8  // PB.08
#define KEYS_GPIO_REG_PAGEUP          GPIOB
#define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_7  // PB.07
#define KEYS_GPIO_REG_EXIT            GPIOB
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5  // PB.05
#define KEYS_GPIO_REG_ENTER           GPIOE
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PE.10
#define KEYS_GPIO_REG_SYS             GPIOB
#define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
#define KEYS_GPIO_REG_MDL             GPIOB
#define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_6  // PB.06

// Rotary Encoder
#define ROTARY_ENCODER_NAVIGATION
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOE
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_9 // PE.09
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_11 // PE.11
#define ROTARY_ENCODER_POSITION()       (((ROTARY_ENCODER_GPIO->IDR >> 10) & 0x02) + ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x01))
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_9
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_11
#if !defined(USE_EXTI9_IRQ)
#define USE_EXTI9_IRQ
#define EXTI9_IRQ_Priority 5
#endif
#if !defined(USE_EXTI11_IRQ)
#define USE_EXTI11_IRQ
#define EXTI11_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_EXTI_EXTI_PORTE
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_EXTI_EXTI_LINE9
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_EXTI_EXTI_LINE11
#define ROTARY_ENCODER_TIMER            TIM17
#define ROTARY_ENCODER_TIMER_IRQn       TIM17_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM17_IRQHandler

// Trims
#define TRIMS_GPIO_REG_LHL            GPIOA
#define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_8  // PA.08
#define TRIMS_GPIO_REG_LHR            GPIOE
#define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_0  // PE.00
#define TRIMS_GPIO_REG_LVD            GPIOA
#define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_15 // PA.15
#define TRIMS_GPIO_REG_LVU            GPIOC
#define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_13 // PC.13
#define TRIMS_GPIO_REG_RVD            GPIOE
#define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_12 // PE.12
#define TRIMS_GPIO_REG_RHL            GPIOB
#define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_3  // PB.03
#define TRIMS_GPIO_REG_RVU            GPIOD
#define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_11 // PD.11
#define TRIMS_GPIO_REG_RHR            GPIOD
#define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_10 // PD.10

// Switches
#define SWITCHES_GPIO_REG_A           GPIOE
#define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_13 // PE.13
#define SWITCHES_GPIO_REG_B_L         GPIOD
#define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_1   // PD.01
#define SWITCHES_GPIO_REG_B_H         GPIOD
#define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_3   // PD.03
#define SWITCHES_GPIO_REG_C_L         GPIOD
#define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_8   // PD.08
#define SWITCHES_GPIO_REG_C_H         GPIOD
#define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_9   // PD.09
#define SWITCHES_GPIO_REG_D_L         GPIOB
#define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_15 // PB.15
#define SWITCHES_GPIO_REG_D_H         GPIOE
#define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_14 // PE.14
#define SWITCHES_GPIO_REG_E           GPIOD
#define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_14  // PD.14
#define SWITCHES_GPIO_REG_F           GPIOD
#define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_13  // PD.13
#define SWITCHES_E_INVERTED
#define SWITCHES_F_INVERTED

// ADC
#define ADC_MAIN                      ADC2	// RTC bat reading is only available on ADC2
#define ADC_DMA                       GPDMA1
#define ADC_DMA_CHANNEL               LL_GPDMA1_REQUEST_ADC2
#define ADC_DMA_STREAM                LL_DMA_CHANNEL_0
#define ADC_DMA_STREAM_IRQ            GPDMA1_Channel0_IRQn
#define ADC_DMA_STREAM_IRQHandler     GPDMA1_Channel0_IRQHandler

#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_6CYCLES_5
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT

#define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1  // PA.01
#define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0  // PA.00
#define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_2  // PC.02
#define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_3  // PC.03
#define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_1  // PC.01
#define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0  // PC.00
#define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_0  // PB.00
#define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_1  // PB.01
#define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_2  // PA.02
#define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1  // ADC12_INP1
#define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0  // ADC12_INP0
#define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_12 // ADC12_INP12
#define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_13 // ADC12_INP13
#define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_11 // ADC12_INP11
#define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_10 // ADC12_INP10
#define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_9  // ADC12_INP9
#define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_5  // ADC12_INP5
#define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_14 // ADC12_INP14
#define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_BATT)
#define ADC_GPIOB_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2)
#define ADC_GPIOC_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
#define ADC_VREF_PREC2                330

#define ADC_DIRECTION {-1,1,-1,1,  -1,-1, 1,-1}

// PWR and LED driver
#define PWR_SWITCH_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
#define PWR_ON_GPIO                   GPIO_PIN(GPIOD, 6)  // PD.06

#define STATUS_LEDS
#define GPIO_LED_GPIO_ON              gpio_set
#define GPIO_LED_GPIO_OFF             gpio_clear
#define LED_RED_GPIO                  GPIO_PIN(GPIOE, 6)
#define LED_BLUE_GPIO                 GPIO_PIN(GPIOB, 13)
#define LED_GREEN_GPIO                GPIO_PIN(GPIOB, 14)

// LED Strip
#if defined(RGBLEDS)
  #define LED_STRIP_LENGTH                  1
  #define BLING_LED_STRIP_START             0
  #define BLING_LED_STRIP_LENGTH            0
  #define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 10) // PA.10 / TIM1_CH3
  #define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1
  #define LED_STRIP_TIMER                   TIM1
  #define LED_STRIP_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH3
  #define LED_STRIP_TIMER_DMA               DMA2
  #define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMA_CHANNEL_6
  #define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_5
  #define LED_STRIP_TIMER_DMA_IRQn          DMA2_Stream5_IRQn
  #define LED_STRIP_TIMER_DMA_IRQHandler    DMA2_Stream5_IRQHandler
  #define LED_STRIP_REFRESH_PERIOD          50  //ms
#endif

// Internal Module
#define INTMODULE_BOOTCMD_DEFAULT       0 // RESET
#define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOE, 5)  // PE.05
#define INTMODULE_BOOTCMD_GPIO          GPIO_PIN(GPIOE, 4)  // PE.04
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOE, 3)  // PE.03
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOE, 2)  // PE.02
#define INTMODULE_USART                  USART10
#define INTMODULE_USART_IRQHandler       USART10_IRQHandler
#define INTMODULE_USART_IRQn             USART10_IRQn
/*  #define INTMODULE_DMA                    GPDMA1
  #define INTMODULE_DMA_STREAM             LL_DMA_CHANNEL_1
  #define INTMODULE_DMA_STREAM_IRQ         GPDMA1_Channel1_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  GPDMA1_Channel1_IRQHandler
  #define INTMODULE_DMA_CHANNEL            LL_GPDMA1_REQUEST_USART10_TX
  #define INTMODULE_RX_DMA                 GPDMA1
  #define INTMODULE_RX_DMA_STREAM          LL_DMA_CHANNEL_2
  #define INTMODULE_RX_DMA_CHANNEL         LL_GPDMA1_REQUEST_USART10_RX*/

// External Module
#define EXTMODULE_PWR_GPIO                 GPIO_PIN(GPIOD, 0) // PD.00
#define EXTERNAL_MODULE_PWR_ON()           gpio_set(EXTMODULE_PWR_GPIO)
#define EXTERNAL_MODULE_PWR_OFF()          gpio_clear(EXTMODULE_PWR_GPIO)
#define IS_EXTERNAL_MODULE_ON()            gpio_read(EXTMODULE_PWR_GPIO)
#define EXTMODULE_TX_GPIO                  GPIO_PIN(GPIOA, 9)  // PA.09 TIM1_CH2
#define EXTMODULE_RX_GPIO                  GPIO_PIN(GPIOA, 10) // PA.10
#define EXTMODULE_TX_GPIO_AF               LL_GPIO_AF_7
#define EXTMODULE_TIMER                    TIM1
#define EXTMODULE_TIMER_Channel            LL_TIM_CHANNEL_CH2
#define EXTMODULE_TIMER_IRQn               TIM1_UP_IRQn
#define EXTMODULE_TIMER_IRQHandler         TIM1_UP_IRQHandler
#define EXTMODULE_TIMER_FREQ               (PERI2_FREQUENCY * TIMER_MULT_APB2)
#define EXTMODULE_TIMER_TX_GPIO_AF         LL_GPIO_AF_1

//USART
#define EXTMODULE_USART                    USART1
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOA, 10)
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOA, 9)
#define EXTMODULE_USART_TX_DMA             GPDMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_GPDMA2_REQUEST_USART1_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_CHANNEL_1
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_GPDMA2_REQUEST_USART1_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_CHANNEL_2
#define EXTMODULE_USART_IRQHandler         USART1_IRQHandler
#define EXTMODULE_USART_IRQn               USART1_IRQn

//TIMER
// TIM1_CH2
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_GPDMA2_REQUEST_TIM1_CH2
#define EXTMODULE_TIMER_DMA                GPDMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_CHANNEL_0
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    GPDMA2_Channel0_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     GPDMA2_Channel0_IRQHandler

// Trainer Port
#define TRAINER_DETECT_GPIO           GPIO_PIN(GPIOE, 15) // PE.15
#define TRAINER_IN_GPIO               GPIO_PIN(GPIOC, 7)  // PC.07 TIM3_CH2, TIM8_CH2
#define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH2
#define TRAINER_OUT_GPIO              GPIO_PIN(GPIOC, 6)  // PC.06 TIM3_CH1, TIM8_CH1,
#define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH1
#define TRAINER_GPIO_AF               LL_GPIO_AF_1
#define TRAINER_TIMER                 TIM3
#define TRAINER_TIMER_IRQn            TIM15_IRQn
//#define TRAINER_TIMER_IRQHandler      TIM1_IRQHandler
#define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Serial Port
#define HARDWARE_TRAINER_AUX_SERIAL
#define AUX_SERIAL_PWR_GPIO               GPIO_PIN(GPIOD, 12) // PD.12
#define AUX_SERIAL_GPIO                   GPIOB
#define AUX_SERIAL_TX_GPIO                GPIO_PIN(GPIOE, 7)  // PE.07
#define AUX_SERIAL_RX_GPIO                GPIO_PIN(GPIOE, 8) // PE.08
#define AUX_SERIAL_USART                  USART7
#define AUX_SERIAL_USART_IRQn             USART7_IRQn
#define AUX_SERIAL_DMA_RX                 DMA1
#define AUX_SERIAL_DMA_RX_STREAM          LL_DMA_STREAM_1
#define AUX_SERIAL_DMA_RX_CHANNEL         LL_DMA_CHANNEL_4

// Telemetry
// half duplex telem
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOB, 9) // PB.09
#define TELEMETRY_RX_GPIO               GPIO_UNDEF

#define TELEMETRY_USART                 USART2
#define TELEMETRY_DMA                   GPDMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_CHANNEL_3
#define TELEMETRY_DMA_Channel_TX        LL_GPDMA2_REQUEST_USART2_TX
#define TELEMETRY_DMA_TX_IRQHandler     GPDMA1_Channel3_IRQHandler
#define TELEMETRY_DMA_TX_Stream_IRQ     GPDMA1_Channel3_IRQn
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn
#define TELEMETRY_EXTI_PORT             LL_SYSCFG_EXTI_PORTD
#define TELEMETRY_EXTI_SYS_LINE         LL_SYSCFG_EXTI_LINE6
#define TELEMETRY_EXTI_LINE             LL_EXTI_LINE_6
#define TELEMETRY_EXTI_TRIGGER          LL_EXTI_TRIGGER_RISING

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

// USB Charger
#define USB_CHARGER_GPIO              GPIO_PIN(GPIOD, 7)

// Trainer / Trainee from the module bay
//TODO

// USB
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOB, 12) // PB.12
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// BackLight
// No backlight: OLED display

// LCD driver
#define SSD1309_LCD
#define LCD_MOSI_GPIO                 GPIO_PIN(GPIOA, 7) // PA.07
#define LCD_CLK_GPIO                  GPIO_PIN(GPIOA, 5) // PA.05
#define LCD_A0_GPIO                   GPIO_PIN(GPIOA, 6) // PA.06
#define LCD_NCS_GPIO                  GPIO_PIN(GPIOC, 5) // PC.05
#define LCD_RST_GPIO                  GPIO_PIN(GPIOC, 4) // PC.04
#define LCD_DMA                       DMA1
#define LCD_DMA_Stream                DMA1_Stream7
#define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
#define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
#define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
#define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
#define LCD_SPI                       SPI1
#define LCD_GPIO_AF                   GPIO_AF8
#define LCD_SPI_PRESCALER             SPI_CR1_BR_1

// EEPROM
  // no EEPROM

// SD - SPI2
// Using chip, so no detect
#define STORAGE_USE_SDIO

/*
 * SDIO pins:
 *  - SDMMC1_D0-D3 (PC.08-11)
 *  - SDMMC1_CK    (PC.12)
 *  - SDMMC1_CMD   (PD.02)
 */
#define SD_SDIO SDMMC1
#define SD_SDIO_TRANSFER_CLK_DIV SDMMC_NSPEED_CLK_DIV


// Audio
#define AUDIO_OUTPUT_GPIO               GPIO_PIN(GPIOA, 4)
#define AUDIO_DMA                       GPDMA1
#define AUDIO_DMA_Stream                LL_DMA_CHANNEL_5
#define AUDIO_DMA_Stream_IRQn           GPDMA1_Channel5_IRQn
#define AUDIO_DMA_Stream_IRQHandler     GPDMA1_Channel5_IRQHandler
#define AUDIO_TIMER                     TIM6
#define AUDIO_DMA_REQUEST               LL_GPDMA1_REQUEST_DAC1_CH1
#define AUDIO_DAC                       DAC1

#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOB, 2)
#define AUDIO_MUTE_DELAY              500  // ms
#define AUDIO_UNMUTE_DELAY            150  // ms

// Haptic
#define HAPTIC_PWM
#define HAPTIC_GPIO                   GPIO_PIN(GPIOB, 10) // PB.10
#define HAPTIC_GPIO_AF                GPIO_AF1
#define HAPTIC_TIMER                  TIM2  // Timer 2 Channel 3
#define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR3
#define HAPTIC_CCMR1                  TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2
#define HAPTIC_CCER                   TIM_CCER_CC3E

// Bluetooth

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM12_IRQHandler
