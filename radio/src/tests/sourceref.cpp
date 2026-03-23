/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "gtests.h"
#include "sourceref.h"

// ---- SourceRef struct tests ----

TEST(SourceRef, SizeIs32Bits)
{
  EXPECT_EQ(sizeof(SourceRef), 4u);
}

TEST(SourceRef, DefaultIsNone)
{
  SourceRef ref = {};
  EXPECT_TRUE(ref.isNone());
  EXPECT_FALSE(ref.isInverted());
  EXPECT_EQ(ref.index, 0);
}

TEST(SourceRef, ClearResetsAll)
{
  SourceRef ref = SourceRef_(SOURCE_TYPE_TELEMETRY, 42, SOURCE_FLAG_INVERTED);
  ref.clear();
  EXPECT_TRUE(ref.isNone());
  EXPECT_FALSE(ref.isInverted());
  EXPECT_EQ(ref.index, 0);
}

TEST(SourceRef, Equality)
{
  SourceRef a = SourceRef_(SOURCE_TYPE_STICK, 2);
  SourceRef b = SourceRef_(SOURCE_TYPE_STICK, 2);
  SourceRef c = SourceRef_(SOURCE_TYPE_STICK, 3);
  SourceRef d = SourceRef_(SOURCE_TYPE_POT, 2);

  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}

TEST(SourceRef, InvertedFlag)
{
  SourceRef ref = SourceRef_(SOURCE_TYPE_STICK, 1, SOURCE_FLAG_INVERTED);
  EXPECT_TRUE(ref.isInverted());
  EXPECT_EQ(ref.type, SOURCE_TYPE_STICK);
  EXPECT_EQ(ref.index, 1);
}

TEST(SourceRef, HighIndex)
{
  // 16-bit index supports values far beyond old 10-bit limit
  SourceRef ref = SourceRef_(SOURCE_TYPE_TELEMETRY, 65535);
  EXPECT_EQ(ref.type, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(ref.index, 65535);
}

// ---- SwitchRef struct tests ----

TEST(SwitchRef, SizeIs32Bits)
{
  EXPECT_EQ(sizeof(SwitchRef), 4u);
}

TEST(SwitchRef, DefaultIsNone)
{
  SwitchRef ref = {};
  EXPECT_TRUE(ref.isNone());
  EXPECT_FALSE(ref.isInverted());
}

TEST(SwitchRef, InvertedFlag)
{
  SwitchRef ref = SwitchRef_(SWITCH_TYPE_LOGICAL, 5, SWITCH_FLAG_INVERTED);
  EXPECT_TRUE(ref.isInverted());
  EXPECT_EQ(ref.type, SWITCH_TYPE_LOGICAL);
  EXPECT_EQ(ref.index, 5);
}

TEST(SwitchRef, Equality)
{
  SwitchRef a = SwitchRef_(SWITCH_TYPE_SWITCH, 10);
  SwitchRef b = SwitchRef_(SWITCH_TYPE_SWITCH, 10);
  SwitchRef c = SwitchRef_(SWITCH_TYPE_LOGICAL, 10);

  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
}

// ---- ValueOrSource struct tests ----

TEST(ValueOrSource, SizeIs32Bits)
{
  EXPECT_EQ(sizeof(ValueOrSource), 4u);
}

TEST(ValueOrSource, NumericValue)
{
  ValueOrSource v = {};
  v.setNumeric(42);
  EXPECT_EQ(v.isSource, 0);
  EXPECT_EQ(v.numericValue(), 42);
}

TEST(ValueOrSource, NegativeNumericValue)
{
  ValueOrSource v = {};
  v.setNumeric(-100);
  EXPECT_EQ(v.isSource, 0);
  EXPECT_EQ(v.numericValue(), -100);
}

TEST(ValueOrSource, SourceReference)
{
  SourceRef src = SourceRef_(SOURCE_TYPE_GVAR, 3);
  ValueOrSource v = {};
  v.setSource(src);

  EXPECT_EQ(v.isSource, 1);
  EXPECT_EQ(v.srcType, SOURCE_TYPE_GVAR);
  EXPECT_EQ(v.value, 3);  // index stored in value

  SourceRef back = v.toSourceRef();
  EXPECT_EQ(back.type, SOURCE_TYPE_GVAR);
  EXPECT_EQ(back.index, 3);
}

TEST(ValueOrSource, TelemetrySourceReference)
{
  // Can hold telemetry sensor indices that overflow old 10-bit field
  SourceRef src = SourceRef_(SOURCE_TYPE_TELEMETRY, 240);  // sensor 80
  ValueOrSource v = {};
  v.setSource(src);

  EXPECT_EQ(v.isSource, 1);
  EXPECT_EQ(v.srcType, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(v.value, 240);

  SourceRef back = v.toSourceRef();
  EXPECT_EQ(back.type, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(back.index, 240);
}

TEST(ValueOrSource, Clear)
{
  ValueOrSource v = {};
  v.setSource(SourceRef_(SOURCE_TYPE_GVAR, 5));
  v.clear();
  EXPECT_EQ(v.value, 0);
  EXPECT_EQ(v.isSource, 0);
  EXPECT_EQ(v.srcType, 0);
}

TEST(ValueOrSource, SwitchBetweenModes)
{
  ValueOrSource v = {};

  // Start as numeric
  v.setNumeric(75);
  EXPECT_EQ(v.isSource, 0);
  EXPECT_EQ(v.numericValue(), 75);

  // Switch to source
  v.setSource(SourceRef_(SOURCE_TYPE_GVAR, 2));
  EXPECT_EQ(v.isSource, 1);
  EXPECT_EQ(v.srcType, SOURCE_TYPE_GVAR);

  // Switch back to numeric
  v.setNumeric(-50);
  EXPECT_EQ(v.isSource, 0);
  EXPECT_EQ(v.numericValue(), -50);
  EXPECT_EQ(v.srcType, 0);
}
