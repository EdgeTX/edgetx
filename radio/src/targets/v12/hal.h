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
DMA1
Stream0:  LED_STRIP_TIMER_DMA_STREAM
Stream1:  INTMODULE_DMA_STREAM
Stream2:  FLYSKY_HALL_DMA_Stream_RX
Stream3:  TELEMETRY_DMA_Stream_RX
Stream4:  ADC_DMA_STREAM
Stream5:  INTMODULE_RX_DMA_STREAM
Stream6:
Stream7:  TELEMETRY_DMA_Stream_TX

DMA2
Stream0:
Stream1:  AUDIO_DMA_Stream
Stream2:
Stream3:  EXTMODULE_TIMER_DMA_STREAM
Stream4:  ADC_DMA_STREAM
Stream5:  EXTMODULE_USART_RX_DMA_STREAM
Stream6:  EXTMODULE_USART_TX_DMA_STREAM

TIM1:     LED_STRIP_TIMER
TIM2:	  BACKLIGHT_TIMER
TIM3:     EXTMODULE_TIMER
TIM4:	  (no pins)
TIM5:     TRAINER_TIMER
TIM6:     AUDIO_TIMER
TIM7:	  (no pins)
TIM8:	  TRAINER_TIMER
TIM12:	  MIXER_SCHEDULER_TIMER
TIM13:
TIM14:    MS_TIMER
TIM15:    HAPTIC_GPIO_TIMER
TIM16:
TIM17:	  ROTARY_ENCODER_TIMER

USARTS
USART1: INTMODULE_USART
USART2: TELEMETRY_USART
USART3: DGONE
USART4: BT MODULE
USART6: EXTMODULE_USART
*/

#ifndef _HAL_H_
#define _HAL_H_

#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

// Keys
#define KEYS_GPIO_REG_ENTER           GPIOI
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_8
#define KEYS_GPIO_REG_PAGEUP          GPIOI
#define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_10
#define KEYS_GPIO_REG_PAGEDN          GPIOI
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_11
#define KEYS_GPIO_REG_MDL             GPIOE
#define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_5 
#define KEYS_GPIO_REG_EXIT            GPIOK
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5 
#define KEYS_GPIO_REG_SYS             GPIOJ
#define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_3 
#define KEYS_GPIO_REG_TELE            GPIOK
#define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_4 

// Trims
#define TRIMS_GPIO_REG_LHL            GPIOB
#define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_2

#define TRIMS_GPIO_REG_LHR            GPIOJ
#define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_4

#define TRIMS_GPIO_REG_LVD            GPIOJ
#define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_2

#define TRIMS_GPIO_REG_LVU            GPIOI
#define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_15

#define TRIMS_GPIO_REG_RHL            GPIOB
#define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_6

#define TRIMS_GPIO_REG_RHR            GPIOB
#define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4

#define TRIMS_GPIO_REG_RVD            GPIOC
#define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_13

#define TRIMS_GPIO_REG_RVU            GPIOC
#define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2

// Switches
// SWA
#define SWITCHES_GPIO_REG_A             GPIOH
#define SWITCHES_GPIO_PIN_A             LL_GPIO_PIN_8  // PE.05

// SWB
#define STORAGE_SWITCH_B
#define SWITCHES_GPIO_REG_B_H           GPIOD
#define SWITCHES_GPIO_PIN_B_H           LL_GPIO_PIN_11
#define SWITCHES_GPIO_REG_B_L           GPIOH
#define SWITCHES_GPIO_PIN_B_L           LL_GPIO_PIN_7
#define SWITCHES_B_INVERTED

// SWC
#define STORAGE_SWITCH_C
#define SWITCHES_GPIO_REG_C_H           GPIOG
#define SWITCHES_GPIO_PIN_C_H           LL_GPIO_PIN_3
#define SWITCHES_GPIO_REG_C_L           GPIOD
#define SWITCHES_GPIO_PIN_C_L           LL_GPIO_PIN_4
#define SWITCHES_C_INVERTED

