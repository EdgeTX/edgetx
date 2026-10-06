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

// FlySky G11P (STM32H750IBT6, LQFP176) - surface radio on the ST16 platform.
//
// This HAL is derived from the ST16 HAL; only the nets that differ on the G11P
// schematic are overridden here. See st16-vs-g11p-pinout.md for the pin map.
//
// TODO(hardware): ADC channel assignments for the new G11P analog lines
// (PC0 = 2S_ADC, PC5 = TR_ADC) and the exact analog input ordering
// (ADC_DIRECTION) need to be validated on hardware.

#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

// Keys are decoded by g11p_key_driver.cpp from ADC ladders (K12_ADC/K34_ADC,
// K6A/K7A GPIO); no generic GPIO keys.
#define KEYS_GPIO_REG_ENTER
#define KEYS_GPIO_PIN_ENTER
#define KEYS_GPIO_REG_PAGEDN
#define KEYS_GPIO_PIN_PAGEDN
#define KEYS_GPIO_REG_EXIT
#define KEYS_GPIO_PIN_EXIT

// Digital keys / switch
#define G11P_K6_GPIO                    GPIOC
#define G11P_K6_PIN                     LL_GPIO_PIN_7   // PC.07 K6A
#define G11P_K7_GPIO                    GPIOA
#define G11P_K7_PIN                     LL_GPIO_PIN_8   // PA.08 K7A
#define G11P_SW1_GPIO                   GPIOH
#define G11P_SW1_PIN                    LL_GPIO_PIN_4   // PH.04 SW1 (digital)

// Switches (positions are provided by g11p_switch_driver.cpp)
#define SWITCHES_A_2POS
#define SWITCHES_B_3POS
#define SWITCHES_C_3POS
#define SWITCHES_D_3POS

// Trims are decoded by g11p_key_driver.cpp from the TRx_ADC lines.
// (all TRIMS_GPIO_REG_*/PIN_* left undefined)

// ADC
//
// Analog input order (this defines the getAnalogValue() indices used by the
// custom g11p_key_driver / g11p_switch_driver):
//
//   0  ST        PA1        8  RAW6 = TR    PC5
//   1  TH        PA0        9  RAW7 = TR1   PF8
//   2  POT1=VR2  PC2       10  RAW8 = TR2   PF9
//   3  RAW1=K12  PB0       11  RAW9 = TR3   PA2
//   4  RAW2=K34  PB1       12  RAW10= TR4   PA3
//   5  RAW3=SW2  PA6       13  RAW11= 2S    PC0
//   6  RAW4=SW3  PA7       14  BATT = LIBAT PC1
//   7  RAW5=SW4  PC3       15  RTC_BAT
//
// TODO(hardware): all channel assignments below are derived from the ST16 map
// and must be validated (PC0/PC5 channels are guesses).
#define ADC_GPIO_PIN_STICK_ST           LL_GPIO_PIN_1      // PA.01 ST_ADC
#define ADC_GPIO_PIN_STICK_TH           LL_GPIO_PIN_0      // PA.00 TH_ADC
#define ADC_CHANNEL_STICK_ST            LL_ADC_CHANNEL_15  // ADC12_INP15
#define ADC_CHANNEL_STICK_TH            LL_ADC_CHANNEL_14  // ADC12_INP14

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_2      // PC.02 VR2A
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_0   // ADC3_INP0

