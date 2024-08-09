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
 * TIM7 = 2MHz counter
 *
 *
 * TIM14 = 1ms counter
 */

/* DMA Allocation:
   DMA/Stream/Channel
   1/5/7 DAC/Audio
   2/0/0 ADC1
   2/3/4 SDIO
*/


// Trims
#define TRIMS_GPIO_REG_RHL              GPIOD
#define TRIMS_GPIO_PIN_RHL              LL_GPIO_PIN_7  // PD.07
#define TRIMS_GPIO_REG_RHR              GPIOG
#define TRIMS_GPIO_PIN_RHR              LL_GPIO_PIN_10 // PG.10
#define TRIMS_GPIO_REG_RVD              GPIOJ
#define TRIMS_GPIO_PIN_RVD              LL_GPIO_PIN_0  // PJ.00
#define TRIMS_GPIO_REG_RVU              GPIOB
#define TRIMS_GPIO_PIN_RVU              LL_GPIO_PIN_15 // PB.15

#define KEYS_GPIO_REG_ENTER             GPIOC
#define KEYS_GPIO_PIN_ENTER             LL_GPIO_PIN_13 // PC.13

#define TRIMS_GPIO_REG_LHL              GPIOH
#define TRIMS_GPIO_PIN_LHL              LL_GPIO_PIN_2  // PH.02
#define TRIMS_GPIO_REG_LHR              GPIOG
#define TRIMS_GPIO_PIN_LHR              LL_GPIO_PIN_2  // PG.02
#define TRIMS_GPIO_REG_LVU              GPIOH
#define TRIMS_GPIO_PIN_LVU              LL_GPIO_PIN_7  // PH.07
#define TRIMS_GPIO_REG_LVD              GPIOJ
#define TRIMS_GPIO_PIN_LVD              LL_GPIO_PIN_12 // PJ.12

#define KEYS_GPIO_REG_EXIT              GPIOG
#define KEYS_GPIO_PIN_EXIT              LL_GPIO_PIN_11 // PG.11

// Monitor pin
#define VBUS_MONITOR_GPIO               GPIO_PIN(GPIOJ, 14) // PJ.14

// Switches
#define HARDWARE_SWITCH_A
#define STORAGE_SWITCH_A
#define HARDWARE_SWITCH_B
#define STORAGE_SWITCH_B
#define HARDWARE_SWITCH_C
#define STORAGE_SWITCH_C
#define HARDWARE_SWITCH_D
#define STORAGE_SWITCH_D
#define HARDWARE_SWITCH_E
#define STORAGE_SWITCH_E
#define HARDWARE_SWITCH_F
#define STORAGE_SWITCH_F
#define HARDWARE_SWITCH_G
#define STORAGE_SWITCH_G
#define HARDWARE_SWITCH_H
#define STORAGE_SWITCH_H

// Index of all switches / trims
#define KEYS_GPIO_ACTIVE_HIGH
#define TRIMS_GPIO_ACTIVE_HIGH

// ADC

#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_2      // PA.02
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_3      // PA.03
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_4      // PA.04
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_5      // PA.05

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_6      // PA.06 VRA
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_4      // PC.04 VRB
#define ADC_GPIO_PIN_SWA                LL_GPIO_PIN_1      // PB.01
#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_8      // PF.08
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_0      // PB.00
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_10     // PF.10
#define ADC_GPIO_PIN_SWE                LL_GPIO_PIN_0      // PC.00
#define ADC_GPIO_PIN_SWF                LL_GPIO_PIN_1      // PC.01
#define ADC_GPIO_PIN_SWG                LL_GPIO_PIN_2      // PC.02
#define ADC_GPIO_PIN_SWH                LL_GPIO_PIN_7      // PA.07

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_5      // PC.05

// FLYSKY_HALL_STICKS
// #define ADC_GPIOA_PINS_FS               (GPIO_Pin_6 | GPIO_Pin_7)
#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH \
                                         | ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SWH)
#define ADC_GPIOB_PINS                  (ADC_GPIO_PIN_SWA | ADC_GPIO_PIN_SWC)
#define ADC_GPIOC_PINS \
  (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SWE | ADC_GPIO_PIN_SWF | ADC_GPIO_PIN_SWG | ADC_GPIO_PIN_BATT)
