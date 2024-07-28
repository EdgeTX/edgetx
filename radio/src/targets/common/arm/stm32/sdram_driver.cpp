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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_hal_ll.h"

#include "board.h"

#if defined(SDRAM_BANK1)
  #define SDRAM_BANK                FMC_SDRAM_BANK1
  #define COMMAND_TARGET            FMC_SDRAM_CMD_TARGET_BANK1
#elif defined(SDRAM_BANK2)
  #define SDRAM_BANK                FMC_SDRAM_BANK2
  #define COMMAND_TARGET            FMC_SDRAM_CMD_TARGET_BANK2
#else
  #error "No bank definition for SDRAM"
#endif

#define SDRAM_MEMORY_WIDTH    FMC_SDRAM_MEM_BUS_WIDTH_16
#define SDRAM_CAS_LATENCY     FMC_SDRAM_CAS_LATENCY_3
#define SDRAM_READBURST       FMC_SDRAM_RBURST_ENABLE
#define SDRAM_READPIPEDELAY   FMC_SDRAM_RPIPE_DELAY_1
#define SDRAM_CLK_PERIOD      FMC_SDRAM_CLOCK_PERIOD_2

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
    | PC3 <-> FMC_SDCKE0| PB5 <-> FMC_SDCKE1 |
    | PH3 <-> FMC_SDNE0 | PH6 <-> FMC_SDNE1  |
    | PH5 <-> FMC_SDNWE |                    |
    +-------------------++-------------------+
  */

#if defined(SDRAM_BANK1)
  /* GPIOC configuration */
  gpio_init_af(GPIO_PIN(GPIOC, 3), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
#else
  /* GPIOB configuration */
  gpio_init_af(GPIO_PIN(GPIOB, 5), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
#endif

  /* GPIOH configuration */
  gpio_init_af(GPIO_PIN(GPIOH, 5), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
#if defined(SDRAM_BANK1)
  gpio_init_af(GPIO_PIN(GPIOH, 3), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
#else
  gpio_init_af(GPIO_PIN(GPIOH, 6), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
#endif

  /* GPIOD configuration */
  gpio_init_af(GPIO_PIN(GPIOD, 0), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 1), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 8), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 9), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 10), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 14), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOD, 15), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);

  /* GPIOE configuration */
  gpio_init_af(GPIO_PIN(GPIOE, 0), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 1), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 7), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 8), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 9), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 10), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 11), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 12), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 13), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 14), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOE, 15), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);

  /* GPIOF configuration */
  gpio_init_af(GPIO_PIN(GPIOF, 0), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 1), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 2), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 3), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 4), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 5), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 11), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 12), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 13), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 14), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOF, 15), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);

  /* GPIOG configuration */
  gpio_init_af(GPIO_PIN(GPIOG, 0), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOG, 1), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOG, 4), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOG, 5), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOG, 8), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOG, 15), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
}

void SDRAM_InitSequence(void)
{
  FMC_SDRAM_CommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;

  /* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  FMC_SDRAMCommandStructure.CommandTarget = COMMAND_TARGET;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while((__FMC_SDRAM_GET_FLAG(FMC_SDRAM_DEVICE, FMC_SDRAM_FLAG_BUSY) != 0));

  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &FMC_SDRAMCommandStructure, 10);

  /* Step 4 --------------------------------------------------------------------*/
  /* Insert 100 ms delay */
  delay_ms(100);

  /* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_PALL;
  FMC_SDRAMCommandStructure.CommandTarget = COMMAND_TARGET;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
  /* Wait until the SDRAM controller is ready */
  while((__FMC_SDRAM_GET_FLAG(FMC_SDRAM_DEVICE, FMC_SDRAM_FLAG_BUSY) != 0));

  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &FMC_SDRAMCommandStructure, 10);

  /* Step 6 --------------------------------------------------------------------*/
    /* Configure a Auto-Refresh command */
    FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
    FMC_SDRAMCommandStructure.CommandTarget = COMMAND_TARGET;
    FMC_SDRAMCommandStructure.AutoRefreshNumber = 4;
    FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;

    /* Wait until the SDRAM controller is ready */
    while((__FMC_SDRAM_GET_FLAG(FMC_SDRAM_DEVICE, FMC_SDRAM_FLAG_BUSY) != 0));

    /* Send the command */
    FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &FMC_SDRAMCommandStructure, 10);

  /* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
         SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
         SDRAM_MODEREG_CAS_LATENCY_3 |
         SDRAM_MODEREG_OPERATING_MODE_STANDARD |
         SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  FMC_SDRAMCommandStructure.CommandTarget = COMMAND_TARGET;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = tmpr;

  /* Wait until the SDRAM controller is ready */
  while((__FMC_SDRAM_GET_FLAG(FMC_SDRAM_DEVICE, FMC_SDRAM_FLAG_BUSY) != 0));

  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_SDRAM_DEVICE, &FMC_SDRAMCommandStructure, 10);

  /* Step 8: Set the refresh rate counter - refer to section SDRAM refresh timer register in RM0455 */
  /* Set the device refresh rate
   * COUNT = [(SDRAM self refresh time / number of row) x  SDRAM CLK] – 20
           = [(64ms/4096) * 84MHz] - 20 = 1312 - 20 ~ 1292 */
  FMC_SDRAM_ProgramRefreshRate(FMC_SDRAM_DEVICE, 1292);
  /* Wait until the SDRAM controller is ready */
  while((__FMC_SDRAM_GET_FLAG(FMC_SDRAM_DEVICE, FMC_SDRAM_FLAG_BUSY) != 0));
}

