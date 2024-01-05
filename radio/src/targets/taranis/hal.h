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
  #define KEYS_GPIO_REG_PAGE            GPIOD
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_3  // PD.03
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
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET)
#if defined(RADIO_MT12)
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_7  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_3  // PD.07
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
#if defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
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
#elif defined(RADIO_T20)
  #define KEYS_GPIO_REG_PAGE            GPIOD
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_11  // PD.11
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_10  // PD.10
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_12  // PD.12
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_7 // PE.7
#elif defined(PCBX7)
  #define KEYS_GPIO_REG_PAGE            GPIOD
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_10 // PE.10
#elif defined(PCBX9LITE)
  #define KEYS_GPIO_REG_PAGE            GPIOE
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_8  // PE.08
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
  #define KEYS_GPIO_REG_PAGE            GPIOD
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_12 // PE.12
#else
  #define KEYS_GPIO_REG_MENU            GPIOD
  #define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_EXIT            GPIOD
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_2  // PD.02
  #define KEYS_GPIO_REG_PAGE            GPIOD
  #define KEYS_GPIO_PIN_PAGE            LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PLUS            GPIOE
  #define KEYS_GPIO_PIN_PLUS            LL_GPIO_PIN_10 // PE.10
  #define KEYS_GPIO_REG_MINUS           GPIOE
  #define KEYS_GPIO_PIN_MINUS           LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_ENTER           GPIOE
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_12 // PE.12
#endif

// Rotary Encoder
#if defined(PCBX9E)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOD
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_12 // PD.12
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_13 // PD.13
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
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_11 // PE.11
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
#elif defined(RADIO_X7) || defined(RADIO_X7ACCESS) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_T20)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO           GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_9  // PE.09
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_11 // PE.11
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
  #define ROTARY_ENCODER_GPIO_PIN_A     GPIO_Pin_10 // PE.10
  #define ROTARY_ENCODER_GPIO_PIN_B     GPIO_Pin_12 // PE.12
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
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET)
  #define ROTARY_ENCODER_NAVIGATION
  #define ROTARY_ENCODER_GPIO              GPIOE
  #define ROTARY_ENCODER_GPIO_PIN_A        GPIO_Pin_9 // PE.9
  #define ROTARY_ENCODER_GPIO_PIN_B        GPIO_Pin_10 // PE.10
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
  #if defined(RADIO_TX12)
    #define ROTARY_ENCODER_SUPPORT_BUGGY_WIRING
  #endif
  #if defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET)
    #define ROTARY_ENCODER_INVERTED
  #endif
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
#elif defined(RADIO_T20)
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
  #define TRIMS_GPIO_REG_T7L            GPIOB
  #define TRIMS_GPIO_PIN_T7L            LL_GPIO_PIN_3  // PB.03
  #define TRIMS_GPIO_REG_T7R            GPIOG
  #define TRIMS_GPIO_PIN_T7R            LL_GPIO_PIN_15 // PG.15
  #define TRIMS_GPIO_REG_T8D            GPIOG
  #define TRIMS_GPIO_PIN_T8D            LL_GPIO_PIN_13 // PG.13
  #define TRIMS_GPIO_REG_T8U            GPIOB
  #define TRIMS_GPIO_PIN_T8U            LL_GPIO_PIN_4  // PB.04
#elif defined(RADIO_MT12)
  #define TRIMS_GPIO_REG_T1R            GPIOC
  #define TRIMS_GPIO_PIN_T1R            LL_GPIO_PIN_1 // PC.01
  #define TRIMS_GPIO_REG_T1L            GPIOC
  #define TRIMS_GPIO_PIN_T1L            LL_GPIO_PIN_2 // PC.02
  #define TRIMS_GPIO_REG_T2R            GPIOE
  #define TRIMS_GPIO_PIN_T2R            LL_GPIO_PIN_3 // PE.03
  #define TRIMS_GPIO_REG_T2L            GPIOE
  #define TRIMS_GPIO_PIN_T2L            LL_GPIO_PIN_4 // PE.04
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
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOD
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_10 // PD.10
  #define SWITCHES_GPIO_REG_A_L         GPIOD
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_MT12)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_0  // PE.00
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A           GPIOC
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_13  // PC.13
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A           GPIOE
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_14  // PE.14
#elif defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A           GPIOE
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_3  // PE.03
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_13 // PE.13
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_A_INVERTED
  #define SWITCHES_GPIO_REG_A           GPIOD
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_0  // PD.00
#elif defined(PCBX7)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_A_H         GPIOE
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_13 // PE.13
#else
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
  #define SWITCHES_GPIO_REG_A_H         GPIOB
  #define SWITCHES_GPIO_PIN_A_H         LL_GPIO_PIN_5  // PB.05
  #define SWITCHES_GPIO_REG_A_L         GPIOE
  #define SWITCHES_GPIO_PIN_A_L         LL_GPIO_PIN_0  // PE.00
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H         GPIOG
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_11 // PG.11
  #define SWITCHES_GPIO_REG_B_L         GPIOG
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_10 // PG.10
#elif defined(PCBXLITE)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOA
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_6  // PA.06
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_13 // PE.13
#elif defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_5  // PE.05
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_4  // PE.04
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_2  // PE.02
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_B_INVERTED
  #define SWITCHES_GPIO_REG_B           GPIOE
  #define SWITCHES_GPIO_PIN_B           LL_GPIO_PIN_2  // PE.02
#elif (RADIO_LR3PRO)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15  // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5   // PA.05
#elif defined(RADIO_BOXER)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15  // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_7   // PE.07
#elif defined(RADIO_MT12)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B           GPIOD
  #define SWITCHES_GPIO_PIN_B           LL_GPIO_PIN_11 // PD.11
