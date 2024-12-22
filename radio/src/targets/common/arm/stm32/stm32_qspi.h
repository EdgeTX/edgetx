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

void qspiInit();
void qspiDeInit();
void qspiEnableMemoryMappedMode();
void qspiEraseSector(intptr_t address);
void qspiWritePage(intptr_t address, uint8_t* data);
void qspiWriteBlock(intptr_t address, uint8_t* data);

