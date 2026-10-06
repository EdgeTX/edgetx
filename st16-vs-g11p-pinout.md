# ST16 vs G11P MCU Pinout Mapping

FlySky ST16 vs FlySky G11P - STM32H750IBT6 (LQFP176) pin-by-pin net comparison.

Format mirrors `PL20-PL18U.xlsx`. Generated 2026-10-06.

## Sources

- **ST16 net labels**: `FS-ST16-Main V1.5 20240426.pdf`, sheet 4 of 11 ("ST32H750IBT6_LQFP176-24X24MM" MCU sheet).
- **G11P net labels**: `FS-G11P-V1.0原理图及点位图-20250109.pdf`, MCU sheet (page 2, `STM32H750IBT6_LQFP176`).
- **Pin name / pin number**: paired directly from the schematic symbol edges (top 133-176, left 1-44, bottom 45-88, right 89-132) via `pdftotext -bbox` coordinate clustering. Cross-checked against `radio/src/targets/st16/{hal.h,board.h,bsp_io.h,sdram_driver.cpp,lcd_driver.cpp,extflash_driver.cpp}`.

## How to read

- `Diff? = YES` marks a pin whose net differs between the two boards. All other I/O pins are net-identical.
- `-` = pin not assigned a schematic net (power/ground/NC or unused).

## Delta summary (34 differing I/O pins)

| Pin | Pin # | ST16 | G11P |
|---|---:|---|---|
| PE5 | 4 | PPM_IN | RF_PWR_ON |
| PE6 | 5 | PPM_OUT | TP_INT |
| PC13 | 8 | DFU_DET | DFU |
| PI9 | 11 | UART4_RX_HEAD | TP_RESET |
| PI11 | 13 | BACK_KEY4 | LCD_NRST |
| PF6 | 24 | SWF | PPM_IN |
| PF7 | 25 | SWD | PPM_OUT |
| PC0 | 32 | VBUS_ADC | 2S_ADC |
| PC2_C | 34 | SWA | VR2A |
| PC3_C | 35 | SWB | SW4_ADC |
| PA0 | 40 | CH1 | TH_ADC |
| PA1 | 41 | CH2 | ST_ADC |
| PA2 | 42 | CH3 | TR3_ADC |
| PH4 | 45 | BAT2_ADC | SW1_ADC |
| PA3 | 47 | CH4 | TR4_ADC |
| PA6 | 52 | VRA | SW2_ADC |
| PA7 | 53 | VRC | SW3_ADC |
| PC5 | 55 | KEY_EXIT | TR_ADC |
| PB0 | 56 | VRD | K12_ADC |
| PB1 | 57 | VRB | K34_ADC |
| PH6 | 83 | PCA9555PW_NINT | USB_SW |
| PH7 | 84 | I2C3_SCL | CHARGE_ENB |
| PH8 | 85 | I2C3_SDA | LCD_CS |
| PB12 | 92 | UART5_RX_SPORT | RF_DFU_C |
| PB13 | 93 | UART5_TX_SPORT | LED_DATA2 |
| PC7 | 116 | GEMO_NINT | K6A |
| PA8 | 119 | KEY_MENU | K7A |
| PI3 | 134 | BACK_KEY3 | VBUS_I |
| PA15 | 138 | SHINE_LAMP_PWM | LED_DATA_IN |
| PD5 | 147 | HALL_RX | PA_NMUTE |
| PD6 | 150 | HALL_TX | VS1053B_RST |
| PB7 | 165 | SENSOR_SDA | HW_VER |
| PB8 | 167 | SENSOR_SCL | TP_SCL |
| PB9 | 168 | UART4_TX_PPM | TP_SDA |

## Full 176-pin table