// SWD
#define SWITCHES_GPIO_REG_D             GPIOI
#define SWITCHES_GPIO_PIN_D             LL_GPIO_PIN_3  // PI.03

// function switches
//SW1

// 6POS SW
#define SIXPOS_SWITCH_INDEX           6
#define SIXPOS_LED_RED                200
#define SIXPOS_LED_GREEN              0
#define SIXPOS_LED_BLUE               0

// ADC
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_2      // PA.00 ADC1_INP16
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_3      // PA.03 ADC12_INP15
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_0      // PA.01 ADC1_INP17
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_1      // PA.02 ADC12_INP14

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_1      // PC.03 POT2 ADC12_INP10
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_0      // PC.02 POT1 ADC123_INP11
#define ADC_GPIO_PIN_POT3               LL_GPIO_PIN_5      // PC.02 POT1 ADC123_INP8
#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_3      // PC.05 ADC12_INP13

#define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV, ADC_GPIO_PIN_STICK_RH, ADC_GPIO_PIN_STICK_RV)
#define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 |ADC_GPIO_PIN_POT3| ADC_GPIO_PIN_BATT )

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_14
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_17
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_16

#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_11
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_10
#define ADC_CHANNEL_POT3                LL_ADC_CHANNEL_8
#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_13
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT  // ADC3 IMP17

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_CHANNELS                { ADC_CHANNEL_RTC_BAT }
#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  330

#define ADC_DIRECTION {       	 \
0,0,-1,-1, 	/* gimbals */    \
1,1,0       	/* pots */    \
}

// Power
#define PWR_SWITCH_GPIO               GPIO_PIN(GPIOJ, 3) // SYS
#define PWR_EXTRA_SWITCH_GPIO         GPIO_PIN(GPIOE, 5) // MDL
#define PWR_SWITCH_GPIO1              GPIO_PIN(GPIOE, 5) // MDL
#define PWR_ON_GPIO                   GPIO_PIN(GPIOE, 2) //


// S.Port update connector
#define HAS_SPORT_UPDATE_CONNECTOR()    (false)

// Telemetry SPORT
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
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOB, 12) // 
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// Chargers (USB and wireless)
// USB Chargers
#define UCHARGER_PW                     GPIO_PIN(GPIOJ, 5)  //cherge power sw 0=DISABLE 1=ENABLE
#define UCHARGER_EN                     GPIO_PIN(GPIOK, 6)  //cherge EN  0=ENANLE 1=DISABLE
#define USB_GPIO_SEL                    GPIO_PIN(GPIOH, 6)  // USB HUB SELECT 0->H7 MCU  1->CHARGE MCU
#define UCHARGER_GPIO                   GPIO_PIN(GPIOJ, 15) //charge status

// LCD
#define LCD_SPI                         SPI1
#define LCD_SPI_DMA                     DMA1
#define LCD_SPI_TX_DMA                  LL_DMAMUX1_REQ_SPI1_TX
#define LCD_SPI_RX_DMA                  LL_DMAMUX1_REQ_SPI1_RX
#define LCD_SPI_TX_DMA_STREAM           LL_DMA_STREAM_4
#define LCD_SPI_RX_DMA_STREAM           LL_DMA_STREAM_5
#define LCD_FMARK                       GPIO_PIN(GPIOH,  15)
#define LCD_NRST                        GPIO_PIN(GPIOI,  0)
#define LCD_SPI_RS                      GPIO_PIN(GPIOI, 1)
#define LCD_SPI_CS                      GPIO_PIN(GPIOG, 10)
#define LCD_SPI_CLK                     GPIO_PIN(GPIOG, 11)
#define LCD_SPI_MOSI                    GPIO_PIN(GPIOB,  5)
#define LCD_SPI_MISO                    GPIO_PIN(GPIOK,  0)
#define LCD_SPI_BAUD                    100000000     //100000000

