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

#define CPU_FREQ                600000000

#define PERI1_FREQUENCY         150000000
#define PERI2_FREQUENCY         150000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

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


// ADC

#define ADC_VREF_PREC2 660

#define ADC_GPIO_PIN_STICK_LH LL_GPIO_PIN_0
#define ADC_GPIO_PIN_STICK_LV LL_GPIO_PIN_2
#define ADC_GPIO_PIN_STICK_RV
#define ADC_GPIO_PIN_STICK_RH

// #define ADC_GPIO_PIN_POT1 LL_GPIO_PIN_0 // PA0_C VR1
// #define ADC_GPIO_PIN_POT2 LL_GPIO_PIN_1 // PA1_C VR2

// #define ADC_GPIOA_PINS (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
#define ADC_GPIOC_PINS (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)

// Fake gimbal channels
#define ADC_CHANNEL_STICK_LH LL_ADC_CHANNEL_10
#define ADC_CHANNEL_STICK_LV LL_ADC_CHANNEL_12
#define ADC_CHANNEL_STICK_RV
#define ADC_CHANNEL_STICK_RH

// #define ADC_CHANNEL_POT1 LL_ADC_CHANNEL_0 // ADC12_INP0
// #define ADC_CHANNEL_POT2 LL_ADC_CHANNEL_1 // ADC12_INP1

#define ADC_MAIN ADC1
#define ADC_DMA  GPDMA1
#define ADC_DMA_CHANNEL LL_GPDMA1_REQUEST_ADC1
#define ADC_DMA_STREAM LL_DMA_CHANNEL_0
#define ADC_DMA_STREAM_IRQ GPDMA1_Channel0_IRQn
#define ADC_DMA_STREAM_IRQHandler GPDMA1_Channel0_IRQHandler

#define ADC_SAMPTIME LL_ADC_SAMPLINGTIME_92CYCLES_5
#define ADC_DIRECTION {0,0,0,0,0,0}

// Power
// #define PWR_SWITCH_GPIO             GPIO_PIN(GPIOI, 11)  // PI.11
// #define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 14)  // PI.14

// Chargers (USB and wireless)
// #define UCHARGER_GPIO               GPIO_PIN(GPIOB, 14) // PB.14 input
// #define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOB, 13) // PB.13 input
// #define UCHARGER_EN_GPIO            GPIO_PIN(GPIOG, 3)  // PG.03 output

// #if defined (WIRELESS_CHARGER)
//   #define WCHARGER_GPIO               GPIO_PIN(GPIOI, 9)  // PI.09 input
//   #define WCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOI, 10) // PI.10 input
//   #define WCHARGER_EN_GPIO            GPIO_PIN(GPIOH, 4)  // PH.04 output
//   #define WCHARGER_I_CONTROL_GPIO     GPIO_PIN(GPIOH, 13) // PH.13 output
// #endif

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
// #define SPORT_MAX_BAUDRATE              400000
// #define SPORT_UPDATE_RCC_AHB1Periph     0
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
// #define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOJ | RCC_AHB1Periph_DMA1)
// #define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
// #define TELEMETRY_RX_REV_GPIO           GPIO_PIN(GPIOJ, 8)  // PJ.08
// #define TELEMETRY_TX_REV_GPIO           GPIO_PIN(GPIOJ, 7)  // PJ.07
// #define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOJ, 13) // PJ.13
// #define TELEMETRY_SET_INPUT             1
// #define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
// #define TELEMETRY_RX_GPIO               GPIO_PIN(GPIOD, 6)  // PD.06
// #define TELEMETRY_USART                 USART2
// #define TELEMETRY_USART_IRQn            USART2_IRQn
// #define TELEMETRY_DMA                   DMA1
// #define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_6
// #define TELEMETRY_DMA_Channel_TX        LL_DMA_CHANNEL_4
// #define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
// #define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
// #define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
// // #define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_5
// // #define TELEMETRY_DMA_Channel_RX        LL_DMA_CHANNEL_4
// #define TELEMETRY_USART_IRQHandler      USART2_IRQHandler

// #define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
// #define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
// #define TELEMETRY_TX_POL_NORM()         TELEMETRY_REV_GPIO->BSRRH = TELEMETRY_TX_REV_GPIO_PIN
// #define TELEMETRY_TX_POL_INV()          TELEMETRY_REV_GPIO->BSRRL = TELEMETRY_TX_REV_GPIO_PIN
// #define TELEMETRY_RX_POL_NORM()         TELEMETRY_REV_GPIO->BSRRH = TELEMETRY_RX_REV_GPIO_PIN
// #define TELEMETRY_RX_POL_INV()          TELEMETRY_REV_GPIO->BSRRL = TELEMETRY_RX_REV_GPIO_PIN

