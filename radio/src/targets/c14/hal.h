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

/*

STM32H750

SDMMC uses its own DMA controller (SDMMC1).

DMA1
Stream0:  LED_STRIP_TIMER_DMA_STREAM     (TIM2_UP)
Stream1:  AUDIO_DMA_Stream               (DAC1_CH1)
Stream2:  EXTMODULE_USART_RX_DMA_STREAM  (USART2_RX)
Stream3:  TELEMETRY_DMA_Stream_RX        (UART5_RX)
Stream4:  EXTMODULE_USART_TX_DMA_STREAM  (USART2_TX)
Stream7:  TELEMETRY_DMA_Stream_TX        (UART5_TX)

DMA2
Stream0:  ADC_EXT_DMA_STREAM             (ADC3, do not move)
Stream4:  ADC_DMA_STREAM                 (ADC1, do not move)

TIM2:   LED_STRIP_TIMER
TIM3:   BACKLIGHT_TIMER (CH1) / HAPTIC_GPIO_TIMER (CH2)
TIM5:   PWM_TIMER (stick PWM)
TIM6:   AUDIO_TIMER
TIM12:  MIXER_SCHEDULER_TIMER
TIM14:  MS_TIMER
TIM15:  TRAINER_TIMER
TIM17:  ROTARY_ENCODER_TIMER

USART2: EXTMODULE_USART
USART3: INTMODULE_USART (CRSF internal module, interrupt-driven; its
        DMA defines are disabled below)
UART5:  TELEMETRY_USART

 */


#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2


#define KEYS_GPIO_REG_ENTER           GPIOC
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_5
#define KEYS_GPIO_REG_PAGEDN          GPIOA
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8
#define KEYS_GPIO_REG_EXIT            GPIOG
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_14

#define KEYS_GPIO_REG_SYS             GPIOC
#define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4
#define KEYS_GPIO_REG_PAGEUP          GPIOC
#define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_13
#define KEYS_GPIO_REG_MDL             GPIOA
#define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_10

#define KEYS_GPIO_REG_UP              GPIOG
#define KEYS_GPIO_PIN_UP              LL_GPIO_PIN_2
#define KEYS_GPIO_REG_DOWN            GPIOG
#define KEYS_GPIO_PIN_DOWN            LL_GPIO_PIN_3

// Rotary encoder simulated via KEY_UP (PG.02) and KEY_DOWN (PG.03) buttons
#define ROTARY_ENCODER_NAVIGATION
#define ROTARY_ENCODER_GPIO             GPIOG
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_2 // PG.02 KEY_UP
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_3 // PG.03 KEY_DOWN
#define ROTARY_ENCODER_TIMER            TIM17
#define ROTARY_ENCODER_TIMER_IRQn       TIM17_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM17_IRQHandler

// Direct switches
#define SWITCHES_GPIO_REG_E             GPIOF
#define SWITCHES_GPIO_PIN_E             LL_GPIO_PIN_8

#define SWITCHES_GPIO_REG_F             GPIOF
#define SWITCHES_GPIO_PIN_F             LL_GPIO_PIN_6

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


#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_1      // PA.01
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_0      // PA.00
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_3      // PA.03
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_2      // PA.02

#define ADC_GPIO_PIN_SWA                LL_GPIO_PIN_2      // PC.02
#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_3      // PC.03
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_9      // PF.09
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_7      // PF.07

//S1,S2
#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_6      // PA.06 VRA
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 VRB
//LS,LR
#define ADC_GPIO_PIN_SLIDER1            LL_GPIO_PIN_0      // PB.00 VRC/LS
#define ADC_GPIO_PIN_SLIDER2            LL_GPIO_PIN_7      // PA.07 VRD/RS

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_STICK_LH | \
   ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)

#define ADC_GPIOB_PINS (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER1)

#define ADC_GPIOC_PINS (ADC_GPIO_PIN_SWA | ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_BATT)

#define ADC_GPIOF_PINS \
  (ADC_GPIO_PIN_SWC | ADC_GPIO_PIN_SWD  )

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_17    // ADC12_INP17

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_SLIDER1             LL_ADC_CHANNEL_9    // ADC12_INP9
#define ADC_CHANNEL_SLIDER2             LL_ADC_CHANNEL_7    // ADC12_INP7

// Analog switches
#define ADC_CHANNEL_SWA                 LL_ADC_CHANNEL_0    // ADC3_INP0
#define ADC_CHANNEL_SWB                 LL_ADC_CHANNEL_1    // ADC3_INP1
#define ADC_CHANNEL_SWC                 LL_ADC_CHANNEL_2    // ADC3_INP2
#define ADC_CHANNEL_SWD                 LL_ADC_CHANNEL_3    // ADC3_INP3

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
#define ADC_EXT_CHANNELS                                                  \
{                                                                         \
  ADC_CHANNEL_SWA, ADC_CHANNEL_SWB, ADC_CHANNEL_SWC, ADC_CHANNEL_SWD,     \
  ADC_CHANNEL_BATT, ADC_CHANNEL_RTC_BAT \
}

#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_64CYCLES_5

#define ADC_VREF_PREC2                  329

#define ADC_DIRECTION {     \
-1,0,-1,0, /* gimbals */  \
0,0,     /* pots */       \
-1,0,    /* sliders */    \
0,	   /* vbat */       \
0,       /* rtc_bat */    \
0,       /* SWA */        \
0,       /* SWB */        \
0,       /* SWC */        \
0        /* SWD */        \
}

#define PWM_STICKS
#define PWM_TIMER                   TIM5
#define PWM_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define PWM_GPIO                    GPIOA
#define PWM_GPIO_AF                 GPIO_AF2

#define PWM_IRQHandler              TIM5_IRQHandler
#define PWM_IRQn                    TIM5_IRQn
#define PWM_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)