// Backlight
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOI, 2) // TIM8_CH4
#define BACKLIGHT_TIMER                 TIM8
#define BACKLIGHT_TIMER_CHANNEL		      LL_TIM_CHANNEL_CH4
#define BACKLIGHT_GPIO_AF               GPIO_AF3
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// QSPI Flash
#define QSPI_MAX_FREQ                   80000000U // 80 MHz
#define QSPI_CLK_GPIO                   GPIO_PIN(GPIOF, 10)  //OK
#define QSPI_CLK_GPIO_AF                GPIO_AF9
#define QSPI_CS_GPIO                    GPIO_PIN(GPIOB, 10)   //OK
#define QSPI_CS_GPIO_AF                 GPIO_AF9
#define QSPI_MISO_GPIO                  GPIO_PIN(GPIOF, 9)  //IO1
#define QSPI_MISO_GPIO_AF               GPIO_AF10
#define QSPI_MOSI_GPIO                  GPIO_PIN(GPIOF, 8)  //IO0
#define QSPI_MOSI_GPIO_AF               GPIO_AF10
#define QSPI_WP_GPIO                    GPIO_PIN(GPIOF, 7)  //IO2
#define QSPI_WP_GPIO_AF                 GPIO_AF9
#define QSPI_HOLD_GPIO                  GPIO_PIN(GPIOF, 6)  //IO3
#define QSPI_HOLD_GPIO_AF               GPIO_AF9
#define QSPI_FLASH_SIZE                 0x800000

#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(24000000)
#define STORAGE_USE_SDIO

// Audio
#define INVERTED_MUTE_PIN
#define AUDIO_MUTE_GPIO                GPIO_PIN(GPIOB, 1) //
#define AUDIO_OUTPUT_GPIO              GPIO_PIN(GPIOA, 4)
#define AUDIO_DAC                      DAC1
#define AUDIO_DMA_Stream               LL_DMA_STREAM_1
#define AUDIO_DMA_Channel              LL_DMAMUX1_REQ_DAC1_CH1
#define AUDIO_TIM_IRQn                 TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler           TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQn          DMA2_Stream1_IRQn
#define AUDIO_DMA_Stream_IRQHandler    DMA2_Stream1_IRQHandler
#define AUDIO_TIMER                    TIM6
#define AUDIO_DMA                      DMA2

// Haptic
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOE, 6) // TIM15_CH2
#define HAPTIC_GPIO_TIMER               TIM15
#define HAPTIC_GPIO_AF                  GPIO_AF4
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

// LED Strip
#define LED_STRIP_LENGTH                  7  // 6POS + 1 common (many leds in //)
#define BLING_LED_STRIP_START             6
#define BLING_LED_STRIP_LENGTH            1
#define CFS_LED_STRIP_START               0
#define CFS_LED_STRIP_LENGTH              6
#define CFS_LEDS_PER_SWITCH               1
#define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 10)  // PA.00 / TIM1_CH3
#define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1         // TIM1_CH3
#define LED_STRIP_TIMER                   TIM1
#define LED_STRIP_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH3
#define LED_STRIP_TIMER_DMA               DMA1
#define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMAMUX1_REQ_TIM1_UP
#define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_0
#define LED_STRIP_TIMER_DMA_IRQn          DMA1_Stream0_IRQn
#define LED_STRIP_TIMER_DMA_IRQHandler    DMA1_Stream0_IRQHandler
#define LED_STRIP_REFRESH_PERIOD          50 //ms

#define STATUS_LEDS
#define GPIO_LED_GPIO_ON              gpio_set
#define GPIO_LED_GPIO_OFF             gpio_clear
#define LED_RED_GPIO                  GPIO_PIN(GPIOI, 12)
#define LED_GREEN_GPIO                GPIO_PIN(GPIOI, 13)
#define LED_BLUE_GPIO                 GPIO_PIN(GPIOI, 14)

// Internal Module
#define EXTERNAL_ANTENNA

