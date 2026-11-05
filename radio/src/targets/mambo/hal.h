// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <stdbool.h>

// Keep the original bootloader and Tracer application outside the UI image.
#define FLASHSIZE                       0xC0000
#define BOOTLOADER_SIZE                 0xC000

// Two parallel 18650 cells form a 1S battery.
#define BATTERY_WARN                    34
#define BATTERY_MIN                     33
#define BATTERY_MAX                     42
#define BATT_SCALE                      128

#define LCD_CONTRAST_MIN                0
#define LCD_CONTRAST_MAX                255
#define LCD_CONTRAST_DEFAULT            159

// Pin mapping from public FreedomTX target at revision 4528db71.
#define PWR_SWITCH_GPIO                 GPIO_PIN(GPIOB, 14)
#define PWR_SWITCH_PIN_ACTIVE_HIGH
#define PWR_ON_GPIO                     GPIO_PIN(GPIOB, 12)

#define EXTMODULE_PWR_GPIO              GPIO_PIN(GPIOE, 2)
void mamboExternalModulePower(bool enable);
void mamboExternalModuleShutdown();
#define EXTERNAL_MODULE_PWR_ON()        mamboExternalModulePower(true)
#define EXTERNAL_MODULE_PWR_OFF()       mamboExternalModulePower(false)
#define IS_EXTERNAL_MODULE_ON()         gpio_read(EXTMODULE_PWR_GPIO)
// CRSF uses the stock bidirectional telemetry path, not USART6 on PPM.
#define TELEMETRY_USART                USART2
#define TELEMETRY_TX_GPIO              GPIO_PIN(GPIOD, 5)
#define TELEMETRY_RX_GPIO              GPIO_PIN(GPIOD, 6)
#define TELEMETRY_USART_IRQn           USART2_IRQn
#define TELEMETRY_USART_IRQHandler     USART2_IRQHandler
#define TELEMETRY_DMA                  DMA1
#define TELEMETRY_DMA_Stream_TX        LL_DMA_STREAM_6
#define TELEMETRY_DMA_Channel_TX       LL_DMA_CHANNEL_4
#define TELEMETRY_DMA_TX_Stream_IRQ    DMA1_Stream6_IRQn
#define TELEMETRY_DMA_TX_IRQHandler    DMA1_Stream6_IRQHandler
#define TELEMETRY_DIR_GPIO             GPIO_PIN(GPIOB, 13)
#define TELEMETRY_SET_INPUT            0
#define TELEMETRY_TX_REV_GPIO          GPIO_PIN(GPIOB, 2)
#define TELEMETRY_RX_REV_GPIO          GPIO_PIN(GPIOB, 11)
// Tested hardware: normal CRSF needs both controls low. The historical stock
// high/high setting broke channel output; retain the working configuration.
#define TELEMETRY_INVERTER_ACTIVE_HIGH
// Receive parsing is deferred from USART/DMA to this software EXTI interrupt.
#define TELEMETRY_RX_FRAME_EXTI_LINE   LL_EXTI_LINE_4
#define USE_EXTI4_IRQ
#define EXTI4_IRQ_Priority             5
#define TELEMETRY_EXTI_PRIO             0

// Use continuous TIM10 sampling for the Mambo wheel in every board profile.
#define ROTARY_ENCODER_POLLING

#define BACKLIGHT_GPIO                  GPIO_PIN(GPIOD, 15)
#define BACKLIGHT_GPIO_AF               GPIO_AF2
#define BACKLIGHT_TIMER                 TIM4
#define BACKLIGHT_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define BACKLIGHT_COUNTER_REGISTER      BACKLIGHT_TIMER->CCR4
#define BACKLIGHT_CCMR2                 (TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2)
#define BACKLIGHT_CCER                  TIM_CCER_CC4E

#define LCD_SPI                        SPI3
#define LCD_SPI_PRESCALER               SPI_CR1_BR_1
#define LCD_GPIO_AF                    GPIO_AF6
#define LCD_MOSI_GPIO                  GPIO_PIN(GPIOB, 5)
#define LCD_CLK_GPIO                   GPIO_PIN(GPIOB, 3)
#define LCD_A0_GPIO                    GPIO_PIN(GPIOD, 0)
#define LCD_NCS_GPIO                   GPIO_PIN(GPIOD, 1)
#define LCD_RST_GPIO                   GPIO_PIN(GPIOD, 3)
#define LCD_DMA                        DMA1
#define LCD_DMA_Stream                 DMA1_Stream7
#define LCD_DMA_Stream_IRQn            DMA1_Stream7_IRQn
#define LCD_DMA_Stream_IRQHandler      DMA1_Stream7_IRQHandler
#define LCD_DMA_FLAGS                  (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
#define LCD_DMA_FLAG_INT               DMA_HIFCR_CTCIF7

// SDIO, not the SPI2 storage connection used by most monochrome targets.
#define STORAGE_USE_SDIO
#define SD_SDIO_DMA                    DMA2
#define SD_SDIO_DMA_STREAM             DMA2_Stream3
#define SD_SDIO_DMA_CHANNEL            LL_DMA_CHANNEL_4
#define SD_SDIO_DMA_IRQn               DMA2_Stream3_IRQn
#define SD_SDIO_DMA_IRQHANDLER         DMA2_Stream3_IRQHandler
#define SD_SDIO_CLK_DIV(fq)            ((48000000 / (fq)) - 2)
#define SD_SDIO_TRANSFER_CLK_DIV       SD_SDIO_CLK_DIV(24000000)

#define AUDIO_OUTPUT_GPIO              GPIO_PIN(GPIOA, 4)
#define AUDIO_MUTE_GPIO                GPIO_PIN(GPIOE, 0)
#define AUDIO_DMA                      DMA1
#define AUDIO_DMA_Stream               DMA1_Stream5
#define AUDIO_DMA_Stream_IRQn          DMA1_Stream5_IRQn
#define AUDIO_DMA_Stream_IRQHandler    DMA1_Stream5_IRQHandler
#define AUDIO_TIMER                    TIM6

// FreedomTX anaIn(VBAT) / 4.55 returns hundredths of a volt. The generic
// BATT_SCALE branch returns counts * BATT_SCALE * 128 / BATTERY_DIVIDER.
// Include both scale factors so its nominal result matches FreedomTX.
#define ADC_VREF_PREC2                 300
#define BATTERY_DIVIDER                (4.55f * BATT_SCALE * 128.0f)

// Direct Hall acquisition owns SPI2; internal SX1281 reset stays asserted.
#define MAMBO_RF_RESET_GPIO            GPIO_PIN(GPIOC, 13)
void mamboEarlyInit();
