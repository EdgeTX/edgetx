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

#define CPU_FREQ                        168000000
#define PERI1_FREQUENCY                 42000000
#define PERI2_FREQUENCY                 84000000
#define TIMER_MULT_APB1                 2
#define TIMER_MULT_APB2                 2

// Keys
#define KEYS_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOJ)
#if defined(PCBX12S)
  #define KEYS_GPIO_REG_PGUP            GPIOC->IDR
  #define KEYS_GPIO_PIN_PGUP            GPIO_Pin_13 // PC.13
  #define KEYS_GPIO_REG_PGDN            GPIOI->IDR
  #define KEYS_GPIO_PIN_PGDN            GPIO_Pin_8  // PI.08
  #define KEYS_GPIO_REG_LEFT            GPIOI->IDR
  #define KEYS_GPIO_PIN_LEFT            GPIO_Pin_7  // PI.07
  #define KEYS_GPIO_REG_ENTER           GPIOC->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_1  // PC.01
  #define KEYS_GPIO_REG_UP              GPIOG->IDR
  #define KEYS_GPIO_PIN_UP              GPIO_Pin_13 // PG.13
  #define KEYS_GPIO_REG_DOWN            GPIOI->IDR
  #define KEYS_GPIO_PIN_DOWN            GPIO_Pin_6  // PI.06
  #define KEYS_GPIO_REG_RIGHT           GPIOC->IDR
  #define KEYS_GPIO_PIN_RIGHT           GPIO_Pin_4  // PC.04
#elif defined(RADIO_TX16S)
  #define KEYS_GPIO_REG_ENTER           GPIOI->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_8  // PI.08
  #define KEYS_GPIO_REG_PGUP            GPIOC->IDR
  #define KEYS_GPIO_PIN_PGUP            GPIO_Pin_13 // PC.13
  #define KEYS_GPIO_REG_PGDN            GPIOI->IDR
  #define KEYS_GPIO_PIN_PGDN            GPIO_Pin_11 // PI.11
  #define KEYS_GPIO_REG_UP              GPIOI->IDR
  #define KEYS_GPIO_PIN_UP              GPIO_Pin_6  // PI.06
  #define KEYS_GPIO_REG_DOWN            GPIOI->IDR
  #define KEYS_GPIO_PIN_DOWN            GPIO_Pin_4  // PI.04
  #define KEYS_GPIO_REG_LEFT            GPIOI->IDR
  #define KEYS_GPIO_PIN_LEFT            GPIO_Pin_7  // PI.07
  #define KEYS_GPIO_REG_RIGHT           GPIOI->IDR
  #define KEYS_GPIO_PIN_RIGHT           GPIO_Pin_5  // PI.05
#elif defined(PCBX10)
  #define KEYS_GPIO_REG_ENTER           GPIOI->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_8  // PI.08
  #define KEYS_GPIO_REG_PGDN            GPIOI->IDR
  #define KEYS_GPIO_PIN_PGDN            GPIO_Pin_11 // PI.11
  #define KEYS_GPIO_REG_UP              GPIOI->IDR
  #define KEYS_GPIO_PIN_UP              GPIO_Pin_4  // PI.04
  #define KEYS_GPIO_REG_DOWN            GPIOI->IDR
  #define KEYS_GPIO_PIN_DOWN            GPIO_Pin_6  // PI.06
  #define KEYS_GPIO_REG_LEFT            GPIOI->IDR
  #define KEYS_GPIO_PIN_LEFT            GPIO_Pin_7  // PI.07
  #define KEYS_GPIO_REG_RIGHT           GPIOI->IDR
  #define KEYS_GPIO_PIN_RIGHT           GPIO_Pin_5  // PI.05
#endif

// Rotary Encoder
#define ROTARY_ENCODER_RCC_APB1Periph   RCC_APB1Periph_TIM4
#define ROTARY_ENCODER_GPIO             GPIOH
#define ROTARY_ENCODER_GPIO_PIN_A       GPIO_Pin_11 // PH.11
#define ROTARY_ENCODER_GPIO_PIN_B       GPIO_Pin_10 // PH.10
#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 10) & 0x03)
#define ROTARY_ENCODER_EXTI_LINE1       EXTI_Line11
#define ROTARY_ENCODER_EXTI_LINE2       EXTI_Line10
#define ROTARY_ENCODER_EXTI_IRQn1       EXTI15_10_IRQn
#define ROTARY_ENCODER_EXTI_IRQHandler1 EXTI15_10_IRQHandler
#define ROTARY_ENCODER_EXTI_PortSource  EXTI_PortSourceGPIOH
#define ROTARY_ENCODER_EXTI_PinSource1  EXTI_PinSource11
#define ROTARY_ENCODER_EXTI_PinSource2  EXTI_PinSource10
#define ROTARY_ENCODER_TIMER            TIM4
#define ROTARY_ENCODER_TIMER_IRQn       TIM4_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM4_IRQHandler

// Switches
#define STORAGE_SWITCH_A
#define HARDWARE_SWITCH_A
#define SWITCHES_GPIO_REG_A_H           GPIOH->IDR
#define SWITCHES_GPIO_PIN_A_H           GPIO_Pin_9  // PH.09
#define SWITCHES_GPIO_REG_A_L           GPIOI->IDR
#define SWITCHES_GPIO_PIN_A_L           GPIO_Pin_15 // PI.15
#define STORAGE_SWITCH_B
#define HARDWARE_SWITCH_B
#define SWITCHES_GPIO_REG_B_H           GPIOH->IDR
#define SWITCHES_GPIO_PIN_B_H           GPIO_Pin_12 // PH.12
#define SWITCHES_GPIO_REG_B_L           GPIOB->IDR
#define SWITCHES_GPIO_PIN_B_L           GPIO_Pin_12 // PB.12
#define STORAGE_SWITCH_C
#define HARDWARE_SWITCH_C
#define SWITCHES_GPIO_REG_C_H           GPIOD->IDR
#define SWITCHES_GPIO_PIN_C_H           GPIO_Pin_11 // PD.11
#define SWITCHES_GPIO_REG_C_L           GPIOB->IDR
#define SWITCHES_GPIO_PIN_C_L           GPIO_Pin_15 // PB.15
#define STORAGE_SWITCH_D
#define HARDWARE_SWITCH_D
#define SWITCHES_GPIO_REG_D_H           GPIOJ->IDR
#define SWITCHES_GPIO_PIN_D_H           GPIO_Pin_7  // PJ.07
#define SWITCHES_GPIO_REG_D_L           GPIOG->IDR
#define SWITCHES_GPIO_PIN_D_L           GPIO_Pin_2  // PG.02
#define STORAGE_SWITCH_E
#define HARDWARE_SWITCH_E
#define SWITCHES_GPIO_REG_E_H           GPIOH->IDR
#define SWITCHES_GPIO_PIN_E_H           GPIO_Pin_4  // PH.04
#define SWITCHES_GPIO_REG_E_L           GPIOE->IDR
#define SWITCHES_GPIO_PIN_E_L           GPIO_Pin_3  // PE.03
#define STORAGE_SWITCH_F
#define HARDWARE_SWITCH_F
#define SWITCHES_GPIO_REG_F             GPIOH->IDR
#define SWITCHES_GPIO_PIN_F             GPIO_Pin_3  // PH.03
#define STORAGE_SWITCH_G
#define HARDWARE_SWITCH_G
#define SWITCHES_GPIO_REG_G_H           GPIOG->IDR
#define SWITCHES_GPIO_PIN_G_H           GPIO_Pin_6  // PG.06
#define SWITCHES_GPIO_REG_G_L           GPIOG->IDR
#define SWITCHES_GPIO_PIN_G_L           GPIO_Pin_3  // PG.03
#define STORAGE_SWITCH_H
#define HARDWARE_SWITCH_H
#define SWITCHES_GPIO_REG_H             GPIOG->IDR
#define SWITCHES_GPIO_PIN_H             GPIO_Pin_7  // PG.07