#elif defined(PCBX7)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PA.05
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_L         GPIOB
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_4  // PB.04
  #define SWITCHES_GPIO_REG_B_H         GPIOB
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PB.05
#else
  #define STORAGE_SWITCH_B
  #define HARDWARE_SWITCH_B
  #define SWITCHES_GPIO_REG_B_H         GPIOE
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_2  // PE.02
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H         GPIOF
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_13 // PF.13
  #define SWITCHES_GPIO_REG_C_L         GPIOF
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_14 // PF.14
#elif defined(PCBXLITE) || defined(PCBX9LITE)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_3  // PE.03
#elif  defined(RADIO_TX12MK2) || defined(RADIO_BOXER)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_0   // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_11  // PD.11
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_0  // PE.00
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_0  // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_11  // PD.11
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PE.02
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOC
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_4  // PC.04 
  #define SWITCHES_GPIO_REG_C_H         GPIOA
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_6  // PA.06   
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C           GPIOE
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_L         GPIOD
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_2  // PD.02
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_1  // PD.01
#elif defined(RADIO_MT12)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C           GPIOC
  #define SWITCHES_GPIO_PIN_C           LL_GPIO_PIN_13 // PC.13
#elif defined(PCBX7)
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
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
  #define STORAGE_SWITCH_C
  #define HARDWARE_SWITCH_C
  #define SWITCHES_GPIO_REG_C_H         GPIOE
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_C_L         GPIOA
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_5  // PA.05
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX9DP)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_13 // PE.13
#elif defined(PCBXLITE)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOB
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_4  // PB.04
  #define SWITCHES_GPIO_REG_D_H         GPIOB
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_5  // PB.05
#elif defined(RADIO_T8)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOD
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_14  // PD.14
#elif defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOA
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOE
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_8  // PE.08
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOD
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_14 // PD.14
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_0  // PE.00
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_MT12)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOC
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_3  // PC.03
#elif defined(PCBX7) && !defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX7) && defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_L         GPIOE
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_2  // PE.02
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D           GPIOC
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_13 // PC.13
#else
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
  #define SWITCHES_GPIO_REG_D_H         GPIOE
  #define SWITCHES_GPIO_PIN_D_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_D_L         GPIOB
  #define SWITCHES_GPIO_PIN_D_L         LL_GPIO_PIN_1  // PB.01
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_13 // PE.13
#elif defined(PCBXLITES)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_5  // PE.05
#elif defined(PCBXLITE)
  // no SWE but we want to remain compatible with XLiteS
  #define STORAGE_SWITCH_E
#elif defined(PCBX9LITE)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOA
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_5  // PA.05
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_BOXER)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_13 // PE.13
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOE
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_14 // PE.14
  #define SWITCHES_GPIO_REG_E_L         GPIOE
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_7  // PE.07
#elif defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOG
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_0  // PG.00
#elif defined(RADIO_TPRO)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOG
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_10  // PG.10
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E           GPIOD
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_3  // PD.03
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(PCBX7)
  // no SWE
#else
  #define STORAGE_SWITCH_E
  #define HARDWARE_SWITCH_E
  #define SWITCHES_GPIO_REG_E_H         GPIOB
  #define SWITCHES_GPIO_PIN_E_H         LL_GPIO_PIN_3  // PB.03
  #define SWITCHES_GPIO_REG_E_L         GPIOB
  #define SWITCHES_GPIO_PIN_E_L         LL_GPIO_PIN_4  // PB.04
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_11 // PE.11
#elif defined(PCBXLITES)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOC
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_3  // PC.03
#elif defined(PCBXLITE)
  // no SWF but we want to remain compatible with XLiteS
  #define STORAGE_SWITCH_F
#elif defined(PCBX9LITES)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOC
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_3 // PC.03
#elif defined(PCBX9LITE)
// no SWF
#elif defined(RADIO_ZORRO) || defined(RADIO_BOXER)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_1 // PE.01
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F_L         GPIOE
  #define SWITCHES_GPIO_PIN_F_L         LL_GPIO_PIN_1  // PE.01
  #define SWITCHES_GPIO_REG_F_H         GPIOE
  #define SWITCHES_GPIO_PIN_F_H         LL_GPIO_PIN_2 // PE.02
#elif defined(RADIO_TX12MK2)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F_L         GPIOD
  #define SWITCHES_GPIO_PIN_F_L         LL_GPIO_PIN_14 // PD.14
  #define SWITCHES_GPIO_REG_F_H         GPIOE
  #define SWITCHES_GPIO_PIN_F_H         LL_GPIO_PIN_1  // PE.01
#elif defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOG
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_1  // PG.01
#elif defined(RADIO_TPRO)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOG
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_9  // PG.09
#elif defined(RADIO_T20)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOB
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_5 // PB.05
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_TPROV2) || defined(RADIO_COMMANDO8) || defined(RADIO_LR3PRO) || defined(RADIO_POCKET)
  // no SWF
  #define STORAGE_SWITCH_F
#elif defined(RADIO_T12)
  // no SWF
#elif defined(PCBX7)
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_14 // PE.14
#else
  #define STORAGE_SWITCH_F
  #define HARDWARE_SWITCH_F
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_14 // PE.14
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G_H         GPIOF
  #define SWITCHES_GPIO_PIN_G_H         LL_GPIO_PIN_3  // PF.03
  #define SWITCHES_GPIO_REG_G_L         GPIOF
  #define SWITCHES_GPIO_PIN_G_L         LL_GPIO_PIN_4  // PF.04
#elif defined(PCBX9LITES)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOC
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_2 // PC.02
#elif defined(RADIO_T12) || defined(RADIO_ZORRO)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOE
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_14 // PE.14
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER)
  #define STORAGE_SWITCH_G
#elif defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOG
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_10 // PG.10
#elif defined(RADIO_TPRO)
  // SW1
  #define FUNCTION_SWITCH_1             SG
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOG
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_7  // PG.07
#elif defined(RADIO_T20)
  //TRIM left low
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G           GPIOD
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_15  // PD.15
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)  || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_MT12) || defined(RADIO_POCKET)
  // no SWG
#else
  #define STORAGE_SWITCH_G
  #define HARDWARE_SWITCH_G
  #define SWITCHES_GPIO_REG_G_H         GPIOE
  #define SWITCHES_GPIO_PIN_G_H         LL_GPIO_PIN_9  // PE.09
  #define SWITCHES_GPIO_REG_G_L         GPIOE
  #define SWITCHES_GPIO_PIN_G_L         LL_GPIO_PIN_8  // PE.08
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOF
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_1  // PF.01
#elif defined(PCBX9DP)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_14 // PD.14
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_POCKET)
  // no SWH
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_H
  // no SWH
#elif defined(RADIO_TX12) || defined(RADIO_MT12)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER)
  #define STORAGE_SWITCH_H
