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

#include "hal/storage.h"
#include "hal.h"

#include "debug.h"

#if defined(STORAGE_USE_SDIO)
  #include "diskio_sdio.h"
  #define _STORAGE_DRIVER sdio_diskio_driver
#elif defined(STORAGE_USE_SDCARD_SPI)
  #include "diskio_spi.h"
  #define _STORAGE_DRIVER sdcard_spi_driver
#elif defined(STORAGE_USE_SPI_FLASH)
  #include "diskio_spi_flash.h"
  #define _STORAGE_DRIVER spi_flash_diskio_driver
#else
  #error "No supported storage driver configured"
#endif

#if defined(DISK_CACHE)
  #include "disk_cache.h"
  const diskio_driver_t disk_cache_shim = {
    .initialize = _STORAGE_DRIVER.initialize,
    .status = _STORAGE_DRIVER.status,
    .read = disk_cache_read,
    .write = disk_cache_write,
    .ioctl = _STORAGE_DRIVER.ioctl,
  };
#endif

void storageInit()
{
  if (fatfsGetDriver(0) != nullptr)
    return;

  const diskio_driver_t* drv = &_STORAGE_DRIVER;

#if defined(DISK_CACHE)
  diskCache.initialize(drv);
  drv = &disk_cache_shim;
#endif

  if (!fatfsRegisterDriver(drv, 0)) {
    TRACE("fatfsRegisterDriver: [FAILED]");
  }
}

void storageDeInit()
{
  fatfsUnregisterDrivers();
}

void storagePreMountHook()
{
#if defined(DISK_CACHE)
  diskCache.clear();
#endif
}

bool storageIsPresent()
{
  return (_STORAGE_DRIVER.status(0) & STA_NODISK) == 0;
}

const diskio_driver_t* storageGetDefaultDriver()
{
  return &_STORAGE_DRIVER;
}
