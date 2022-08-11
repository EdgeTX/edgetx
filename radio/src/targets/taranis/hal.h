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

#if defined(STM32F4)
  #define CPU_FREQ            168000000
  #define PERI1_FREQUENCY     42000000
  #define PERI2_FREQUENCY     84000000
#else
  #define CPU_FREQ            120000000
  #define PERI1_FREQUENCY     30000000
  #define PERI2_FREQUENCY     60000000
#endif

#define TIMER_MULT_APB1                 2
#define TIMER_MULT_APB2                 2

// Keys
#if defined(PCBX9E)
  #define KEYS_GPIO_REG_MENU            GPIOD->IDR
  #define KEYS_GPIO_PIN_MENU            GPIO_Pin_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_2  // PD.02
  #define KEYS_GPIO_REG_PAGE            GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGE            GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_ENTER           GPIOF->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_0  // PF.00
#elif defined(PCBXLITE)
  #define KEYS_GPIO_REG_SHIFT           GPIOE->IDR
  #define KEYS_GPIO_PIN_SHIFT           GPIO_Pin_8  // PE.08
  #define KEYS_GPIO_REG_EXIT            GPIOE->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_7  // PE.07
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_11 // PE.11
  #define KEYS_GPIO_REG_UP              GPIOE->IDR
  #define KEYS_GPIO_PIN_UP              GPIO_Pin_10 // PE.10
  #define KEYS_GPIO_REG_DOWN            GPIOE->IDR
  #define KEYS_GPIO_PIN_DOWN            GPIO_Pin_14 // PE.14
  #define KEYS_GPIO_REG_LEFT            GPIOE->IDR
  #define KEYS_GPIO_PIN_LEFT            GPIO_Pin_12 // PE.12
  #define KEYS_GPIO_REG_RIGHT           GPIOE->IDR
  #define KEYS_GPIO_PIN_RIGHT           GPIO_Pin_13 // PE.13
#elif defined(RADIO_FAMILY_JUMPER_T12) && !defined(RADIO_TPRO)
  #define KEYS_GPIO_REG_EXIT            GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_10 // PE.10
  #define KEYS_GPIO_REG_UP              GPIOE->IDR
  #define KEYS_GPIO_PIN_UP              GPIO_Pin_9 // PE.09
  #define KEYS_GPIO_REG_DOWN            GPIOE->IDR
  #define KEYS_GPIO_PIN_DOWN            GPIO_Pin_11 // PE.11
  #define KEYS_GPIO_REG_LEFT            GPIOD->IDR
  #define KEYS_GPIO_PIN_LEFT            GPIO_Pin_7 // PD.07
  #define KEYS_GPIO_REG_RIGHT           GPIOD->IDR
  #define KEYS_GPIO_PIN_RIGHT           GPIO_Pin_3 // PD.03
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGEUP          GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGEDN          GPIO_Pin_7  // PD.07
#if defined(RADIO_TX12)
  #define KEYS_GPIO_REG_EXIT            GPIOB->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_3  // PB.03
#else
  #define KEYS_GPIO_REG_EXIT            GPIOC->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_5  // PC.05
#endif
#if defined(RADIO_TX12MK2)
  #define KEYS_GPIO_REG_ENTER           GPIOA->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_10 // PA.10
#else
  #define KEYS_GPIO_REG_ENTER           GPIOA->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_13 // PA.13

#endif
  #define KEYS_GPIO_REG_SYS             GPIOB->IDR
  #define KEYS_GPIO_PIN_SYS             GPIO_Pin_4  // PB.04
  #define KEYS_GPIO_REG_MDL             GPIOE->IDR
  #define KEYS_GPIO_PIN_MDL             GPIO_Pin_11 // PE.11
  #define KEYS_GPIO_REG_TELE            GPIOD->IDR
  #define KEYS_GPIO_PIN_TELE            GPIO_Pin_2  // PD.02
#elif defined(RADIO_T8)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGEUP          GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGEDN          GPIO_Pin_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOB->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_3  // PB.03
  #define KEYS_GPIO_REG_ENTER           GPIOA->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_13 // PA.13
  #define KEYS_GPIO_REG_SYS             GPIOB->IDR
  #define KEYS_GPIO_PIN_SYS             GPIO_Pin_4  // PB.04
  #define KEYS_GPIO_REG_MDL             GPIOE->IDR
  #define KEYS_GPIO_PIN_MDL             GPIO_Pin_11 // PE.11
  #define KEYS_GPIO_REG_PLUS            GPIOE->IDR
  #define KEYS_GPIO_PIN_PLUS            GPIO_Pin_9  // PE.09
  #define KEYS_GPIO_REG_MINUS           GPIOE->IDR
  #define KEYS_GPIO_PIN_MINUS           GPIO_Pin_10 // PE.10
  #define KEYS_GPIO_REG_BIND            GPIOA->IDR
  #define KEYS_GPIO_PIN_BIND            GPIO_Pin_10 // PA.10
#elif defined(PCBX7)
  #define KEYS_GPIO_REG_PAGE            GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGE            GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_MENU            GPIOD->IDR
  #define KEYS_GPIO_PIN_MENU            GPIO_Pin_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_10 // PE.10
#elif defined(PCBX9LITE)
  #define KEYS_GPIO_REG_PAGE            GPIOE->IDR
  #define KEYS_GPIO_PIN_PAGE            GPIO_Pin_8  // PE.08
  #define KEYS_GPIO_REG_MENU            GPIOE->IDR
  #define KEYS_GPIO_PIN_MENU            GPIO_Pin_7  // PE.07
  #define KEYS_GPIO_REG_EXIT            GPIOE->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_9  // PE.09
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_11 // PE.11
#elif defined(RADIO_X9DP2019)
  #define KEYS_GPIO_REG_MENU            GPIOD->IDR
  #define KEYS_GPIO_PIN_MENU            GPIO_Pin_2  // PD.02
  #define KEYS_GPIO_REG_EXIT            GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_7  // PD.07
  #define KEYS_GPIO_REG_PAGE            GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGE            GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_12 // PE.12
#else
  #define KEYS_GPIO_REG_MENU            GPIOD->IDR
  #define KEYS_GPIO_PIN_MENU            GPIO_Pin_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT            GPIO_Pin_2  // PD.02
  #define KEYS_GPIO_REG_PAGE            GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGE            GPIO_Pin_3  // PD.03
  #define KEYS_GPIO_REG_PLUS            GPIOE->IDR
  #define KEYS_GPIO_PIN_PLUS            GPIO_Pin_10 // PE.10
  #define KEYS_GPIO_REG_MINUS           GPIOE->IDR
  #define KEYS_GPIO_PIN_MINUS           GPIO_Pin_11 // PE.11
  #define KEYS_GPIO_REG_ENTER           GPIOE->IDR
  #define KEYS_GPIO_PIN_ENTER           GPIO_Pin_12 // PE.12
#endif

// Rotary Encoder
#if defined(PCBX9E)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOD
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_12 // PD.12
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_13 // PD.13
  #define ROTARY_ENCODER_POSITION()     (ROTARY_ENCODER_GPIO->IDR >> 12) & 0x03
  #define ROTARY_ENCODER_EXTI_LINE1     EXTI_Line12
  #define ROTARY_ENCODER_EXTI_LINE2     EXTI_Line13
  #define ROTARY_ENCODER_EXTI_IRQn1        EXTI15_10_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1  EXTI15_10_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource   EXTI_PortSourceGPIOD
  #define ROTARY_ENCODER_EXTI_PinSource1   EXTI_PinSource12
  #define ROTARY_ENCODER_EXTI_PinSource2   EXTI_PinSource13
#elif defined(RADIO_X9DP2019)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_11 // PE.11
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x02) + ((ROTARY_ENCODER_GPIO->IDR >> 11) & 0x01))
  #define ROTARY_ENCODER_EXTI_LINE1     EXTI_Line10
  #define ROTARY_ENCODER_EXTI_LINE2     EXTI_Line11
  #define ROTARY_ENCODER_EXTI_IRQn1        EXTI15_10_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1  EXTI15_10_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource   EXTI_PortSourceGPIOE
  #define ROTARY_ENCODER_EXTI_PinSource1   EXTI_PinSource10
  #define ROTARY_ENCODER_EXTI_PinSource2   EXTI_PinSource11
#elif defined(RADIO_X7) || defined(RADIO_X7ACCESS) || defined(RADIO_TPRO)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_9  // PE.09
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_11 // PE.11
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 10) & 0x02) + ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x01))
  #define ROTARY_ENCODER_EXTI_LINE1     EXTI_Line9
  #define ROTARY_ENCODER_EXTI_LINE2     EXTI_Line11
  #define ROTARY_ENCODER_EXTI_IRQn1        EXTI9_5_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1  EXTI9_5_IRQHandler
  #define ROTARY_ENCODER_EXTI_IRQn2        EXTI15_10_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler2  EXTI15_10_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource  EXTI_PortSourceGPIOE
  #define ROTARY_ENCODER_EXTI_PinSource1  EXTI_PinSource9
  #define ROTARY_ENCODER_EXTI_PinSource2  EXTI_PinSource11
#elif defined(PCBX9LITE)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_12 // PE.12
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 12) & 0x01) + ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x02))
  #define ROTARY_ENCODER_EXTI_LINE1     EXTI_Line10
  #define ROTARY_ENCODER_EXTI_LINE2     EXTI_Line12
  #define ROTARY_ENCODER_EXTI_IRQn1        EXTI15_10_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1  EXTI15_10_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource  EXTI_PortSourceGPIOE
  #define ROTARY_ENCODER_EXTI_PinSource1  EXTI_PinSource10
  #define ROTARY_ENCODER_EXTI_PinSource2  EXTI_PinSource12
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO              GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A        GPIO_Pin_9 // PE.9
  #define ROTARY_ENCODER_GPIO_PIN_B        GPIO_Pin_10 // PE.10
  #define ROTARY_ENCODER_POSITION()        ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x03)
  #define ROTARY_ENCODER_EXTI_LINE1        EXTI_Line9
  #define ROTARY_ENCODER_EXTI_LINE2        EXTI_Line10
  #define ROTARY_ENCODER_EXTI_IRQn1        EXTI9_5_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1  EXTI9_5_IRQHandler
  #define ROTARY_ENCODER_EXTI_IRQn2        EXTI15_10_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler2  EXTI15_10_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource   EXTI_PortSourceGPIOE
  #define ROTARY_ENCODER_EXTI_PinSource1   EXTI_PinSource9
  #define ROTARY_ENCODER_EXTI_PinSource2   EXTI_PinSource10
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define ROTARY_ENCODER_RCC_APB1Periph   RCC_APB1Periph_TIM5
  #define ROTARY_ENCODER_TIMER            TIM5
  #define ROTARY_ENCODER_TIMER_IRQn       TIM5_IRQn
  #define ROTARY_ENCODER_TIMER_IRQHandler TIM5_IRQHandler
#else
  #define ROTARY_ENCODER_RCC_APB1Periph   0
#endif

// Trims
#if defined(PCBX9E)
  #define TRIMS_GPIO_REG_LHL            GPIOG->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_1  // PG.01
  #define TRIMS_GPIO_REG_LHR            GPIOG->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_0  // PG.00
  #define TRIMS_GPIO_REG_LVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_4  // PE.04
  #define TRIMS_GPIO_REG_LVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_3  // PE.03
  #define TRIMS_GPIO_REG_RVD            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_1  // PC.01
  #define TRIMS_GPIO_REG_RVU            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_13 // PC.13
#elif defined(PCBXLITE)
  #define TRIMS_GPIO_REG_LHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_4  // PC.04
  #define TRIMS_GPIO_REG_LHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_5  // PC.05
  #define TRIMS_GPIO_REG_LVU            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_0  // PB.00
  #define TRIMS_GPIO_REG_LVD            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_1  // PB.01