#elif defined(RADIO_TPROV2)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOG
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_9 // PG.09
#elif defined(RADIO_TPRO)
  // SW2
  #define FUNCTION_SWITCH_2             SH
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOG
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_6  // PG.06
#elif defined(RADIO_T20)
  //TRIM right low
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOF
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_0  // PF.00
#elif defined(RADIO_TLITE) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
  // no SWH
  #define STORAGE_SWITCH_H
#elif defined(PCBX7)
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_14 // PD.14
#else
  #define STORAGE_SWITCH_H
  #define HARDWARE_SWITCH_H
  #define SWITCHES_GPIO_REG_H           GPIOE
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_13 // PE.13
#endif

#if defined(RADIO_X9DP2019)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOD
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_10 // PD.10
#elif defined(PCBX9D) || defined(PCBX9DP)
  #define STORAGE_SWITCH_I
#endif

// X7 P400 P401 headers additionnal momentary switches
#if defined(PCBX7ACCESS)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOC
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_13  // PC.13
  #define STORAGE_SWITCH_J
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  #define STORAGE_SWITCH_I
  #define STORAGE_SWITCH_J
  // no SWI/J
#elif defined(RADIO_TX12)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOE
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_14  // PE.14
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOD
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_14  // PD.14
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
  // no headers
#elif defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  // no SWI
  #define STORAGE_SWITCH_I
  // no SWJ
  #define STORAGE_SWITCH_J
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  // No I/J
#elif defined(RADIO_TPROV2)
  // SW1
  #define FUNCTION_SWITCH_1             SI
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_7  // PG.07
  // SW2
  #define FUNCTION_SWITCH_2             SJ
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_6  // PG.06
  // SW3
  #define FUNCTION_SWITCH_3             SK
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K           GPIOG
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_5  // PG.05
  // SW4
  #define FUNCTION_SWITCH_4             SL
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L           GPIOG
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_4  // PG.04
  // SW5
  #define FUNCTION_SWITCH_5             SM
  #define STORAGE_SWITCH_M
  #define HARDWARE_SWITCH_M
  #define SWITCHES_GPIO_REG_M           GPIOG
  #define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_3  // PG.03
  // SW6
  #define FUNCTION_SWITCH_6             SO
  #define STORAGE_SWITCH_O
  #define HARDWARE_SWITCH_O
  #define SWITCHES_GPIO_REG_O           GPIOG
  #define SWITCHES_GPIO_PIN_O           LL_GPIO_PIN_2  // PG.02
#elif defined(RADIO_TPRO)
  // SW3
  #define FUNCTION_SWITCH_3             SI
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_5  // PG.05
  // SW4
  #define FUNCTION_SWITCH_4             SJ
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_4  // PG.04
  // SW5
  #define FUNCTION_SWITCH_5             SK
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K           GPIOG
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_3  // PG.03
  // SW6
  #define FUNCTION_SWITCH_6             SL
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L           GPIOG
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_2  // PG.02
#elif defined(RADIO_T20)
  //TRIM left up
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOG
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_11  // PG.11
  //TRIM right up
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOG
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_14  // PG.14
#elif defined(PCBX7)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I           GPIOC
  #define SWITCHES_GPIO_PIN_I           LL_GPIO_PIN_13  // PC.13
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J           GPIOD
  #define SWITCHES_GPIO_PIN_J           LL_GPIO_PIN_10  // PD.10
#endif

#if defined(RADIO_T20)
  //SW1
  #define FUNCTION_SWITCH_1             SK
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K           GPIOE
  #define SWITCHES_GPIO_PIN_K           LL_GPIO_PIN_10  // PE.10
  //SW2
  #define FUNCTION_SWITCH_2             SL
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L           GPIOE
  #define SWITCHES_GPIO_PIN_L           LL_GPIO_PIN_8   // PE.08
  //SW3
  #define FUNCTION_SWITCH_3             SM
  #define STORAGE_SWITCH_M
  #define HARDWARE_SWITCH_M
  #define SWITCHES_GPIO_REG_M           GPIOE
  #define SWITCHES_GPIO_PIN_M           LL_GPIO_PIN_12  // PE.12
  //SW4
  #define FUNCTION_SWITCH_4             SN
  #define STORAGE_SWITCH_N
  #define HARDWARE_SWITCH_N
  #define SWITCHES_GPIO_REG_N           GPIOE
  #define SWITCHES_GPIO_PIN_N           LL_GPIO_PIN_13  // PE.13
  //SW5
  #define FUNCTION_SWITCH_5             SO
  #define STORAGE_SWITCH_O
  #define HARDWARE_SWITCH_O
  #define SWITCHES_GPIO_REG_O           GPIOE
  #define SWITCHES_GPIO_PIN_O           LL_GPIO_PIN_14  // PE.14
  //SW6
  #define FUNCTION_SWITCH_6             SP
  #define STORAGE_SWITCH_P
  #define HARDWARE_SWITCH_P
  #define SWITCHES_GPIO_REG_P           GPIOE
  #define SWITCHES_GPIO_PIN_P           LL_GPIO_PIN_15  // PE.15
#endif

