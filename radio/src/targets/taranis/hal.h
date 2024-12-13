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

#if defined(STM32F413xx)
  #define CPU_FREQ            100000000
  #define PERI1_FREQUENCY     50000000
  #define PERI2_FREQUENCY     100000000
  #define TIMER_MULT_APB1     1
  #define TIMER_MULT_APB2     1
#elif defined(STM32F4)
  #define CPU_FREQ            168000000
  #define PERI1_FREQUENCY     42000000
  #define PERI2_FREQUENCY     84000000
  #define TIMER_MULT_APB1     2
  #define TIMER_MULT_APB2     2
#else
  #define CPU_FREQ            120000000
  #define PERI1_FREQUENCY     30000000
  #define PERI2_FREQUENCY     60000000
  #define TIMER_MULT_APB1     2
  #define TIMER_MULT_APB2     2
#endif

#define TELEMETRY_EXTI_PRIO             0 // required for soft serial

// Keys
#if defined(PCBX9E)
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_ENTER           GPIOF
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_0  // PF.00
#elif defined(PCBXLITE)
  #define KEYS_GPIO_REG_SHIFT           GPIOE
  #define KEYS_GPIO_PIN_SHIFT           LL_GPIO_PIN_8  // PE.08
  #define KEYS_GPIO_REG_EXIT            GPIOE
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_7  // PE.07
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_UP              GPIOE
  #define KEYS_GPIO_PIN_UP              LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_DOWN            GPIOE
  #define KEYS_GPIO_PIN_DOWN            LL_GPIO_PIN_14 // PE.14
  #define KEYS_GPIO_REG_LEFT            GPIOE
  #define KEYS_GPIO_PIN_LEFT            LL_GPIO_PIN_12 // PE.12
  #define KEYS_GPIO_REG_RIGHT           GPIOE
  #define KEYS_GPIO_PIN_RIGHT           LL_GPIO_PIN_13 // PE.13
#elif defined(RADIO_TPROS)
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_13 // PA.13
#elif defined(RADIO_FAMILY_JUMPER_T12) && !defined(RADIO_TPRO) && !defined(RADIO_TPROV2)
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_UP              GPIOE
  #define KEYS_GPIO_PIN_UP              LL_GPIO_PIN_9 // PE.09
  #define KEYS_GPIO_REG_DOWN            GPIOE
  #define KEYS_GPIO_PIN_DOWN            LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_LEFT            GPIOD
  #define KEYS_GPIO_PIN_LEFT            LL_GPIO_PIN_7 // PD.07
  #define KEYS_GPIO_REG_RIGHT           GPIOD
  #define KEYS_GPIO_PIN_RIGHT           LL_GPIO_PIN_3 // PD.03
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_GX12)
#if defined(RADIO_MT12)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
#else
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_7  // PD.07
#endif
#if defined(RADIO_TX12)
  #define KEYS_GPIO_REG_EXIT            GPIOB
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_3  // PB.03
#else
  #define KEYS_GPIO_REG_EXIT            GPIOC
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5  // PC.05
#endif
#if defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_GX12)
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PA.10
#else
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_13 // PA.13
#endif
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
  #define KEYS_GPIO_REG_MDL             GPIOE
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_TELE            GPIOD
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_2  // PD.02
#elif defined(RADIO_T8)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOB
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_3  // PB.03
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_13 // PA.13
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
  #define KEYS_GPIO_REG_MDL             GPIOE
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_PLUS            GPIOE
  #define KEYS_GPIO_PIN_PLUS            LL_GPIO_PIN_9  // PE.09
  #define KEYS_GPIO_REG_MINUS           GPIOE
  #define KEYS_GPIO_PIN_MINUS           LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_BIND            GPIOA
  #define KEYS_GPIO_PIN_BIND            LL_GPIO_PIN_10 // PA.10
#elif defined(RADIO_COMMANDO8)
  #define KEYS_GPIO_REG_PAGEUP          GPIOE
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_11  // PE.11
  #define KEYS_GPIO_REG_PAGEDN          GPIOE
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_13  // PE.13
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_SYS             GPIOD
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_1  // PD.01
  #define KEYS_GPIO_REG_MDL             GPIOD
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_0 // PD.00
  #define KEYS_GPIO_REG_PLUS            GPIOE
  #define KEYS_GPIO_PIN_PLUS            LL_GPIO_PIN_12  // PE.12
  #define KEYS_GPIO_REG_MINUS           GPIOE
  #define KEYS_GPIO_PIN_MINUS           LL_GPIO_PIN_14 // PE.14
  #define KEYS_GPIO_REG_BIND            GPIOD
  #define KEYS_GPIO_PIN_BIND            LL_GPIO_PIN_9 // PD.09
#elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_11  // PD.11
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_10  // PD.10
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_12  // PD.12
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_7 // PE.7
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_13 // PA.13
#elif defined(RADIO_GX12)
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PA.10
  #define KEYS_GPIO_REG_EXIT            GPIOC
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5  // PC.05
  #define KEYS_GPIO_REG_MDL             GPIOE
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_7  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.07
  #define KEYS_GPIO_REG_TELE            GPIOD
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_2  // PD.02
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOE
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_1  // PE.01
  #define KEYS_GPIO_REG_ENTER           GPIOD
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_11 // PD.11
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
  #define KEYS_GPIO_REG_MDL             GPIOE
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_TELE            GPIOD
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_2  // PD.02
#elif defined(PCBX7)
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PE.10
#elif defined(PCBX9LITE)
  #define KEYS_GPIO_REG_PAGEDN          GPIOE
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8  // PE.08
  #define KEYS_GPIO_REG_MENU            GPIOE
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PE.07
  #define KEYS_GPIO_REG_EXIT            GPIOE
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_9  // PE.09
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_11 // PE.11
#elif defined(RADIO_X9DP2019)
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_12 // PE.12
#else
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PLUS            GPIOE
  #define KEYS_GPIO_PIN_PLUS            LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_MINUS           GPIOE
  #define KEYS_GPIO_PIN_MINUS           LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_12 // PE.12
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
#endif

// Rotary Encoder
#if defined(PCBX9E)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOD
  #define ROTARY_ENCODER_GPIO_PIN_A     LL_GPIO_PIN_12 // PD.12
  #define ROTARY_ENCODER_GPIO_PIN_B     LL_GPIO_PIN_13 // PD.13
  #define ROTARY_ENCODER_POSITION()     (ROTARY_ENCODER_GPIO->IDR >> 12) & 0x03
  #define ROTARY_ENCODER_EXTI_LINE1     LL_EXTI_LINE_12
  #define ROTARY_ENCODER_EXTI_LINE2     LL_EXTI_LINE_13
  #define ROTARY_ENCODER_EXTI_PORT      LL_SYSCFG_EXTI_PORTD
  #define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE12
  #define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE13
  // ROTARY_ENCODER_EXTI IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
#elif defined(RADIO_X9DP2019)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     LL_GPIO_PIN_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     LL_GPIO_PIN_11 // PE.11
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x02) + ((ROTARY_ENCODER_GPIO->IDR >> 11) & 0x01))
  #define ROTARY_ENCODER_EXTI_LINE1     LL_EXTI_LINE_10
  #define ROTARY_ENCODER_EXTI_LINE2     LL_EXTI_LINE_11
  #define ROTARY_ENCODER_EXTI_PORT      LL_SYSCFG_EXTI_PORTE
  #define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE10
  #define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE11
  // ROTARY_ENCODER_EXTI IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
#elif defined(RADIO_X7) || defined(RADIO_X7ACCESS) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     LL_GPIO_PIN_9  // PE.09
  #define ROTARY_ENCODER_GPIO_PIN_B     LL_GPIO_PIN_11 // PE.11
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 10) & 0x02) + ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x01))
  #define ROTARY_ENCODER_EXTI_LINE1     LL_EXTI_LINE_9
  #define ROTARY_ENCODER_EXTI_LINE2     LL_EXTI_LINE_11
  // ROTARY_ENCODER_EXTI_LINE1 IRQ
  #if !defined(USE_EXTI9_5_IRQ)
    #define USE_EXTI9_5_IRQ
    #define EXTI9_5_IRQ_Priority 5
  #endif
  // ROTARY_ENCODER_EXTI_LINE2 IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
  #define ROTARY_ENCODER_EXTI_PORT      LL_SYSCFG_EXTI_PORTE
  #define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE9
  #define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE11
#elif defined(PCBX9LITE)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     LL_GPIO_PIN_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     LL_GPIO_PIN_12 // PE.12
  #define ROTARY_ENCODER_POSITION()     (((ROTARY_ENCODER_GPIO->IDR >> 12) & 0x01) + ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x02))
  #define ROTARY_ENCODER_EXTI_LINE1     LL_EXTI_LINE_10
  #define ROTARY_ENCODER_EXTI_LINE2     LL_EXTI_LINE_12
  #define ROTARY_ENCODER_EXTI_PORT      LL_SYSCFG_EXTI_PORTE
  #define ROTARY_ENCODER_EXTI_SYS_LINE1  LL_SYSCFG_EXTI_LINE10
  #define ROTARY_ENCODER_EXTI_SYS_LINE2  LL_SYSCFG_EXTI_LINE12
  // ROTARY_ENCODER_EXTI IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_T12MAX)  || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12) || defined(RADIO_GX12)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO              GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A        LL_GPIO_PIN_9 // PE.9
  #define ROTARY_ENCODER_GPIO_PIN_B        LL_GPIO_PIN_10 // PE.10
  #define ROTARY_ENCODER_POSITION()        ((ROTARY_ENCODER_GPIO->IDR >> 9) & 0x03)
  #define ROTARY_ENCODER_EXTI_LINE1        LL_EXTI_LINE_9
  #define ROTARY_ENCODER_EXTI_LINE2        LL_EXTI_LINE_10
  #define ROTARY_ENCODER_EXTI_PORT         LL_SYSCFG_EXTI_PORTE
  #define ROTARY_ENCODER_EXTI_SYS_LINE1    LL_SYSCFG_EXTI_LINE9
  #define ROTARY_ENCODER_EXTI_SYS_LINE2    LL_SYSCFG_EXTI_LINE10
  // ROTARY_ENCODER_EXTI_LINE1 IRQ
  #if !defined(USE_EXTI9_5_IRQ)
    #define USE_EXTI9_5_IRQ
    #define EXTI9_5_IRQ_Priority 5
  #endif
  // ROTARY_ENCODER_EXTI_LINE2 IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
  #if defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12)
    #define ROTARY_ENCODER_INVERTED
  #endif
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define ROTARY_ENCODER_TIMER            TIM5
  #define ROTARY_ENCODER_TIMER_IRQn       TIM5_IRQn
  #define ROTARY_ENCODER_TIMER_IRQHandler TIM5_IRQHandler
#endif

// Trims
#if defined(PCBX9E)
  #define TRIMS_GPIO_REG_LHL            GPIOG
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_1  // PG.01
  #define TRIMS_GPIO_REG_LHR            GPIOG
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_0  // PG.00
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_4  // PE.04
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOC
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOC
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_13 // PC.13
#elif defined(PCBXLITE)
  #define TRIMS_GPIO_REG_LHL            GPIOC
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_4  // PC.04
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_5  // PC.05
  #define TRIMS_GPIO_REG_LVU            GPIOB
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_0  // PB.00
  #define TRIMS_GPIO_REG_LVD            GPIOB
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_1  // PB.01
  #define TRIMS_VIRTUAL_RV
  #define TRIMS_VIRTUAL_RH
#elif defined(RADIO_X7ACCESS)
  #define TRIMS_GPIO_REG_LHR            GPIOD
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_LHL            GPIOC
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_4  // PE.04
#elif defined(RADIO_FAMILY_T20)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_13 // PD.13
  #define TRIMS_GPIO_REG_LHR            GPIOG
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_2  // PG.02
  #define TRIMS_GPIO_REG_LVD            GPIOG
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_3  // PG.03
  #define TRIMS_GPIO_REG_LVU            GPIOD
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_14 // PD.14
  #define TRIMS_GPIO_REG_RVD            GPIOF
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_2  // PF.02
  #define TRIMS_GPIO_REG_RVU            GPIOE
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RHR            GPIOF
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_1  // PF.01
  #define TRIMS_GPIO_REG_LSD            GPIOG
  #define TRIMS_GPIO_PIN_LSD            LL_GPIO_PIN_9  // PG.09
  #define TRIMS_GPIO_REG_LSU            GPIOG
  #define TRIMS_GPIO_PIN_LSU            LL_GPIO_PIN_10 // PG.10
  #define TRIMS_GPIO_REG_RSD            GPIOD
  #define TRIMS_GPIO_PIN_RSD            LL_GPIO_PIN_7  // PD.07
  #define TRIMS_GPIO_REG_RSU            GPIOG
  #define TRIMS_GPIO_PIN_RSU            LL_GPIO_PIN_12 // PG.12
