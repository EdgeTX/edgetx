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
  #define KEYS_GPIO_REG_PAGEUP          GPIOC
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_13 // PC.13
  #define KEYS_GPIO_REG_PAGEDN          GPIOI
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8  // PI.08
  #define KEYS_GPIO_REG_SYS             GPIOI
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_7  // PI.07
  #define KEYS_GPIO_REG_ENTER           GPIOC
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_1  // PC.01
  #define KEYS_GPIO_REG_MDL             GPIOG
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_13 // PG.13
  #define KEYS_GPIO_REG_EXIT            GPIOI
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_6  // PI.06
  #define KEYS_GPIO_REG_TELE            GPIOC
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_4  // PC.04
#elif defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
  #define KEYS_GPIO_REG_ENTER           GPIOI
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_8  // PI.08
  #define KEYS_GPIO_REG_PAGEUP          GPIOC
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_13 // PC.13
  #define KEYS_GPIO_REG_PAGEDN          GPIOI
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_11 // PI.11
  #define KEYS_GPIO_REG_MDL             GPIOI
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_6  // PI.06
  #define KEYS_GPIO_REG_EXIT            GPIOI
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_4  // PI.04
  #define KEYS_GPIO_REG_SYS             GPIOI
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_7  // PI.07
  #define KEYS_GPIO_REG_TELE            GPIOI
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_5  // PI.05
#elif defined(RADIO_T15)
  #define KEYS_GPIO_REG_ENTER           GPIOI
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_8  // PI.08
  #define KEYS_GPIO_REG_PAGEDN          GPIOI
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_11 // PI.11
  #define KEYS_GPIO_REG_MDL             GPIOI
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_6  // PI.06
  #define KEYS_GPIO_REG_EXIT            GPIOI
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5  // PI.05
  #define KEYS_GPIO_REG_SYS             GPIOI
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_7  // PI.07
  #define KEYS_GPIO_REG_TELE            GPIOI
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_4  // PI.04
#elif defined(PCBX10)
  #define KEYS_GPIO_REG_ENTER           GPIOI
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_8  // PI.08
  #define KEYS_GPIO_REG_PAGEDN          GPIOI
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_11 // PI.11
  #define KEYS_GPIO_REG_MDL             GPIOI
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_4  // PI.04
  #define KEYS_GPIO_REG_EXIT            GPIOI
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_6  // PI.06
  #define KEYS_GPIO_REG_SYS             GPIOI
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_7  // PI.07
  #define KEYS_GPIO_REG_TELE            GPIOI
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_5  // PI.05
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
  #define SWITCHES_GPIO_REG_A_H           GPIOG
  #define SWITCHES_GPIO_PIN_A_H           LL_GPIO_PIN_3  // PG.03
  #define SWITCHES_GPIO_REG_A_L           GPIOD
  #define SWITCHES_GPIO_PIN_A_L           LL_GPIO_PIN_11 // PD.11
  #define SWITCHES_A_INVERTED

  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H           GPIOB
  #define SWITCHES_GPIO_PIN_B_H           LL_GPIO_PIN_12 // PB.12
  #define SWITCHES_GPIO_REG_B_L           GPIOH
  #define SWITCHES_GPIO_PIN_B_L           LL_GPIO_PIN_9  // PH.09

  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H           GPIOG
  #define SWITCHES_GPIO_PIN_C_H           LL_GPIO_PIN_2  // PG.02
  #define SWITCHES_GPIO_REG_C_L           GPIOH
  #define SWITCHES_GPIO_PIN_C_L           LL_GPIO_PIN_14 // PH.14
  #define SWITCHES_C_INVERTED

  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H           GPIOI
  #define SWITCHES_GPIO_PIN_D_H           LL_GPIO_PIN_15 // PI.15
  #define SWITCHES_GPIO_REG_D_L           GPIOH
  #define SWITCHES_GPIO_PIN_D_L           LL_GPIO_PIN_15 // PH.15

  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E             GPIOB
  #define SWITCHES_GPIO_PIN_E             LL_GPIO_PIN_15 // PB.15
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F             GPIOH
  #define SWITCHES_GPIO_PIN_F             LL_GPIO_PIN_12 // PH.12
  //SW1
  #define FUNCTION_SWITCH_1             SG
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOB
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_14  // PB.14
  //SW2
  #define FUNCTION_SWITCH_2             SH
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_13  // PD.13
  //SW3
  #define FUNCTION_SWITCH_3             SI
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOJ
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_7  // PJ.07
  //SW4
  #define FUNCTION_SWITCH_4             SJ
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_13 // PG.13
  //SW5
  #define FUNCTION_SWITCH_5             SK
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K           GPIOJ
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_8  // PJ.08
  //SW6
  #define FUNCTION_SWITCH_6             SL
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L           GPIOB
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_13 // PB.13
#else
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H           GPIOH
  #define SWITCHES_GPIO_PIN_A_H           LL_GPIO_PIN_9  // PH.09
  #define SWITCHES_GPIO_REG_A_L           GPIOI
  #define SWITCHES_GPIO_PIN_A_L           LL_GPIO_PIN_15 // PI.15
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H           GPIOH
  #define SWITCHES_GPIO_PIN_B_H           LL_GPIO_PIN_12 // PH.12
  #define SWITCHES_GPIO_REG_B_L           GPIOB
  #define SWITCHES_GPIO_PIN_B_L           LL_GPIO_PIN_12 // PB.12
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H           GPIOD
  #define SWITCHES_GPIO_PIN_C_H           LL_GPIO_PIN_11 // PD.11
  #define SWITCHES_GPIO_REG_C_L           GPIOB
  #define SWITCHES_GPIO_PIN_C_L           LL_GPIO_PIN_15 // PB.15
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H           GPIOJ
  #define SWITCHES_GPIO_PIN_D_H           LL_GPIO_PIN_7  // PJ.07
  #define SWITCHES_GPIO_REG_D_L           GPIOG
  #define SWITCHES_GPIO_PIN_D_L           LL_GPIO_PIN_2  // PG.02
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H           GPIOH
  #define SWITCHES_GPIO_PIN_E_H           LL_GPIO_PIN_4  // PH.04
  #define SWITCHES_GPIO_REG_E_L           GPIOE
  #define SWITCHES_GPIO_PIN_E_L           LL_GPIO_PIN_3  // PE.03
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F             GPIOH
  #define SWITCHES_GPIO_PIN_F             LL_GPIO_PIN_3  // PH.03
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G_H           GPIOG
  #define SWITCHES_GPIO_PIN_G_H           LL_GPIO_PIN_6  // PG.06
  #define SWITCHES_GPIO_REG_G_L           GPIOG
  #define SWITCHES_GPIO_PIN_G_L           LL_GPIO_PIN_3  // PG.03
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H             GPIOG
  #define SWITCHES_GPIO_PIN_H             LL_GPIO_PIN_7  // PG.07

  #if defined(PCBX12S)
    #define SWITCHES_F_INVERTED
  #elif defined(PCBX10)
    #define SWITCHES_B_INVERTED
    #define SWITCHES_D_INVERTED
    #define SWITCHES_E_INVERTED
  #endif

  #if defined(PCBX10) && !defined(RADIO_F16)
    // Gimbal switch left
    #define STORAGE_SWITCH_I
    #define HARDWARE_SWITCH_I
    #define SWITCHES_GPIO_REG_I           GPIOH
    #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_14 // PH.14
    // Gimbal switch right
    #define STORAGE_SWITCH_J
    #define HARDWARE_SWITCH_J
    #define SWITCHES_GPIO_REG_J           GPIOH
    #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_15 // PH.15
  #elif defined(PCBX12S)
    // Gimbal switch left
    #define STORAGE_SWITCH_I
    #define HARDWARE_SWITCH_I
    #define SWITCHES_GPIO_REG_I           GPIOB
    #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_1 // PB.01
    // Gimbal switch right
    #define STORAGE_SWITCH_J
    #define HARDWARE_SWITCH_J
    #define SWITCHES_GPIO_REG_J           GPIOB
    #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_0 // PB.00
  #endif