#define ADC_GPIO_PIN_RAW1               LL_GPIO_PIN_0      // PB.00 K12_ADC
#define ADC_GPIO_PIN_RAW2               LL_GPIO_PIN_1      // PB.01 K34_ADC
#define ADC_GPIO_PIN_RAW3               LL_GPIO_PIN_6      // PA.06 SW2_ADC
#define ADC_GPIO_PIN_RAW4               LL_GPIO_PIN_7      // PA.07 SW3_ADC
#define ADC_GPIO_PIN_RAW5               LL_GPIO_PIN_3      // PC.03 SW4_ADC
#define ADC_GPIO_PIN_RAW6               LL_GPIO_PIN_5      // PC.05 TR_ADC
#define ADC_GPIO_PIN_RAW7               LL_GPIO_PIN_8      // PF.08 TR1_ADC
#define ADC_GPIO_PIN_RAW8               LL_GPIO_PIN_9      // PF.09 TR2_ADC
#define ADC_GPIO_PIN_RAW9               LL_GPIO_PIN_2      // PA.02 TR3_ADC
#define ADC_GPIO_PIN_RAW10              LL_GPIO_PIN_3      // PA.03 TR4_ADC
#define ADC_GPIO_PIN_RAW11              LL_GPIO_PIN_0      // PC.00 2S_ADC
#define ADC_CHANNEL_RAW1                LL_ADC_CHANNEL_9   // ADC12_INP9
#define ADC_CHANNEL_RAW2                LL_ADC_CHANNEL_5   // ADC12_INP5
#define ADC_CHANNEL_RAW3                LL_ADC_CHANNEL_3   // ADC12_INP3
#define ADC_CHANNEL_RAW4                LL_ADC_CHANNEL_7   // ADC12_INP7
#define ADC_CHANNEL_RAW5                LL_ADC_CHANNEL_1   // ADC3_INP1
#define ADC_CHANNEL_RAW6                LL_ADC_CHANNEL_3   // ADC3_INP3 (TODO)
#define ADC_CHANNEL_RAW7                LL_ADC_CHANNEL_8   // ADC3_INP8
#define ADC_CHANNEL_RAW8                LL_ADC_CHANNEL_2   // ADC3_INP2
#define ADC_CHANNEL_RAW9                LL_ADC_CHANNEL_16  // ADC12_INP16
#define ADC_CHANNEL_RAW10               LL_ADC_CHANNEL_17  // ADC12_INP17
#define ADC_CHANNEL_RAW11               LL_ADC_CHANNEL_10  // ADC3_INP10 (TODO)

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01 LIBAT_ADC
#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_11  // ADC123_INP11

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_STICK_ST | ADC_GPIO_PIN_STICK_TH | ADC_GPIO_PIN_RAW3 | \
   ADC_GPIO_PIN_RAW4 | ADC_GPIO_PIN_RAW9 | ADC_GPIO_PIN_RAW10)
#define ADC_GPIOB_PINS (ADC_GPIO_PIN_RAW1 | ADC_GPIO_PIN_RAW2)
#define ADC_GPIOC_PINS                                                \
  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_RAW5 | ADC_GPIO_PIN_RAW6 |         \
   ADC_GPIO_PIN_RAW11 | ADC_GPIO_PIN_BATT)
#define ADC_GPIOF_PINS (ADC_GPIO_PIN_RAW7 | ADC_GPIO_PIN_RAW8)

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC12_IN16

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                                                    \
  {                                                                         \
    ADC_CHANNEL_POT1, ADC_CHANNEL_RAW5, ADC_CHANNEL_RAW6, ADC_CHANNEL_RAW7, \
    ADC_CHANNEL_RAW8, ADC_CHANNEL_RAW11, ADC_CHANNEL_BATT, ADC_CHANNEL_RTC_BAT \
  }

#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  660

// TODO(hardware): validate the analog input ordering/inversion on G11P.
#define ADC_DIRECTION {                 \
    0,0,   /* ST, TH */                 \
    0,     /* POT1 = VR2 */             \
    0,0,0,0,0,0, /* RAW1..RAW6 */       \
    0,0,0,0,0,   /* RAW7..RAW11 */      \
    0,     /* BATT */                   \
    0      /* RTC_BAT */                \
  }

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOB, 3)  // PB.03 PWR_SW
#define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 8)  // PI.08 PWR_CTR

