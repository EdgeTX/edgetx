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

// ST16 platform HAL, shared by the ST16 (air) and G11P (surface, PCBREV=G11P)
// boards. Net-identical mappings are defined once below; only the nets that
// differ on the G11P schematic are branched with #if defined(RADIO_G11P).
// See st16-vs-g11p-pinout.md for the pin-by-pin map.

// DMA streams:
// 
// - LEDs       TIM2_UP: DMA1 stream 0
// - Audio      SPI1_TX: DMA1 stream 1
// - Gimbals  USART2_RX: DMA1 stream 2
// - Telemetry UART5_RX: DMA1 stream 3
// - Telemetry UART5_TX: DMA1 stream 7
// 
// - ADC3:               DMA2 stream 0
// - ADC1:               DMA2 stream 4
// - Ext. Mod. UART4_RX: DMA2 stream 5
// - Ext. Mod. UART4_TX: DMA2 stream 6
// - Ext. Mod.  TIM4_UP: DMA2 stream 6
// 

#define CPU_FREQ                400000000

#define PERI1_FREQUENCY         100000000
#define PERI2_FREQUENCY         100000000
#define TIMER_MULT_APB1         2
#define TIMER_MULT_APB2         2

// Keys
#if defined(RADIO_G11P)
// Direct GPIO keys (active low, pull-up): K6A, K7A and the encoder push (W-KEY).
// K1/K2 (K12_ADC) and K3/K4 (K34_ADC) are ADC ladders decoded by g11p_key_driver.cpp.
#define KEYS_GPIO_REG_MENU            GPIOC
#define KEYS_GPIO_PIN_MENU            LL_GPIO_PIN_7  // PC.07 K6A
#define KEYS_GPIO_REG_EXIT            GPIOA
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_8  // PA.08 K7A
#define KEYS_GPIO_REG_ENTER           GPIOG
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_14 // PG.14 W-KEY
#else
#define KEYS_GPIO_REG_ENTER           GPIOG
#define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_14  // PG.14
#define KEYS_GPIO_REG_PAGEDN          GPIOA
#define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_8   // PA.08
#define KEYS_GPIO_REG_EXIT            GPIOC
#define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5   // PC.05
#endif

// Trims
#if defined(RADIO_G11P)
// Trims are decoded by g11p_key_driver.cpp from the TRx_ADC lines.
// (all TRIMS_GPIO_REG_*/PIN_* left undefined)
#else
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
#endif

// Digital keys / switches
#if defined(RADIO_G11P)
// SW1 is a digital 2-position switch on PH4 (the "_ADC" suffix is only the connector
// net name). SW2/SW3/SW4 are analog and are mapped as ADC switches (SWB/SWC/SWD) in
// the ADC section below, handled by the generic stm32 switch driver.
#define SWITCHES_GPIO_REG_A             GPIOH
#define SWITCHES_GPIO_PIN_A             LL_GPIO_PIN_4  // PH.04 SW1
#else
// function switches
#define FUNCTION_SWITCH_1               I2C
#define FUNCTION_SWITCH_2               I2C
#define FUNCTION_SWITCH_3               I2C
#define FUNCTION_SWITCH_4               I2C
#define FUNCTION_SWITCH_5               I2C
#define FUNCTION_SWITCH_6               I2C

// Direct switches

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

// ADC
#if defined(RADIO_G11P)
//
// Analog input order (this defines the getAnalogValue() indices used by the
// custom g11p_key_driver):
//
//   0  ST        PA1        6  RAW4 = TR2   PF9    (ADC3)
//   1  TH        PA0        7  RAW5 = TR3   PA2
//   2  POT1=VR2  PC2        8  RAW6 = TR4   PA3
//   3  RAW1=K12  PB0        9  BATT = LIBAT PC1
//   4  RAW2=K34  PB1       10  RTC_BAT
//   5  RAW3=TR1  PF8 (ADC3) 11 SWB = SW2   PA6
//                          12  SWC = SW3   PA7
//                          13  SWD = SW4   PC3
//
// ADC1 (8): ST, TH, K12, K34, TR3, TR4, SW2, SW3
// ADC3 (6): POT1, TR1, TR2, SW4, LIBAT, RTC_BAT
//
// TR_ADC (PC5) and 2S_ADC (PC0) are intentionally not mapped; see
// g11p-dev-plan.md section 10.
//
// TODO(hardware): validate the analog input ordering/inversion and the switch
// directions on G11P.
#define ADC_GPIO_PIN_STICK_ST           LL_GPIO_PIN_1      // PA.01 ST_ADC
#define ADC_GPIO_PIN_STICK_TH           LL_GPIO_PIN_0      // PA.00 TH_ADC
#define ADC_CHANNEL_STICK_ST            LL_ADC_CHANNEL_17  // ADC1_INP17
#define ADC_CHANNEL_STICK_TH            LL_ADC_CHANNEL_16  // ADC1_INP16

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_2      // PC.02 VR2A
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_12  // ADC123_INP12

