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
#if defined(RADIO_GAJS)
  #define KEYS_GPIO_REG_SYS             GPIOB
  #define KEYS_GPIO_PIN_SYS             LL_GPIO_PIN_4  // PB.04
  #define KEYS_GPIO_REG_EXIT            GPIOC
  #define KEYS_GPIO_PIN_EXIT            LL_GPIO_PIN_5  // PC.05
  #define KEYS_GPIO_REG_PAGEUP          GPIOD
  #define KEYS_GPIO_PIN_PAGEUP          LL_GPIO_PIN_3  // PD.03
  #define KEYS_GPIO_REG_PAGEDN          GPIOD
  #define KEYS_GPIO_PIN_PAGEDN          LL_GPIO_PIN_7  // PD.07
  #define KEYS_GPIO_REG_MDL             GPIOE
  #define KEYS_GPIO_PIN_MDL             LL_GPIO_PIN_11 // PE.11
  #define KEYS_GPIO_REG_ENTER           GPIOA
  #define KEYS_GPIO_PIN_ENTER           LL_GPIO_PIN_13  // PA.13
  #define KEYS_GPIO_REG_TELE            GPIOD
  #define KEYS_GPIO_PIN_TELE            LL_GPIO_PIN_2  // PD.02
#endif

// Rotary Encoder
#if defined(RADIO_GAJS)
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
    #if defined(RADIO_GAJS)
    #define ROTARY_ENCODER_INVERTED
  #endif
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  #define ROTARY_ENCODER_TIMER            TIM5
  #define ROTARY_ENCODER_TIMER_IRQn       TIM5_IRQn
  #define ROTARY_ENCODER_TIMER_IRQHandler TIM5_IRQHandler
#endif

// Trims
#if defined(RADIO_GAJS)
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
#endif

// Switches
#if defined(RADIO_GAJS)
  #define SWITCHES_GPIO_REG_A           GPIOC
  #define SWITCHES_GPIO_PIN_A           LL_GPIO_PIN_13  // PC.13
  #define SWITCHES_GPIO_REG_B_L         GPIOE
  #define SWITCHES_GPIO_PIN_B_L         LL_GPIO_PIN_15 // PE.15
  #define SWITCHES_GPIO_REG_B_H         GPIOA
  #define SWITCHES_GPIO_PIN_B_H         LL_GPIO_PIN_5  // PA.05
  #define SWITCHES_GPIO_REG_C_L         GPIOE
  #define SWITCHES_GPIO_PIN_C_L         LL_GPIO_PIN_0   // PE.00
  #define SWITCHES_GPIO_REG_C_H         GPIOD
  #define SWITCHES_GPIO_PIN_C_H         LL_GPIO_PIN_11  // PD.11
  #define SWITCHES_GPIO_REG_D           GPIOE
  #define SWITCHES_GPIO_PIN_D           LL_GPIO_PIN_8  // PE.08
  #define SWITCHES_GPIO_REG_E           GPIOE
  #define SWITCHES_GPIO_PIN_E           LL_GPIO_PIN_7  // PE.07
  #define SWITCHES_GPIO_REG_F           GPIOE
  #define SWITCHES_GPIO_PIN_F           LL_GPIO_PIN_1 // PE.01
  #define SWITCHES_GPIO_REG_G           GPIOE
  #define SWITCHES_GPIO_PIN_G           LL_GPIO_PIN_14 // PE.14
  #define SWITCHES_GPIO_REG_H           GPIOD
  #define SWITCHES_GPIO_PIN_H           LL_GPIO_PIN_14 // PD.14
#endif

// ADC
#define ADC_MAIN                      ADC1
#define ADC_DMA                       DMA2
#define ADC_DMA_CHANNEL               LL_DMA_CHANNEL_0
#define ADC_DMA_STREAM                LL_DMA_STREAM_4
#define ADC_DMA_STREAM_IRQ            DMA2_Stream4_IRQn
#define ADC_DMA_STREAM_IRQHandler     DMA2_Stream4_IRQHandler

#define ADC_SAMPTIME                    LL_ADC_SAMPLINGTIME_28CYCLES
#define ADC_CHANNEL_RTC_BAT             LL_ADC_CHANNEL_VBAT

#if defined(RADIO_GAJS)
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
#endif 
  #define ADC_GPIOB_PINS                ADC_GPIO_PIN_POT1
  #define ADC_GPIOC_PINS                ADC_GPIO_PIN_BATT
  #define ADC_CHANNEL_POT1              LL_ADC_CHANNEL_8
  #define ADC_CHANNEL_POT2              LL_ADC_CHANNEL_6
  #define ADC_CHANNEL_BATT              LL_ADC_CHANNEL_10
  #define ADC_VREF_PREC2                330

#if defined(RADIO_GAJS)
  #define ADC_DIRECTION {-1, 1, 1, -1, -1, 1, 1, 1}
#endif