#if defined(RADIO_T20V2)
  #define TRIMS_GPIO_REG_T7L            GPIOC
  #define TRIMS_GPIO_PIN_T7L            LL_GPIO_PIN_13 // PC.13
#else
  #define TRIMS_GPIO_REG_T7L            GPIOB
  #define TRIMS_GPIO_PIN_T7L            LL_GPIO_PIN_3  // PB.03
#endif
  #define TRIMS_GPIO_REG_T7R            GPIOG
  #define TRIMS_GPIO_PIN_T7R            LL_GPIO_PIN_15 // PG.15
  #define TRIMS_GPIO_REG_T8D            GPIOG
  #define TRIMS_GPIO_PIN_T8D            LL_GPIO_PIN_13 // PG.13
  #define TRIMS_GPIO_REG_T8U            GPIOB
  #define TRIMS_GPIO_PIN_T8U            LL_GPIO_PIN_4  // PB.04
#elif defined(RADIO_MT12)
  #define TRIMS_GPIO_REG_T1R            GPIOE
  #define TRIMS_GPIO_PIN_T1R            LL_GPIO_PIN_3 // PE.03
  #define TRIMS_GPIO_REG_T1L            GPIOE
  #define TRIMS_GPIO_PIN_T1L            LL_GPIO_PIN_4 // PE.04
  #define TRIMS_GPIO_REG_T2R            GPIOC
  #define TRIMS_GPIO_PIN_T2R            LL_GPIO_PIN_1 // PC.01
  #define TRIMS_GPIO_REG_T2L            GPIOC
  #define TRIMS_GPIO_PIN_T2L            LL_GPIO_PIN_2 // PC.02
  #define TRIMS_GPIO_REG_T3R            GPIOE
  #define TRIMS_GPIO_PIN_T3R            LL_GPIO_PIN_5 // PE.05
  #define TRIMS_GPIO_REG_T3L            GPIOE
  #define TRIMS_GPIO_PIN_T3L            LL_GPIO_PIN_6 // PE.06
  #define TRIMS_GPIO_REG_T4R            GPIOE
  #define TRIMS_GPIO_PIN_T4R            LL_GPIO_PIN_7 // PE.07
  #define TRIMS_GPIO_REG_T4L            GPIOE
  #define TRIMS_GPIO_PIN_T4L            LL_GPIO_PIN_8 // PE.08
  #define TRIMS_GPIO_REG_T5R            GPIOE
  #define TRIMS_GPIO_PIN_T5R            LL_GPIO_PIN_15 // PE.15
  #define TRIMS_GPIO_REG_T5L            GPIOE
  #define TRIMS_GPIO_PIN_T5L            LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_GX12)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4  // PE.04
#elif defined(RADIO_TPROS)
  #define TRIMS_GPIO_REG_LHL            GPIOC
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_4  // PE.04
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_RVU            GPIOD
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_5  // PE.05
#elif defined(RADIO_BUMBLEBEE)
  #define TRIMS_GPIO_REG_LHR            GPIOG
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_2  // PG.02
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_13 // PD.13
  #define TRIMS_GPIO_REG_LVD            GPIOG
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_3  // PG.03
  #define TRIMS_GPIO_REG_LVU            GPIOD
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_14 // PD.14
  #define TRIMS_GPIO_REG_RVD            GPIOF
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_2  // PF.02
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVU            GPIOE
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_RHR            GPIOF
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_1  // PF.01
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define TRIMS_GPIO_REG_LHL            GPIOE
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_LHR            GPIOE
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_4  // PE.04
  #define TRIMS_GPIO_REG_LVD            GPIOC
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_LVU            GPIOC
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RVD            GPIOE
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_RHL            GPIOD
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_RVU            GPIOE
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RHR            GPIOC
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_1  // PC.01
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOD
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_0  // PD.00
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4  // PE.04
#elif defined(PCBX7) && !defined(RADIO_COMMANDO8)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_4  // PE.04
#elif defined(RADIO_COMMANDO8)
  #define TRIMS_GPIO_REG_LHL            GPIOD
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_15 // PD.15
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_6  // PC.06 
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06 
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_1  // PE.01 
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOE
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_0  // PE.00 
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_7  // PC.07 
  #define TRIMS_GPIO_REG_RHR            GPIOE
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_15  // PE.15  
#elif defined(PCBX9LITE)
  #define TRIMS_GPIO_REG_LHR            GPIOC
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_4  // PC.04
  #define TRIMS_GPIO_REG_LHL            GPIOC
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_5  // PC.05
  #define TRIMS_GPIO_REG_LVU            GPIOB
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_0  // PB.00
  #define TRIMS_GPIO_REG_LVD            GPIOB
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_1  // PB.01
  #define TRIMS_GPIO_REG_RVU            GPIOE
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_13 // PE.13
  #define TRIMS_GPIO_REG_RHR            GPIOD
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_8  // PD.08
  #define TRIMS_GPIO_REG_RVD            GPIOE
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_14 // PE.14
  #define TRIMS_GPIO_REG_RHL            GPIOD
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_9  // PD.09
#elif defined(RADIO_X9DP2019)
  #define TRIMS_GPIO_REG_LHL            GPIOE
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_LHR            GPIOE
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_4  // PE.04
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHL            GPIOC
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_13 // PC.13
  #define TRIMS_GPIO_REG_RHR            GPIOC
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_1  // PC.01
#else
  #define TRIMS_GPIO_REG_LHL            GPIOE
  #define TRIMS_GPIO_PIN_LHL            LL_GPIO_PIN_4  // PE.04
  #define TRIMS_GPIO_REG_LHR            GPIOE
  #define TRIMS_GPIO_PIN_LHR            LL_GPIO_PIN_3  // PE.03
  #define TRIMS_GPIO_REG_LVD            GPIOE
  #define TRIMS_GPIO_PIN_LVD            LL_GPIO_PIN_6  // PE.06
  #define TRIMS_GPIO_REG_LVU            GPIOE
  #define TRIMS_GPIO_PIN_LVU            LL_GPIO_PIN_5  // PE.05
  #define TRIMS_GPIO_REG_RVD            GPIOC
  #define TRIMS_GPIO_PIN_RVD            LL_GPIO_PIN_3  // PC.03
  #define TRIMS_GPIO_REG_RHL            GPIOC
  #define TRIMS_GPIO_PIN_RHL            LL_GPIO_PIN_1  // PC.01
  #define TRIMS_GPIO_REG_RVU            GPIOC
  #define TRIMS_GPIO_PIN_RVU            LL_GPIO_PIN_2  // PC.02
  #define TRIMS_GPIO_REG_RHR            GPIOC
  #define TRIMS_GPIO_PIN_RHR            LL_GPIO_PIN_13 // PC.13
#endif

// Switches
#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_A_H         GPIOD
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_10 // PD.10
  #define SWITCHES_GPIO_REG_A_L         GPIOD
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_MT12)
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_0  // PE.00
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define SWITCHES_GPIO_REG_A           GPIOC
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_13  // PC.13
#elif defined(RADIO_GX12)
  #define SWITCHES_A_2POS
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_A           GPIOC
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_5   // PC.05
#elif defined(RADIO_T8)
  #define SWITCHES_GPIO_REG_A           GPIOE
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_14  // PE.14
#elif defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_A           GPIOE
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_3  // PE.03
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_13 // PE.13
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_0   // PE.00
  #define SWITCHES_GPIO_REG_A_L         GPIOD
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_A_L         GPIOD
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_2  // PD.02
  #define SWITCHES_GPIO_REG_A_H         GPIOD
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_1  // PD.01
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_A_INVERTED
  #define SWITCHES_GPIO_REG_A           GPIOD
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_0  // PD.00
#elif defined(RADIO_V14) || defined(RADIO_V12)
  // ADC based switch
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_13 // PE.13
#else
  #define SWITCHES_GPIO_REG_A_H         GPIOB
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_5  // PB.05
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_0  // PE.00
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_B_H         GPIOG
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_11 // PG.11
  #define SWITCHES_GPIO_REG_B_L         GPIOG
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_10 // PG.10
#elif defined(PCBXLITE)
  #define SWITCHES_GPIO_REG_B_L         GPIOA
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_6  // PA.06
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_T8)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_13 // PE.13
#elif defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_5  // PE.05
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_4  // PE.04
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_2  // PE.02
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_B_L         GPIOA
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_5  // PA.05
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_15 // PE.15
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_0  // PE.00
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_B_INVERTED
  #define SWITCHES_GPIO_REG_B           GPIOE
  #define SWITCHES_GPIO_PIN_B           LL_GPIO_PIN_2  // PE.02
#elif (RADIO_LR3PRO)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15  // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5   // PA.05
#elif defined(RADIO_BOXER)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15  // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_7   // PE.07
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_B           GPIOE
  #define SWITCHES_GPIO_PIN_B           LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_MT12)
  #define SWITCHES_GPIO_REG_B           GPIOD
  #define SWITCHES_GPIO_PIN_B           LL_GPIO_PIN_11 // PD.11
#elif defined(RADIO_GX12)
  #define SWITCHES_B_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
// ADC based switch
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PA.05
#elif defined(PCBX9LITE)
  #define SWITCHES_GPIO_REG_B_L         GPIOB
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_4  // PB.04
  #define SWITCHES_GPIO_REG_B_H         GPIOB
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PB.05
#else
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_2  // PE.02
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_C_H         GPIOF
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_13 // PF.13
  #define SWITCHES_GPIO_REG_C_L         GPIOF
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_14 // PF.14
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_3  // PE.03
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER)
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_0   // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_0   // PE.00
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_TX12)
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_11  // PD.11
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_0  // PE.00
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_0  // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_T8)
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_C_L         GPIOC
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_4  // PC.04 
  #define SWITCHES_GPIO_REG_C_H         GPIOA
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_6  // PA.06   
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
  #define SWITCHES_GPIO_REG_C           GPIOE
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_C           GPIOE
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_8  // PE.08
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_C           GPIOD
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_0  // PD.00
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PD.02
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_1  // PD.01
#elif defined(RADIO_MT12)
  #define SWITCHES_GPIO_REG_C           GPIOC
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_13 // PC.13
#elif defined(RADIO_GX12)
  #define SWITCHES_C_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
// ADC based switch
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_11 // PD.11
  #if defined(PCBX7ACCESS)    
    #define SWITCHES_GPIO_REG_C_H       GPIOD
    #define SWITCHES_GPIO_PIN_C_H       LL_GPIO_PIN_10 // PD.10
  #else
    #define SWITCHES_GPIO_REG_C_H       GPIOE
    #define SWITCHES_GPIO_PIN_C_H       LL_GPIO_PIN_0  // PE.00
  #endif
#else
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_C_L         GPIOA
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_5  // PA.05
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX9DP)
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_13 // PE.13
#elif defined(PCBXLITE)
  #define SWITCHES_GPIO_REG_D_L         GPIOB
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_4  // PB.04
  #define SWITCHES_GPIO_REG_D_H         GPIOB
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_5  // PB.05
#elif defined(RADIO_T8)
  #define SWITCHES_GPIO_REG_D           GPIOD
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_14  // PD.14
#elif defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_D           GPIOA
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define SWITCHES_GPIO_REG_D           GPIOE
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_8  // PE.08
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
  #define SWITCHES_GPIO_REG_D           GPIOD
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_14 // PD.14
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_D           GPIOC
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_13// PC.13
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_D_L         GPIOA
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_5  // PA.05
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_15 // PE.15
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_D           GPIOE
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_2  // PE.02
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_0  // PE.00
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_MT12)
  #define SWITCHES_GPIO_REG_D           GPIOC
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_3  // PC.03
#elif defined(RADIO_GX12)
  #define SWITCHES_D_2POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
// ADC based switch
#elif defined(PCBX7) && !defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX7) && defined(RADIO_COMMANDO8)
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX9LITE)
  #define SWITCHES_GPIO_REG_D           GPIOC
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_13 // PC.13
#else
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOB
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PB.01
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_13 // PE.13
#elif defined(PCBXLITES)
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_5  // PE.05
#elif defined(PCBXLITE)
  // no SWE but we want to remain compatible with XLiteS
