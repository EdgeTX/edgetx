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
#elif defined(PCBXLITE)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_FAMILY_JUMPER_T12) && !defined(RADIO_TPRO) && !defined(RADIO_TPROV2)
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_GX12)
#if defined(RADIO_MT12)
#else
#endif
#if defined(RADIO_TX12)
#else
#endif
#if defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_GX12)
#else
#endif
#elif defined(RADIO_T8)
  #define KEYS_GPIO_REG_BIND            GPIOA
  #define KEYS_GPIO_PIN_BIND            LL_GPIO_PIN_10 // PA.10
#elif defined(RADIO_COMMANDO8)
  #define KEYS_GPIO_REG_BIND            GPIOD
  #define KEYS_GPIO_PIN_BIND            LL_GPIO_PIN_9 // PD.09
#elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_GX12)
#elif defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(PCBX7)
#elif defined(PCBX9LITE)
#elif defined(RADIO_X9DP2019)
#else
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
#elif defined(PCBXLITE)
#elif defined(RADIO_X7ACCESS)
#elif defined(RADIO_FAMILY_T20)
#if defined(RADIO_T20V2)
#else
#endif
#elif defined(RADIO_MT12)
#elif defined(RADIO_GX12)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(PCBX7) && !defined(RADIO_COMMANDO8)
#elif defined(RADIO_COMMANDO8)
#elif defined(PCBX9LITE)
#elif defined(RADIO_X9DP2019)
#else
#endif

// Switches
#if defined(PCBX9E)
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_MT12)
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
#elif defined(RADIO_GX12)
  #define SWITCHES_A_2POS
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_T8)
#elif defined(RADIO_COMMANDO8)
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define STORAGE_SWITCH_A
  #define HARDWARE_SWITCH_A
#elif defined(PCBX7)
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBXLITE)
#elif defined(RADIO_T8)
#elif defined(RADIO_COMMANDO8)
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
#elif (RADIO_LR3PRO)
#elif defined(RADIO_BOXER)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_MT12)
#elif defined(RADIO_GX12)
  #define SWITCHES_B_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
// ADC based switch
#elif defined(PCBX7)
#elif defined(PCBX9LITE)
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBXLITE) || defined(PCBX9LITE)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_TX12)
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
#elif defined(RADIO_T8)
#elif defined(RADIO_COMMANDO8)
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_MT12)
#elif defined(RADIO_GX12)
  #define SWITCHES_C_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
// ADC based switch
#elif defined(PCBX7)
  #if defined(PCBX7ACCESS)
  #else
  #endif
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBX9DP)
#elif defined(PCBXLITE)
#elif defined(RADIO_T8)
#elif defined(RADIO_COMMANDO8)
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET)
#elif defined(RADIO_TLITE) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_LR3PRO)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_MT12)
#elif defined(RADIO_GX12)
  #define SWITCHES_D_2POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
  #define STORAGE_SWITCH_D
  #define HARDWARE_SWITCH_D
#elif defined(PCBX7) && !defined(RADIO_COMMANDO8)
#elif defined(PCBX7) && defined(RADIO_COMMANDO8)
#elif defined(PCBX9LITE)
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBXLITES)
#elif defined(PCBXLITE)
  // no SWE but we want to remain compatible with XLiteS
#elif defined(PCBX9LITE)
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET)
#elif defined(RADIO_BOXER)
#elif defined(RADIO_TX12)
#elif defined(RADIO_TX12MK2)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_TPRO)
#elif defined(RADIO_TPROV2)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(RADIO_GX12)
  #define SWITCHES_E_3POS
#elif  defined(RADIO_V14) || defined(RADIO_V12)
  // ADC based switch
#elif defined(PCBX7)
  // no SWE
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBXLITES)
#elif defined(PCBXLITE)
  // no SWF but we want to remain compatible with XLiteS
#elif defined(PCBX9LITES)
#elif defined(PCBX9LITE)
// no SWF
#elif defined(RADIO_ZORRO) || defined(RADIO_BOXER)
#elif defined(RADIO_TX12)
#elif defined(RADIO_TX12MK2)
#elif defined(RADIO_TPROS)
#elif defined(RADIO_TPRO)
#elif defined(RADIO_TPROV2)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_T14) || defined(RADIO_T12MAX)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_MT12)
  // ADC based switch/pot