// Chargers (USB)
#define UCHARGER_GPIO               GPIO_PIN(GPIOH, 6)  // PH.06 USB_SW
#define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOD, 4)  // PD.04 CHRG
#define CHARGE_EN_GPIO              GPIO_PIN(GPIOH, 7)  // PH.07 CHARGE_ENB
#define VBUS_DETECT_GPIO            GPIO_PIN(GPIOI, 3)  // PI.03 VBUS_I

#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

// Telemetry: G11P has no dedicated S.Port UART (AFHDS3 telemetry comes from
// the internal module), so TELEMETRY_USART is intentionally not defined.

// USB
#define USB_GPIO                        GPIOA
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOI, 3)  // PI.03 VBUS_I
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD (LTDC RGB pins are configured in lcd_driver.cpp)
#define LCD_SPI_CS_GPIO                 GPIOH
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_8  // PH.08 LCD_CS
#define LCD_SPI_GPIO                    GPIOE
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_4  // PE.04 LCD_SCL
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_3  // PE.03 LCD_SDA
#define LCD_NRST_GPIO                   GPIO_PIN(GPIOI, 11) // PI.11 LCD_NRST
#define LCD_ID_GPIO                     GPIO_PIN(GPIOC, 4)  // PC.04 LCD_ID
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight (TIM3_CH1)
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOC, 6) // PC.06
#define BACKLIGHT_TIMER                 TIM3
#define BACKLIGHT_GPIO_AF               GPIO_AF2
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// QSPI Flash
#define QSPI_MAX_FREQ                   80000000U
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

#define SD_PRESENT_GPIO                GPIO_PIN(GPIOD, 3) // PD.03
#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_INIT_CLK_DIV           SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(20000000)
#define STORAGE_USE_SDIO

// Audio (VS1053B on SPI1)
#define AUDIO_XDCS_GPIO               GPIO_PIN(GPIOG, 12) // PG.12 VS1053B_BSYNC
#define AUDIO_CS_GPIO                 GPIO_PIN(GPIOG, 10) // PG.10 SPI1_NCS
#define AUDIO_DREQ_GPIO               GPIO_PIN(GPIOG, 13) // PG.13 VS1053B_DREQ
#define AUDIO_RST_GPIO                GPIO_PIN(GPIOD, 6)  // PD.06 VS1053B_RST
#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05 PA_NMUTE
#define AUDIO_SPI                     SPI1
#define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
#define AUDIO_SPI_SCK_GPIO            GPIO_PIN(GPIOA, 5)  // PA.05
#define AUDIO_SPI_MISO_GPIO           GPIO_PIN(GPIOG, 9)  // PG.09
#define AUDIO_SPI_MOSI_GPIO           GPIO_PIN(GPIOD, 7)  // PD.07
#define AUDIO_SPI_DMA                 DMA1
#define AUDIO_SPI_DMA_REQ             LL_DMAMUX1_REQ_SPI1_TX
#define AUDIO_SPI_DMA_STREAM          LL_DMA_STREAM_1
#define AUDIO_UNMUTE_DELAY            180  // ms
#define AUDIO_MUTE_DELAY              200  // ms
#define INVERTED_MUTE_PIN

// I2C bus (touch)
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 9)  // PB.09 TP_SDA
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08 TP_SCL
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

// Haptic: TIM3_CH2 (MOTOR_PWM)
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOB, 5) // PB.05 MOTOR_PWM
#define HAPTIC_GPIO_TIMER               TIM3
#define HAPTIC_GPIO_AF                  GPIO_AF2
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// RGB LED strip (single WS2812 chain, 2 logical LEDs)
#define LED_STRIP_LENGTH                  2
#define LED_STRIP_RESERVED_AT_END         0
#define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 15)  // PA.15 LED_DATA_IN
#define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1
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
#define LED_CHARGING_START                0
#define LED_CHARGING_END                  1

// Internal RF module (AFHDS3) via the M0 (STM32F072) on USART3
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 11) // PB.11 M0_USART2_TX
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 10) // PB.10 M0_USART2_RX
#define INTMODULE_USART                 USART3
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART3_IRQn
#define INTMODULE_USART_IRQHandler      USART3_IRQHandler
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_5
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_3
#define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5

