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

#include "stm32_xspi_nor.h"

#define AUTO_POLLING_INTERVAL 0x10

#define SPI_NOR_CMD_RDSR 0x05
#define SPI_NOR_CMD_WREN 0x06
#define SPI_NOR_CMD_READ_FAST_4B 0x0C
#define SPI_NOR_CMD_PP_4B 0x12
#define SPI_NOR_CMD_WR_CFGREG2 0x72

#define SPI_NOR_DUMMY_REG_OCTAL 4U

#define SPI_NOR_OCMD_RDSR 0x05FA
#define SPI_NOR_OCMD_WREN 0x06F9
#define SPI_NOR_OCMD_READ 0xEC13
#define SPI_NOR_OCMD_DTR_READ 0xEE11
#define SPI_NOR_OCMD_PAGE_PRG 0x12ED
#define SPI_NOR_OCMD_ERASE_64K 0xDC23;

#define SPI_NOR_DUMMY_REG_OCTAL_DTR 5U
#define SPI_NOR_DUMMY_READ 20U

#define SPI_NOR_REG2_ADDR1 0x0000000
#define SPI_NOR_CR2_STR_OPI_EN 0x01
#define SPI_NOR_CR2_DTR_OPI_EN 0x02

#define SPI_NOR_REG2_ADDR3 0x00000300

#define MEMORY_READY_MATCH_VALUE 0x00
#define MEMORY_READY_MASK_VALUE 0x01

#define WRITE_ENABLE_MATCH_VALUE 0x02
#define WRITE_ENABLE_MASK_VALUE 0x02

static uint32_t xspi_nor_read_instr(const uint8_t mode, const uint8_t rate)
{
  if (rate == XSPI_STR_TRANSFER) {
    return (mode == XSPI_SPI_MODE) ? SPI_NOR_CMD_PP_4B : SPI_NOR_OCMD_READ;
  } else {
    return SPI_NOR_OCMD_DTR_READ;
  }
}

static uint32_t xspi_nor_page_prg_instr(const uint8_t mode)
{
  if (mode == XSPI_SPI_MODE) {
    return SPI_NOR_CMD_PP_4B;
  } else {
    return SPI_NOR_OCMD_PAGE_PRG;
  }
}

static XSPI_RegularCmdTypeDef xspi_prepare_cmd(const uint8_t transfer_mode,
                                               const uint8_t transfer_rate)
{
  XSPI_RegularCmdTypeDef cmd_tmp = {
      .OperationType = HAL_XSPI_OPTYPE_COMMON_CFG,
      .InstructionDTRMode = ((transfer_rate == XSPI_DTR_TRANSFER)
                                 ? HAL_XSPI_INSTRUCTION_DTR_ENABLE
                                 : HAL_XSPI_INSTRUCTION_DTR_DISABLE),
      .AddressWidth = HAL_XSPI_ADDRESS_32_BITS,
      .AddressDTRMode =
          ((transfer_rate == XSPI_DTR_TRANSFER) ? HAL_XSPI_ADDRESS_DTR_ENABLE
                                                : HAL_XSPI_ADDRESS_DTR_DISABLE),
      .AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE,
      .DataDTRMode =
          ((transfer_rate == XSPI_DTR_TRANSFER) ? HAL_XSPI_DATA_DTR_ENABLE
                                                : HAL_XSPI_DATA_DTR_DISABLE),
      .DQSMode = (transfer_rate == XSPI_DTR_TRANSFER) ? HAL_XSPI_DQS_ENABLE
                                                      : HAL_XSPI_DQS_DISABLE,
  };

  switch (transfer_mode) {
  case XSPI_OCTO_MODE:
    cmd_tmp.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
    cmd_tmp.InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS;
    cmd_tmp.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
    cmd_tmp.DataMode = HAL_XSPI_DATA_8_LINES;
    break;

  case XSPI_SPI_MODE:
  default:
    cmd_tmp.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
    cmd_tmp.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
    cmd_tmp.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
    cmd_tmp.DataMode = HAL_XSPI_DATA_1_LINE;
    break;
  }

  return cmd_tmp;
}

