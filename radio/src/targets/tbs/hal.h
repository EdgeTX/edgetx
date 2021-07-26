/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

  #define CPU_FREQ            168000000

  // Keys
  #define KEYS_GPIO_REG_MENU                GPIOD->IDR
  #define KEYS_GPIO_PIN_MENU                GPIO_Pin_13  // PD.13
  #define KEYS_GPIO_REG_EXIT                GPIOD->IDR
  #define KEYS_GPIO_PIN_EXIT                GPIO_Pin_14  // PD.14
  #define KEYS_GPIO_REG_PAGE                GPIOD->IDR
  #define KEYS_GPIO_PIN_PAGE                GPIO_Pin_12  // PD.12
  #define KEYS_GPIO_REG_ENTER               GPIOD->IDR
  #define KEYS_GPIO_PIN_ENTER               GPIO_Pin_4   // PD.4

  // Rotary Encoder
  #define ROTARY_ENCODER_NAVIGATION
  #define ENC_GPIO                          GPIOA
  #define ENC_GPIO_PIN_A                    GPIO_Pin_8    // PA.8
  #define ENC_GPIO_PIN_B                    GPIO_Pin_10   // PA.10
  #define ROTARY_ENCODER_POSITION()         ((ENC_GPIO->IDR >> 9) & 0x02) + ((ENC_GPIO->IDR >> 8) & 0x01)
  #define ROTARY_ENCODER_EXTI_LINE1         EXTI_Line8
  #define ROTARY_ENCODER_EXTI_IRQn1         EXTI9_5_IRQn
  #define ROTARY_ENCODER_EXTI_IRQHandler1   EXTI9_5_IRQHandler
  #define ROTARY_ENCODER_EXTI_PortSource    EXTI_PortSourceGPIOA
  #define ROTARY_ENCODER_EXTI_PinSource1    EXTI_PinSource8
  #if defined(ROTARY_ENCODER_NAVIGATION)
    #define ROTARY_ENCODER_RCC_APB2Periph   RCC_APB2Periph_TIM10
    #define ROTARY_ENCODER_TIMER            TIM10
    #define ROTARY_ENCODER_TIMER_IRQn       TIM1_UP_TIM10_IRQn
    #define ROTARY_ENCODER_TIMER_IRQHandler TIM1_UP_TIM10_IRQHandler
  #else
    #define ROTARY_ENCODER_RCC_APB1Periph   0
  #endif

  // This is for SIMU: reuse rotary encoder pins to map UP and DOWN keyboard keys
  #if defined(SIMU)
    #define KEYS_GPIO_REG_PLUS              ENC_GPIO->IDR
    #define KEYS_GPIO_PIN_PLUS              ENC_GPIO_PIN_A
    #define KEYS_GPIO_REG_MINUS             ENC_GPIO->IDR
    #define KEYS_GPIO_PIN_MINUS             ENC_GPIO_PIN_B
  #endif

  // Trims
  #if defined(SIMU)
    #define TRIMS_GPIO_REG_LHL              GPIOG->IDR
    #define TRIMS_GPIO_PIN_LHL              GPIO_Pin_0  // PG.00
    #define TRIMS_GPIO_REG_LHR              GPIOG->IDR
    #define TRIMS_GPIO_PIN_LHR              GPIO_Pin_1  // PG.01
    #define TRIMS_GPIO_REG_LVD              GPIOG->IDR
    #define TRIMS_GPIO_PIN_LVD              GPIO_Pin_2  // PG.02
    #define TRIMS_GPIO_REG_LVU              GPIOG->IDR
    #define TRIMS_GPIO_PIN_LVU              GPIO_Pin_3  // PG.03
    #define TRIMS_GPIO_REG_RVD              GPIOG->IDR
    #define TRIMS_GPIO_PIN_RVD              GPIO_Pin_4  // PG.04
    #define TRIMS_GPIO_REG_RHL              GPIOG->IDR
    #define TRIMS_GPIO_PIN_RHL              GPIO_Pin_5  // PG.05
    #define TRIMS_GPIO_REG_RVU              GPIOG->IDR
    #define TRIMS_GPIO_PIN_RVU              GPIO_Pin_6  // PG.06
    #define TRIMS_GPIO_REG_RHR              GPIOG->IDR
    #define TRIMS_GPIO_PIN_RHR              GPIO_Pin_7  // PG.07
  #endif

  // Switches
  #if defined(RADIO_TANGO)
    #define STORAGE_SWITCH_A
    #define HARDWARE_SWITCH_A
    #define HARDWARE_SWITCH_A
    #define SWITCHES_GPIO_REG_A             GPIOC->IDR
    #define SWITCHES_GPIO_PIN_A             GPIO_Pin_4 // PC.04

    #define STORAGE_SWITCH_B
    #define HARDWARE_SWITCH_B
    #define SWITCHES_GPIO_REG_B_H           GPIOA->IDR
    #define SWITCHES_GPIO_PIN_B_H           GPIO_Pin_5 // PA.05
    #define SWITCHES_GPIO_REG_B_L           GPIOA->IDR
    #define SWITCHES_GPIO_PIN_B_L           GPIO_Pin_6 // PA.06

    #define STORAGE_SWITCH_C
    #define HARDWARE_SWITCH_C
    #define SWITCHES_GPIO_REG_C_H           GPIOE->IDR
    #define SWITCHES_GPIO_PIN_C_H           GPIO_Pin_3 // PE.03
    #define SWITCHES_GPIO_REG_C_L           GPIOE->IDR
    #define SWITCHES_GPIO_PIN_C_L           GPIO_Pin_2 // PE.02

    #define STORAGE_SWITCH_D
    #define HARDWARE_SWITCH_D
    #define SWITCHES_GPIO_REG_D             GPIOE->IDR
    #define SWITCHES_GPIO_PIN_D             GPIO_Pin_0  // PE.00

    #define STORAGE_SWITCH_E
    #define HARDWARE_SWITCH_E
    #define SWITCHES_GPIO_REG_E             GPIOE->IDR
    #define SWITCHES_GPIO_PIN_E             GPIO_Pin_1  // PE.01

    #define STORAGE_SWITCH_F
    #define HARDWARE_SWITCH_F
    #define SWITCHES_GPIO_REG_F             GPIOA->IDR
    #define SWITCHES_GPIO_PIN_F             GPIO_Pin_7 // PA.07

    #define KEYS_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG)
    #define KEYS_GPIOA_PINS                 (SWITCHES_GPIO_PIN_F | SWITCHES_GPIO_PIN_B_H | SWITCHES_GPIO_PIN_B_L | ENC_GPIO_PIN_A | ENC_GPIO_PIN_B)
    #define KEYS_GPIOB_PINS                 (0)
    #define KEYS_GPIOC_PINS                 (SWITCHES_GPIO_PIN_A)
    #define KEYS_GPIOD_PINS                 (KEYS_GPIO_PIN_ENTER | KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_PAGE | KEYS_GPIO_PIN_EXIT)
    #define KEYS_GPIOE_PINS                 (SWITCHES_GPIO_PIN_C_H | SWITCHES_GPIO_PIN_C_L | SWITCHES_GPIO_PIN_D | SWITCHES_GPIO_PIN_E)
    #define KEYS_GPIOF_PINS                 (0)
  #elif defined(RADIO_MAMBO)
    #define HARDWARE_SWITCH_A

    #define HARDWARE_SWITCH_B

    #define HARDWARE_SWITCH_C

    #define HARDWARE_SWITCH_D

    #define HARDWARE_SWITCH_E
    #define SWITCHES_GPIO_REG_E             GPIOE->IDR
    #define SWITCHES_GPIO_PIN_E             GPIO_Pin_5  // PE.05

    #define HARDWARE_SWITCH_F
    #define SWITCHES_GPIO_REG_F             GPIOE->IDR
    #define SWITCHES_GPIO_PIN_F             GPIO_Pin_4  // PE.04

    #define KEYS_RCC_AHB1Periph             (RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_GPIOG)
    #define KEYS_GPIOA_PINS                 (ENC_GPIO_PIN_A | ENC_GPIO_PIN_B)
    #define KEYS_GPIOB_PINS                 (0)
    #define KEYS_GPIOC_PINS                 (0)
    #define KEYS_GPIOD_PINS                 (KEYS_GPIO_PIN_ENTER | KEYS_GPIO_PIN_MENU | KEYS_GPIO_PIN_PAGE | KEYS_GPIO_PIN_EXIT)
    #define KEYS_GPIOE_PINS                 (SWITCHES_GPIO_PIN_E | SWITCHES_GPIO_PIN_F)
    #define KEYS_GPIOF_PINS                 (0)
  #endif

  // ADC
  #if defined(RADIO_TANGO)
    #define ADC_MAIN                        ADC1
    #define ADC_DMA                         DMA2
    #define ADC_DMA_Channel                 DMA_Channel_0
    #define ADC_DMA_Stream                  DMA2_Stream4
    #define ADC_DMA_TC_Flag                 DMA_FLAG_TCIF4
    #define ADC_SAMPTIME                    2   // sample time = 28 cycles
    #define ADC_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA2)
    #define ADC_RCC_APB2Periph              RCC_APB2Periph_ADC1
    #define ADC_GPIO_PIN_BATT               GPIO_Pin_1  // PB.01
    #define ADC_GPIOB_PINS                  ADC_GPIO_PIN_BATT
    #define ADC_CHANNEL_BATT                ADC_Channel_9  // ADC1_IN9
    #define ADC_VREF_PREC2                  307
  #elif defined(RADIO_MAMBO)
    #define ADC_MAIN                        ADC1
    #define ADC_DMA                         DMA2
    #define ADC_DMA_Channel                 DMA_Channel_0
    #define ADC_DMA_Stream                  DMA2_Stream4
    #define ADC_SET_DMA_FLAGS()             ADC_DMA->HIFCR = (DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4)
    #define ADC_TRANSFER_COMPLETE()         (ADC_DMA->HISR & DMA_HISR_TCIF4)
    #define ADC_SAMPTIME                    2   // sample time = 28 cycles
    #define ADC_VREF_PREC2                  300

    #define HARDWARE_POT1
    #define HARDWARE_POT2
    #define ADC_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2)
    #define ADC_RCC_APB2Periph              RCC_APB2Periph_ADC1

    #define ADC_GPIO_PIN_SWITCH_A           GPIO_Pin_6  // PA.06
    #define ADC_GPIO_PIN_SWITCH_B           GPIO_Pin_7  // PA.07
    #define ADC_GPIO_PIN_SWITCH_C           GPIO_Pin_5  // PA.05
    #define ADC_GPIO_PIN_SWITCH_D           GPIO_Pin_3  // PA.03
    #define ADC_GPIO_PIN_POT1               GPIO_Pin_4  // PC.04
    #define ADC_GPIO_PIN_POT2               GPIO_Pin_2  // PA.02
    #define ADC_GPIO_PIN_TRIM               GPIO_Pin_1  // PC.01

    #define ADC_GPIO_PIN_BATT               GPIO_Pin_1  // PB.01
    #define ADC_GPIOA_PINS                  (ADC_GPIO_PIN_SWITCH_A | ADC_GPIO_PIN_SWITCH_B | ADC_GPIO_PIN_SWITCH_C | ADC_GPIO_PIN_SWITCH_D | ADC_GPIO_PIN_POT2)
    #define ADC_GPIOB_PINS                  (ADC_GPIO_PIN_BATT)
    #define ADC_GPIOC_PINS                  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_TRIM)

    #define ADC_CHANNEL_BATT                ADC_Channel_9   // ADC1_IN9
    #define ADC_CHANNEL_SWITCH_A            ADC_Channel_6   // ADC1_IN6
    #define ADC_CHANNEL_SWITCH_B            ADC_Channel_7   // ADC1_IN7
    #define ADC_CHANNEL_SWITCH_C            ADC_Channel_5   // ADC1_IN5
    #define ADC_CHANNEL_SWITCH_D            ADC_Channel_3   // ADC1_IN3
    #define ADC_CHANNEL_POT1                ADC_Channel_14  // ADC1_IN14
    #define ADC_CHANNEL_POT2                ADC_Channel_2   // ADC1_IN2
    #define ADC_CHANNEL_TRIM                ADC_Channel_11  // ADC1_IN11
  #endif

  #define ADC_MAIN_SMPR1                    (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24);
  #define ADC_MAIN_SMPR2                    (ADC_SAMPTIME << 0) + (ADC_SAMPTIME << 3) + (ADC_SAMPTIME << 6) + (ADC_SAMPTIME << 9) + (ADC_SAMPTIME << 12) + (ADC_SAMPTIME << 15) + (ADC_SAMPTIME << 18) + (ADC_SAMPTIME << 21) + (ADC_SAMPTIME << 24) + (ADC_SAMPTIME << 27);

  // PWR driver
  #define PWR_RCC_AHB1Periph                (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE)
  #define PWR_SWITCH_GPIO                   GPIOB
  #define PWR_SWITCH_GPIO_PIN               GPIO_Pin_14 // PB.14
  #define PWR_SWITCH_PIN_ACTIVE_HIGH
  #define PWR_ON_GPIO                       GPIOB
  #define PWR_ON_GPIO_PIN                   GPIO_Pin_12 // PB.12

  // Charger
  #if defined(RADIO_TANGO)
    #define CHARGER_STATE_GPIO              GPIOD
    #define CHARGER_STATE_GPIO_PIN          GPIO_Pin_10 // PD.10
    #define CHARGER_FAULT_GPIO              GPIOD
    #define CHARGER_FAULT_GPIO_PIN          GPIO_Pin_11 // PD.11
  #elif defined(RADIO_MAMBO)
    #define CHARGER_STATE_GPIO              GPIOD
    #define CHARGER_STATE_GPIO_PIN          GPIO_Pin_11 // PD.11
    #define CHARGER_FAULT_GPIO              0
    #define CHARGER_FAULT_GPIO_PIN          0
  #endif

  // External Module
  #define EXTMODULE_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_DMA2)
  #define EXTMODULE_RCC_APB2Periph          (RCC_APB2Periph_TIM8 | RCC_APB2Periph_USART6)
  #define EXTMODULE_PWR_GPIO                GPIOE
  #if defined(RADIO_TANGO)
    #define EXTMODULE_PWR_GPIO_PIN          GPIO_Pin_5 // PE.05
  #elif defined(RADIO_MAMBO)
    #define EXTMODULE_PWR_GPIO_PIN          GPIO_Pin_2 // PE.02
  #endif
  #define EXTERNAL_MODULE_PWR_ON()          GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define EXTERNAL_MODULE_PWR_OFF()         GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
  #define EXTMODULE_TX_GPIO                 GPIOC
  #define EXTMODULE_USART_GPIO              EXTMODULE_TX_GPIO
  #define EXTMODULE_TX_GPIO_PIN             GPIO_Pin_6  // PC.06
  #define EXTMODULE_TX_GPIO_PinSource       GPIO_PinSource6
  #define EXTMODULE_RX_GPIO_PIN             GPIO_Pin_7  // PC.07
  #define EXTMODULE_RX_GPIO_PinSource       GPIO_PinSource7
  #define EXTMODULE_TIMER                   TIM8
  #define EXTMODULE_TIMER_FREQ              (PERI2_FREQUENCY * TIMER_MULT_APB2)
  #define EXTMODULE_TIMER_CC_IRQn           TIM8_CC_IRQn
  #define EXTMODULE_TIMER_CC_IRQHandler     TIM8_CC_IRQHandler
  #define EXTMODULE_TIMER_TX_GPIO_AF        GPIO_AF_TIM8
  #define EXTMODULE_TIMER_DMA_CHANNEL       DMA_Channel_7
  #define EXTMODULE_TIMER_DMA_STREAM        DMA2_Stream1
  #define EXTMODULE_TIMER_DMA_STREAM_IRQn   DMA2_Stream1_IRQn
  #define EXTMODULE_TIMER_DMA_STREAM_IRQHandler DMA2_Stream1_IRQHandler
  #define EXTMODULE_TIMER_DMA_FLAG_TC       DMA_IT_TCIF1
  #define EXTMODULE_TIMER_OUTPUT_ENABLE     TIM_CCER_CC1E
  #define EXTMODULE_TIMER_OUTPUT_POLARITY   TIM_CCER_CC1P
  #define EXTMODULE_USART_GPIO_AF           GPIO_AF_USART6
  #define EXTMODULE_USART                   USART6
  #define EXTMODULE_USART_IRQn              USART6_IRQn
  #define EXTMODULE_USART_IRQHandler        USART6_IRQHandler
  #define EXTMODULE_USART_TX_DMA_CHANNEL    DMA_Channel_5
  #define EXTMODULE_USART_TX_DMA_STREAM     DMA2_Stream6
  #define EXTMODULE_USART_RX_DMA_CHANNEL    DMA_Channel_5
  #define EXTMODULE_USART_RX_DMA_STREAM     DMA2_Stream1

  // Led
  #if defined(RADIO_TANGO)
    #define LED_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_DMA1)
    #define LED_RCC_APB1Periph              RCC_APB1Periph_TIM2
    #define LED_GPIO                        GPIOB
    #define LED_GPIO_PIN                    GPIO_Pin_11  // PB.11
    #define LED_GPIO_PinSource              GPIO_PinSource11
    #define LED_TIMER                       TIM2
    #define LED_GPIO_AF                     GPIO_AF_TIM2
    #define LED_DMA_STREAM                  DMA1_Stream6
    #define LED_DMA_CHANNEL                 DMA_Channel_3
    #define LED_DMA_FLAG_TC                 DMA_FLAG_TCIF6
    #define LED_DMA_FLAG_ERRORS             (DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6)
  #endif

  // Serial Port
  #define AUX_SERIAL_RCC_AHB1Periph         (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
  #define AUX_SERIAL_RCC_APB1Periph         RCC_APB1Periph_USART3
  #define AUX_SERIAL_GPIO                   GPIOD
  #define AUX_SERIAL_GPIO_PIN_TX            GPIO_Pin_8 // PD.08
  #define AUX_SERIAL_GPIO_PIN_RX            GPIO_Pin_9 // PD.09
  #define AUX_SERIAL_GPIO_PinSource_TX      GPIO_PinSource8
  #define AUX_SERIAL_GPIO_PinSource_RX      GPIO_PinSource9
  #define AUX_SERIAL_GPIO_AF                GPIO_AF_USART3
  #define AUX_SERIAL_USART                  USART3
  #define AUX_SERIAL_USART_IRQHandler       USART3_IRQHandler
  #define AUX_SERIAL_USART_IRQn             USART3_IRQn
  #define AUX_SERIAL_DMA_Stream_RX          DMA1_Stream1
  #define AUX_SERIAL_DMA_Channel_RX         DMA_Channel_4

  // Telemetry
  #if defined(RADIO_TANGO)
    #define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_DMA1)
    #define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART3
    #define TELEMETRY_DIR_GPIO              GPIOE
    #define TELEMETRY_DIR_GPIO_PIN          GPIO_Pin_4  // PE.04
    #define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
    #define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
    #define TELEMETRY_RX_POLARITY_GPIO      GPIOC
    #define TELEMETRY_RX_POLARITY_PIN       GPIO_Pin_7  // PC.07
    #define TELEMETRY_RX_POLARITY_INVERT()  TELEMETRY_RX_POLARITY_GPIO->BSRRL = TELEMETRY_RX_POLARITY_PIN
    #define TELEMETRY_RX_POLARITY_NORMAL()  TELEMETRY_RX_POLARITY_GPIO->BSRRH = TELEMETRY_RX_POLARITY_PIN

    #define TELEMETRY_GPIO                  GPIOD
    #define TELEMETRY_TX_GPIO_PIN           GPIO_Pin_8  // PD.08
    #define TELEMETRY_RX_GPIO_PIN           GPIO_Pin_9  // PD.09
    #define TELEMETRY_GPIO_PinSource_TX     GPIO_PinSource8
    #define TELEMETRY_GPIO_PinSource_RX     GPIO_PinSource9
    #define TELEMETRY_GPIO_AF               GPIO_AF_USART3
    #define TELEMETRY_USART                 USART3
    #define TELEMETRY_DMA_Stream_TX         DMA1_Stream3
    #define TELEMETRY_DMA_Channel_TX        DMA_Channel_4
    #define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream3_IRQn
    #define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream3_IRQHandler
    #define TELEMETRY_DMA_TX_FLAG_TC        DMA_IT_TCIF3
    #define TELEMETRY_USART_IRQHandler      USART3_IRQHandler
    #define TELEMETRY_USART_IRQn            USART3_IRQn
    #define TELEMETRY_EXTI_PortSource       EXTI_PortSourceGPIOD
    #define TELEMETRY_EXTI_PinSource        EXTI_PinSource9
    #define TELEMETRY_EXTI_LINE             EXTI_Line9
    #define TELEMETRY_EXTI_IRQn             EXTI9_5_IRQn
    #define TELEMETRY_EXTI_TRIGGER          EXTI_Trigger_Rising
  #elif defined(RADIO_MAMBO)
    #define TELEMETRY_RCC_AHB1Periph        (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
    #define TELEMETRY_RCC_APB1Periph        RCC_APB1Periph_USART2
    #define TELEMETRY_DIR_GPIO              GPIOB
    #define TELEMETRY_DIR_GPIO_PIN          GPIO_Pin_13 // PB.13
    #define TELEMETRY_DIR_OUTPUT()          TELEMETRY_DIR_GPIO->BSRRL = TELEMETRY_DIR_GPIO_PIN
    #define TELEMETRY_DIR_INPUT()           TELEMETRY_DIR_GPIO->BSRRH = TELEMETRY_DIR_GPIO_PIN
    #define TELEMETRY_TX_POLARITY_GPIO      GPIOB
    #define TELEMETRY_TX_POLARITY_PIN       GPIO_Pin_2  // PB.02
    #define TELEMETRY_TX_POLARITY_INVERT()  TELEMETRY_TX_POLARITY_GPIO->BSRRL = TELEMETRY_TX_POLARITY_PIN
    #define TELEMETRY_TX_POLARITY_NORMAL()  TELEMETRY_TX_POLARITY_GPIO->BSRRH = TELEMETRY_TX_POLARITY_PIN
    #define TELEMETRY_RX_POLARITY_GPIO      GPIOB
    #define TELEMETRY_RX_POLARITY_PIN       GPIO_Pin_11 // PB.11
    #define TELEMETRY_RX_POLARITY_INVERT()  TELEMETRY_RX_POLARITY_GPIO->BSRRL = TELEMETRY_RX_POLARITY_PIN
    #define TELEMETRY_RX_POLARITY_NORMAL()  TELEMETRY_RX_POLARITY_GPIO->BSRRH = TELEMETRY_RX_POLARITY_PIN

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
  #endif