#if defined(PCBX9E)
  #define STORAGE_SWITCH_I
  #define HARDWARE_SWITCH_I
  #define SWITCHES_GPIO_REG_I_H         GPIOF
  #define SWITCHES_GPIO_PIN_I_H         LL_GPIO_PIN_15 // PF.15
  #define SWITCHES_GPIO_REG_I_L         GPIOE
  #define SWITCHES_GPIO_PIN_I_L         LL_GPIO_PIN_14 // PE.14
  #define STORAGE_SWITCH_J
  #define HARDWARE_SWITCH_J
  #define SWITCHES_GPIO_REG_J_H         GPIOG
  #define SWITCHES_GPIO_PIN_J_H         LL_GPIO_PIN_7  // PG.07
  #define SWITCHES_GPIO_REG_J_L         GPIOG
  #define SWITCHES_GPIO_PIN_J_L         LL_GPIO_PIN_8  // PG.08
  #define STORAGE_SWITCH_K
  #define HARDWARE_SWITCH_K
  #define SWITCHES_GPIO_REG_K_H         GPIOG
  #define SWITCHES_GPIO_PIN_K_H         LL_GPIO_PIN_13 // PG.13
  #define SWITCHES_GPIO_REG_K_L         GPIOG
  #define SWITCHES_GPIO_PIN_K_L         LL_GPIO_PIN_12 // PG.12
  #define STORAGE_SWITCH_L
  #define HARDWARE_SWITCH_L
  #define SWITCHES_GPIO_REG_L_H         GPIOE
  #define SWITCHES_GPIO_PIN_L_H         LL_GPIO_PIN_9  // PE.09
  #define SWITCHES_GPIO_REG_L_L         GPIOE
  #define SWITCHES_GPIO_PIN_L_L         LL_GPIO_PIN_8  // PE.08
  #define STORAGE_SWITCH_M
  #define HARDWARE_SWITCH_M
  #define SWITCHES_GPIO_REG_M_H         GPIOE
  #define SWITCHES_GPIO_PIN_M_H         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_M_L         GPIOA
  #define SWITCHES_GPIO_PIN_M_L         LL_GPIO_PIN_5  // PA.05
  #define STORAGE_SWITCH_N
  #define HARDWARE_SWITCH_N
  #define SWITCHES_GPIO_REG_N_H         GPIOB
  #define SWITCHES_GPIO_PIN_N_H         LL_GPIO_PIN_3  // PB.03
  #define SWITCHES_GPIO_REG_N_L         GPIOB
  #define SWITCHES_GPIO_PIN_N_L         LL_GPIO_PIN_4  // PB.04
  #define STORAGE_SWITCH_O
  #define HARDWARE_SWITCH_O
  #define SWITCHES_GPIO_REG_O_H         GPIOF
  #define SWITCHES_GPIO_PIN_O_H         LL_GPIO_PIN_7  // PF.07
  #define SWITCHES_GPIO_REG_O_L         GPIOE
  #define SWITCHES_GPIO_PIN_O_L         LL_GPIO_PIN_10 // PE.10
  #define STORAGE_SWITCH_P
  #define HARDWARE_SWITCH_P
  #define SWITCHES_GPIO_REG_P_H         GPIOF
  #define SWITCHES_GPIO_PIN_P_H         LL_GPIO_PIN_11 // PF.11
  #define SWITCHES_GPIO_REG_P_L         GPIOF
  #define SWITCHES_GPIO_PIN_P_L         LL_GPIO_PIN_12 // PF.12
  #define STORAGE_SWITCH_Q
  #define HARDWARE_SWITCH_Q
  #define SWITCHES_GPIO_REG_Q_H         GPIOF
  #define SWITCHES_GPIO_PIN_Q_H         LL_GPIO_PIN_5  // PF.05
  #define SWITCHES_GPIO_REG_Q_L         GPIOF
  #define SWITCHES_GPIO_PIN_Q_L         LL_GPIO_PIN_6  // PF.06
  #define STORAGE_SWITCH_R
  #define HARDWARE_SWITCH_R
  #define SWITCHES_GPIO_REG_R_H         GPIOB
  #define SWITCHES_GPIO_PIN_R_H         LL_GPIO_PIN_5  // PB.05
  #define SWITCHES_GPIO_REG_R_L         GPIOE
  #define SWITCHES_GPIO_PIN_R_L         LL_GPIO_PIN_0  // PE.00
#endif

// ADC
#if !defined(RADIO_T20)
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
  #define PWM_GPIO_AF                   LL_GPIO_AF_2
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
  #define PWM_GPIO_AF                   GPIO_AF_TIM5
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
#elif defined(RADIO_T20)
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
  #define ADC_RCC_AHB1Periph            (RCC_AHB1Periph_DMA2)
  #define ADC_RCC_APB1Periph            0
  #define ADC_RCC_APB2Periph            0
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
#if defined(RADIO_BOXER)
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
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define ADC_DIRECTION {1,-1,1,-1,  1,1,  1,  1}
#elif defined(RADIO_T20)
  #define ADC_DIRECTION {1,-1,1,-1,  -1,1,-1,-1,-1,-1}
#elif defined(RADIO_MT12)
  #define ADC_DIRECTION {1,-1,  -1,-1, 0, 0, 0, 0}
#elif defined(PCBX7)
  #define ADC_DIRECTION {-1,1,-1,1,  1,1,  1,  1}
#elif defined(PCBX9LITE)
  #define ADC_DIRECTION {-1,1,-1,1,  1,1,  1}
#elif defined(PCBXLITE)
  #define ADC_DIRECTION {1,-1,-1,1,  -1,1,  1,  1}
#endif

// PWR and LED driver
#define PWR_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG)

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
#elif defined(RADIO_COMMANDO8)
  #define PWR_SWITCH_GPIO               GPIOD
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_11  // PD.11
  #define PWR_ON_GPIO                   GPIOD
  #define PWR_ON_GPIO_PIN               GPIO_Pin_10  // PD.10
#elif defined(RADIO_T20)
  #define PWR_SWITCH_GPIO               GPIOG
  #define PWR_SWITCH_GPIO_PIN           GPIO_Pin_5   // PG.05
  #define PWR_ON_GPIO                   GPIOG
  #define PWR_ON_GPIO_PIN               GPIO_Pin_6   // PG.06
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
#elif defined(RADIO_COMMANDO8)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_2  // PC.02
  #define LED_BLUE_GPIO                 GPIOC
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_1  // PC.01
  #define LED_GREEN_GPIO                GPIOC
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_0  // PC.00
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOE
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_2  // PE.02
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_13 // PE.13
  #define LED_BLUE_GPIO                 GPIOA
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_7  // PA.07
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TX12)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOB
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_1  // PB.01
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_5  // PC.05
  #define LED_BLUE_GPIO                 GPIOC
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_4  // PC.04
#elif defined(RADIO_T20)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOG
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_1  // PG.01
  #define LED_RED_GPIO                  GPIOC
  #define LED_RED_GPIO_PIN              GPIO_Pin_5  // PC.05
  #define LED_BLUE_GPIO                 GPIOC
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_4  // PC.04
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              GPIO_SetBits
  #define GPIO_LED_GPIO_OFF             GPIO_ResetBits
  #define LED_GREEN_GPIO                GPIOA
  #define LED_GREEN_GPIO_PIN            GPIO_Pin_7  // PA.07
  #define LED_RED_GPIO                  GPIOE
  #define LED_RED_GPIO_PIN              GPIO_Pin_13 // PE.13
  #define LED_BLUE_GPIO                 GPIOE
  #define LED_BLUE_GPIO_PIN             GPIO_Pin_2  // PE.02
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