| # | Pin Name | Pin # | Net Label (ST16) | Net Label (G11P) | Diff? |
|---:|---|---:|---|---|---|
| 1 | PE2 | 1 | QSPI_BK1_IO2 | QSPI_BK1_IO2 |  |
| 2 | PE3 | 2 | LCD_SDA | LCD_SDA |  |
| 3 | PE4 | 3 | LCD_SCL | LCD_SCL |  |
| 4 | PE5 | 4 | PPM_IN | RF_PWR_ON | YES |
| 5 | PE6 | 5 | PPM_OUT | TP_INT | YES |
| 6 | VBAT | 6 | - | - |  |
| 7 | PI8 | 7 | PWR_CTR | PWR_CTR |  |
| 8 | PC13 | 8 | DFU_DET | DFU | YES |
| 9 | PC14-OSC32_IN | 9 | OSC32_IN | OSC32_IN |  |
| 10 | PC15-OSC32_OUT | 10 | OSC32_OUT | OSC32_OUT |  |
| 11 | PI9 | 11 | UART4_RX_HEAD | TP_RESET | YES |
| 12 | PI10 | 12 | LCD_HSYNC | LCD_HSYNC |  |
| 13 | PI11 | 13 | BACK_KEY4 | LCD_NRST | YES |
| 14 | VSS | 14 | - | - |  |
| 15 | VDD | 15 | - | - |  |
| 16 | PF0 | 16 | FMC_A0 | FMC_A0 |  |
| 17 | PF1 | 17 | FMC_A1 | FMC_A1 |  |
| 18 | PF2 | 18 | FMC_A2 | FMC_A2 |  |
| 19 | PF3 | 19 | FMC_A3 | FMC_A3 |  |
| 20 | PF4 | 20 | FMC_A4 | FMC_A4 |  |
| 21 | PF5 | 21 | FMC_A5 | FMC_A5 |  |
| 22 | VSS | 22 | - | - |  |
| 23 | VDD | 23 | - | - |  |
| 24 | PF6 | 24 | SWF | PPM_IN | YES |
| 25 | PF7 | 25 | SWD | PPM_OUT | YES |
| 26 | PF8 | 26 | SWE | SWE |  |
| 27 | PF9 | 27 | SWC | SWC |  |
| 28 | PF10 | 28 | LCD_DE | LCD_DE |  |
| 29 | PH0-OSC_IN | 29 | OSC_IN | OSC_IN |  |
| 30 | PH1-OSC_OUT | 30 | OSC_OUT | OSC_OUT |  |
| 31 | NRST | 31 | M7_NRST | M7_NRST |  |
| 32 | PC0 | 32 | VBUS_ADC | 2S_ADC | YES |
| 33 | PC1 | 33 | LIBAT_ADC | LIBAT_ADC |  |
| 34 | PC2_C | 34 | SWA | VR2A | YES |
| 35 | PC3_C | 35 | SWB | SW4_ADC | YES |
| 36 | VDD | 36 | - | - |  |
| 37 | VSSA | 37 | - | - |  |
| 38 | VREF+ | 38 | - | - |  |
| 39 | VDDA | 39 | - | - |  |
| 40 | PA0 | 40 | CH1 | TH_ADC | YES |
| 41 | PA1 | 41 | CH2 | ST_ADC | YES |
| 42 | PA2 | 42 | CH3 | TR3_ADC | YES |
| 43 | PH2 | 43 | FMC_CKE | FMC_CKE |  |
| 44 | PH3 | 44 | FMC_NCS | FMC_NCS |  |
| 45 | PH4 | 45 | BAT2_ADC | SW1_ADC | YES |
| 46 | PH5 | 46 | FMC_NWE | FMC_NWE |  |
| 47 | PA3 | 47 | CH4 | TR4_ADC | YES |
| 48 | VSS | 48 | - | - |  |
| 49 | VDD | 49 | - | - |  |
| 50 | PA4 | 50 | LCD_VSYNC | LCD_VSYNC |  |
| 51 | PA5 | 51 | SPI1_SCK | SPI1_SCK |  |
| 52 | PA6 | 52 | VRA | SW2_ADC | YES |
| 53 | PA7 | 53 | VRC | SW3_ADC | YES |
| 54 | PC4 | 54 | LCD_ID | LCD_ID |  |
| 55 | PC5 | 55 | KEY_EXIT | TR_ADC | YES |
| 56 | PB0 | 56 | VRD | K12_ADC | YES |
| 57 | PB1 | 57 | VRB | K34_ADC | YES |
| 58 | PB2 | 58 | QSPI_CLK | QSPI_CLK |  |
| 59 | PF11 | 59 | FMC_NRAS | FMC_NRAS |  |
| 60 | PF12 | 60 | FMC_A6 | FMC_A6 |  |
| 61 | VSS | 61 | - | - |  |
| 62 | VDD | 62 | - | - |  |
| 63 | PF13 | 63 | FMC_A7 | FMC_A7 |  |
| 64 | PF14 | 64 | FMC_A8 | FMC_A8 |  |
| 65 | PF15 | 65 | FMC_A9 | FMC_A9 |  |
| 66 | PG0 | 66 | FMC_A10 | FMC_A10 |  |
| 67 | PG1 | 67 | FMC_A11 | FMC_A11 |  |
| 68 | PE7 | 68 | FMC_DQ4 | FMC_DQ4 |  |
| 69 | PE8 | 69 | FMC_DQ5 | FMC_DQ5 |  |
| 70 | PE9 | 70 | FMC_DQ6 | FMC_DQ6 |  |
| 71 | VSS | 71 | - | - |  |
| 72 | VDD | 72 | - | - |  |
| 73 | PE10 | 73 | FMC_DQ7 | FMC_DQ7 |  |
| 74 | PE11 | 74 | FMC_DQ8 | FMC_DQ8 |  |
| 75 | PE12 | 75 | FMC_DQ9 | FMC_DQ9 |  |
| 76 | PE13 | 76 | FMC_DQ10 | FMC_DQ10 |  |
| 77 | PE14 | 77 | FMC_DQ11 | FMC_DQ11 |  |
| 78 | PE15 | 78 | FMC_DQ12 | FMC_DQ12 |  |
| 79 | PB10 | 79 | M0_USART2_RX | M0_USART2_RX |  |
| 80 | PB11 | 80 | M0_USART2_TX | M0_USART2_TX |  |
| 81 | VCAP | 81 | VCAP1 | VCAP1 |  |
| 82 | VDD | 82 | - | - |  |
| 83 | PH6 | 83 | PCA9555PW_NINT | USB_SW | YES |
| 84 | PH7 | 84 | I2C3_SCL | CHARGE_ENB | YES |
| 85 | PH8 | 85 | I2C3_SDA | LCD_CS | YES |
| 86 | PH9 | 86 | LCD_R3 | LCD_R3 |  |
| 87 | PH10 | 87 | LCD_R4 | LCD_R4 |  |
| 88 | PH11 | 88 | LCD_R5 | LCD_R5 |  |
| 89 | PH12 | 89 | LCD_R6 | LCD_R6 |  |
| 90 | VSS | 90 | - | - |  |
| 91 | VDD | 91 | - | - |  |
| 92 | PB12 | 92 | UART5_RX_SPORT | RF_DFU_C | YES |
| 93 | PB13 | 93 | UART5_TX_SPORT | LED_DATA2 | YES |
| 94 | PB14 | 94 | FLASH_SPI2_MISO | FLASH_SPI2_MISO |  |
| 95 | PB15 | 95 | FLASH_SPI2_MOSI | FLASH_SPI2_MOSI |  |
| 96 | PD8 | 96 | FMC_DQ13 | FMC_DQ13 |  |
| 97 | PD9 | 97 | FMC_DQ14 | FMC_DQ14 |  |
| 98 | PD10 | 98 | FMC_DQ15 | FMC_DQ15 |  |
| 99 | PD11 | 99 | QSPI_BK1_IO0 | QSPI_BK1_IO0 |  |
| 100 | PD12 | 100 | QSPI_BK1_IO1 | QSPI_BK1_IO1 |  |
| 101 | PD13 | 101 | QSPI_BK1_IO3 | QSPI_BK1_IO3 |  |
| 102 | VSS | 102 | - | - |  |
| 103 | VDD | 103 | - | - |  |
| 104 | PD14 | 104 | FMC_DQ0 | FMC_DQ0 |  |
| 105 | PD15 | 105 | FMC_DQ1 | FMC_DQ1 |  |
| 106 | PG2 | 106 | W-A | W-A |  |
| 107 | PG3 | 107 | W-B | W-B |  |
| 108 | PG4 | 108 | FMC_BA0 | FMC_BA0 |  |
| 109 | PG5 | 109 | FMC_BA1 | FMC_BA1 |  |
| 110 | PG6 | 110 | LCD_R7 | LCD_R7 |  |
| 111 | PG7 | 111 | LCD_DOCLK | LCD_DOCLK |  |
| 112 | PG8 | 112 | FMC_CLK | FMC_CLK |  |
| 113 | VSS | 113 | - | - |  |
| 114 | VDD33USB | 114 | - | - |  |
| 115 | PC6 | 115 | BACKLIGHT_PWM | BACKLIGHT_PWM |  |
| 116 | PC7 | 116 | GEMO_NINT | K6A | YES |
| 117 | PC8 | 117 | SD_D0 | SD_D0 |  |
| 118 | PC9 | 118 | SD_D1 | SD_D1 |  |
| 119 | PA8 | 119 | KEY_MENU | K7A | YES |
| 120 | PA9 | 120 | FLASH_SPI2_SCK | FLASH_SPI2_SCK |  |
| 121 | PA10 | 121 | M0_SYNC | M0_SYNC |  |
| 122 | PA11 | 122 | M7_FS1_DM | M7_FS1_DM |  |
| 123 | PA12 | 123 | M7_FS1_DP | M7_FS1_DP |  |
| 124 | PA13 | 124 | M7_SWDIO | M7_SWDIO |  |
| 125 | VCAP | 125 | VCAP1 | VCAP1 |  |
| 126 | VSS | 126 | - | - |  |
| 127 | VDD | 127 | - | - |  |
| 128 | PH13 | 128 | LCD_G2 | LCD_G2 |  |
| 129 | PH14 | 129 | LCD_G3 | LCD_G3 |  |
| 130 | PH15 | 130 | LCD_G4 | LCD_G4 |  |
| 131 | PI0 | 131 | LCD_G5 | LCD_G5 |  |
| 132 | PI1 | 132 | LCD_G6 | LCD_G6 |  |
| 133 | PI2 | 133 | LCD_G7 | LCD_G7 |  |
| 134 | PI3 | 134 | BACK_KEY3 | VBUS_I | YES |
| 135 | VSS | 135 | - | - |  |
| 136 | VDD | 136 | - | - |  |
| 137 | PA14 | 137 | M7_SWCLK | M7_SWCLK |  |
| 138 | PA15 | 138 | SHINE_LAMP_PWM | LED_DATA_IN | YES |
| 139 | PC10 | 139 | SD_D2 | SD_D2 |  |
| 140 | PC11 | 140 | SD_D3 | SD_D3 |  |
| 141 | PC12 | 141 | SD_CLK | SD_CLK |  |
| 142 | PD0 | 142 | FMC_DQ2 | FMC_DQ2 |  |
| 143 | PD1 | 143 | FMC_DQ3 | FMC_DQ3 |  |
| 144 | PD2 | 144 | SD_CMD | SD_CMD |  |
| 145 | PD3 | 145 | SD_NDET | SD_NDET |  |
| 146 | PD4 | 146 | CHRG | CHRG |  |
| 147 | PD5 | 147 | HALL_RX | PA_NMUTE | YES |
| 148 | VSS | 148 | - | - |  |
| 149 | VDD | 149 | - | - |  |
| 150 | PD6 | 150 | HALL_TX | VS1053B_RST | YES |
| 151 | PD7 | 151 | SPI1_MOSI | SPI1_MOSI |  |
| 152 | PG9 | 152 | SPI1_MISO | SPI1_MISO |  |
| 153 | PG10 | 153 | SPI1_NCS | SPI1_NCS |  |
| 154 | PG11 | 154 | LCD_B3 | LCD_B3 |  |
| 155 | PG12 | 155 | VS1053B_BSYNC | VS1053B_BSYNC |  |
| 156 | PG13 | 156 | VS1053B_DREQ | VS1053B_DREQ |  |
| 157 | PG14 | 157 | W-KEY | W-KEY |  |
| 158 | VSS | 158 | - | - |  |
| 159 | VDD | 159 | - | - |  |
| 160 | PG15 | 160 | FMC_NCAS | FMC_NCAS |  |
| 161 | PB3 | 161 | PWR_SW | PWR_SW |  |
| 162 | PB4 | 162 | FLASH_SPI2_NCS | FLASH_SPI2_NCS |  |
| 163 | PB5 | 163 | MOTOR_PWM | MOTOR_PWM |  |
| 164 | PB6 | 164 | QSPI_BK1_NCS | QSPI_BK1_NCS |  |
| 165 | PB7 | 165 | SENSOR_SDA | HW_VER | YES |
| 166 | BOOT0 | 166 | BOOT0 | BOOT0 |  |
| 167 | PB8 | 167 | SENSOR_SCL | TP_SCL | YES |
| 168 | PB9 | 168 | UART4_TX_PPM | TP_SDA | YES |
| 169 | PE0 | 169 | FMC_LDQM | FMC_LDQM |  |
| 170 | PE1 | 170 | FMC_UDQM | FMC_UDQM |  |
| 171 | PDR_ON | 171 | PDR_ON | PDR_ON |  |
| 172 | VDD | 172 | - | - |  |
| 173 | PI4 | 173 | LCD_B4 | LCD_B4 |  |
| 174 | PI5 | 174 | LCD_B5 | LCD_B5 |  |
| 175 | PI6 | 175 | LCD_B6 | LCD_B6 |  |
| 176 | PI7 | 176 | LCD_B7 | LCD_B7 |  |