#define ADC_GPIO_PIN_RAW1               LL_GPIO_PIN_0      // PB.00 K12_ADC
#define ADC_GPIO_PIN_RAW2               LL_GPIO_PIN_1      // PB.01 K34_ADC
#define ADC_GPIO_PIN_RAW3               LL_GPIO_PIN_8      // PF.08 TR1_ADC
#define ADC_GPIO_PIN_RAW4               LL_GPIO_PIN_9      // PF.09 TR2_ADC
#define ADC_GPIO_PIN_RAW5               LL_GPIO_PIN_2      // PA.02 TR3_ADC
#define ADC_GPIO_PIN_RAW6               LL_GPIO_PIN_3      // PA.03 TR4_ADC
#define ADC_CHANNEL_RAW1                LL_ADC_CHANNEL_9   // ADC12_INP9
#define ADC_CHANNEL_RAW2                LL_ADC_CHANNEL_5   // ADC12_INP5
#define ADC_CHANNEL_RAW3                LL_ADC_CHANNEL_8   // ADC3_INP8
#define ADC_CHANNEL_RAW4                LL_ADC_CHANNEL_2   // ADC3_INP2
#define ADC_CHANNEL_RAW5                LL_ADC_CHANNEL_14  // ADC12_INP14
#define ADC_CHANNEL_RAW6                LL_ADC_CHANNEL_15  // ADC12_INP15

#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_6      // PA.06 SW2_ADC
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_7      // PA.07 SW3_ADC
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_3      // PC.03 SW4_ADC
#define ADC_CHANNEL_SWB                 LL_ADC_CHANNEL_3   // ADC12_INP3
#define ADC_CHANNEL_SWC                 LL_ADC_CHANNEL_7   // ADC12_INP7
#define ADC_CHANNEL_SWD                 LL_ADC_CHANNEL_13  // ADC123_INP13

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01 LIBAT_ADC

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_STICK_ST | ADC_GPIO_PIN_STICK_TH |                    \
   ADC_GPIO_PIN_RAW5 | ADC_GPIO_PIN_RAW6 |                            \
   ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_SWC)
#define ADC_GPIOB_PINS (ADC_GPIO_PIN_RAW1 | ADC_GPIO_PIN_RAW2)
#define ADC_GPIOC_PINS (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SWD | ADC_GPIO_PIN_BATT)
#define ADC_GPIOF_PINS (ADC_GPIO_PIN_RAW3 | ADC_GPIO_PIN_RAW4)

#define ADC_EXT_CHANNELS                                                   \
  {                                                                        \
    ADC_CHANNEL_POT1, ADC_CHANNEL_SWD, ADC_CHANNEL_RAW3, ADC_CHANNEL_RAW4, \
    ADC_CHANNEL_BATT, ADC_CHANNEL_RTC_BAT                                  \
  }

// TODO(hardware): validate the analog input ordering/inversion on G11P.
#define ADC_DIRECTION {                 \
    0,0,   /* ST, TH */                 \
    0,     /* POT1 = VR2 */             \
    0,0,0,0,0,0, /* RAW1..RAW6 */       \
    0,     /* BATT */                   \
    0,     /* RTC_BAT */                \
    0,0,0  /* SWB, SWC, SWD */          \
  }
#else
#define ADC_GPIO_PIN_STICK_LH           LL_GPIO_PIN_0      // PA.00
#define ADC_GPIO_PIN_STICK_LV           LL_GPIO_PIN_1      // PA.01
#define ADC_GPIO_PIN_STICK_RV           LL_GPIO_PIN_2      // PA.02
#define ADC_GPIO_PIN_STICK_RH           LL_GPIO_PIN_3      // PA.03