#if defined(RADIO_MT12)
  #define LED_STRIP_LENGTH                  7
  #define LED_STRIP_GPIO                    GPIOA
  #define LED_STRIP_GPIO_PIN_DATA           LL_GPIO_PIN_8 // PA.08 / TIM1_CH1
  #define LED_STRIP_GPIO_PIN_AF             LL_GPIO_AF_1   // TIM1 / TIM2
  #define LED_STRIP_TIMER                   TIM1
  #define LED_STRIP_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define LED_STRIP_TIMER_CHANNEL           LL_TIM_CHANNEL_CH1
  #define LED_STRIP_TIMER_DMA               DMA2
  #define LED_STRIP_TIMER_DMA_CHANNEL       LL_DMA_CHANNEL_6
  #define LED_STRIP_TIMER_DMA_STREAM        LL_DMA_STREAM_5
  #define LED_STRIP_TIMER_DMA_IRQn          DMA2_Stream5_IRQn
  #define LED_STRIP_TIMER_DMA_IRQHandler    DMA2_Stream5_IRQHandler
  #define LED_STRIP_REFRESH_PERIOD          50  //ms
#endif

#if defined(FUNCTION_SWITCHES)
#if defined(RADIO_T20)
  #define FS_RCC_AHB1Periph             RCC_AHB1Periph_GPIOF
  #define GPIO_FSLED_GPIO_ON            GPIO_SetBits
  #define GPIO_FSLED_GPIO_OFF           GPIO_ResetBits
  #define FSLED_GPIO                    GPIOF
  #define FSLED_GPIO_PIN_1              GPIO_Pin_10
  #define FSLED_GPIO_PIN_2              GPIO_Pin_9
  #define FSLED_GPIO_PIN_3              GPIO_Pin_8
  #define FSLED_GPIO_PIN_4              GPIO_Pin_7
  #define FSLED_GPIO_PIN_5              GPIO_Pin_6
  #define FSLED_GPIO_PIN_6              GPIO_Pin_13
#else
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
#endif

// Internal Module
#if defined(PCBXLITE)
#define EXTERNAL_ANTENNA
#endif
#if defined(PCBXLITE) || defined(PCBX9LITE)
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
    #define INTMODULE_BOOTCMD_DEFAULT      1 // SET
    #define INIT_INTMODULE_BOOTCMD_PIN()   GPIO_SetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #endif
#elif defined(RADIO_X9DP2019)
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
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
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
  #define INTMODULE_BOOTCMD_DEFAULT     0 // RESET
  #define INIT_INTMODULE_BOOTCMD_PIN()  GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
#elif defined(PCBX9E) || defined(PCBX9DP) || defined(RADIO_X7)
  #define INTMODULE_PULSES
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
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
#elif (defined(RADIO_FAMILY_JUMPER_T12) && defined(HARDWARE_INTERNAL_MODULE)) || defined(RADIO_TX12) || defined(RADIO_T8) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_T20)
  #define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA1)
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
  #define INTMODULE_RX_DMA                DMA1
  #define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_1
  #define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_4
  #if defined(RADIO_TLITE)
    #define INTMODULE_BOOTCMD_GPIO          GPIOA
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_5  // PA.05
    #define INTMODULE_BOOTCMD_DEFAULT       0 // RESET
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
    #define INTMODULE_BOOTCMD_GPIO          GPIOF
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_11  // PF.11
    #define INTMODULE_BOOTCMD_DEFAULT       0 // RESET
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #elif defined(RADIO_T20)
    #define INTMODULE_BOOTCMD_GPIO          GPIOE
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_4   // PE.04
    #define INTMODULE_BOOTCMD_DEFAULT       0 // RESET
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #elif defined(RADIO_LR3PRO)
    #define INTMODULE_BOOTCMD_GPIO          GPIOB
    #define INTMODULE_BOOTCMD_GPIO_PIN      GPIO_Pin_5  // PB.05
    #define INTMODULE_BOOTCMD_DEFAULT       0 // RESET
    #define INIT_INTMODULE_BOOTCMD_PIN()    GPIO_ResetBits(INTMODULE_BOOTCMD_GPIO, INTMODULE_BOOTCMD_GPIO_PIN);
  #endif
#elif defined(RADIO_COMMANDO8)
  #define INTMODULE_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_DMA1)
  #define INTMODULE_RCC_APB1Periph        (RCC_APB1Periph_USART3 | RCC_APB1Periph_TIM2)
  #define INTMODULE_RCC_APB2Periph        0
  #define INTMODULE_PWR_GPIO              GPIOE
  #define INTMODULE_PWR_GPIO_PIN          GPIO_Pin_8  // PE.08
  #define INTMODULE_GPIO                  GPIOB
  #define INTMODULE_TX_GPIO_PIN           GPIO_Pin_10  // PB.10
  #define INTMODULE_RX_GPIO_PIN           GPIO_Pin_11  // PB.11
  #define INTMODULE_GPIO_PinSource_TX     GPIO_PinSource10
  #define INTMODULE_GPIO_PinSource_RX     GPIO_PinSource11
  #define INTMODULE_USART                 USART3
  #define INTMODULE_GPIO_AF               GPIO_AF_USART3
  #define INTMODULE_GPIO_AF_LL            LL_GPIO_AF_7
  #define INTMODULE_USART_IRQn            USART3_IRQn
  #define INTMODULE_USART_IRQHandler      USART3_IRQHandler
  // // DMA1_Stream3 is already used by SDIO
  // #define INTMODULE_DMA                   NULL
  // #define INTMODULE_DMA_CHANNEL           0
  // #define INTMODULE_DMA_STREAM            0