#elif defined(PCBX9LITE)
  #define SWITCHES_GPIO_REG_E           GPIOA
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_BOXER)
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_TX12)
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_13 // PE.13
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TX12MK2)
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_14 // PE.14
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_E           GPIOC
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_5   // PC.05
#elif defined(RADIO_TPRO)
  #define SWITCHES_GPIO_REG_E           GPIOG
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_10  // PG.10
#elif defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_E           GPIOG
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_0  // PG.00
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_E           GPIOD
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_3  // PD.03
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_8  // PE.08
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_GPIO_REG_E           GPIOD
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_3  // PD.03
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(RADIO_GX12)
  #define SWITCHES_E_3POS
#elif  defined(RADIO_V14) || defined(RADIO_V12)
  #define SWITCHES_GPIO_REG_E           GPIOC
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_13  // PC.13
#elif defined(PCBX7)
  // no SWE
#else
  #define SWITCHES_GPIO_REG_E_H         GPIOB
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_3  // PB.03
  #define SWITCHES_GPIO_REG_E_L         GPIOB
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_4  // PB.04
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_11 // PE.11
#elif defined(PCBXLITES)
  #define SWITCHES_GPIO_REG_F           GPIOC
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_3  // PC.03
#elif defined(PCBXLITE)
  // no SWF but we want to remain compatible with XLiteS
#elif defined(PCBX9LITES)
  #define SWITCHES_GPIO_REG_F           GPIOC
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_3 // PC.03
#elif defined(PCBX9LITE)
// no SWF
#elif defined(RADIO_ZORRO) || defined(RADIO_BOXER)
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_1 // PE.01
#elif defined(RADIO_TX12)
  #define SWITCHES_GPIO_REG_F_L         GPIOE
  #define SWITCHES_GPIO_PIN_F_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_F_H         GPIOE
  #define SWITCHES_GPIO_PIN_F_H         LL_GPIO_PIN_2 // PE.02
#elif defined(RADIO_TX12MK2)
  #define SWITCHES_GPIO_REG_F_L         GPIOD
  #define SWITCHES_GPIO_PIN_F_L         LL_GPIO_PIN_14 // PD.14
  #define SWITCHES_GPIO_REG_F_H         GPIOE
  #define SWITCHES_GPIO_PIN_F_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_TPROS)
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TPRO)
  #define SWITCHES_GPIO_REG_F           GPIOG
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_9  // PG.09
#elif defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_F           GPIOG
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_1  // PG.01
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_F           GPIOB
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_5  // PB.05
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
  #define SWITCHES_GPIO_REG_F           GPIOC
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_13// PC.13
#elif defined(RADIO_FAMILY_T20)
  #define SWITCHES_GPIO_REG_F           GPIOB
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_5 // PB.05
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_TPROV2) || defined(RADIO_COMMANDO8) || defined(RADIO_LR3PRO) || defined(RADIO_POCKET)
  // no SWF
#elif defined(RADIO_T12)
  // no SWF
#elif defined(RADIO_GX12)
  #define SWITCHES_F_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_8 // PE.08
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_14 // PE.14
#else
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_14 // PE.14
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_G_H         GPIOF
  #define SWITCHES_GPIO_PIN_G_H         LL_GPIO_PIN_3  // PF.03
  #define SWITCHES_GPIO_REG_G_L         GPIOF
  #define SWITCHES_GPIO_PIN_G_L         LL_GPIO_PIN_4  // PF.04
#elif defined(PCBX9LITES)
  #define SWITCHES_GPIO_REG_G           GPIOC
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_2 // PC.02
#elif defined(RADIO_T12) || defined(RADIO_ZORRO)
  #define SWITCHES_GPIO_REG_G           GPIOE
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_G           GPIOG
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_10 // PG.10
#elif defined(RADIO_TPRO)
  // SW1
  #define FUNCTION_SWITCH_1             SG
  #define SWITCHES_GPIO_REG_G           GPIOG
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_7  // PG.07
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_G           GPIOD
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_15// PD.15 - Left trim push
#elif defined(RADIO_FAMILY_T20)
  //TRIM left low
  #define SWITCHES_GPIO_REG_G           GPIOD
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_15  // PD.15
#elif defined(RADIO_GX12)
  #define SWITCHES_G_2POS
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)  || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
  // no SWG
#else
  #define SWITCHES_GPIO_REG_G_H         GPIOE
  #define SWITCHES_GPIO_PIN_G_H         LL_GPIO_PIN_9  // PE.09
  #define SWITCHES_GPIO_REG_G_L         GPIOE
  #define SWITCHES_GPIO_PIN_G_L         LL_GPIO_PIN_8  // PE.08
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_H           GPIOF
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_1  // PF.01
#elif defined(PCBX9DP)
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_POCKET)
  // no SWH
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  // no SWH
#elif defined(RADIO_TX12) || defined(RADIO_MT12)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(RADIO_TPROV2)
  #define SWITCHES_GPIO_REG_H           GPIOG
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_9 // PG.09
#elif defined(RADIO_TPRO)
  // SW2
  #define FUNCTION_SWITCH_2             SH
  #define SWITCHES_GPIO_REG_H           GPIOG
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_6  // PG.06
#elif defined(RADIO_BUMBLEBEE)
  #define SWITCHES_GPIO_REG_H           GPIOF
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_0  // PF.00 - Right trim push
#elif defined(RADIO_FAMILY_T20)
  //TRIM right low
  #define SWITCHES_GPIO_REG_H           GPIOF
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_0  // PF.00
#elif defined(RADIO_TLITE) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_LR3PRO)
  // no SWH
#elif defined(RADIO_GX12)
  #define SWITCHES_H_2POS
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_14 // PD.14
#else
  #define SWITCHES_GPIO_REG_H           GPIOE
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_13 // PE.13
#endif

#if defined(RADIO_X9DP2019)
  #define SWITCHES_GPIO_REG_I           GPIOD
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_10 // PD.10
#elif defined(PCBX9D) || defined(PCBX9DP)
#endif

// X7 P400 P401 headers additionnal momentary switches
#if defined(PCBX7ACCESS)
  #define SWITCHES_GPIO_REG_I           GPIOC
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_13  // PC.13
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  // no SWI/J
#elif defined(RADIO_TX12)
  #define SWITCHES_GPIO_REG_I           GPIOE
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_14  // PE.14
  #define SWITCHES_GPIO_REG_J           GPIOD
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_14  // PD.14
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_V14) || defined(RADIO_V12)
  // no headers
#elif defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  // no SWI
  // no SWJ
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
// No I/J
#elif defined(RADIO_TPROV2)
  // SW1
  #define FUNCTION_SWITCH_1             SI
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_7  // PG.07
  // SW2
  #define FUNCTION_SWITCH_2             SJ
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_6  // PG.06
  // SW3
  #define FUNCTION_SWITCH_3             SK
  #define SWITCHES_GPIO_REG_K           GPIOG
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_5  // PG.05
  // SW4
  #define FUNCTION_SWITCH_4             SL
  #define SWITCHES_GPIO_REG_L           GPIOG
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_4  // PG.04
  // SW5
  #define FUNCTION_SWITCH_5             SM
  #define SWITCHES_GPIO_REG_M           GPIOG
  #define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_3  // PG.03
  // SW6
  #define FUNCTION_SWITCH_6             SO
  #define SWITCHES_GPIO_REG_O           GPIOG
  #define SWITCHES_GPIO_PIN_O           LL_GPIO_PIN_2  // PG.02
#elif defined(RADIO_BUMBLEBEE)
  // SW1
  #define FUNCTION_SWITCH_1             SJ
  #define SWITCHES_GPIO_REG_J           GPIOE
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_15 // PE.15
  // SW2
  #define FUNCTION_SWITCH_2             SK
  #define SWITCHES_GPIO_REG_K           GPIOE
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_14 // PE.14
  // SW3
  #define FUNCTION_SWITCH_3             SL
  #define SWITCHES_GPIO_REG_L          GPIOE
  #define SWITCHES_GPIO_PIN_L          LL_GPIO_PIN_13 // PE.13
  // SW4
  #define FUNCTION_SWITCH_4             SM
  #define SWITCHES_GPIO_REG_M           GPIOE
  #define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_10 // PE.10
  // SW5
  #define FUNCTION_SWITCH_5             SO
  #define SWITCHES_GPIO_REG_O           GPIOE
  #define SWITCHES_GPIO_PIN_O           LL_GPIO_PIN_8  // PE.08
  // SW6
  #define FUNCTION_SWITCH_6             SP
  #define SWITCHES_GPIO_REG_P           GPIOE
  #define SWITCHES_GPIO_PIN_P           LL_GPIO_PIN_12 // PE.12
#elif defined(RADIO_TPRO)
  // SW3
  #define FUNCTION_SWITCH_3             SI
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_5  // PG.05
  // SW4
  #define FUNCTION_SWITCH_4             SJ
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_4  // PG.04
  // SW5
  #define FUNCTION_SWITCH_5             SK
  #define SWITCHES_GPIO_REG_K           GPIOG
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_3  // PG.03
  // SW6
  #define FUNCTION_SWITCH_6             SL
  #define SWITCHES_GPIO_REG_L           GPIOG
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_2  // PG.02
#elif defined(RADIO_FAMILY_T20)
  //TRIM left up
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_11  // PG.11
  //TRIM right up
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_14  // PG.14
#elif defined(RADIO_GX12)
  // I and J are part of function switches bellow
#elif defined(PCBX7)
  #define SWITCHES_GPIO_REG_I           GPIOC
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_13  // PC.13
  #define SWITCHES_GPIO_REG_J           GPIOD
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_10  // PD.10
#endif

#if defined(RADIO_FAMILY_T20)
  //SW1
  #define FUNCTION_SWITCH_1             SK
  #define SWITCHES_GPIO_REG_K           GPIOE
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_10  // PE.10
  //SW2
  #define FUNCTION_SWITCH_2             SL
  #define SWITCHES_GPIO_REG_L           GPIOE
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_8   // PE.08
  //SW3
  #define FUNCTION_SWITCH_3             SM
  #define SWITCHES_GPIO_REG_M           GPIOE
  #define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_12  // PE.12
  //SW4
  #define FUNCTION_SWITCH_4             SN
  #define SWITCHES_GPIO_REG_N           GPIOE
  #define SWITCHES_GPIO_PIN_N           LL_GPIO_PIN_13  // PE.13
  //SW5
  #define FUNCTION_SWITCH_5             SO
  #define SWITCHES_GPIO_REG_O           GPIOE
  #define SWITCHES_GPIO_PIN_O           LL_GPIO_PIN_14  // PE.14
  //SW6
  #define FUNCTION_SWITCH_6             SP
  #define SWITCHES_GPIO_REG_P           GPIOE
  #define SWITCHES_GPIO_PIN_P           LL_GPIO_PIN_15  // PE.15
#endif

#if defined(RADIO_GX12)
  //SW1
  #define FUNCTION_SWITCH_1
  //SW2
  #define FUNCTION_SWITCH_2
  //SW3
  #define FUNCTION_SWITCH_3
  //SW4
  #define FUNCTION_SWITCH_4
  //SW5
  #define FUNCTION_SWITCH_5
  //SW6
  #define FUNCTION_SWITCH_6
#endif