static int xspi_auto_polling(const stm32_xspi_nor_t* dev, uint32_t value,
                             uint32_t mask)
{
  XSPI_AutoPollingTypeDef s_config = {
    .MatchValue = value,
    .MatchMask = mask,
    .MatchMode = HAL_XSPI_MATCH_MODE_AND,
    .AutomaticStop = HAL_XSPI_AUTOMATIC_STOP_ENABLE,
    .IntervalTime = AUTO_POLLING_INTERVAL,
  };

  if (HAL_XSPI_AutoPolling(dev->hxspi, &s_config,
                           HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return 0;
}

static int xspi_nor_auto_polling(const stm32_xspi_nor_t* dev, const uint8_t mode,
                                 const uint8_t rate, uint32_t value,
                                 uint32_t mask)
{
  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);

  if (mode == XSPI_OCTO_MODE) {
    s_command.Instruction = SPI_NOR_OCMD_RDSR;
    s_command.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
    s_command.DataMode = HAL_XSPI_DATA_8_LINES;
    s_command.DummyCycles = (rate == XSPI_DTR_TRANSFER)
                                ? SPI_NOR_DUMMY_REG_OCTAL_DTR
                                : SPI_NOR_DUMMY_REG_OCTAL;
  } else {
    s_command.Instruction = SPI_NOR_CMD_RDSR;
    s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
    s_command.DataMode = HAL_XSPI_DATA_1_LINE;
    s_command.DummyCycles = 0;
  }
  s_command.DataLength = ((rate == XSPI_DTR_TRANSFER) ? 2U : 1U);
  s_command.Address = 0U;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return xspi_auto_polling(dev, value, mask);
}

static int xspi_nor_auto_polling_mem_ready(const stm32_xspi_nor_t* dev,
                                           const uint8_t mode,
                                           const uint8_t rate)
{
  return xspi_nor_auto_polling(dev, mode, rate, MEMORY_READY_MATCH_VALUE,
                               MEMORY_READY_MASK_VALUE);
}

static int xspi_nor_write_enable(const stm32_xspi_nor_t* dev, const uint8_t mode,
                                 const uint8_t rate)
{
  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  // Initialize the write enable command
  if (mode == XSPI_OCTO_MODE) {
    s_command.Instruction = SPI_NOR_OCMD_WREN;
  } else {
    s_command.Instruction = SPI_NOR_CMD_WREN;
    s_command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
  }
  s_command.AddressMode = HAL_XSPI_ADDRESS_NONE;
  s_command.DataMode = HAL_XSPI_DATA_NONE;
  s_command.DummyCycles = 0U;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return xspi_nor_auto_polling(dev, mode, rate, WRITE_ENABLE_MATCH_VALUE,
                               WRITE_ENABLE_MASK_VALUE);
}

static int xspi_nor_write_enable(const stm32_xspi_nor_t* dev)
{
  uint8_t mode = dev->data_mode;
  uint8_t rate = dev->data_rate;
  return xspi_nor_write_enable(dev, mode, rate);
}

static int xspi_nor_config_mem(const stm32_xspi_nor_t* dev)
{
  if (dev->data_mode != XSPI_OCTO_MODE) {
    return 0;
  }

  uint8_t mode = XSPI_SPI_MODE;
  uint8_t rate = XSPI_STR_TRANSFER;

  int err = xspi_nor_write_enable(dev, mode, rate);
  if (err != 0) return -1;

  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  s_command.Instruction = SPI_NOR_CMD_WR_CFGREG2;
  s_command.DummyCycles = 0;

  s_command.Address = SPI_NOR_REG2_ADDR1;
  s_command.DataLength = 1U;
  
  uint8_t reg = dev->data_rate == XSPI_STR_TRANSFER ? SPI_NOR_CR2_STR_OPI_EN
                                                    : SPI_NOR_CR2_DTR_OPI_EN;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  if (HAL_XSPI_Transmit(dev->hxspi, &reg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) !=
      HAL_OK) {
    return -1;
  }

  return xspi_nor_auto_polling(dev, mode, rate, WRITE_ENABLE_MATCH_VALUE,
                               WRITE_ENABLE_MASK_VALUE);
}

int stm32_xspi_nor_init(const stm32_xspi_nor_t* dev)
{
  if (HAL_XSPI_Init(dev->hxspi) != HAL_OK) {
    return -1;
  }

#if defined(HAL_XSPIM_IOPORT_1) || defined(HAL_XSPIM_IOPORT_2)
  // XSPI I/O manager init Function
  XSPIM_CfgTypeDef xspi_mgr_cfg;

  if (dev->hxspi->Instance == XSPI1) {
    xspi_mgr_cfg.IOPort = HAL_XSPIM_IOPORT_1;
  } else if (dev->hxspi->Instance == XSPI2) {
    xspi_mgr_cfg.IOPort = HAL_XSPIM_IOPORT_2;
  }
  // HAL_XSPI_CSSEL_OVR_NCS1 ??
  xspi_mgr_cfg.nCSOverride = HAL_XSPI_CSSEL_OVR_DISABLED;
  xspi_mgr_cfg.Req2AckTime = 1;

  if (HAL_XSPIM_Config(dev->hxspi, &xspi_mgr_cfg,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }
#endif /* XSPIM */

  if (xspi_nor_config_mem(dev) != 0) {
    return -1;
  }

  return 0;
}

int stm32_xspi_nor_memory_mapped(const stm32_xspi_nor_t* dev)
{
  XSPI_MemoryMappedTypeDef s_mem_mapped_cfg = {0};

  if (xspi_nor_write_enable(dev) != 0) {
    return -1;
  }

  uint8_t mode = dev->data_mode;
  uint8_t rate = dev->data_rate;

  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  s_command.OperationType = HAL_XSPI_OPTYPE_WRITE_CFG;
  s_command.Instruction = xspi_nor_page_prg_instr(mode);
  s_command.Address = 0x0;
  s_command.DummyCycles = 0;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  s_command.OperationType = HAL_XSPI_OPTYPE_READ_CFG;
  s_command.Instruction = xspi_nor_read_instr(mode, rate);
  s_command.DummyCycles = SPI_NOR_DUMMY_READ;
  
  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  s_mem_mapped_cfg.TimeOutActivation = HAL_XSPI_TIMEOUT_COUNTER_DISABLE;
  if (HAL_XSPI_MemoryMapped(dev->hxspi, &s_mem_mapped_cfg) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_xspi_nor_read(const stm32_xspi_nor_t* dev, uint32_t address, void* data,
                        uint32_t len)
{
  uint8_t mode = dev->data_mode;
  uint8_t rate = dev->data_rate;

  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  s_command.Instruction = xspi_nor_read_instr(mode, rate);
  s_command.Address = address;
  s_command.DataLength = len;
  s_command.DummyCycles = SPI_NOR_DUMMY_READ;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  if (HAL_XSPI_Receive(dev->hxspi, (uint8_t *)data,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_xspi_nor_erase_sector(const stm32_xspi_nor_t* dev, uint32_t address)
{
  if (xspi_nor_write_enable(dev) != 0) {
    return -1;
  }

  uint8_t mode = dev->data_mode;
  uint8_t rate = dev->data_rate;

  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  s_command.Instruction = SPI_NOR_OCMD_ERASE_64K;
  s_command.Address = address;
  s_command.DataMode = HAL_XSPI_DATA_NONE;
  s_command.DummyCycles = 0;
  
  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return xspi_nor_auto_polling_mem_ready(dev, mode, rate);
}

int stm32_xspi_nor_program(const stm32_xspi_nor_t* dev, uint32_t address,
                           void* data, uint32_t len)
{
  if (xspi_nor_write_enable(dev) != 0) {
    return -1;
  }
  
  uint8_t mode = dev->data_mode;
  uint8_t rate = dev->data_rate;

  XSPI_RegularCmdTypeDef s_command = xspi_prepare_cmd(mode, rate);
  s_command.Instruction = xspi_nor_page_prg_instr(mode);
  s_command.Address = address;
  s_command.DataLength = len;
  s_command.DummyCycles = 0;

  if (HAL_XSPI_Command(dev->hxspi, &s_command,
                       HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  if (HAL_XSPI_Transmit(dev->hxspi, (uint8_t*)data,
                        HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    return -1;
  }

  return xspi_nor_auto_polling_mem_ready(dev, mode, rate);
}