#elif defined(RADIO_X7ACCESS)
  #define TRIMS_GPIO_REG_LHR            GPIOD->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_15 // PD.15
  #define TRIMS_GPIO_REG_LHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_3  // PC.03
  #define TRIMS_GPIO_REG_RHR            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_2  // PC.02
  #define TRIMS_GPIO_REG_RHL            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_4  // PE.04
#elif defined(PCBX7)
  #define TRIMS_GPIO_REG_LHL            GPIOD->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_15 // PD.15
  #define TRIMS_GPIO_REG_LHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_4  // PE.04
#elif defined(PCBX9LITE)
  #define TRIMS_GPIO_REG_LHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_4  // PC.04
  #define TRIMS_GPIO_REG_LHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_5  // PC.05
  #define TRIMS_GPIO_REG_LVU            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_0  // PB.00
  #define TRIMS_GPIO_REG_LVD            GPIOB->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_1  // PB.01
  #define TRIMS_GPIO_REG_RVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_13 // PE.13
  #define TRIMS_GPIO_REG_RHR            GPIOD->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_8  // PD.08
  #define TRIMS_GPIO_REG_RVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_14 // PE.14
  #define TRIMS_GPIO_REG_RHL            GPIOD->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_9  // PD.09
#elif defined(RADIO_X9DP2019)
  #define TRIMS_GPIO_REG_LHL            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_3  // PE.03
  #define TRIMS_GPIO_REG_LHR            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_4  // PE.04
  #define TRIMS_GPIO_REG_LVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_3  // PC.03
  #define TRIMS_GPIO_REG_RVU            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_2  // PC.02
  #define TRIMS_GPIO_REG_RHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_13 // PC.13
  #define TRIMS_GPIO_REG_RHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_1  // PC.01
#else
  #define TRIMS_GPIO_REG_LHL            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LHL            GPIO_Pin_4  // PE.04
  #define TRIMS_GPIO_REG_LHR            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LHR            GPIO_Pin_3  // PE.03
  #define TRIMS_GPIO_REG_LVD            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVD            GPIO_Pin_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE->IDR
  #define TRIMS_GPIO_PIN_LVU            GPIO_Pin_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVD            GPIO_Pin_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHL            GPIO_Pin_1  // PC.01
  #define TRIMS_GPIO_REG_RVU            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RVU            GPIO_Pin_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOC->IDR
  #define TRIMS_GPIO_PIN_RHR            GPIO_Pin_13 // PC.13
#endif

// Switches
#if defined(PCBX9E)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_A_H         GPIO_Pin_10 // PD.10
  #define SWITCHES_GPIO_REG_A_L         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_A_L         GPIO_Pin_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_H         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_A_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_L         GPIO_Pin_0  // PE.00
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_A           GPIO_Pin_13  // PC.13
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A           GPIO_Pin_14  // PE.14
#elif defined(RADIO_TPRO)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_L         GPIO_Pin_13 // PE.13
  #define SWITCHES_GPIO_REG_A_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_H         GPIO_Pin_7  // PE.07
#elif defined(PCBX7)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_L         GPIO_Pin_7  // PE.07
  #define SWITCHES_GPIO_REG_A_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_H         GPIO_Pin_13 // PE.13
#else
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_A_H         GPIO_Pin_5  // PB.05
  #define SWITCHES_GPIO_REG_A_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_A_L         GPIO_Pin_0  // PE.00
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_11 // PG.11
  #define SWITCHES_GPIO_REG_B_L         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_10 // PG.10
#elif defined(PCBXLITE)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOA->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_6  // PA.06
  #define SWITCHES_GPIO_REG_B_H         GPIOA->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_5  // PA.05
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_7  // PE.07
  #define SWITCHES_GPIO_REG_B_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_13 // PE.13
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_B_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_2  // PE.02
#elif defined(PCBX7)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_15 // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_5  // PA.05
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_4  // PB.04
  #define SWITCHES_GPIO_REG_B_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_5  // PB.05
#else
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_H         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_B_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_B_L         GPIO_Pin_2  // PE.02
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_13 // PF.13
  #define SWITCHES_GPIO_REG_C_L         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_14 // PF.14
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_3  // PE.03
#elif  defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_0   // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_11  // PD.11
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_11  // PD.11
  #define SWITCHES_GPIO_REG_C_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_0  // PE.00
#elif defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_0  // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_11  // PD.11
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_1  // PE.01
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C           GPIO_Pin_14 // PE.14
#elif defined(PCBX7)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_11 // PD.11
  #if defined(PCBX7ACCESS)    
    #define SWITCHES_GPIO_REG_C_H       GPIOD->IDR
    #define SWITCHES_GPIO_PIN_C_H       GPIO_Pin_10 // PD.10
  #else
    #define SWITCHES_GPIO_REG_C_H       GPIOE->IDR
    #define SWITCHES_GPIO_PIN_C_H       GPIO_Pin_0  // PE.00
  #endif
#else
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_C_H         GPIO_Pin_15 // PE.15
  #define SWITCHES_GPIO_REG_C_L         GPIOA->IDR
  #define SWITCHES_GPIO_PIN_C_L         GPIO_Pin_5  // PA.05
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_H         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_D_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_L         GPIO_Pin_2  // PE.02
#elif defined(PCBX9DP)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_H         GPIO_Pin_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_L         GPIO_Pin_13 // PE.13
#elif defined(PCBXLITE)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_D_L         GPIO_Pin_4  // PB.04
  #define SWITCHES_GPIO_REG_D_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_D_H         GPIO_Pin_5  // PB.05
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_D           GPIO_Pin_14  // PD.14
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D           GPIO_Pin_8  // PE.08
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_D           GPIO_Pin_14 // PD.14
#elif defined(PCBX7)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_L         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_D_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_H         GPIO_Pin_2  // PE.02
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_D           GPIO_Pin_13 // PC.13
#else
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_D_H         GPIO_Pin_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_D_L         GPIO_Pin_1  // PB.01
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_H         GPIO_Pin_7  // PE.07
  #define SWITCHES_GPIO_REG_E_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_L         GPIO_Pin_13 // PE.13
#elif defined(PCBXLITES)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E           GPIO_Pin_5  // PE.05
#elif defined(PCBXLITE)
  // no SWE but we want to remain compatible with XLiteS
  #define STORAGE_SWITCH_E
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOA->IDR
  #define SWITCHES_GPIO_PIN_E           GPIO_Pin_5  // PA.05
#elif defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E           GPIO_Pin_7  // PE.07
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_H         GPIO_Pin_13 // PE.13
  #define SWITCHES_GPIO_REG_E_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_L         GPIO_Pin_7  // PE.07
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_H         GPIO_Pin_14 // PE.14
  #define SWITCHES_GPIO_REG_E_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_E_L         GPIO_Pin_7  // PE.07
#elif defined(RADIO_TPRO)
  // SW1
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_E           GPIO_Pin_7  // PG.07
#elif defined(PCBX7)
  // no SWE
#else
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_E_H         GPIO_Pin_3  // PB.03
  #define SWITCHES_GPIO_REG_E_L         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_E_L         GPIO_Pin_4  // PB.04
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_11 // PE.11
#elif defined(PCBXLITES)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_3  // PC.03
#elif defined(PCBXLITE)
  // no SWF but we want to remain compatible with XLiteS
  #define STORAGE_SWITCH_F
#elif defined(PCBX9LITES)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_3 // PC.03
#elif defined(PCBX9LITE)
  // no SWF
#elif defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_1 // PE.01
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F_L         GPIO_Pin_1  // PE.01
  #define SWITCHES_GPIO_REG_F_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F_H         GPIO_Pin_2 // PE.02
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F_L         GPIOD->IDR
  #define SWITCHES_GPIO_PIN_F_L         GPIO_Pin_14 // PD.14
  #define SWITCHES_GPIO_REG_F_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F_H         GPIO_Pin_1  // PE.01
#elif defined(RADIO_TPRO)
  // SW2
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_6  // PG.06
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_TPRO)
  // no SWF
  #define STORAGE_SWITCH_F
#elif defined(RADIO_T12)
  // no SWF
#elif defined(PCBX7)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_14 // PE.14
#else
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_F           GPIO_Pin_14 // PE.14
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_G_H         GPIO_Pin_3  // PF.03
  #define SWITCHES_GPIO_REG_G_L         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_G_L         GPIO_Pin_4  // PF.04
#elif defined(PCBX9LITES)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_G           GPIO_Pin_2 // PC.02
#elif defined(RADIO_T12) || defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_G           GPIO_Pin_14 // PE.14
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_G
#elif defined(RADIO_TPRO)
  // SW3
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_G           GPIO_Pin_5  // PG.05
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)  || defined(RADIO_T8)
  // no SWG
#else
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_G_H         GPIO_Pin_9  // PE.09
  #define SWITCHES_GPIO_REG_G_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_G_L         GPIO_Pin_8  // PE.08
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOF->IDR
  #define SWITCHES_GPIO_PIN_H           GPIO_Pin_1  // PF.01
#elif defined(PCBX9DP)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_H           GPIO_Pin_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  // no SWH
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_H
  // no SWH
#elif defined(RADIO_TX12)
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_H
#elif defined(RADIO_TPRO)
  // SW4
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_H           GPIO_Pin_4  // PG.04
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO)
  // no SWH
  #define STORAGE_SWITCH_H
#elif defined(PCBX7)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_H           GPIO_Pin_14 // PD.14
#else
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_H           GPIO_Pin_13 // PE.13
#endif

#if defined(RADIO_X9DP2019)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_10 // PD.10
#elif defined(PCBX9D) || defined(PCBX9DP)
  #define STORAGE_SWITCH_I
#endif

// X7 P400 P401 headers additionnal momentary switches
#if defined(PCBX7ACCESS)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_13  // PC.13
  #define STORAGE_SWITCH_J
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_I
  #define STORAGE_SWITCH_J
  // no SWI/J
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOE->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_14  // PE.14
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_J           GPIO_Pin_14  // PD.14
#elif defined(RADIO_TX12MK2)
  // no headers
#elif defined(RADIO_TLITE)
  // no SWI
  #define STORAGE_SWITCH_I
  // no SWJ
  #define STORAGE_SWITCH_J
#elif defined(RADIO_ZORRO)
  // No I/J
#elif defined(RADIO_TPRO)
  // SW5
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_3  // PG.03
  // SW6
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOG->IDR
  #define SWITCHES_GPIO_PIN_J           GPIO_Pin_2  // PG.02