// Software IRQ (Prio 5 -> FreeRTOS compatible)
// #define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
// #define USE_EXTI4_IRQ
// #define EXTI4_IRQ_Priority 5

// USB
#if defined(USE_USB_HS)
  #define USB_GPIO_VBUS                 GPIO_PIN(GPIOM, 8) // PM.08
  #define USB_GPIO_VBUS_OPEN_DRAIN
  #define USB_GPIO_DM                   GPIO_PIN(GPIOM, 5) // PM.05
  #define USB_GPIO_DP                   GPIO_PIN(GPIOM, 6) // PM.06
#else
  #define USB_GPIO_VBUS                 GPIO_PIN(GPIOD,  4) // PD.04
  #define USB_GPIO_DM                   GPIO_PIN(GPIOM, 11) // PM.11
  #define USB_GPIO_DP                   GPIO_PIN(GPIOM, 12) // PM.12
#endif
#define USB_GPIO_AF                     GPIO_AF10

// LCD
#define LCD_RCC_AHB1Periph              LL_AHB1_GRP1_PERIPH_DMA2D
#define LCD_RCC_APB1Periph              0
#define LCD_RCC_APB2Periph              LL_APB2_GRP1_PERIPH_LTDC
#define LCD_NRST_GPIO                   GPIOG
#define LCD_NRST_GPIO_PIN               LL_GPIO_PIN_9  // PG.09
#define LCD_SPI_GPIO                    GPIOE
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_4  // PE.04
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_2  // PE.02
#define LCD_SPI_MISO_GPIO_PIN           LL_GPIO_PIN_5  // PE.05
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_6  // PE.06
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
// TODO TIM3, TIM8, TIM14, review the channel in backlight_driver.cpp according to the chosen timer
// #define BACKLIGHT_RCC_APB2Periph        0
// #define BACKLIGHT_GPIO                  GPIO_PIN(GPIOA, 15)
// #define BACKLIGHT_TIMER                 TIM2
// #define BACKLIGHT_GPIO_AF               GPIO_AF1
// #define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Storage
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
// #define AUDIO_RCC_APB1Periph            (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
// #define AUDIO_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
// #define AUDIO_OUTPUT_GPIO               GPIO_PIN(GPIOA, 4)  // PA.04
// #define AUDIO_DMA_Stream                DMA1_Stream5
// #define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
// #define AUDIO_TIM_IRQn                  TIM6_DAC_IRQn
// #define AUDIO_TIM_IRQHandler            TIM6_DAC_IRQHandler
// #define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
// #define AUDIO_TIMER                     TIM6
// #define AUDIO_DMA                       DMA1

// I2C Bus
#define I2C_B1                          I2C1
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 9)  // PB.09
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 6)  // PB.06
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 100000

// Touch
#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOE, 3) // PE.03

// TOUCH_INT_EXTI IRQ
#if !defined(USE_EXTI3_IRQ)
  #define USE_EXTI3_IRQ
  #define EXTI3_IRQ_Priority  9
#endif

// Haptic: TIM1_CH1
// #define HAPTIC_PWM
// #define HAPTIC_GPIO                     GPIO_PIN(GPIOA, 8) // PA.08
// #define HAPTIC_GPIO_TIMER               TIM1
// #define HAPTIC_GPIO_AF                  GPIO_AF1
// #define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC1E | TIM_CCER_CC1NE;
// #define HAPTIC_TIMER_MODE               TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE
// #define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR1

// Flysky Hall Stick
// #define FLYSKY_HALL_SERIAL_USART                 UART4
// #define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
// #define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOA, 0)  // PA.00
// #define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOA, 1)  // PA.01
// #define FLYSKY_HALL_SERIAL_GPIO_AF               LL_GPIO_AF_8

// #define FLYSKY_HALL_RCC_AHB1Periph               RCC_AHB1Periph_DMA1
// #define FLYSKY_HALL_RCC_APB1Periph               RCC_APB1Periph_UART4

// #define FLYSKY_HALL_SERIAL_USART_IRQHandler      UART4_IRQHandler
// #define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
// #define FLYSKY_HALL_SERIAL_DMA                   DMA1
// #define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
// #define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4

// External Module
// #define EXTMODULE
// #define EXTMODULE_PULSES
// #define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOD, 11) // PD.11
// #define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
// #define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOC, 7)  // PC.07
// #define EXTMODULE_TX_GPIO_AF            LL_GPIO_AF_3 // TIM8_CH1
// #define EXTMODULE_TX_GPIO_AF_USART      GPIO_AF_USART6
// #define EXTMODULE_RX_GPIO_AF_USART      GPIO_AF_USART6
// #define EXTMODULE_TIMER                 TIM8
// #define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH1
// #define EXTMODULE_TIMER_IRQn            TIM8_UP_TIM13_IRQn
// #define EXTMODULE_TIMER_IRQHandler      TIM8_UP_TIM13_IRQHandler
// #define EXTMODULE_TIMER_FREQ            (PERI2_FREQUENCY * TIMER_MULT_APB2)
// #define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_3