#define ADC_GPIO_PIN_POT1               LL_GPIO_PIN_6      // PA.06 VRA
#define ADC_GPIO_PIN_POT2               LL_GPIO_PIN_1      // PB.01 VRB
#define ADC_GPIO_PIN_SLIDER1            LL_GPIO_PIN_0      // PB.00 VRC/LS
#define ADC_GPIO_PIN_SLIDER2            LL_GPIO_PIN_7      // PA.07 VRD/RS

// If serial gimbals are used, we can reuse the channels
#define ADC_GPIO_PIN_EXT1               LL_GPIO_PIN_0      // PA.00 EXT1
#define ADC_GPIO_PIN_EXT2               LL_GPIO_PIN_1      // PA.01 EXT2
#define ADC_GPIO_PIN_EXT3               LL_GPIO_PIN_2      // PA.02 EXT3
#define ADC_GPIO_PIN_EXT4               LL_GPIO_PIN_3      // PA.03 EXT4

#define ADC_GPIO_PIN_SWA                LL_GPIO_PIN_2      // PC.02_C
#define ADC_GPIO_PIN_SWB                LL_GPIO_PIN_3      // PC.03_C
#define ADC_GPIO_PIN_SWC                LL_GPIO_PIN_9      // PF.09
#define ADC_GPIO_PIN_SWD                LL_GPIO_PIN_7      // PF.07
#define ADC_GPIO_PIN_SWE                LL_GPIO_PIN_8      // PF.08
#define ADC_GPIO_PIN_SWF                LL_GPIO_PIN_6      // PF.06

#define ADC_GPIO_PIN_BATT               LL_GPIO_PIN_1      // PC.01

#define ADC_GPIOA_PINS                                                \
  (ADC_GPIO_PIN_POT1 | ADC_GPIO_PIN_SLIDER2 | ADC_GPIO_PIN_STICK_LH | \
   ADC_GPIO_PIN_STICK_LV | ADC_GPIO_PIN_STICK_RH | ADC_GPIO_PIN_STICK_RV)

#define ADC_GPIOB_PINS (ADC_GPIO_PIN_POT2 | ADC_GPIO_PIN_SLIDER1)

#define ADC_GPIOC_PINS (ADC_GPIO_PIN_SWA | ADC_GPIO_PIN_SWB | ADC_GPIO_PIN_BATT)

#define ADC_GPIOF_PINS \
  (ADC_GPIO_PIN_SWC | ADC_GPIO_PIN_SWD | ADC_GPIO_PIN_SWE | ADC_GPIO_PIN_SWF)

#define ADC_CHANNEL_STICK_LH            LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_STICK_LV            LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_STICK_RV            LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_STICK_RH            LL_ADC_CHANNEL_17    // ADC12_INP17

// Each ADC cannot map more than 8 channels, otherwise it will cause problems
#define ADC_CHANNEL_POT1                LL_ADC_CHANNEL_3    // ADC12_INP3
#define ADC_CHANNEL_POT2                LL_ADC_CHANNEL_5    // ADC12_INP5
#define ADC_CHANNEL_SLIDER1             LL_ADC_CHANNEL_9    // ADC12_INP9
#define ADC_CHANNEL_SLIDER2             LL_ADC_CHANNEL_7    // ADC12_INP7

// Ext inputs: analog gimbal channel re-use
#define ADC_CHANNEL_EXT1                LL_ADC_CHANNEL_14    // ADC12_INP14
#define ADC_CHANNEL_EXT2                LL_ADC_CHANNEL_15    // ADC12_INP15
#define ADC_CHANNEL_EXT3                LL_ADC_CHANNEL_16    // ADC12_INP16
#define ADC_CHANNEL_EXT4                LL_ADC_CHANNEL_17    // ADC12_INP17

// Analog switches
#define ADC_CHANNEL_SWA                 LL_ADC_CHANNEL_0    // ADC3_INP0
#define ADC_CHANNEL_SWB                 LL_ADC_CHANNEL_1    // ADC3_INP1
#define ADC_CHANNEL_SWC                 LL_ADC_CHANNEL_2    // ADC3_INP2
#define ADC_CHANNEL_SWD                 LL_ADC_CHANNEL_3    // ADC3_INP3
#define ADC_CHANNEL_SWE                 LL_ADC_CHANNEL_8    // ADC3_INP8
#define ADC_CHANNEL_SWF                 LL_ADC_CHANNEL_7    // ADC3_INP7

