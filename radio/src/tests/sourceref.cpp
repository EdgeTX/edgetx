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
  EXPECT_EQ(sizeof(SwitchRef), 4u);
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
  SourceRef ref = {SOURCE_TYPE_TELEMETRY, SOURCE_FLAG_INVERTED, 42};
  ref.clear();
  EXPECT_TRUE(ref.isNone());
  EXPECT_FALSE(ref.isInverted());
  EXPECT_EQ(ref.index, 0);
}

TEST(SourceRef, Equality)
{
  SourceRef a = {SOURCE_TYPE_STICK, 0, 2};
  SourceRef b = {SOURCE_TYPE_STICK, 0, 2};
  SourceRef c = {SOURCE_TYPE_STICK, 0, 3};
  SourceRef d = {SOURCE_TYPE_POT, 0, 2};

  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_NE(a, d);
}

// ---- MixSources <-> SourceRef round-trip ----

TEST(SourceRef, NoneRoundTrip)
{
  SourceRef ref = sourceRefFromMixSrc(MIXSRC_NONE);
  EXPECT_TRUE(ref.isNone());
  EXPECT_EQ(mixSrcFromSourceRef(ref), MIXSRC_NONE);
}

TEST(SourceRef, InputRoundTrip)
{
  for (int i = 0; i < MAX_INPUTS; i++) {
    int32_t src = MIXSRC_FIRST_INPUT + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_INPUT);
    EXPECT_EQ(ref.index, i);
    EXPECT_FALSE(ref.isInverted());
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, StickRoundTrip)
{
  for (int i = 0; i < MAX_STICKS; i++) {
    int32_t src = MIXSRC_FIRST_STICK + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_STICK);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, ChannelRoundTrip)
{
  for (int i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
    int32_t src = MIXSRC_FIRST_CH + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_CHANNEL);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, TelemetryRoundTrip)
{
  // Test all telemetry sensors (3 values per sensor: val, min, max)
  for (int i = 0; i < 3 * MAX_TELEMETRY_SENSORS; i++) {
    int32_t src = MIXSRC_FIRST_TELEM + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_TELEMETRY);
    EXPECT_EQ(ref.index, i);
    // Round-trip always works with SourceRef (no 10-bit limit)
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, LogicalSwitchRoundTrip)
{
  for (int i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    int32_t src = MIXSRC_FIRST_LOGICAL_SWITCH + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_LOGICAL_SWITCH);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, GVarRoundTrip)
{
  for (int i = 0; i < MAX_GVARS; i++) {
    int32_t src = MIXSRC_FIRST_GVAR + i;
    SourceRef ref = sourceRefFromMixSrc(src);
    EXPECT_EQ(ref.type, SOURCE_TYPE_GVAR);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(mixSrcFromSourceRef(ref), src);
  }
}

TEST(SourceRef, InvertedRoundTrip)
{
  // Negative srcRaw = inverted source
  int32_t src = -(MIXSRC_FIRST_STICK + 1);
  SourceRef ref = sourceRefFromMixSrc(src);
  EXPECT_EQ(ref.type, SOURCE_TYPE_STICK);
  EXPECT_EQ(ref.index, 1);
  EXPECT_TRUE(ref.isInverted());
  EXPECT_EQ(mixSrcFromSourceRef(ref), src);
}

TEST(SourceRef, MinMaxRoundTrip)
{
  SourceRef refMin = sourceRefFromMixSrc(MIXSRC_MIN);
  EXPECT_EQ(refMin.type, SOURCE_TYPE_MIN);
  EXPECT_EQ(mixSrcFromSourceRef(refMin), (int32_t)MIXSRC_MIN);

  SourceRef refMax = sourceRefFromMixSrc(MIXSRC_MAX);
  EXPECT_EQ(refMax.type, SOURCE_TYPE_MAX);
  EXPECT_EQ(mixSrcFromSourceRef(refMax), (int32_t)MIXSRC_MAX);
}

TEST(SourceRef, TxSpecialRoundTrip)
{
  SourceRef ref;

  ref = sourceRefFromMixSrc(MIXSRC_TX_VOLTAGE);
  EXPECT_EQ(ref.type, SOURCE_TYPE_TX_VOLTAGE);
  EXPECT_EQ(mixSrcFromSourceRef(ref), (int32_t)MIXSRC_TX_VOLTAGE);

  ref = sourceRefFromMixSrc(MIXSRC_TX_TIME);
  EXPECT_EQ(ref.type, SOURCE_TYPE_TX_TIME);
  EXPECT_EQ(mixSrcFromSourceRef(ref), (int32_t)MIXSRC_TX_TIME);
}

// ---- SwitchSources <-> SwitchRef round-trip ----

