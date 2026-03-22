# DMA Mapping of TX16S

## DMA1

| Peripheral requests | Stream 0              | Stream 1                               | Stream 2                           | Stream 3                    | Stream 4                    | Stream 5        | Stream 6                  | Stream 7              |
| ------------------- | --------------------- | -------------------------------------- | ---------------------------------- | --------------------------- | --------------------------- | --------------- | ------------------------- | --------------------- |
| Channel 0           | SPI3\_RX              | \-                                     | SPI3\_RX                           | SPI2\_RX                    | SPI2\_TX                    | SPI3\_TX        | \-                        | SPI3\_TX              |
| Channel 1           | I2C1\_RX              | \-                                     | TIM7\_UP                           | \-                          | TIM7\_UP                    | I2C1\_RX        | I2C1\_TX                  | I2C1\_TX              |
| Channel 2           | TIM4\_CH1             | \-                                     | I2S3\_EXT\_RX                      | TIM4\_CH2                   | I2S2\_EXT\_TX               | I2S3\_EXT\_TX   | TIM4\_UP                  | TIM4\_CH3             |
| Channel 3           | I2S3\_EXT\_RX         | **TIM2\_UP<br>TIM2\_CH3<br>(ACCESS\_MOD)** | I2C3\_RX                           | I2S2\_EXT\_RX               | I2C3\_TX                    | TIM2\_CH1       | TIM2\_CH2<br>TIM2\_CH4    | TIM2\_UP<br>TIM2\_CH4 |
| Channel 4           | UART5\_RX             | USART3\_RX                             | UART4\_RX                      | **USART3\_TX<br>(ACCESS\_MOD)** | **UART4\_TX<br>(FLYSKY\_HALL)** | USART2\_RX      | **USART2\_TX<br>(TELEMETRY)** | UART5\_TX             |
| Channel 5           | UART8\_TX             | UART7\_TX                              | **TIM3\_CH4<br>TIM3\_UP<br>(TRAINER)** | UART7\_RX                   | TIM3\_CH1<br>TIM3\_TRIG     | TIM3\_CH2       | UART8\_RX                 | TIM3\_CH3             |
| Channel 6           | TIM5\_CH3<br>TIM5\_UP | TIM5\_CH4<br>TIM5\_TRIG                | TIM5\_CH1                          | TIM5\_CH4<br>TIM5\_TRIG     | TIM5\_CH2                   | \-              | TIM5\_UP                  | \-                    |
| Channel 7           | \-                    | TIM6\_UP                               | I2C2\_RX                           | I2C2\_RX                    | USART3\_TX                  | **DAC1<br>(AUDIO)** | DAC2                      | I2C2\_TX              |

## DMA2

| Peripheral requests | Stream 0   | Stream 1             | Stream 2                            | Stream 3  | Stream 4                             | Stream 5                | Stream 6                            | Stream 7                             |
| ------------------- | ---------- | -------------------- | ----------------------------------- | --------- | ------------------------------------ | ----------------------- | ----------------------------------- | ------------------------------------ |
| Channel 0           | ADC1       | SAI1\_A              | TIM8\_CH1<br>TIM8\_CH2<br>TIM8\_CH3 | SAI1\_A   | ADC1                                 | SAI1\_B                 | TIM1\_CH1<br>TIM1\_CH2<br>TIM1\_CH3 | \-                                   |
| Channel 1           | \-         | DCMI                 | ADC2                                | ADC2      | SAI1\_B                              | SPI6\_TX                | SPI6\_RX                            | DCMI                                 |
| Channel 2           | **ADC3**   | ADC3                 | \-                                  | SPI5\_RX  | SPI5\_TX                             | CRYP\_OUT               | CRYP\_IN                            | HASH\_IN                             |
| Channel 3           | SPI1\_RX   | \-                   | SPI1\_RX                            | SPI1\_TX  | \-                                   | SPI1\_TX                | \-                                  | \-                                   |
| Channel 4           | SPI4\_RX   | SPI4\_TX             | USART1\_RX                          | **SDIO**  | \-                                   | USART1\_RX              | SDIO                                | **USART1\_TX<br>(INTMODULE)**        |
| Channel 5           | \-         | **USART6\_RX<br>(AUX2)** | USART6\_RX                          | SPI4\_RX  | SPI4\_TX                             | \-                      | USART6\_TX                          | USART6\_TX                           |
| Channel 6           | TIM1\_TRIG | TIM1\_CH1            | TIM1\_CH2                           | TIM1\_CH1 | TIM1\_CH4<br>TIM1\_TRIG<br>TIM1\_COM | **TIM1\_UP<br>(EXTMODULE)** | TIM1\_CH3                           | \-                                   |
| Channel 7           | \-         | TIM8\_UP             | TIM8\_CH1                           | TIM8\_CH2 | TIM8\_CH3                            | SPI5\_RX                | SPI5\_TX                            | TIM8\_CH4<br>TIM8\_TRIG<br>TIM8\_COM |
