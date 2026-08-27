---
hide:
  - toc
---

# IRQ Usage of TX16S

| IRQn | IRQ handler acronym                                                | Used for              | Priority | Prio Addr  | Description                                 | Address    |
| ---- | ------------------------------------------------------------------ | --------------------- | -------- | ---------- | ------------------------------------------- | ---------- |
|    0 | WWDG_IRQHandler                                                    | -                     | -        | 0xE000E400 | Window WatchDog                             | 0x00000040 |
|    1 | PVD_IRQHandler                                                     | -                     | -        | 0xE000E401 | PVD through EXTI Line detection             | 0x00000044 |
|    2 | TAMP_STAMP_IRQHandler                                              | -                     | -        | 0xE000E402 | Tamper and TimeStamps through the EXTI line | 0x00000048 |
|    3 | RTC_WKUP_IRQHandler                                                | -                     | -        | 0xE000E403 | RTC Wakeup through the EXTI line            | 0x0000004C |
|    4 | FLASH_IRQHandler                                                   | -                     | -        | 0xE000E404 | FLASH                                       | 0x00000050 |
|    5 | RCC_IRQHandler                                                     | -                     | -        | 0xE000E405 | RCC                                         | 0x00000054 |
|    6 | EXTI0_IRQHandler                                                   | -                     | -        | 0xE000E406 | EXTI Line0                                  | 0x00000058 |
|    7 | EXTI1_IRQHandler                                                   | -                     | -        | 0xE000E407 | EXTI Line1                                  | 0x0000005C |
|    8 | **EXTI2_IRQHandler**                                               | TOUCH_INT_EXTI        | 9        | 0xE000E408 | EXTI Line2                                  | 0x00000060 |
|    9 | EXTI3_IRQHandler                                                   | -                     | -        | 0xE000E409 | EXTI Line3                                  | 0x00000064 |
|   10 | EXTI4_IRQHandler                                                   | -                     | -        | 0xE000E40A | EXTI Line4                                  | 0x00000068 |
|   11 | DMA1_Stream0_IRQHandler                                            | -                     | -        | 0xE000E40B | DMA1 Stream 0                               | 0x0000006C |
|   12 | **DMA1_Stream1_IRQHandler<br>(only HARDWARE_EXTERNAL_ACCESS_MOD)** | EXTMODULE_TIMER_DMA   | 7        | 0xE000E40C | DMA1 Stream 1                               | 0x00000070 |
|   13 | DMA1_Stream2_IRQHandler                                            | -                     | -        | 0xE000E40D | DMA1 Stream 2                               | 0x00000074 |
|   14 | DMA1_Stream3_IRQHandler                                            | -                     | -        | 0xE000E40E | DMA1 Stream 3                               | 0x00000078 |
|   15 | **DMA1_Stream4_IRQHandler**                                        | FLYSKY_HALL_TX_DMA    | 7        | 0xE000E40F | DMA1 Stream 4                               | 0x0000007C |
|   16 | **DMA1_Stream5_IRQHandler**                                        | AUDIO_DMA             | 7        | 0xE000E410 | DMA1 Stream 5                               | 0x00000080 |
|   17 | **DMA1_Stream6_IRQHandler**                                        | TELEMETRY_DMA_TX      | 1        | 0xE000E411 | DMA1 Stream 6                               | 0x00000084 |
|   18 | ADC_IRQHandler                                                     | -                     | -        | 0xE000E412 | ADC1, ADC2 and ADC3s                        | 0x00000088 |
|   19 | CAN1_TX_IRQHandler                                                 | -                     | -        | 0xE000E413 | CAN1 TX                                     | 0x0000008C |
|   20 | CAN1_RX0_IRQHandler                                                | -                     | -        | 0xE000E414 | CAN1 RX0                                    | 0x00000090 |
|   21 | CAN1_RX1_IRQHandler                                                | -                     | -        | 0xE000E415 | CAN1 RX1                                    | 0x00000094 |
|   22 | CAN1_SCE_IRQHandler                                                | -                     | -        | 0xE000E416 | CAN1 SCE                                    | 0x00000098 |
|   23 | **EXTI9_5_IRQHandler**                                             | TELEMETRY_EXTI        | 0        | 0xE000E417 | External Line[9:5]s                         | 0x0000009C |
|   24 | TIM1_BRK_TIM9_IRQHandler                                           | -                     | -        | 0xE000E418 | TIM1 Break and TIM9                         | 0x000000A0 |
|   25 | TIM1_UP_TIM10_IRQHandler                                           | -                     | -        | 0xE000E419 | TIM1 Update and TIM10                       | 0x000000A4 |
|   26 | **TIM1_TRG_COM_TIM11_IRQHandler**                                  | TELEMETRY_TIMER       | 0        | 0xE000E41A | TIM1 Trigger and Commutation and TIM11      | 0x000000A8 |
|   27 | **TIM1_CC_IRQHandler**                                             | EXTMODULE_TIMER       | 7        | 0xE000E41B | TIM1 Capture Compare                        | 0x000000AC |
|   28 | **TIM2_IRQHandler<br>(only HARDWARE_EXTERNAL_ACCESS_MOD)**         | EXTMODULE_TIMER       | 7        | 0xE000E41C | TIM2                                        | 0x000000B0 |
|   29 | **TIM3_IRQHandler**                                                | TRAINER_TIMER         | 7        | 0xE000E41D | TIM3                                        | 0x000000B4 |
|   30 | **TIM4_IRQHandler**                                                | ROTARY_ENCODER_TIMER  | 7        | 0xE000E41E | TIM4                                        | 0x000000B8 |
|   31 | I2C1_EV_IRQHandler                                                 | -                     | -        | 0xE000E41F | I2C1 Event                                  | 0x000000BC |
|   32 | I2C1_ER_IRQHandler                                                 | -                     | -        | 0xE000E420 | I2C1 Error                                  | 0x000000C0 |
|   33 | I2C2_EV_IRQHandler                                                 | -                     | -        | 0xE000E421 | I2C2 Event                                  | 0x000000C4 |
|   34 | I2C2_ER_IRQHandler                                                 | -                     | -        | 0xE000E422 | I2C2 Error                                  | 0x000000C8 |
|   35 | SPI1_IRQHandler                                                    | -                     | -        | 0xE000E423 | SPI1                                        | 0x000000CC |
|   36 | SPI2_IRQHandler                                                    | -                     | -        | 0xE000E424 | SPI2                                        | 0x000000D0 |
|   37 | **USART1_IRQHandler**                                              | INTMODULE_USART       | 6        | 0xE000E425 | USART1                                      | 0x000000D4 |
|   38 | **USART2_IRQHandler**                                              | TELEMETRY_USART       | 6        | 0xE000E426 | USART2                                      | 0x000000D8 |
|   39 | **USART3_IRQHandler** | AUX_SERIAL_USART<br>(EXTMODULE_USART with HARDWARE_EXTERNAL_ACCESS_MOD) | 7<br>(6 with HARDWARE_EXTERNAL_ACCESS_MOD) | 0xE000E427 | USART3 | 0x000000DC |
|   40 | **EXTI15_10_IRQHandler**                                           | ROTARY_ENCODER_EXTI and INTMODULE_HEARTBEAT | rotaryEncoderInit() sets to 8, but is overridden by init_intmodule_heartbeat() to 5 | 0xE000E428 | External Line[15:10]s                       | 0x000000E0 |
|   41 | RTC_Alarm_IRQHandler                                               | -                     | -        | 0xE000E429 | RTC Alarm (A and B) through EXTI Line       | 0x000000E4 |
|   42 | OTG_FS_WKUP_IRQHandler                                             | -                     | -        | 0xE000E42A | USB OTG FS Wakeup through EXTI line         | 0x000000E8 |
|   43 | TIM8_BRK_TIM12_IRQHandler                                          | -                     | -        | 0xE000E42B | TIM8 Break and TIM12                        | 0x000000EC |
|   44 | **TIM8_UP_TIM13_IRQHandler**                                       | MIXER_SCHEDULER_TIMER | 5        | 0xE000E42C | TIM8 Update and TIM13                       | 0x000000F0 |
|   45 | **TIM8_TRG_COM_TIM14_IRQHandler**                                  | INTERRUPT_xMS         | 4<br>(bootloader uses 0) | 0xE000E42D | TIM8 Trigger and Commutation and TIM14 | 0x000000F4 |
|   46 | TIM8_CC_IRQHandler                                                 | -                     | -        | 0xE000E42E | TIM8 Capture Compare                        | 0x000000F8 |
|   47 | DMA1_Stream7_IRQHandler                                            | -                     | -        | 0xE000E42F | DMA1 Stream7                                | 0x000000FC |
|   48 | FSMC_IRQHandler                                                    | -                     | -        | 0xE000E430 | FSMC                                        | 0x00000100 |
|   49 | **SDIO_IRQHandler**                                                | SDIO                  | 0        | 0xE000E431 | SDIO                                        | 0x00000104 |
|   50 | TIM5_IRQHandler                                                    | -                     | -        | 0xE000E432 | TIM5                                        | 0x00000108 |
|   51 | SPI3_IRQHandler                                                    | -                     | -        | 0xE000E433 | SPI3                                        | 0x0000010C |
|   52 | UART4_IRQHandler                                                   | -                     | -        | 0xE000E434 | UART4                                       | 0x00000110 |
|   53 | UART5_IRQHandler                                                   | -                     | -        | 0xE000E435 | UART5                                       | 0x00000114 |
|   54 | **TIM6_DAC_IRQHandler**                                            | AUDIO_TIM             | 7        | 0xE000E436 | TIM6 and DAC1&2 underrun errors             | 0x00000118 |
|   55 | TIM7_IRQHandler                                                    | -                     | -        | 0xE000E437 | TIM7                                        | 0x0000011C |
|   56 | DMA2_Stream0_IRQHandler                                            | -                     | -        | 0xE000E438 | DMA2 Stream 0                               | 0x00000120 |
|   57 | DMA2_Stream1_IRQHandler                                            | -                     | -        | 0xE000E439 | DMA2 Stream 1                               | 0x00000124 |
|   58 | DMA2_Stream2_IRQHandler                                            | -                     | -        | 0xE000E43A | DMA2 Stream 2                               | 0x00000128 |
|   59 | **DMA2_Stream3_IRQHandler**                                        | SD_SDIO_DMA           | 1        | 0xE000E43B | DMA2 Stream 3                               | 0x0000012C |
|   60 | DMA2_Stream4_IRQHandler                                            | -                     | -        | 0xE000E43C | DMA2 Stream 4                               | 0x00000130 |
|   61 | ETH_IRQHandler                                                     | -                     | -        | 0xE000E43D | Ethernet                                    | 0x00000134 |
|   62 | ETH_WKUP_IRQHandler                                                | -                     | -        | 0xE000E43E | Ethernet Wakeup through EXTI line           | 0x00000138 |
|   63 | CAN2_TX_IRQHandler                                                 | -                     | -        | 0xE000E43F | CAN2 TX                                     | 0x0000013C |
|   64 | CAN2_RX0_IRQHandler                                                | -                     | -        | 0xE000E440 | CAN2 RX0                                    | 0x00000140 |
|   65 | CAN2_RX1_IRQHandler                                                | -                     | -        | 0xE000E441 | CAN2 RX1                                    | 0x00000144 |
|   66 | CAN2_SCE_IRQHandler                                                | -                     | -        | 0xE000E442 | CAN2 SCE                                    | 0x00000148 |
|   67 | **OTG_FS_IRQHandler**                                              | OTG_FS (USB)          | 11       | 0xE000E443 | USB OTG FS                                  | 0x0000014C |
|   68 | **DMA2_Stream5_IRQHandler**                                        | EXTMODULE_TIMER_DMA   | 7        | 0xE000E444 | DMA2 Stream 5                               | 0x00000150 |
|   69 | DMA2_Stream6_IRQHandler                                            | -                     | -        | 0xE000E445 | DMA2 Stream 6                               | 0x00000154 |
|   70 | **DMA2_Stream7_IRQHandler**                                        | INTMODULE_DMA         | 7        | 0xE000E446 | DMA2 Stream 7                               | 0x00000158 |
|   71 | **USART6_IRQHandler**                                              | AUX2_SERIAL_USART<br>(optional GPS_USART and BT_USART) | 7<br>(GPS 9, BT 6) | 0xE000E447 | USART6 | 0x0000015C |
|   72 | I2C3_EV_IRQHandler                                                 | -                     | -        | 0xE000E448 | I2C3 event                                  | 0x00000160 |
|   73 | I2C3_ER_IRQHandler                                                 | -                     | -        | 0xE000E449 | I2C3 error                                  | 0x00000164 |
|   74 | OTG_HS_EP1_OUT_IRQHandler                                          | -                     | -        | 0xE000E44A | USB OTG HS End Point 1 Out                  | 0x00000168 |
|   75 | OTG_HS_EP1_IN_IRQHandler                                           | -                     | -        | 0xE000E44B | USB OTG HS End Point 1 In                   | 0x0000016C |
|   76 | OTG_HS_WKUP_IRQHandler                                             | -                     | -        | 0xE000E44C | USB OTG HS Wakeup through EXTI              | 0x00000170 |
|   77 | OTG_HS_IRQHandler                                                  | -                     | -        | 0xE000E44D | USB OTG HS                                  | 0x00000174 |
|   78 | DCMI_IRQHandler                                                    | -                     | -        | 0xE000E44E | DCMI                                        | 0x00000178 |
|   79 | CRYP_IRQHandler                                                    | -                     | -        | 0xE000E44F | CRYP crypto                                 | 0x0000017C |
|   80 | HASH_RNG_IRQHandler                                                | -                     | -        | 0xE000E450 | Hash and Rng                                | 0x00000180 |
|   81 | FPU_IRQHandler                                                     | -                     | -        | 0xE000E451 | FPU                                         | 0x00000184 |
|   82 | UART7_IRQHandler                                                   | -                     | -        | 0xE000E452 | UART7                                       | 0x00000188 |
|   83 | UART8_IRQHandler                                                   | -                     | -        | 0xE000E453 | UART8                                       | 0x0000018C |
|   84 | SPI4_IRQHandler                                                    | -                     | -        | 0xE000E454 | SPI4                                        | 0x00000190 |
|   85 | SPI5_IRQHandler                                                    | -                     | -        | 0xE000E455 | SPI5                                        | 0x00000194 |
|   86 | SPI6_IRQHandler                                                    | -                     | -        | 0xE000E456 | SPI6                                        | 0x00000198 |
|   87 | SAI1_IRQHandler                                                    | -                     | -        | 0xE000E457 | SAI1                                        | 0x0000019C |
|   88 | **LTDC_IRQHandler**                                                | LTDC                  | 4<br>(LCD_Init sets first to 7) | 0xE000E458 | LTDC                 | 0x000001A0 |
|   89 | LTDC_ER_IRQHandler                                                 | -                     | -        | 0xE000E459 | LTDC error                                  | 0x000001A4 |
|   90 | DMA2D_IRQHandler                                                   | -                     | -        | 0xE000E45A | DMA2D                                       | 0x000001A8 |

All interrupt priorities higher than [configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY](https://github.com/EdgeTX/edgetx/blob/c5c142d38d2c3b5150180f43731182240a9ccbb2/radio/src/FreeRTOSConfig.h#L98-L102) must not call any FreeRTOS functions!
Note that on Cortex ARM architectures higher priorities have lower value.

With an in-circuit-debugger, the read out on a TX16S with only internal module configured resulted in the following NVIC register readouts (all values as hex values):

Interrupt enable state (NVIC_ISER) from 0xE000E100:
```
00 01 03 60 E0 31 42 08  80 00 00 01 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
```

Interrupt priority (NVIC_IPR) from 0xE000E400:
```
00 00 00 00 00 00 00 00  90 00 00 00 00 00 00 00
70 10 00 00 00 00 00 00  00 00 00 00 00 70 70 00
00 00 00 00 00 60 60 70  50 00 00 00 50 40 00 00
00 00 00 00 00 00 70 00  00 00 00 10 00 00 00 00
00 00 00 00 00 00 00 70  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  40 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00
```
