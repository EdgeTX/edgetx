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
#include "hal.h"

#include <string.h>
#include <cstddef>

#if !defined(QSPI_MAX_FREQ)
#define QSPI_MAX_FREQ 100000000U // 100 MHz
#endif

#define QSPI_PRESCALER_MAX 255

#define SPI_NOR_WIP_BIT (1 << 0)
#define SPI_NOR_WEL_BIT (1 << 1)

#define SPI_NOR_CMD_WRSR     0x01
#define SPI_NOR_CMD_RDSR     0x05
#define SPI_NOR_CMD_WRSR2    0x31
#define SPI_NOR_CMD_RDSR2    0x35

#define SPI_NOR_CMD_WREN     0x06
#define SPI_NOR_CMD_PP_1_1_4 0x32
#define SPI_NOR_CMD_BE       0xD8

#define SPI_NOR_CMD_4READ    0xEB

#define QSPI_STATUS_REG 1

// TODO: we might need to read this via SFDP
//       -> 15th DWORD "Quad Enable Requirements"
//
// GD25Q127C: does not expose these via SFDP
#if !defined(QSPI_QE_REG)
#define QSPI_QE_REG 2
#endif

#if !defined(QSPI_QE_BIT)
#define QSPI_QE_BIT (1 << 1)
#endif

// TODO: use SFDP to detect this
//       -> 3rd DWORD "Instruction / Mode Clocks / Dummy cycles"
//
// GD25Q127C: 2 mode clocks + 4 dummy cycles
#if !defined(QSPI_READ_DUMMY_CYCLES)
#define QSPI_READ_DUMMY_CYCLES 6
#endif

// reduce read time by not repeating read command
// #define QSPI_CONTINUOUS_READ (1 << 5)
// #define QSPI_READ_DUMMY_CYCLES 4

QSPI_HandleTypeDef hqspi = { 0 };

static const QSPI_CommandTypeDef cmd_write_en = {
	.Instruction = SPI_NOR_CMD_WREN,
	.InstructionMode = QSPI_INSTRUCTION_1_LINE
};

