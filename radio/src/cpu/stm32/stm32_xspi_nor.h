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

#pragma once

#include <stdint.h>
#include "stm32_hal_ll.h"
#include "stm32_hal.h"

typedef enum {
  XSPI_SPI_MODE = 0,
  XSPI_OCTO_MODE,
} stm32_xspi_mode_t;

typedef enum {
  XSPI_STR_TRANSFER = 0,
  XSPI_DTR_TRANSFER,
} stm32_xspi_rate_t;

typedef struct {
  uint32_t          flash_size;
  uint32_t          page_size;
  stm32_xspi_mode_t data_mode;
  stm32_xspi_rate_t data_rate;

  XSPI_HandleTypeDef* hxspi; 
} stm32_xspi_nor_t;

int stm32_xspi_nor_init(const stm32_xspi_nor_t* dev);
int stm32_xspi_nor_memory_mapped(const stm32_xspi_nor_t* dev);
int stm32_xspi_nor_read(const stm32_xspi_nor_t* dev, uint32_t address,
                        void* data, uint32_t len);
int stm32_xspi_nor_erase_sector(const stm32_xspi_nor_t* dev, uint32_t address);
int stm32_xspi_nor_program(const stm32_xspi_nor_t* dev, uint32_t address,
                           void* data, uint32_t len);
