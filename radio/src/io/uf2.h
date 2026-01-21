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
#include "boards/FirmwareVersionTag.h"

#define UF2_VERSION_TAG 0x9FC7BC
#define UF2_DEVICE_TAG  0x650D9D
#define UF2_REBOOT_TAG  0xE60835

// ignore family ID if not defined
#if !defined(UF2_FAMILY_ID)
#define UF2_FAMILY_ID 0
#endif 

bool isUF2Block(const void* block, uint32_t len);
bool isUF2FirmwareImage(const void* buffer, uint32_t length);
bool isUF2FirmwareFile(const char * filename);

bool extractUF2FirmwareVersion(const void* block, VersionTag* tag);

#if defined(STM32) && !defined(SIMU)
void writeUF2FirmwareVersion(void* block);
void writeUF2RebootBlock(void* block);
#endif