#if defined(PCBX10)
  // Gimbal switch left
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOH->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_14 // PH.14
  // Gimbal switch right
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOH->IDR
  #define SWITCHES_GPIO_PIN_J           GPIO_Pin_15 // PH.15
#elif defined(PCBX12S)
  // Gimbal switch left
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOB->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_1 // PB.01
  // Gimbal switch right
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOB->IDR
  #define SWITCHES_GPIO_PIN_J           GPIO_Pin_0 // PB.00
#endif

// Trims
#if defined(PCBX12S)
  #define TRIMS_GPIO_REG_RHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_0  // PC.00
  #define TRIMS_GPIO_REG_RHR            GPIOI->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_4  // PI.04
  #define TRIMS_GPIO_REG_RVD            GPIOG->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_12 // PG.12
  #define TRIMS_GPIO_REG_RVU            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_14 // PJ.14
  #define TRIMS_GPIO_REG_LVD            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_13 // PJ.13
  #define TRIMS_GPIO_REG_LHL            GPIOD->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_3  // PD.03
  #define TRIMS_GPIO_REG_LVU            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_12 // PJ.12
  #define TRIMS_GPIO_REG_LHR            GPIOD->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_7  // PD.07
  #define TRIMS_GPIO_REG_RSD            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_RSD            GPIO_Pin_8  // PJ.08
  #define TRIMS_GPIO_REG_RSU            GPIOD->IDR
  #define TRIMS_GPIO_PIN_RSU            GPIO_Pin_13 // PD.13
  #define TRIMS_GPIO_REG_LSD            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LSD            GPIO_Pin_14 // PB.14
  #define TRIMS_GPIO_REG_LSU            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LSU            GPIO_Pin_13 // PB.13
#elif defined(PCBX10)
  #if defined(RADIO_TX16S)
    #define TRIMS_GPIO_REG_LHL            GPIOA->IDR
    #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_6  // PA.06
    #define TRIMS_GPIO_REG_LHR            GPIOC->IDR
    #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_4  // PC.04
  #else
    #define TRIMS_GPIO_REG_LHL            GPIOB->IDR
    #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_8  // PB.08
    #define TRIMS_GPIO_REG_LHR            GPIOB->IDR
    #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_9  // PB.09
  #endif
  #define TRIMS_GPIO_REG_LVD            GPIOG->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_12 // PG.12
  #define TRIMS_GPIO_REG_LVU            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_14 // PJ.14
  #define TRIMS_GPIO_REG_RVD            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_13 // PJ.13
  #define TRIMS_GPIO_REG_RHL            GPIOD->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_3  // PD.03
  #define TRIMS_GPIO_REG_RVU            GPIOJ->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_12 // PJ.12
  #define TRIMS_GPIO_REG_RHR            GPIOD->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_7  // PD.07
  #if defined(RADIO_FAMILY_T16)
    #define TRIMS_GPIO_REG_LSU          GPIOD->IDR
    #define TRIMS_GPIO_PIN_LSU          GPIO_Pin_13 // PD.13
    #define TRIMS_GPIO_REG_LSD          GPIOJ->IDR
    #define TRIMS_GPIO_PIN_LSD          GPIO_Pin_8  // PJ.08
  #else
    #define TRIMS_GPIO_REG_LSU          GPIOJ->IDR
    #define TRIMS_GPIO_PIN_LSU          GPIO_Pin_8  // PJ.08
    #define TRIMS_GPIO_REG_LSD          GPIOD->IDR
    #define TRIMS_GPIO_PIN_LSD          GPIO_Pin_13 // PD.13
  #endif
  #define TRIMS_GPIO_REG_RSU            GPIOB->IDR
  #define TRIMS_GPIO_PIN_RSU            GPIO_Pin_14 // PB.14
  #define TRIMS_GPIO_REG_RSD            GPIOB->IDR
  #define TRIMS_GPIO_PIN_RSD            GPIO_Pin_13 // PB.13
#endif

// Index of all keys
#if defined(PCBX12S)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_13)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_3)
  #define KEYS_GPIOG_PINS               (KEYS_GPIO_PIN_UP | SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L | SWITCHES_GPIO_PIN_H | TRIMS_GPIO_PIN_RVD)
  #define KEYS_GPIOH_PINS               (GPIO_Pin_9 | GPIO_Pin_12 | SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_F | ROTARY_ENCODER_GPIO_PIN_A | ROTARY_ENCODER_GPIO_PIN_B)
  #define KEYS_GPIOI_PINS               (KEYS_GPIO_PIN_PGDN | KEYS_GPIO_PIN_LEFT | KEYS_GPIO_PIN_DOWN | SWITCHES_GPIO_PIN_A_L | GPIO_Pin_4)
  #define KEYS_GPIOJ_PINS               (SWITCHES_GPIO_PIN_D_H | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_RSD)
#elif defined(PCBX10)
  #if defined(RADIO_TX16S)
    #define KEYS_GPIOB_PINS             (GPIO_Pin_12 | GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13)
  #else
    #define KEYS_GPIOB_PINS             (GPIO_Pin_12 | GPIO_Pin_15 | GPIO_Pin_14 | GPIO_Pin_13 | GPIO_Pin_8 | GPIO_Pin_9)
  #endif
  #define KEYS_GPIOD_PINS               (GPIO_Pin_11 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_13)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_3)
  #define KEYS_GPIOG_PINS               (SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L | SWITCHES_GPIO_PIN_H | TRIMS_GPIO_PIN_LVD)
#if defined(RADIO_TX16S)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_6)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_4 | GPIO_Pin_13)
  #define KEYS_GPIOH_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15)
#else
  #define KEYS_GPIOH_PINS               (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15)
#endif
  #define KEYS_GPIOI_PINS               (GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_15)
  #define KEYS_GPIOJ_PINS               (SWITCHES_GPIO_PIN_D_H | TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | GPIO_Pin_8)
#endif

// ADC
#if defined(PCBX12S)
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            (RCC_APB2Periph_SPI4 | RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC3)
  #define ADC_SPI                       SPI4
  #define ADC_GPIO_AF                   GPIO_AF_SPI4
  #define ADC_SPI_GPIO                  GPIOE
  #define ADC_SPI_PIN_SCK               GPIO_Pin_2 // PE.02
  #define ADC_SPI_PIN_CS                GPIO_Pin_4 // PE.04
  #define ADC_SPI_PIN_MOSI              GPIO_Pin_6 // PE.06
  #define ADC_SPI_PIN_MISO              GPIO_Pin_5 // PE.05
  #define ADC_SPI_PinSource_SCK         GPIO_PinSource2
  #define ADC_SPI_PinSource_MISO        GPIO_PinSource5
  #define ADC_SPI_PinSource_MOSI        GPIO_PinSource6
  #define ADC_GPIO_PIN_MOUSE1           GPIO_Pin_8 // PF.08 ADC3_IN6 J5 MOUSE_X
  #define ADC_GPIO_PIN_MOUSE2           GPIO_Pin_9 // PF.09 ADC3_IN7 J6 MOUSE_Y
  #define ADC_GPIO_MOUSE                GPIOF
  #define ADC_CHANNEL_MOUSE1            6
  #define ADC_CHANNEL_MOUSE2            7
  #define ADC_DMA                       DMA2
  #define ADC_DMA_Stream                DMA2_Stream0
  #define ADC_SAMPTIME                  3
  #define ADC_VREF_PREC2                300
