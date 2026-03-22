/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "gtests.h"
#include "model_arena.h"
#include "mixes.h"
#include "expos.h"
#include "customfn.h"
#include "curves.h"

// ---- ModelArena unit tests ----

class ArenaTest : public testing::Test {
 protected:
  static constexpr uint32_t TEST_ARENA_SIZE = 1024;
  uint8_t buf[TEST_ARENA_SIZE];
  ModelArena arena;

  void SetUp() override {
    memset(buf, 0, sizeof(buf));
    arena.attach(buf, TEST_ARENA_SIZE);
  }
};

TEST_F(ArenaTest, AttachSetsCapacity)
{
  EXPECT_EQ(arena.capacity(), TEST_ARENA_SIZE);
  EXPECT_EQ(arena.base(), buf);
  EXPECT_EQ(arena.usedBytes(), (uint32_t)0);
  EXPECT_EQ(arena.freeBytes(), TEST_ARENA_SIZE);
}

TEST_F(ArenaTest, LayoutComputesOffsets)
{
  ModelDynData dyn = {};
  dyn.mixCount = 3;
  dyn.expoCount = 2;
  dyn.curveCount = 1;
  dyn.pointsCount = 5;
  dyn.logicalSwCount = 4;
  dyn.customFnCount = 2;

  arena.layout(dyn);

  EXPECT_EQ(arena.sectionOffset(ARENA_MIXES), (uint32_t)0);
  EXPECT_EQ(arena.sectionOffset(ARENA_EXPOS), 3u * sizeof(MixData));
  EXPECT_EQ(arena.sectionOffset(ARENA_CURVES),
            3u * sizeof(MixData) + 2u * sizeof(ExpoData));

  uint32_t expectedUsed =
      3 * sizeof(MixData) +
      2 * sizeof(ExpoData) +
      1 * sizeof(CurveHeader) +
      5 * sizeof(int8_t) +
      4 * sizeof(LogicalSwitchData) +
      2 * sizeof(CustomFunctionData);
  EXPECT_EQ(arena.usedBytes(), expectedUsed);
  EXPECT_EQ(arena.freeBytes(), TEST_ARENA_SIZE - expectedUsed);
}

TEST_F(ArenaTest, ClearZerosDataPreservesLayout)
{
  ModelDynData dyn = {};
  dyn.mixCount = 2;
  dyn.expoCount = 1;
  arena.layout(dyn);

  // Write some data
  buf[0] = 0xAA;
  buf[sizeof(MixData)] = 0xBB;

  uint32_t usedBefore = arena.usedBytes();
  uint32_t offsetBefore = arena.sectionOffset(ARENA_EXPOS);

  arena.clear();

  EXPECT_EQ(buf[0], 0);
  EXPECT_EQ(buf[sizeof(MixData)], 0);
  // Layout preserved
  EXPECT_EQ(arena.usedBytes(), usedBefore);
  EXPECT_EQ(arena.sectionOffset(ARENA_EXPOS), offsetBefore);
}

TEST_F(ArenaTest, InsertSlotShiftsData)
{
  // Set up a small layout
  ModelDynData dyn = {};
  dyn.mixCount = 2;
  arena.layout(dyn);

  // Write known data to mix slots
  MixData* mix0 = reinterpret_cast<MixData*>(arena.sectionBase(ARENA_MIXES));
  mix0[0].destCh = 5;
  mix0[1].destCh = 10;

  // Insert a slot at the beginning
  EXPECT_TRUE(arena.insertSlot(0, sizeof(MixData)));

  // Original data should have shifted forward
  MixData* newMix = reinterpret_cast<MixData*>(arena.sectionBase(ARENA_MIXES));
  EXPECT_EQ(newMix[0].destCh, 0);  // new empty slot
  EXPECT_EQ(newMix[1].destCh, 5);  // shifted
  EXPECT_EQ(newMix[2].destCh, 10); // shifted
}

TEST_F(ArenaTest, DeleteSlotShiftsData)
{
  ModelDynData dyn = {};
  dyn.mixCount = 3;
  arena.layout(dyn);

  MixData* mix = reinterpret_cast<MixData*>(arena.sectionBase(ARENA_MIXES));
  mix[0].destCh = 1;
  mix[1].destCh = 2;
  mix[2].destCh = 3;

  arena.deleteSlot(0, sizeof(MixData));

  EXPECT_EQ(mix[0].destCh, 2);
  EXPECT_EQ(mix[1].destCh, 3);
}

TEST_F(ArenaTest, InsertSlotFailsWhenFull)
{
  // Fill the arena completely
  ModelDynData dyn = {};
  dyn.mixCount = TEST_ARENA_SIZE / sizeof(MixData);
  arena.layout(dyn);

  EXPECT_FALSE(arena.insertSlot(0, sizeof(MixData)));
}