#endif

// 6POS SW
#if defined(RADIO_V16)
  #define SIXPOS_SWITCH_INDEX             5
  #define SIXPOS_LED_RED                255
  #define SIXPOS_LED_GREEN              255
  #define SIXPOS_LED_BLUE               255
#endif

// Trims
#if defined(RADIO_T15)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_3  // PD.03
  #define TRIMS_GPIO_REG_LHR            GPIOD
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_7  // PD.07
  #define TRIMS_GPIO_REG_LVU            GPIOJ
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_12 // PJ.12
  #define TRIMS_GPIO_REG_LVD            GPIOJ
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_13 // PJ.13
  #define TRIMS_GPIO_REG_RVD            GPIOG
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_12 // PG.12
  #define TRIMS_GPIO_REG_RHL            GPIOA
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_6  // PA.06
  #define TRIMS_GPIO_REG_RVU            GPIOJ
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_14 // PJ.14
  #define TRIMS_GPIO_REG_RHR            GPIOC
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4  // PC.04
#elif defined(PCBX12S)
  #define TRIMS_GPIO_REG_RHL            GPIOC
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_0  // PC.00
  #define TRIMS_GPIO_REG_RHR            GPIOI
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4  // PI.04
  #define TRIMS_GPIO_REG_RVD            GPIOG
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_12 // PG.12
  #define TRIMS_GPIO_REG_RVU            GPIOJ
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_14 // PJ.14
  #define TRIMS_GPIO_REG_LVD            GPIOJ
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_13 // PJ.13
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_3  // PD.03
  #define TRIMS_GPIO_REG_LVU            GPIOJ
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_12 // PJ.12
  #define TRIMS_GPIO_REG_LHR            GPIOD
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_7  // PD.07
  #define TRIMS_GPIO_REG_RSD            GPIOJ
  #define TRIMS_GPIO_PIN_RSD            LL_GPIO_PIN_8  // PJ.08
  #define TRIMS_GPIO_REG_RSU            GPIOD
  #define TRIMS_GPIO_PIN_RSU            LL_GPIO_PIN_13 // PD.13
  #define TRIMS_GPIO_REG_LSD            GPIOB
  #define TRIMS_GPIO_PIN_LSD            LL_GPIO_PIN_14 // PB.14
  #define TRIMS_GPIO_REG_LSU            GPIOB
  #define TRIMS_GPIO_PIN_LSU            LL_GPIO_PIN_13 // PB.13