#elif defined(PCBX10)
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            (RCC_APB1Periph_TIM5)
  #define ADC_RCC_APB2Periph            (RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC3)
  #define ADC_GPIO_PIN_STICK_LH       GPIO_Pin_0      // PA.00
  #define ADC_GPIO_PIN_STICK_LV       GPIO_Pin_1      // PA.01
  #define ADC_GPIO_PIN_STICK_RH       GPIO_Pin_2      // PA.02
  #define ADC_GPIO_PIN_STICK_RV       GPIO_Pin_3      // PA.03
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_0      // PC.00
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_1      // PC.01
  #define ADC_GPIO_PIN_POT3             GPIO_Pin_2      // PC.02
  #define ADC_GPIO_PIN_SLIDER1          GPIO_Pin_6      // PF.06
  #define ADC_GPIO_PIN_SLIDER2          GPIO_Pin_3      // PC.03
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_7      // PF.07
  #define ADC_GPIO_PIN_EXT1             GPIO_Pin_8      // PF.08
  #define ADC_GPIO_PIN_EXT2             GPIO_Pin_9      // PF.09
  #if !defined(RADIO_TX16S)
    #define PWM_TIMER                   TIM5
    #define PWM_GPIO                    GPIOA
    #define PWM_GPIO_AF                 GPIO_AF_TIM5
    #define PWM_IRQHandler              TIM5_IRQHandler
    #define PWM_IRQn                    TIM5_IRQn
    #define PWM_GPIOA_PINS              (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #endif
  #define ADC_GPIOA_PINS_FS            (GPIO_Pin_2 | GPIO_Pin_3)
  #define ADC_GPIOA_PINS               (STICKS_PWM_ENABLED() ? 0 : (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3))
  #define ADC_GPIOC_PINS                (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define ADC_GPIOF_PINS                (GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9)
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_0   // ADC3_IN0
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_1   // ADC3_IN1
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_2   // ADC3_IN2
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_3   // ADC3_IN3
  #define ADC_CHANNEL_POT1              ADC_Channel_10  // ADC3_IN10
  #define ADC_CHANNEL_POT2              ADC_Channel_11  // ADC3_IN11
  #define ADC_CHANNEL_POT3              ADC_Channel_12  // ADC3_IN12
  #define ADC_CHANNEL_SLIDER1           ADC_Channel_4   // ADC3_IN4
  #define ADC_CHANNEL_SLIDER2           ADC_Channel_13  // ADC3_IN13
  #define ADC_CHANNEL_BATT              ADC_Channel_5   // ADC3_IN5
  #define ADC_CHANNEL_EXT1              ADC_Channel_6   // ADC3_IN6
  #define ADC_CHANNEL_EXT2              ADC_Channel_7   // ADC3_IN7
  #define ADC_CHANNEL_EXT3              ADC_Channel_2   // ADC3_IN2: same as RH
  #define ADC_CHANNEL_EXT4              ADC_Channel_3   // ADC3_IN3: same as RV
  #define ADC_MAIN                      ADC3
  #define ADC_EXT                       ADC1
  #define ADC_SAMPTIME                  3
  #define ADC_DMA                       DMA2
  #define ADC_DMA_Channel               DMA_Channel_2
  #define ADC_DMA_Stream                DMA2_Stream0
  #define ADC_TRANSFER_COMPLETE()       (ADC_DMA->LISR & DMA_LISR_TCIF0)
  #define ADC_DMA_TC_Flag               DMA_FLAG_TCIF0

  #if defined(RADIO_TX16S)
    #define ADC_VREF_PREC2              330
  #elif defined(RADIO_T16) || defined(RADIO_T18)
    #define ADC_VREF_PREC2              300
  #else
    #define ADC_VREF_PREC2              250
  #endif
#endif

// Power
#if defined(RADIO_T18)
#define PWR_RCC_AHB1Periph              RCC_AHB1Periph_GPIOJ | RCC_AHB1Periph_GPIOB
#define PWR_ON_GPIO                     GPIOJ
#define PWR_ON_GPIO_PIN                 GPIO_Pin_1  // PJ.01
#define PWR_SWITCH_GPIO                 GPIOJ
#define PWR_SWITCH_GPIO_PIN             GPIO_Pin_0  // PJ.00
#define PWR_EXTRA_SWITCH_GPIO           GPIOB
#define PWR_EXTRA_SWITCH_GPIO_PIN       GPIO_Pin_0  // PB.00
#else
#define PWR_RCC_AHB1Periph              RCC_AHB1Periph_GPIOJ
#define PWR_ON_GPIO                     GPIOJ
#define PWR_ON_GPIO_PIN                 GPIO_Pin_1  // PJ.01
#define PWR_SWITCH_GPIO                 GPIOJ
#define PWR_SWITCH_GPIO_PIN             GPIO_Pin_0  // PJ.00
#endif

// USB Charger
#if defined(USB_CHARGER)
#define USB_CHARGER_RCC_AHB1Periph      RCC_AHB1Periph_GPIOG
#define USB_CHARGER_GPIO                GPIOG
#define USB_CHARGER_GPIO_PIN            GPIO_Pin_11  // PG.11
#define USB_USBDet_GPIO_PIN             GPIO_Pin_13  // PG.13
#else
#define USB_CHARGER_RCC_AHB1Periph      0
#endif

// S.Port update connector
#if defined(RADIO_FAMILY_T16)
  #define SPORT_MAX_BAUDRATE              400000
#else
  #define SPORT_MAX_BAUDRATE              250000 // < 400000
#endif

#if defined(PCBX10) && !defined(RADIO_FAMILY_T16)
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOH
  #define SPORT_UPDATE_PWR_GPIO         GPIOH
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_13  // PH.13
  #define HAS_SPORT_UPDATE_CONNECTOR()  true
#else
  #define SPORT_UPDATE_RCC_AHB1Periph   0
  #define HAS_SPORT_UPDATE_CONNECTOR()  false
#endif

// PCBREV
#if defined(PCBX10)
  #define PCBREV_RCC_AHB1Periph         RCC_AHB1Periph_GPIOH
  #define PCBREV_GPIO_PIN               (GPIO_Pin_7 | GPIO_Pin_8)
  #define PCBREV_GPIO                   GPIOH
  #define PCBREV_VALUE()                (GPIO_ReadInputDataBit(PCBREV_GPIO, GPIO_Pin_7) + (GPIO_ReadInputDataBit(PCBREV_GPIO, GPIO_Pin_8) << 1))
#else
  #define PCBREV_RCC_AHB1Periph         RCC_AHB1Periph_GPIOI
  #define PCBREV_GPIO                   GPIOI
  #define PCBREV_GPIO_PIN               GPIO_Pin_11  // PI.11
  #define PCBREV_VALUE()                GPIO_ReadInputDataBit(PCBREV_GPIO, PCBREV_GPIO_PIN)
#endif