#else
  #define INTMODULE_PULSES
  #define INTMODULE_RCC_AHB1Periph      (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
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
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_X9DP2019) || defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
  #if defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS) || defined(RADIO_ZORRO)|| defined(RADIO_POCKET) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
    #define EXTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
    #define EXTMODULE_PWR_GPIO          GPIOD
    #define EXTMODULE_PWR_GPIO_PIN      GPIO_Pin_8  // PD.08
  #elif defined(PCBX9LITE)
    #define EXTMODULE_RCC_AHB1Periph    (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
    #define EXTMODULE_PWR_GPIO          GPIOA
    #define EXTMODULE_PWR_GPIO_PIN      GPIO_Pin_8  // PA.08
  #else
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
  #define EXTMODULE_USART_TX_DMA_CHANNEL        LL_DMA_CHANNEL_5
  #define EXTMODULE_USART_TX_DMA_STREAM         DMA2_Stream6
  #define EXTMODULE_USART_TX_DMA_STREAM_LL      LL_DMA_STREAM_6
  #define EXTMODULE_USART_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5
  #define EXTMODULE_USART_RX_DMA_STREAM         DMA2_Stream1
  #define EXTMODULE_USART_RX_DMA_STREAM_LL      LL_DMA_STREAM_1
#else
  #define EXTMODULE_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #if defined(RADIO_COMMANDO8)
    #define EXTMODULE_RF_SWITCH_GPIO              GPIOE              //Antenna switching port
    #define EXTMODULE_RF_SWITCH_GPIO_PIN          GPIO_Pin_7         //Antenna switching pin      //PE.07
    #define EXTMODULE_PWR_GPIO                    GPIOE              //External tuner power port  //PE.02
    #define EXTMODULE_PWR_GPIO_PIN                GPIO_Pin_2         //External tuner power pin
    #define EXTERNAL_MODULE_PWR_ON()              GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN|EXTMODULE_RF_SWITCH_GPIO_PIN)
    #define EXTERNAL_MODULE_PWR_OFF()             GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN|EXTMODULE_RF_SWITCH_GPIO_PIN)
  #elif defined(RADIO_T20)
    #define EXTMODULE_PWR_GPIO                    GPIOB
    #define EXTMODULE_PWR_GPIO_PIN                GPIO_Pin_1 // PB.01
    #define EXTERNAL_MODULE_PWR_ON()              GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
    #define EXTERNAL_MODULE_PWR_OFF()             GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #else
    #define EXTMODULE_PWR_GPIO                    GPIOD
    #define EXTMODULE_PWR_GPIO_PIN                GPIO_Pin_8 // PD.08
    #define EXTERNAL_MODULE_PWR_ON()              GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
    #define EXTERNAL_MODULE_PWR_OFF()             GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #endif
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
#if defined(PCBXLITES) || defined(PCBX9LITE)
  // on these 2 radios the trainer port already uses DMA1_Stream6, we won't use the DMA
  #define TRAINER_GPIO                  GPIOD
  #define TRAINER_IN_GPIO_PIN           LL_GPIO_PIN_13 // PD.13
  #define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH2
  #define TRAINER_OUT_GPIO_PIN          LL_GPIO_PIN_12 // PD.12
  #define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH1
#if defined(PCBX9LITE)
  #define TRAINER_DETECT_GPIO           GPIOD
  #define TRAINER_DETECT_GPIO_PIN       LL_GPIO_PIN_11 // PD.11
#endif
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_GPIO_AF               GPIO_AF_TIM4 // TIM4_CH1 (Out) + TIM4_CH2 (In)
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
  #define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#elif defined(PCBXLITE)
  #define TRAINER_TIMER                 TIM4
  #define TRAINER_TIMER_IRQn            TIM4_IRQn
  #define TRAINER_TIMER_IRQHandler      TIM4_IRQHandler
#else
  #define TRAINER_GPIO                  GPIOC
  #define TRAINER_IN_GPIO_PIN           LL_GPIO_PIN_8  // PC.08
  #define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH3
  #define TRAINER_OUT_GPIO_PIN          LL_GPIO_PIN_9  // PC.09
  #define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH4
#if defined(RADIO_T20)
  #define TRAINER_DETECT_GPIO           GPIOE
  #define TRAINER_DETECT_GPIO_PIN       LL_GPIO_PIN_8  // PE.03
#elif defined(RADIO_MT12)
  #define TRAINER_DETECT_GPIO           GPIOD
  #define TRAINER_DETECT_GPIO_PIN       LL_GPIO_PIN_14 // PD.14
#else
  #define TRAINER_DETECT_GPIO           GPIOA
  #define TRAINER_DETECT_GPIO_PIN       LL_GPIO_PIN_8  // PA.08
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
#else
  #define HARDWARE_TRAINER_AUX_SERIAL
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
  #define TELEMETRY_DIR_OUTPUT()          LL_GPIO_ResetOutputPin(TELEMETRY_DIR_GPIO, TELEMETRY_DIR_GPIO_PIN)
  #define TELEMETRY_DIR_INPUT()           LL_GPIO_SetOutputPin(TELEMETRY_DIR_GPIO, TELEMETRY_DIR_GPIO_PIN)
  #define TELEMETRY_SET_INPUT           1
#else
  #define TELEMETRY_DIR_OUTPUT()          LL_GPIO_SetOutputPin(TELEMETRY_DIR_GPIO, TELEMETRY_DIR_GPIO_PIN)
  #define TELEMETRY_DIR_INPUT()           LL_GPIO_ResetOutputPin(TELEMETRY_DIR_GPIO, TELEMETRY_DIR_GPIO_PIN)
  #define TELEMETRY_SET_INPUT           0
#endif
#define TELEMETRY_GPIO                  GPIOD
#define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_5  // PD.05
#define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_6  // PD.06
#define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource5
#define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource6
#define TELEMETRY_GPIO_AF               GPIO_AF_USART2
#define TELEMETRY_USART                 USART2
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_6
#define TELEMETRY_DMA_Channel_TX        DMA_Channel_4
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
#define TELEMETRY_TIMER_PRESCALER       ()
#define TELEMETRY_TIMER_IRQn            TIM1_TRG_COM_TIM11_IRQn
#define TELEMETRY_TIMER_IRQHandler      TIM1_TRG_COM_TIM11_IRQHandler

// Software IRQ (Prio 5 -> FreeRTOS compatible)
#define TELEMETRY_RX_FRAME_EXTI_LINE    LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority 5

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
#define INTMODULE_HEARTBEAT_TRIGGER               LL_EXTI_TRIGGER_FALLING
#if !defined(HARDWARE_EXTERNAL_MODULE)
  // No heartbeat