// PWR and LED driver
#define PWR_SWITCH_GPIO               GPIO_PIN(GPIOD, 1)  // PD.01
#define PWR_ON_GPIO                   GPIO_PIN(GPIOD, 0)  // PD.00

#if defined(RADIO_GAJS)
  #define STATUS_LEDS
  #define GPIO_LED_GPIO_ON              gpio_set
  #define GPIO_LED_GPIO_OFF             gpio_clear
  #define LED_RED_GPIO                  GPIO_PIN(GPIOE, 13) // PE.13
  #define LED_GREEN_GPIO                GPIO_PIN(GPIOE, 2)  // PE.02
  #define LED_BLUE_GPIO                 GPIO_PIN(GPIOA, 7)  // PA.07
#endif

// Internal Module
#define INTMODULE_BOOTCMD_GPIO           GPIO_PIN(GPIOB, 1) // PB.01
#define INTMODULE_PWR_GPIO               GPIO_PIN(GPIOC, 4) // PC.04

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

// External Module
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

// Trainer Port
#define TRAINER_IN_GPIO               GPIO_PIN(GPIOC, 8) // PC.08
#define TRAINER_IN_TIMER_Channel      LL_TIM_CHANNEL_CH3
#define TRAINER_OUT_GPIO              GPIO_PIN(GPIOC, 9) // PC.09
#define TRAINER_OUT_TIMER_Channel     LL_TIM_CHANNEL_CH4
#define TRAINER_DETECT_INVERTED
#define TRAINER_TIMER                 TIM3
#define TRAINER_TIMER_IRQn            TIM3_IRQn
#define TRAINER_GPIO_AF               LL_GPIO_AF_2
#define TRAINER_TIMER_IRQn            TIM3_IRQn
#define TRAINER_TIMER_IRQHandler      TIM3_IRQHandler
#define TRAINER_TIMER_FREQ            (PERI1_FREQUENCY * TIMER_MULT_APB1)

// Serial Port
#define HARDWARE_TRAINER_AUX_SERIAL
#define AUX_SERIAL_GPIO                   GPIOB
#define AUX_SERIAL_TX_GPIO                GPIO_PIN(GPIOB, 10) // PB.10
#define AUX_SERIAL_RX_GPIO                GPIO_PIN(GPIOB, 11) // PB.11
#define AUX_SERIAL_USART                  USART3
#define AUX_SERIAL_USART_IRQn             USART3_IRQn
#define AUX_SERIAL_DMA_RX                 DMA1
#define AUX_SERIAL_DMA_RX_STREAM          LL_DMA_STREAM_1
#define AUX_SERIAL_DMA_RX_CHANNEL         LL_DMA_CHANNEL_4

// Telemetry
#define TELEMETRY_DIR_GPIO              GPIO_PIN(GPIOD, 4) // PD.04
#define TELEMETRY_SET_INPUT           0
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

// PCBREV - not used
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
  #define SPORT_MAX_BAUDRATE            400000

// Heartbeat for iXJT / ISRM synchro - no

// Trainer / Trainee from the module bay
#define TRAINER_MODULE_CPPM_TIMER            TIM3
#define TRAINER_MODULE_CPPM_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define TRAINER_MODULE_CPPM_GPIO             EXTMODULE_RX_GPIO
#define TRAINER_MODULE_CPPM_TIMER_Channel    LL_TIM_CHANNEL_CH2
#define TRAINER_MODULE_CPPM_TIMER_IRQn       TIM3_IRQn
#define TRAINER_MODULE_CPPM_GPIO_AF          LL_GPIO_AF_2

#if defined(INTMODULE_HEARTBEAT_GPIO) && defined(HARDWARE_EXTERNAL_MODULE)
  // Trainer CPPM input on heartbeat pin
  #define TRAINER_MODULE_CPPM_TIMER               TRAINER_TIMER
  #define TRAINER_MODULE_CPPM_FREQ                (PERI1_FREQUENCY * TIMER_MULT_APB1)
  #define TRAINER_MODULE_CPPM_GPIO                INTMODULE_HEARTBEAT_GPIO
  #define TRAINER_MODULE_CPPM_TIMER_Channel       LL_TIM_CHANNEL_CH2
  #define TRAINER_MODULE_CPPM_TIMER_IRQn          TRAINER_TIMER_IRQn
  #define TRAINER_MODULE_CPPM_GPIO_AF             GPIO_AF2
  // Trainer SBUS input on heartbeat pin
  #define TRAINER_MODULE_SBUS_USART               USART6
  #define TRAINER_MODULE_SBUS_USART_IRQn          USART6_IRQn
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
#define BACKLIGHT_TIMER_FREQ          (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define BACKLIGHT_TIMER               TIM4
#define BACKLIGHT_GPIO                GPIO_PIN(GPIOD, 13) // PD.13
#define BACKLIGHT_GPIO_AF             GPIO_AF2
#define BACKLIGHT_CCMR1               TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 // Channel2, PWM
#define BACKLIGHT_CCER                TIM_CCER_CC2E
#define BACKLIGHT_COUNTER_REGISTER    BACKLIGHT_TIMER->CCR2