#if defined(RADIO_TANGO)
  #define TELEMETRY_EXTI_REUSE_INTERRUPT_ROTARY_ENCODER
#endif
  #define TELEMETRY_TIMER                   TIM11
  #define TELEMETRY_TIMER_IRQn              TIM1_TRG_COM_TIM11_IRQn
  #define TELEMETRY_TIMER_IRQHandler        TIM1_TRG_COM_TIM11_IRQHandler

  // USB
  #define USB_RCC_AHB1Periph_GPIO           RCC_AHB1Periph_GPIOA
  #define USB_GPIO                          GPIOA
  #define USB_GPIO_PIN_VBUS                 GPIO_Pin_9  // PA.09
  #define USB_GPIO_PIN_DM                   GPIO_Pin_11 // PA.11
  #define USB_GPIO_PIN_DP                   GPIO_Pin_12 // PA.12
  #define USB_GPIO_PinSource_DM             GPIO_PinSource11
  #define USB_GPIO_PinSource_DP             GPIO_PinSource12
  #define USB_GPIO_AF                       GPIO_AF_OTG1_FS

  // BackLight
  #if defined(RADIO_TANGO)
    #define BACKLIGHT_RCC_AHB1Periph        0
    #define BACKLIGHT_RCC_APB1Periph        0
  #else
    #define BACKLIGHT_RCC_AHB1Periph        RCC_AHB1Periph_GPIOD
    #define BACKLIGHT_RCC_APB1Periph        RCC_APB1Periph_TIM4
    #define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
    #define BACKLIGHT_TIMER                 TIM4
    #define BACKLIGHT_GPIO                  GPIOD
    #define BACKLIGHT_GPIO_PIN              GPIO_Pin_15 // PD.15
    #define BACKLIGHT_GPIO_PinSource        GPIO_PinSource15
    #define BACKLIGHT_GPIO_AF               GPIO_AF_TIM4
    #define BACKLIGHT_COUNTER_REGISTER      BACKLIGHT_TIMER->CCR4
  #endif

  // LCD driver
  #if defined(RADIO_TANGO)
    #define LCD_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
    #define LCD_RCC_APB1Periph              RCC_APB1Periph_SPI3
    #define LCD_SPI_GPIO                    GPIOB
    #define LCD_MOSI_GPIO_PIN               GPIO_Pin_5 // PB.5
    #define LCD_MOSI_GPIO_PinSource         GPIO_PinSource5
    #define LCD_CLK_GPIO_PIN                GPIO_Pin_3 // PB.3
    #define LCD_CLK_GPIO_PinSource          GPIO_PinSource3
    #define LCD_NCS_GPIO                    GPIOD
    #define LCD_NCS_GPIO_PIN                GPIO_Pin_1 // PD.1
    #define LCD_RST_GPIO                    GPIOD
    #define LCD_RST_GPIO_PIN                GPIO_Pin_3 // PD.3
    #define LCD_DC_GPIO                     GPIOD
    #define LCD_DC_GPIO_PIN                 GPIO_Pin_6 // PD.6
    #define LCD_DMA                         DMA1
    #define LCD_DMA_Stream                  DMA1_Stream7
    #define LCD_DMA_Stream_IRQn             DMA1_Stream7_IRQn
    #define LCD_DMA_Stream_IRQHandler       DMA1_Stream7_IRQHandler
    #define LCD_DMA_FLAGS                   (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
    #define LCD_DMA_FLAG_INT                DMA_HIFCR_CTCIF7
    #define LCD_SPI                         SPI3
    #define LCD_GPIO_AF                     GPIO_AF_SPI3
  #elif defined(RADIO_MAMBO)
    #define LCD_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA1)
    #define LCD_RCC_APB1Periph              RCC_APB1Periph_SPI3
    #define LCD_SPI_GPIO                    GPIOB
    #define LCD_MOSI_GPIO_PIN               GPIO_Pin_5 // PB.5
    #define LCD_MOSI_GPIO_PinSource         GPIO_PinSource5
    #define LCD_CLK_GPIO_PIN                GPIO_Pin_3 // PB.3
    #define LCD_CLK_GPIO_PinSource          GPIO_PinSource3
    #define LCD_A0_GPIO                     GPIOD
    #define LCD_A0_GPIO_PIN                 GPIO_Pin_0 // PD.0
    #define LCD_NCS_GPIO                    GPIOD
    #define LCD_NCS_GPIO_PIN                GPIO_Pin_1 // PD.1
    #define LCD_RST_GPIO                    GPIOD
    #define LCD_RST_GPIO_PIN                GPIO_Pin_3 // PD.3
    #define LCD_DMA                         DMA1
    #define LCD_DMA_Stream                  DMA1_Stream7
    #define LCD_DMA_Stream_IRQn             DMA1_Stream7_IRQn
    #define LCD_DMA_Stream_IRQHandler       DMA1_Stream7_IRQHandler
    #define LCD_DMA_FLAGS                   (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
    #define LCD_DMA_FLAG_INT                DMA_HIFCR_CTCIF7
    #define LCD_SPI                         SPI3
    #define LCD_GPIO_AF                     GPIO_AF_SPI3
  #endif

  // SD
  #define SD_RCC_AHB1Periph                 (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_DMA2)
  #define SD_PRESENT_GPIO                   GPIOC
  #define SD_PRESENT_GPIO_PIN               GPIO_Pin_5  // PC.05
  #define SD_SDIO_DMA_STREAM                DMA2_Stream3
  #define SD_SDIO_DMA_CHANNEL               DMA_Channel_4
  #define SD_SDIO_DMA_FLAG_FEIF             DMA_FLAG_FEIF3
  #define SD_SDIO_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF3
  #define SD_SDIO_DMA_FLAG_TEIF             DMA_FLAG_TEIF3
  #define SD_SDIO_DMA_FLAG_HTIF             DMA_FLAG_HTIF3
  #define SD_SDIO_DMA_FLAG_TCIF             DMA_FLAG_TCIF3
  #define SD_SDIO_DMA_IRQn                  DMA2_Stream3_IRQn
  #define SD_SDIO_DMA_IRQHANDLER            DMA2_Stream3_IRQHandler
  #define SD_SDIO_FIFO_ADDRESS              ((uint32_t)0x40012C80)
  #define SD_SDIO_CLK_DIV(fq)               ((48000000 / (fq)) - 2)
  #define SD_SDIO_INIT_CLK_DIV              SD_SDIO_CLK_DIV(400000)
  #define SD_SDIO_TRANSFER_CLK_DIV          SD_SDIO_CLK_DIV(24000000)

  // Audio
  #define AUDIO_RCC_AHB1Periph              (RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_DMA1)
  #define AUDIO_RCC_APB1Periph              (RCC_APB1Periph_TIM6 | RCC_APB1Periph_DAC)
  #define AUDIO_OUTPUT_GPIO                 GPIOA
  #define AUDIO_OUTPUT_GPIO_PIN             GPIO_Pin_4  // PA.04
  #define AUDIO_DMA_Stream                  DMA1_Stream5
  #define AUDIO_DMA_Stream_IRQn             DMA1_Stream5_IRQn
  #define AUDIO_TIM_IRQn                    TIM6_DAC_IRQn
  #define AUDIO_TIM_IRQHandler              TIM6_DAC_IRQHandler
  #define AUDIO_DMA_Stream_IRQHandler       DMA1_Stream5_IRQHandler
  #define AUDIO_TIMER                       TIM6
  #define AUDIO_DMA                         DMA1
  #if defined(RADIO_TANGO)
    #define AUDIO_MUTE_GPIO                 GPIOD
    #define AUDIO_MUTE_GPIO_PIN             GPIO_Pin_5  // PD.05
  #elif defined(RADIO_MAMBO)
    #define AUDIO_MUTE_GPIO                 GPIOE
    #define AUDIO_MUTE_GPIO_PIN             GPIO_Pin_0  // PE.00
  #endif

  // Haptic
  #define HAPTIC_RCC_AHB1Periph             RCC_AHB1Periph_GPIOB
  #define HAPTIC_GPIO                       GPIOB
  #define HAPTIC_GPIO_PIN                   GPIO_Pin_0  // PB.00

  // ESP
  #if defined(ESP_SERIAL)
    #if defined(RADIO_TANGO)
      #define ESP_EN_GPIO                   GPIOA
      #define ESP_EN_GPIO_PIN               GPIO_Pin_3 // PA.03
      #define ESP_DMA_Stream_RX             DMA1_Stream2
      #define ESP_DMA_Stream_IRQHandler     DMA1_Stream2_IRQHandler
      #define ESP_DMA_RX_FLAG_TC            DMA_IT_TCIF2
    #elif defined(RADIO_MAMBO)
      #define ESP_EN_GPIO                   GPIOA
      #define ESP_EN_GPIO_PIN               GPIO_Pin_15 // PA.15
      #define ESP_DMA_Stream_RX             DMA1_Stream2
      #define ESP_DMA_Stream_IRQHandler     DMA1_Stream2_IRQHandler
      #define ESP_DMA_RX_FLAG_TC            DMA_IT_TCIF2
    #endif
  #endif

  // Xms Interrupt
  #define INTERRUPT_xMS_RCC_APB1Periph      RCC_APB1Periph_TIM14
  #define INTERRUPT_xMS_TIMER               TIM14
  #define INTERRUPT_xMS_IRQn                TIM8_TRG_COM_TIM14_IRQn
  #define INTERRUPT_xMS_IRQHandler          TIM8_TRG_COM_TIM14_IRQHandler

  // 2MHz Timer
  #define TIMER_2MHz_RCC_APB1Periph         RCC_APB1Periph_TIM7
  #define TIMER_2MHz_TIMER                  TIM7

  // Mixer scheduler timer
  #define MIXER_SCHEDULER_TIMER_RCC_APB1Periph RCC_APB1Periph_TIM12
  #define MIXER_SCHEDULER_TIMER             TIM12
  #define MIXER_SCHEDULER_TIMER_FREQ        (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define MIXER_SCHEDULER_TIMER_IRQn        TIM8_BRK_TIM12_IRQn
  #define MIXER_SCHEDULER_TIMER_IRQHandler  TIM8_BRK_TIM12_IRQHandler

  // crossfire Interrupt
  #define INTERRUPT_NOT_TIMER               TIM13
  #define INTERRUPT_NOT_TIMER_IRQn          TIM8_UP_TIM13_IRQn  
  #define INTERRUPT_TIM13_IRQHandler        TIM8_UP_TIM13_IRQHandler
  #define INTERRUPT_EXTI_IRQn               EXTI15_10_IRQn
  #define INTERRUPT_EXTI_IRQHandler         EXTI15_10_IRQHandler

  // S.Port update connector
  #define SPORT_MAX_BAUDRATE                400000

#endif // _HAL_H_