#elif defined(PCBXLITE)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOD
  #define INTMODULE_HEARTBEAT_GPIO                GPIOD
  #define INTMODULE_HEARTBEAT_GPIO_PIN            LL_GPIO_PIN_15  // PD.15
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTD
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE15
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_15
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI15_10_IRQ)
    #define USE_EXTI15_10_IRQ
    #define EXTI15_10_IRQ_Priority 5
  #endif
#elif defined(PCBX9LITE)
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      0
#elif defined(RADIO_X7ACCESS)
  #define INTMODULE_HEARTBEAT
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOA
  #define INTMODULE_HEARTBEAT_GPIO                GPIOA
  #define INTMODULE_HEARTBEAT_GPIO_PIN            LL_GPIO_PIN_7  // PA.07
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
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOB
  #define INTMODULE_HEARTBEAT_GPIO                GPIOB
  #define INTMODULE_HEARTBEAT_GPIO_PIN            LL_GPIO_PIN_1  // PB.01
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
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      RCC_AHB1Periph_GPIOC
  #define INTMODULE_HEARTBEAT_GPIO                GPIOC
  #define INTMODULE_HEARTBEAT_GPIO_PIN            LL_GPIO_PIN_7  // PC.07
  #define INTMODULE_HEARTBEAT_EXTI_PORT           LL_SYSCFG_EXTI_PORTC
  #define INTMODULE_HEARTBEAT_EXTI_SYS_LINE       LL_SYSCFG_EXTI_LINE7
  #define INTMODULE_HEARTBEAT_EXTI_LINE           LL_EXTI_LINE_7
  // INTMODULE_HEARTBEAT_EXTI IRQ
  #if !defined(USE_EXTI9_5_IRQ)
    #define USE_EXTI9_5_IRQ
    #define EXTI9_5_IRQ_Priority 5
  #endif
#else
  #define INTMODULE_HEARTBEAT_RCC_AHB1Periph      0
#endif

// Trainer / Trainee from the module bay
#if defined(PCBX9LITE) || defined(PCBXLITE) || defined(RADIO_X9DP2019) || \
  defined(PCBX7ACCESS) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || \
  defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12)
  #define TRAINER_MODULE_CPPM_TIMER            TIM3
  #define TRAINER_MODULE_CPPM_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO             EXTMODULE_USART_GPIO
  #define TRAINER_MODULE_CPPM_GPIO_PIN         EXTMODULE_RX_GPIO_PIN
  #define TRAINER_MODULE_CPPM_TIMER_Channel    LL_TIM_CHANNEL_CH2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn       TIM3_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF          GPIO_AF_TIM3
#if defined(PCBX9LITE) ||  defined(PCBXLITE)
  #define TRAINER_MODULE_CPPM_TIMER_IRQHandler TIM3_IRQHandler
#endif
#elif defined(INTMODULE_HEARTBEAT_GPIO) && defined(HARDWARE_EXTERNAL_MODULE)
  // Trainer CPPM input on heartbeat pin
  #define TRAINER_MODULE_CPPM_TIMER               TRAINER_TIMER
  #define TRAINER_MODULE_CPPM_FREQ                (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_CPPM_GPIO_PIN            INTMODULE_HEARTBEAT_GPIO_PIN
  #define TRAINER_MODULE_CPPM_TIMER_Channel       LL_TIM_CHANNEL_CH2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn          TRAINER_TIMER_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF             GPIO_AF_TIM3
  // Trainer SBUS input on heartbeat pin
  #define TRAINER_MODULE_SBUS_USART               USART6
  #define TRAINER_MODULE_SBUS_GPIO_AF             GPIO_AF_USART6
  #define TRAINER_MODULE_SBUS_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_SBUS_GPIO_PIN            INTMODULE_HEARTBEAT_GPIO_PIN
  #define TRAINER_MODULE_SBUS_DMA                 DMA2
  #define TRAINER_MODULE_SBUS_DMA_STREAM          DMA2_Stream1
  #define TRAINER_MODULE_SBUS_DMA_STREAM_LL       LL_DMA_STREAM_1
  #define TRAINER_MODULE_SBUS_DMA_CHANNEL         DMA_Channel_5
#else
  // TODO: replace SBUS trainer with S.PORT pin
#endif

// USB
#define USB_RCC_AHB1Periph_GPIO         RCC_AHB1Periph_GPIOA
#define USB_GPIO                        GPIOA
#define USB_GPIO_PIN_VBUS               LL_GPIO_PIN_9  // PA.09
#define USB_GPIO_PIN_DM                 LL_GPIO_PIN_11 // PA.11
#define USB_GPIO_PIN_DP                 LL_GPIO_PIN_12 // PA.12

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
#elif defined(RADIO_T8) || defined(RADIO_TPROV2) || defined(RADIO_T20)
  #define BACKLIGHT_RCC_AHB1Periph      0
  #define BACKLIGHT_RCC_APB1Periph      0
  #define BACKLIGHT_RCC_APB2Periph      0
#elif defined(RADIO_COMMANDO8)
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
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_T20) || defined(RADIO_MT12)
  #define LCD_VERTICAL_INVERT
#endif
#if defined(RADIO_LR3PRO) || defined(RADIO_TPROV2) || defined(RADIO_T20)
  #define LCD_HORIZONTAL_INVERT
  #define OLED_SCREEN
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
#if defined(RADIO_T20)
  #define LCD_RST_GPIO                  GPIOA
  #define LCD_RST_GPIO_PIN              GPIO_Pin_14 // PA.14
#else
  #define LCD_RST_GPIO                  GPIOD
  #define LCD_RST_GPIO_PIN              GPIO_Pin_12 // PD.12
#endif
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
    defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_X9DP2019)
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
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER)
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
#if defined(RADIO_T20)
  // Using chip, so no detect
#else
#define SD_PRESENT_GPIO                 GPIOD
#if defined(PCBXLITE) || defined(PCBX9LITE)
  #define SD_PRESENT_GPIO_PIN           LL_GPIO_PIN_10 // PD.10
#elif defined(RADIO_COMMANDO8)
  #define SD_PRESENT_GPIO_PIN           LL_GPIO_PIN_8  // PD.08