#elif defined(PCBX7)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOC->IDR
  #define SWITCHES_GPIO_PIN_I           GPIO_Pin_13  // PC.13
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOD->IDR
  #define SWITCHES_GPIO_PIN_J           GPIO_Pin_10  // PD.10
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_I_H         GPIO_Pin_15 // PF.15
  #define SWITCHES_GPIO_REG_I_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_I_L         GPIO_Pin_14 // PE.14
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J_H         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_J_H         GPIO_Pin_7  // PG.07
  #define SWITCHES_GPIO_REG_J_L         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_J_L         GPIO_Pin_8  // PG.08
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K_H         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_K_H         GPIO_Pin_13 // PG.13
  #define SWITCHES_GPIO_REG_K_L         GPIOG->IDR
  #define SWITCHES_GPIO_PIN_K_L         GPIO_Pin_12 // PG.12
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_L_H         GPIO_Pin_9  // PE.09
  #define SWITCHES_GPIO_REG_L_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_L_L         GPIO_Pin_8  // PE.08
  #define STORAGE_SWITCH_M
  #define HARDWARE_SWITCH_M
  #define SWITCHES_GPIO_REG_M_H         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_M_H         GPIO_Pin_15 // PE.15
  #define SWITCHES_GPIO_REG_M_L         GPIOA->IDR
  #define SWITCHES_GPIO_PIN_M_L         GPIO_Pin_5  // PA.05
  #define STORAGE_SWITCH_N
  #define HARDWARE_SWITCH_N
  #define SWITCHES_GPIO_REG_N_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_N_H         GPIO_Pin_3  // PB.03
  #define SWITCHES_GPIO_REG_N_L         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_N_L         GPIO_Pin_4  // PB.04
  #define STORAGE_SWITCH_O
  #define HARDWARE_SWITCH_O
  #define SWITCHES_GPIO_REG_O_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_O_H         GPIO_Pin_7  // PF.07
  #define SWITCHES_GPIO_REG_O_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_O_L         GPIO_Pin_10 // PE.10
  #define STORAGE_SWITCH_P
  #define HARDWARE_SWITCH_P
  #define SWITCHES_GPIO_REG_P_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_P_H         GPIO_Pin_11 // PF.11
  #define SWITCHES_GPIO_REG_P_L         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_P_L         GPIO_Pin_12 // PF.12
  #define STORAGE_SWITCH_Q
  #define HARDWARE_SWITCH_Q
  #define SWITCHES_GPIO_REG_Q_H         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_Q_H         GPIO_Pin_5  // PF.05
  #define SWITCHES_GPIO_REG_Q_L         GPIOF->IDR
  #define SWITCHES_GPIO_PIN_Q_L         GPIO_Pin_6  // PF.06
  #define STORAGE_SWITCH_R
  #define HARDWARE_SWITCH_R
  #define SWITCHES_GPIO_REG_R_H         GPIOB->IDR
  #define SWITCHES_GPIO_PIN_R_H         GPIO_Pin_5  // PB.05
  #define SWITCHES_GPIO_REG_R_L         GPIOE->IDR
  #define SWITCHES_GPIO_PIN_R_L         GPIO_Pin_0  // PE.00
#endif

#if defined(PCBX9E)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG)
  #define KEYS_GPIOA_PINS               (SWITCHES_GPIO_PIN_M_L)
  #define KEYS_GPIOB_PINS               (SWITCHES_GPIO_PIN_N_H | SWITCHES_GPIO_PIN_N_L | SWITCHES_GPIO_PIN_R_H)
  #define KEYS_GPIOC_PINS               (TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR)
  #define KEYS_GPIOD_PINS               (KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_PAGE | SWITCHES_GPIO_PIN_A_H | SWITCHES_GPIO_PIN_A_L | ROTARY_ENCODER_GPIO_PIN_A | ROTARY_ENCODER_GPIO_PIN_B)
  #define KEYS_GPIOE_PINS               (TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_LVD | SWITCHES_GPIO_PIN_D_H | SWITCHES_GPIO_PIN_D_L| SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_E_L | SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_I_L | SWITCHES_GPIO_PIN_L_H | SWITCHES_GPIO_PIN_L_L | SWITCHES_GPIO_PIN_M_H | SWITCHES_GPIO_PIN_O_L | SWITCHES_GPIO_PIN_R_L)
  #define KEYS_GPIOF_PINS               (KEYS_GPIO_PIN_ENTER | SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_C_L | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L | SWITCHES_GPIO_PIN_H | SWITCHES_GPIO_PIN_I_H | SWITCHES_GPIO_PIN_O_H | SWITCHES_GPIO_PIN_P_H | SWITCHES_GPIO_PIN_P_L | SWITCHES_GPIO_PIN_Q_H | SWITCHES_GPIO_PIN_Q_L)
  #define KEYS_GPIOG_PINS               (TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LHR| SWITCHES_GPIO_PIN_B_H | SWITCHES_GPIO_PIN_B_L | SWITCHES_GPIO_PIN_J_H | SWITCHES_GPIO_PIN_J_L | SWITCHES_GPIO_PIN_K_H | SWITCHES_GPIO_PIN_K_L)
#elif defined(RADIO_X9DP2019)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_14)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(PCBX9DP)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (SWITCHES_GPIO_PIN_C_L)
  #define KEYS_GPIOB_PINS               (SWITCHES_GPIO_PIN_E_L | SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_A_H)
  #define KEYS_GPIOC_PINS               (TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR)
  #define KEYS_GPIOD_PINS               (KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_PAGE | SWITCHES_GPIO_PIN_H)
  #define KEYS_GPIOE_PINS               (KEYS_GPIO_PIN_PLUS | KEYS_GPIO_PIN_ENTER | KEYS_GPIO_PIN_MINUS | TRIMS_GPIO_PIN_LHR | TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_LVU | SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_A_L | SWITCHES_GPIO_PIN_B_H | SWITCHES_GPIO_PIN_B_L | SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_D_H | SWITCHES_GPIO_PIN_D_L | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L)
#elif defined(PCBXLITES)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5 | GPIO_Pin_6)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)
#elif defined(PCBXLITE)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5 | GPIO_Pin_6)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)
#elif defined(RADIO_TPRO)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOG_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
#if !defined(RADIO_TLITE)
  #define KEYS_GPIOA_PINS               GPIO_Pin_5
#endif
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(RADIO_ZORRO)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (KEYS_GPIO_PIN_ENTER | SWITCHES_GPIO_PIN_B_H)
  #define KEYS_GPIOB_PINS               (KEYS_GPIO_PIN_SYS)
  #define KEYS_GPIOC_PINS               (KEYS_GPIO_PIN_EXIT | TRIMS_GPIO_PIN_LHR | TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | SWITCHES_GPIO_PIN_A)
  #define KEYS_GPIOD_PINS               (KEYS_GPIO_PIN_PAGEUP | KEYS_GPIO_PIN_PAGEDN | KEYS_GPIO_PIN_TELE | TRIMS_GPIO_PIN_LHL |SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_H)
  #define KEYS_GPIOE_PINS               (KEYS_GPIO_PIN_MDL | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR | SWITCHES_GPIO_PIN_B_L | SWITCHES_GPIO_PIN_C_L | SWITCHES_GPIO_PIN_D | SWITCHES_GPIO_PIN_E | SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_G | ROTARY_ENCODER_GPIO_PIN_A | ROTARY_ENCODER_GPIO_PIN_B)
#elif defined(RADIO_TX12)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_13 | GPIO_Pin_5)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_3 | GPIO_Pin_4)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |GPIO_Pin_8| GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(RADIO_TX12MK2)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_10 | GPIO_Pin_5)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_4)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(RADIO_T8)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (KEYS_GPIO_PIN_ENTER | KEYS_GPIO_PIN_BIND)
  #define KEYS_GPIOB_PINS               (KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_SYS)
  #define KEYS_GPIOC_PINS               (TRIMS_GPIO_PIN_LHR | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_RVD)
  #define KEYS_GPIOD_PINS               (KEYS_GPIO_PIN_PAGEUP | KEYS_GPIO_PIN_PAGEDN | SWITCHES_GPIO_PIN_D | TRIMS_GPIO_PIN_LHL)
  #define KEYS_GPIOE_PINS               (SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_C_L | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR | TRIMS_GPIO_PIN_LVU | TRIMS_GPIO_PIN_LVD | SWITCHES_GPIO_PIN_B_L | KEYS_GPIO_PIN_PLUS  | KEYS_GPIO_PIN_MINUS | KEYS_GPIO_PIN_MDL | SWITCHES_GPIO_PIN_B_H | SWITCHES_GPIO_PIN_A)
#elif defined(RADIO_X7ACCESS)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               GPIO_Pin_5
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(RADIO_X7)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               GPIO_Pin_5
  #define KEYS_GPIOC_PINS               (GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15)
#elif defined(PCBX9LITES)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_8 | GPIO_Pin_9)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)
#elif defined(PCBX9LITE)
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (GPIO_Pin_5)
  #define KEYS_GPIOB_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5)
  #define KEYS_GPIOC_PINS               (GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_13)
  #define KEYS_GPIOD_PINS               (GPIO_Pin_8 | GPIO_Pin_9)
  #define KEYS_GPIOE_PINS               (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14)
#else
  #define KEYS_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE)
  #define KEYS_GPIOA_PINS               (SWITCHES_GPIO_PIN_C_L)
  #define KEYS_GPIOB_PINS               (SWITCHES_GPIO_PIN_E_L | SWITCHES_GPIO_PIN_E_H | SWITCHES_GPIO_PIN_A_H | SWITCHES_GPIO_PIN_D_L)
  #define KEYS_GPIOC_PINS               (TRIMS_GPIO_PIN_RVD | TRIMS_GPIO_PIN_RVU | TRIMS_GPIO_PIN_RHL | TRIMS_GPIO_PIN_RHR)
  #define KEYS_GPIOD_PINS               (KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_EXIT | KEYS_GPIO_PIN_PAGE)
  #define KEYS_GPIOE_PINS               (KEYS_GPIO_PIN_PLUS | KEYS_GPIO_PIN_ENTER | KEYS_GPIO_PIN_MINUS | TRIMS_GPIO_PIN_LHR | TRIMS_GPIO_PIN_LHL | TRIMS_GPIO_PIN_LVD | TRIMS_GPIO_PIN_LVU | SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_A_L | SWITCHES_GPIO_PIN_B_H | SWITCHES_GPIO_PIN_B_L | SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_D_H | SWITCHES_GPIO_PIN_G_H | SWITCHES_GPIO_PIN_G_L | SWITCHES_GPIO_PIN_H)
#endif

// ADC
#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_Channel                 DMA_Channel_0
#define ADC_DMA_Stream                  DMA2_Stream4
#define ADC_DMA_TC_Flag                 DMA_FLAG_TCIF4
#define ADC_SAMPTIME                    2   // sample time = 28 cycles

#if defined(PCBX9E)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #define HARDWARE_POT4
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            (RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC3)
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_8  // PF.08
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_0  // PB.00
  #define ADC_GPIO_PIN_POT3             GPIO_Pin_5  // PC.05
  #define ADC_GPIO_PIN_POT4             GPIO_Pin_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER1          GPIO_Pin_10 // PF.10
  #define ADC_GPIO_PIN_SLIDER2          GPIO_Pin_9  // PF.09
  #define ADC_GPIO_PIN_SLIDER3          GPIO_Pin_6  // PA.06
  #define ADC_GPIO_PIN_SLIDER4          GPIO_Pin_1  // PB.01
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_SLIDER3)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER4)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_POT4 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_CHANNEL_POT1              ADC_Channel_6  // ADC3_IN6
  #define ADC_CHANNEL_POT2              ADC_Channel_8  // ADC1_IN8
  #define ADC_CHANNEL_POT3              ADC_Channel_15 // ADC1_IN15
  #define ADC_CHANNEL_POT4              ADC_Channel_14 // ADC1_IN14
  #define ADC_CHANNEL_SLIDER1           ADC_Channel_8  // ADC3_IN8
  #define ADC_CHANNEL_SLIDER2           ADC_Channel_7  // ADC3_IN7
  #define ADC_CHANNEL_SLIDER3           ADC_Channel_6  // ADC1_IN6
  #define ADC_CHANNEL_SLIDER4           ADC_Channel_9  // ADC1_IN9
  #define ADC_CHANNEL_BATT              ADC_Channel_10 // ADC1_IN10
  #define ADC_EXT                       ADC3
  #define ADC_EXT_DMA                   DMA2
  #define ADC_EXT_DMA_Stream            DMA2_Stream0
  #define ADC_EXT_DMA_Channel           DMA_Channel_2
  #define ADC_EXT_TC_Flag               DMA_FLAG_TCIF0
  #define ADC_EXT_SAMPTIME              3    // sample time = 56 cycles
  #define ADC_VREF_PREC2                300