#if defined(PCBX9E)
  #define SWITCHES_GPIO_REG_I_H         GPIOF
  #define SWITCHES_GPIO_PIN_I_H         LL_GPIO_PIN_15 // PF.15
  #define SWITCHES_GPIO_REG_I_L         GPIOE
  #define SWITCHES_GPIO_PIN_I_L         LL_GPIO_PIN_14 // PE.14
  #define SWITCHES_GPIO_REG_J_H         GPIOG
  #define SWITCHES_GPIO_PIN_J_H         LL_GPIO_PIN_7  // PG.07
  #define SWITCHES_GPIO_REG_J_L         GPIOG
  #define SWITCHES_GPIO_PIN_J_L         LL_GPIO_PIN_8  // PG.08
  #define SWITCHES_GPIO_REG_K_H         GPIOG
  #define SWITCHES_GPIO_PIN_K_H         LL_GPIO_PIN_13 // PG.13
  #define SWITCHES_GPIO_REG_K_L         GPIOG
  #define SWITCHES_GPIO_PIN_K_L         LL_GPIO_PIN_12 // PG.12
  #define SWITCHES_GPIO_REG_L_H         GPIOE
  #define SWITCHES_GPIO_PIN_L_H         LL_GPIO_PIN_9  // PE.09
  #define SWITCHES_GPIO_REG_L_L         GPIOE
  #define SWITCHES_GPIO_PIN_L_L         LL_GPIO_PIN_8  // PE.08
  #define SWITCHES_GPIO_REG_M_H         GPIOE
  #define SWITCHES_GPIO_PIN_M_H         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_M_L         GPIOA
  #define SWITCHES_GPIO_PIN_M_L         LL_GPIO_PIN_5  // PA.05
  #define SWITCHES_GPIO_REG_N_H         GPIOB
  #define SWITCHES_GPIO_PIN_N_H         LL_GPIO_PIN_3  // PB.03
  #define SWITCHES_GPIO_REG_N_L         GPIOB
  #define SWITCHES_GPIO_PIN_N_L         LL_GPIO_PIN_4  // PB.04
  #define SWITCHES_GPIO_REG_O_H         GPIOF
  #define SWITCHES_GPIO_PIN_O_H         LL_GPIO_PIN_7  // PF.07
  #define SWITCHES_GPIO_REG_O_L         GPIOE
  #define SWITCHES_GPIO_PIN_O_L         LL_GPIO_PIN_10 // PE.10
  #define SWITCHES_GPIO_REG_P_H         GPIOF
  #define SWITCHES_GPIO_PIN_P_H         LL_GPIO_PIN_11 // PF.11
  #define SWITCHES_GPIO_REG_P_L         GPIOF
  #define SWITCHES_GPIO_PIN_P_L         LL_GPIO_PIN_12 // PF.12
  #define SWITCHES_GPIO_REG_Q_H         GPIOF
  #define SWITCHES_GPIO_PIN_Q_H         LL_GPIO_PIN_5  // PF.05
  #define SWITCHES_GPIO_REG_Q_L         GPIOF
  #define SWITCHES_GPIO_PIN_Q_L         LL_GPIO_PIN_6  // PF.06
  #define SWITCHES_GPIO_REG_R_H         GPIOB
  #define SWITCHES_GPIO_PIN_R_H         LL_GPIO_PIN_5  // PB.05
  #define SWITCHES_GPIO_REG_R_L         GPIOE
  #define SWITCHES_GPIO_PIN_R_L         LL_GPIO_PIN_0  // PE.00
#endif

// 6POS SW
#if defined(RADIO_V14) || defined(RADIO_V12)
  #define SIXPOS_SWITCH_INDEX             6
  #define SIXPOS_LED_RED                255
  #define SIXPOS_LED_GREEN              255
  #define SIXPOS_LED_BLUE               255
#endif

// ADC
#if !defined(RADIO_FAMILY_T20) && !defined(RADIO_BUMBLEBEE)
  #define ADC_MAIN                      ADC1
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_0
  #define ADC_DMA_STREAM                LL_DMA_STREAM_4
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream4_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream4_IRQHandler
#endif

#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_28CYCLES
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT

#if defined(PCBX9E)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #define HARDWARE_POT4
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_8  // PF.08
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_5  // PC.05
  #define ADC_GPIO_PIN_POT4             LL_GPIO_PIN_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_10 // PF.10
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_9  // PF.09
  #define ADC_GPIO_PIN_SLIDER3          LL_GPIO_PIN_6  // PA.06
  #define ADC_GPIO_PIN_SLIDER4          LL_GPIO_PIN_1  // PB.01
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_SLIDER3)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER4)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_POT4 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_6  // ADC3_IN6
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_8  // ADC1_IN8
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_15 // ADC1_IN15
  #define ADC_CHANNEL_POT4              LL_ADC_CHANNEL_14 // ADC1_IN14
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_8  // ADC3_IN8
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_7  // ADC3_IN7
  #define ADC_CHANNEL_SLIDER3           LL_ADC_CHANNEL_6  // ADC1_IN6
  #define ADC_CHANNEL_SLIDER4           LL_ADC_CHANNEL_9  // ADC1_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10 // ADC1_IN10
  #define ADC_EXT                       ADC3
  #define ADC_EXT_CHANNELS              { ADC_CHANNEL_POT1, ADC_CHANNEL_SLIDER1, ADC_CHANNEL_SLIDER2 }
  #define ADC_EXT_DMA                   DMA2
  #define ADC_EXT_DMA_CHANNEL           LL_DMA_CHANNEL_2
  #define ADC_EXT_DMA_STREAM            LL_DMA_STREAM_0
  #define ADC_EXT_DMA_STREAM_IRQ        DMA2_Stream0_IRQn
  #define ADC_EXT_DMA_STREAM_IRQHandler DMA2_Stream0_IRQHandler
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_VREF_PREC2                300
#elif defined(PCBX9DP)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_6  // PA.06
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0  // PB.00
  #if PCBREV < 2019
    #define ADC_GPIO_PIN_POT3           LL_GPIO_PIN_1  // PB.01
    #define ADC_GPIOB_PINS              (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3)
    #define ADC_CHANNEL_POT3            LL_ADC_CHANNEL_9
    #define ADC_VREF_PREC2              330
  #else
    #define ADC_GPIOB_PINS              (ADC_GPIO_PIN_POT2)
    #define ADC_CHANNEL_POT3            0
    #define ADC_VREF_PREC2              300
  #endif
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_5  // PC.05
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_POT1)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_6
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_8
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_14
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_15
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
#elif defined(PCBXLITE)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_3  // PA.03
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_1  // PC.01
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_2  // PC.02
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define PWM_STICKS
  #define PWM_TIMER                     TIM5
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF2
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_IRQn                      TIM5_IRQn
  #define PWM_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH)
  #define STICK_PWM_CHANNEL_LH          0
  #define STICK_PWM_CHANNEL_LV          1
  #define STICK_PWM_CHANNEL_RV          3
  #define STICK_PWM_CHANNEL_RH          2
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_LH)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_11 // ADC1_IN11
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_12 // ADC1_IN12
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10 // ADC1_IN10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_T8)
  // No pots
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                0
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_COMMANDO8)
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_2  // PA.02  
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_2  // ADC1_IN2  
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_3  // ADC1_IN3  
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_0  // ADC1_IN0  
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_1  // ADC1_IN1  
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_5  // PC.05 
  #define ADC_GPIOB_PINS                0
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_15
  #define ADC_VREF_PREC2                320  
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define PWM_STICKS
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_TIMER                     TIM5
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF2
  #define PWM_IRQn                      TIM5_IRQn
  #define PWM_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define STICK_PWM_CHANNEL_LH          1
  #define STICK_PWM_CHANNEL_LV          0
  #define STICK_PWM_CHANNEL_RV          2
  #define STICK_PWM_CHANNEL_RH          3
#elif defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                0
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_T14)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_1  // PB.01
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8  // ADC12_IN8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_9  // ADC12_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_TPROS)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_3  // PA.03
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_2  // PA.02
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_1  // PB.01
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8  // ADC12_IN8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_9  // ADC12_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_T12MAX)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_3  // PA.03
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_2  // PA.02
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_1  // PB.01
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8  // ADC12_IN8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_9  // ADC12_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_6  // PA.06
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2)
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT1
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_6
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_8
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_BUMBLEBEE)
  #define ADC_MAIN                      ADC3
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_2
  #define ADC_DMA_STREAM                LL_DMA_STREAM_0
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream0_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream0_IRQHandler
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC123_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC123_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_2  // ADC123_IN2
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_3  // ADC123_IN3
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_15 // ADC3_IN15
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_9  // ADC3_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_14 // ADC3_IN14
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_5  // PF.05
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_3  // PF.03
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_4  // PF.04
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_POT1, ADC_GPIO_PIN_POT2, ADC_GPIO_PIN_BATT)
  #define ADC_EXT                       ADC1
  #define ADC_EXT_CHANNELS              { ADC_CHANNEL_RTC_BATT }
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_FAMILY_T20)
  #define ADC_MAIN                      ADC3
  #define ADC_DMA                       DMA2
  #define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_2
  #define ADC_DMA_STREAM                LL_DMA_STREAM_0
  #define ADC_DMA_STREAM_IRQ            DMA2_Stream0_IRQn
  #define ADC_DMA_STREAM_IRQHandler     DMA2_Stream0_IRQHandler
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_1  // ADC123_IN1
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_0  // ADC123_IN0
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_2  // ADC123_IN2
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_3  // ADC123_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_1  // PC.01
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_3  // PC.03
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_2  // PC.02
  #define ADC_GPIO_PIN_SLIDER3          LL_GPIO_PIN_5  // PF.05
  #define ADC_GPIO_PIN_SLIDER4          LL_GPIO_PIN_3  // PF.03
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_4  // PF.04
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_11  // ADC123_IN11
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_10  // ADC123_IN10
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_13  // ADC123_IN13
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_12  // ADC123_IN12
  #define ADC_CHANNEL_SLIDER3           LL_ADC_CHANNEL_15  // ADC3_IN15
  #define ADC_CHANNEL_SLIDER4           LL_ADC_CHANNEL_9   // ADC3_IN9
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_14  // ADC3_IN14
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_GPIOF_PINS                (ADC_GPIO_PIN_SLIDER3 | ADC_GPIO_PIN_SLIDER4 | ADC_GPIO_PIN_BATT)
  #define ADC_EXT                       ADC1
  #define ADC_EXT_CHANNELS              { ADC_CHANNEL_RTC_BATT }
  #define ADC_EXT_SAMPTIME              LL_ADC_SAMPLINGTIME_56CYCLES
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_MT12)
  #define ADC_GPIO_PIN_STICK_TH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_ST         LL_GPIO_PIN_1  // PA.01
  #define ADC_CHANNEL_STICK_TH          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_ST          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_3  // PA.03
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_5  // PA.05
  #define ADC_GPIO_PIN_POT4             LL_GPIO_PIN_6  // PA.06
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_2  // ADC123_IN2
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_3  // ADC123_IN3
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_5  // ADC12_IN5
  #define ADC_CHANNEL_POT4              LL_ADC_CHANNEL_6  // ADC12_IN6
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10 // ADC123_IN10
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_TH | ADC_GPIO_PIN_STICK_ST | ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_POT4)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_BATT)
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_GX12)
  // Serial gimbal only
  #define ADC_GPIO_PIN_STICK_LH
  #define ADC_GPIO_PIN_STICK_LV
  #define ADC_GPIO_PIN_STICK_RV
  #define ADC_GPIO_PIN_STICK_RH
  #define ADC_CHANNEL_STICK_LH
  #define ADC_CHANNEL_STICK_LV
  #define ADC_CHANNEL_STICK_RV
  #define ADC_CHANNEL_STICK_RH
  // Analogs
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_3  // PA.03
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_5  // PA.05
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_2  // ADC123_IN2
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_3  // ADC123_IN3
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_8  // ADC12_IN8
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_5  // ADC12_IN6
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10 // ADC123_IN10
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER2)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_SLIDER1)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_BATT)
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0     // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1     // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_2     // PA.02
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_3     // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC123_IN0 -> ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC123_IN1 -> ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_2  // ADC123_IN2 -> ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_3  // ADC123_IN3 -> ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0     // PB.00
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_6     // PA.06
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_2     // PC.02
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8  // ADC12_IN8 -> ADC1_IN8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_6  // ADC12_IN6 -> ADC1_IN6
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_12 // ADC123_IN12 -> ADC1_IN12
  #define ADC_GPIO_PIN_SWA              LL_GPIO_PIN_7     // PA.07
  #define ADC_GPIO_PIN_SWB              LL_GPIO_PIN_5     // PA.05
  #define ADC_GPIO_PIN_SWC              LL_GPIO_PIN_0     // PC.00
  #define ADC_GPIO_PIN_SWD              LL_GPIO_PIN_5     // PC.05
  #define ADC_CHANNEL_SWA               LL_ADC_CHANNEL_7  // ADC12_IN7 -> ADC1_IN7
  #define ADC_CHANNEL_SWB               LL_ADC_CHANNEL_5  // ADC12_IN5 -> ADC1_IN5
  #define ADC_CHANNEL_SWC               LL_ADC_CHANNEL_10 // ADC123_IN10 -> ADC1_IN10
  #define ADC_CHANNEL_SWD               LL_ADC_CHANNEL_15 // ADC12_IN15 -> ADC1_IN15
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SWA | ADC_GPIO_PIN_SWB)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT1)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT3 | ADC_GPIO_PIN_SWC | ADC_GPIO_PIN_SWD)
  #define ADC_VREF_PREC2                330
#elif defined(PCBX7)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_0  // PB.00
#if !defined(RADIO_POCKET)
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_6  // PA.06
#endif
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
#if defined(RADIO_BOXER) || defined(RADIO_V14) || defined(RADIO_V12)
  #define HARDWARE_POT3                 // 6 POS
  #define ADC_GPIO_PIN_POT3             LL_GPIO_PIN_5  // PA.05
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_5
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3)
  #define ADC_GPIOA_PINS_FS             (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3)