static int qspi_read(QSPI_CommandTypeDef* cmd, uint8_t* data, size_t size)
{
  cmd->NbData = size;
  int ret = HAL_QSPI_Command(&hqspi, cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  if (ret == HAL_OK) {
    ret = HAL_QSPI_Receive(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  }

  return ret == HAL_OK ? 0 : -1;
}

static int qspi_write(QSPI_CommandTypeDef* cmd, uint8_t* data, size_t size)
{
  cmd->NbData = size;
  int ret = HAL_QSPI_Command(&hqspi, cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  if (ret == HAL_OK) {
    ret = HAL_QSPI_Transmit(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  }

  return ret == HAL_OK ? 0 : -1;
}

static int qspi_read_status_register(uint8_t reg_num, uint8_t* value)
{
  QSPI_CommandTypeDef cmd = {
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .DataMode = QSPI_DATA_1_LINE,
  };

  switch(reg_num) {
    case 1:
      cmd.Instruction = SPI_NOR_CMD_RDSR;
      break;
    case 2:
      cmd.Instruction = SPI_NOR_CMD_RDSR2;
      break;
    default:
      return -1;
  }

  return qspi_read(&cmd, value, sizeof(value));
}

static int qspi_write_status_register(uint8_t reg_num, uint8_t value)
{
  QSPI_CommandTypeDef cmd = {
      .Instruction = SPI_NOR_CMD_WRSR,
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .DataMode = QSPI_DATA_1_LINE,
  };

  switch(reg_num) {
    case 1:
      cmd.Instruction = SPI_NOR_CMD_WRSR;
      break;
    case 2:
      // TODO: some parts can only write upper
      //       registers together with lower ones.
      cmd.Instruction = SPI_NOR_CMD_WRSR2;
      break;
    default:
      return -1;
  }

  return qspi_write(&cmd, &value, sizeof(value));
}

static int qspi_write_enable()
{
  int ret = HAL_QSPI_Command(&hqspi, (QSPI_CommandTypeDef*)&cmd_write_en,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  if (ret == HAL_OK) {
    uint8_t reg;
    do {
		  ret = qspi_read_status_register(1U, &reg);
	  } while (!ret && !(reg & SPI_NOR_WEL_BIT));
  }

  return ret == HAL_OK ? 0 : -1;
}

static int qspi_wait_until_ready()
{
  uint8_t reg;
  int ret;

  QSPI_CommandTypeDef cmd = {
      .Instruction = SPI_NOR_CMD_RDSR,
      .InstructionMode = QSPI_INSTRUCTION_1_LINE,
      .DataMode = QSPI_DATA_1_LINE,
  };

  do {
    ret = qspi_read(&cmd, &reg, sizeof(reg));
  } while (!ret && (reg & SPI_NOR_WIP_BIT));

  return ret;
}

static int qspi_enable_quad_io()
{
  uint8_t reg;

  if (qspi_read_status_register(QSPI_QE_REG, &reg) != 0) {
    return -1;
  }

  // check if already enabled
  if ((reg & QSPI_QE_BIT) != 0) {
    return 0;
  }

  // enable quad-io
  reg |= QSPI_QE_BIT;

  if (qspi_write_enable() != 0) return -1;

  if (qspi_write_status_register(QSPI_QE_REG, reg) != 0) {
    return -1;
  }

  if (qspi_wait_until_ready() != 0) return -1;

  // check if now enabled
  if (qspi_read_status_register(QSPI_QE_REG, &reg) != 0) {
    return -1;
  }

  return (reg & QSPI_QE_BIT) != 0 ? 0 : -1;
}

static void qspi_gpio_init()
{
  gpio_init_af(QSPI_CLK_GPIO,  QSPI_CLK_GPIO_AF,  GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(QSPI_CS_GPIO,   QSPI_CS_GPIO_AF,   GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(QSPI_MISO_GPIO, QSPI_MISO_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(QSPI_MOSI_GPIO, QSPI_MOSI_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(QSPI_WP_GPIO ,  QSPI_WP_GPIO_AF,   GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(QSPI_HOLD_GPIO, QSPI_HOLD_GPIO_AF, GPIO_PIN_SPEED_VERY_HIGH);
}

static void qspi_gpio_deinit()
{
  gpio_init(QSPI_CLK_GPIO,  GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(QSPI_CS_GPIO,   GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(QSPI_MISO_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(QSPI_MOSI_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(QSPI_WP_GPIO,   GPIO_IN, GPIO_PIN_SPEED_LOW);
  gpio_init(QSPI_HOLD_GPIO, GPIO_IN, GPIO_PIN_SPEED_LOW);
}

static uint32_t qspi_compute_prescaler(uint32_t max_freq)
{
  uint32_t qspi_freq = LL_RCC_GetQSPIClockFreq(LL_RCC_QSPI_CLKSOURCE);
  uint32_t prescaler = 0;
  for (; prescaler <= QSPI_PRESCALER_MAX; prescaler++) {
    uint32_t clk = qspi_freq / (prescaler + 1);
    if (clk <= max_freq) break;
  }
  
  return prescaler;
}

static void qspi_init_hal_struct()
{
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = qspi_compute_prescaler(QSPI_MAX_FREQ);
  hqspi.Init.FifoThreshold = 8;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = POSITION_VAL(QSPI_FLASH_SIZE);
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
}

int stm32_qspi_nor_init()
{
  qspi_gpio_init();
  
  // Peripheral clock enable
  __HAL_RCC_QSPI_CLK_ENABLE();

  qspi_init_hal_struct();
  if (HAL_QSPI_Init(&hqspi) != HAL_OK) return -1;

  return qspi_enable_quad_io();
}

static bool qspi_is_memory_mapped(QSPI_HandleTypeDef* h)
{
  return ((READ_BIT(h->Instance->CCR, QUADSPI_CCR_FMODE) == QUADSPI_CCR_FMODE)
              ? true
              : false);
}

static int qspi_abort(QSPI_HandleTypeDef* h)
{
  if (HAL_QSPI_Abort(h) != HAL_OK) {
    return -1;
  }

  return 0;
}

void stm32_qspi_no_init_runtime()
{
  qspi_init_hal_struct();
  hqspi.Timeout = HAL_QSPI_TIMEOUT_DEFAULT_VALUE;
  hqspi.State = HAL_QSPI_STATE_BUSY_MEM_MAPPED;
}

void stm32_qspi_nor_deinit()
{
  if (qspi_is_memory_mapped(&hqspi)) {
    qspi_abort(&hqspi);
  }
  
  HAL_QSPI_DeInit(&hqspi);
  qspi_gpio_deinit();
}

int stm32_qspi_nor_memory_mapped()
{
  // assume quad mode / 3-byte address
  QSPI_CommandTypeDef cmd = {
    .Instruction = SPI_NOR_CMD_4READ,
    .AddressSize = QSPI_ADDRESS_24_BITS,
    .DummyCycles = QSPI_READ_DUMMY_CYCLES,
    .InstructionMode = QSPI_INSTRUCTION_1_LINE,
    .AddressMode = QSPI_ADDRESS_4_LINES,
    .DataMode = QSPI_DATA_4_LINES,
  };

  // not detectable via SFDP (apart from matching on part ID)
#if defined(QSPI_CONTINUOUS_READ)
  cmd.AlternateBytes = QSPI_CONTINUOUS_READ;
  cmd.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
  cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
  cmd.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
#endif

  QSPI_MemoryMappedTypeDef mem_mapped = {
      .TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE,
  };

  if (HAL_QSPI_MemoryMapped(&hqspi, &cmd, &mem_mapped) != HAL_OK) {
    return -1;
  }

  return 0;
}

int stm32_qspi_nor_read(uint32_t address, void* data, uint32_t size)
{
  if (!qspi_is_memory_mapped(&hqspi)) {
    if (stm32_qspi_nor_memory_mapped() != 0) {
      return -1;
    }
  }

  uintptr_t mmap_addr = QSPI_BASE + address;
  memcpy(data, (void *)mmap_addr, size);

  return 0;
}

int stm32_qspi_nor_erase_sector(uint32_t address)
{
  // verify block alignment
  if (address & 0xFFFF) return -1;

  // backup & diable memory-mapped mode
  uint32_t qspi_ccr_backup = 0;
  if (qspi_is_memory_mapped(&hqspi)) {
    qspi_ccr_backup = READ_REG(hqspi.Instance->CCR);
    if (qspi_abort(&hqspi) != 0) return -1;
  }

  // write enable
  int ret = HAL_QSPI_Command(&hqspi, (QSPI_CommandTypeDef*)&cmd_write_en,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  if (ret == HAL_OK) {
    // block erase
    QSPI_CommandTypeDef s_command = {
        .Instruction = SPI_NOR_CMD_BE,
        .Address = (uint32_t)address,
        .AddressSize = QSPI_ADDRESS_24_BITS,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .AddressMode = QSPI_ADDRESS_1_LINE,
    };
    ret = HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if (ret == HAL_OK) {
    ret = qspi_wait_until_ready();
  }

  if (qspi_ccr_backup) {
    WRITE_REG(hqspi.Instance->CCR, qspi_ccr_backup);
  }

  return ret;
}

int stm32_qspi_nor_program(uint32_t address, void* data, uint32_t len)
{
  uint32_t qspi_ccr_backup = 0;
  if (qspi_is_memory_mapped(&hqspi)) {
    qspi_ccr_backup = READ_REG(hqspi.Instance->CCR);
    if (qspi_abort(&hqspi) != 0) return -1;
  }

  // write enable
  int ret = HAL_QSPI_Command(&hqspi, (QSPI_CommandTypeDef*)&cmd_write_en,
                             HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

  // page program
  if (ret == HAL_OK) {
    QSPI_CommandTypeDef s_command = {
        .Instruction = SPI_NOR_CMD_PP_1_1_4,
        .Address = address,
        .AddressSize = QSPI_ADDRESS_24_BITS,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .AddressMode = QSPI_ADDRESS_1_LINE,
        .DataMode = QSPI_DATA_4_LINES,
        .NbData = len,
    };
    ret = HAL_QSPI_Command(&hqspi, &s_command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  }

  // xmit data
  if (ret == HAL_OK) {
    ret = HAL_QSPI_Transmit(&hqspi, (uint8_t*)data,
                            HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
  }

  if (ret == HAL_OK) {
    ret = qspi_wait_until_ready();
  }

  if (qspi_ccr_backup) {
    WRITE_REG(hqspi.Instance->CCR, qspi_ccr_backup);
  }

  return ret;
}