#define ADC_GPIOF_PINS                  (ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_SWD)

#define ADC_CHANNEL_STICK_LH             LL_ADC_CHANNEL_2  // ADC123_IN2 -> ADC1_IN2
#define ADC_CHANNEL_STICK_LV             LL_ADC_CHANNEL_3  // ADC123_IN3 -> ADC1_IN3
#define ADC_CHANNEL_STICK_RH             LL_ADC_CHANNEL_4  // ADC12_IN4  -> ADC1_IN4
#define ADC_CHANNEL_STICK_RV             LL_ADC_CHANNEL_5  // ADC12_IN5  -> ADC1_IN5

#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_6   // ADC12_IN6  -> ADC1_IN6
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_14  // ADC12_IN14 -> ADC1_IN14
#define ADC_CHANNEL_SWA                 LL_ADC_CHANNEL_9   // ADC12_IN9  -> ADC1_IN9
#define ADC_CHANNEL_SWB                 LL_ADC_CHANNEL_6   // ADC3_IN6   -> ADC3_IN6
#define ADC_CHANNEL_SWC                 LL_ADC_CHANNEL_8   // ADC12_IN8  -> ADC1_IN8
#define ADC_CHANNEL_SWD                 LL_ADC_CHANNEL_8   // ADC3_IN8   -> ADC3_IN8
#define ADC_CHANNEL_SWE                 LL_ADC_CHANNEL_10  // ADC123_IN10-> ADC1_IN10
#define ADC_CHANNEL_SWF                 LL_ADC_CHANNEL_11  // ADC123_IN11-> ADC1_IN11
#define ADC_CHANNEL_SWG                 LL_ADC_CHANNEL_12  // ADC123_IN12-> ADC1_IN12
#define ADC_CHANNEL_SWH                 LL_ADC_CHANNEL_7   // ADC12_IN7  -> ADC1_IN7

#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_15  // ADC12_IN15 -> ADC1_IN15
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC1_IN18

#define ADC_MAIN                        ADC1
#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                { ADC_CHANNEL_SWB, ADC_CHANNEL_SWD }
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_28CYCLES
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMA_CHANNEL_0
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler

#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMA_CHANNEL_2
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_28CYCLES
#define ADC_VREF_PREC2                  660

#define ADC_DIRECTION                                                   \
    { 0 /*STICK1*/, 0 /*STICK2*/, 0 /*STICK3*/, 0 /*STICK4*/,           \
      -1 /*POT1*/, 0 /*POT2*/, 0 /*TX_VOLTAGE*/, 0 /*TX_VBAT*/,		\
      0 /*SWA*/, 0 /*SWB*/, 0 /*SWC*/, 0 /*SWD*/, 0 /*SWE*/,		\
      -1 /*SWF*/,  -1 /*SWG*/, 0 /*SWH*/				\
    }

// Power
#define PWR_SWITCH_GPIO                 GPIO_PIN(GPIOI, 11) // PI.11
#define PWR_ON_GPIO                     GPIO_PIN(GPIOI, 14) // PI.14

// S.Port update connector
#define SPORT_MAX_BAUDRATE              400000
#define SPORT_UPDATE_RCC_AHB1Periph     0

// Led
// #define STATUS_LEDS
#define LED_RCC_AHB1Periph              RCC_AHB1Periph_GPIOI
#define LED_GPIO                        GPIOI
#define LED_GPIO_PIN                    GPIO_Pin_5  // PI.05

// Serial Port (DEBUG)
// We will temporarily used the PPM and the HEARTBEAT PINS
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
// #define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_5
// #define TELEMETRY_DMA_Channel_RX        LL_DMA_CHANNEL_4


// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOA, 9)  // PA.09
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10
#define USB_SW_GPIO                     GPIO_PIN(GPIOI, 10) // PI.10

