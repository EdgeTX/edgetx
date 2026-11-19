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

TEST(TelemetryUnits, RadiansToDegrees)
{
  // pi/2 rad (prec 3) -> 90.0 deg (prec 1)
  EXPECT_EQ(900, convertTelemetryValue(1571, UNIT_RADIANS, 3, UNIT_DEGREE, 1));
  EXPECT_EQ(-900, convertTelemetryValue(-1571, UNIT_RADIANS, 3, UNIT_DEGREE, 1));

  // pi rad (prec 3) -> 180 deg (prec 0)
  EXPECT_EQ(180, convertTelemetryValue(3142, UNIT_RADIANS, 3, UNIT_DEGREE, 0));

  // 1 rad (prec 0) -> 57.29 deg (prec 2)
  EXPECT_EQ(5729, convertTelemetryValue(1, UNIT_RADIANS, 0, UNIT_DEGREE, 2));
}

TEST(TelemetryUnits, DegreesToRadians)
{
  // 90 deg (prec 0) -> 1.57 rad (prec 2)
  EXPECT_EQ(157, convertTelemetryValue(90, UNIT_DEGREE, 0, UNIT_RADIANS, 2));

  // 180.0 deg (prec 1) -> 3.141 rad (prec 3), result is truncated
  EXPECT_EQ(3141, convertTelemetryValue(1800, UNIT_DEGREE, 1, UNIT_RADIANS, 3));
  EXPECT_EQ(-3141, convertTelemetryValue(-1800, UNIT_DEGREE, 1, UNIT_RADIANS, 3));
}

TEST(TelemetryUnits, SameUnitPrecisionChange)
{
  EXPECT_EQ(15, convertTelemetryValue(1571, UNIT_RADIANS, 3, UNIT_RADIANS, 1));
  EXPECT_EQ(15710, convertTelemetryValue(1571, UNIT_RADIANS, 3, UNIT_RADIANS, 4));
}