#elif defined(PCBX9DP)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_6  // PA.06
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_0  // PB.00
  #if PCBREV < 2019
    #define ADC_GPIO_PIN_POT3           GPIO_Pin_1  // PB.01
    #define ADC_GPIOB_PINS              (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3)
    #define ADC_CHANNEL_POT3            ADC_Channel_9
    #define ADC_VREF_PREC2              330
  #else
    #define ADC_GPIOB_PINS              (ADC_GPIO_PIN_POT2)
    #define ADC_CHANNEL_POT3            0
    #define ADC_VREF_PREC2              300
  #endif
  #define ADC_GPIO_PIN_SLIDER1          GPIO_Pin_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER2          GPIO_Pin_5  // PC.05
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT1)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_POT1              ADC_Channel_6
  #define ADC_CHANNEL_POT2              ADC_Channel_8
  #define ADC_CHANNEL_SLIDER1           ADC_Channel_14
  #define ADC_CHANNEL_SLIDER2           ADC_Channel_15
  #define ADC_CHANNEL_BATT              ADC_Channel_10
#elif defined(PCBXLITE)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            RCC_APB1Periph_TIM5
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_1  // PC.01
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_2  // PC.02
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define PWM_TIMER                     TIM5
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF_TIM5
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_IRQn                      TIM5_IRQn
  #define PWM_GPIOA_PINS                (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define ADC_GPIOA_PINS                (STICKS_PWM_ENABLED() ? 0 : (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3))
  #define ADC_GPIOC_PINS                (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2)
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_3  // ADC1_IN3
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_POT1              ADC_Channel_11 // ADC1_IN11
  #define ADC_CHANNEL_POT2              ADC_Channel_12 // ADC1_IN12
  #define ADC_CHANNEL_BATT              ADC_Channel_10 // ADC1_IN10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_T8)
  // No pots
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                0
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_BATT              ADC_Channel_10
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_TLITE)
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                0
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_BATT              ADC_Channel_10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_TPRO)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_0  // PB.00
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_6  // PA.06
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT1
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              ADC_Channel_6
  #define ADC_CHANNEL_POT2              ADC_Channel_8
  #define ADC_CHANNEL_BATT              ADC_Channel_10
  #define ADC_VREF_PREC2                330
#elif defined(PCBX7)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_0  // PB.00
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_6  // PA.06
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT1
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              ADC_Channel_8
  #define ADC_CHANNEL_POT2              ADC_Channel_6
  #define ADC_CHANNEL_BATT              ADC_Channel_10
  #define ADC_VREF_PREC2                330
#elif defined(PCBX9LITE)
  #define HARDWARE_POT1
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_1  // PC.01
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3)
  #define ADC_GPIOC_PINS                (GPIO_Pin_0 | GPIO_Pin_1)
  #define ADC_CHANNEL_POT1              ADC_Channel_11 // ADC1_IN11
  #define ADC_CHANNEL_BATT              ADC_Channel_10 // ADC1_IN10
  #define ADC_VREF_PREC2                300
#else
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            RCC_APB2Periph_ADC1
  #define ADC_GPIO_PIN_STICK_RV         GPIO_Pin_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         GPIO_Pin_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         GPIO_Pin_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         GPIO_Pin_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          ADC_Channel_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          ADC_Channel_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          ADC_Channel_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          ADC_Channel_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             GPIO_Pin_6  // PA.06
  #define ADC_GPIO_PIN_POT2             GPIO_Pin_0  // PB.00
  #define ADC_GPIO_PIN_SLIDER1          GPIO_Pin_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER2          GPIO_Pin_5  // PC.05
  #define ADC_GPIO_PIN_BATT             GPIO_Pin_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT1)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT2)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_POT1              ADC_Channel_6
  #define ADC_CHANNEL_POT2              ADC_Channel_8
  #define ADC_CHANNEL_POT3              ADC_Channel_9
  #define ADC_CHANNEL_SLIDER1           ADC_Channel_14
  #define ADC_CHANNEL_SLIDER2           ADC_Channel_15
  #define ADC_CHANNEL_BATT              ADC_Channel_10
  #define ADC_VREF_PREC2                330
#endif

// PWR and LED driver
#define PWR_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)

#if defined(PCBX9LITE)
  #define PWR_SWITCH_GPIO               GPIOA
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_7  // PA.07
  #define PWR_ON_GPIO                   GPIOA
  #define PWR_ON_GPIO_PIN               GPIO_Pin_6  // PA.06
#elif defined(PCBXLITE)
  #define PWR_SWITCH_GPIO               GPIOA
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_7  // PA.07
  #define PWR_ON_GPIO                   GPIOE
  #define PWR_ON_GPIO_PIN               GPIO_Pin_9  // PE.09
#else
  #define PWR_SWITCH_GPIO               GPIOD
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_1  // PD.01
  #define PWR_ON_GPIO                   GPIOD
  #define PWR_ON_GPIO_PIN               GPIO_Pin_0  // PD.00
#endif

#if defined(RADIO_X9DP2019)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_ResetBits
  #define GPIO_LED_GPIO_OFF             GPIO_SetBits
  #define LED_RED_GPIO                  GPIOA
  #define LED_RED_GPIO_PIN              GPIO_Pin_13 // PA.13
#elif defined(PCBXLITES)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_ResetBits
  #define GPIO_LED_GPIO_OFF             GPIO_SetBits
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_6  // PE.06
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_4  // PE.04
#elif defined(PCBXLITE)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_ResetBits
  #define GPIO_LED_GPIO_OFF             GPIO_SetBits
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_6  // PE.06
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_4  // PE.04
  #define LED_GREEN_GPIO                GPIOE
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_5  // PE.05
#elif defined(PCBX7ACCESS)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_BLUE_GPIO                 GPIOB
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_1  // PB.01
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_4  // PC.04
#elif defined(RADIO_T8)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_4  // PC.04
  #define LED_BLUE_GPIO                 GPIOC
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_5  // PC.05
  #define LED_GREEN_GPIO                GPIOB
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_1  // PB.01
#elif defined(RADIO_ZORRO)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOA
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_7  // PA.07
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_13 // PE.13
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_2  // PE.02
#elif defined(RADIO_TX12MK2)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOE
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_2  // PE.02
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_13 // PE.13
  #define LED_BLUE_GPIO                 GPIOA
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_7  // PA.07
#elif defined(PCBX7)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOC
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_4  // PC.04
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_5  // PC.05
  #define LED_BLUE_GPIO                 GPIOB
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_1  // PB.01
#elif defined(PCBX9LITES)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_5  // PE.05
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_6  // PE.06
#elif defined(PCBX9LITE)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_5  // PE.05
  #define LED_GREEN_GPIO                GPIOE
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_4  // PE.04
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_6  // PE.06
#endif

#if defined(FUNCTION_SWITCHES)
  #define FS_RCC_AHB1Periph             RCC_AHB1Periph_GPIOF
  #define GPIO_FSLED_GPIO_ON            GPIO_SetBits
  #define GPIO_FSLED_GPIO_OFF           GPIO_ResetBits
  #define FSLED_GPIO                    GPIOF
  #define FSLED_GPIO_PIN_1              GPIO_Pin_5
  #define FSLED_GPIO_PIN_2              GPIO_Pin_4
  #define FSLED_GPIO_PIN_3              GPIO_Pin_3
  #define FSLED_GPIO_PIN_4              GPIO_Pin_2
  #define FSLED_GPIO_PIN_5              GPIO_Pin_1
  #define FSLED_GPIO_PIN_6              GPIO_Pin_0
#endif

// Internal Module
#if defined(PCBXLITE)
#define EXTERNAL_ANTENNA
#endif
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #if defined(PCBXLITES) || defined(PCBX9LITE)
    #define INTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
    #define INTMODULE_PWR_GPIO          GPIOA
    #define INTMODULE_PWR_GPIO_PIN      GPIO_Pin_15 // PA.15
  #else
    #define INTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
    #define INTMODULE_PWR_GPIO          GPIOD
    #define INTMODULE_PWR_GPIO_PIN      GPIO_Pin_9  // PD.09
  #endif
  #define INTMODULE_GPIO                   GPIOB
  #define INTMODULE_TX_GPIO_PIN            GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO_PIN            GPIO_Pin_7  // PB.07
  #define INTMODULE_GPIO_PinSource_TX      GPIO_PinSource6
  #define INTMODULE_GPIO_PinSource_RX      GPIO_PinSource7
  #define INTMODULE_USART                  USART1
  #define INTMODULE_GPIO_AF                LL_GPIO_AF_7
  #define INTMODULE_USART_IRQHandler       USART1_IRQHandler
  #define INTMODULE_USART_IRQn             USART1_IRQn
  #define INTMODULE_DMA                    DMA2
  #define INTMODULE_DMA_STREAM             LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC            DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL            DMA_Channel_4
  #if defined(PCBXLITES)
    #define INTMODULE_BOOTCMD_GPIO         GPIOC
    #define INTMODULE_BOOTCMD_GPIO_PIN     GPIO_Pin_8  // PC.08
    #define INIT_INTMODULE_BOOTCMD_PIN()   GPIO_SetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #endif
#elif defined(RADIO_X9DP2019)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
  #define INTMODULE_PWR_GPIO            GPIOA
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_7  // PA.07
  #define INTMODULE_GPIO                GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_GPIO_PinSource_TX   GPIO_PinSource6
  #define INTMODULE_GPIO_PinSource_RX   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_GPIO_AF             LL_GPIO_AF_7
  #define INTMODULE_USART_IRQHandler    USART1_IRQHandler
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA                 DMA2
  #define INTMODULE_DMA_STREAM          LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
#elif defined(RADIO_X7ACCESS)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
  #define INTMODULE_PWR_GPIO            GPIOC
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_5  // PC.05
  #define INTMODULE_GPIO                GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_GPIO_PinSource_TX   GPIO_PinSource6
  #define INTMODULE_GPIO_PinSource_RX   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_GPIO_AF             LL_GPIO_AF_7
  #define INTMODULE_USART_IRQHandler    USART1_IRQHandler
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA                 DMA2
  #define INTMODULE_DMA_STREAM          LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
#elif defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_USART1
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
  #define INTMODULE_PWR_GPIO            GPIOC
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_4  // PC.04
  #define INTMODULE_GPIO                GPIOB
  #define INTMODULE_TX_GPIO_PIN         GPIO_Pin_6  // PB.06
  #define INTMODULE_RX_GPIO_PIN         GPIO_Pin_7  // PB.07
  #define INTMODULE_GPIO_PinSource_TX   GPIO_PinSource6
  #define INTMODULE_GPIO_PinSource_RX   GPIO_PinSource7
  #define INTMODULE_USART               USART1
  #define INTMODULE_GPIO_AF             LL_GPIO_AF_7
  #define INTMODULE_USART_IRQHandler    USART1_IRQHandler
  #define INTMODULE_USART_IRQn          USART1_IRQn
  #define INTMODULE_DMA                 DMA2
  #define INTMODULE_DMA_STREAM          LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_FLAG_TC         DMA_IT_TCIF7
  #define INTMODULE_DMA_CHANNEL         DMA_Channel_4
  #define INTMODULE_RX_DMA              DMA2
  #define INTMODULE_RX_DMA_STREAM       LL_DMA_STREAM_2
  #define INTMODULE_RX_DMA_CHANNEL      LL_DMA_CHANNEL_4
  #define INTMODULE_BOOTCMD_GPIO        GPIOB
  #define INTMODULE_BOOTCMD_GPIO_PIN    GPIO_Pin_1  // PB.01
  #define INIT_INTMODULE_BOOTCMD_PIN()  GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#elif defined(PCBX9E) || defined(PCBX9DP) || defined(RADIO_X7)
  #define INTMODULE_PULSES
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define INTMODULE_PWR_GPIO            GPIOC
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_6  // PC.06
  #define INTMODULE_TX_GPIO             GPIOA
  #define INTMODULE_TX_GPIO_PIN         LL_GPIO_PIN_10 // PA.10
  #define INTMODULE_TX_GPIO_AF          LL_GPIO_AF_1
  #define INTMODULE_TIMER               TIM1
  #define INTMODULE_TIMER_Channel       LL_TIM_CHANNEL_CH3
  #define INTMODULE_TIMER_IRQn          TIM1_UP_TIM10_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM1_UP_TIM10_IRQHandler
  #define INTMODULE_TIMER_DMA           DMA2
  #define INTMODULE_TIMER_DMA_CHANNEL   LL_DMA_CHANNEL_6
  #define INTMODULE_TIMER_DMA_STREAM    LL_DMA_STREAM_5
  #define INTMODULE_TIMER_DMA_STREAM_IRQn DMA2_Stream5_IRQn
  #define INTMODULE_TIMER_DMA_IRQHandler DMA2_Stream5_IRQHandler
  #define INTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
