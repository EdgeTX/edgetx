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

// Unit tests for RawSource and RawSwitch value encoding/decoding.
// These classes form the core data model that every conversion test relies on,
// but their encoding invariants were not directly tested.

#include "gtests.h"
#include "firmwares/rawsource.h"
#include "firmwares/rawswitch.h"
#include "storage/storage.h"

// ---------------------------------------------------------------------------
// RawSource tests
// ---------------------------------------------------------------------------

TEST(RawSource, defaultConstructedIsNone)
{
  RawSource src;
  EXPECT_EQ(SOURCE_TYPE_NONE, src.type);
  EXPECT_EQ(0, src.index);
  EXPECT_FALSE(src.isSet());
  EXPECT_EQ(0, src.toValue());
}

TEST(RawSource, clearResetsToNone)
{
  RawSource src(SOURCE_TYPE_TELEMETRY, 3);
  src.clear();
  EXPECT_EQ(SOURCE_TYPE_NONE, src.type);
  EXPECT_EQ(0, src.index);
  EXPECT_FALSE(src.isSet());
}

// For non-NONE sources with index=0 the constructor normalises index to 1.
// This documents the intentional "first item is 1, not 0" design.
TEST(RawSource, zeroIndexNormalisedToOne)
{
  RawSource src(SOURCE_TYPE_TELEMETRY, 0);
  EXPECT_EQ(SOURCE_TYPE_TELEMETRY, src.type);
  EXPECT_EQ(1, src.index);
  EXPECT_TRUE(src.isSet());
}

// SOURCE_TYPE_NONE with index=0 must NOT normalise — stays at 0.
TEST(RawSource, noneTypeKeepsZeroIndex)
{
  RawSource src(SOURCE_TYPE_NONE, 0);
  EXPECT_EQ(SOURCE_TYPE_NONE, src.type);
  EXPECT_EQ(0, src.index);
  EXPECT_EQ(0, src.toValue());
}

// toValue() / RawSource(int) round-trip for a variety of types and indices.
TEST(RawSource, roundTrip)
{
  const RawSource cases[] = {
    RawSource(SOURCE_TYPE_TELEMETRY, 5),
    RawSource(SOURCE_TYPE_PPM,       2),
    RawSource(SOURCE_TYPE_CH,        4),
    RawSource(SOURCE_TYPE_INPUT,     1),
    RawSource(SOURCE_TYPE_GVAR,      3),
    RawSource(),                          // NONE — stays 0
  };
  for (const auto& src : cases) {
    const RawSource decoded(src.toValue());
    EXPECT_EQ(src.type,  decoded.type)  << "type mismatch (index=" << src.index << ")";
    EXPECT_EQ(src.index, decoded.index) << "index mismatch (type=" << src.type  << ")";
  }
}

// Encoding is type * 65536 + index; verify the arithmetic directly.
TEST(RawSource, toValueEncoding)
{
  const int type  = SOURCE_TYPE_PPM;  // 11
  const int index = 3;
  RawSource src(SOURCE_TYPE_PPM, index);
  EXPECT_EQ(type * 65536 + index, src.toValue());
}

// ---------------------------------------------------------------------------
// RawSwitch tests
// ---------------------------------------------------------------------------

TEST(RawSwitch, defaultConstructedIsNone)
{
  RawSwitch sw;
  EXPECT_EQ(SWITCH_TYPE_NONE, sw.type);
  EXPECT_EQ(0, sw.index);
  EXPECT_FALSE(sw.isSet());
  EXPECT_EQ(0, sw.toValue());
}

TEST(RawSwitch, clearResetsToNone)
{
  RawSwitch sw(SWITCH_TYPE_SWITCH, SWITCH_SA0);
  sw.clear();
  EXPECT_EQ(SWITCH_TYPE_NONE, sw.type);
  EXPECT_EQ(0, sw.index);
  EXPECT_FALSE(sw.isSet());
}

// toValue() / RawSwitch(int) round-trip for positive indices.
TEST(RawSwitch, roundTripPositiveIndex)
{
  const RawSwitch cases[] = {
    RawSwitch(SWITCH_TYPE_SWITCH,    SWITCH_SA0),
    RawSwitch(SWITCH_TYPE_TELEMETRY, 1),
    RawSwitch(SWITCH_TYPE_VIRTUAL,   3),
    RawSwitch(SWITCH_TYPE_ON),
    RawSwitch(),
  };
  for (const auto& sw : cases) {
    const RawSwitch decoded(sw.toValue());
    EXPECT_EQ(sw.type,  decoded.type)  << "type mismatch (index=" << sw.index << ")";
    EXPECT_EQ(sw.index, decoded.index) << "index mismatch (type=" << sw.type  << ")";
  }
}

// Negated switches (index < 0) encode and decode correctly.
TEST(RawSwitch, roundTripNegativeIndex)
{
  RawSwitch sw(SWITCH_TYPE_SWITCH, -SWITCH_SA0);
  const int value = sw.toValue();
  EXPECT_LT(value, 0) << "negative index must produce negative value";
  const RawSwitch decoded(value);
  EXPECT_EQ(sw.type,  decoded.type);
  EXPECT_EQ(sw.index, decoded.index);
  EXPECT_LT(decoded.index, 0);
}

// Encoding is type * 256 + index; verify the arithmetic directly.
TEST(RawSwitch, toValueEncoding)
{
  const int type  = SWITCH_TYPE_SWITCH;  // 1
  const int index = SWITCH_SA0;          // 1
  RawSwitch sw(SWITCH_TYPE_SWITCH, SWITCH_SA0);
  EXPECT_EQ(type * 256 + index, sw.toValue());
}

TEST(RawSwitch, equality)
{
  EXPECT_EQ(RawSwitch(SWITCH_TYPE_ON),  RawSwitch(SWITCH_TYPE_ON));
  EXPECT_NE(RawSwitch(SWITCH_TYPE_ON),  RawSwitch(SWITCH_TYPE_OFF));
  EXPECT_NE(RawSwitch(SWITCH_TYPE_SWITCH, SWITCH_SA0),
            RawSwitch(SWITCH_TYPE_SWITCH, SWITCH_SA1));
}

// ---------------------------------------------------------------------------
// Storage error handling
// ---------------------------------------------------------------------------

TEST(Storage, loadFromNonExistentPathReturnsFalse)
{
  Storage store(QString("/nonexistent/path/does/not/exist.bin"));
  RadioData radioData;
  // Must fail gracefully — no crash, no exception, returns false.
  EXPECT_FALSE(store.load(radioData));
}