#define ADC_EXT_CHANNELS                                                    \
  {                                                                         \
    ADC_CHANNEL_SWA, ADC_CHANNEL_SWB, ADC_CHANNEL_SWC, ADC_CHANNEL_SWD,     \
    ADC_CHANNEL_SWE, ADC_CHANNEL_SWF, ADC_CHANNEL_BATT, ADC_CHANNEL_RTC_BAT \
  }

#define ADC_DIRECTION {       \
    0,0,0,0, /* gimbals */    \
    0,0,     /* pots */       \
    0,0,     /* sliders */    \
    0,	     /* vbat */       \
    0,       /* rtc_bat */    \
    0,       /* SWA */        \
    0,       /* SWB */        \
    0,       /* SWC */        \
    0,       /* SWD */        \
    0,       /* SWE */        \
    0        /* SWF */        \
  }
#endif

#define ADC_MAIN                        ADC1
#define ADC_DMA                         DMA2
#define ADC_DMA_CHANNEL                 LL_DMAMUX1_REQ_ADC1
#define ADC_DMA_STREAM                  LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ              DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler       DMA2_Stream4_IRQHandler
#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_EXT                         ADC3
#define ADC_EXT_DMA                     DMA2
#define ADC_EXT_DMA_CHANNEL             LL_DMAMUX1_REQ_ADC3
#define ADC_EXT_DMA_STREAM              LL_DMA_STREAM_0
#define ADC_EXT_DMA_STREAM_IRQ          DMA2_Stream0_IRQn
#define ADC_EXT_DMA_STREAM_IRQHandler   DMA2_Stream0_IRQHandler
#define ADC_EXT_SAMPTIME                LL_ADC_SAMPLINGTIME_8CYCLES_5

#define ADC_VREF_PREC2                  660

#define ADC_CHANNEL_BATT                LL_ADC_CHANNEL_11   // ADC123_INP11
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT // ADC12_IN16

#if !defined(RADIO_G11P)
#define USE_EXTI9_5_IRQ // used for I2C port extender interrupt
#define EXTI9_5_IRQ_Priority 5
#endif

// Power
#define PWR_SWITCH_GPIO             GPIO_PIN(GPIOB, 3)  // PB.03 PWR_SW
#define PWR_ON_GPIO                 GPIO_PIN(GPIOI, 8)  // PI.08 PWR_CTR

// Chargers (USB and wireless)
#define UCHARGER_CHARGE_END_GPIO    GPIO_PIN(GPIOD, 4) // PD.04
#if defined(RADIO_G11P)
#define UCHARGER_GPIO               GPIO_PIN(GPIOH, 6)  // PH.06 USB_SW
#define CHARGE_EN_GPIO              GPIO_PIN(GPIOH, 7)  // PH.07 CHARGE_ENB
#define VBUS_DETECT_GPIO            GPIO_PIN(GPIOI, 3)  // PI.03 VBUS_I
#else
#define UCHARGER_GPIO               GPIO_PIN(GPIOC, 0) // PC.00
#endif

// TODO! Check IOLL1 to PI.01 connectivity!