#elif defined(RADIO_T8) || defined(RADIO_TLITE) || defined(RADIO_COMMANDO8) || defined(RADIO_LR3PRO) || defined(RADIO_POCKET)
  // no SWF
#elif defined(RADIO_T12)
  // no SWF
#elif defined(RADIO_GX12)
  #define SWITCHES_F_3POS
#elif defined(RADIO_V14) || defined(RADIO_V12)
  // ADC based switch
#elif defined(PCBX7)
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBX9LITES)
#elif defined(RADIO_T12) || defined(RADIO_ZORRO)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(RADIO_TPROV2)
#elif defined(RADIO_TPRO)
  // SW1
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
  //TRIM left low
#elif defined(RADIO_GX12)
  #define SWITCHES_G_2POS
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)  || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_MT12) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
  // no SWG
#else
#endif

#if defined(PCBX9E)
#elif defined(PCBX9DP)
#elif defined(PCBXLITE) || defined(PCBX9LITE) || defined(RADIO_POCKET)
  // no SWH
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  // no SWH
#elif defined(RADIO_TX12) || defined(RADIO_MT12)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_V14) || defined(RADIO_V12)
#elif defined(RADIO_TPROV2)
#elif defined(RADIO_TPRO)
  // SW2
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
  //TRIM right low
#elif defined(RADIO_TLITE) || defined(RADIO_TPROS) || defined(RADIO_LR3PRO)
  // no SWH
#elif defined(RADIO_GX12)
  #define SWITCHES_H_2POS
#elif defined(PCBX7)
#else
#endif

#if defined(RADIO_X9DP2019)
#elif defined(PCBX9D) || defined(PCBX9DP)
#endif

// X7 P400 P401 headers additionnal momentary switches
#if defined(PCBX7ACCESS)
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  // no SWI/J
#elif defined(RADIO_TX12)
#elif defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_MT12) || defined(RADIO_V14) || defined(RADIO_V12)
  // no headers
#elif defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  // no SWI
  // no SWJ
#elif defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_T14) || defined(RADIO_T12MAX) || defined(RADIO_TPROS)
// No I/J
#elif defined(RADIO_TPROV2)
  // SW1
  // SW2
  // SW3
  // SW4
  // SW5
  // SW6
#elif defined(RADIO_BUMBLEBEE)
  // SW1
  // SW2
  // SW3
  // SW4
  // SW5
  // SW6
#elif defined(RADIO_TPRO)
  // SW3
  // SW4
  // SW5
  // SW6
#elif defined(RADIO_FAMILY_T20)
  //TRIM left up
  //TRIM right up
#elif defined(RADIO_GX12)
  // I and J are part of function switches bellow
#elif defined(PCBX7)
#endif

#if defined(RADIO_FAMILY_T20)
  //SW1
  //SW2
  //SW3
  //SW4
  //SW5
  //SW6
#endif

#if defined(RADIO_GX12)
  //SW1
  //SW2
  //SW3
  //SW4
  //SW5
  //SW6
#endif

#if defined(PCBX9E)
#endif

// 6POS SW
#if defined(RADIO_V14) || defined(RADIO_V12)
  #define SIXPOS_SWITCH_INDEX             6
  #define SIXPOS_LED_RED                200
  #define SIXPOS_LED_GREEN              0
  #define SIXPOS_LED_BLUE               0
#endif

// ADC
#if !defined(RADIO_FAMILY_T20) && !defined(RADIO_BUMBLEBEE)
#endif


#if defined(PCBX9E)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #define HARDWARE_POT4
  #define ADC_VREF_PREC2                300
#elif defined(PCBX9DP)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define HARDWARE_POT3
  #if PCBREV < 2019
    #define ADC_VREF_PREC2              330
  #else
    #define ADC_VREF_PREC2              300
  #endif
#elif defined(PCBXLITE)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define PWM_STICKS
  #define PWM_TIMER                     TIM5
  #define PWM_TIMER_FREQ                (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF2
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_IRQn                      TIM5_IRQn
  #define PWM_GPIOA_PINS                (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH)
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_T8)
  // No pots
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_COMMANDO8)
  #define ADC_VREF_PREC2                320
  #define PWM_STICKS
  #define PWM_TIMER                     TIM5
  #define PWM_TIMER_FREQ                (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define PWM_GPIO                      GPIOA
  #define PWM_GPIO_AF                   GPIO_AF2
  #define PWM_IRQn                      TIM5_IRQn
  #define PWM_IRQHandler                TIM5_IRQHandler
  #define PWM_GPIOA_PINS                (ADC_GPIO_PIN_STICK_RV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV)