#else
  #define SD_PRESENT_GPIO_PIN           LL_GPIO_PIN_9  // PD.09
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
#elif defined(RADIO_T20)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_MUTE_GPIO               GPIOG
  #define AUDIO_MUTE_GPIO_PIN           GPIO_Pin_4  // PG.04
  #define AUDIO_MUTE_DELAY              500  // ms
  #define AUDIO_UNMUTE_DELAY            150  // ms
#elif defined(RADIO_COMMANDO8)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_MUTE_GPIO               GPIOB
  #define AUDIO_MUTE_GPIO_PIN           GPIO_Pin_1  // PB.01
  #define AUDIO_MUTE_DELAY              500  // ms
  #define INVERTED_MUTE_PIN
#elif defined(MANUFACTURER_RADIOMASTER) || defined(MANUFACTURER_JUMPER) || defined(RADIO_LR3PRO)
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_MUTE_GPIO               GPIOE
  #define AUDIO_MUTE_GPIO_PIN           GPIO_Pin_12  // PE.12
  #define AUDIO_MUTE_DELAY              500  // ms
#if defined(MANUFACTURER_JUMPER) || defined(RADIO_LR3PRO)
  #define AUDIO_UNMUTE_DELAY            250  // ms
#else
  #define AUDIO_UNMUTE_DELAY            150  // ms
#endif
#else
  #define AUDIO_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
#endif

// Haptic
#if defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER) || defined(RADIO_MT12)
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

#if defined(RADIO_BOXER) || defined(RADIO_T20) || defined(RADIO_X9DP2019)
  // Flysky Hall Stick
  #define FLYSKY_HALL_SERIAL_USART                 UART4
  #define FLYSKY_HALL_SERIAL_GPIO                  GPIOA
  #define FLYSKY_HALL_DMA_Channel                  LL_DMA_CHANNEL_4
  #define FLYSKY_HALL_SERIAL_TX_GPIO_PIN           LL_GPIO_PIN_0  // PA.00
  #define FLYSKY_HALL_SERIAL_RX_GPIO_PIN           LL_GPIO_PIN_1  // PA.01
  #define FLYSKY_HALL_SERIAL_GPIO_AF               LL_GPIO_AF_8

  #define FLYSKY_HALL_RCC_AHB1Periph               RCC_AHB1Periph_DMA1
  #define FLYSKY_HALL_RCC_APB1Periph               RCC_APB1Periph_UART4

  #define FLYSKY_HALL_SERIAL_USART_IRQHandler      UART4_IRQHandler
  #define FLYSKY_HALL_SERIAL_USART_IRQn            UART4_IRQn
  #define FLYSKY_HALL_SERIAL_DMA                   DMA1
  #define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
  #define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
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
  #define BT_GPIO_AF                   LL_GPIO_AF_8
  #define BT_USART_IRQn                USART6_IRQn
  #define BT_RCC_AHB1Periph            (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG)
  #define BT_RCC_APB1Periph            0
  #define BT_RCC_APB2Periph            RCC_APB2Periph_USART6
  #define BT_USART_GPIO                GPIOG
  #define BT_TX_GPIO_PIN               LL_GPIO_PIN_14 // PG.14
  #define BT_RX_GPIO_PIN               LL_GPIO_PIN_9  // PG.09
  #define BT_EN_GPIO                   GPIOD
  #define BT_EN_GPIO_PIN               LL_GPIO_PIN_11 // PD.11
  // #define BT_BRTS_GPIO                 GPIOE
  // #define BT_BRTS_GPIO_PIN             GPIO_Pin_12 // PE.12
  // #define BT_BCTS_GPIO                 GPIOG
  // #define BT_BCTS_GPIO_PIN             GPIO_Pin_6  // PG.06
  #define BT_TX_GPIO_PinSource         GPIO_PinSource14
  #define BT_RX_GPIO_PinSource         GPIO_PinSource9
  #define BT_USART_IRQHandler          USART6_IRQHandler
#elif defined(BLUETOOTH)
  #define STORAGE_BLUETOOTH
  #define BT_RCC_APB1Periph             RCC_APB1Periph_USART3
  #define BT_RCC_APB2Periph             0
  #if defined(PCBX9DP)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOB
    #define BT_EN_GPIO_PIN              LL_GPIO_PIN_2  // PB.02
  #elif defined(PCBXLITE)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOE
    #define BT_EN_GPIO_PIN              LL_GPIO_PIN_15 // PE.15
  #elif defined(PCBX9LITES)
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOD
    #define BT_EN_GPIO_PIN              LL_GPIO_PIN_14 // PD.14
  #elif defined(MANUFACTURER_RADIOMASTER)
    #if defined(RADIO_POCKET)
      #define BT_EN_GPIO                  GPIOA
      #define BT_EN_GPIO_PIN              LL_GPIO_PIN_6  // PA.06
      #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA) // RCC_AHB1Periph_DMA1
    #else
      // No enable port on those RM BLE port
      #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB)
    #endif
  #else
    #define BT_RCC_AHB1Periph           (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE) // RCC_AHB1Periph_DMA1
    #define BT_EN_GPIO                  GPIOE
    #define BT_EN_GPIO_PIN              LL_GPIO_PIN_12 // PE.12
  #endif
  #define BT_USART_GPIO                 GPIOB
  #define BT_TX_GPIO_PIN                LL_GPIO_PIN_10 // PB.10
  #define BT_RX_GPIO_PIN                LL_GPIO_PIN_11 // PB.11
  #define BT_GPIO_AF                    LL_GPIO_AF_7
  #define BT_USART                      USART3
  #define BT_USART_IRQHandler           USART3_IRQHandler
  #define BT_USART_IRQn                 USART3_IRQn
  // #define BT_DMA_Stream_RX              DMA1_Stream1
  // #define BT_DMA_Channel_RX             DMA_Channel_4
#else
  #if defined(PCBX9D) || defined(PCBX9DP) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TX12) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_ZORRO)
    // To avoid change in modelsize, todo: remove me
    #define STORAGE_BLUETOOTH
  #endif
  #define BT_RCC_AHB1Periph             0
  #define BT_RCC_APB1Periph             0
  #define BT_RCC_APB2Periph             0
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

#endif // _HAL_H_
