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

#define SDRAM_MEMORY_WIDTH    FMC_SDRAM_MEM_BUS_WIDTH_16

#define SDRAM_CAS_LATENCY     FMC_SDRAM_CAS_LATENCY_3

#define SDCLOCK_PERIOD    FMC_SDRAM_CLOCK_PERIOD_2

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF)

#define SDRAM_READBURST    FMC_SDRAM_RBURST_DISABLE

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
  */

  /* GPIOC configuration */

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

  gpio_init_af(GPIO_PIN(GPIOH, 5), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOH, 6), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(GPIO_PIN(GPIOH, 7), GPIO_AF_FMC, GPIO_PIN_SPEED_VERY_HIGH);

}

extern "C" void SDRAM_InitSequence(void)
{
  FMC_SDRAM_CommandTypeDef FMC_SDRAMCommandStructure;
  uint32_t tmpr = 0;

  /* Step 3 --------------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_Bank5_6_R, &FMC_SDRAMCommandStructure, 10);

  /* Step 4 --------------------------------------------------------------------*/
  /* Insert 1 ms delay */
  delay_ms(100);

  /* Step 5 --------------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_PALL;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_Bank5_6_R, &FMC_SDRAMCommandStructure, 10);

  /* Step 6 --------------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 8;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = 0;
  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_Bank5_6_R, &FMC_SDRAMCommandStructure, 10);

  /* Step 7 --------------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2 |
         SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
         SDRAM_MODEREG_CAS_LATENCY_3 |
         SDRAM_MODEREG_OPERATING_MODE_STANDARD |
         SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  FMC_SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  FMC_SDRAMCommandStructure.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  FMC_SDRAMCommandStructure.AutoRefreshNumber = 1;
  FMC_SDRAMCommandStructure.ModeRegisterDefinition = tmpr;
  /* Send the command */
  FMC_SDRAM_SendCommand(FMC_Bank5_6_R, &FMC_SDRAMCommandStructure, 10);

  /* Step 8 --------------------------------------------------------------------*/
  /* Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  FMC_SDRAM_SetAutoRefreshNumber(FMC_Bank5_6_R, 15);
  FMC_SDRAM_ProgramRefreshRate(FMC_Bank5_6_R, 2479);
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
  FMC_SDRAM_InitTypeDef  FMC_SDRAMInitStructure;
  FMC_SDRAM_TimingTypeDef  FMC_SDRAMTimingInitStructure;

  /* FMC SDRAM Bank configuration */
  FMC_SDRAMTimingInitStructure.LoadToActiveDelay    = 3;  // tMRD
  FMC_SDRAMTimingInitStructure.ExitSelfRefreshDelay = 11; // tXSR = 70ns
  FMC_SDRAMTimingInitStructure.SelfRefreshTime      = 7;  // tRAS = 42ns
  FMC_SDRAMTimingInitStructure.RowCycleDelay        = 10; // tRC = 60ns
  FMC_SDRAMTimingInitStructure.WriteRecoveryTime    = 2;  // tWR
  FMC_SDRAMTimingInitStructure.RPDelay              = 3;  // tRP = 18ns
  FMC_SDRAMTimingInitStructure.RCDDelay             = 3;  // tRCD = 18ns

  /* FMC SDRAM control configuration */
  FMC_SDRAMInitStructure.SDBank = FMC_SDRAM_BANK2;
  FMC_SDRAMInitStructure.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  FMC_SDRAMInitStructure.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  FMC_SDRAMInitStructure.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  FMC_SDRAMInitStructure.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  FMC_SDRAMInitStructure.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  FMC_SDRAMInitStructure.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  FMC_SDRAMInitStructure.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_3;
  FMC_SDRAMInitStructure.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  FMC_SDRAMInitStructure.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;

  /* FMC SDRAM bank initialization */
  FMC_SDRAM_Init(FMC_Bank5_6_R, &FMC_SDRAMInitStructure);
  FMC_SDRAM_Timing_Init(FMC_Bank5_6_R, &FMC_SDRAMTimingInitStructure, FMC_SDRAM_BANK2);

  __FMC_ENABLE();

  /* FMC SDRAM device initialization sequence */
  SDRAM_InitSequence();
}