// S.Port update connector
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
#if defined(RADIO_G11P)
#define TELEMETRY_TX_GPIO               GPIO_UNDEF
#define TELEMETRY_RX_GPIO               GPIO_UNDEF
#else
#define TELEMETRY_TX_GPIO               GPIO_PIN(GPIOB, 13)
#define TELEMETRY_RX_GPIO               GPIO_UNDEF //GPIO_PIN(GPIOB, 12)
#endif
#define TELEMETRY_USART                 UART5
#define TELEMETRY_USART_IRQn            UART5_IRQn
#define TELEMETRY_DMA                   DMA1
#define TELEMETRY_DMA_Stream_TX         LL_DMA_STREAM_7
#define TELEMETRY_DMA_Channel_TX        LL_DMAMUX1_REQ_UART5_TX
#define TELEMETRY_DMA_TX_Stream_IRQ     DMA1_Stream7_IRQn
#define TELEMETRY_DMA_TX_IRQHandler     DMA1_Stream7_IRQHandler
#if !defined(RADIO_G11P)
#define TELEMETRY_DMA_Stream_RX         LL_DMA_STREAM_3
#define TELEMETRY_DMA_Channel_RX        LL_DMAMUX1_REQ_UART5_RX
#endif
#define TELEMETRY_USART_IRQHandler      UART5_IRQHandler

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
#if defined(RADIO_G11P)
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOI, 3)  // PI.03 VBUS_I
#else
#define USB_GPIO_VBUS                   GPIO_PIN(GPIOC, 0)  // PC.00
#endif
#define USB_GPIO_DM                     GPIO_PIN(GPIOA, 11) // PA.11
#define USB_GPIO_DP                     GPIO_PIN(GPIOA, 12) // PA.12
#define USB_GPIO_AF                     GPIO_AF10

// LCD (LTDC RGB pins are configured in lcd_driver.cpp)
#define LCD_SPI_CS_GPIO                 GPIOH
#if defined(RADIO_G11P)
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_8  // PH.08 LCD_CS
#define LCD_NRST_GPIO                   GPIO_PIN(GPIOI, 11) // PI.11 LCD_NRST
#define LCD_ID_GPIO                     GPIO_PIN(GPIOC, 4)  // PC.04 LCD_ID
#else
#define LCD_SPI_CS_GPIO_PIN             LL_GPIO_PIN_6  // PH.06
#endif
#define LCD_SPI_GPIO                    GPIOE
#define LCD_SPI_SCK_GPIO_PIN            LL_GPIO_PIN_4  // PE.04
#define LCD_SPI_MOSI_GPIO_PIN           LL_GPIO_PIN_3  // PE.03
#define LTDC_IRQ_PRIO                   4
#define DMA_SCREEN_IRQ_PRIO             6

// Backlight
#if defined(RADIO_G11P)
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOC, 6) // PC.06
#else
#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOC, 06) // TIM3_CH1
#endif
#define BACKLIGHT_TIMER                 TIM3
#define BACKLIGHT_GPIO_AF               GPIO_AF2
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// QSPI Flash
#define QSPI_MAX_FREQ                   80000000U // 80 MHz
#define QSPI_CLK_GPIO                   GPIO_PIN(GPIOB, 2)
#define QSPI_CLK_GPIO_AF                GPIO_AF9
#define QSPI_CS_GPIO                    GPIO_PIN(GPIOB, 6)
#define QSPI_CS_GPIO_AF                 GPIO_AF10
#define QSPI_MISO_GPIO                  GPIO_PIN(GPIOD, 12)
#define QSPI_MISO_GPIO_AF               GPIO_AF9
#define QSPI_MOSI_GPIO                  GPIO_PIN(GPIOD, 11)
#define QSPI_MOSI_GPIO_AF               GPIO_AF9
#define QSPI_WP_GPIO                    GPIO_PIN(GPIOE, 2)
#define QSPI_WP_GPIO_AF                 GPIO_AF9
#define QSPI_HOLD_GPIO                  GPIO_PIN(GPIOD, 13)
#define QSPI_HOLD_GPIO_AF               GPIO_AF9
#define QSPI_FLASH_SIZE                 0x800000

#define SD_PRESENT_GPIO                GPIO_PIN(GPIOD, 3) // PD.03
#define SD_SDIO                        SDMMC1
#define SD_SDIO_CLK_DIV(fq)            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC) / (2 * fq))
#define SD_SDIO_INIT_CLK_DIV           SD_SDIO_CLK_DIV(400000)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(20000000)
#define STORAGE_USE_SDIO