// LCD
#define LCD_NRST_GPIO                   GPIO_PIN(GPIOG, 9) // PG.09
#define LCD_SPI_CS_GPIO                 GPIO_PIN(GPIOE, 4)  // PE.04
#define LCD_SPI_SCK_GPIO                GPIO_PIN(GPIOE, 2)  // PE.02
#define LCD_SPI_MISO_GPIO               GPIO_PIN(GPIOE, 5)  // PE.05
#define LCD_SPI_MOSI_GPIO               GPIO_PIN(GPIOE, 6)  // PE.06
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
// TODO TIM3, TIM8, TIM14, review the channel in backlight_driver.cpp according to the chosen timer
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOA, 15) // PA.15
#define BACKLIGHT_TIMER                 TIM2
#define BACKLIGHT_GPIO_AF               GPIO_AF1
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

//used in BOOTLOADER
#define SERIAL_RCC_AHB1Periph           0
#define SERIAL_RCC_APB1Periph           0
#define ROTARY_ENCODER_RCC_APB1Periph   0

// SD card
#define SD_PRESENT_GPIO                 GPIO_PIN(GPIOH, 10) // PH.10
#define SD_SDIO_DMA                     DMA2
#define SD_SDIO_DMA_STREAM              DMA2_Stream3
#define SD_SDIO_DMA_CHANNEL             LL_DMA_CHANNEL_4
#define SD_SDIO_DMA_IRQn                DMA2_Stream3_IRQn
#define SD_SDIO_DMA_IRQHANDLER          DMA2_Stream3_IRQHandler
#define SD_SDIO_CLK_DIV(fq)             ((48000000 / (fq)) - 2)
#define SD_SDIO_INIT_CLK_DIV            SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV        SD_SDIO_CLK_DIV(24000000)
#define STORAGE_USE_SDIO

// SPI NOR Flash 
#define FLASH_SPI                      SPI6
#define FLASH_SPI_CS_GPIO              GPIOG
#define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_6  // PG.06
#define FLASH_SPI_GPIO                 GPIOG
#define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_13 // PG.13
#define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_12 // PG.12
#define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_14 // PG.14
#define FLASH_SPI_DMA                  DMA2
#define FLASH_SPI_DMA_CHANNEL          LL_DMA_CHANNEL_1
#define FLASH_SPI_DMA_TX_STREAM        LL_DMA_STREAM_5
#define FLASH_SPI_DMA_RX_STREAM        LL_DMA_STREAM_6

// SPI FLASH
#define EEPROM_RCC_AHB1Periph           RCC_AHB1Periph_GPIOG
#define EEPROM_RCC_APB1Periph           RCC_APB1Periph_SPI6
#define EEPROM_SPI_CS_GPIO              GPIOG
#define EEPROM_SPI_CS_GPIO_PIN          GPIO_Pin_6 // PG.06
#define EEPROM_SPI_SCK_GPIO             GPIOG
#define EEPROM_SPI_SCK_GPIO_PIN         GPIO_Pin_13 // PG.13
#define EEPROM_SPI_SCK_GPIO_PinSource   GPIO_PinSource13
#define EEPROM_SPI_MISO_GPIO            GPIOG
#define EEPROM_SPI_MISO_GPIO_PIN        GPIO_Pin_12 // PG.12
#define EEPROM_SPI_MISO_GPIO_PinSource  GPIO_PinSource12
#define EEPROM_SPI_MOSI_GPIO            GPIOG
#define EEPROM_SPI_MOSI_GPIO_PIN        GPIO_Pin_14 // PG.14
#define EEPROM_SPI_MOSI_GPIO_PinSource  GPIO_PinSource14

// Audio
#define AUDIO_XDCS_GPIO               GPIOH
#define AUDIO_XDCS_GPIO_PIN           LL_GPIO_PIN_14 // PH.14
#define AUDIO_CS_GPIO                 GPIOH
#define AUDIO_CS_GPIO_PIN             LL_GPIO_PIN_13 // PH.13
#define AUDIO_DREQ_GPIO               GPIOH
#define AUDIO_DREQ_GPIO_PIN           LL_GPIO_PIN_15 // PH.15
#define AUDIO_RST_GPIO                GPIOD
#define AUDIO_RST_GPIO_PIN            LL_GPIO_PIN_4  // PD.04
#define AUDIO_SPI                     SPI1
#define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
#define AUDIO_SPI_SCK_GPIO            GPIOB
#define AUDIO_SPI_SCK_GPIO_PIN        LL_GPIO_PIN_3  // PB.03
#define AUDIO_SPI_MISO_GPIO           GPIOB
#define AUDIO_SPI_MISO_GPIO_PIN       LL_GPIO_PIN_4  // PB.04
#define AUDIO_SPI_MOSI_GPIO           GPIOB
#define AUDIO_SPI_MOSI_GPIO_PIN       LL_GPIO_PIN_5  // PB.05
#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOH, 8) // PH.08 audio amp control pin
#define AUDIO_UNMUTE_DELAY            120  // ms
#define AUDIO_MUTE_DELAY              500  // ms
#define INVERTED_MUTE_PIN