extern "C" void SDRAM_Init(void)
{
  //delay funcion needed
  delaysInit();
  // Clocks must be enabled here, because the sdramInit is called before main
  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_FMC);

  /* GPIO configuration for FMC SDRAM bank */
  SDRAM_GPIOConfig();


  /* FMC Configuration ---------------------------------------------------------*/
  FMC_SDRAM_InitTypeDef  FMC_SDRAMInitStructure;
  FMC_SDRAM_TimingTypeDef  FMC_SDRAMTimingInitStructure;

  /* FMC SDRAM Bank configuration */
  /* Timing configuration for 84 Mhz of SD clock frequency (168Mhz/2) */
  /* TMRD: 2 Clock cycles */
  FMC_SDRAMTimingInitStructure.LoadToActiveDelay    = 2;
  /* TXSR: min=72ns (7x11.9ns) */
  FMC_SDRAMTimingInitStructure.ExitSelfRefreshDelay = 7;
  /* TRAS: min=42ns (4x11.9ns) max=120k (ns) */
  FMC_SDRAMTimingInitStructure.SelfRefreshTime      = 4;
  /* TRC:  min=60 (6x11.9ns) */
  FMC_SDRAMTimingInitStructure.RowCycleDelay        = 6;
  /* TWR:  2 CLK */
  FMC_SDRAMTimingInitStructure.WriteRecoveryTime    = 2;
  /* TRP:  15ns => 2x11.9ns */
  FMC_SDRAMTimingInitStructure.RPDelay              = 2;
  /* TRCD: 15ns => 2x11.9ns */
  FMC_SDRAMTimingInitStructure.RCDDelay             = 2;

  /* FMC SDRAM control configuration */
  FMC_SDRAMInitStructure.SDBank = SDRAM_BANK;
  /* Row addressing: [7:0] */
  FMC_SDRAMInitStructure.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  /* Column addressing: [11:0] */
  FMC_SDRAMInitStructure.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  FMC_SDRAMInitStructure.MemoryDataWidth = SDRAM_MEMORY_WIDTH;
  FMC_SDRAMInitStructure.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  FMC_SDRAMInitStructure.CASLatency = SDRAM_CAS_LATENCY;
  FMC_SDRAMInitStructure.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  FMC_SDRAMInitStructure.SDClockPeriod = SDRAM_CLK_PERIOD;
  FMC_SDRAMInitStructure.ReadBurst = SDRAM_READBURST;
  FMC_SDRAMInitStructure.ReadPipeDelay = SDRAM_READPIPEDELAY;
//  FMC_SDRAMInitStructure.FMC_SDRAMTimingStruct = &FMC_SDRAMTimingInitStructure;

  /* FMC SDRAM bank initialization */
  FMC_SDRAM_Init(FMC_SDRAM_DEVICE, &FMC_SDRAMInitStructure);
  FMC_SDRAM_Timing_Init(FMC_SDRAM_DEVICE, &FMC_SDRAMTimingInitStructure, SDRAM_BANK);


  /* FMC SDRAM device initialization sequence */
  SDRAM_InitSequence();
  FMC_SDRAM_WriteProtection_Disable(FMC_SDRAM_DEVICE, SDRAM_BANK);
}