#else
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2)
#endif
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT1
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_6
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330
#elif defined(PCBX9LITE)
  #define HARDWARE_POT1
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_1  // PC.01
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_11 // ADC1_IN11
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10 // ADC1_IN10
  #define ADC_VREF_PREC2                300
#else
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_GPIO_PIN_STICK_RV         LL_GPIO_PIN_0  // PA.00
  #define ADC_GPIO_PIN_STICK_RH         LL_GPIO_PIN_1  // PA.01
  #define ADC_GPIO_PIN_STICK_LH         LL_GPIO_PIN_2  // PA.02
  #define ADC_GPIO_PIN_STICK_LV         LL_GPIO_PIN_3  // PA.03
  #define ADC_CHANNEL_STICK_RV          LL_ADC_CHANNEL_0  // ADC1_IN0
  #define ADC_CHANNEL_STICK_RH          LL_ADC_CHANNEL_1  // ADC1_IN1
  #define ADC_CHANNEL_STICK_LH          LL_ADC_CHANNEL_2  // ADC1_IN2
  #define ADC_CHANNEL_STICK_LV          LL_ADC_CHANNEL_3  // ADC1_IN3
  #define ADC_GPIO_PIN_POT1             LL_GPIO_PIN_6  // PA.06
  #define ADC_GPIO_PIN_POT2             LL_GPIO_PIN_0  // PB.00
  #define ADC_GPIO_PIN_SLIDER1          LL_GPIO_PIN_4  // PC.04
  #define ADC_GPIO_PIN_SLIDER2          LL_GPIO_PIN_5  // PC.05
  #define ADC_GPIO_PIN_BATT             LL_GPIO_PIN_0  // PC.00
  #define ADC_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT1)
  #define ADC_GPIOB_PINS                (ADC_GPIO_PIN_POT2)
  #define ADC_GPIOC_PINS                (ADC_GPIO_PIN_SLIDER1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_BATT)
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_6
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_8
  #define ADC_CHANNEL_POT3              LL_ADC_CHANNEL_9
  #define ADC_CHANNEL_SLIDER1           LL_ADC_CHANNEL_14
  #define ADC_CHANNEL_SLIDER2           LL_ADC_CHANNEL_15
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330
#endif

#if defined(PCBX9E)
  #if defined(HORUS_STICKS)
    #define ADC_DIRECTION { 1,-1,1,-1,-1,-1,-1,1, -1,1,1,-1, -1,-1 }
  #else
    #define ADC_DIRECTION { 1,1,-1,-1,-1,-1,-1,1, -1,1,1,-1, -1,-1 }
  #endif // HORUS_STICKS
#elif defined(PCBX9DP)
  #define ADC_DIRECTION {1,-1,1,-1,  1,1,-1,  1,1,  1,  1}
#elif defined(PCBX9D)
  #define ADC_DIRECTION {1,-1,1,-1,  1,1,0,   1,1,  1,  1}
#elif defined(RADIO_TX12)
  #define ADC_DIRECTION {-1,1,-1,1,  -1,-1,  1,  1}
#elif defined(RADIO_TX12MK2)
  #define ADC_DIRECTION {-1,1,-1,1,  1,-1}
#elif defined(RADIO_BOXER)
  #define ADC_DIRECTION {1,-1,1,-1, -1,-1, 1}
#elif defined(RADIO_ZORRO)
  #define ADC_DIRECTION {-1, 1, 1, -1, -1, 1, 1, 1}
#elif defined(RADIO_POCKET)
  #define ADC_DIRECTION {-1, 1, 1, -1, -1}
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS)
  #define ADC_DIRECTION {1,-1,1,-1,  1,1,  1,  1}
#elif defined(RADIO_T14)
  #define ADC_DIRECTION {1, -1, 1, -1,  1, 1, 1}
#elif defined(RADIO_T12MAX)
  #define ADC_DIRECTION {-1, 1, -1, 1,  1, 1, 1}
#elif defined(RADIO_BUMBLEBEE)
  #define ADC_DIRECTION {1,-1,1,-1,  1,1, 1}
#elif defined(RADIO_FAMILY_T20)
  #define ADC_DIRECTION {1,-1,1,-1,  -1,1,-1,-1,-1,-1}
#elif defined(RADIO_MT12)
  #define ADC_DIRECTION {1,-1,  -1,-1, 0, 0, 0, 0}
#elif defined(RADIO_GX12)
  #define ADC_DIRECTION {1,1,1,1, -1,-1, 1,-1}
#elif defined(RADIO_V14)
  #define ADC_DIRECTION {1,-1,1,-1, -1,-1,1, 0, 1,1,1,1 }
#elif defined(RADIO_V12)
  #define ADC_DIRECTION {1,-1,1,-1, -1,-1,1, 0, 1,1,1,1 }
#elif defined(PCBX7)
  #define ADC_DIRECTION {-1,1,-1,1,  1,1,  1,  1}
#elif defined(PCBX9LITE)
  #define ADC_DIRECTION {-1,1,-1,1,  1,1,  1}
#elif defined(PCBXLITE)
  #define ADC_DIRECTION {1,-1,-1,1,  -1,1,  1,  1}
#endif

// PWR and LED driver

#if defined(PCBX9LITE)
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOA, 7)  // PA.07
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOA, 6)  // PA.06
#elif defined(PCBXLITE)
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOA, 7)  // PA.07
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOE, 9)  // PE.09
#elif defined(RADIO_COMMANDO8)
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOD, 11) // PD.11
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOD, 10) // PD.10
#elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOG, 5)  // PG.05
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOG, 6)  // PG.06
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOD, 1)  // PD.01
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOB, 2)  // PB.02
#else
  #define PWR_SWITCH_GPIO               GPIO_PIN(GPIOD, 1)  // PD.01
  #define PWR_ON_GPIO                   GPIO_PIN(GPIOD, 0)  // PD.00
#endif

#if defined(RADIO_X9DP2019)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_clear
  #define GPIO_LED_GPIO_OFF             gpio_set
  #define LED_RED_GPIO                  GPIO_PIN(GPIOA, 13)
#elif defined(PCBXLITES)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_clear
  #define GPIO_LED_GPIO_OFF             gpio_set
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 4) // PE.04
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 6) // PE.06
#elif defined(PCBXLITE)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_clear
  #define GPIO_LED_GPIO_OFF             gpio_set
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 5) // PE.05
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 4) // PE.04
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 6) // PE.06
#elif defined(PCBX7ACCESS)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOB, 1) // PB.01
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 4) // PC.04
#elif defined(RADIO_T8)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOB, 1) // PB.01
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 4) // PC.04
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOC, 5) // PC.05
#elif defined(RADIO_COMMANDO8)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOC, 0) // PC.00
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 2) // PC.02
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOC, 1) // PC.01
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 2)  // PE.02
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 13) // PE.13
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOA, 7)  // PA.07
#elif defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 2)  // PE.02
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 13)  // PE.13
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOA, 7)  // PA.07
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TX12)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOB, 1)  // PB.01
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 5)  // PC.05
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOC, 4)  // PC.04
#elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOG, 1)  // PG.01
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 5)  // PC.05
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOC, 4)  // PC.04
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_GX12)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOA, 7)  // PA.07
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 13) // PE.13
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 2)  // PE.02
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 14)  // PE.14
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 13) // PE.13
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 2)  // PE.02
#elif defined(PCBX7)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOC, 4)  // PC.04
  #define LED_RED_GPIO                  GPIO_PIN(GPIOC, 5)  // PC.05
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOB, 1)  // PB.01
#elif defined(PCBX9LITES)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 5)  // PE.05
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 6)  // PE.06
#elif defined(PCBX9LITE)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 4)  // PE.04
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 5)  // PE.05
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOE, 6)  // PE.06
#endif

  // LED Strip
#if defined(RGBLEDS)
  #if defined(RADIO_GX12)
    #define LED_STRIP_LENGTH                8
  #else
    #define LED_STRIP_LENGTH                7
  #endif
  #define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 8) // PA.08 / TIM1_CH1
  #define LED_STRIP_GPIO_AF                 LL_GPIO_AF_1   // TIM1 / TIM2
  #define LED_STRIP_TIMER                   TIM1
  #define LED_STRIP_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH1
  #define LED_STRIP_TIMER_DMA               DMA2
  #define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMA_CHANNEL_6
  #define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_5
  #define LED_STRIP_TIMER_DMA_IRQn          DMA2_Stream5_IRQn
  #define LED_STRIP_TIMER_DMA_IRQHandler    DMA2_Stream5_IRQHandler
  #define LED_STRIP_REFRESH_PERIOD          50  //ms
#elif defined(RADIO_T14) || defined(RADIO_V14) || defined(RADIO_V12)
#if defined(RADIO_V14)
  #define LED_STRIP_LENGTH                  38
#elif defined(RADIO_V12)
  #define LED_STRIP_LENGTH                  6
#else
  #define LED_STRIP_LENGTH                  1
#endif
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

#if defined(FUNCTION_SWITCHES)
#if defined(RADIO_BUMBLEBEE)
  #define GPIO_FSLED_GPIO_ON            GPIO_SetBits
  #define GPIO_FSLED_GPIO_OFF           GPIO_ResetBits
  #define FSLED_GPIO                    GPIOF
  #define FSLED_GPIO_PIN_1              GPIO_PIN(FSLED_GPIO, 13)
  #define FSLED_GPIO_PIN_2              GPIO_PIN(FSLED_GPIO, 6)
  #define FSLED_GPIO_PIN_3              GPIO_PIN(FSLED_GPIO, 7)
  #define FSLED_GPIO_PIN_4              GPIO_PIN(FSLED_GPIO, 10)
  #define FSLED_GPIO_PIN_5              GPIO_PIN(FSLED_GPIO, 9)
  #define FSLED_GPIO_PIN_6              GPIO_PIN(FSLED_GPIO, 8)
#elif defined(RADIO_FAMILY_T20)
  #define GPIO_FSLED_GPIO_ON            GPIO_SetBits
  #define GPIO_FSLED_GPIO_OFF           GPIO_ResetBits
  #define FSLED_GPIO                    GPIOF
  #define FSLED_GPIO_PIN_1              GPIO_PIN(FSLED_GPIO, 10)
  #define FSLED_GPIO_PIN_2              GPIO_PIN(FSLED_GPIO, 9)
  #define FSLED_GPIO_PIN_3              GPIO_PIN(FSLED_GPIO, 8)
  #define FSLED_GPIO_PIN_4              GPIO_PIN(FSLED_GPIO, 7)
  #define FSLED_GPIO_PIN_5              GPIO_PIN(FSLED_GPIO, 6)
  #define FSLED_GPIO_PIN_6              GPIO_PIN(FSLED_GPIO, 13)
#else
  #define GPIO_FSLED_GPIO_ON            GPIO_SetBits
  #define GPIO_FSLED_GPIO_OFF           GPIO_ResetBits
  #define FSLED_GPIO                    GPIOF
  #define FSLED_GPIO_PIN_1              GPIO_PIN(FSLED_GPIO, 5)
  #define FSLED_GPIO_PIN_2              GPIO_PIN(FSLED_GPIO, 4)
  #define FSLED_GPIO_PIN_3              GPIO_PIN(FSLED_GPIO, 3)
  #define FSLED_GPIO_PIN_4              GPIO_PIN(FSLED_GPIO, 2)
  #define FSLED_GPIO_PIN_5              GPIO_PIN(FSLED_GPIO, 1)
  #define FSLED_GPIO_PIN_6              GPIO_PIN(FSLED_GPIO, 0)
#endif
#endif

// Internal Module
#if defined(PCBXLITE)
#define EXTERNAL_ANTENNA
#endif
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #if defined(PCBXLITES) || defined(PCBX9LITE)
    #define INTMODULE_PWR_GPIO             GPIO_PIN(GPIOA, 15) // PA.15
  #else
    #define INTMODULE_PWR_GPIO             GPIO_PIN(GPIOD, 9) // PD.09
  #endif
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 6) // PB.06
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 7) // PB.07
  #define INTMODULE_USART                  USART1
  #define INTMODULE_USART_IRQn             USART1_IRQn
  #define INTMODULE_DMA                    DMA2
  #define INTMODULE_DMA_STREAM             LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_CHANNEL            LL_DMA_CHANNEL_4
  #if defined(PCBXLITES)
    #define INTMODULE_BOOTCMD_GPIO         GPIO_PIN(GPIOC, 8) // PC.08
    #define INTMODULE_BOOTCMD_DEFAULT      1 // SET
  #endif
