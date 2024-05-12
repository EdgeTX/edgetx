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

#include "stm32_hal.h"
#include "delays_driver.h"

typedef struct 
{
  uint32_t TargetBank;           /*!< Target Bank                             */
  uint32_t RefreshMode;          /*!< Refresh Mode                            */
  uint32_t RefreshRate;          /*!< Refresh Rate                            */
  uint32_t BurstLength;          /*!< Burst Length                            */
  uint32_t BurstType;            /*!< Burst Type                              */
  uint32_t CASLatency;           /*!< CAS Latency                             */
  uint32_t OperationMode;        /*!< Operation Mode                          */
  uint32_t WriteBurstMode;       /*!< Write Burst Mode                        */
} IS42S32800J_Context_t;

#define IS42S32800J_OK                (0)
#define IS42S32800J_ERROR             (-1)

/* Register Mode */
#define IS42S32800J_BURST_LENGTH_1              0x00000000U
#define IS42S32800J_BURST_LENGTH_2              0x00000001U
#define IS42S32800J_BURST_LENGTH_4              0x00000002U
#define IS42S32800J_BURST_LENGTH_8              0x00000004U
#define IS42S32800J_BURST_TYPE_SEQUENTIAL       0x00000000U
#define IS42S32800J_BURST_TYPE_INTERLEAVED      0x00000008U
#define IS42S32800J_CAS_LATENCY_2               0x00000020U
#define IS42S32800J_CAS_LATENCY_3               0x00000030U
#define IS42S32800J_OPERATING_MODE_STANDARD     0x00000000U
#define IS42S32800J_WRITEBURST_MODE_PROGRAMMED  0x00000000U 
#define IS42S32800J_WRITEBURST_MODE_SINGLE      0x00000200U 

/* Command Mode */
#define IS42S32800J_NORMAL_MODE_CMD             0x00000000U
#define IS42S32800J_CLK_ENABLE_CMD              0x00000001U
#define IS42S32800J_PALL_CMD                    0x00000002U
#define IS42S32800J_AUTOREFRESH_MODE_CMD        0x00000003U
#define IS42S32800J_LOAD_MODE_CMD               0x00000004U
#define IS42S32800J_SELFREFRESH_MODE_CMD        0x00000005U
#define IS42S32800J_POWERDOWN_MODE_CMD          0x00000006U

/* SDRAM refresh counter (100Mhz SD clock) */
#define REFRESH_COUNT                    ((uint32_t)0x0603)
#define IS42S32800J_TIMEOUT              ((uint32_t)0xFFFF)


static void sdram_gpio_init()
{
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();

  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8| GPIO_PIN_9 | GPIO_PIN_10 |\
                              GPIO_PIN_14 | GPIO_PIN_15;

  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7| GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |\
                              GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /* GPIOH configuration */
  gpio_init_structure.Pin   = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |\
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |\
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  /* GPIOI configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
                              GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);
}

HAL_StatusTypeDef sdram_bank2_init(SDRAM_HandleTypeDef *hSdram)
{
  FMC_SDRAM_TimingTypeDef sdram_timing;

  /* SDRAM device configuration */
  hSdram->Instance = FMC_SDRAM_DEVICE;

  /* SDRAM handle configuration */
  hSdram->Init.SDBank             = FMC_SDRAM_BANK2;
  hSdram->Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hSdram->Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hSdram->Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hSdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hSdram->Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  hSdram->Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hSdram->Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
  hSdram->Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  hSdram->Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

  /* Timing configuration for as SDRAM */
  sdram_timing.LoadToActiveDelay    = 2;
  sdram_timing.ExitSelfRefreshDelay = 7;
  sdram_timing.SelfRefreshTime      = 4;
  sdram_timing.RowCycleDelay        = 7;
  sdram_timing.WriteRecoveryTime    = 2;
  sdram_timing.RPDelay              = 2;
  sdram_timing.RCDDelay             = 2;

  /* SDRAM controller initialization */
  if(HAL_SDRAM_Init(hSdram, &sdram_timing) != HAL_OK)
  {
    return  HAL_ERROR;
  }

  return HAL_OK;
}

/**
  * @brief  Enable SDRAM clock
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
static int32_t IS42S32800J_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  FMC_SDRAM_CommandTypeDef Command;

  Command.CommandMode            = IS42S32800J_CLK_ENABLE_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Precharge all sdram banks
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2
  * @retval error status
  */