#elif defined(PCBX10)
  #if defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
    #define TRIMS_GPIO_REG_LHL            GPIOA
    #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_6  // PA.06
    #define TRIMS_GPIO_REG_LHR            GPIOC
    #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_4  // PC.04
  #else
    #define TRIMS_GPIO_REG_LHL            GPIOB
    #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_8  // PB.08
    #define TRIMS_GPIO_REG_LHR            GPIOB
    #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_9  // PB.09
  #endif
  #define TRIMS_GPIO_REG_LVD            GPIOG
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_12 // PG.12
  #define TRIMS_GPIO_REG_LVU            GPIOJ
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_14 // PJ.14
  #define TRIMS_GPIO_REG_RVD            GPIOJ
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_13 // PJ.13
  #define TRIMS_GPIO_REG_RHL            GPIOD
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_3  // PD.03
  #define TRIMS_GPIO_REG_RVU            GPIOJ
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_12 // PJ.12
  #define TRIMS_GPIO_REG_RHR            GPIOD
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_7  // PD.07
  #if defined(RADIO_FAMILY_T16)
    #define TRIMS_GPIO_REG_LSU          GPIOD
    #define TRIMS_GPIO_PIN_LSU          LL_GPIO_PIN_13 // PD.13
    #define TRIMS_GPIO_REG_LSD          GPIOJ
    #define TRIMS_GPIO_PIN_LSD          LL_GPIO_PIN_8  // PJ.08
  #else
    #define TRIMS_GPIO_REG_LSU          GPIOJ
    #define TRIMS_GPIO_PIN_LSU          LL_GPIO_PIN_8  // PJ.08
    #define TRIMS_GPIO_REG_LSD          GPIOD
    #define TRIMS_GPIO_PIN_LSD          LL_GPIO_PIN_13 // PD.13
  #endif
  #define TRIMS_GPIO_REG_RSU            GPIOB
  #define TRIMS_GPIO_PIN_RSU            LL_GPIO_PIN_14 // PB.14
  #define TRIMS_GPIO_REG_RSD            GPIOB
  #define TRIMS_GPIO_PIN_RSD            LL_GPIO_PIN_13 // PB.13