#elif (defined(RADIO_FAMILY_JUMPER_T12) && defined(HARDWARE_INTERNAL_MODULE)) || defined(RADIO_TX12) || defined(RADIO_T8) || defined(RADIO_TPRO)
  #define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA1)
  #define INTMODULE_RCC_APB1Periph        (RCC_APB1Periph_USART3 | RCC_APB1Periph_TIM2)
  #define INTMODULE_RCC_APB2Periph        0
  #define INTMODULE_PWR_GPIO              GPIOC
  #define INTMODULE_PWR_GPIO_PIN          GPIO_Pin_6  // PC.06
  #define INTMODULE_GPIO                  GPIOB
  #define INTMODULE_TX_GPIO_PIN           LL_GPIO_PIN_10  // PB.10
  #define INTMODULE_RX_GPIO_PIN           LL_GPIO_PIN_11  // PB.11
  #define INTMODULE_USART                 USART3
  #define INTMODULE_GPIO_AF               LL_GPIO_AF_7
  #define INTMODULE_USART_IRQn            USART3_IRQn
  #define INTMODULE_USART_IRQHandler      USART3_IRQHandler
  // DMA1_Stream3 is already used by SDIO
  // #define INTMODULE_DMA                   NULL
  // #define INTMODULE_DMA_CHANNEL           0
  // #define INTMODULE_DMA_STREAM            0
  // TODO: RX DMA seems to be buggy together with passthrough
  // #define INTMODULE_RX_DMA                DMA1
  // #define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_1
  // #define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_4
  #if defined(RADIO_TLITE)
    #define INTMODULE_BOOTCMD_GPIO          GPIOA
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_5  // PA.05
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #elif defined(RADIO_TPRO)
    #define INTMODULE_BOOTCMD_GPIO          GPIOF
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_11  // PF.11
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #endif
#else
  #define INTMODULE_PULSES
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define INTMODULE_RCC_APB1Periph      0
  #define INTMODULE_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define INTMODULE_PWR_GPIO            GPIOD
  #define INTMODULE_PWR_GPIO_PIN        GPIO_Pin_15 // PD.15
  #define INTMODULE_TX_GPIO             GPIOA
  #define INTMODULE_TX_GPIO_PIN         LL_GPIO_PIN_10 // PA.10
  #define INTMODULE_TX_GPIO_AF          LL_GPIO_AF_1
  #define INTMODULE_TIMER               TIM1
  #define INTMODULE_TIMER_Channel       LL_TIM_CHANNEL_CH3
  #define INTMODULE_TIMER_IRQn          TIM1_UP_TIM10_IRQn
  #define INTMODULE_TIMER_IRQHandler    TIM1_UP_TIM10_IRQHandler
  #define INTMODULE_TIMER_DMA           DMA2
  #define INTMODULE_TIMER_DMA_CHANNEL   LL_DMA_CHANNEL_6
  #define INTMODULE_TIMER_DMA_STREAM    LL_DMA_STREAM_5
  #define INTMODULE_TIMER_DMA_STREAM_IRQn DMA2_Stream5_IRQn
  #define INTMODULE_TIMER_DMA_IRQHandler DMA2_Stream5_IRQHandler
  #define INTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

// External Module
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019) || defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define EXTMODULE_RCC_APB2Periph      (RCC_APB2Periph_TIM8 | RCC_APB2Periph_USART6)
  #if defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
    #define EXTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
    #define EXTMODULE_PWR_GPIO          GPIOD
    #define EXTMODULE_PWR_GPIO_PIN      GPIO_Pin_8  // PD.08
    #if defined(RADIO_ZORRO)
      #define HARDWARE_EXTERNAL_MODULE_SIZE_SML
    #endif
  #elif defined(PCBX9LITE)
    #define HARDWARE_EXTERNAL_MODULE_SIZE_SML
    #define EXTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
    #define EXTMODULE_PWR_GPIO          GPIOA
    #define EXTMODULE_PWR_GPIO_PIN      GPIO_Pin_8  // PA.08
  #else
    #define HARDWARE_EXTERNAL_MODULE_SIZE_SML
    #define EXTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
    #define EXTMODULE_PWR_GPIO          GPIOD
    #define EXTMODULE_PWR_GPIO_PIN      GPIO_Pin_11 // PD.11
  #endif
  #define EXTERNAL_MODULE_PWR_ON()      GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define EXTERNAL_MODULE_PWR_OFF()     GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define IS_EXTERNAL_MODULE_ON()       (GPIO_ReadInputDataBit(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN) == Bit_SET)
  #define EXTMODULE_TX_GPIO             GPIOC
  #define EXTMODULE_USART_GPIO          EXTMODULE_TX_GPIO
  #define EXTMODULE_TX_GPIO_PIN         LL_GPIO_PIN_6  // PC.06
  #define EXTMODULE_RX_GPIO_PIN         LL_GPIO_PIN_7  // PC.07
  #define EXTMODULE_TIMER               TIM8
  #define EXTMODULE_TIMER_Channel       LL_TIM_CHANNEL_CH1
  #define EXTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_IRQn          TIM8_UP_TIM13_IRQn
  #define EXTMODULE_TIMER_IRQHandler    TIM8_UP_TIM13_IRQHandler
  #define EXTMODULE_TIMER_TX_GPIO_AF    GPIO_AF_TIM8 // TIM8_CH1
  #define EXTMODULE_TIMER_DMA_CHANNEL           LL_DMA_CHANNEL_7
  #define EXTMODULE_TIMER_DMA_STREAM            DMA2_Stream1
  #define EXTMODULE_TIMER_DMA                   DMA2
  #define EXTMODULE_TIMER_DMA_STREAM_LL         LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn       DMA2_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler        DMA2_Stream1_IRQHandler
  #define EXTMODULE_USART_GPIO_AF               GPIO_AF_USART6
  #define EXTMODULE_USART_GPIO_AF_LL            LL_GPIO_AF_8
  #define EXTMODULE_USART                       USART6
  #define EXTMODULE_USART_IRQn                  USART6_IRQn
  #define EXTMODULE_USART_IRQHandler            USART6_IRQHandler
  #define EXTMODULE_USART_TX_DMA                DMA2
  #define EXTMODULE_USART_TX_DMA_CHANNEL        DMA_Channel_5
  #define EXTMODULE_USART_TX_DMA_STREAM         DMA2_Stream6
  #define EXTMODULE_USART_TX_DMA_STREAM_LL      LL_DMA_STREAM_6
  #define EXTMODULE_USART_TX_DMA_FLAG_TC        DMA_FLAG_TCIF6
  #define EXTMODULE_USART_RX_DMA_CHANNEL        DMA_Channel_5
  #define EXTMODULE_USART_RX_DMA_STREAM         DMA2_Stream1
#else
  #define EXTMODULE_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB2Periph              RCC_APB2Periph_TIM8
  #define EXTMODULE_PWR_GPIO                    GPIOD
  #define EXTMODULE_PWR_GPIO_PIN                GPIO_Pin_8 // PD.08
  #define EXTERNAL_MODULE_PWR_ON()              GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define EXTERNAL_MODULE_PWR_OFF()             GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define IS_EXTERNAL_MODULE_ON()               (GPIO_ReadInputDataBit(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN) == Bit_SET)
  #define EXTMODULE_TX_GPIO                     GPIOA
  #define EXTMODULE_TX_GPIO_PIN                 GPIO_Pin_7
  #define EXTMODULE_TX_GPIO_PinSource           GPIO_PinSource7 // PA.07
  #define EXTMODULE_TIMER                       TIM8
  #define EXTMODULE_TIMER_Channel               LL_TIM_CHANNEL_CH1N
  #define EXTMODULE_TIMER_TX_GPIO_AF            GPIO_AF_TIM8 // TIM8_CH1N
  #define EXTMODULE_TIMER_IRQn                  TIM8_UP_TIM13_IRQn
  #define EXTMODULE_TIMER_IRQHandler            TIM8_UP_TIM13_IRQHandler
  #define EXTMODULE_TIMER_DMA_CHANNEL           LL_DMA_CHANNEL_7
  #define EXTMODULE_TIMER_DMA_STREAM            DMA2_Stream1
  #define EXTMODULE_TIMER_DMA                   DMA2
  #define EXTMODULE_TIMER_DMA_STREAM_LL         LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn       DMA2_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler        DMA2_Stream1_IRQHandler
  #define EXTMODULE_TIMER_FREQ                  (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

// Trainer Port
#define HARDWARE_TRAINER_EXTERNAL_MODULE
#if defined(PCBXLITES) || defined(PCBX9LITE)
  // on these 2 radios the trainer port already uses DMA1_Stream6, we won't use the DMA
  #define TRAINER_RCC_AHB1Periph        RCC_AHB1Periph_GPIOD
  #define TRAINER_RCC_APB1Periph        RCC_APB1Periph_TIM4
  #define TRAINER_GPIO                  GPIOD
  #define TRAINER_IN_GPIO_PIN           GPIO_Pin_13 // PD.13
  #define TRAINER_IN_GPIO_PinSource     GPIO_PinSource13
  #define TRAINER_OUT_GPIO_PIN          GPIO_Pin_12 // PD.12
  #define TRAINER_OUT_GPIO_PinSource    GPIO_PinSource12
#if defined(PCBX9LITE)
  #define TRAINER_DETECT_GPIO           GPIOD
  #define TRAINER_DETECT_GPIO_PIN       GPIO_Pin_11 // PD.11
  #define TRAINER_DETECT_GPIO_PIN_VALUE Bit_SET
#endif
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_GPIO_AF               GPIO_AF_TIM4 // TIM4_CH1 (Out) + TIM4_CH2 (In)
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
  #define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_OUT_CCMR1             TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;
  #define TRAINER_IN_CCMR1              TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC2S_0;
  #define TRAINER_OUT_COUNTER_REGISTER  TRAINER_TIMER->CCR1
  #define TRAINER_IN_COUNTER_REGISTER   TRAINER_TIMER->CCR2
  #define TRAINER_SETUP_REGISTER        TRAINER_TIMER->CCR3
  #define TRAINER_OUT_INTERRUPT_FLAG    TIM_SR_CC3IF
  #define TRAINER_OUT_INTERRUPT_ENABLE  TIM_DIER_CC3IE
  #define TRAINER_IN_INTERRUPT_ENABLE   TIM_DIER_CC2IE
  #define TRAINER_IN_INTERRUPT_FLAG     TIM_SR_CC2IF
  #define TRAINER_OUT_CCER              TIM_CCER_CC1E
  #define TRAINER_CCER_POLARYTY         TIM_CCER_CC1P
  #define TRAINER_IN_CCER               TIM_CCER_CC2E
#elif defined(PCBXLITE)
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_RCC_AHB1Periph        0
  #define TRAINER_RCC_APB1Periph        RCC_APB1Periph_TIM4
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
#else
  #define TRAINER_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA1)
  #define TRAINER_RCC_APB1Periph        RCC_APB1Periph_TIM3
  #define TRAINER_GPIO                  GPIOC
  #define TRAINER_IN_GPIO_PIN           GPIO_Pin_8  // PC.08
  #define TRAINER_IN_GPIO_PinSource     GPIO_PinSource8
  #define TRAINER_OUT_GPIO_PIN          GPIO_Pin_9  // PC.09
  #define TRAINER_OUT_GPIO_PinSource    GPIO_PinSource9
  #define TRAINER_DETECT_GPIO           GPIOA
  #define TRAINER_DETECT_GPIO_PIN       GPIO_Pin_8  // PA.08