#define STICK_PWM_CHANNEL_LH          0
#define STICK_PWM_CHANNEL_LV          1
#define STICK_PWM_CHANNEL_RV          2
#define STICK_PWM_CHANNEL_RH          3

#define USE_EXTI9_5_IRQ // used for I2C port extender interrupt
#define EXTI9_5_IRQ_Priority 5

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOB, 3)  // PB.03
#define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 8)  // PI.08

// Charger
#define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOD, 4) // PD.04

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

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
#define TELEMETRY_USE_CUSTOM_EXTI
#define CUSTOM_EXTI_IRQ_NAME ETH_WKUP_IRQ
#define ETH_WKUP_IRQ_Priority 5
#define CUSTOM_EXTI_IRQ_LINE 86
#define TELEMETRY_RX_FRAME_EXTI_LINE    CUSTOM_EXTI_IRQ_LINE

// USB
#define USB_GPIO                        GPIOA
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOC, 0)  // PC.00
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6
#define LCD_DISP_PORT                   GPIOC
#define LCD_GPIO_NRST                   GPIO_PIN(GPIOC, 7) // PC.7     OK

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

/*
// SPI NOR Flash
#define FLASH_SPI                      SPI6
#define FLASH_SPI_CS_GPIO              GPIOG
#define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_6  // PG.06
#define FLASH_SPI_GPIO                 GPIOG
#define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_13 // PG.13
#define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_12 // PG.12
#define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_14 // PG.14
*/
#define SD_PRESENT_GPIO                GPIO_PIN(GPIOD, 3) // PD.03
#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_INIT_CLK_DIV           SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(20000000)
#define STORAGE_USE_SDIO


// Audio
#define AUDIO_OUTPUT_GPIO             GPIO_PIN(GPIOA, 4) // PA.04
#define AUDIO_DAC                     DAC1
#define AUDIO_DMA_Stream              LL_DMA_STREAM_1
#define AUDIO_DMA_Channel             LL_DMAMUX1_REQ_DAC1_CH1

#define AUDIO_TIM_IRQn                TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler          TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQn         DMA1_Stream1_IRQn
#define AUDIO_DMA_Stream_IRQHandler   DMA1_Stream1_IRQHandler
#define AUDIO_TIMER                   TIM6
#define AUDIO_DMA                     DMA1

#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOD, 7) // PD.07
#define INVERTED_MUTE_PIN
#define AUDIO_UNMUTE_DELAY            100
#define AUDIO_MUTE_DELAY              500


// I2C Bus  touch
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 7)  // PB.07   OK
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08   OK
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

//PCA9555
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

// LED Strip
#define LED_STRIP_LENGTH                  27
#define BLING_LED_STRIP_START             0
#define BLING_LED_STRIP_LENGTH            27

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
#define GPIO_LED_GPIO_ON                  gpio_set
#define GPIO_LED_GPIO_OFF                 gpio_clear
#define LED_RED_GPIO                      GPIO_PIN(GPIOG, 10)
#define LED_GREEN_GPIO                    GPIO_PIN(GPIOG, 12)
#define LED_BLUE_GPIO                     GPIO_PIN(GPIOG, 13)


// Internal Module
#define INTMODULE_BOOTCMD_BSP
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

// c14 ext-module is wired to USART2 on PD.5 (TX) / PD.6 (RX). PPM via timer is
// not supported: PD.5 has no TIM4 alternate function. EXTMODULE_TIMER is
// intentionally left undefined so the timer-pulse port is compiled out of
// module_ports.cpp (the `extmoduleTimer` block there is guarded by
// `#if defined(EXTMODULE_TIMER)`); the module uses USART only.
#define EXTMODULE
// Module is powered by BSP
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOD, 5)
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOD, 6)

//USART
#define EXTMODULE_USART                    USART2
#define EXTMODULE_USART_RX_GPIO            EXTMODULE_RX_GPIO
#define EXTMODULE_USART_TX_GPIO            EXTMODULE_TX_GPIO
#define EXTMODULE_USART_TX_DMA             DMA1
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART2_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_4

#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART2_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_2

#define EXTMODULE_USART_IRQHandler         USART2_IRQHandler
#define EXTMODULE_USART_IRQn               USART2_IRQn

// Trainer Port
#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOE, 5) // PE.05
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOE, 6) // PE.06
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

#define TRAINER_TIMER                   TIM15
#define TRAINER_TIMER_IRQn              TIM15_IRQn
#define TRAINER_TIMER_IRQHandler        TIM15_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_4
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Touch
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOI, 3)  // PI.03   OK
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOI, 11)  // PI.11   OK
#define TOUCH_INT_EXTI_Line             LL_EXTI_LINE_3
#define TOUCH_INT_EXTI_Port             LL_SYSCFG_EXTI_PORTI
#define TOUCH_INT_EXTI_SysCfgLine       LL_SYSCFG_EXTI_LINE3
#define USE_EXTI3_IRQ
#define EXTI3_IRQ_Priority  9
#define GT911_ROTATION_MODE             180

#define TOUCH_I2C_CLK_RATE              100000

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
#define LCD_H                           272

#define LCD_PHYS_W                      LCD_W
#define LCD_PHYS_H                      LCD_H

#define LCD_DEPTH                       16

#define LSE_DRIVE_STRENGTH     RCC_LSEDRIVE_HIGH

// IMU
#define IMU_I2C_BUS                     I2C_Bus_2
#define IMU_INT_GPIO                    GPIO_PIN(GPIOH, 4) // PH.04
// IMU_INT_EXTI IRQ
#if !defined(USE_EXTI4_IRQ)
  #define USE_EXTI4_IRQ
  #define EXTI4_IRQ_Priority       6
#endif
