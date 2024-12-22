/*
 * Copyright (C) EdgeTX
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

/**
* @brief QSPI MSP Initialization
* This function configures the hardware resources used in this example
* @param hqspi: QSPI handle pointer
* @retval None
*/

#include "stm32_hal.h"
#include "stm32_hal_ll.h"

#include "stm32_gpio.h"
#include "hal/gpio.h"

#include "stm32_qspi.h"
#include "delays_driver.h"
QSPI_HandleTypeDef hqspi ={0};
// extern RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
void qspiReadByte(uint32_t address, uint8_t& value);

void qspiInit()
{
  hqspi.Instance = QUADSPI;
//  hqspi.Init.ClockPrescaler = 7;
  hqspi.Init.ClockPrescaler = 3;
  hqspi.Init.FifoThreshold = 32;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = POSITION_VAL(8 * 1024 * 1024);
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

  if(hqspi.Instance==QUADSPI)
  {
    // PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    // PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    // if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    // {
    //   Error_Handler();
    // }

    /* Peripheral clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_QSPI_FORCE_RESET();
    delay_ms(10);
    __HAL_RCC_QSPI_RELEASE_RESET();

    gpio_init_af(GPIO_PIN(GPIOE, 2), GPIO_AF9, GPIO_PIN_SPEED_VERY_HIGH);
    gpio_init_af(GPIO_PIN(GPIOB, 2), GPIO_AF9, GPIO_PIN_SPEED_VERY_HIGH);

    gpio_init_af(GPIO_PIN(GPIOD, 11), GPIO_AF9, GPIO_PIN_SPEED_VERY_HIGH);
    gpio_init_af(GPIO_PIN(GPIOD, 12), GPIO_AF9, GPIO_PIN_SPEED_VERY_HIGH);
    gpio_init_af(GPIO_PIN(GPIOD, 13), GPIO_AF9, GPIO_PIN_SPEED_VERY_HIGH);

    gpio_init_af(GPIO_PIN(GPIOB, 06), GPIO_AF10, GPIO_PIN_SPEED_VERY_HIGH);

#if 0
    /* QUADSPI MDMA Init */
    /* QUADSPI_FIFO_TH Init */
    hmdma_quadspi_fifo_th.Instance = MDMA_Channel0;
    hmdma_quadspi_fifo_th.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
    hmdma_quadspi_fifo_th.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_quadspi_fifo_th.Init.Priority = MDMA_PRIORITY_LOW;
    hmdma_quadspi_fifo_th.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_quadspi_fifo_th.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_quadspi_fifo_th.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma_quadspi_fifo_th.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_quadspi_fifo_th.Init.BufferTransferLength = 1;
    hmdma_quadspi_fifo_th.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.SourceBlockAddressOffset = 0;
    hmdma_quadspi_fifo_th.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_quadspi_fifo_th) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_MDMA_ConfigPostRequestMask(&hmdma_quadspi_fifo_th, 0, 0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hqspi,hmdma,hmdma_quadspi_fifo_th);
#endif


    HAL_QSPI_Init(&hqspi);

    // this is to get the WSPI out of memory mapped mode when the FW starts
    qspiEnableMemoryMappedMode();
    HAL_QSPI_Abort(&hqspi);
    uint8_t buf;
    qspiReadByte(0, buf);
  }

}

void qspiDeInit()
{
  hqspi.Instance=QUADSPI;
//  hqspi.State = HAL_QSPI_STATE_BUSY_MEM_MAPPED;
//  HAL_QSPI_Abort(&hqspi);

  if(hqspi.Instance==QUADSPI)
  {

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB2     ------> QUADSPI_CLK
    PD11     ------> QUADSPI_BK1_IO0
    PD12     ------> QUADSPI_BK1_IO1
    PD13     ------> QUADSPI_BK1_IO3
    PB6     ------> QUADSPI_BK1_NCS
    */
    gpio_init(GPIO_PIN(GPIOE, 2), GPIO_IN, GPIO_PIN_SPEED_LOW);
    gpio_init(GPIO_PIN(GPIOB, 2), GPIO_IN, GPIO_PIN_SPEED_LOW);

    gpio_init(GPIO_PIN(GPIOD, 11), GPIO_IN, GPIO_PIN_SPEED_LOW);
    gpio_init(GPIO_PIN(GPIOD, 12), GPIO_IN, GPIO_PIN_SPEED_LOW);
    gpio_init(GPIO_PIN(GPIOD, 13), GPIO_IN, GPIO_PIN_SPEED_LOW);

    gpio_init(GPIO_PIN(GPIOB, 06), GPIO_IN, GPIO_PIN_SPEED_LOW);

    /* QUADSPI MDMA DeInit */
//    HAL_MDMA_DeInit(hqspi->hmdma);
  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }

}