#elif defined(RADIO_X9DP2019)
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOA, 7) // PA.07
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 6) // PB.06
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 7) // PB.07
  #define INTMODULE_USART                  USART1
  #define INTMODULE_USART_IRQn             USART1_IRQn
  #define INTMODULE_DMA                    DMA2
  #define INTMODULE_DMA_STREAM             LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_CHANNEL            LL_DMA_CHANNEL_4
#elif defined(RADIO_X7ACCESS)
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOA, 7) // PC.05
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 6) // PB.06
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 7) // PB.07
  #define INTMODULE_USART                  USART1
  #define INTMODULE_USART_IRQHandler       USART1_IRQHandler
  #define INTMODULE_USART_IRQn             USART1_IRQn
  #define INTMODULE_DMA                    DMA2
  #define INTMODULE_DMA_STREAM             LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_CHANNEL            LL_DMA_CHANNEL_4
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_T20V2) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12) || defined(RADIO_GX12) || defined(RADIO_BUMBLEBEE)
#if defined(RADIO_T20V2)  || defined(RADIO_BUMBLEBEE)
  #define INTMODULE_BOOTCMD_GPIO           GPIO_PIN(GPIOE, 4) // PE.04
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 6) // PC.06
#else
#if defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
  #define INTMODULE_BOOTCMD_GPIO           GPIO_PIN(GPIOE, 14) // PE.14
#else
  #define INTMODULE_BOOTCMD_GPIO           GPIO_PIN(GPIOB, 1) // PB.01
#endif
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 4) // PC.04
#endif
#if defined(RADIO_V14) || defined(RADIO_V12)
  #define INTMODULE_BOOTCMD_DEFAULT     1 // RESET
#else
  #define INTMODULE_BOOTCMD_DEFAULT     0 // RESET
#endif
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 6) // PB.06
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 7) // PB.07
  #define INTMODULE_USART                  USART1
  #define INTMODULE_USART_IRQHandler       USART1_IRQHandler
  #define INTMODULE_USART_IRQn             USART1_IRQn
  #define INTMODULE_DMA                    DMA2
  #define INTMODULE_DMA_STREAM             LL_DMA_STREAM_7
  #define INTMODULE_DMA_STREAM_IRQ         DMA2_Stream7_IRQn
  #define INTMODULE_DMA_STREAM_IRQHandler  DMA2_Stream7_IRQHandler
  #define INTMODULE_DMA_CHANNEL            LL_DMA_CHANNEL_4
  #define INTMODULE_RX_DMA                 DMA2
  #define INTMODULE_RX_DMA_STREAM          LL_DMA_STREAM_2
  #define INTMODULE_RX_DMA_CHANNEL         LL_DMA_CHANNEL_4
#elif defined(PCBX9E) || defined(PCBX9DP) || defined(RADIO_X7)
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOA, 10) // PA.10
  #define INTMODULE_TX_GPIO_AF             LL_GPIO_AF_1
  #define INTMODULE_TIMER                  TIM1
  #define INTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define INTMODULE_TIMER_IRQn             TIM1_UP_TIM10_IRQn
  #define INTMODULE_TIMER_IRQHandler       TIM1_UP_TIM10_IRQHandler
  #define INTMODULE_TIMER_DMA              DMA2
  #define INTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_6
  #define INTMODULE_TIMER_DMA_STREAM       LL_DMA_STREAM_5
  #define INTMODULE_TIMER_DMA_STREAM_IRQn  DMA2_Stream5_IRQn
  #define INTMODULE_TIMER_DMA_IRQHandler   DMA2_Stream5_IRQHandler
  #define INTMODULE_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
#elif (defined(RADIO_FAMILY_JUMPER_T12) && defined(HARDWARE_INTERNAL_MODULE)) || defined(RADIO_TX12) || defined(RADIO_T8) || defined(RADIO_TPRO) || defined(RADIO_TPROV2)|| defined(RADIO_T20)
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 6)  // PC.06
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 10) // PB.10
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 11) // PB.11
  #define INTMODULE_USART                  USART3
  #define INTMODULE_USART_IRQn             USART3_IRQn
  // DMA1_Stream3 is already used by SDIO
  // #define INTMODULE_DMA                   NULL
  // #define INTMODULE_DMA_CHANNEL           0
  // #define INTMODULE_DMA_STREAM            0
  #define INTMODULE_RX_DMA                 DMA1
  #define INTMODULE_RX_DMA_STREAM          LL_DMA_STREAM_1
  #define INTMODULE_RX_DMA_CHANNEL         LL_DMA_CHANNEL_4
  #if defined(RADIO_TLITE)
    #define INTMODULE_BOOTCMD_GPIO         GPIO_PIN(GPIOA, 5) // PA.05
    #define INTMODULE_BOOTCMD_DEFAULT      0 // RESET
  #elif defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS)
    #define INTMODULE_BOOTCMD_GPIO         GPIO_PIN(GPIOF, 11) // PF.11
    #define INTMODULE_BOOTCMD_DEFAULT      0 // RESET
  #elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
    #define INTMODULE_BOOTCMD_GPIO         GPIO_PIN(GPIOE, 4) // PE.04
    #define INTMODULE_BOOTCMD_DEFAULT      0 // RESET
  #elif defined(RADIO_LR3PRO)
    #define INTMODULE_BOOTCMD_GPIO         GPIO_PIN(GPIOB, 5) // PB.05
    #define INTMODULE_BOOTCMD_DEFAULT      0 // RESET
  #endif
#elif defined(RADIO_COMMANDO8)
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOE, 8)  // PE.08
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOB, 10) // PB.10
  #define INTMODULE_RX_GPIO                GPIO_PIN(GPIOB, 11) // PB.11
  #define INTMODULE_USART                  USART3
  #define INTMODULE_USART_IRQn             USART3_IRQn
  // // DMA1_Stream3 is already used by SDIO
  // #define INTMODULE_DMA                   NULL
  // #define INTMODULE_DMA_CHANNEL           0
  // #define INTMODULE_DMA_STREAM            0
#else
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOD, 15) // PD.15
  #define INTMODULE_TX_GPIO                GPIO_PIN(GPIOA, 10) // PA.10
  #define INTMODULE_TX_GPIO_AF             LL_GPIO_AF_1
  #define INTMODULE_TIMER                  TIM1
  #define INTMODULE_TIMER_Channel          LL_TIM_CHANNEL_CH3
  #define INTMODULE_TIMER_IRQn             TIM1_UP_TIM10_IRQn
  #define INTMODULE_TIMER_IRQHandler       TIM1_UP_TIM10_IRQHandler
  #define INTMODULE_TIMER_DMA              DMA2
  #define INTMODULE_TIMER_DMA_CHANNEL      LL_DMA_CHANNEL_6
  #define INTMODULE_TIMER_DMA_STREAM       LL_DMA_STREAM_5
  #define INTMODULE_TIMER_DMA_STREAM_IRQn  DMA2_Stream5_IRQn
  #define INTMODULE_TIMER_DMA_IRQHandler   DMA2_Stream5_IRQHandler
  #define INTMODULE_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

// External Module
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019) || defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12) || defined(RADIO_T12MAX) || defined(RADIO_GX12)
  #if defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS) || defined(RADIO_ZORRO)|| defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)|| defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12)
    #define EXTMODULE_PWR_GPIO          GPIO_PIN(GPIOD, 8) // PD.08
  #elif defined(PCBX9LITE)
    #define EXTMODULE_PWR_GPIO          GPIO_PIN(GPIOA, 8) // PA.08
  #elif defined(RADIO_GX12)
    #define EXTMODULE_PWR_GPIO          GPIO_PIN(GPIOE, 0) // PE.00
  #else
    #define EXTMODULE_PWR_GPIO          GPIO_PIN(GPIOD, 11) // PD.11
  #endif
  #define EXTERNAL_MODULE_PWR_ON()      gpio_set(EXTMODULE_PWR_GPIO)
  #define EXTERNAL_MODULE_PWR_OFF()     gpio_clear(EXTMODULE_PWR_GPIO)
  #define IS_EXTERNAL_MODULE_ON()       gpio_read(EXTMODULE_PWR_GPIO)
  #define EXTMODULE_TX_GPIO             GPIO_PIN(GPIOC, 6) // PC.06
  #define EXTMODULE_RX_GPIO             GPIO_PIN(GPIOC, 7) // PC.07
  #define EXTMODULE_TIMER               TIM8
  #define EXTMODULE_TIMER_Channel       LL_TIM_CHANNEL_CH1
  #define EXTMODULE_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_IRQn          TIM8_UP_TIM13_IRQn
  #define EXTMODULE_TIMER_IRQHandler    TIM8_UP_TIM13_IRQHandler
  #define EXTMODULE_TIMER_TX_GPIO_AF    LL_GPIO_AF_3 // TIM8_CH1
  #define EXTMODULE_TIMER_DMA_CHANNEL           LL_DMA_CHANNEL_7
  #define EXTMODULE_TIMER_DMA                   DMA2
  #define EXTMODULE_TIMER_DMA_STREAM            LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn       DMA2_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler        DMA2_Stream1_IRQHandler
  #define EXTMODULE_USART                       USART6
  #define EXTMODULE_USART_IRQn                  USART6_IRQn
  #define EXTMODULE_USART_IRQHandler            USART6_IRQHandler
  #define EXTMODULE_USART_TX_DMA                DMA2
  #define EXTMODULE_USART_TX_DMA_CHANNEL        LL_DMA_CHANNEL_5
  #define EXTMODULE_USART_TX_DMA_STREAM         LL_DMA_STREAM_6
  #define EXTMODULE_USART_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5
  #define EXTMODULE_USART_RX_DMA_STREAM         LL_DMA_STREAM_1
  #if defined(RADIO_V14) || defined(RADIO_V12)
    #define EXTMODULE_TX_INVERT_GPIO           GPIO_PIN(GPIOE, 7) // PE.07
    #define EXTMODULE_RX_INVERT_GPIO           GPIO_PIN(GPIOE, 15) // PE.15
  #endif
#else
  #if defined(RADIO_COMMANDO8)
    #define EXTMODULE_RF_SWITCH_GPIO              GPIO_PIN(GPIOE, 7) // Antenna switching:    PE.07
    #define EXTMODULE_PWR_GPIO                    GPIO_PIN(GPIOE, 2) // External tuner power: PE.02
    #define EXTERNAL_MODULE_PWR_ON()        \
      do {                                  \
        gpio_set(EXTMODULE_PWR_GPIO);       \
        gpio_set(EXTMODULE_RF_SWITCH_GPIO); \
      } while (0)
    #define EXTERNAL_MODULE_PWR_OFF()         \
      do {                                    \
        gpio_clear(EXTMODULE_PWR_GPIO);       \
        gpio_clear(EXTMODULE_RF_SWITCH_GPIO); \
      } while (0)
  #elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
    #define EXTMODULE_PWR_GPIO                    GPIO_PIN(GPIOB, 1) // PB.01
    #define EXTERNAL_MODULE_PWR_ON()              gpio_set(EXTMODULE_PWR_GPIO)
    #define EXTERNAL_MODULE_PWR_OFF()             gpio_clear(EXTMODULE_PWR_GPIO)
  #else
    #define EXTMODULE_PWR_GPIO                    GPIO_PIN(GPIOD, 8) // PD.08
    #define EXTERNAL_MODULE_PWR_ON()              gpio_set(EXTMODULE_PWR_GPIO)
    #define EXTERNAL_MODULE_PWR_OFF()             gpio_clear(EXTMODULE_PWR_GPIO)
  #endif
  #define IS_EXTERNAL_MODULE_ON()               gpio_read(EXTMODULE_PWR_GPIO)
  #define EXTMODULE_TX_GPIO                     GPIO_PIN(GPIOA, 7) // PA.07
  #define EXTMODULE_TIMER                       TIM8
  #define EXTMODULE_TIMER_Channel               LL_TIM_CHANNEL_CH1N
  #define EXTMODULE_TIMER_TX_GPIO_AF            GPIO_AF3 // TIM8_CH1N
  #define EXTMODULE_TIMER_IRQn                  TIM8_UP_TIM13_IRQn
  #define EXTMODULE_TIMER_IRQHandler            TIM8_UP_TIM13_IRQHandler
  #define EXTMODULE_TIMER_DMA                   DMA2
  #define EXTMODULE_TIMER_DMA_STREAM            LL_DMA_STREAM_1
  #define EXTMODULE_TIMER_DMA_CHANNEL           LL_DMA_CHANNEL_7
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn       DMA2_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_IRQHandler        DMA2_Stream1_IRQHandler
  #define EXTMODULE_TIMER_FREQ                  (PERI2_FREQUENCY * TIMER_MULT_APB2)