#define INTMODULE_PWR_GPIO              GPIO_PIN(GPIOG, 13)
#define INTMODULE_ANTSEL_GPIO           GPIO_PIN(GPIOA, 8)  //ANE SELECT 0=Int 1=Ext
#define INTMODULE_BOOTCMD_GPIO          GPIO_PIN(GPIOG, 9)
#define INTMODULE_BOOTCMD_DEFAULT       0
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOD, 5)
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOD, 6)
#define INTMODULE_USART                 USART2
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART2_IRQn
#define INTMODULE_USART_IRQHandler      USART2_IRQHandler
#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMAMUX1_REQ_USART2_TX
#define INTMODULE_RX_DMA                DMA1
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_5
#define INTMODULE_RX_DMA_CHANNEL        LL_DMAMUX1_REQ_USART2_RX
#define INTMODULE_RX_DMA_Stream_IRQn    DMA1_Stream5_IRQn
#define INTMODULE_RX_DMA_Stream_IRQHandler DMA1_Stream5_IRQHandler

// External Module
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOI, 4)
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6) // TIM3_CH1
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)

#define EXTMODULE_TIMER                 TIM3
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
#define EXTMODULE_TIMER_IRQn            TIM3_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM3_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_2

//USART
#define EXTMODULE_USART                    USART6
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART6_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_USART6_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5
#define EXTMODULE_USART_IRQHandler         USART6_IRQHandler
#define EXTMODULE_USART_IRQn               USART6_IRQn

//TIMER
// TODO
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM3_UP
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_3
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream3_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream3_IRQHandler

// Trainer Port
#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOD, 13)  // TIM4_CH2
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH4

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOD, 12)  // TIM4_CH1
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH1

#define TRAINER_TIMER                   TIM4
#define TRAINER_TIMER_IRQn              TIM4_IRQn
#define TRAINER_TIMER_IRQHandler        TIM4_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_2
#define TRAINER_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)

// I2C Bus
#define I2C_B4                          I2C4
#define I2C_B4_SDA_GPIO                 GPIO_PIN(GPIOH, 12)  // PB.09
#define I2C_B4_SCL_GPIO                 GPIO_PIN(GPIOH, 11)  // PB.08
#define I2C_B4_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B4_CLK_RATE                 400000

// Touch
#define TOUCH_I2C_BUS                 I2C_Bus_4
#define TOUCH_RST_GPIO                GPIO_PIN(GPIOJ, 14)  // PJ.14
#define TOUCH_INT_GPIO                GPIO_PIN(GPIOD, 11)  // PD.11

#define TOUCH_INT_EXTI_Line           LL_EXTI_LINE_11
#define TOUCH_INT_EXTI_Port           LL_SYSCFG_EXTI_PORTD
#define TOUCH_INT_EXTI_SysCfgLine     LL_SYSCFG_EXTI_LINE11

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI15_10_IRQ)
#define USE_EXTI15_10_IRQ
#define EXTI15_10_IRQ_Priority 9
#endif

// IMU
#define IMU_I2C_BUS                     I2C_Bus_4
#define IMU_I2C_ADDRESS                 0x69
//#define IMU_INT_GPIO	                GPIO_PIN(GPIOJ, 10) // PG.13
// IMU_INT_EXTI IRQ
#if !defined(USE_EXTI15_10_IRQ)
  #define USE_EXTI15_10_IRQ
  #define EXTI15_10_IRQ_Priority       6
#endif


//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION
// Rotary Encoder
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOE
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_3 // PE.03
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_4 // PE.04
#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 3) & 0x03)
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_3
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_4
#if !defined(USE_EXTI3_IRQ)
  #define USE_EXTI3_IRQ
  #define EXTI3_IRQ_Priority 5
#endif
#if !defined(USE_EXTI4_IRQ)
  #define USE_EXTI4_IRQ
  #define EXTI4_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_SYSCFG_EXTI_PORTE
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE3
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE4
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

#define LCD_W                           320
#define LCD_H                           240

#define LCD_PHYS_W                      LCD_W
#define LCD_PHYS_H                      LCD_H

#define LCD_DEPTH                       16

#define LSE_DRIVE_STRENGTH  RCC_LSEDRIVE_HIGH

#endif // _HAL_H_