// Led
#define STATUS_LEDS
#if defined(PCBX12S)
  #define LED_RCC_AHB1Periph            RCC_AHB1Periph_GPIOI
  #define LED_GPIO                      GPIOI
  #define LED_GPIO_PIN                  GPIO_Pin_5  // PI.05
#elif defined(PCBX10)
  #define LED_RCC_AHB1Periph            RCC_AHB1Periph_GPIOE
  #define LED_GPIO                      GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_2
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_4
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_5
  #define LED_GPIO_PIN                  (LED_RED_GPIO_PIN | LED_GREEN_GPIO_PIN | LED_BLUE_GPIO_PIN)
#endif

// Serial Port (DEBUG)
#if defined(AUX_SERIAL)
  #define AUX_SERIAL_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
  #define AUX_SERIAL_RCC_APB1Periph           RCC_APB1Periph_USART3
  #define AUX_SERIAL_RCC_APB2Periph           0
  #define AUX_SERIAL_GPIO                     GPIOB
  #define AUX_SERIAL_GPIO_PIN_TX              GPIO_Pin_10 // PB.10
  #define AUX_SERIAL_GPIO_PIN_RX              GPIO_Pin_11 // PB.11
  #define AUX_SERIAL_GPIO_PinSource_TX        GPIO_PinSource10
  #define AUX_SERIAL_GPIO_PinSource_RX        GPIO_PinSource11
  #define AUX_SERIAL_GPIO_AF                  GPIO_AF_USART3
  #define AUX_SERIAL_GPIO_AF_LL               LL_GPIO_AF_7
  #define AUX_SERIAL_USART                    USART3
  #define AUX_SERIAL_USART_IRQHandler         USART3_IRQHandler
  #define AUX_SERIAL_USART_IRQn               USART3_IRQn
  #define AUX_SERIAL_DMA_RX                   DMA1
  #define AUX_SERIAL_DMA_Stream_RX            DMA1_Stream1
  #define AUX_SERIAL_DMA_Stream_RX_LL         LL_DMA_STREAM_1
  #define AUX_SERIAL_DMA_Channel_RX           DMA_Channel_4
  #if defined(RADIO_TX16S)
    #define AUX_SERIAL_PWR_GPIO                 GPIOA
    #define AUX_SERIAL_PWR_GPIO_PIN             GPIO_Pin_15  // PA.15
  #endif
#else
  #define AUX_SERIAL_RCC_AHB1Periph           0
  #define AUX_SERIAL_RCC_APB1Periph           0
  #define AUX_SERIAL_RCC_APB2Periph           0
#endif

#if defined(AUX2_SERIAL)
#if defined(PCBX12S)
  #define AUX2_SERIAL_RCC_AHB1Periph           RCC_AHB1Periph_GPIOA
  #define AUX2_SERIAL_RCC_APB1Periph           RCC_APB1Periph_UART4
  #define AUX2_SERIAL_RCC_APB2Periph           0
  #define AUX2_SERIAL_USART                    UART4
  #define AUX2_SERIAL_GPIO_AF                  GPIO_AF_UART4
  #define AUX2_SERIAL_GPIO_AF_LL               LL_GPIO_AF_8
  #define AUX2_SERIAL_USART_IRQn               UART4_IRQn
  #define AUX2_SERIAL_USART_IRQHandler         UART4_IRQHandler
  #define AUX2_SERIAL_GPIO                     GPIOA
  #define AUX2_SERIAL_GPIO_PIN_TX              GPIO_Pin_0 // PA.00
  #define AUX2_SERIAL_GPIO_PIN_RX              GPIO_Pin_1 // PA.01
  #define AUX2_SERIAL_GPIO_TX_PinSource        GPIO_PinSource0
  #define AUX2_SERIAL_GPIO_RX_PinSource        GPIO_PinSource1
  #define AUX2_SERIAL_DMA_RX                   DMA1
  #define AUX2_SERIAL_DMA_Stream_RX            DMA1_Stream2
  #define AUX2_SERIAL_DMA_Channel_RX           DMA_Channel_4
  #define AUX2_SERIAL_DMA_Stream_RX_LL         LL_DMA_STREAM_2
#else
  #define AUX2_SERIAL_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_DMA2)
  #define AUX2_SERIAL_RCC_APB1Periph           0
  #define AUX2_SERIAL_RCC_APB2Periph           RCC_APB2Periph_USART6
  #define AUX2_SERIAL_USART                    USART6
  #define AUX2_SERIAL_GPIO_AF                  GPIO_AF_USART6
  #define AUX2_SERIAL_GPIO_AF_LL               LL_GPIO_AF_8
  #define AUX2_SERIAL_USART_IRQn               USART6_IRQn
  #define AUX2_SERIAL_USART_IRQHandler         USART6_IRQHandler
  #define AUX2_SERIAL_GPIO                     GPIOG
  #define AUX2_SERIAL_GPIO_PIN_TX              GPIO_Pin_14 // PG.14
  #define AUX2_SERIAL_GPIO_PIN_RX              GPIO_Pin_9  // PG.09
  #define AUX2_SERIAL_GPIO_PinSource_TX        GPIO_PinSource14
  #define AUX2_SERIAL_GPIO_PinSource_RX        GPIO_PinSource9
  #define AUX2_SERIAL_DMA_RX                   DMA2
  #define AUX2_SERIAL_DMA_Stream_RX            DMA2_Stream1 // or DMA2_Stream2
  #define AUX2_SERIAL_DMA_Channel_RX           DMA_Channel_5
  #define AUX2_SERIAL_DMA_Stream_RX_LL         LL_DMA_STREAM_1
  #define AUX2_SERIAL_PWR_GPIO                 GPIOB
  #define AUX2_SERIAL_PWR_GPIO_PIN             GPIO_Pin_0  // PB.00
#endif
#else // AUX2_SERIAL
  #define AUX2_SERIAL_RCC_AHB1Periph           0
  #define AUX2_SERIAL_RCC_APB1Periph           0
  #define AUX2_SERIAL_RCC_APB2Periph           0
#endif

// Telemetry
#define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
#define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
#define TELEMETRY_RCC_APB2Periph        RCC_APB2Periph_TIM11
#define TELEMETRY_DIR_GPIO              GPIOD
#define TELEMETRY_DIR_GPIO_PIN          GPIO_Pin_4  // PD.04
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_5  // PD.05
#define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_6  // PD.06
#define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource5
#define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource6
#define TELEMETRY_GPIO_AF               GPIO_AF_USART2
#define TELEMETRY_USART                 USART2
#define TELEMETRY_EXTI_PortSource       EXTI_PortSourceGPIOD
#define TELEMETRY_EXTI_PinSource        EXTI_PinSource6
#define TELEMETRY_EXTI_LINE             EXTI_Line6
#define TELEMETRY_EXTI_IRQn             EXTI9_5_IRQn
#define TELEMETRY_EXTI_IRQHandler       EXTI9_5_IRQHandler
#define TELEMETRY_EXTI_TRIGGER          EXTI_Trigger_Rising
#define TELEMETRY_TIMER                 TIM11
#define TELEMETRY_TIMER_IRQn            TIM1_TRG_COM_TIM11_IRQn
#define TELEMETRY_TIMER_IRQHandler      TIM1_TRG_COM_TIM11_IRQHandler
#if defined(PCBX12S)
  #define TELEMETRY_DMA_Stream_RX       DMA1_Stream5
  #define TELEMETRY_DMA_Channel_RX      DMA_Channel_4