// LCD driver
#if defined(RADIO_GAJS)
  #define SSD1309_LCD
#endif

#define LCD_MOSI_GPIO                 GPIO_PIN(GPIOC, 12) // PC.12
#define LCD_CLK_GPIO                  GPIO_PIN(GPIOC, 10) // PC.10
#define LCD_A0_GPIO                   GPIO_PIN(GPIOC, 11) // PC.11
#define LCD_NCS_GPIO                  GPIO_PIN(GPIOA, 15) // PA.15

#define LCD_DMA                       DMA1
#define LCD_DMA_Stream                DMA1_Stream7
#define LCD_DMA_Stream_IRQn           DMA1_Stream7_IRQn
#define LCD_DMA_Stream_IRQHandler     DMA1_Stream7_IRQHandler
#define LCD_DMA_FLAGS                 (DMA_HIFCR_CTCIF7 | DMA_HIFCR_CHTIF7 | DMA_HIFCR_CTEIF7 | DMA_HIFCR_CDMEIF7 | DMA_HIFCR_CFEIF7)
#define LCD_DMA_FLAG_INT              DMA_HIFCR_CTCIF7
#define LCD_SPI                       SPI3
#define LCD_GPIO_AF                   GPIO_AF6

#if defined(SSD1309_LCD)
  #define LCD_SPI_PRESCALER             SPI_CR1_BR_1
#else
  #define LCD_SPI_PRESCALER             0
#endif

// I2C Bus 1: EEPROM and CAT5137 digital pot for volume control
#define I2C_B1                          I2C1
#define I2C_B1_GPIO_AF                  LL_GPIO_AF_4

#define I2C_B1_SCL_GPIO               GPIO_PIN(GPIOB, 8)  // PB.08
#define I2C_B1_SDA_GPIO               GPIO_PIN(GPIOB, 9)  // PB.09

#define EEPROM_WP_GPIO                GPIOD
#define EEPROM_WP_GPIO_PIN            LL_GPIO_PIN_10  // PD.10

// I2C Volume control
#if !defined(SOFTWARE_VOLUME)
  #define VOLUME_I2C_ADDRESS            0x2E
  #define VOLUME_I2C_BUS                I2C_Bus_1

  #include <stdint.h>
  int32_t getVolume();
#endif

#define I2C_B1_CLK_RATE                 400000

// EEPROM
#define EEPROM_I2C_ADDRESS              0x51
#define EEPROM_I2C_BUS                  I2C_Bus_1
#define EEPROM_PAGESIZE                 16
#define EEPROM_SIZE                     256

// SD - SPI2
#if defined(RADIO_GAJS)
  // Using chip, so no detect
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

#define AUDIO_MUTE_GPIO               GPIO_PIN(GPIOE, 12)
#define AUDIO_MUTE_DELAY              500  // ms

// Haptic
#define HAPTIC_PWM
#define HAPTIC_GPIO                   GPIO_PIN(GPIOB, 3) // PB.03
#define HAPTIC_GPIO_AF                GPIO_AF1
#define HAPTIC_TIMER                  TIM2 // Timer 2 Channel1
#define HAPTIC_TIMER_FREQ             (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define HAPTIC_COUNTER_REGISTER       HAPTIC_TIMER->CCR2
#define HAPTIC_CCMR1                  TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2
#define HAPTIC_CCER                   TIM_CCER_CC2E
#define BACKLIGHT_BDTR                TIM_BDTR_MOE

// Bluetooth
#define BT_TX_GPIO                    GPIO_PIN(GPIOB, 10) // PB.10
#define BT_RX_GPIO                    GPIO_PIN(GPIOB, 11) // PB.11
#define BT_USART                      USART3
#define BT_USART_IRQn                 USART3_IRQn
// #define BT_DMA_Stream_RX              DMA1_Stream1
// #define BT_DMA_Channel_RX             DMA_Channel_4

// Millisecond timer
#define MS_TIMER                        TIM14
#define MS_TIMER_IRQn                   TIM8_TRG_COM_TIM14_IRQn
#define MS_TIMER_IRQHandler             TIM8_TRG_COM_TIM14_IRQHandler

// Mixer scheduler timer
#define MIXER_SCHEDULER_TIMER                TIM12
#define MIXER_SCHEDULER_TIMER_FREQ           (PERI1_FREQUENCY * TIMER_MULT_APB1)
#define MIXER_SCHEDULER_TIMER_IRQn           TIM8_BRK_TIM12_IRQn
#define MIXER_SCHEDULER_TIMER_IRQHandler  

// Keys held together to toggle the keyboard lock. Override either define
// in this file to map the combo to a different pair on this target.
#define KEYS_LOCK_KEY1                  KEY_SYS
#define KEYS_LOCK_KEY2                  KEY_MODEL

