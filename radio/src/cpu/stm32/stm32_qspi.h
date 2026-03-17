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

#pragma once

#include <stdint.h>

int stm32_qspi_nor_init();
void stm32_qspi_nor_deinit();

void stm32_qspi_no_init_runtime();

int stm32_qspi_nor_memory_mapped();
int stm32_qspi_nor_read(uint32_t address, void* data, uint32_t len);

int stm32_qspi_nor_erase_sector(uint32_t address);
int stm32_qspi_nor_program(uint32_t address, void* data, uint32_t len);