#endif

// Trainer Port
#if defined(PCBXLITES) || defined(PCBX9LITE)
  // on these 2 radios the trainer port already uses DMA1_Stream6, we won't use the DMA
  #define TRAINER_IN_GPIO               GPIO_PIN(GPIOD, 13) // PD.13
  #define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH2
  #define TRAINER_OUT_GPIO              GPIO_PIN(GPIOD, 12) // PD.12
  #define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH1
  #if defined(PCBX9LITE)
    #define TRAINER_DETECT_GPIO         GPIO_PIN(GPIOD, 11) // PD.11
  #endif
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_GPIO_AF               GPIO_AF2 // TIM4_CH1 (Out) + TIM4_CH2 (In)
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
  #define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#elif defined(PCBXLITE)
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
#else
  #define TRAINER_IN_GPIO               GPIO_PIN(GPIOC, 8) // PC.08
  #define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH3
  #define TRAINER_OUT_GPIO              GPIO_PIN(GPIOC, 9) // PC.09
  #define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH4
#if defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define TRAINER_DETECT_GPIO           GPIO_PIN(GPIOE, 3) // PE.03
#elif defined(RADIO_MT12)
  #define TRAINER_DETECT_GPIO           GPIO_PIN(GPIOD, 14) // PD.14
#else
  #define TRAINER_DETECT_GPIO           GPIO_PIN(GPIOA, 8) // PA.08
#endif
#if !defined(RADIO_X9DP2019) && !defined(RADIO_X7ACCESS)
  #define TRAINER_DETECT_INVERTED
#endif
  #define TRAINER_TIMER                 TIM3
  #define TRAINER_TIMER_IRQn            TIM3_IRQn
  #define TRAINER_GPIO_AF               LL_GPIO_AF_2
  #define TRAINER_TIMER_IRQn            TIM3_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM3_IRQHandler
  #define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#endif

// Serial Port
#if (defined(PCBX7) && !defined(AUX_SERIAL)) || defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019)
#elif defined(RADIO_GX12)
  #define HARDWARE_TRAINER_AUX_SERIAL
  #define AUX_SERIAL_GPIO                   GPIOD
  #define AUX_SERIAL_TX_GPIO                GPIO_PIN(GPIOD, 8) // PD.08
  #define AUX_SERIAL_RX_GPIO                GPIO_PIN(GPIOD, 9) // PD.09
  #define AUX_SERIAL_USART                  USART3
  #define AUX_SERIAL_USART_IRQn             USART3_IRQn
  #define AUX_SERIAL_DMA_RX                 DMA1
  #define AUX_SERIAL_DMA_RX_STREAM          LL_DMA_STREAM_1
  #define AUX_SERIAL_DMA_RX_CHANNEL         LL_DMA_CHANNEL_4
#else
  #define HARDWARE_TRAINER_AUX_SERIAL
  #define AUX_SERIAL_GPIO                   GPIOB
  #define AUX_SERIAL_TX_GPIO                GPIO_PIN(GPIOB, 10) // PB.10
  #define AUX_SERIAL_RX_GPIO                GPIO_PIN(GPIOB, 11) // PB.11
  #define AUX_SERIAL_USART                  USART3
  #define AUX_SERIAL_USART_IRQn             USART3_IRQn
  #define AUX_SERIAL_DMA_RX                 DMA1
  #define AUX_SERIAL_DMA_RX_STREAM          LL_DMA_STREAM_1
  #define AUX_SERIAL_DMA_RX_CHANNEL         LL_DMA_CHANNEL_4
#endif

// Telemetry
#define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOD, 4) // PD.04
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019) || \
    defined(RADIO_X7ACCESS)
  #define TELEMETRY_SET_INPUT           1
#else
  #define TELEMETRY_SET_INPUT           0
#endif
#if defined(RADIO_V14) || defined(RADIO_V12)
  #define TELEMETRY_RX_REV_GPIO           GPIO_PIN(GPIOE, 0)  // PE.00
  #define TELEMETRY_TX_REV_GPIO           GPIO_PIN(GPIOE, 0)  // PE.00
#endif
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

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

// PCBREV
#if defined(RADIO_X7) && !defined(DEBUG_SEGGER_RTT)
  #define PCBREV_GPIO_PIN               GPIO_PIN(GPIOA, 14) // PA.14
  #define PCBREV_GPIO_PULL_DOWN
  #define PCBREV_VALUE()                (gpio_read(PCBREV_GPIO) >> 14)
#endif


// USB Charger
#if defined(USB_CHARGER)
  #define USB_CHARGER_GPIO              GPIO_PIN(GPIOB, 5)
#endif

// S.Port update connector
#if defined(PCBXLITE)
  #define SPORT_MAX_BAUDRATE            250000 // not tested
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOD, 8) // PD.08
#elif defined(PCBX7ACCESS)
  #define SPORT_MAX_BAUDRATE            400000
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOB, 3) // PB.03
#elif defined(RADIO_X7)
  #define SPORT_MAX_BAUDRATE            250000 // < 400000
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOB, 3) // PB.02
#elif defined(PCBX9LITE)
  #define SPORT_MAX_BAUDRATE            250000 // not tested
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOE, 15) // PE.15
#elif defined(RADIO_X9DP2019)
  #define SPORT_MAX_BAUDRATE            400000
  #define SPORT_UPDATE_PWR_GPIO         GPIO_PIN(GPIOA, 14) // PA.14
#else
  #define SPORT_MAX_BAUDRATE            400000
#endif

// Heartbeat for iXJT / ISRM synchro
#define INTMODULE_HEARTBEAT_TRIGGER               GPIO_FALLING
#if !defined(HARDWARE_EXTERNAL_MODULE)
  // No heartbeat
#elif defined(PCBXLITE)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_GPIO                GPIO_PIN(GPIOD, 15) // PD.15
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTD
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE15
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_15
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
#elif defined(PCBX9LITE)
  // No heartbeat
#elif defined(RADIO_X7ACCESS)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_GPIO                GPIO_PIN(GPIOA, 7) // PA.07
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTA
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE7
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_7
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI9_5_IRQ)
    #define USE_EXTI9_5_IRQ
    #define EXTI9_5_IRQ_Priority 5
  #endif
#elif defined(RADIO_X9DP2019)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_GPIO                GPIO_PIN(GPIOB, 1) // PB.01
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTB
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE1
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_1
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI1_IRQ)
    #define USE_EXTI1_IRQ
    #define EXTI1_IRQ_Priority 5
  #endif
#elif defined(MANUFACTURER_FRSKY)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_GPIO                GPIO_PIN(GPIOC, 7) // PC.07
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTC
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE7
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_7
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI9_5_IRQ)
    #define USE_EXTI9_5_IRQ
    #define EXTI9_5_IRQ_Priority 5
  #endif
#endif

// Trainer / Trainee from the module bay
#if defined(PCBX9LITE) || defined(PCBXLITE) || defined(RADIO_X9DP2019) || \
  defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || \
  defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) ||\
  defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_V14) || defined(RADIO_V12) \
  || defined(RADIO_GX12)
  #define TRAINER_MODULE_CPPM_TIMER            TIM3
  #define TRAINER_MODULE_CPPM_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO             EXTMODULE_RX_GPIO
  #define TRAINER_MODULE_CPPM_TIMER_Channel    LL_TIM_CHANNEL_CH2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn       TIM3_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF          LL_GPIO_AF_2
#if defined(PCBX9LITE) ||  defined(PCBXLITE)
  #define TRAINER_MODULE_CPPM_TIMER_IRQHandler TIM3_IRQHandler
#endif
#elif defined(INTMODULE_HEARTBEAT_GPIO) && defined(HARDWARE_EXTERNAL_MODULE)
  // Trainer CPPM input on heartbeat pin
  #define TRAINER_MODULE_CPPM_TIMER               TRAINER_TIMER
  #define TRAINER_MODULE_CPPM_FREQ                (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_CPPM_TIMER_Channel       LL_TIM_CHANNEL_CH2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn          TRAINER_TIMER_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF             GPIO_AF2
  // Trainer SBUS input on heartbeat pin
  #define TRAINER_MODULE_SBUS_USART               USART6
  #define TRAINER_MODULE_SBUS_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_SBUS_DMA                 DMA2
  #define TRAINER_MODULE_SBUS_DMA_STREAM          DMA2_Stream1
  #define TRAINER_MODULE_SBUS_DMA_STREAM_LL       LL_DMA_STREAM_1
  #define TRAINER_MODULE_SBUS_DMA_CHANNEL         LL_DMA_CHANNEL_5
#else
  // TODO: replace SBUS trainer with S.PORT pin
#endif

// USB
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOA, 9)  // PA.09
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// BackLight
#if defined(PCBX9E)
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM9
  #define BACKLIGHT_GPIO_1              GPIO_PIN(GPIOE, 6) // PE.06
  #define BACKLIGHT_GPIO_2              GPIO_PIN(GPIOE, 5) // PE.05
  #define BACKLIGHT_GPIO_AF             GPIO_AF3
#elif defined(PCBX9DP)
  #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define BACKLIGHT_TIMER               TIM4
  #define BACKLIGHT_GPIO_1              GPIO_PIN(GPIOD, 15) // PD.15
  #define BACKLIGHT_GPIO_2              GPIO_PIN(GPIOD, 13) // PD.13
  #define BACKLIGHT_GPIO_AF             GPIO_AF2
#elif defined(PCBXLITES) || defined(PCBX9LITE)
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIO_PIN(GPIOA, 10) // PA.10
  #define BACKLIGHT_GPIO_AF             GPIO_AF1
  #define BACKLIGHT_CCMR2               TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 // Channel 3, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC3E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR3
#elif defined(PCBXLITE)
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM1
  #define BACKLIGHT_GPIO                GPIO_PIN(GPIOA, 8) // PA.08
  #define BACKLIGHT_GPIO_AF             GPIO_AF1
  #define BACKLIGHT_CCMR1               TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 // Channel 1, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC1E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR1
#elif defined(RADIO_T8) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_FAMILY_T20) || defined(RADIO_T14) || defined(RADIO_BUMBLEBEE) || defined(RADIO_GX12)
  // No backlight: OLED display
#elif defined(RADIO_COMMANDO8)
  #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define BACKLIGHT_TIMER               TIM4
  #define BACKLIGHT_GPIO                GPIO_PIN(GPIOD, 13) // PD.13
  #define BACKLIGHT_GPIO_AF             GPIO_AF2
  #define BACKLIGHT_CCMR1               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 // Channel2, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC2E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR2
#elif defined(PCBX7)
  #define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define BACKLIGHT_TIMER               TIM4
  #define BACKLIGHT_GPIO                GPIO_PIN(GPIOD, 13) // PD.13
  #define BACKLIGHT_GPIO_AF             GPIO_AF2
  #define BACKLIGHT_CCMR1               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 // Channel2, PWM
  #define BACKLIGHT_CCER                TIM_CCER_CC2E
  #define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR2
#else
  #define BACKLIGHT_TIMER_FREQ          (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define BACKLIGHT_TIMER               TIM10
  #define BACKLIGHT_GPIO                GPIO_PIN(GPIOB, 8) // PB.08
  #define BACKLIGHT_GPIO_AF             GPIO_AF3
#endif

// LCD driver
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_FAMILY_T20) || defined(RADIO_MT12) || defined(RADIO_T12MAX) || defined(RADIO_V14) || defined(RADIO_V12) || defined(RADIO_BUMBLEBEE)
  #define LCD_VERTICAL_INVERT
#endif
#if defined(RADIO_LR3PRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_FAMILY_T20) || defined(RADIO_T14) || defined(RADIO_BUMBLEBEE) || defined(RADIO_GX12)
  #define LCD_HORIZONTAL_INVERT
  #define OLED_SCREEN
#endif
#if defined(RADIO_T14) || defined(RADIO_GX12)
  #define SSD1309_LCD