static int32_t IS42S32800J_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) 
{
  FMC_SDRAM_CommandTypeDef Command;

  Command.CommandMode            = IS42S32800J_PALL_CMD;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Configure Refresh mode
  * @param  Ctx Component object pointer
  * @param  Interface Could be FMC_SDRAM_CMD_TARGET_BANK1 or FMC_SDRAM_CMD_TARGET_BANK2 
  * @param  RefreshMode Could be IS42S32800J_CMD_AUTOREFRESH_MODE or
  *                      IS42S32800J_CMD_SELFREFRESH_MODE
  * @retval error status
  */
static int32_t IS42S32800J_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode) 
{
  FMC_SDRAM_CommandTypeDef Command;

  Command.CommandMode            = RefreshMode;
  Command.CommandTarget          = Interface;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Program the external memory mode register
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
static int32_t IS42S32800J_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode) 
{
  uint32_t tmpmrd;
  FMC_SDRAM_CommandTypeDef Command;

  /* Program the external memory mode register */
  tmpmrd = (uint32_t)pRegMode->BurstLength   |\
                     pRegMode->BurstType     |\
                     pRegMode->CASLatency    |\
                     pRegMode->OperationMode |\
                     pRegMode->WriteBurstMode;
  
  Command.CommandMode            = IS42S32800J_LOAD_MODE_CMD;
  Command.CommandTarget          = pRegMode->TargetBank;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;
  
  /* Send the command */
  if(HAL_SDRAM_SendCommand(Ctx, &Command, IS42S32800J_TIMEOUT) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Set the device refresh rate
  * @param  Ctx Component object pointer
  * @param  RefreshCount The refresh rate to be programmed
  * @retval error status
  */
static int32_t IS42S32800J_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount)
{
  /* Set the device refresh rate */
  if(HAL_SDRAM_ProgramRefreshRate(Ctx, RefreshCount) != HAL_OK)
  {
    return IS42S32800J_ERROR;
  }
  else
  {
    return IS42S32800J_OK;
  }
}

/**
  * @brief  Initializes the IS42S32800J SDRAm memory
  * @param  Ctx Component object pointer
  * @param  pRegMode : Pointer to Register Mode structure
  * @retval error status
  */
static int32_t IS42S32800J_Init(SDRAM_HandleTypeDef *Ctx, IS42S32800J_Context_t *pRegMode) 
{
  int32_t ret = IS42S32800J_ERROR;
  
  /* Step 1: Configure a clock configuration enable command */
  if(IS42S32800J_ClockEnable(Ctx, pRegMode->TargetBank) == IS42S32800J_OK)
  {
    /* Step 2: Insert 100 us minimum delay */ 
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    delay_ms(1);
    
    /* Step 3: Configure a PALL (precharge all) command */ 
    if(IS42S32800J_Precharge(Ctx, pRegMode->TargetBank) == IS42S32800J_OK)
    {
      /* Step 4: Configure a Refresh command */ 
      if(IS42S32800J_RefreshMode(Ctx, pRegMode->TargetBank, pRegMode->RefreshMode) == IS42S32800J_OK)
      {
        /* Step 5: Program the external memory mode register */
        if(IS42S32800J_ModeRegConfig(Ctx, pRegMode) == IS42S32800J_OK)
        {
          /* Step 6: Set the refresh rate counter */
          if(IS42S32800J_RefreshRate(Ctx, pRegMode->RefreshRate) == IS42S32800J_OK)
          {
            ret = IS42S32800J_OK;
          }
        }
      }
    }
  } 
  return ret;
}

extern "C" void SDRAM_Init()
{
  SDRAM_HandleTypeDef h_sdram;
  IS42S32800J_Context_t pRegMode;

  sdram_gpio_init();
  if (sdram_bank2_init(&h_sdram) != HAL_OK) {
    return; // error
  }

  /* External memory mode register configuration */
  pRegMode.TargetBank      = FMC_SDRAM_CMD_TARGET_BANK2;
  pRegMode.RefreshMode     = IS42S32800J_AUTOREFRESH_MODE_CMD;
  pRegMode.RefreshRate     = REFRESH_COUNT;
  pRegMode.BurstLength     = IS42S32800J_BURST_LENGTH_1;
  pRegMode.BurstType       = IS42S32800J_BURST_TYPE_SEQUENTIAL;
  pRegMode.CASLatency      = IS42S32800J_CAS_LATENCY_3;
  pRegMode.OperationMode   = IS42S32800J_OPERATING_MODE_STANDARD;
  pRegMode.WriteBurstMode  = IS42S32800J_WRITEBURST_MODE_SINGLE;

  IS42S32800J_Init(&h_sdram, &pRegMode);
}