#endif
#define TELEMETRY_DMA_Stream_TX         DMA1_Stream6
#define TELEMETRY_DMA_Channel_TX        DMA_Channel_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               GPIO_Pin_9  // PA.09
#define USB_GPIO_PIN_DM                 GPIO_Pin_11 // PA.11
#define USB_GPIO_PIN_DP                 GPIO_Pin_12 // PA.12
#define USB_GPIO_PinSource_DM           GPIO_PinSource11
#define USB_GPIO_PinSource_DP           GPIO_PinSource12
#define USB_GPIO_AF                     GPIO_AF_OTG1_FS

// LCD
#define LCD_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOJ | RCC_AHB1Periph_GPIOK | RCC_AHB1Periph_DMA2D)
#define LCD_RCC_APB1Periph              0
#define LCD_RCC_APB2Periph              RCC_APB2Periph_LTDC
#if defined(PCBX12S)
  #define LCD_GPIO_NRST                 GPIOF
  #define LCD_GPIO_PIN_NRST             GPIO_Pin_10 // PF.10
#elif defined(PCBX10)
  #define LCD_GPIO_NRST                 GPIOI
  #define LCD_GPIO_PIN_NRST             GPIO_Pin_10 // PI.10
#endif
#if defined(PCBX10) && !defined(RADIO_T18)
  #define LCD_VERTICAL_INVERT
#endif
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
#if defined(PCBX12S)
  #define BACKLIGHT_RCC_AHB1Periph             RCC_AHB1Periph_GPIOA
  #define BACKLIGHT_GPIO                       GPIOA
  #if PCBREV >= 13
    #define BACKLIGHT_TIMER                    TIM5
    #define BACKLIGHT_GPIO_PIN                 GPIO_Pin_3  // PA.03
    #define BACKLIGHT_GPIO_PinSource           GPIO_PinSource3
    #define BACKLIGHT_RCC_APB1Periph           RCC_APB1Periph_TIM5
    #define BACKLIGHT_RCC_APB2Periph           0
    #define BACKLIGHT_GPIO_AF                  GPIO_AF_TIM5
    #define BACKLIGHT_TIMER_FREQ               (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #else
    #define BACKLIGHT_TIMER                    TIM8
    #define BACKLIGHT_GPIO_PIN                 GPIO_Pin_5  // PA.05
    #define BACKLIGHT_GPIO_PinSource           GPIO_PinSource5
    #define BACKLIGHT_RCC_APB1Periph           0
    #define BACKLIGHT_RCC_APB2Periph           RCC_APB2Periph_TIM8
    #define BACKLIGHT_GPIO_AF                  GPIO_AF_TIM8
    #define BACKLIGHT_TIMER_FREQ               (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #endif
#elif defined(PCBX10)
  #define BACKLIGHT_RCC_AHB1Periph             RCC_AHB1Periph_GPIOB
  #define BACKLIGHT_GPIO                       GPIOB
  #define BACKLIGHT_TIMER                      TIM8
  #define BACKLIGHT_GPIO_PIN                   GPIO_Pin_1  // PB.01
  #define BACKLIGHT_GPIO_PinSource             GPIO_PinSource1
  #define BACKLIGHT_RCC_APB1Periph             0
  #define BACKLIGHT_RCC_APB2Periph             RCC_APB2Periph_TIM8
  #define BACKLIGHT_GPIO_AF                    GPIO_AF_TIM8
  #define BACKLIGHT_TIMER_FREQ                 (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif
#if defined(RADIO_T18)
  #define KEYS_BACKLIGHT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOC
  #define KEYS_BACKLIGHT_GPIO                  GPIOC
  #define KEYS_BACKLIGHT_GPIO_PIN              GPIO_Pin_4  // PC.04
  #define KEYS_BACKLIGHT_GPIO_PinSource        GPIO_PinSource4
#else
  #define KEYS_BACKLIGHT_RCC_AHB1Periph        0
#endif

// SD
#define SD_RCC_AHB1Periph               (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
#define SD_RCC_APB1Periph               0
#define SD_PRESENT_GPIO                 GPIOC
#define SD_PRESENT_GPIO_PIN             GPIO_Pin_5  // PC.05
#define SD_SDIO_DMA_STREAM              DMA2_Stream3
#define SD_SDIO_DMA_CHANNEL             DMA_Channel_4
#define SD_SDIO_DMA_FLAG_FEIF           DMA_FLAG_FEIF3
#define SD_SDIO_DMA_FLAG_DMEIF          DMA_FLAG_DMEIF3
#define SD_SDIO_DMA_FLAG_TEIF           DMA_FLAG_TEIF3
#define SD_SDIO_DMA_FLAG_HTIF           DMA_FLAG_HTIF3
#define SD_SDIO_DMA_FLAG_TCIF           DMA_FLAG_TCIF3
#define SD_SDIO_DMA_IRQn                DMA2_Stream3_IRQn
#define SD_SDIO_DMA_IRQHANDLER          DMA2_Stream3_IRQHandler
#define SD_SDIO_FIFO_ADDRESS            ((uint32_t)0x40012C80)
#define SD_SDIO_CLK_DIV(fq)             ((48000000 / (fq)) - 2)
#define SD_SDIO_INIT_CLK_DIV            SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV        SD_SDIO_CLK_DIV(24000000)

// EEPROM
#if defined(PCBX12S) && PCBREV >= 13
  #define EEPROM_RCC_AHB1Periph           RCC_AHB1Periph_GPIOA
  #define EEPROM_RCC_APB1Periph           RCC_APB1Periph_SPI1
  #define EEPROM_SPI_CS_GPIO              GPIOA
  #define EEPROM_SPI_CS_GPIO_PIN          GPIO_Pin_15 // PA.15
  #define EEPROM_SPI_SCK_GPIO             GPIOA
  #define EEPROM_SPI_SCK_GPIO_PIN         GPIO_Pin_5  // PA.05
  #define EEPROM_SPI_SCK_GPIO_PinSource   GPIO_PinSource5
  #define EEPROM_SPI_MISO_GPIO            GPIOA
  #define EEPROM_SPI_MISO_GPIO_PIN        GPIO_Pin_6  // PA.06
  #define EEPROM_SPI_MISO_GPIO_PinSource  GPIO_PinSource6
  #define EEPROM_SPI_MOSI_GPIO            GPIOA
  #define EEPROM_SPI_MOSI_GPIO_PIN        GPIO_Pin_7  // PA.07
  #define EEPROM_SPI_MOSI_GPIO_PinSource  GPIO_PinSource7
#elif defined(PCBX10)
  #define EEPROM_RCC_AHB1Periph           RCC_AHB1Periph_GPIOI
  #define EEPROM_RCC_APB1Periph           RCC_APB1Periph_SPI2
  #define EEPROM_SPI_CS_GPIO              GPIOI
  #define EEPROM_SPI_CS_GPIO_PIN          GPIO_Pin_0  // PI.00
  #define EEPROM_SPI_SCK_GPIO             GPIOI
  #define EEPROM_SPI_SCK_GPIO_PIN         GPIO_Pin_1  // PI.01
  #define EEPROM_SPI_SCK_GPIO_PinSource   GPIO_PinSource1
  #define EEPROM_SPI_MISO_GPIO            GPIOI
  #define EEPROM_SPI_MISO_GPIO_PIN        GPIO_Pin_2  // PI.02
  #define EEPROM_SPI_MISO_GPIO_PinSource  GPIO_PinSource2
  #define EEPROM_SPI_MOSI_GPIO            GPIOI
  #define EEPROM_SPI_MOSI_GPIO_PIN        GPIO_Pin_3  // PI.03
  #define EEPROM_SPI_MOSI_GPIO_PinSource  GPIO_PinSource3
