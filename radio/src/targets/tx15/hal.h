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
Stream2:  FLYSKY_HALL_DMA_Stream_RX
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  I2S_DMA_Stream
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:  ADC_EXT_DMA_STREAM (do not move)
Stream1:
Stream2:
Stream3:  EXTMODULE_TIMER_DMA_STREAM
Stream4:  ADC_DMA_STREAM (do not move)
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM
Stream7:

TIM1:	BACKLIGHT_TIMER
TIM2:	LED_STRIP_TIMER
TIM3:	HAPTIC_GPIO_TIMER
TIM4:
TIM5:	EXTMODULE_TIMER
TIM7:
TIM8:	TRAINER_TIMER
TIM12:	MIXER_SCHEDULER_TIMER
TIM14:	MS_TIMER
TIM15:
TIM16:
TIM17:	ROTARY_ENCODER_TIMER
 */

#ifndef _HAL_H_
#define _HAL_H_

#define CPU_FREQ                480000000

#define PERI1_FREQUENCY         120000000
#define PERI2_FREQUENCY         120000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

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
#define FUNCTION_SWITCH_1               I2C
#define FUNCTION_SWITCH_2               I2C
#define FUNCTION_SWITCH_3               I2C
#define FUNCTION_SWITCH_4               I2C
#define FUNCTION_SWITCH_5               I2C
#define FUNCTION_SWITCH_6               I2C

// Extender Switches
#define SWITCHES_A_3POS
#define SWITCHES_B_3POS
#define SWITCHES_C_3POS
#define SWITCHES_D_3POS
#define SWITCHES_E_3POS
#define SWITCHES_F_3POS

// Direct switches
#if 0
// Key 1
#define STORAGE_SWITCH_G
#define HARDWARE_SWITCH_G
#define SWITCHES_G_2POS
//#define SWITCHES_GPIO_REG_G           GPIOI
//#define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_3  // PI.03

// Key 2
#define STORAGE_SWITCH_H
#define HARDWARE_SWITCH_H
#define SWITCHES_H_2POS
//#define SWITCHES_GPIO_REG_H           GPIOI
//#define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_11 // PI.11


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
#endif


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

#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_LH)
#define ADC_GPIOB_PINS                  (ADC_GPIO_PIN_POT2)
#define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_POT1)
#define ADC_GPIOH_PINS                  (ADC_GPIO_PIN_BATT)


#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_13   // ADC12_INP13
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_4    // ADC12_INP4
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_8    // ADC12_INP8

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

#define ADC_DIRECTION {       	 \
    0, 0, -1,-1,/* gimbals */    \
    -1,-1,   	/* pots */       \
    0,0,     	/* sliders */    \
    0,	     	/* vbat */       \
    0,       	/* rtc_bat */    \
    0,       	/* SWA */        \
    0,       	/* SWB */        \
    0,       	/* SWC */        \
    0,       	/* SWD */        \
    0,       	/* SWE */        \
    0        	/* SWF */        \
  }

// Serial gimbal sync port
#define HALL_SYNC                   GPIO_PIN(GPIOH, 11)

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
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOH, 5)
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11)
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12)
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#define LCD_SPI_CS_GPIO                 GPIOA
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_7
#define LCD_SPI_SCK_GPIO                GPIOB
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_0
#define LCD_SPI_MOSI_GPIO               GPIOI
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_9

#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

#define LCD_RESET_GPIO                  GPIOJ
#define LCD_RESET_GPIO_PIN              LL_GPIO_PIN_12

#define LCD_TE_GPIO                     GPIOI
#define LCD_TE_GPIO_PIN                 LL_GPIO_PIN_4

// Backlight
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOA, 10) // TIM1_CH3
#define BACKLIGHT_TIMER                 TIM1
#define BACKLIGHT_TIMER_CHANNEL			LL_TIM_CHANNEL_CH3
#define BACKLIGHT_GPIO_AF               GPIO_AF1
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

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
#define SD_SDIO_INIT_CLK_DIV            SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV        SD_SDIO_CLK_DIV(20000000)

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