// Audio
#define AUDIO_XDCS_GPIO               GPIO_PIN(GPIOG, 12) // PG.12
#define AUDIO_CS_GPIO                 GPIO_PIN(GPIOG, 10) // PG.10
#define AUDIO_DREQ_GPIO               GPIO_PIN(GPIOG, 13) // PG.13
#define AUDIO_SPI                     SPI1
#define AUDIO_SPI_GPIO_AF             LL_GPIO_AF_5
#define AUDIO_SPI_SCK_GPIO            GPIO_PIN(GPIOA, 5)  // PA.05
#define AUDIO_SPI_MISO_GPIO           GPIO_PIN(GPIOG, 9)  // PG.09
#define AUDIO_SPI_MOSI_GPIO           GPIO_PIN(GPIOD, 7)  // PD.07
#define AUDIO_SPI_DMA                 DMA1
#define AUDIO_SPI_DMA_REQ             LL_DMAMUX1_REQ_SPI1_TX
#define AUDIO_SPI_DMA_STREAM          LL_DMA_STREAM_1
#if defined(RADIO_G11P)
#define AUDIO_RST_GPIO                GPIO_PIN(GPIOD, 6)  // PD.06 VS1053B_RST
#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05 PA_NMUTE
#else
#define AUDIO_MUTE_GPIO               0
#endif
#define AUDIO_UNMUTE_DELAY            180  // ms
#define AUDIO_MUTE_DELAY              200  // ms
#define INVERTED_MUTE_PIN

// I2C Bus
#define I2C_B1                          I2C1
#if defined(RADIO_G11P)
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 9)  // PB.09 TP_SDA
#else
#define I2C_B1_SDA_GPIO                 GPIO_PIN(GPIOB, 7)  // PB.07
#endif
#define I2C_B1_SCL_GPIO                 GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B1_CLK_RATE                 400000

#if !defined(RADIO_G11P)
#define I2C_B2                          I2C3
#define I2C_B2_SDA_GPIO                 GPIO_PIN(GPIOH, 8)  // PH.08
#define I2C_B2_SCL_GPIO                 GPIO_PIN(GPIOH, 7)  // PH.07
#define I2C_B2_GPIO_AF                  LL_GPIO_AF_4
#define I2C_B2_CLK_RATE                 400000
#endif

// Haptic: TIM3_CH2
#define HAPTIC_PWM
#define HAPTIC_GPIO                     GPIO_PIN(GPIOB, 5) // PB.05
#define HAPTIC_GPIO_TIMER               TIM3
#define HAPTIC_GPIO_AF                  GPIO_AF2
#define HAPTIC_TIMER_OUTPUT_ENABLE      TIM_CCER_CC2E | TIM_CCER_CC2NE;
#define HAPTIC_TIMER_MODE               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE
#define HAPTIC_TIMER_COMPARE_VALUE      HAPTIC_GPIO_TIMER->CCR2

#if !defined(RADIO_G11P)
// Flysky Hall Stick
#define FLYSKY_HALL_SERIAL_USART                 USART2

#define FLYSKY_HALL_SERIAL_TX_GPIO               GPIO_PIN(GPIOD, 5)  // PD.05
#define FLYSKY_HALL_SERIAL_RX_GPIO               GPIO_PIN(GPIOD, 6)  // PD.06
#define FLYSKY_HALL_SERIAL_USART_IRQn            USART2_IRQn

#define FLYSKY_HALL_SERIAL_DMA                   DMA1
#define FLYSKY_HALL_DMA_Stream_RX                LL_DMA_STREAM_2
#define FLYSKY_HALL_DMA_Stream_TX                LL_DMA_STREAM_4
#define FLYSKY_HALL_DMA_Channel                  LL_DMAMUX1_REQ_USART2_RX
#endif

// LED Strip
#if defined(RADIO_G11P)
#define LED_STRIP_LENGTH                  2
#define LED_STRIP_RESERVED_AT_END         0
#else
#define LED_STRIP_LENGTH                  24
#define LED_STRIP_RESERVED_AT_END         12
#endif
#define LED_STRIP_GPIO                    GPIO_PIN(GPIOA, 15)  // PA.15 / TIM2_CH1
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
#if defined(RADIO_G11P)
#define LED_CHARGING_START                0
#define LED_CHARGING_END                  1
#else
#define LED_CHARGING_START                12
#define LED_CHARGING_END                  23
#endif