#endif


// ADC
#if defined(PCBX12S)
  #define ADC_SPI                       SPI4
  #define ADC_SPI_GPIO                  GPIOE
  #define ADC_SPI_GPIO_AF               LL_GPIO_AF_5
  #define ADC_SPI_GPIO_PIN_SCK          LL_GPIO_PIN_2 // PE.02
  #define ADC_SPI_GPIO_PIN_CS           LL_GPIO_PIN_4 // PE.04
  #define ADC_SPI_GPIO_PIN_MOSI         LL_GPIO_PIN_6 // PE.06
  #define ADC_SPI_GPIO_PIN_MISO         LL_GPIO_PIN_5 // PE.05

  #define ADC_SPI_STICK_LH              0
  #define ADC_SPI_STICK_LV              1
  #define ADC_SPI_STICK_RV              2
  #define ADC_SPI_STICK_RH              3
  #define ADC_SPI_POT1                  4
  #define ADC_SPI_POT2                  5
  #define ADC_SPI_POT3                  6
  #define ADC_SPI_SLIDER1               7
  #define ADC_SPI_SLIDER2               8
  #define ADC_SPI_BATT                  9
  #define ADC_SPI_SLIDER3               11
  #define ADC_SPI_SLIDER4               10

  #define ADC_MAIN                      ADC3
  #define ADC_GPIO_PIN_MOUSE1           LL_GPIO_PIN_8 // PF.08 ADC3_IN6 J5 MOUSE_X
  #define ADC_GPIO_PIN_MOUSE2           LL_GPIO_PIN_9 // PF.09 ADC3_IN7 J6 MOUSE_Y
  #define ADC_GPIO_MOUSE                GPIOF
  #define ADC_CHANNEL_MOUSE1            6
  #define ADC_CHANNEL_MOUSE2            7
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_MOUSE1 | ADC_GPIO_PIN_MOUSE2)
  #define ADC_EXT                       ADC1
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_CHANNEL_RTC_BAT           LL_ADC_CHANNEL_VBAT // ADC1_IN16
  #define ADC_EXT_CHANNELS              { ADC_CHANNEL_RTC_BATT }
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_2
  // TODO: use for SPI1_RX instead?
  #define ADC_DMA_STREAM                LL_DMA_STREAM_0
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream0_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream0_IRQHandler
  #define ADC_SAMPTIME                  LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_VREF_PREC2                600