#if defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS)
  #define TRAINER_DETECT_GPIO_PIN_VALUE Bit_SET
#else
  #define TRAINER_DETECT_GPIO_PIN_VALUE Bit_RESET
#endif
  #define TRAINER_TIMER                 TIM3
  #define TRAINER_TIMER_IRQn            TIM3_IRQn
  #define TRAINER_GPIO_AF               GPIO_AF_TIM3
  #define TRAINER_DMA                   DMA1
  #define TRAINER_DMA_CHANNEL           DMA_Channel_5
  #define TRAINER_DMA_STREAM            DMA1_Stream2
  #define TRAINER_DMA_IRQn              DMA1_Stream2_IRQn
  #define TRAINER_DMA_IRQHandler        DMA1_Stream2_IRQHandler
  #define TRAINER_DMA_FLAG_TC           DMA_IT_TCIF2
  #define TRAINER_TIMER_IRQn            TIM3_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM3_IRQHandler
  #define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_OUT_CCMR2             TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE;
  #define TRAINER_IN_CCMR2              TIM_CCMR2_IC3F_0 | TIM_CCMR2_IC3F_1 | TIM_CCMR2_CC3S_0;
  #define TRAINER_OUT_COUNTER_REGISTER  TRAINER_TIMER->CCR4
  #define TRAINER_IN_COUNTER_REGISTER   TRAINER_TIMER->CCR3
  #define TRAINER_SETUP_REGISTER        TRAINER_TIMER->CCR1
  #define TRAINER_OUT_INTERRUPT_FLAG    TIM_SR_CC1IF
  #define TRAINER_OUT_INTERRUPT_ENABLE  TIM_DIER_CC1IE
  #define TRAINER_IN_INTERRUPT_ENABLE   TIM_DIER_CC3IE
  #define TRAINER_IN_INTERRUPT_FLAG     TIM_SR_CC3IF
  #define TRAINER_OUT_CCER              TIM_CCER_CC4E
  #define TRAINER_IN_CCER               TIM_CCER_CC3E
  #define TRAINER_CCER_POLARYTY         TIM_CCER_CC4P
#endif

// Serial Port
#if (defined(PCBX7) && !defined(RADIO_ZORRO) && !defined(RADIO_TX12MK2)) || defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019)
  #define AUX_SERIAL_RCC_AHB1Periph         0
  #define AUX_SERIAL_RCC_APB1Periph         0
  #define AUX_SERIAL_RCC_APB2Periph         0
#else
  #define HARDWARE_TRAINER_AUX_SERIAL
  #define AUX_SERIAL_RCC_AHB1Periph         (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
  #define AUX_SERIAL_RCC_APB1Periph         RCC_APB1Periph_USART3
  #define AUX_SERIAL_RCC_APB2Periph         0
  #define AUX_SERIAL_GPIO                   GPIOB
  #define AUX_SERIAL_GPIO_PIN_TX            GPIO_Pin_10 // PB.10
  #define AUX_SERIAL_GPIO_PIN_RX            GPIO_Pin_11 // PB.11
  #define AUX_SERIAL_GPIO_PinSource_TX      GPIO_PinSource10
  #define AUX_SERIAL_GPIO_PinSource_RX      GPIO_PinSource11
  #define AUX_SERIAL_GPIO_AF                GPIO_AF_USART3
  #define AUX_SERIAL_GPIO_AF_LL             LL_GPIO_AF_7
  #define AUX_SERIAL_USART                  USART3
  #define AUX_SERIAL_USART_IRQHandler       USART3_IRQHandler
  #define AUX_SERIAL_USART_IRQn             USART3_IRQn
  #define AUX_SERIAL_DMA_RX                 DMA1
  #define AUX_SERIAL_DMA_Stream_RX          DMA1_Stream1
  #define AUX_SERIAL_DMA_Stream_RX_LL       LL_DMA_STREAM_1
  #define AUX_SERIAL_DMA_Channel_RX         DMA_Channel_4
#endif

// No aux2 on taranis
#define AUX2_SERIAL_RCC_AHB1Periph           0
#define AUX2_SERIAL_RCC_APB1Periph           0
#define AUX2_SERIAL_RCC_APB2Periph           0

// Telemetry
#define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
#define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
#define TELEMETRY_RCC_APB2Periph        RCC_APB2Periph_TIM11
#define TELEMETRY_DIR_GPIO              GPIOD
#define TELEMETRY_DIR_GPIO_PIN          GPIO_Pin_4  // PD.04
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS)
  #define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
  #define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
#else
  #define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
  #define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
#endif
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_5  // PD.05
#define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_6  // PD.06
#define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource5
#define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource6
#define TELEMETRY_GPIO_AF               GPIO_AF_USART2
#define TELEMETRY_USART                 USART2
#define TELEMETRY_DMA_Stream_TX         DMA1_Stream6
#define TELEMETRY_DMA_Channel_TX        DMA_Channel_4
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream6_IRQHandler
#define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF6
#define TELEMETRY_USART_IRQHandler      USART2_IRQHandler
#define TELEMETRY_USART_IRQn            USART2_IRQn
#define TELEMETRY_EXTI_PortSource       EXTI_PortSourceGPIOD
#define TELEMETRY_EXTI_PinSource        EXTI_PinSource6
#define TELEMETRY_EXTI_LINE             EXTI_Line6
#define TELEMETRY_EXTI_IRQn             EXTI9_5_IRQn
#define TELEMETRY_EXTI_TRIGGER          EXTI_Trigger_Rising

#if defined(RADIO_X7) || defined(RADIO_X7ACCESS) || defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO) || defined(RADIO_T8) || defined(RADIO_TPRO)
  #define TELEMETRY_EXTI_REUSE_INTERRUPT_ROTARY_ENCODER
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019)
  #define TELEMETRY_EXTI_IRQHandler       EXTI9_5_IRQHandler
#else
  #define TELEMETRY_EXTI_REUSE_INTERRUPT_INTMODULE_HEARTBEAT
#endif

#define TELEMETRY_TIMER                 TIM11
#define TELEMETRY_TIMER_IRQn            TIM1_TRG_COM_TIM11_IRQn
#define TELEMETRY_TIMER_IRQHandler      TIM1_TRG_COM_TIM11_IRQHandler

// PCBREV
#if defined(RADIO_X7)
  #define PCBREV_RCC_AHB1Periph         RCC_AHB1Periph_GPIOA
  #define PCBREV_GPIO                   GPIOA
  #define PCBREV_GPIO_PIN               GPIO_Pin_14  // PA.14
  #define PCBREV_GPIO_PULL_DOWN
  #define PCBREV_VALUE()                GPIO_ReadInputDataBit(PCBREV_GPIO, PCBREV_GPIO_PIN)
#else
  #define PCBREV_RCC_AHB1Periph         0
#endif


// USB Charger
#if defined(USB_CHARGER)
  #define USB_CHARGER_RCC_AHB1Periph      RCC_AHB1Periph_GPIOB
  #define USB_CHARGER_GPIO                GPIOB
  #define USB_CHARGER_GPIO_PIN            GPIO_Pin_5  // PB.05
#else
  #define USB_CHARGER_RCC_AHB1Periph      0
#endif

// S.Port update connector
#if defined(PCBXLITE)
  #define SPORT_MAX_BAUDRATE            250000 // not tested
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOD
  #define SPORT_UPDATE_PWR_GPIO         GPIOD
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_8  // PD.08
  #define GPIO_SPORT_UPDATE_PWR_GPIO_ON  GPIO_SetBits
  #define GPIO_SPORT_UPDATE_PWR_GPIO_OFF GPIO_ResetBits
#elif defined(PCBX7ACCESS)  
  #define SPORT_MAX_BAUDRATE            400000
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOB
  #define SPORT_UPDATE_PWR_GPIO         GPIOB
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_3  // PB.03
  #define GPIO_SPORT_UPDATE_PWR_GPIO_ON  GPIO_SetBits
  #define GPIO_SPORT_UPDATE_PWR_GPIO_OFF GPIO_ResetBits
#elif defined(RADIO_X7)
  #define SPORT_MAX_BAUDRATE            250000 // < 400000
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOB
  #define SPORT_UPDATE_PWR_GPIO         GPIOB
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_2  // PB.02
  #define GPIO_SPORT_UPDATE_PWR_GPIO_ON  GPIO_SetBits
  #define GPIO_SPORT_UPDATE_PWR_GPIO_OFF GPIO_ResetBits
#elif defined(PCBX9LITE)
  #define SPORT_MAX_BAUDRATE            250000 // not tested
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOE
  #define SPORT_UPDATE_PWR_GPIO         GPIOE
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_15 // PE.15
  #define GPIO_SPORT_UPDATE_PWR_GPIO_ON  GPIO_SetBits
  #define GPIO_SPORT_UPDATE_PWR_GPIO_OFF GPIO_ResetBits
#elif defined(RADIO_X9DP2019)
  #define SPORT_MAX_BAUDRATE            400000
  #define SPORT_UPDATE_RCC_AHB1Periph   RCC_AHB1Periph_GPIOA
  #define SPORT_UPDATE_PWR_GPIO         GPIOA
  #define SPORT_UPDATE_PWR_GPIO_PIN     GPIO_Pin_14 // PA.14
  #define GPIO_SPORT_UPDATE_PWR_GPIO_ON  GPIO_SetBits
  #define GPIO_SPORT_UPDATE_PWR_GPIO_OFF GPIO_ResetBits
#else
  #define SPORT_MAX_BAUDRATE            400000
  #define SPORT_UPDATE_RCC_AHB1Periph   0
#endif

// Heartbeat for iXJT / ISRM synchro
#define INTMODULE_HEARTBEAT_TRIGGER               EXTI_Trigger_Falling
#if !defined(HARDWARE_EXTERNAL_MODULE)
  // No heartbeat
#elif defined(PCBXLITE)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
  #define INTMODULE_HEARTBEAT_GPIO                GPIOD
  #define INTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_15
  #define INTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOD
  #define INTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource15
  #define INTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line15
  #define INTMODULE_HEARTBEAT_EXTI_IRQn           EXTI15_10_IRQn
  #define INTMODULE_HEARTBEAT_EXTI_IRQHandler     EXTI15_10_IRQHandler
#elif defined(PCBX9LITE)
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      0
#elif defined(RADIO_X7ACCESS)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define INTMODULE_HEARTBEAT_GPIO                GPIOA
  #define INTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_7  // PA.07
  #define INTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOA
  #define INTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource7
  #define INTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line7
  #define INTMODULE_HEARTBEAT_EXTI_IRQn           EXTI9_5_IRQn
#elif defined(RADIO_X9DP2019)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOB
  #define INTMODULE_HEARTBEAT_GPIO                GPIOB
  #define INTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_1  // PB.01
  #define INTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOB
  #define INTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource1
  #define INTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line1
  #define INTMODULE_HEARTBEAT_EXTI_IRQn           EXTI1_IRQn
  #define INTMODULE_HEARTBEAT_EXTI_IRQHandler     EXTI1_IRQHandler
#elif defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      0
#else
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOC
  #define INTMODULE_HEARTBEAT_GPIO                GPIOC
  #define INTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_7  // PC.07
  #define INTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOC
  #define INTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource7
  #define INTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line7
  #define INTMODULE_HEARTBEAT_EXTI_IRQn           EXTI9_5_IRQn
  #if defined(RADIO_X7) || defined(RADIO_TX12) || defined(RADIO_TPRO)
    #define INTMODULE_HEARTBEAT_REUSE_INTERRUPT_ROTARY_ENCODER
  #else
    #define INTMODULE_HEARTBEAT_EXTI_IRQHandler     EXTI9_5_IRQHandler
  #endif
#endif