#endif

// Audio
#if defined(PCBX12S)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH)
  #define AUDIO_RCC_APB1Periph          RCC_APB1Periph_SPI2
  #define AUDIO_SHUTDOWN_GPIO           GPIOI
  #define AUDIO_SHUTDOWN_GPIO_PIN       GPIO_Pin_9  // PI.09
  #define AUDIO_XDCS_GPIO               GPIOI
  #define AUDIO_XDCS_GPIO_PIN           GPIO_Pin_0  // PI.00
  #define AUDIO_CS_GPIO                 GPIOH
  #define AUDIO_CS_GPIO_PIN             GPIO_Pin_13 // PH.13
  #define AUDIO_DREQ_GPIO               GPIOH
  #define AUDIO_DREQ_GPIO_PIN           GPIO_Pin_14 // PH.14
  #define AUDIO_RST_GPIO                GPIOH
  #define AUDIO_RST_GPIO_PIN            GPIO_Pin_15 // PH.15
  #define AUDIO_SPI                     SPI2
  #define AUDIO_SPI_GPIO_AF             GPIO_AF_SPI2
  #define AUDIO_SPI_SCK_GPIO            GPIOI
  #define AUDIO_SPI_SCK_GPIO_PIN        GPIO_Pin_1  // PI.01
  #define AUDIO_SPI_SCK_GPIO_PinSource  GPIO_PinSource1
  #define AUDIO_SPI_MISO_GPIO           GPIOI
  #define AUDIO_SPI_MISO_GPIO_PIN       GPIO_Pin_2  // PI.02
  #define AUDIO_SPI_MISO_GPIO_PinSource GPIO_PinSource2
  #define AUDIO_SPI_MOSI_GPIO           GPIOI
  #define AUDIO_SPI_MOSI_GPIO_PIN       GPIO_Pin_3  // PI.03
  #define AUDIO_SPI_MOSI_GPIO_PinSource GPIO_PinSource3
#elif defined (PCBX10)
  #define AUDIO_RCC_APB1Periph          (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_MUTE_GPIO               GPIOA
  #define AUDIO_MUTE_GPIO_PIN           GPIO_Pin_7  // PA.07
  #define AUDIO_OUTPUT_GPIO             GPIOA
  #define AUDIO_OUTPUT_GPIO_PIN         GPIO_Pin_4  // PA.04
  #define AUDIO_GPIO_AF                 GPIO_AF_DAC1
  #define AUDIO_GPIO_PinSource          GPIO_PinSource4
  #define AUDIO_DMA_Stream              DMA1_Stream5
  #define AUDIO_DMA_Stream_IRQn         DMA1_Stream5_IRQn
  #define AUDIO_TIM_IRQn                TIM6_DAC_IRQn
  #define AUDIO_TIM_IRQHandler          TIM6_DAC_IRQHandler
  #define AUDIO_DMA_Stream_IRQHandler   DMA1_Stream5_IRQHandler
  #define AUDIO_TIMER                   TIM6
  #define AUDIO_DMA                     DMA1
#endif

#if defined(RADIO_FAMILY_T16)
  #define AUDIO_UNMUTE_DELAY            120  // ms
  #define AUDIO_MUTE_DELAY              500  // ms
#endif

#if defined(RADIO_TX16S)
// Only slight noise with 868MHz > 1W, if complaints later remove and set AUDIO_UNMUTE_DELAY to 150
  #undef AUDIO_MUTE_GPIO_PIN
#endif

// Touch
#if defined(HARDWARE_TOUCH)
  #define TOUCH_I2C_BUS                   I2C_Bus_1
  #define TOUCH_I2C_CLK_RATE              400000

  #define TOUCH_INT_GPIO                  GPIOH
  #define TOUCH_INT_GPIO_PIN              LL_GPIO_PIN_2    // PH.02

  #define TOUCH_RST_GPIO                  GPIOF
#if defined(PCBX12S)
  #define TOUCH_RST_GPIO_PIN              LL_GPIO_PIN_7   // PF.7
#else
  #define TOUCH_RST_GPIO_PIN              LL_GPIO_PIN_10  // PF.10
#endif
  #define TOUCH_INT_EXTI_LINE1            EXTI_Line2
  #define TOUCH_INT_EXTI_IRQn1            EXTI2_IRQn
  #define TOUCH_INT_EXTI_IRQHandler1      EXTI2_IRQHandler
  #define TOUCH_INT_EXTI_PortSource       EXTI_PortSourceGPIOH
  #define TOUCH_INT_EXTI_PinSource1       EXTI_PinSource2

  #define TOUCH_INT_EXTI_Line             LL_EXTI_LINE_2
  #define TOUCH_INT_EXTI_Port             LL_SYSCFG_EXTI_PORTH
  #define TOUCH_INT_EXTI_SysCfgLine       LL_SYSCFG_EXTI_LINE2
  #define TOUCH_INT_EXTI_IRQn             EXTI2_IRQn
  #define TOUCH_INT_EXTI_IRQHandler       EXTI2_IRQHandler

#if defined(PCBX12S)
  #define TOUCH_PANEL_INVERTED
#endif
#endif // HARDWARE_TOUCH

// First I2C Bus
#if defined(RADIO_T18) || defined(RADIO_T16)
  #define I2C_B1                          I2C3
  #define I2C_B1_GPIO                     GPIOH
  #define I2C_B1_SCL_GPIO_PIN             LL_GPIO_PIN_7  // PH.07
  #define I2C_B1_SDA_GPIO_PIN             LL_GPIO_PIN_8  // PH.08
  #define I2C_B1_GPIO_AF                  LL_GPIO_AF_4   // I2C3
#else
  #define I2C_B1                          I2C1
  #define I2C_B1_GPIO                     GPIOB
  #define I2C_B1_SCL_GPIO_PIN             LL_GPIO_PIN_8  // PB.08
  #define I2C_B1_SDA_GPIO_PIN             LL_GPIO_PIN_9  // PB.09
  #define I2C_B1_GPIO_AF                  LL_GPIO_AF_4   // I2C1
#endif

// Second I2C Bus
#if !defined(AUX_SERIAL) && defined(IMU_LSM6DS33)
  #define I2C_B2                          I2C2
  #define I2C_B2_GPIO                     GPIOB
  #define I2C_B2_SCL_GPIO_PIN             LL_GPIO_PIN_10  // PB.10
  #define I2C_B2_SDA_GPIO_PIN             LL_GPIO_PIN_11  // PB.11
  #define I2C_B2_GPIO_AF                  LL_GPIO_AF_4    // I2C2
#endif

#if defined(IMU)
 #if defined(PCBX12S)
   #define IMU_I2C_BUS                   I2C_Bus_1
   #define IMU_I2C_ADDRESS               0x6A
 #elif !defined(AUX_SERIAL) && defined(IMU_LSM6DS33)
   #define IMU_I2C_BUS                   I2C_Bus_2
   #define IMU_I2C_ADDRESS               0x6A
 #endif
 #define IMU_I2C_CLK_RATE                400000
#endif

