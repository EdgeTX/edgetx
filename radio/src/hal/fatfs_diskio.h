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

#include "FatFs/diskio.h"
#include <stdint.h>

struct diskio_driver_t
{
  DSTATUS (*initialize)(BYTE pdrv);

  DSTATUS (*deinit)(BYTE pdrv);

  DSTATUS (*status)(BYTE pdrv);

  DRESULT (*read)(BYTE pdrv, BYTE* buff, DWORD sector, UINT count);

  DRESULT (*write)(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);

  DRESULT (*ioctl)(BYTE pdrv, BYTE cmd, void* buff);
};

// returns 1 if successful, 0 otherwise
int fatfsRegisterDriver(const diskio_driver_t* drv, uint8_t lun);

// gracefully tear down all drivers
void fatfsUnregisterDrivers();

// returns a pyhsical disk driver or NULL
const diskio_driver_t* fatfsGetDriver(uint8_t pdrv);

// returns a physical LUN or 0
uint8_t fatfsGetLun(uint8_t pdrv);