// Internal Module
#define INTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 11) // PB.11
#define INTMODULE_RX_GPIO               GPIO_PIN(GPIOB, 10) // PB.10
#define INTMODULE_USART                 USART3
#define INTMODULE_GPIO_AF               LL_GPIO_AF_7
#define INTMODULE_USART_IRQn            USART3_IRQn
#define INTMODULE_USART_IRQHandler      USART3_IRQHandler
//#define INTMODULE_DMA                   DMA1
#define INTMODULE_DMA_STREAM            LL_DMA_STREAM_1
#define INTMODULE_DMA_STREAM_IRQ        DMA1_Stream1_IRQn
#define INTMODULE_DMA_FLAG_TC           DMA_FLAG_TCIF1
#define INTMODULE_DMA_CHANNEL           LL_DMA_CHANNEL_5
//#define INTMODULE_RX_DMA                DMA1
#define INTMODULE_RX_DMA_STREAM         LL_DMA_STREAM_3
#define INTMODULE_RX_DMA_CHANNEL        LL_DMA_CHANNEL_5

// External Module
#if defined(RADIO_G11P)
// Exposed on the trainer port (UART7, PF6/PF7). No module bay and no PPM: the
// port is serial-only, but the shared module framework still requires a pulse
// timer definition (left at the ST16 values, unused).
#define EXTMODULE
#define EXTMODULE_TX_GPIO                  GPIO_PIN(GPIOF, 7)  // PF.07 PPM_OUT
#define EXTMODULE_RX_GPIO                  GPIO_PIN(GPIOF, 6)  // PF.06 PPM_IN
#define EXTMODULE_USART                    UART7
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOF, 7)
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOF, 6)
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART7_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6
#define EXTMODULE_USART_RX_DMA             DMA2
#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART7_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5
#define EXTMODULE_USART_IRQHandler         UART7_IRQHandler
#define EXTMODULE_USART_IRQn               UART7_IRQn

#define EXTMODULE_TIMER                    TIM4
#define EXTMODULE_TIMER_Channel            LL_TIM_CHANNEL_CH4
#define EXTMODULE_TIMER_IRQn               TIM4_IRQn
#define EXTMODULE_TIMER_IRQHandler         TIM4_IRQHandler
#define EXTMODULE_TIMER_FREQ               (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF         LL_GPIO_AF_2
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM4_UP
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_6
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream6_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream6_IRQHandler
#else
#define EXTMODULE
#define EXTMODULE_PULSES
#define EXTMODULE_TX_GPIO               GPIO_PIN(GPIOB, 9)  // PB.09
#define EXTMODULE_RX_GPIO               GPIO_PIN(GPIOI, 9)  // PI.09
#define EXTMODULE_TIMER                 TIM4
#define EXTMODULE_TIMER_Channel         LL_TIM_CHANNEL_CH4
#define EXTMODULE_TIMER_IRQn            TIM4_IRQn
#define EXTMODULE_TIMER_IRQHandler      TIM4_IRQHandler
#define EXTMODULE_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define EXTMODULE_TIMER_TX_GPIO_AF      LL_GPIO_AF_2 // TIM4_CH4

//USART
#define EXTMODULE_USART                    UART4
#define EXTMODULE_USART_RX_GPIO            GPIO_PIN(GPIOI, 9)
#define EXTMODULE_USART_TX_GPIO            GPIO_PIN(GPIOB, 9)
#define EXTMODULE_USART_TX_DMA             DMA2
#define EXTMODULE_USART_TX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_TX
#define EXTMODULE_USART_TX_DMA_STREAM      LL_DMA_STREAM_6

#define EXTMODULE_USART_RX_DMA_CHANNEL     LL_DMAMUX1_REQ_UART4_RX
#define EXTMODULE_USART_RX_DMA_STREAM      LL_DMA_STREAM_5

#define EXTMODULE_USART_IRQHandler         UART4_IRQHandler
#define EXTMODULE_USART_IRQn               UART4_IRQn

//TIMER
#define EXTMODULE_TIMER_DMA_CHANNEL        LL_DMAMUX1_REQ_TIM4_UP
#define EXTMODULE_TIMER_DMA                DMA2
#define EXTMODULE_TIMER_DMA_STREAM         LL_DMA_STREAM_6
#define EXTMODULE_TIMER_DMA_STREAM_IRQn    DMA2_Stream6_IRQn
#define EXTMODULE_TIMER_DMA_IRQHandler     DMA2_Stream6_IRQHandler
#endif

#if !defined(RADIO_G11P)
// Trainer Port
#define TRAINER_RCC_AHB1Periph          (RCC_AHB1Periph_GPIOD)