// Haptic
#define HAPTIC_PWM
#if defined(PCBX12S)
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOA
  #define HAPTIC_RCC_APB2Periph         RCC_APB2ENR_TIM9EN
  #define HAPTIC_GPIO                   GPIOA
  #define HAPTIC_GPIO_PIN               GPIO_Pin_2
  #define HAPTIC_GPIO_TIMER             TIM9
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM9
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource2
  #define HAPTIC_TIMER_OUTPUT_ENABLE    TIM_CCER_CC1E
  #define HAPTIC_TIMER_MODE             TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2
  #define HAPTIC_TIMER_COMPARE_VALUE    HAPTIC_GPIO_TIMER->CCR1
#elif defined(PCBX10)
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOE
  #define HAPTIC_RCC_APB2Periph         RCC_APB2ENR_TIM9EN
  #define HAPTIC_GPIO                   GPIOE
  #define HAPTIC_GPIO_PIN               GPIO_Pin_6  // PE.06
  #define HAPTIC_GPIO_TIMER             TIM9
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM9
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource6
  #define HAPTIC_TIMER_OUTPUT_ENABLE    TIM_CCER_CC2E
  #define HAPTIC_TIMER_MODE             TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_TIMER_COMPARE_VALUE    HAPTIC_GPIO_TIMER->CCR2
  // FlySky Hall Sticks
  #define FLYSKY_HALL_SERIAL_USART                 UART4
  #define FLYSKY_HALL_SERIAL_GPIO                  GPIOA
  #define FLYSKY_HALL_DMA_Channel                  DMA_Channel_4
  #define FLYSKY_HALL_SERIAL_TX_GPIO_PIN           GPIO_Pin_0  // PA.00
  #define FLYSKY_HALL_SERIAL_RX_GPIO_PIN           GPIO_Pin_1  // PA.01
  #define FLYSKY_HALL_SERIAL_TX_GPIO_PinSource     GPIO_PinSource0
  #define FLYSKY_HALL_SERIAL_RX_GPIO_PinSource     GPIO_PinSource1
  #define FLYSKY_HALL_SERIAL_GPIO_AF               GPIO_AF_UART4

  #define FLYSKY_HALL_RCC_AHB1Periph               RCC_AHB1Periph_DMA1
  #define FLYSKY_HALL_RCC_APB1Periph               RCC_APB1Periph_UART4

  #define FLYSKY_HALL_SERIAL_USART_IRQHandler      UART4_IRQHandler
  #define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
  #define FLYSKY_HALL_SERIAL_RX_DMA_Stream_IRQn    DMA1_Stream2_IRQn
  #define FLYSKY_HALL_SERIAL_TX_DMA_Stream_IRQn    DMA1_Stream4_IRQn
  #define FLYSKY_HALL_DMA_Stream_RX                DMA1_Stream2
  #define FLYSKY_HALL_DMA_Stream_TX                DMA1_Stream4
  #define FLYSKY_HALL_DMA_TX_FLAG_TC               DMA_IT_TCIF4

  #define FLYSKY_HALL_RX_DMA_Stream_IRQHandler     DMA1_Stream2_IRQHandler
  #define FLYSKY_HALL_TX_DMA_Stream_IRQHandler     DMA1_Stream4_IRQHandler
#endif

// Internal Module
#if !defined(RADIO_FAMILY_T16)
  #define EXTERNAL_ANTENNA
#endif
#define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
#define INTMODULE_PWR_GPIO              GPIOA
#define INTMODULE_PWR_GPIO_PIN          GPIO_Pin_8  // PA.08
#define INTMODULE_GPIO                  GPIOB
#define INTMODULE_TX_GPIO_PIN           LL_GPIO_PIN_6  // PB.06
#define INTMODULE_RX_GPIO_PIN           LL_GPIO_PIN_7  // PB.07
#define INTMODULE_USART                 USART1
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART1_IRQn
#define INTMODULE_USART_IRQHandler      USART1_IRQHandler
#define INTMODULE_DMA                   DMA2
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_7
#define INTMODULE_DMA_STREAM_IRQ        DMA2_Stream7_IRQn
#define INTMODULE_DMA_STREAM_IRQHandler DMA2_Stream7_IRQHandler
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF7
#define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_4
#define INTMODULE_RX_DMA                DMA2
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_2
#define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_4
#if defined(PCBX12S)
  #define INTMODULE_BOOTCMD_GPIO        GPIOC
  #define INTMODULE_BOOTCMD_GPIO_PIN    GPIO_Pin_2  // PC.02
#elif defined(PCBX10)
  #define INTMODULE_BOOTCMD_GPIO        GPIOI
  #define INTMODULE_BOOTCMD_GPIO_PIN    GPIO_Pin_9  // PI.09
#endif
#define INIT_INTMODULE_BOOTCMD_PIN() GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#define INTMODULE_RCC_APB1Periph      0
#define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1

// External Module
#define EXTMODULE_PWR_GPIO                 GPIOB
#define EXTMODULE_PWR_GPIO_PIN             GPIO_Pin_3  // PB.03
#define EXTERNAL_MODULE_PWR_OFF()          GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
#if (defined(PCBX10) && defined(PCBREV_EXPRESS)) || defined(HARDWARE_EXTERNAL_ACCESS_MOD)
  #define EXTMODULE_RCC_AHB1Periph         (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
  #define EXTMODULE_RCC_APB1Periph         (RCC_APB1Periph_TIM2 | RCC_APB1Periph_USART3)
  #define EXTMODULE_RCC_APB2Periph         0
  #define EXTMODULE_TX_GPIO                GPIOB
  #define EXTMODULE_USART_GPIO             EXTMODULE_TX_GPIO
  #define EXTMODULE_TX_GPIO_PIN            LL_GPIO_PIN_10 // PB.10 (TIM2_CH3)
  #define EXTMODULE_RX_GPIO_PIN            LL_GPIO_PIN_11 // PB.11
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF_TIM2
  #define EXTMODULE_TIMER                  TIM2
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define EXTMODULE_TIMER_32BITS
  #define EXTMODULE_TIMER_DMA_SIZE         (DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1)
  #define EXTMODULE_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define EXTMODULE_TIMER_IRQn             TIM2_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM2_IRQHandler
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_3
  #define EXTMODULE_TIMER_DMA_STREAM       DMA1_Stream1
  #define EXTMODULE_TIMER_DMA              DMA1
  #define EXTMODULE_TIMER_DMA_STREAM_LL    LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_FLAG_TC      DMA_IT_TCIF1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA1_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA1_Stream1_IRQHandler
  #define EXTMODULE_USART_GPIO_AF          GPIO_AF_USART3
  #define EXTMODULE_USART_GPIO_AF_LL       LL_GPIO_AF_7
  #define EXTMODULE_USART                  USART3
  #define EXTMODULE_USART_IRQn             USART3_IRQn
  #define EXTMODULE_USART_IRQHandler       USART3_IRQHandler
  #define EXTMODULE_USART_TX_DMA           DMA1
  #define EXTMODULE_USART_TX_DMA_CHANNEL   LL_DMA_CHANNEL_4
  #define EXTMODULE_USART_TX_DMA_STREAM    DMA1_Stream3
  #define EXTMODULE_USART_TX_DMA_STREAM_LL LL_DMA_STREAM_3
  #define EXTMODULE_USART_TX_DMA_FLAG_TC   DMA_FLAG_TCIF3
  #define EXTMODULE_USART_RX_DMA_CHANNEL   LL_DMA_CHANNEL_4
  #define EXTMODULE_USART_RX_DMA_STREAM    DMA1_Stream1