#if defined(PCBX9LITE)
  #define EXTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
  #define EXTMODULE_HEARTBEAT_GPIO                GPIOD
  #define EXTMODULE_HEARTBEAT_GPIO_PIN            GPIO_Pin_15
  #define EXTMODULE_HEARTBEAT_EXTI_PortSource     EXTI_PortSourceGPIOD
  #define EXTMODULE_HEARTBEAT_EXTI_PinSource      GPIO_PinSource15
  #define EXTMODULE_HEARTBEAT_EXTI_LINE           EXTI_Line15
  #define EXTMODULE_HEARTBEAT_EXTI_IRQn           EXTI15_10_IRQn
  #define EXTMODULE_HEARTBEAT_EXTI_IRQHandler     EXTI15_10_IRQHandler
#endif

// Trainer / Trainee from the module bay
#if defined(PCBX9LITE) ||  defined(PCBXLITE) || defined(RADIO_X9DP2019) || defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define TRAINER_MODULE_CPPM
  #define TRAINER_MODULE_SBUS
  #define TRAINER_MODULE_RCC_AHB1Periph      0
  #define TRAINER_MODULE_RCC_APB1Periph      RCC_APB1Periph_TIM3
  #define TRAINER_MODULE_RCC_APB2Periph      0
  #define TRAINER_MODULE_CPPM_GPIO           EXTMODULE_USART_GPIO
  #define TRAINER_MODULE_CPPM_GPIO_PIN       GPIO_Pin_7
  #define TRAINER_MODULE_CPPM_GPIO_PinSource GPIO_PinSource7
  #define TRAINER_MODULE_CPPM_GPIO_AF        GPIO_AF_TIM3
  #define TRAINER_MODULE_CPPM_TIMER          TIM3
  #define TRAINER_MODULE_CPPM_COUNTER_REGISTER TRAINER_MODULE_CPPM_TIMER->CCR2
  #define TRAINER_MODULE_CPPM_INTERRUPT_ENABLE TIM_DIER_CC2IE
  #define TRAINER_MODULE_CPPM_INTERRUPT_FLAG   TIM_SR_CC2IF
  #define TRAINER_MODULE_CPPM_CCMR1            (TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC2S_0)
  #define TRAINER_MODULE_CPPM_CCER             TIM_CCER_CC2E
  #define TRAINER_MODULE_CPPM_TIMER_IRQn       TIM3_IRQn
#if defined(PCBX9LITE) ||  defined(PCBXLITE)
  #define TRAINER_MODULE_CPPM_TIMER_IRQHandler TIM3_IRQHandler
#endif
  #define TRAINER_MODULE_SBUS_GPIO             EXTMODULE_USART_GPIO
  #define TRAINER_MODULE_SBUS_GPIO_PIN         TRAINER_MODULE_CPPM_GPIO_PIN
  #define TRAINER_MODULE_SBUS_GPIO_AF          EXTMODULE_USART_GPIO_AF
  #define TRAINER_MODULE_SBUS_USART            EXTMODULE_USART
  #define TRAINER_MODULE_SBUS_GPIO_PinSource   TRAINER_MODULE_CPPM_GPIO_PinSource
  #define TRAINER_MODULE_SBUS_DMA_STREAM       EXTMODULE_USART_RX_DMA_STREAM
  #define TRAINER_MODULE_SBUS_DMA_CHANNEL      EXTMODULE_USART_RX_DMA_CHANNEL
#else
#if defined(INTMODULE_HEARTBEAT_GPIO) && defined(HARDWARE_EXTERNAL_MODULE)
  // Trainer CPPM input on heartbeat pin
  #define TRAINER_MODULE_CPPM
  #define TRAINER_MODULE_RCC_AHB1Periph           RCC_AHB1Periph_GPIOC
  #define TRAINER_MODULE_RCC_APB2Periph           RCC_APB2Periph_USART6
  #define TRAINER_MODULE_RCC_APB1Periph           RCC_APB1Periph_TIM3
  #define TRAINER_MODULE_CPPM_TIMER               TRAINER_TIMER
  #define TRAINER_MODULE_CPPM_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_CPPM_GPIO_PIN            INTMODULE_HEARTBEAT_GPIO_PIN
  #define TRAINER_MODULE_CPPM_GPIO_PinSource      INTMODULE_HEARTBEAT_EXTI_PinSource
  #define TRAINER_MODULE_CPPM_INTERRUPT_ENABLE    TIM_DIER_CC2IE
  #define TRAINER_MODULE_CPPM_INTERRUPT_FLAG      TIM_SR_CC2IF
  #define TRAINER_MODULE_CPPM_CCMR1               (TIM_CCMR1_IC2F_0 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_CC2S_0)
  #define TRAINER_MODULE_CPPM_CCER                TIM_CCER_CC2E
  #define TRAINER_MODULE_CPPM_COUNTER_REGISTER    TRAINER_TIMER->CCR2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn          TRAINER_TIMER_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF             GPIO_AF_TIM3
  // Trainer SBUS input on heartbeat pin
  #define TRAINER_MODULE_SBUS
  #define TRAINER_MODULE_SBUS_GPIO_AF             GPIO_AF_USART6
  #define TRAINER_MODULE_SBUS_USART               USART6
  #define TRAINER_MODULE_SBUS_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_SBUS_GPIO_PIN            INTMODULE_HEARTBEAT_GPIO_PIN
  #define TRAINER_MODULE_SBUS_GPIO_PinSource      INTMODULE_HEARTBEAT_EXTI_PinSource
  #define TRAINER_MODULE_SBUS_DMA_STREAM          DMA2_Stream1
  #define TRAINER_MODULE_SBUS_DMA_CHANNEL         DMA_Channel_5
#else
  // TODO: replace SBUS trainer with S.PORT pin
  #define TRAINER_MODULE_RCC_AHB1Periph           0
  #define TRAINER_MODULE_RCC_APB2Periph           0
  #define TRAINER_MODULE_RCC_APB1Periph           0
#endif
#endif

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               GPIO_Pin_9  // PA.09
#define USB_GPIO_PIN_DM                 GPIO_Pin_11 // PA.11
#define USB_GPIO_PIN_DP                 GPIO_Pin_12 // PA.12
#define USB_GPIO_PinSource_DM           GPIO_PinSource11
#define USB_GPIO_PinSource_DP           GPIO_PinSource12
#define USB_GPIO_AF                     GPIO_AF_OTG1_FS

// BackLight
#if defined(PCBX9E)
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOE
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM9
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM9
  #define BACKLIGHT_GPIO                GPIOE
  #define BACKLIGHT_GPIO_PIN_1          GPIO_Pin_6 // PE.06
  #define BACKLIGHT_GPIO_PIN_2          GPIO_Pin_5 // PE.05
  #define BACKLIGHT_GPIO_PinSource_1    GPIO_PinSource6
  #define BACKLIGHT_GPIO_PinSource_2    GPIO_PinSource5
  #define BACKLIGHT_GPIO_AF_1           GPIO_AF_TIM9
  #define BACKLIGHT_GPIO_AF_2           GPIO_AF_TIM9
#elif defined(PCBX9DP)
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
  #define BACKLIGHT_RCC_APB1Periph      RCC_APB1Periph_TIM4
  #define BACKLIGHT_RCC_APB2Periph      0
  #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define BACKLIGHT_TIMER               TIM4
  #define BACKLIGHT_GPIO                GPIOD
  #define BACKLIGHT_GPIO_PIN_1          GPIO_Pin_15 // PD.15
  #define BACKLIGHT_GPIO_PIN_2          GPIO_Pin_13 // PD.13
  #define BACKLIGHT_GPIO_PinSource_1    GPIO_PinSource15
  #define BACKLIGHT_GPIO_PinSource_2    GPIO_PinSource13
  #define BACKLIGHT_GPIO_AF_1           GPIO_AF_TIM4
  #define BACKLIGHT_GPIO_AF_2           GPIO_AF_TIM4
#elif defined(PCBXLITES) || defined(PCBX9LITE)
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIOA
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_10 // PA.10
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource10
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM1
  #define BACKLIGHT_CCMR2               TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 // Channel 3, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC3E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR3
#elif defined(PCBXLITE)
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM1
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIOA
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_8 // PA.08
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource8
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM1
  #define BACKLIGHT_CCMR1               TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 // Channel 1, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC1E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR1
#elif defined(RADIO_T8)
  #define BACKLIGHT_RCC_AHB1Periph      0
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      0
#elif defined(PCBX7)
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
  #define BACKLIGHT_RCC_APB1Periph      RCC_APB1Periph_TIM4
  #define BACKLIGHT_RCC_APB2Periph      0
  #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define BACKLIGHT_TIMER               TIM4
  #define BACKLIGHT_GPIO                GPIOD
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_13 // PD.13
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource13
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM4
  #define BACKLIGHT_CCMR1               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 // Channel2, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC2E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR2
#else
  #define BACKLIGHT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOB
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      RCC_APB2Periph_TIM10
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM10
  #define BACKLIGHT_GPIO                GPIOB
  #define BACKLIGHT_GPIO_PIN            GPIO_Pin_8  // PB.08
  #define BACKLIGHT_GPIO_PinSource      GPIO_PinSource8
  #define BACKLIGHT_GPIO_AF             GPIO_AF_TIM10
#endif
#define KEYS_BACKLIGHT_RCC_AHB1Periph        0

// LCD driver
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_T8) || defined(RADIO_TPRO)
  #define LCD_VERTICAL_INVERT
#endif
#if defined(PCBX9E)
  #define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOA
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_15 // PA.15
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_15 // PD.15
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOD
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_3  // PD.03
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_2  // PD.02
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3
#elif defined(PCBX9DP) || defined(PCBX7)
  #define LCD_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define LCD_RCC_APB1Periph            RCC_APB1Periph_SPI3
  #define LCD_SPI_GPIO                  GPIOC
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_12 // PC.12
  #define LCD_MOSI_GPIO_PinSource       GPIO_PinSource12
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_10 // PC.10
  #define LCD_CLK_GPIO_PinSource        GPIO_PinSource10
  #define LCD_A0_GPIO_PIN               GPIO_Pin_11 // PC.11
  #define LCD_NCS_GPIO                  GPIOA
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_15 // PA.15
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_12 // PD.12
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF_SPI3
#else
  #define LCD_RCC_AHB1Periph            RCC_AHB1Periph_GPIOD
  #define LCD_RCC_APB1Periph            0
  #define LCD_SPI_GPIO                  GPIOD
  #define LCD_MOSI_GPIO_PIN             GPIO_Pin_10 // PD.10
  #define LCD_CLK_GPIO_PIN              GPIO_Pin_11 // PD.11
  #define LCD_A0_GPIO_PIN               GPIO_Pin_13 // PD.13
  #define LCD_NCS_GPIO                  GPIOD
  #define LCD_NCS_GPIO_PIN              GPIO_Pin_14 // PD.14
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_12 // PD.12
#endif
#define LCD_RCC_APB2Periph              0

// I2C Bus 1: EEPROM and CAT5137 digital pot for volume control
#define I2C_B1                          I2C1
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(PCBX7ACCESS) || \
    defined(RADIO_ZORRO) || defined(RADIO_X9DP2019)
  #define I2C_B1_GPIO                   GPIOB
  #define I2C_B1_SDA_GPIO_PIN           LL_GPIO_PIN_9  // PB.09
  #define I2C_B1_SCL_GPIO_PIN           LL_GPIO_PIN_8  // PB.08
#else
  #define I2C_B1_GPIO                   GPIOB
  #define I2C_B1_SCL_GPIO_PIN           LL_GPIO_PIN_6  // PB.06
  #define I2C_B1_SDA_GPIO_PIN           LL_GPIO_PIN_7  // PB.07
#endif

// EEPROM
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define EEPROM_WP_GPIO                GPIOD
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_7  // PD.07
#elif defined(PCBX7ACCESS)
  #define EEPROM_WP_GPIO                GPIOB
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_5  // PB.05
#elif defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define EEPROM_WP_GPIO                GPIOD
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_10  // PD.10
#elif defined(RADIO_X9DP2019)
  #define EEPROM_WP_GPIO                GPIOF
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_0  // PF.00
#else
  #define EEPROM_WP_GPIO                GPIOB
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_9  // PB.09
#endif