#define STORAGE_USE_SDIO


// AUDIO
#define AUDIO_SPI                       SPI2
#define AUDIO_RESET_PIN                 GPIO_PIN(GPIOH, 10)
#define AUDIO_HP_DETECT_PIN             GPIO_PIN(GPIOA, 5)
#define I2S_DMA                   	DMA1
#define I2S_DMA_Stream            	LL_DMA_STREAM_4
#define I2S_DMA_Stream_Request    	LL_DMAMUX1_REQ_SPI2_TX
#define I2S_DMA_Stream_IRQn       	DMA1_Stream4_IRQn
#define I2S_DMA_Stream_IRQHandler 	DMA1_Stream4_IRQHandler


// I2C Bus
#define I2C_B1                          I2C4
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOD, 13) // PD.13
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOD, 12) // PD.12
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000
/*
#define I2C_B2                          I2C3
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOH, 8)  // PH.08
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOH, 7)  // PH.07
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000
*/
// Haptic: TIM3_CH2
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOC, 7)
#define HAPTIC_GPIO_TIMER               TIM3
#define HAPTIC_GPIO_AF                  GPIO_AF2
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 UART4

#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOB, 9)
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOB, 8)
#define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn

#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Channel                  LL_DMAMUX1_REQ_UART4_RX

// LED Strip
#define LED_STRIP_LENGTH                  26  // 6POS + 2 rings of 10
#define LED_RGB_OFFSET                    6
#define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 0)  // PA.00 / TIM2_CH1
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
#define GPIO_LED_GPIO_ON                  gpio_clear
#define GPIO_LED_GPIO_OFF                 gpio_set
#define LED_RED_GPIO                      GPIO_PIN(GPIOI, 8)   // PI.08
#define LED_GREEN_GPIO                    GPIO_PIN(GPIOI, 11)  // PI.11
#define LED_BLUE_GPIO                     GPIO_PIN(GPIOI, 10)  // PI.10

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
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH3
#define EXTMODULE_TIMER_IRQn            TIM5_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM5IRQHandler
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
// TODO
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM5_CH3
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_3
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream1_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream1_IRQHandler

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


// Touch
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOE, 2) // PE.02
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOJ, 13) // PJ.13

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI2_IRQ)
#define USE_EXTI2_IRQ
#define EXTI2_IRQ_Priority  9
#endif

#define IMU_I2C_BUS                     I2C_Bus_1
#define IMU_I2C_ADDRESS                 0x6A

//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION
// Rotary Encoder
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO_A           GPIOI
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_7
#define ROTARY_ENCODER_GPIO_B           GPIOJ
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_8
//#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 2) & 0x03)
#define ROTARY_ENCODER_POSITION()       (((ROTARY_ENCODER_GPIO_A->IDR >> 7) & 0x01)|((ROTARY_ENCODER_GPIO_B->IDR >> 7) & 0x02))
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_7
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_8
#if !defined(USE_EXTI7_IRQ)
  #define USE_EXTI7_IRQ
  #define EXTI7_IRQ_Priority 5
#endif
#if !defined(USE_EXTI8_IRQ)
  #define USE_EXTI8_IRQ
  #define EXTI8_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT_A      LL_SYSCFG_EXTI_PORTI
#define ROTARY_ENCODER_EXTI_PORT_B      LL_SYSCFG_EXTI_PORTJ
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE7
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE8
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
#define LANDSCAPE_LCD_SML false
#define LANDSCAPE_LCD_STD true
#define LANDSCAPE_LCD_LRG false

#define LCD_W                           480
#define LCD_H                           320

#define LCD_PHYS_W                      LCD_H
#define LCD_PHYS_H                      LCD_W

#define LCD_DEPTH                       16

#define LSE_DRIVE_STRENGTH  RCC_LSEDRIVE_HIGH

#endif // _HAL_H_