TEST_F(ArenaTest, InsertInSectionUpdatesSubsequentOffsets)
{
  ModelDynData dyn = {};
  dyn.mixCount = 2;
  dyn.expoCount = 2;
  dyn.curveCount = 1;
  arena.layout(dyn);

  uint32_t expoOffBefore = arena.sectionOffset(ARENA_EXPOS);
  uint32_t curveOffBefore = arena.sectionOffset(ARENA_CURVES);

  // Insert a mix element
  EXPECT_TRUE(arena.insertInSection(ARENA_MIXES, 0, sizeof(MixData)));

  // Expo and curve offsets should shift forward by sizeof(MixData)
  EXPECT_EQ(arena.sectionOffset(ARENA_EXPOS),
            expoOffBefore + sizeof(MixData));
  EXPECT_EQ(arena.sectionOffset(ARENA_CURVES),
            curveOffBefore + sizeof(MixData));
}

TEST_F(ArenaTest, DeleteFromSectionUpdatesSubsequentOffsets)
{
  ModelDynData dyn = {};
  dyn.mixCount = 3;
  dyn.expoCount = 2;
  arena.layout(dyn);

  uint32_t expoOffBefore = arena.sectionOffset(ARENA_EXPOS);

  arena.deleteFromSection(ARENA_MIXES, 1, sizeof(MixData));

  EXPECT_EQ(arena.sectionOffset(ARENA_EXPOS),
            expoOffBefore - sizeof(MixData));
}

// ---- Accessor function tests with arena ----

class ArenaAccessorTest : public EdgeTxTest {};

TEST_F(ArenaAccessorTest, MixAddressPointsToArena)
{
  MixData* mix = mixAddress(0);
  EXPECT_NE(mix, nullptr);
  EXPECT_GE((uint8_t*)mix, g_modelArena.base());
  EXPECT_LT((uint8_t*)mix,
             g_modelArena.base() + g_modelArena.capacity());
}

TEST_F(ArenaAccessorTest, ExpoAddressPointsToArena)
{
  ExpoData* expo = expoAddress(0);
  EXPECT_NE(expo, nullptr);
  EXPECT_GE((uint8_t*)expo, g_modelArena.base());
  EXPECT_LT((uint8_t*)expo,
             g_modelArena.base() + g_modelArena.capacity());
}