#elif defined(RADIO_V16)
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0      // PA.00
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1      // PA.01
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_3      // PA.03
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_2      // PA.02
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_2      // PC.00 // ADC123_IN10 -> ADC3_IN10
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_1      // PC.02 // ADC123_IN12 -> ADC3_IN12
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_0      // PC.01 // ADC123_IN11 -> ADC3_IN11
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_5      // PA.05
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_3      // PC.03
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_7      // PF.07
  #define ADC_GPIO_PIN_EXT1             LL_GPIO_PIN_8      // PF.08
  #define ADC_GPIO_PIN_EXT2             LL_GPIO_PIN_9      // PF.09
  #define ADC_GPIO_PIN_EXT3             ADC_GPIO_PIN_STICK_RH
  #define ADC_GPIO_PIN_EXT4             ADC_GPIO_PIN_STICK_RV
  #define ADC_GPIOA_PINS_FS             (LL_GPIO_PIN_2 | LL_GPIO_PIN_3)
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_SLIDER1)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_EXT1 | ADC_GPIO_PIN_EXT2)
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0   // ADC123_IN0 -> ADC3_IN0
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1   // ADC123_IN1 -> ADC3_IN1
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_2   // ADC123_IN2 -> ADC3_IN2
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_3   // ADC123_IN3 -> ADC3_IN3
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_12  // ADC123_IN10 -> ADC3_IN10
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_11  // ADC123_IN12 -> ADC3_IN12
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_10  // ADC123_IN11 -> ADC3_IN11
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_5   // ADC12_IN5 -> ADC1_IN5
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_13  // ADC123_IN13 -> ADC1_IN13
  #define ADC_CHANNEL_EXT1              LL_ADC_CHANNEL_6   // ADC3_IN6 -> ADC3_IN6
  #define ADC_CHANNEL_EXT2              LL_ADC_CHANNEL_7   // ADC3_IN7 -> ADC3_IN7
  #define ADC_CHANNEL_EXT3              LL_ADC_CHANNEL_2   // ADC3_IN2: same as RH
  #define ADC_CHANNEL_EXT4              LL_ADC_CHANNEL_3   // ADC3_IN3: same as RV
  #define ADC_CHANNEL_RTC_BAT           LL_ADC_CHANNEL_VBAT // ADC1_IN18
  #define ADC_MAIN                      ADC3
  #define ADC_EXT                       ADC1
  #define ADC_EXT_CHANNELS              { ADC_CHANNEL_SLIDER1, ADC_CHANNEL_SLIDER2, ADC_CHANNEL_RTC_BAT }
  #define ADC_EXT_DMA                   DMA2
  #define ADC_EXT_DMA_CHANNEL           LL_DMA_CHANNEL_0
  #define ADC_EXT_DMA_STREAM            LL_DMA_STREAM_4
  #define ADC_EXT_DMA_STREAM_IRQ        DMA2_Stream4_IRQn
  #define ADC_EXT_DMA_STREAM_IRQHandler DMA2_Stream4_IRQHandler
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_SAMPTIME                  LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_2
  #define ADC_DMA_STREAM                LL_DMA_STREAM_0
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream0_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream0_IRQHandler
  #define ADC_VREF_PREC2                660
#elif defined(PCBX10)
#if defined(RADIO_T15)
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_1      // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_0      // PA.00
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_2      // PA.02
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_3      // PA.03
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_2      // PC.02
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0      // PC.00
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_12  // ADC3_IN12
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_10  // ADC3_IN10
#else
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0      // PA.00
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1      // PA.01
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_3      // PA.03
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_2      // PA.02
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0      // PC.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_1      // PC.01
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_10  // ADC3_IN10
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_11  // ADC3_IN11
#endif
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_7      // PF.07
#if !defined(RADIO_T15)
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_2      // PC.02 //
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_6      // PF.06
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_3      // PC.03 //
  #define ADC_GPIO_PIN_EXT1             LL_GPIO_PIN_8      // PF.08
  #define ADC_GPIO_PIN_EXT2             LL_GPIO_PIN_9      // PF.09
  #define ADC_GPIO_PIN_EXT3             ADC_GPIO_PIN_STICK_RH
  #define ADC_GPIO_PIN_EXT4             ADC_GPIO_PIN_STICK_RV
#endif
  #if !(defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_T15) || defined(RADIO_V16))
    #define PWM_STICKS
    #define PWM_TIMER                   TIM5
    #define PWM_GPIO                    GPIOA
    #define PWM_GPIO_AF                 GPIO_AF2
    #define PWM_IRQHandler              TIM5_IRQHandler
    #define PWM_IRQn                    TIM5_IRQn
    #define PWM_GPIOA_PINS              (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)
    #define STICK_PWM_CHANNEL_LH          0
    #define STICK_PWM_CHANNEL_LV          1
    #define STICK_PWM_CHANNEL_RV          3
    #define STICK_PWM_CHANNEL_RH          2
  #endif
#if defined(RADIO_T15)
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_1   // ADC3_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_0   // ADC3_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_3   // ADC3_IN3
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_2   // ADC3_IN2
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_5   // ADC3_IN5
#else
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0   // ADC3_IN0
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1   // ADC3_IN1
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_2   // ADC3_IN2
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_3   // ADC3_IN3
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_5   // ADC3_IN5
#endif
#if !defined(RADIO_T15)
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_12  // ADC3_IN12
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_4   // ADC3_IN4
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_13  // ADC3_IN13
  #define ADC_CHANNEL_EXT1              LL_ADC_CHANNEL_6   // ADC3_IN6
  #define ADC_CHANNEL_EXT2              LL_ADC_CHANNEL_7   // ADC3_IN7
  #define ADC_CHANNEL_EXT3              LL_ADC_CHANNEL_2   // ADC3_IN2: same as RH
  #define ADC_CHANNEL_EXT4              LL_ADC_CHANNEL_3   // ADC3_IN3: same as RV
