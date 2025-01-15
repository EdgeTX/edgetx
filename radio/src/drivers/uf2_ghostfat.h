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

#define UF2_NUM_BLOCKS 32768 // at least 16MB
#define UF2_INVALID_NUM_BLOCKS 0xFFFFFFFF

#if !defined(UF2_MAX_FW_SIZE)
#define UF2_MAX_FW_SIZE (2 * 1024 * 1024)
#endif

#define UF2_MAX_BLOCKS (UF2_MAX_FW_SIZE / 256)

#define UF2_ERASE_BLOCK_SIZE (4 * 1024)
#define UF2_ERASE_BLOCKS (UF2_MAX_FW_SIZE / UF2_ERASE_BLOCK_SIZE)

typedef struct {
    uint32_t num_blocks;
    uint32_t num_written;
    uint32_t written_mask[UF2_MAX_BLOCKS / 32];
    uint32_t erased_mask[UF2_ERASE_BLOCKS / 32];
} uf2_fat_write_state_t;

void uf2_fat_reset_state();
const uf2_fat_write_state_t* uf2_fat_get_state();

void uf2_fat_read_block(uint32_t block_no, uint8_t *data);
int uf2_fat_write_block(uint32_t block_no, uint8_t *data);