#endif
#if defined(PCBX9E)
  #define LCD_MOSI_GPIO                 GPIO_PIN(GPIOC, 12) // PC.12
  #define LCD_CLK_GPIO                  GPIO_PIN(GPIOC, 10) // PC.10
  #define LCD_A0_GPIO                   GPIO_PIN(GPIOC, 11) // PC.11
  #define LCD_NCS_GPIO                  GPIO_PIN(GPIOA, 15) // PA.15
  #define LCD_RST_GPIO                  GPIO_PIN(GPIOD, 15) // PD.15
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF6
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define LCD_MOSI_GPIO                 GPIO_PIN(GPIOC, 12) // PC.12
  #define LCD_CLK_GPIO                  GPIO_PIN(GPIOC, 10) // PC.10
  #define LCD_A0_GPIO                   GPIO_PIN(GPIOC, 11) // PC.11
  #define LCD_NCS_GPIO                  GPIO_PIN(GPIOD, 3) // PD.03
  #define LCD_RST_GPIO                  GPIO_PIN(GPIOD, 2) // PD.02
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF6
#elif defined(PCBX9DP) || defined(PCBX7)
  #define LCD_MOSI_GPIO                 GPIO_PIN(GPIOC, 12) // PC.12
  #define LCD_CLK_GPIO                  GPIO_PIN(GPIOC, 10) // PC.10
  #define LCD_A0_GPIO                   GPIO_PIN(GPIOC, 11) // PC.11
  #define LCD_NCS_GPIO                  GPIO_PIN(GPIOA, 15) // PA.15
#if defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
  #define LCD_RST_GPIO                  GPIO_PIN(GPIOA, 14) // PA.14
#else
  #define LCD_RST_GPIO                  GPIO_PIN(GPIOD, 12) // PD.12
#endif
  #define LCD_DMA                       DMA1
  #define LCD_DMA_Stream                DMA1_Stream7
  #define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
  #define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
  #define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
  #define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
  #define LCD_SPI                       SPI3
  #define LCD_GPIO_AF                   GPIO_AF6
#else
  // Soft SPI: these pins are not connected to SPI periph on STM32F205
  #define LCD_MOSI_GPIO                 GPIO_PIN(GPIOD, 10) // PD.10
  #define LCD_CLK_GPIO                  GPIO_PIN(GPIOD, 11) // PD.11
  #define LCD_A0_GPIO                   GPIO_PIN(GPIOD, 13) // PD.13
  #define LCD_NCS_GPIO                  GPIO_PIN(GPIOD, 14) // PD.14
  #define LCD_RST_GPIO                  GPIO_PIN(GPIOD, 12) // PD.12
#endif
#if defined(SSD1309_LCD)
  #define LCD_SPI_PRESCALER             SPI_CR1_BR_1
#else
  #define LCD_SPI_PRESCALER             0
#endif
#if defined(RADIO_GX12)
  #define OLED_VCC_CS                   GPIO_PIN(GPIOD, 11) // PD.11
#endif

// I2C Bus 1: EEPROM and CAT5137 digital pot for volume control
#define I2C_B1                          I2C1
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4

#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(PCBX7ACCESS) || \
    defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_X9DP2019) || \
    defined(RADIO_GX12)
  #define I2C_B1_SCL_GPIO               GPIO_PIN(GPIOB, 8)  // PB.08
  #define I2C_B1_SDA_GPIO               GPIO_PIN(GPIOB, 9)  // PB.09
#else
  #define I2C_B1_SCL_GPIO               GPIO_PIN(GPIOB, 6)  // PB.06
  #define I2C_B1_SDA_GPIO               GPIO_PIN(GPIOB, 7)  // PB.07
#endif

// EEPROM
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define EEPROM_WP_GPIO                GPIOD
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_7  // PD.07
#elif defined(PCBX7ACCESS)
  #define EEPROM_WP_GPIO                GPIOB
  #define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_5  // PB.05
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2)|| \
      defined(RADIO_BOXER) || defined(RADIO_GX12)
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
  #define I2C_B2_SCL_GPIO               GPIO_PIN(GPIOA, 8) // PA.08
  #define I2C_B2_SDA_GPIO               GPIO_PIN(GPIOC, 9) // PC.09
  #define I2C_B2_GPIO_AF                LL_GPIO_AF_4
  #define I2C_B2_CLK_RATE               400000

  #define IMU_I2C_BUS                   I2C_Bus_2
  #define IMU_I2C_ADDRESS               0x6B
#endif

#if defined(RADIO_GX12)
  #define I2C_B2                        I2C2
  #define I2C_B2_SCL_GPIO               GPIO_PIN(GPIOB, 10) // PB.10
  #define I2C_B2_SDA_GPIO               GPIO_PIN(GPIOB, 11) // PB.11
  #define I2C_B2_GPIO_AF                LL_GPIO_AF_4
  #define I2C_B2_CLK_RATE               400000

  #define USE_EXTI1_IRQ
  #define EXTI1_IRQ_Priority 5
#endif

// SD - SPI2
#if defined(RADIO_FAMILY_T20) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS) || defined(RADIO_BUMBLEBEE) || defined(RADIO_GX12)
  // Using chip, so no detect
#else
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define SD_PRESENT_GPIO               GPIO_PIN(GPIOD, 10) // PD.10
#elif defined(RADIO_COMMANDO8)
  #define SD_PRESENT_GPIO               GPIO_PIN(GPIOD, 8)  // PD.08
#else
  #define SD_PRESENT_GPIO               GPIO_PIN(GPIOD, 9)  // PD.09
#endif
#endif

#define STORAGE_USE_SDCARD_SPI

#define SD_GPIO                         GPIOB
#define SD_GPIO_PIN_CS                  LL_GPIO_PIN_12 // PB.12
#define SD_GPIO_PIN_SCK                 LL_GPIO_PIN_13 // PB.13
#define SD_GPIO_PIN_MISO                LL_GPIO_PIN_14 // PB.14
#define SD_GPIO_PIN_MOSI                LL_GPIO_PIN_15 // PB.15

#define SD_SPI                          SPI2
#define SD_SPI_DMA                      DMA1
#define SD_SPI_DMA_RX_STREAM            LL_DMA_STREAM_3
#define SD_SPI_DMA_TX_STREAM            LL_DMA_STREAM_4
#define SD_SPI_DMA_CHANNEL              LL_DMA_CHANNEL_0

// Audio
#define AUDIO_RCC_APB1Periph            LL_APB1_GRP1_PERIPH_DAC1
#define AUDIO_OUTPUT_GPIO               GPIO_PIN(GPIOA, 4)
#define AUDIO_DMA                       DMA1
#define AUDIO_TIM_IRQn                  TIM6_DAC_IRQn
#define AUDIO_TIM_IRQHandler            TIM6_DAC_IRQHandler
#define AUDIO_DMA_Stream                DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
#define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                     TIM6

#if defined(PCBXLITES)
  #define JACK_DETECT_GPIO              GPIO_PIN(GPIOC, 13) // PC.13
  #define AUDIO_SPEAKER_ENABLE_GPIO     GPIO_PIN(GPIOD, 14) // PD.14
  #define HEADPHONE_TRAINER_SWITCH_GPIO GPIO_PIN(GPIOD, 9)  // PD.09
#elif defined(RADIO_FAMILY_T20)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOG, 4) // PG.04
  #define AUDIO_MUTE_DELAY              500  // ms
  #define AUDIO_UNMUTE_DELAY            150  // ms
#elif defined(RADIO_COMMANDO8)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOB, 1)
  #define AUDIO_MUTE_DELAY              500  // ms
  #define INVERTED_MUTE_PIN
#elif defined(MANUFACTURER_RADIOMASTER) || defined(MANUFACTURER_JUMPER) || defined(RADIO_LR3PRO)
  #define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOE, 12)
  #define AUDIO_MUTE_DELAY              500  // ms
  #if defined(MANUFACTURER_JUMPER) || defined(RADIO_LR3PRO)
    #define AUDIO_UNMUTE_DELAY          250  // ms
  #else
    #define AUDIO_UNMUTE_DELAY          150  // ms
  #endif
#endif

// Haptic
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_T20V2)  || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS) || defined(RADIO_V14) || defined(RADIO_V12) || defined(RADIO_BUMBLEBEE) || defined(RADIO_GX12)
  #define HAPTIC_PWM
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOB, 3) // PB.03
  #define HAPTIC_GPIO_AF                GPIO_AF1
  #define HAPTIC_TIMER                  TIM2 // Timer 2 Channel1
  #define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR2
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
  #define HAPTIC_CCER                   TIM_CCER_CC2E
  #define BACKLIGHT_BDTR                TIM_BDTR_MOE
#elif defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS)
  #define HAPTIC_PWM
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOA, 10) // PA.10
  #define HAPTIC_GPIO_AF                GPIO_AF1
  #define HAPTIC_TIMER                  TIM1 // Timer1 Channel3
  #define HAPTIC_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR3
  #define HAPTIC_CCMR2                  TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2
  #define HAPTIC_CCER                   TIM_CCER_CC3E
#elif defined(PCBX9E) || defined(PCBX9DP) || defined(PCBX7)
  #define HAPTIC_PWM
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOB, 8) // PB.08
  #define HAPTIC_GPIO_AF                GPIO_AF3
  #define HAPTIC_TIMER                  TIM10  // Timer 10 Channel1
  #define HAPTIC_TIMER_FREQ             (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR1
  #define HAPTIC_CCMR1                  TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2
  #define HAPTIC_CCER                   TIM_CCER_CC1E
#else
  #define HAPTIC_GPIO                   GPIO_PIN(GPIOC, 12) // PC.12
#endif

#if defined(RADIO_BOXER) || defined(RADIO_FAMILY_T20) || defined(RADIO_X9DP2019) || defined (RADIO_V14) || defined(RADIO_GX12)
  // Flysky Hall Stick
  #define FLYSKY_HALL_SERIAL_USART                 UART4
  #define FLYSKY_HALL_SERIAL_GPIO                  GPIOA
  #define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
  #define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOA, 0) // PA.00
  #define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOA, 1) // PA.01
  #define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
  #define FLYSKY_HALL_SERIAL_DMA                   DMA1
  #define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
  #define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
#endif

#if defined(RADIO_GX12)
  #define HALL_SYNC                                GPIO_PIN(GPIOE, 7)  // PE.07
#endif

// Top LCD on X9E
#if defined(PCBX9E)
  #define TOPLCD_GPIO
  #define TOPLCD_GPIO_DATA              GPIO_PIN(GPIOG, 5)  // PG.05
  #define TOPLCD_GPIO_WR                GPIO_PIN(GPIOG, 4)  // PG.04
  #define TOPLCD_GPIO_CS1               GPIO_PIN(GPIOG, 3)  // PG.03
  #define TOPLCD_GPIO_CS2               GPIO_PIN(GPIOG, 15) // PG.15
  #define TOPLCD_GPIO_BL                GPIO_PIN(GPIOG, 2)  // PG.02
#endif

// Bluetooth
#if defined(PCBX9E)
  #define STORAGE_BLUETOOTH
  #define BT_USART                     USART6
  #define BT_USART_IRQn                USART6_IRQn
  #define BT_TX_GPIO                   GPIO_PIN(GPIOG, 14) // PG.14
  #define BT_RX_GPIO                   GPIO_PIN(GPIOG, 9)  // PG.09
  #define BT_EN_GPIO                   GPIO_PIN(GPIOG, 11) // PD.11
  // #define BT_BRTS_GPIO                 GPIOE
  // #define BT_BRTS_GPIO_PIN             LL_GPIO_PIN_12 // PE.12
  // #define BT_BCTS_GPIO                 GPIOG
  // #define BT_BCTS_GPIO_PIN             LL_GPIO_PIN_6  // PG.06
#elif defined(BLUETOOTH)
  #define STORAGE_BLUETOOTH
  #if defined(PCBX9DP)
    #define BT_EN_GPIO                  GPIO_PIN(GPIOB, 2) // PB.02
  #elif defined(PCBXLITE)
    #define BT_EN_GPIO                  GPIO_PIN(GPIOE, 15) // PE.15
  #elif defined(PCBX9LITES)
    #define BT_EN_GPIO                  GPIO_PIN(GPIOD, 14) // PD.14
  #elif defined(MANUFACTURER_RADIOMASTER)
    #if defined(RADIO_POCKET)
      #define BT_EN_GPIO                GPIO_PIN(GPIOA, 6) // PA.06
    #endif
  #else
    #define BT_EN_GPIO                  GPIO_PIN(GPIOE, 12) // PE.12
  #endif
  #define BT_TX_GPIO                    GPIO_PIN(GPIOB, 10) // PB.10
  #define BT_RX_GPIO                    GPIO_PIN(GPIOB, 11) // PB.11
  #define BT_USART                      USART3
  #define BT_USART_IRQn                 USART3_IRQn
  // #define BT_DMA_Stream_RX              DMA1_Stream1
  // #define BT_DMA_Channel_RX             DMA_Channel_4
#else
  #if defined(PCBX9D) || defined(PCBX9DP) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TX12) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_ZORRO)
    // To avoid change in modelsize, todo: remove me
    #define STORAGE_BLUETOOTH
  #endif
#endif

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler     TIM8_BRK_TIM12_IRQHandler