#endif
#if defined(RADIO_T15)
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_BATT)
#else
  #define ADC_GPIOA_PINS_FS             (LL_GPIO_PIN_2 | LL_GPIO_PIN_3)
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_BATT | ADC_GPIO_PIN_EXT1 | ADC_GPIO_PIN_EXT2)
#endif
  #define ADC_CHANNEL_RTC_BAT           LL_ADC_CHANNEL_VBAT // ADC1_IN18
  #define ADC_MAIN                      ADC3
  #define ADC_EXT                       ADC1
  #if defined(RADIO_V16)
    #define ADC_EXT_CHANNELS            { ADC_CHANNEL_SLIDER1, ADC_CHANNEL_SLIDER2, ADC_CHANNEL_RTC_BAT }
    // Required DMA for more than one channel for EXT_ADC
    #define ADC_EXT_DMA                    DMA2
    #define ADC_EXT_DMA_CHANNEL            LL_DMA_CHANNEL_0
    #define ADC_EXT_DMA_STREAM             LL_DMA_STREAM_4
    #define ADC_EXT_DMA_STREAM_IRQ         DMA2_Stream4_IRQn
    #define ADC_EXT_DMA_STREAM_IRQHandler  DMA2_Stream4_IRQHandler
  #else
    #define ADC_EXT_CHANNELS              { ADC_CHANNEL_RTC_BAT }
  #endif
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_SAMPTIME                  LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_2
  #define ADC_DMA_STREAM                LL_DMA_STREAM_0
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream0_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream0_IRQHandler

  // VBat divider is /4 on F42x and F43x devices
  #if defined(RADIO_TX16S) || defined(RADIO_T15) || defined(RADIO_F16) || defined(RADIO_V16)
    #define ADC_VREF_PREC2              660
  #elif defined(RADIO_T16) || defined(RADIO_T18)
    #define ADC_VREF_PREC2              600
  #else
    #define ADC_VREF_PREC2              500
  #endif
#endif

#if defined(RADIO_T15)
  #define ADC_DIRECTION                 {1,-1,1,-1,  1,1}
#elif defined(RADIO_T16)
  #define ADC_DIRECTION                 {1,-1,1,-1,  1,1,1,   -1,1,1,1,  -1,1 }
#elif defined(RADIO_T18)
  #define ADC_DIRECTION                 {1,-1,1,-1, -1,1,-1,  -1,1,1,1,  -1,1 }
#elif defined(RADIO_TX16S) || defined(RADIO_F16)
  #define ADC_DIRECTION                 {1,-1,1,-1,  1,1,1,   -1,1,1,1,  -1,1 }
#elif defined(RADIO_V16)
  #define ADC_DIRECTION                 {1,-1,1,-1,  -1,1,-1,   -1,1,1,1,  -1,1 }
#elif defined(PCBX10)
  #define ADC_DIRECTION                 {1,-1,1,-1,  -1,1,-1,  1,-1,1,1,   1,-1 }
#elif defined(PCBX12S)
  #define ADC_DIRECTION                 {1,-1,1,-1,  -1,1,-1,  1,-1,  -1,-1, 0,0,0}
#else
  #error "Missing ADC_DIRECTION array"
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
    #define PCBREV_VALUE()  {0}
#elif defined(PCBX10)
  #define PCBREV_GPIO_1                 GPIO_PIN(GPIOH, 7) // PH.07
  #define PCBREV_GPIO_2                 GPIO_PIN(GPIOH, 8) // PH.08
  #define PCBREV_TOUCH_GPIO             GPIO_PIN(GPIOA, 6) // PA.06
  #define PCBREV_VALUE()                ((gpio_read(PCBREV_GPIO_1) | gpio_read(PCBREV_GPIO_2)) >> 7)
  #define PCBREV_TOUCH_GPIO_PULL_UP