TEST(SwitchRef, NoneRoundTrip)
{
  SwitchRef ref = switchRefFromSwSrc(SWSRC_NONE);
  EXPECT_TRUE(ref.isNone());
  EXPECT_EQ(swSrcFromSwitchRef(ref), SWSRC_NONE);
}

TEST(SwitchRef, PhysicalSwitchRoundTrip)
{
  for (int i = 0; i <= SWSRC_LAST_SWITCH - SWSRC_FIRST_SWITCH; i++) {
    int32_t sw = SWSRC_FIRST_SWITCH + i;
    SwitchRef ref = switchRefFromSwSrc(sw);
    EXPECT_EQ(ref.type, SWITCH_TYPE_SWITCH);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(swSrcFromSwitchRef(ref), sw);
  }
}

TEST(SwitchRef, LogicalSwitchRoundTrip)
{
  for (int i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    int32_t sw = SWSRC_FIRST_LOGICAL_SWITCH + i;
    SwitchRef ref = switchRefFromSwSrc(sw);
    EXPECT_EQ(ref.type, SWITCH_TYPE_LOGICAL);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(swSrcFromSwitchRef(ref), sw);
  }
}

TEST(SwitchRef, InvertedRoundTrip)
{
  int32_t sw = -(SWSRC_FIRST_LOGICAL_SWITCH + 5);
  SwitchRef ref = switchRefFromSwSrc(sw);
  EXPECT_EQ(ref.type, SWITCH_TYPE_LOGICAL);
  EXPECT_EQ(ref.index, 5);
  EXPECT_TRUE(ref.isInverted());
  EXPECT_EQ(swSrcFromSwitchRef(ref), sw);
}

TEST(SwitchRef, OnRoundTrip)
{
  SwitchRef ref = switchRefFromSwSrc(SWSRC_ON);
  EXPECT_EQ(ref.type, SWITCH_TYPE_ON);
  EXPECT_EQ(swSrcFromSwitchRef(ref), (int32_t)SWSRC_ON);
}

TEST(SwitchRef, FlightModeRoundTrip)
{
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    int32_t sw = SWSRC_FIRST_FLIGHT_MODE + i;
    SwitchRef ref = switchRefFromSwSrc(sw);
    EXPECT_EQ(ref.type, SWITCH_TYPE_FLIGHT_MODE);
    EXPECT_EQ(ref.index, i);
    EXPECT_EQ(swSrcFromSwitchRef(ref), sw);
  }
}

// ---- SourceRef can handle values that overflow srcRaw:10 ----

TEST(SourceRef, HighTelemetryIndexRoundTrip)
{
  // SourceRef can hold telemetry indices that would overflow srcRaw:10.
  // Even if the current build doesn't have that many sensors,
  // we can create a SourceRef with a high index and verify round-trip.
  SourceRef ref = {};
  ref.type = SOURCE_TYPE_TELEMETRY;
  ref.index = 3 * 80;  // sensor 80, value slot

  // Convert to MixSources and back
  int32_t mixsrc = mixSrcFromSourceRef(ref);
  EXPECT_EQ(mixsrc, (int32_t)(MIXSRC_FIRST_TELEM + 3 * 80));

  SourceRef back = sourceRefFromMixSrc(mixsrc);
  // This round-trip works if MIXSRC_FIRST_TELEM + 240 <= MIXSRC_LAST_TELEM
  // On builds with < 81 sensors, the mixsrc is out of the enum range
  // and sourceRefFromMixSrc won't match it. That's expected —
  // the point is that SourceRef ITSELF can hold the value.
  if (MIXSRC_FIRST_TELEM + 3 * 80 <= MIXSRC_LAST_TELEM) {
    EXPECT_EQ(back, ref);
  }

  // Direct construction always works, regardless of enum range
  EXPECT_EQ(ref.type, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(ref.index, 240);
  EXPECT_EQ(sizeof(ref), 4u);
}

// ---- Full enum round-trip (all valid MixSources) ----

TEST(SourceRef, AllMixSourcesRoundTrip)
{
  // Test every valid MixSources value round-trips through SourceRef
  int failures = 0;
  for (int32_t src = MIXSRC_FIRST; src <= MIXSRC_LAST_TELEM; src++) {
    SourceRef ref = sourceRefFromMixSrc(src);
    if (ref.type == SOURCE_TYPE_NONE && src != MIXSRC_NONE) {
      // Unmapped source (e.g., platform-specific gap)
      continue;
    }
    int32_t back = mixSrcFromSourceRef(ref);
    if (back != src) {
      failures++;
      if (failures <= 5) {
        EXPECT_EQ(back, src) << "Round-trip failed for MIXSRC " << src
                              << " (type=" << (int)ref.type
                              << " index=" << ref.index << ")";
      }
    }
  }
  EXPECT_EQ(failures, 0) << failures << " MixSources values failed round-trip";
}