//USART
// #define EXTMODULE_USART                    USART6
// #define EXTMODULE_USART_GPIO               GPIOC
// #define EXTMODULE_USART_GPIO_AF            GPIO_AF_USART6
// #define EXTMODULE_USART_GPIO_AF_LL         LL_GPIO_AF_8
// #define EXTMODULE_USART_TX_DMA             DMA2
// #define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMA_CHANNEL_5
// #define EXTMODULE_USART_TX_DMA_STREAM     LL_DMA_STREAM_7

// #define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMA_CHANNEL_5
// #define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_2

// #define EXTMODULE_USART_IRQHandler         USART6_IRQHandler
// #define EXTMODULE_USART_IRQn               USART6_IRQn

//TIMER
// #define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMA_CHANNEL_7
// #define EXTMODULE_TIMER_DMA                DMA2
// #define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_1
// #define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream1_IRQn
// #define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream1_IRQHandler

// #define EXTMODULE_TX_INVERT_GPIO           GPIO_PIN(GPIOE, 3)  // PE.03
// #define EXTMODULE_RX_INVERT_GPIO           GPIO_PIN(GPIOI, 15) // PI.15

// #define EXTMODULE_TX_NORMAL()              EXTMODULE_TX_INVERT_GPIO->BSRRH = EXTMODULE_TX_INVERT_GPIO_PIN
// #define EXTMODULE_TX_INVERTED()            EXTMODULE_TX_INVERT_GPIO->BSRRL = EXTMODULE_TX_INVERT_GPIO_PIN
// #define EXTMODULE_RX_NORMAL()              EXTMODULE_RX_INVERT_GPIO->BSRRH = EXTMODULE_RX_INVERT_GPIO_PIN
// #define EXTMODULE_RX_INVERTED()            EXTMODULE_RX_INVERT_GPIO->BSRRL = EXTMODULE_RX_INVERT_GPIO_PIN

// Trainer Port
// #define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOD)

// #define TRAINER_IN_GPIO                 GPIO_PIN(GPIOD, 12) // PD.12
// #define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1

// #define TRAINER_OUT_GPIO                GPIO_PIN(GPIOD, 13) // PD.13
// #define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

// #define TRAINER_TIMER                   TIM4
// #define TRAINER_TIMER_IRQn              TIM4_IRQn
// #define TRAINER_TIMER_IRQHandler        TIM4_IRQHandler
// #define TRAINER_GPIO_AF                 LL_GPIO_AF_2
// #define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

//ROTARY emulation for trims as buttons
// #define ROTARY_ENCODER_NAVIGATION

//BLUETOOTH
// #define BLUETOOTH_ON_RCC_AHB1Periph     RCC_AHB1Periph_GPIOI
// #define BT_EN_GPIO                      GPIOI
// #define BT_EN_GPIO_PIN                  GPIO_Pin_8 // PI.8

// #define BT_RCC_AHB1Periph               (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH)
// #define BT_RCC_APB1Periph               (RCC_APB1Periph_USART3)
// #define BT_RCC_APB2Periph                0

// #define BT_USART                        USART3
// #define BT_GPIO_AF                      GPIO_AF_USART3
// #define BT_USART_IRQn                   USART3_IRQn
// #define BT_GPIO_TXRX                    GPIOB
// #define BT_TX_GPIO_PIN                  GPIO_Pin_10  // PB.10
// #define BT_RX_GPIO_PIN                  GPIO_Pin_11  // PB.11
// #define BT_TX_GPIO_PinSource            GPIO_PinSource10
// #define BT_RX_GPIO_PinSource            GPIO_PinSource11
// #define BT_USART_IRQHandler             USART3_IRQHandler

// #define BT_CONNECTED_GPIO               GPIOJ
// #define BT_CONNECTED_GPIO_PIN           GPIO_Pin_1 // PJ.01

// #define BT_CMD_MODE_GPIO                GPIOH
// #define BT_CMD_MODE_GPIO_PIN            GPIO_Pin_6 // PH.6

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM12_IRQHandler

#define LANDSCAPE_LCD true
#define PORTRAIT_LCD false

#define LCD_W                           800
#define LCD_H                           480

#define LCD_PHYS_W                      800
#define LCD_PHYS_H                      480

#define LCD_DEPTH                       16
// #define LCD_CONTRAST_DEFAULT            20

#endif // _HAL_H_
