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

TEST(Haptic, DefaultIntensityUsesUserStrength)
{
  hapticQueue hq;
  EXPECT_EQ(userHapticStrength, hq.getIntensity());
}

TEST(Haptic, PlayWithoutIntensityKeepsUserStrength)
{
  hapticQueue hq;
  hq.play(15, 3, PLAY_NOW);
  EXPECT_EQ(userHapticStrength, hq.getIntensity());
}

TEST(Haptic, PlayAcceptsCustomIntensityWithCustomDuration)
{
  hapticQueue hq;
  hq.play(15, 3, PLAY_NOW, 50);
  EXPECT_TRUE(hq.busy());
  EXPECT_EQ(50, hq.getIntensity());
}

TEST(Haptic, PlayAcceptsMinAndMaxIntensity)
{
  hapticQueue hq;
  hq.play(15, 3, PLAY_NOW, 0);
  EXPECT_EQ(0, hq.getIntensity());

  hq.play(15, 3, PLAY_NOW, 100);
  EXPECT_EQ(100, hq.getIntensity());
}

TEST(Haptic, QueuedPlayDoesNotChangeCurrentIntensity)
{
  hapticQueue hq;
  hq.play(15, 3, PLAY_NOW, 50);
  ASSERT_TRUE(hq.busy());
  ASSERT_EQ(50, hq.getIntensity());

  // queue busy -> gets queued rather than applied immediately
  hq.play(5, 1, PLAY_REPEAT(1), 80);
  EXPECT_EQ(50, hq.getIntensity());
}