// I2C Bus
#define I2C_B1                        I2C1
#define I2C_B1_SCL_GPIO               GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_SDA_GPIO               GPIO_PIN(GPIOB, 7)  // PB.07
#define I2C_B1_GPIO_AF                LL_GPIO_AF_4
#define I2C_B1_CLK_RATE               400000

// Touch
#define TOUCH_I2C_BUS                 I2C_Bus_1
#define TOUCH_INT_GPIO                GPIO_PIN(GPIOB, 9)  // PB.09
#define TOUCH_RST_GPIO                GPIO_PIN(GPIOB, 12) // PB.12

#define TOUCH_INT_EXTI_Line           LL_EXTI_LINE_9
#define TOUCH_INT_EXTI_Port           LL_SYSCFG_EXTI_PORTB
#define TOUCH_INT_EXTI_SysCfgLine     LL_SYSCFG_EXTI_LINE9

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI9_5_IRQ)
  #define USE_EXTI9_5_IRQ
  #define EXTI9_5_IRQ_Priority  9
#endif

// Haptic: TIM1_CH1
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOA, 8) // PA.08
#define HAPTIC_GPIO_TIMER               TIM1
#define HAPTIC_GPIO_AF                  GPIO_AF1
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC1E | TIM_CCER_CC1NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR1

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 UART4
#define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOA, 0) // PA.00
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOA, 1) // PA.01
#define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4

// Internal Module
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


// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOD, 11) // PD.11
#define EXTMODULE_PWR_FIX_GPIO          GPIO_PIN(GPIOA, 2)  // PA.02
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)  // PC.07
#define EXTMODULE_TIMER                 TIM8
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_3 // TIM8_CH1
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
#define EXTMODULE_TIMER_IRQn            TIM8_UP_TIM13_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM8_UP_TIM13_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
//USART
#define EXTMODULE_USART                    USART6
#define EXTMODULE_USART_GPIO               GPIOC
#define EXTMODULE_USART_GPIO_AF            GPIO_AF_USART6
#define EXTMODULE_USART_GPIO_AF_LL         LL_GPIO_AF_8
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMA_CHANNEL_5
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_7
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMA_CHANNEL_5
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_2
#define EXTMODULE_USART_IRQn               USART6_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMA_CHANNEL_7
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_1
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream1_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream1_IRQHandler

#define EXTMODULE_TX_INVERT_GPIO           GPIO_PIN(GPIOE, 3)  // PE.03
#define EXTMODULE_RX_INVERT_GPIO           GPIO_PIN(GPIOI, 15) // PI.15

// Trainer Port
#define TRAINER_GPIO                    GPIOD

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
#define BLUETOOTH_ON_GPIO               GPIO_PIN(GPIOI, 8) // PI.8
#define BT_USART                        USART3
#define BT_USART_IRQn                   USART3_IRQn
#define BT_GPIO_TXRX                    GPIOB
#define BT_TX_GPIO                      GPIO_PIN(GPIOB, 10) // PB.10
#define BT_RX_GPIO                      GPIO_PIN(GPIOB, 11) // PB.11
#define BT_EN_GPIO                      GPIO_PIN(GPIOI, 8)  // PI.08
#define BT_CONNECTED_GPIO               GPIO_PIN(GPIOJ, 10) // PJ.10
#define BT_CMD_MODE_GPIO                GPIO_PIN(GPIOH, 6)  // PH.06

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

#define PORTRAIT_LCD true
#define LANDSCAPE_LCD false

#define LCD_W                           320
#define LCD_H                           480

#define LCD_PHYS_W                      320
#define LCD_PHYS_H                      480

#define LCD_DEPTH                       16
#define LCD_CONTRAST_DEFAULT            20