#elif defined(PCBX10) || PCBREV >= 13
  #define EXTMODULE_RCC_AHB1Periph         (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB1Periph         0
  #define EXTMODULE_RCC_APB2Periph         RCC_APB2Periph_TIM1
  #define EXTMODULE_TX_GPIO                GPIOA
  #define EXTMODULE_TX_GPIO_PIN            GPIO_Pin_10 // PA.10 (TIM1_CH3)
  #define EXTMODULE_TX_GPIO_PinSource      GPIO_PinSource10
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF_TIM1
  #define EXTMODULE_TIMER                  TIM1
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define EXTMODULE_TIMER_DMA_SIZE         (DMA_SxCR_PSIZE_0 | DMA_SxCR_MSIZE_0)
  #define EXTMODULE_TIMER_IRQn             TIM1_UP_TIM10_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM1_UP_TIM10_IRQHandler
  #define EXTMODULE_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_6
  #define EXTMODULE_TIMER_DMA_STREAM       DMA2_Stream5
  #define EXTMODULE_TIMER_DMA              DMA2
  #define EXTMODULE_TIMER_DMA_STREAM_LL    LL_DMA_STREAM_5
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA2_Stream5_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA2_Stream5_IRQHandler
  #define EXTMODULE_TIMER_DMA_FLAG_TC      DMA_IT_TCIF5
#else
  #define EXTMODULE_RCC_AHB1Periph         (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
  #define EXTMODULE_RCC_APB1Periph         RCC_APB1Periph_TIM2
  #define EXTMODULE_RCC_APB2Periph         0
  #define EXTMODULE_TX_GPIO                GPIOA
  #define EXTMODULE_TX_GPIO_PIN            GPIO_Pin_15 // PA.15 (TIM2_CH1)
  #define EXTMODULE_TX_GPIO_PinSource      GPIO_PinSource15
  #define EXTMODULE_TIMER_TX_GPIO_AF       GPIO_AF_TIM2
  #define EXTMODULE_TIMER                  TIM2
  #define EXTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH1
  #define EXTMODULE_TIMER_32BITS
  #define EXTMODULE_TIMER_DMA_SIZE         (DMA_SxCR_PSIZE_1 | DMA_SxCR_MSIZE_1)
  #define EXTMODULE_TIMER_IRQn             TIM2_IRQn
  #define EXTMODULE_TIMER_IRQHandler       TIM2_IRQHandler
  #define EXTMODULE_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define EXTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_3
  #define EXTMODULE_TIMER_DMA_STREAM       DMA1_Stream7
  #define EXTMODULE_TIMER_DMA              DMA1
  #define EXTMODULE_TIMER_DMA_STREAM_LL    LL_DMA_STREAM_7
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn  DMA1_Stream7_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler   DMA1_Stream7_IRQHandler
  #define EXTMODULE_TIMER_DMA_FLAG_TC      DMA_IT_TCIF7
#endif

// Heartbeat
#define INTMODULE_HEARTBEAT
#define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
#define INTMODULE_HEARTBEAT_GPIO                GPIOD
#define INTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_12   // PD.12
#define INTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOD
#define INTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource12
#define INTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line12
#define INTMODULE_HEARTBEAT_EXTI_IRQn           EXTI15_10_IRQn
#define INTMODULE_HEARTBEAT_EXTI_IRQHandler     EXTI15_10_IRQHandler
#define INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER
#if defined(INTERNAL_MODULE_PXX2)
  #define INTMODULE_HEARTBEAT_TRIGGER           EXTI_Trigger_Falling
#else
  #define INTMODULE_HEARTBEAT_TRIGGER           EXTI_Trigger_Rising
#endif

// Trainer Port
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA1)
#define TRAINER_RCC_APB1Periph          RCC_APB1Periph_TIM3
#define TRAINER_GPIO                    GPIOC
#define TRAINER_IN_GPIO_PIN             GPIO_Pin_6  // PC.06
#define TRAINER_IN_GPIO_PinSource       GPIO_PinSource6
#define TRAINER_OUT_GPIO_PIN            GPIO_Pin_7  // PC.07
#define TRAINER_OUT_GPIO_PinSource      GPIO_PinSource7
#define TRAINER_DETECT_GPIO             GPIOB
#define TRAINER_DETECT_GPIO_PIN         GPIO_Pin_4  // PB.04
#define TRAINER_TIMER                   TIM3
#define TRAINER_TIMER_IRQn              TIM3_IRQn
#define TRAINER_TIMER_IRQHandler        TIM3_IRQHandler
#define TRAINER_GPIO_AF                 GPIO_AF_TIM3
#define TRAINER_DMA                     DMA1
#define TRAINER_DMA_CHANNEL             DMA_Channel_5
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Xms Interrupt
#define INTERRUPT_xMS_RCC_APB1Periph    RCC_APB1Periph_TIM14
#define INTERRUPT_xMS_TIMER             TIM14
#define INTERRUPT_xMS_IRQn              TIM8_TRG_COM_TIM14_IRQn
#define INTERRUPT_xMS_IRQHandler        TIM8_TRG_COM_TIM14_IRQHandler

// 2MHz Timer
#define TIMER_2MHz_RCC_APB1Periph       RCC_APB1Periph_TIM7
#define TIMER_2MHz_TIMER                TIM7

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER_RCC_APB1Periph RCC_APB1Periph_TIM13
#define MIXER_SCHEDULER_TIMER                TIM13
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_UP_TIM13_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_UP_TIM13_IRQHandler

// Bluetooth
#define STORAGE_BLUETOOTH
#if defined(BLUETOOTH)
#define BT_RCC_APB2Periph               RCC_APB2Periph_USART6
#define BT_USART                        USART6
#define BT_GPIO_AF                      GPIO_AF_USART6
#define BT_USART_IRQn                   USART6_IRQn
#define BT_USART_GPIO                   GPIOG
#define BT_TX_GPIO_PIN                  GPIO_Pin_14 // PG.14
#define BT_RX_GPIO_PIN                  GPIO_Pin_9  // PG.09
#define BT_TX_GPIO_PinSource            GPIO_PinSource14
#define BT_RX_GPIO_PinSource            GPIO_PinSource9
#define BT_USART_IRQHandler             USART6_IRQHandler
#else
#define BT_RCC_APB2Periph               0
#endif
#if defined(PCBX12S)
  #if PCBREV >= 13
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOG)
    #define BT_EN_GPIO                  GPIOI
    #define BT_EN_GPIO_PIN              GPIO_Pin_10 // PI.10
  #else
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOG)
    #define BT_EN_GPIO                  GPIOA
    #define BT_EN_GPIO_PIN              GPIO_Pin_6 // PA.06
  #endif
  #define BT_BRTS_GPIO                  GPIOG
  #define BT_BRTS_GPIO_PIN              GPIO_Pin_10 // PG.10
  #define BT_BCTS_GPIO                  GPIOG
  #define BT_BCTS_GPIO_PIN              GPIO_Pin_11 // PG.11
#elif defined(PCBX10)
  #define BT_RCC_AHB1Periph             RCC_AHB1Periph_GPIOG
  #define BT_EN_GPIO                    GPIOG
  #define BT_EN_GPIO_PIN                GPIO_Pin_10 // PG.10
#endif

#endif // _HAL_H_