// RF module control
#define RF_PWR_ON_GPIO                  GPIO_PIN(GPIOE, 5)  // PE.05 RF_PWR_ON
#define M0_SYNC_GPIO                    GPIO_PIN(GPIOA, 10) // PA.10 M0_SYNC
#define RF_DFU_C_GPIO                   GPIO_PIN(GPIOB, 12) // PB.12 RF_DFU_C

// External module: exposed on the trainer port (UART7, PF6/PF7). No module bay
// and no PPM: the port is serial-only, but the shared module framework still
// requires a pulse timer definition (left at the ST16 values, unused).
#define EXTMODULE
#define EXTMODULE_TX_GPIO                  GPIO_PIN(GPIOF, 7)  // PF.07 PPM_OUT
#define EXTMODULE_RX_GPIO                  GPIO_PIN(GPIOF, 6)  // PF.06 PPM_IN
#define EXTMODULE_USART                    UART7
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOF, 7)
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOF, 6)
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART7_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6
#define EXTMODULE_USART_RX_DMA             DMA2
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART7_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5
#define EXTMODULE_USART_IRQHandler         UART7_IRQHandler
#define EXTMODULE_USART_IRQn               UART7_IRQn

#define EXTMODULE_TIMER                    TIM4
#define EXTMODULE_TIMER_Channel            LL_TIM_CHANNEL_CH4
#define EXTMODULE_TIMER_IRQn               TIM4_IRQn
#define EXTMODULE_TIMER_IRQHandler         TIM4_IRQHandler
#define EXTMODULE_TIMER_FREQ               (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF         LL_GPIO_AF_2
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM4_UP
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_6
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream6_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream6_IRQHandler

// Telemetry / S.Port: G11P has no dedicated connector, but the module
// framework requires a TELEMETRY_USART definition; kept on an unused UART.
#define TELEMETRY_TX_GPIO               GPIO_UNDEF
#define TELEMETRY_RX_GPIO               GPIO_UNDEF
#define TELEMETRY_USART                 UART5
#define TELEMETRY_USART_IRQn            UART5_IRQn
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_7
#define TELEMETRY_DMA_Channel_TX        LL_DMAMUX1_REQ_UART5_TX
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
#define TELEMETRY_USART_IRQHandler      UART5_IRQHandler

// CRSF telemetry frame detection uses a custom SW IRQ (same as ST16); the line
// is unused on G11P (no S.Port) but the CRSF module code requires it.
#define TELEMETRY_USE_CUSTOM_EXTI
#define CUSTOM_EXTI_IRQ_NAME ETH_WKUP_IRQ
#define ETH_WKUP_IRQ_Priority 5
#define CUSTOM_EXTI_IRQ_LINE 86
#define TELEMETRY_RX_FRAME_EXTI_LINE    CUSTOM_EXTI_IRQ_LINE

// Rotary encoder (W-A/W-B) with push (W-KEY)
#define ROTARY_ENCODER_NAVIGATION
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOG
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_2 // PG.02 W-A
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_3 // PG.03 W-B
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

// W-KEY (encoder push) - read as KEY_ENTER by g11p_key_driver.cpp
#define G11P_WKEY_GPIO                  GPIOG
#define G11P_WKEY_PIN                   LL_GPIO_PIN_14 // PG.14 W-KEY

// Touch (standard FlySky driver)
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_I2C_CLK_RATE              100000
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOE, 6)  // PE.06 TP_INT
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOI, 9)  // PI.09 TP_RESET

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_BRK_TIM12_IRQHandler

// LCD 320x480 portrait (MS0621-32TFL11E)
#define LCD_W                           320
#define LCD_H                           480

#define LCD_PHYS_W                      LCD_W
#define LCD_PHYS_H                      LCD_H

#define LCD_DEPTH                       16
