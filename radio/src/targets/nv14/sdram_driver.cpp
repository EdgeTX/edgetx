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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"

#include "board.h"
#include "stm32f4xx_fmc.h"

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_FULL_PAGE            ((uint16_t)0x0007)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_1              ((uint16_t)0x0010)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define GPIO_AF_FMC GPIO_AF12

extern "C" void SDRAM_GPIOConfig(void)
{
  /*-- GPIOs Configuration -----------------------------------------------------*/
  /*
    +-------------------+--------------------+--------------------+--------------------+
    +                       SDRAM pins assignment                                      +
    +-------------------+--------------------+--------------------+--------------------+
    | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   |
    | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   |
    | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG8  <-> FMC_SDCLK |
    | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG15 <-> FMC_NCAS  |
    | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    |--------------------+
    | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    |
    | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  |
    +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |
                        | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |
                        | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
                        | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
    +-------------------+--------------------+--------------------+
    | PC3 <-> FMC_SDCKE0|
    | PH3 <-> FMC_SDNE0 |
    | PH5 <-> FMC_SDNWE |
    +-------------------+
  */

  /* GPIOC configuration */
  gpio_init_af(GPIO_PIN(GPIOC, 3), GPIO_AF_FMC);

  /* GPIOH configuration */
  gpio_init_af(GPIO_PIN(GPIOH, 3), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOH, 5), GPIO_AF_FMC);

  /* GPIOD configuration */
  gpio_init_af(GPIO_PIN(GPIOD, 0), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 1), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 8), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 9), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 10), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 14), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOD, 15), GPIO_AF_FMC);

  /* GPIOE configuration */
  gpio_init_af(GPIO_PIN(GPIOE, 0), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 1), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 7), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 8), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 9), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 10), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 11), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 12), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 13), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 14), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOE, 15), GPIO_AF_FMC);

  /* GPIOF configuration */
  gpio_init_af(GPIO_PIN(GPIOF, 0), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 1), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 2), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 3), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 4), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 5), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 11), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 12), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 13), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 14), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOF, 15), GPIO_AF_FMC);

  /* GPIOG configuration */
  gpio_init_af(GPIO_PIN(GPIOG, 0), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOG, 1), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOG, 4), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOG, 5), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOG, 8), GPIO_AF_FMC);
  gpio_init_af(GPIO_PIN(GPIOG, 15), GPIO_AF_FMC);
}

void SDRAM_InitSequence(void)
{
  FMC_SDRAMCommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;

  /* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_CLK_Enabled;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  /* Step 4 --------------------------------------------------------------------*/
  /* Insert 100 ms delay */
  delay_ms(100);

  /* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_PALL;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  /* Step 6 --------------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_AutoRefresh;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 4;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
  /* Send the  first command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
  /* Send the second command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  /* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
         SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
         SDRAM_MODEREG_CAS_LATENCY_3 |
         SDRAM_MODEREG_OPERATING_MODE_STANDARD |
         SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  FMC_SDRAMCommandStructure.FMC_CommandMode = FMC_Command_Mode_LoadMode;
  FMC_SDRAMCommandStructure.FMC_CommandTarget = FMC_Command_Target_bank1;
  FMC_SDRAMCommandStructure.FMC_AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.FMC_ModeRegisterDefinition = tmpr;
  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
  /* Send the command */
  FMC_SDRAMCmdConfig(&FMC_SDRAMCommandStructure);

  /* Step 8 --------------------------------------------------------------------*/
  /* Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  FMC_SetRefreshCount(683);//904
  /* Wait until the SDRAM controller is ready */
  while(FMC_GetFlagStatus(FMC_Bank1_SDRAM, FMC_FLAG_Busy) != RESET) {
  }
}

extern "C" void SDRAM_Init(void)
{
  //delay funcion needed
  delaysInit();

  /* GPIO configuration for FMC SDRAM bank */
  SDRAM_GPIOConfig();

  /* Enable FMC clock */
  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FMC);

  /* FMC Configuration ---------------------------------------------------------*/
  FMC_SDRAMInitTypeDef FMC_SDRAMInitStructure;
  FMC_SDRAMTimingInitTypeDef FMC_SDRAMTimingInitStructure;

  /* FMC SDRAM Bank configuration */
  /* Timing configuration for 90 Mhz of SD clock frequency (168Mhz/2) */
  /* TMRD: 2 Clock cycles */
  FMC_SDRAMTimingInitStructure.FMC_LoadToActiveDelay    = 2;
  /* TXSR: min=70ns (7x11.11ns) */
  FMC_SDRAMTimingInitStructure.FMC_ExitSelfRefreshDelay = 7;
  /* TRAS: min=42ns (4x11.11ns) max=120k (ns) */
  FMC_SDRAMTimingInitStructure.FMC_SelfRefreshTime      = 4;
  /* TRC:  min=70 (7x11.11ns) */
  FMC_SDRAMTimingInitStructure.FMC_RowCycleDelay        = 7;
  /* TWR:  min=1+ 7ns (1+1x11.11ns) */
  FMC_SDRAMTimingInitStructure.FMC_WriteRecoveryTime    = 2;
  /* TRP:  20ns => 2x11.11ns */
  FMC_SDRAMTimingInitStructure.FMC_RPDelay              = 2;
  /* TRCD: 20ns => 2x11.11ns */
  FMC_SDRAMTimingInitStructure.FMC_RCDDelay             = 2;

  /* FMC SDRAM control configuration */
  FMC_SDRAMInitStructure.FMC_Bank = FMC_Bank1_SDRAM;
  /* Row addressing: [7:0] */
  FMC_SDRAMInitStructure.FMC_ColumnBitsNumber = FMC_ColumnBits_Number_8b;
  /* Column addressing: [11:0] */
  FMC_SDRAMInitStructure.FMC_RowBitsNumber = FMC_RowBits_Number_12b;
  FMC_SDRAMInitStructure.FMC_SDMemoryDataWidth = FMC_SDMemory_Width_16b;
  FMC_SDRAMInitStructure.FMC_InternalBankNumber = FMC_InternalBank_Number_4;
  FMC_SDRAMInitStructure.FMC_CASLatency = FMC_CAS_Latency_3;
  FMC_SDRAMInitStructure.FMC_WriteProtection = FMC_Write_Protection_Disable;
  FMC_SDRAMInitStructure.FMC_SDClockPeriod = FMC_SDClock_Period_2;
  FMC_SDRAMInitStructure.FMC_ReadBurst = FMC_Read_Burst_Enable;
  FMC_SDRAMInitStructure.FMC_ReadPipeDelay = FMC_ReadPipe_Delay_1;
  FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct = &FMC_SDRAMTimingInitStructure;

  /* FMC SDRAM bank initialization */
  FMC_SDRAMInit(&FMC_SDRAMInitStructure);

  /* FMC SDRAM device initialization sequence */
  SDRAM_InitSequence();
}