void qspiReadStatusReg(uint8_t& value)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0x05;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&hqspi, &value, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

//  return MT25QL512ABB_OK;
}

void qspiReadStatusReg2(uint8_t& value)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0x35;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&hqspi, &value, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

//  return MT25QL512ABB_OK;
}

void qspiReadByte(uint32_t address, uint8_t& value)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0xEB;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.Address           = address;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
  s_command.AlternateBytes    = 0;
  s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  s_command.DummyCycles       = 4;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&hqspi, &value, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }
//  return MT25QL512ABB_OK;
}

void qspiFastRead(uint32_t address, size_t count, uint8_t* buf)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0xEB;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
  s_command.AlternateBytes    = 0;
  s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  s_command.DummyCycles       = 4;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.NbData            = 256;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  for(size_t i=0; i<count/256; ++i)
  {

    s_command.Address           = address;
    /* Configure the command */
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      asm("NOP");
    }

    /* Reception of the data */
    if (HAL_QSPI_Receive(&hqspi, buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      asm("NOP");
    }
    buf +=256;
    address += 256;
  }
//  return MT25QL512ABB_OK;
}


uint8_t sfpd[256];

void qspiReadSFPD()//uint32_t address, uint8_t& value)
{

  QSPI_CommandTypeDef s_command;

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0x5A;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = 0;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 8;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.NbData            = 256;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    asm("NOP");
  }

  HAL_QSPI_Receive(&hqspi, &sfpd[0], HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  asm("NOP");
}

int qspiAutoPollBusy()
{
  QSPI_CommandTypeDef      s_command;
  QSPI_AutoPollingTypeDef  s_config;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = 0x05;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DummyCycles       = 0;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.Mask            = 0x01;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return -1;
  }

  return 0;
}

void qspiEraseSector(intptr_t address)
{
  QSPI_CommandTypeDef s_command;
  uint8_t status = 0;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if((address&0xFFF) == 0)
  {
    // write enable
    s_command.Instruction       = 0x06;

    HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    qspiReadStatusReg(status);
    while((status & 0x02) == 0)
    {
      qspiReadStatusReg(status);
    }

    // sector erase
    s_command.Instruction       = 0x20;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = address;
    HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

//    qspiAutoPollBusy();
    qspiReadStatusReg(status);
    while((status & 0x01) != 0)
    {
      qspiReadStatusReg(status);
    }
  }
}

void qspiWritePage(intptr_t address, uint8_t* data)
{
  QSPI_CommandTypeDef s_command;
  uint8_t status = 0;

  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;


  // write enable
  s_command.Instruction       = 0x06;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  qspiReadStatusReg(status);
  while((status & 0x02) == 0)
  {
    qspiReadStatusReg(status);
  }

  // page program
  s_command.Instruction       = 0x32;
  s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.Address           = address;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.NbData            = 256;
  HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  HAL_QSPI_Transmit(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  //qspiAutoPollBusy();
  qspiReadStatusReg(status);
  while((status & 0x01) != 0)
  {
    qspiReadStatusReg(status);
  }
}


void qspiWriteBlock(intptr_t address, uint8_t* data)
{

  uint8_t buf[4096] ={0};

  qspiFastRead(address, sizeof(buf), buf);

  uint32_t* d1 = (uint32_t*)buf;
  uint32_t* d2 = (uint32_t*)data;
  bool equal = true;
  for(size_t i=0; i<sizeof(buf)/4; ++i)
  {
    if(d1[i]!=d2[i])
    {
      equal = false;
      break;
    }
  }

  if(equal)
    return;

  qspiEraseSector(address);

  for(int i = 0; i<4096/256; ++i)
  {
    qspiWritePage(address, data);

    data+=256;
    address+=256;
  }
}

void qspiEnableMemoryMappedMode()
{
  qspiReadSFPD();
  QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

  s_command.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction        = 0xEB;
  s_command.AddressMode        = QSPI_ADDRESS_4_LINES;
  s_command.DummyCycles        = 4;
  s_command.DataMode           = QSPI_DATA_4_LINES;

  s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
  s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  s_command.AlternateBytes     = 0b00100000;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;

  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;

  HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg);
}