TEST_F(ArenaAccessorTest, CurveHeaderAddressPointsToArena)
{
  CurveHeader* crv = curveHeaderAddress(0);
  EXPECT_NE(crv, nullptr);
  EXPECT_GE((uint8_t*)crv, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, CurvePointsBasePointsToArena)
{
  int8_t* pts = curvePointsBase();
  EXPECT_NE(pts, nullptr);
  EXPECT_GE((uint8_t*)pts, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, LswAddressPointsToArena)
{
  LogicalSwitchData* ls = lswAddress(0);
  EXPECT_NE(ls, nullptr);
  EXPECT_GE((uint8_t*)ls, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, CustomFnAddressPointsToArena)
{
  CustomFunctionData* cf = customFnAddress(0);
  EXPECT_NE(cf, nullptr);
  EXPECT_GE((uint8_t*)cf, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, MixDataRoundTrip)
{
  MixData* mix = mixAddress(3);
  mix->destCh = 7;
  mix->weight.setNumeric(50);
  mix->srcRaw = {SOURCE_TYPE_MAX, 0, 0};

  EXPECT_EQ(mixAddress(3)->destCh, 7);
  EXPECT_EQ(mixAddress(3)->weight.numericValue(), 50);
  EXPECT_EQ(mixAddress(3)->srcRaw, (SourceRef{SOURCE_TYPE_MAX, 0, 0}));
}

TEST_F(ArenaAccessorTest, ExpoDataRoundTrip)
{
  ExpoData* expo = expoAddress(5);
  expo->chn = 2;
  expo->weight.setNumeric(75);
  expo->mode = 3;

  EXPECT_EQ(expoAddress(5)->chn, 2);
  EXPECT_EQ(expoAddress(5)->weight.numericValue(), 75);
  EXPECT_EQ(expoAddress(5)->mode, 3);
}

TEST_F(ArenaAccessorTest, SectionsDoNotOverlap)
{
  // Verify that different section bases are at different addresses
  // and in the expected order
  uint8_t* mixBase = (uint8_t*)mixAddress(0);
  uint8_t* expoBase = (uint8_t*)expoAddress(0);
  uint8_t* curveBase = (uint8_t*)curveHeaderAddress(0);
  uint8_t* pointsBase = (uint8_t*)curvePointsBase();
  uint8_t* lsBase = (uint8_t*)lswAddress(0);
  uint8_t* cfBase = (uint8_t*)customFnAddress(0);

  EXPECT_LT(mixBase, expoBase);
  EXPECT_LT(expoBase, curveBase);
  EXPECT_LT(curveBase, pointsBase);
  EXPECT_LT(pointsBase, lsBase);
  EXPECT_LT(lsBase, cfBase);
}

// ---- Insert/delete centralized operations ----

class ArenaInsertDeleteTest : public EdgeTxTest {};

TEST_F(ArenaInsertDeleteTest, InsertMixPreservesExisting)
{
  MixData* mix0 = mixAddress(0);
  mix0->destCh = 0;
  mix0->srcRaw = {SOURCE_TYPE_MAX, 0, 0};
  mix0->weight.setNumeric(100);
  updateMixCount();

  insertMix(0, 0);

  // Original mix should have moved to index 1
  EXPECT_EQ(mixAddress(1)->srcRaw, (SourceRef{SOURCE_TYPE_MAX, 0, 0}));
  EXPECT_EQ(mixAddress(1)->weight.numericValue(), 100);
  // New mix at index 0
  EXPECT_EQ(mixAddress(0)->destCh, 0);
  EXPECT_NE(mixAddress(0)->srcRaw, (SourceRef{SOURCE_TYPE_MAX, 0, 0}));  // different default source
}

TEST_F(ArenaInsertDeleteTest, DeleteMixShiftsRemaining)
{
  mixAddress(0)->destCh = 0;
  mixAddress(0)->srcRaw = {SOURCE_TYPE_STICK, 0, 0};
  mixAddress(0)->weight.setNumeric(100);
  mixAddress(1)->destCh = 1;
  mixAddress(1)->srcRaw = {SOURCE_TYPE_MAX, 0, 0};
  mixAddress(1)->weight.setNumeric(50);
  updateMixCount();

  deleteMix(0);

  EXPECT_EQ(mixAddress(0)->destCh, 1);
  EXPECT_EQ(mixAddress(0)->srcRaw, (SourceRef{SOURCE_TYPE_MAX, 0, 0}));
  EXPECT_EQ(mixAddress(0)->weight.numericValue(), 50);
}

TEST_F(ArenaInsertDeleteTest, InsertExpoPreservesExisting)
{
  ExpoData* expo0 = expoAddress(0);
  expo0->chn = 0;
  expo0->srcRaw = {SOURCE_TYPE_STICK, 0, 0};
  expo0->weight.setNumeric(100);
  expo0->mode = 3;
  updateExpoCount();

  insertExpo(0, 0);

  // Original expo should have moved to index 1
  EXPECT_EQ(expoAddress(1)->srcRaw, (SourceRef{SOURCE_TYPE_STICK, 0, 0}));
  EXPECT_EQ(expoAddress(1)->weight.numericValue(), 100);
}

TEST_F(ArenaInsertDeleteTest, InsertDeleteCustomFn)
{
  CustomFunctionData* cf0 = customFnAddress(0);
  cf0->swtch = {SWITCH_TYPE_SWITCH, 0, 1};
  cf0->func = FUNC_OVERRIDE_CHANNEL;

  CustomFunctionData* cf1 = customFnAddress(1);
  cf1->swtch = {SWITCH_TYPE_SWITCH, 0, 2};
  cf1->func = FUNC_TRAINER;

  insertCustomFn(0);

  // Original functions should have shifted
  EXPECT_TRUE(customFnAddress(0)->swtch.isNone());  // new empty
  EXPECT_EQ(customFnAddress(1)->swtch, (SwitchRef{SWITCH_TYPE_SWITCH, 0, 1}));  // was cf0
  EXPECT_EQ(customFnAddress(1)->func, FUNC_OVERRIDE_CHANNEL);
  EXPECT_EQ(customFnAddress(2)->swtch, (SwitchRef{SWITCH_TYPE_SWITCH, 0, 2}));  // was cf1

  deleteCustomFn(0);

  // Back to original
  EXPECT_EQ(customFnAddress(0)->swtch, (SwitchRef{SWITCH_TYPE_SWITCH, 0, 1}));
  EXPECT_EQ(customFnAddress(0)->func, FUNC_OVERRIDE_CHANNEL);
  EXPECT_EQ(customFnAddress(1)->swtch, (SwitchRef{SWITCH_TYPE_SWITCH, 0, 2}));
  EXPECT_EQ(customFnAddress(1)->func, FUNC_TRAINER);
}

TEST_F(ArenaInsertDeleteTest, ClearCustomFn)
{
  customFnAddress(3)->swtch = {SWITCH_TYPE_SWITCH, 0, 5};
  customFnAddress(3)->func = FUNC_PLAY_SOUND;

  clearCustomFn(3);

  EXPECT_TRUE(customFnAddress(3)->swtch.isNone());
  EXPECT_EQ(customFnAddress(3)->func, 0);
}

// ---- Arena clear on model reset ----

TEST_F(ArenaAccessorTest, ModelResetClearsArena)
{
  // Write data to arena
  mixAddress(0)->destCh = 5;
  mixAddress(0)->srcRaw = {SOURCE_TYPE_MAX, 0, 0};
  expoAddress(0)->chn = 3;
  lswAddress(10)->func = LS_FUNC_VPOS;

  MODEL_RESET();

  // Arena data should be zeroed (default template may populate some entries)
  // but the specific entries we set should be gone
  EXPECT_EQ(lswAddress(10)->func, 0);
}

// ---- Bit-field capacity tests ----
// These verify that cross-reference fields can hold all valid values.
// Failures indicate that the enum range exceeds the bit-field width.

TEST(BitFieldCapacity, SourceRefRoundTrip)
{
  // SourceRef uses full bytes (type=uint8_t, index=uint16_t),
  // so no bit-field truncation. Verify round-trip works.
  MODEL_RESET();

  MixData* mix = mixAddress(0);

  // Test telemetry source
  mix->srcRaw = {SOURCE_TYPE_TELEMETRY, 0, 42};
  EXPECT_EQ(mix->srcRaw.type, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(mix->srcRaw.index, 42);

  // Test max index value for uint16_t
  mix->srcRaw = {SOURCE_TYPE_CHANNEL, 0, 255};
  EXPECT_EQ(mix->srcRaw.type, SOURCE_TYPE_CHANNEL);
  EXPECT_EQ(mix->srcRaw.index, 255);
}

TEST(BitFieldCapacity, SwitchRefRoundTrip)
{
  // SwitchRef uses full bytes (type=uint8_t, index=uint16_t),
  // so no bit-field truncation. Verify round-trip works.
  MODEL_RESET();

  MixData* mix = mixAddress(0);
  mix->swtch = {SWITCH_TYPE_SWITCH, 0, 42};
  EXPECT_EQ(mix->swtch.type, SWITCH_TYPE_SWITCH);
  EXPECT_EQ(mix->swtch.index, 42);

  mix->swtch = {SWITCH_TYPE_LOGICAL, SWITCH_FLAG_INVERTED, 10};
  EXPECT_EQ(mix->swtch.type, SWITCH_TYPE_LOGICAL);
  EXPECT_EQ(mix->swtch.flags, SWITCH_FLAG_INVERTED);
  EXPECT_EQ(mix->swtch.index, 10);
}

TEST(BitFieldCapacity, SwitchRefCanRoundTrip)
{
  // FlightModeData.swtch is now SwitchRef (4 bytes), can hold any switch.
  // Verify round-trip through the bridge functions for extreme values.
  SwitchRef ref = SwitchRef{SWITCH_TYPE_TRAINER, 0, 0};
  EXPECT_EQ(switchRefToSwSrc(ref), SWSRC_LAST)
      << "SWSRC_LAST should round-trip through SwitchRef";
  ref = SwitchRef{SWITCH_TYPE_TRAINER, SWITCH_FLAG_INVERTED, 0};
  EXPECT_EQ(switchRefToSwSrc(ref), -SWSRC_LAST)
      << "-SWSRC_LAST should round-trip through SwitchRef";
}

TEST(BitFieldCapacity, CurveRefValueRoundTrip)
{
  // CurveRef.value is now ValueOrSource (int16_t value + uint8_t isSource + uint8_t srcType).
  // When type=CURVE_REF_CUSTOM, value holds the curve index (1-based).
  // Verify round-trip for curve indices.
  MODEL_RESET();

  MixData* mix = mixAddress(0);
  mix->curve.type = CURVE_REF_CUSTOM;
  mix->curve.value.setNumeric(MAX_CURVES_HARD);
  EXPECT_EQ(mix->curve.value.numericValue(), (int16_t)MAX_CURVES_HARD)
      << "MAX_CURVES_HARD should round-trip through CurveRef.value";
}

TEST(BitFieldCapacity, LimitCurveCanHoldAllCurves)
{
  // LimitData.curve is int8_t, range -1..127 (0=none, 1-based index)
  constexpr int limitCurveMax = 127;

  EXPECT_LE((int)MAX_CURVES_HARD, limitCurveMax)
      << "MAX_CURVES_HARD (" << MAX_CURVES_HARD
      << ") exceeds LimitData.curve:int8_t range (" << limitCurveMax << ")";
}