#elif defined(RADIO_TLITE) || defined(RADIO_LR3PRO)
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_T14)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_TPROS)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_T12MAX)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_BUMBLEBEE)
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_FAMILY_T20)
  #define ADC_VREF_PREC2                300
#elif defined(RADIO_MT12)
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_GX12)
  // Serial gimbal only
  // Analogs
  #define ADC_VREF_PREC2                330
#elif defined(RADIO_V12) || defined(RADIO_V14)
  #define ADC_VREF_PREC2                330
#elif defined(PCBX7)
  #define HARDWARE_POT1
  #define HARDWARE_POT2
#if !defined(RADIO_POCKET)
#endif
#if defined(RADIO_BOXER) || defined(RADIO_V14) || defined(RADIO_V12)
  #define HARDWARE_POT3                 // 6 POS
  #define ADC_GPIOA_PINS_FS             (ADC_GPIO_PIN_STICK_LH | ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_POT3)
#else
#endif
  #define ADC_VREF_PREC2                330
#elif defined(PCBX9LITE)
  #define HARDWARE_POT1
  #define ADC_VREF_PREC2                300
#else
  #define HARDWARE_POT1
  #define HARDWARE_POT2
  #define ADC_VREF_PREC2                330
#endif

#if defined(PCBX9E)
  #if defined(HORUS_STICKS)
  #else
  #endif // HORUS_STICKS
#elif defined(PCBX9DP)
#elif defined(PCBX9D)
#elif defined(RADIO_TX12)
#elif defined(RADIO_TX12MK2)
#elif defined(RADIO_BOXER)
#elif defined(RADIO_ZORRO)
#elif defined(RADIO_POCKET)
#elif defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS)
#elif defined(RADIO_T14)
#elif defined(RADIO_T12MAX)
#elif defined(RADIO_BUMBLEBEE)
#elif defined(RADIO_FAMILY_T20)
#elif defined(RADIO_MT12)
#elif defined(RADIO_GX12)
#elif defined(RADIO_V14)
#elif defined(RADIO_V12)
#elif defined(PCBX7)
#elif defined(PCBX9LITE)
#elif defined(PCBXLITE)
#endif

#if defined(RADIO_BOXER)
  #define DEFAULT_6POS_CALIB          {5, 13, 22, 31, 40}
  #define DEFAULT_6POS_IDX            6
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
  #define LED_STRIP_LENGTH                  8
  #define BLING_LED_STRIP_START             0
  #define BLING_LED_STRIP_LENGTH            0
  #define CFS_LED_STRIP_START               0
  #define CFS_LED_STRIP_LENGTH              8
  #define CFS_LEDS_PER_SWITCH               1
#else
  #define LED_STRIP_LENGTH                  7
  #define BLING_LED_STRIP_START             0
  #define BLING_LED_STRIP_LENGTH            7
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
  #define BLING_LED_STRIP_START             6
  #define BLING_LED_STRIP_LENGTH            32
#elif defined(RADIO_V12)
  #define LED_STRIP_LENGTH                  6   // 6POS switches only?
#else
  #define LED_STRIP_LENGTH                  1
  #define BLING_LED_STRIP_START             0
  #define BLING_LED_STRIP_LENGTH            1
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

// Switches with LEDs
#if defined(RADIO_V14)
  #define SALED_PWR_GPIO                   GPIO_PIN(GPIOC, 13) // PD.14
  #define SALED_PWR_ON()                   gpio_set(SALED_PWR_GPIO)
  #define SDLED_PWR_GPIO                   GPIO_PIN(GPIOE, 8) // PD.14
  #define SDLED_PWR_ON()                   gpio_set(SDLED_PWR_GPIO)
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
  #define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 5) // PC.05
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
  #define PCBREV_GPIO                   GPIO_PIN(GPIOA, 14) // PA.14
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
#if defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_BOXER) || defined(RADIO_ZORRO) || defined(RADIO_POCKET) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_TPRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_FAMILY_T20) || defined(RADIO_MT12) || defined(RADIO_T12MAX) || defined(RADIO_V12) || defined(RADIO_BUMBLEBEE)
  #define LCD_VERTICAL_INVERT