#define TRAINER_IN_GPIO                 GPIO_PIN(GPIOE, 5) // PE.05
#define TRAINER_IN_TIMER_Channel        LL_TIM_CHANNEL_CH1

#define TRAINER_OUT_GPIO                GPIO_PIN(GPIOE, 6) // PE.06
#define TRAINER_OUT_TIMER_Channel       LL_TIM_CHANNEL_CH2

#define TRAINER_TIMER                   TIM15
#define TRAINER_TIMER_IRQn              TIM15_IRQn
#define TRAINER_TIMER_IRQHandler        TIM15_IRQHandler
#define TRAINER_GPIO_AF                 LL_GPIO_AF_4
#define TRAINER_TIMER_FREQ              (PERI1_FREQUENCY * TIMER_MULT_APB1)
#endif

#define TOUCH_I2C_BUS                   I2C_Bus_1
#define TOUCH_I2C_CLK_RATE              100000
#if defined(RADIO_G11P)
#define TOUCH_INT_GPIO                  GPIO_PIN(GPIOE, 6)  // PE.06 TP_INT
#define TOUCH_RST_GPIO                  GPIO_PIN(GPIOI, 9)  // PI.09 TP_RESET
#endif

#if !defined(RADIO_G11P)
#define IMU_I2C_BUS                     I2C_Bus_1
#define IMU_I2C_ADDRESS                 0x6A
#endif

//ROTARY emulation for trims as buttons
#define ROTARY_ENCODER_NAVIGATION
// Rotary Encoder
#define ROTARY_ENCODER_INVERTED
#define ROTARY_ENCODER_GPIO             GPIOG
#define ROTARY_ENCODER_GPIO_PIN_A       LL_GPIO_PIN_2 // PG.02
#define ROTARY_ENCODER_GPIO_PIN_B       LL_GPIO_PIN_3 // PG.03
#define ROTARY_ENCODER_POSITION()       ((ROTARY_ENCODER_GPIO->IDR >> 2) & 0x03)
#define ROTARY_ENCODER_EXTI_LINE1       LL_EXTI_LINE_2
#define ROTARY_ENCODER_EXTI_LINE2       LL_EXTI_LINE_3
#if !defined(USE_EXTI2_IRQ)
  #define USE_EXTI2_IRQ
  #define EXTI2_IRQ_Priority 5
#endif
#if !defined(USE_EXTI3_IRQ)
  #define USE_EXTI3_IRQ
  #define EXTI3_IRQ_Priority 5
#endif
#define ROTARY_ENCODER_EXTI_PORT        LL_SYSCFG_EXTI_PORTG
#define ROTARY_ENCODER_EXTI_SYS_LINE1   LL_SYSCFG_EXTI_LINE2
#define ROTARY_ENCODER_EXTI_SYS_LINE2   LL_SYSCFG_EXTI_LINE3
#define ROTARY_ENCODER_TIMER            TIM17
#define ROTARY_ENCODER_TIMER_IRQn       TIM17_IRQn
#define ROTARY_ENCODER_TIMER_IRQHandler TIM17_IRQHandler

#if defined(RADIO_G11P)
// W-KEY (encoder push) - read as KEY_ENTER by g11p_key_driver.cpp
#define G11P_WKEY_GPIO                  GPIOG
#define G11P_WKEY_PIN                   LL_GPIO_PIN_14 // PG.14 W-KEY

// RF module control
#define RF_PWR_ON_GPIO                  GPIO_PIN(GPIOE, 5)  // PE.05 RF_PWR_ON
#define M0_SYNC_GPIO                    GPIO_PIN(GPIOA, 10) // PA.10 M0_SYNC
#define RF_DFU_C_GPIO                   GPIO_PIN(GPIOB, 12) // PB.12 RF_DFU_C
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

#if defined(RADIO_G11P)
// LCD 320x480 portrait (MS0621-32TFL11E)
#define LCD_W                           320
#define LCD_H                           480

#define LCD_PHYS_W                      LCD_W
#define LCD_PHYS_H                      LCD_H
#else
#define LCD_W                           480
#define LCD_H                           320

#define LCD_PHYS_W                      LCD_H
#define LCD_PHYS_H                      LCD_W
#endif

#define LCD_DEPTH                       16
