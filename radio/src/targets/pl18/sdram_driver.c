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

#include "board.h"

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

void SDRAM_GPIOConfig(void)
{
    /*
    ------------------------------------------------------------------------------------------------------------------------------------------------
     PC3  <-> FMC_SDCKE0 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   | PH3  <-> FMC_SDNE0 | PI0  <-> FMC_D24
                         | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   | PH5  <-> FMC_SDNWE | PI1  <-> FMC_D25
                         | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG4  <-> FMC_BA0   | PH8  <-> FMC_D16   | PI2  <-> FMC_D26
                         | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG5  <-> FMC_BA1   | PH9  <-> FMC_D17   | PI3  <-> FMC_D27
                         | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG8  <-> FMC_SDCLK | PH10 <-> FMC_D18   | PI4  <-> FMC_NBL2
                         | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG15 <-> FMC_NCAS  | PH11 <-> FMC_D19   | PI5  <-> FMC_NBL3
                         | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  |                    | PH12 <-> FMC_D20   | PI6  <-> FMC_D28
                         |                   | PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |                    | PH13 <-> FMC_D21   | PI7  <-> FMC_D29
                         |                   | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |                    | PH14 <-> FMC_D22   | PI9  <-> FMC_D30
                         |                   | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |                    | PH15 <-> FMC_D23   | PI10 <-> FMC_D31
                         |                   | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |                    |                    |
    */

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    /* GPIOC configuration */
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3 , GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 ;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* GPIOD configuration */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOE configuration */
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource0, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource1, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    /* GPIOF configuration */
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource0, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource1, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource2, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource3, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource4, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource5, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource11, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource12, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource13, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource14, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource15, GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* GPIOG configuration */
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource0 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource1 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource4 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource5 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource8 , GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource15 , GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_15;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    /* GPIOH configuration */
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource3, GPIO_AF_FMC);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource5, GPIO_AF_FMC);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
    GPIO_Init(GPIOH, &GPIO_InitStructure);
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


void SDRAM_Init(void)
{
  //delay funcion needed
  delaysInit();
  // Clocks must be enabled here, because the sdramInit is called before main
  RCC_AHB1PeriphClockCmd(SDRAM_RCC_AHB1Periph, ENABLE);
  RCC_AHB3PeriphClockCmd(SDRAM_RCC_AHB3Periph, ENABLE);

  /* GPIO configuration for FMC SDRAM bank */
  SDRAM_GPIOConfig();

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