// I2C Volume control
#if !defined(SOFTWARE_VOLUME)
  #define VOLUME_I2C_ADDRESS            0x2E
  #define VOLUME_I2C_BUS                I2C_Bus_1
#endif

#define I2C_B1_CLK_RATE                 400000

// EEPROM
#define EEPROM_I2C_ADDRESS              0x51
#define EEPROM_I2C_BUS                  I2C_Bus_1
#define EEPROM_PAGESIZE                 64
#define EEPROM_SIZE                     (32*1024)

// Second I2C Bus: IMU
#if defined(PCBXLITES)
  #define I2C_B2                        I2C3
  #define I2C_B2_SCL_GPIO               GPIOA
  #define I2C_B2_SCL_GPIO_PIN           LL_GPIO_PIN_8  // PA.08
  #define I2C_B2_SDA_GPIO               GPIOC
  #define I2C_B2_SDA_GPIO_PIN           LL_GPIO_PIN_9  // PC.09
  #define I2C_B2_GPIO_AF                LL_GPIO_AF_4
  #define I2C_B2_SPEED                  400000

  #define IMU_I2C_BUS                   I2C_Bus_2
  #define IMU_I2C_ADDRESS               0x6B
  #define IMU_I2C_CLK_RATE              I2C_B2_SPEED
#endif

// SD - SPI2
#define SD_RCC_AHB1Periph               (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
#define SD_RCC_APB1Periph               RCC_APB1Periph_SPI2
#define SD_GPIO_PRESENT_GPIO            GPIOD
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define SD_GPIO_PRESENT_GPIO_PIN      GPIO_Pin_10 // PD.10
#else
  #define SD_GPIO_PRESENT_GPIO_PIN      GPIO_Pin_9  // PD.09
#endif
#define SD_GPIO                         GPIOB
#define SD_GPIO_PIN_CS                  GPIO_Pin_12 // PB.12
#define SD_GPIO_PIN_SCK                 GPIO_Pin_13 // PB.13
#define SD_GPIO_PIN_MISO                GPIO_Pin_14 // PB.14
#define SD_GPIO_PIN_MOSI                GPIO_Pin_15 // PB.15
#define SD_GPIO_AF                      GPIO_AF_SPI2
#define SD_GPIO_PinSource_CS            GPIO_PinSource12
#define SD_GPIO_PinSource_SCK           GPIO_PinSource13
#define SD_GPIO_PinSource_MISO          GPIO_PinSource14
#define SD_GPIO_PinSource_MOSI          GPIO_PinSource15
#define SD_SPI                          SPI2
#define SD_SPI_BaudRatePrescaler        SPI_BaudRatePrescaler_4 // 10.5<20MHZ, make sure < 20MHZ

#if !defined(BOOT)
  #define SD_USE_DMA                    // Enable the DMA for SD
  #define SD_DMA_Stream_SPI_RX          DMA1_Stream3
  #define SD_DMA_Stream_SPI_TX          DMA1_Stream4
  #define SD_DMA_FLAG_SPI_TC_RX         DMA_FLAG_TCIF3
  #define SD_DMA_FLAG_SPI_TC_TX         DMA_FLAG_TCIF4
  #define SD_DMA_Channel_SPI            DMA_Channel_0
#endif

// Audio
#define AUDIO_RCC_APB1Periph            (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
#define AUDIO_OUTPUT_GPIO               GPIOA
#define AUDIO_OUTPUT_GPIO_PIN           GPIO_Pin_4  // PA.04
#define AUDIO_DMA_Stream                DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
#define AUDIO_TIM_IRQn                  TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler            TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                     TIM6
#define AUDIO_DMA                       DMA1

#if defined(PCBXLITES)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define JACK_DETECT_GPIO              GPIOC
  #define JACK_DETECT_GPIO_PIN          GPIO_Pin_13 // PC.13
  #define AUDIO_SPEAKER_ENABLE_GPIO     GPIOD
  #define AUDIO_SPEAKER_ENABLE_GPIO_PIN GPIO_Pin_14 // PD.14
  #define HEADPHONE_TRAINER_SWITCH_GPIO      GPIOD
  #define HEADPHONE_TRAINER_SWITCH_GPIO_PIN  GPIO_Pin_9 // PD.09
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO) || defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_TPRO)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_MUTE_GPIO               GPIOE
  #define AUDIO_MUTE_GPIO_PIN           GPIO_Pin_12  // PE.12
  #define AUDIO_MUTE_DELAY              500  // ms
#if defined(RADIO_TLITE) || defined(RADIO_TPRO)
  #define AUDIO_UNMUTE_DELAY            250  // ms
#else
  #define AUDIO_UNMUTE_DELAY            150  // ms
#endif
#else
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
#endif

// Haptic
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_ZORRO) || defined(RADIO_TX12MK2)
  #define HAPTIC_PWM
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOB
  #define HAPTIC_RCC_APB1Periph         RCC_APB1Periph_TIM2
  #define HAPTIC_RCC_APB2Periph         0
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource3
  #define HAPTIC_GPIO                   GPIOB
  #define HAPTIC_GPIO_PIN               GPIO_Pin_3  // PB.03
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM2
  #define HAPTIC_TIMER                  TIM2
  #define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR2
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_CCER                   TIM_CCER_CC2E
  #define BACKLIGHT_BDTR                TIM_BDTR_MOE
#elif defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS)
  #define HAPTIC_PWM
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOA
  #define HAPTIC_RCC_APB2Periph         RCC_APB2Periph_TIM1
  #define HAPTIC_RCC_APB1Periph         0
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource10
  #define HAPTIC_GPIO                   GPIOA
  #define HAPTIC_GPIO_PIN               GPIO_Pin_10 // PA.10
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM1
  #define HAPTIC_TIMER                  TIM1 // Timer1 Channel3
  #define HAPTIC_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR3
  #define HAPTIC_CCMR2                  TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2
  #define HAPTIC_CCER                   TIM_CCER_CC3E
#elif defined(PCBX9E) || defined(PCBX9DP) || defined(PCBX7)
  #define HAPTIC_PWM
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOB
  #define HAPTIC_RCC_APB2Periph         RCC_APB2Periph_TIM10
  #define HAPTIC_RCC_APB1Periph         0
  #define HAPTIC_GPIO_PinSource         GPIO_PinSource8
  #define HAPTIC_GPIO                   GPIOB
  #define HAPTIC_GPIO_PIN               GPIO_Pin_8  // PB.08
  #define HAPTIC_GPIO_AF                GPIO_AF_TIM10
  #define HAPTIC_TIMER                  TIM10
  #define HAPTIC_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR1
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2
  #define HAPTIC_CCER                   TIM_CCER_CC1E
#else
  #define HAPTIC_RCC_AHB1Periph         RCC_AHB1Periph_GPIOC
  #define HAPTIC_RCC_APB2Periph         0
  #define HAPTIC_RCC_APB1Periph         0
  #define HAPTIC_GPIO                   GPIOC
  #define HAPTIC_GPIO_PIN               GPIO_Pin_12
#endif

// Top LCD on X9E
#if defined(PCBX9E)
  #define TOPLCD_RCC_AHB1Periph         RCC_AHB1Periph_GPIOG
  #define TOPLCD_GPIO                   GPIOG
  #define TOPLCD_GPIO_PIN_DATA          GPIO_Pin_5
  #define TOPLCD_GPIO_PIN_WR            GPIO_Pin_4
  #define TOPLCD_GPIO_PIN_CS1           GPIO_Pin_3
  #define TOPLCD_GPIO_PIN_CS2           GPIO_Pin_15
  #define TOPLCD_GPIO_PIN_BL            GPIO_Pin_2
#endif

// Bluetooth
#if defined(PCBX9E)
  #define STORAGE_BLUETOOTH
  #define BT_USART                     USART6
  #define BT_GPIO_AF                   GPIO_AF_USART6
  #define BT_USART_IRQn                USART6_IRQn
  #define BT_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG)
  #define BT_RCC_APB1Periph            0
  #define BT_RCC_APB2Periph            RCC_APB2Periph_USART6
  #define BT_USART_GPIO                GPIOG
  #define BT_TX_GPIO_PIN               GPIO_Pin_14 // PG.14
  #define BT_RX_GPIO_PIN               GPIO_Pin_9  // PG.09
  #define BT_EN_GPIO                   GPIOD
  #define BT_EN_GPIO_PIN               GPIO_Pin_11 // PD.11
  #define BT_BRTS_GPIO                 GPIOE
  #define BT_BRTS_GPIO_PIN             GPIO_Pin_12 // PE.12
  #define BT_BCTS_GPIO                 GPIOG
  #define BT_BCTS_GPIO_PIN             GPIO_Pin_6  // PG.06
  #define BT_TX_GPIO_PinSource         GPIO_PinSource14
  #define BT_RX_GPIO_PinSource         GPIO_PinSource9
  #define BT_USART_IRQHandler          USART6_IRQHandler
#elif defined(RADIO_X7) || defined(RADIO_X7ACCESS) || defined(PCBXLITE) || defined(PCBX9LITES) || defined(RADIO_X9DP2019)
  #define STORAGE_BLUETOOTH
  #define BT_RCC_APB1Periph             RCC_APB1Periph_USART3
  #define BT_RCC_APB2Periph             0
  #if defined(PCBX9DP)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOB
    #define BT_EN_GPIO_PIN              GPIO_Pin_2  // PB.02
  #elif defined(PCBXLITE)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOE
    #define BT_EN_GPIO_PIN              GPIO_Pin_15 // PE.15
  #elif defined(PCBX9LITES)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOD
    #define BT_EN_GPIO_PIN              GPIO_Pin_14 // PD.14
  #else
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOE
    #define BT_EN_GPIO_PIN              GPIO_Pin_12 // PE.12
  #endif
  #define BT_USART_GPIO                 GPIOB
  #define BT_TX_GPIO_PIN                GPIO_Pin_10 // PB.10
  #define BT_RX_GPIO_PIN                GPIO_Pin_11 // PB.11
  #define BT_TX_GPIO_PinSource          GPIO_PinSource10
  #define BT_RX_GPIO_PinSource          GPIO_PinSource11
  #define BT_GPIO_AF                    GPIO_AF_USART3
  #define BT_USART                      USART3
  #define BT_USART_IRQHandler           USART3_IRQHandler
  #define BT_USART_IRQn                 USART3_IRQn
  // #define BT_DMA_Stream_RX              DMA1_Stream1
  // #define BT_DMA_Channel_RX             DMA_Channel_4
#elif defined(PCBX9D) || defined(PCBX9DP) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_T8) || defined(RADIO_ZORRO)
  #define STORAGE_BLUETOOTH
  #define BT_RCC_AHB1Periph             0
  #define BT_RCC_APB1Periph             0
  #define BT_RCC_APB2Periph             0
#else
  #define BT_RCC_AHB1Periph             0
  #define BT_RCC_APB1Periph             0
  #define BT_RCC_APB2Periph             0
#endif

// Xms Interrupt
#define INTERRUPT_xMS_RCC_APB1Periph    RCC_APB1Periph_TIM14
#define INTERRUPT_xMS_TIMER             TIM14
#define INTERRUPT_xMS_IRQn              TIM8_TRG_COM_TIM14_IRQn
#define INTERRUPT_xMS_IRQHandler        TIM8_TRG_COM_TIM14_IRQHandler

// 2MHz Timer
#define TIMER_2MHz_RCC_APB1Periph       RCC_APB1Periph_TIM7
#define TIMER_2MHz_TIMER                TIM7

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER_RCC_APB1Periph RCC_APB1Periph_TIM12
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_BRK_TIM12_IRQHandler

#endif // _HAL_H_
