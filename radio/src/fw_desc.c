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

#include "fw_desc.h"

// Symbols from linker script
extern uint32_t _estack;
extern uint32_t _firmware_length;
extern uint32_t _firmware_version;

extern void Reset_Handler();

__attribute__((section(".fwdescription"), used))
firmware_description_t _fw_desc = {
  .stack_address = &_estack,
  .reset_handler = (void*)Reset_Handler,
  .length = (uint32_t)&_firmware_length,
  .version_ptr = (uintptr_t)&_firmware_version,
};