#else
  #define PCBREV_GPIO                   GPIO_PIN(GPIOI, 11) // PI.11
  #define PCBREV_VALUE()                (gpio_read(PCBREV_GPIO) >> 11)
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
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 4) // PE.04
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
#define SD_SDIO_INIT_CLK_DIV              SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV          SD_SDIO_CLK_DIV(24000000)

// Uncomment only one line below to select which storage to use
#define STORAGE_USE_SDIO      // Use SD card for storage with SDIO driver
//#define STORAGE_USE_SPI_FLASH // Use SPI flash for storage instead of SD card

// SPI NOR Flash 
#if defined(PCBX12S) && PCBREV >= 13
  #define FLASH_SPI                      SPI1
  #define FLASH_SPI_CS_GPIO              GPIOA
  #define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_15 // PA.15
  #define FLASH_SPI_GPIO                 GPIOA
  #define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_5  // PA.05
  #define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_6  // PA.06
  #define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_7  // PA.07
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
  #define FLASH_SPI_CS_GPIO              GPIOI
  #define FLASH_SPI_CS_GPIO_PIN          LL_GPIO_PIN_0  // PI.00
  #define FLASH_SPI_GPIO                 GPIOI
  #define FLASH_SPI_SCK_GPIO_PIN         LL_GPIO_PIN_1  // PI.01
  #define FLASH_SPI_MISO_GPIO_PIN        LL_GPIO_PIN_2  // PI.02
  #define FLASH_SPI_MOSI_GPIO_PIN        LL_GPIO_PIN_3  // PI.03
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
  #define AUDIO_SHUTDOWN_GPIO           GPIO_PIN(GPIOI, 9)  // PI.09
  #define AUDIO_XDCS_GPIO               GPIOI
  #define AUDIO_XDCS_GPIO_PIN           LL_GPIO_PIN_0  // PI.00
  #define AUDIO_CS_GPIO                 GPIOH
  #define AUDIO_CS_GPIO_PIN             LL_GPIO_PIN_13 // PH.13
  #define AUDIO_DREQ_GPIO               GPIOH
  #define AUDIO_DREQ_GPIO_PIN           LL_GPIO_PIN_14 // PH.14
  #define AUDIO_RST_GPIO                GPIOH
  #define AUDIO_RST_GPIO_PIN            LL_GPIO_PIN_15 // PH.15
  #define AUDIO_SPI                     SPI2
  #define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
  #define AUDIO_SPI_SCK_GPIO            GPIOI
  #define AUDIO_SPI_SCK_GPIO_PIN        LL_GPIO_PIN_1  // PI.01
  #define AUDIO_SPI_MISO_GPIO           GPIOI
  #define AUDIO_SPI_MISO_GPIO_PIN       LL_GPIO_PIN_2  // PI.02
  #define AUDIO_SPI_MOSI_GPIO           GPIOI
  #define AUDIO_SPI_MOSI_GPIO_PIN       LL_GPIO_PIN_3  // PI.03
#elif defined (PCBX10)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOA, 7) // PA.07
  #define AUDIO_OUTPUT_GPIO             GPIO_PIN(GPIOA, 4) // PA.04
  #define AUDIO_DMA_Stream              DMA1_Stream5
  #define AUDIO_DMA_Stream_IRQn         DMA1_Stream5_IRQn
  #define AUDIO_TIM_IRQn                TIM6_DAC_IRQn
  #define AUDIO_TIM_IRQHandler          TIM6_DAC_IRQHandler
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
  // #define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
#endif

#if defined(RADIO_V16)
  // LED Strip
  #define LED_STRIP_LENGTH                  40
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

#define PORTRAIT_LCD false
#define LANDSCAPE_LCD true
#if defined(RADIO_T15)
#define LCD_W                          480
#define LCD_H                          320
#define LCD_PHYS_H                     LCD_H
#define LCD_PHYS_W                     LCD_W
#define LCD_DEPTH                      16
#else
#define LCD_W                          480
#define LCD_H                          272
#define LCD_PHYS_H                     LCD_H
#define LCD_PHYS_W                     LCD_W
#define LCD_DEPTH                      16
#endif