#endif
#if defined(RADIO_LR3PRO) || defined(RADIO_TPROV2) || defined(RADIO_TPROS) || defined(RADIO_FAMILY_T20) || defined(RADIO_T14) || defined(RADIO_BUMBLEBEE) || defined(RADIO_GX12) || defined(RADIO_V14)
  #define LCD_HORIZONTAL_INVERT
  #define OLED_SCREEN
#endif
#if defined(RADIO_T14) || defined(RADIO_GX12) || defined(RADIO_V14)
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
    defined(RADIO_GX12) || defined(RADIO_V12) || defined(RADIO_V14)
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
  #define SD_PRESENT_GPIO           GPIO_PIN(GPIOD, 10) // PD.10
#elif defined(RADIO_COMMANDO8)
  #define SD_PRESENT_GPIO           GPIO_PIN(GPIOD, 8)  // PD.08
#else
  #define SD_PRESENT_GPIO           GPIO_PIN(GPIOD, 9)  // PD.09
#endif
#endif

#define STORAGE_USE_SDCARD_SPI
#if defined(RADIO_GX12)
#define SD_LONG_BUSY_WAIT               (true)
#endif

#define SD_GPIO_PIN_CS                  GPIO_PIN(GPIOB, 12) // PB.12
#define SD_GPIO_PIN_SCK                 GPIO_PIN(GPIOB, 13) // PB.13
#define SD_GPIO_PIN_MISO                GPIO_PIN(GPIOB, 14) // PB.14
#define SD_GPIO_PIN_MOSI                GPIO_PIN(GPIOB, 15) // PB.15

#define SD_SPI                          SPI2
#define SD_SPI_DMA                      DMA1
#define SD_SPI_DMA_RX_STREAM            LL_DMA_STREAM_3
#define SD_SPI_DMA_TX_STREAM            LL_DMA_STREAM_4
#define SD_SPI_DMA_CHANNEL              LL_DMA_CHANNEL_0

// Audio
#define AUDIO_OUTPUT_GPIO               GPIO_PIN(GPIOA, 4)
#define AUDIO_DMA                       DMA1
#define AUDIO_DMA_Stream                DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn           DMA1_Stream5_IRQn
#define AUDIO_DMA_Stream_IRQHandler     DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                     TIM6

#if defined(PCBXLITES)
  #define JACK_DETECT_GPIO              GPIO_PIN(GPIOC, 13) // PC.13
  #define AUDIO_SPEAKER_ENABLE_GPIO     GPIO_PIN(GPIOD, 14) // PD.14
  #define HEADPHONE_TRAINER_SWITCH_GPIO GPIO_PIN(GPIOD, 9)  // PD.09
#elif defined(RADIO_FAMILY_T20) || defined(RADIO_BUMBLEBEE)
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
    #if defined(RADIO_POCKET) || defined(RADIO_GX12)
      #define BT_EN_GPIO                GPIO_PIN(GPIOA, 6) // PA.06
    #endif
  #else
    #define BT_EN_GPIO                  GPIO_PIN(GPIOE, 12) // PE.12
  #endif
  #if defined(RADIO_GX12)
    #define BT_TX_GPIO                    GPIO_PIN(GPIOD, 8) // PD.08
    #define BT_RX_GPIO                    GPIO_PIN(GPIOD, 9) // PD.09
  #else
    #define BT_TX_GPIO                    GPIO_PIN(GPIOB, 10) // PB.10
    #define BT_RX_GPIO                    GPIO_PIN(GPIOB, 11) // PB.11
  #endif
  #define BT_USART                      USART3
  #define BT_USART_IRQn                 USART3_IRQn
  // #define BT_DMA_Stream_RX              DMA1_Stream1
  // #define BT_DMA_Channel_RX             DMA_Channel_4
#else
  #if defined(PCBX9D) || defined(PCBX9DP) || defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TX12) || defined(RADIO_TX12MK2)|| defined(RADIO_BOXER) || defined(RADIO_GX12) || defined(RADIO_T8) || defined(RADIO_COMMANDO8) || defined(RADIO_ZORRO)
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
