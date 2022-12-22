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

#include "gtests.h"
#include "mixer_scheduler.h"

TEST(MixerScheduler, MultiModules)
{
  mixerSchedulerInit();

  EXPECT_EQ(getMixerSchedulerPeriod(), MIXER_SCHEDULER_DEFAULT_PERIOD_US);

  mixerSchedulerSetPeriod(INTERNAL_MODULE, 7000);
  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 2000);

  EXPECT_EQ(getMixerSchedulerPeriod(), 2000);
  EXPECT_EQ(getMixerSchedulerRealPeriod(INTERNAL_MODULE), 8000);
  EXPECT_EQ(getMixerSchedulerRealPeriod(EXTERNAL_MODULE), 2000);

  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 3003);

  EXPECT_EQ(getMixerSchedulerPeriod(), 3003);
  EXPECT_EQ(getMixerSchedulerRealPeriod(INTERNAL_MODULE), 9009);
  EXPECT_EQ(getMixerSchedulerRealPeriod(EXTERNAL_MODULE), 3003);

  mixerSchedulerSetPeriod(EXTERNAL_MODULE, 10000);

  EXPECT_EQ(getMixerSchedulerPeriod(), 7000);
  EXPECT_EQ(getMixerSchedulerRealPeriod(INTERNAL_MODULE), 7000);
  EXPECT_EQ(getMixerSchedulerRealPeriod(EXTERNAL_MODULE), 14000);
}
