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

#include "intmodule_heartbeat.h"

#include "heartbeat_driver.h"
#include "mixer_scheduler.h"
#include "dataconstants.h"
#include "pulses/pxx.h"

#if defined(PXX1)
  #include "pulses/pxx1.h"
  #define isPxx1Driver(drv) (drv == &Pxx1Driver)
#else
  #define isPxx1Driver(drv) (false)
#endif

#if defined(PXX2)
  // #include "pulses/pxx2.h"
  extern const etx_proto_driver_t Pxx2Driver;
  #define isPxx2Driver(drv) (drv == &Pxx2Driver)
#else
  #define isPxx2Driver(drv) (false)
#endif

void _intmodule_heartbeat_init(uint8_t module, const etx_proto_driver_t* drv)
{
  if (module == INTERNAL_MODULE && (isPxx1Driver(drv) || isPxx2Driver(drv))) {
    init_intmodule_heartbeat();
    if (isPxx1Driver(drv)) {
      // XJT / iXJT
      auto period = mixerSchedulerGetPeriod(module);
      mixerSchedulerSetPeriod(module, period + 1000/*us*/);
    } else if (isPxx2Driver(drv)) {
      // ISRM
      mixerSchedulerSetPeriod(module, PXX2_MAX_HEARTBEAT_PERIOD);
    }
  }
}

void _intmodule_heartbeat_deinit(uint8_t module, const etx_proto_driver_t* drv)
{
  if (module == INTERNAL_MODULE) {
    stop_intmodule_heartbeat();
  }
}
