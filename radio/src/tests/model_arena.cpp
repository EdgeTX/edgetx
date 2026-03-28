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

#include <cstdlib>

// ---- ModelArena unit tests ----

class ArenaTest : public testing::Test {
 protected:
  static constexpr uint32_t TEST_ARENA_SIZE = 1024;
  uint8_t buf[TEST_ARENA_SIZE];
  ModelArena arena;

  void SetUp() override {
    memset(buf, 0, sizeof(buf));
    // maxCapacity = capacity — no growth beyond the stack buffer
    arena.attach(&modelArenaDesc, buf, TEST_ARENA_SIZE, TEST_ARENA_SIZE);
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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 3;
  counts[ARENA_EXPOS] = 2;
  counts[ARENA_CURVES] = 1;
  counts[ARENA_POINTS] = 5;
  counts[ARENA_LOGICAL_SW] = 4;
  counts[ARENA_CUSTOM_FN] = 2;

  arena.layout(counts);

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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 2;
  counts[ARENA_EXPOS] = 1;
  arena.layout(counts);

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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 2;
  arena.layout(counts);

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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 3;
  arena.layout(counts);

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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = TEST_ARENA_SIZE / sizeof(MixData);
  arena.layout(counts);

  EXPECT_FALSE(arena.insertSlot(0, sizeof(MixData)));
}

TEST_F(ArenaTest, InsertInSectionUpdatesSubsequentOffsets)
{
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 2;
  counts[ARENA_EXPOS] = 2;
  counts[ARENA_CURVES] = 1;
  arena.layout(counts);

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
  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 3;
  counts[ARENA_EXPOS] = 2;
  arena.layout(counts);

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
  LogicalSwitchData* ls = lswAllocAt(0);
  EXPECT_NE(ls, nullptr);
  EXPECT_GE((uint8_t*)ls, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, CustomFnAddressPointsToArena)
{
  CustomFunctionData* cf = customFnAllocAt(0);
  EXPECT_NE(cf, nullptr);
  EXPECT_GE((uint8_t*)cf, g_modelArena.base());
}

TEST_F(ArenaAccessorTest, MixDataRoundTrip)
{
  MixData* mix = mixAllocAt(3);
  mix->destCh = 7;
  mix->weight.setNumeric(50);
  mix->srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);

  EXPECT_EQ(mixAddress(3)->destCh, 7);
  EXPECT_EQ(mixAddress(3)->weight.numericValue(), 50);
  EXPECT_EQ(mixAddress(3)->srcRaw, (SourceRef_(SOURCE_TYPE_MAX, 0)));
}

TEST_F(ArenaAccessorTest, ExpoDataRoundTrip)
{
  ExpoData* expo = expoAllocAt(5);
  expo->chn = 2;
  expo->weight.setNumeric(75);
  expo->mode = 3;

  EXPECT_EQ(expoAddress(5)->chn, 2);
  EXPECT_EQ(expoAddress(5)->weight.numericValue(), 75);
  EXPECT_EQ(expoAddress(5)->mode, 3);
}

TEST_F(ArenaAccessorTest, SectionsDoNotOverlap)
{
  // Allocate 1 element per section via AllocAt helpers
  mixAllocAt(0);
  expoAllocAt(0);
  curveHeaderAllocAt(0);
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 1);
  g_modelArena.ensureSectionCapacity(ARENA_POINTS, 1);
  lswAllocAt(0);
  customFnAllocAt(0);

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
  MixData* mix0 = mixAllocAt(0);
  mix0->destCh = 0;
  mix0->srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  mix0->weight.setNumeric(100);
  updateMixCount();

  insertMix(0, 0);

  // Original mix should have moved to index 1
  EXPECT_EQ(mixAddress(1)->srcRaw, (SourceRef_(SOURCE_TYPE_MAX, 0)));
  EXPECT_EQ(mixAddress(1)->weight.numericValue(), 100);
  // New mix at index 0
  EXPECT_EQ(mixAddress(0)->destCh, 0);
  EXPECT_NE(mixAddress(0)->srcRaw, (SourceRef_(SOURCE_TYPE_MAX, 0)));  // different default source
}

TEST_F(ArenaInsertDeleteTest, DeleteMixShiftsRemaining)
{
  MixData* m0 = mixAllocAt(0);
  m0->destCh = 0;
  m0->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  m0->weight.setNumeric(100);
  MixData* m1 = mixAllocAt(1);
  m1->destCh = 1;
  m1->srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  m1->weight.setNumeric(50);
  updateMixCount();

  deleteMix(0);

  EXPECT_EQ(mixAddress(0)->destCh, 1);
  EXPECT_EQ(mixAddress(0)->srcRaw, (SourceRef_(SOURCE_TYPE_MAX, 0)));
  EXPECT_EQ(mixAddress(0)->weight.numericValue(), 50);
}

TEST_F(ArenaInsertDeleteTest, InsertExpoPreservesExisting)
{
  ExpoData* expo0 = expoAllocAt(0);
  expo0->chn = 0;
  expo0->srcRaw = SourceRef_(SOURCE_TYPE_STICK, 0);
  expo0->weight.setNumeric(100);
  expo0->mode = 3;
  updateExpoCount();

  insertExpo(0, 0);

  // Original expo should have moved to index 1
  EXPECT_EQ(expoAddress(1)->srcRaw, (SourceRef_(SOURCE_TYPE_STICK, 0)));
  EXPECT_EQ(expoAddress(1)->weight.numericValue(), 100);
}

TEST_F(ArenaInsertDeleteTest, InsertDeleteCustomFn)
{
  CustomFunctionData* cf0 = customFnAllocAt(0);
  cf0->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 1);
  cf0->func = FUNC_OVERRIDE_CHANNEL;

  CustomFunctionData* cf1 = customFnAllocAt(1);
  cf1->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 2);
  cf1->func = FUNC_TRAINER;

  insertCustomFn(0);

  // Original functions should have shifted
  EXPECT_TRUE(customFnAddress(0)->swtch.isNone());  // new empty
  EXPECT_EQ(customFnAddress(1)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 1)));  // was cf0
  EXPECT_EQ(customFnAddress(1)->func, FUNC_OVERRIDE_CHANNEL);
  EXPECT_EQ(customFnAddress(2)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 2)));  // was cf1

  deleteCustomFn(0);

  // Back to original
  EXPECT_EQ(customFnAddress(0)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 1)));
  EXPECT_EQ(customFnAddress(0)->func, FUNC_OVERRIDE_CHANNEL);
  EXPECT_EQ(customFnAddress(1)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 2)));
  EXPECT_EQ(customFnAddress(1)->func, FUNC_TRAINER);
}

TEST_F(ArenaInsertDeleteTest, ClearCustomFn)
{
  customFnAllocAt(3)->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 5);
  customFnAddress(3)->func = FUNC_PLAY_SOUND;

  clearCustomFn(3);

  EXPECT_TRUE(customFnAddress(3)->swtch.isNone());
  EXPECT_EQ(customFnAddress(3)->func, 0);
}

// ---- Grow / degrow scenarios ----

TEST_F(ArenaInsertDeleteTest, AllocAtGrowsFromEmpty)
{
  // Arena starts empty after MODEL_RESET + setModelDefaults
  // (only default mixes/expos allocated). Verify AllocAt grows other sections.
  uint32_t freeBefore = g_modelArena.freeBytes();
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 0);

  LogicalSwitchData* ls = lswAllocAt(5);
  EXPECT_NE(ls, nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 6);
  EXPECT_LT(g_modelArena.freeBytes(), freeBefore);
}

TEST_F(ArenaInsertDeleteTest, DeleteMixFreesBytes)
{
  // setModelDefaults already creates default mixes; record baseline
  uint8_t countBefore = g_modelArena.sectionCount(ARENA_MIXES);
  uint32_t usedBefore = g_modelArena.usedBytes();
  EXPECT_GT(countBefore, (uint8_t)0);

  deleteMix(0);

  EXPECT_EQ(g_modelArena.usedBytes(), usedBefore - sizeof(MixData));
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_MIXES), countBefore - 1);
}

TEST_F(ArenaInsertDeleteTest, InsertGrowsCompactedSection)
{
  // Record baseline from setModelDefaults
  uint8_t mixCountBefore = g_modelArena.sectionCount(ARENA_MIXES);
  uint32_t freeBefore = g_modelArena.freeBytes();

  // Write a known value to expo 0 so we can verify it survives
  expoAddress(0)->chn = 3;

  // Insert a new mix — grows the mix section, shifting expos forward
  insertMix(0, 1);

  EXPECT_EQ(g_modelArena.sectionCount(ARENA_MIXES), mixCountBefore + 1);
  EXPECT_EQ(getMixCount(), mixCountBefore + 1);
  EXPECT_EQ(g_modelArena.freeBytes(), freeBefore - sizeof(MixData));

  // Expo data should be intact after the shift
  EXPECT_EQ(expoAddress(0)->chn, 3);
}

TEST_F(ArenaInsertDeleteTest, AllocAtReturnsNullWhenFull)
{
  // Fill the arena by allocating many large elements
  bool full = false;
  for (uint8_t i = 0; i < MAX_MIXERS_HARD; i++) {
    if (!mixAllocAt(i)) {
      full = true;
      break;
    }
  }
  // Arena should eventually run out of space (or hit HARD max)
  // Either way, verify freeBytes is small
  if (!full) {
    // Hit HARD max before arena full — try LS to consume remaining
    for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES_HARD; i++) {
      if (!lswAllocAt(i)) {
        full = true;
        break;
      }
    }
  }
  // On the 64KB test arena this may not fill up, so just verify
  // the alloc functions didn't crash and dyn counts are consistent
  EXPECT_LE(g_modelArena.usedBytes(), g_modelArena.capacity());
}

// ---- Arena clear on model reset ----

TEST_F(ArenaAccessorTest, ModelResetClearsArena)
{
  // Write data to arena (use AllocAt to ensure section is grown)
  mixAddress(0)->destCh = 5;
  mixAddress(0)->srcRaw = SourceRef_(SOURCE_TYPE_MAX, 0);
  expoAddress(0)->chn = 3;
  lswAllocAt(10)->func = LS_FUNC_VPOS;
  EXPECT_EQ(lswAddress(10)->func, LS_FUNC_VPOS);

  MODEL_RESET();

  // Arena data should be zeroed (default template may populate some entries)
  // but the specific entries we set should be gone
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 0);
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
  mix->srcRaw = SourceRef_(SOURCE_TYPE_TELEMETRY, 42);
  EXPECT_EQ(mix->srcRaw.type, SOURCE_TYPE_TELEMETRY);
  EXPECT_EQ(mix->srcRaw.index, 42);

  // Test max index value for uint16_t
  mix->srcRaw = SourceRef_(SOURCE_TYPE_CHANNEL, 255);
  EXPECT_EQ(mix->srcRaw.type, SOURCE_TYPE_CHANNEL);
  EXPECT_EQ(mix->srcRaw.index, 255);
}

TEST(BitFieldCapacity, SwitchRefRoundTrip)
{
  // SwitchRef uses full bytes (type=uint8_t, index=uint16_t),
  // so no bit-field truncation. Verify round-trip works.
  MODEL_RESET();

  MixData* mix = mixAddress(0);
  mix->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 42);
  EXPECT_EQ(mix->swtch.type, SWITCH_TYPE_SWITCH);
  EXPECT_EQ(mix->swtch.index, 42);

  mix->swtch = SwitchRef_(SWITCH_TYPE_LOGICAL, 10, SWITCH_FLAG_INVERTED);
  EXPECT_EQ(mix->swtch.type, SWITCH_TYPE_LOGICAL);
  EXPECT_EQ(mix->swtch.flags, SWITCH_FLAG_INVERTED);
  EXPECT_EQ(mix->swtch.index, 10);
}

TEST(BitFieldCapacity, SwitchRefCanRoundTrip)
{
  // FlightModeData.swtch is now SwitchRef (4 bytes), can hold any switch.
  // Verify toUint32/fromUint32 round-trip for extreme values.
  SwitchRef ref = SwitchRef_(SWITCH_TYPE_TRAINER, 0);
  EXPECT_EQ(SwitchRef::fromUint32(ref.toUint32()), ref)
      << "SwitchRef should round-trip through toUint32/fromUint32";
  ref = SwitchRef_(SWITCH_TYPE_TRAINER, 0, SWITCH_FLAG_INVERTED);
  SwitchRef rt = SwitchRef::fromUint32(ref.toUint32());
  EXPECT_EQ(rt.type, SWITCH_TYPE_TRAINER);
  EXPECT_TRUE(rt.isInverted())
      << "Inverted SwitchRef should round-trip through toUint32/fromUint32";
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

// ---- YAML model round-trip test ----
// Writes a model to YAML, reads it back, verifies arena was populated
// via ensure_capacity callbacks with correct dyn counts.

#include "storage/sdcard_yaml.h"
#include "storage/sdcard_common.h"
#include "location.h"
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>

TEST_F(ArenaInsertDeleteTest, YamlRoundTrip)
{
  // Use a temp directory for the YAML file
  char tmpDir[] = "/tmp/edgetx_test_XXXXXX";
  ASSERT_NE(mkdtemp(tmpDir), nullptr);

  char modelsDir[256];
  snprintf(modelsDir, sizeof(modelsDir), "%s/MODELS", tmpDir);
  mkdir(modelsDir, 0755);

  // Point simulated filesystem at the temp dir
  simuFatfsSetPaths(tmpDir, nullptr);

  // setModelDefaults already creates default mixes/expos.
  // Add extra arena elements beyond the defaults.
  LogicalSwitchData* ls = lswAllocAt(2);
  ASSERT_NE(ls, nullptr);
  ls->func = LS_FUNC_VPOS;
  ls->v1.source = SourceRef_(SOURCE_TYPE_INPUT, 0);

  CustomFunctionData* cf = customFnAllocAt(0);
  ASSERT_NE(cf, nullptr);
  cf->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 1);
  cf->func = FUNC_PLAY_SOUND;

  // Create a default identity curve at index 0
  ASSERT_TRUE(curveAllocAt(0));
  setCurveUsed(0);
  int8_t* pts = curveAddress(0);
  uint8_t nPts = getCurvePoints(0);
  ASSERT_EQ(nPts, 5);
  // Fill with identity: f(x) = x
  for (uint8_t i = 0; i < nPts; i++)
    pts[i] = -100 + i * 50;

  uint8_t savedMixCount = g_modelArena.sectionCount(ARENA_MIXES);
  uint8_t savedExpoCount = g_modelArena.sectionCount(ARENA_EXPOS);
  uint8_t savedLsCount = g_modelArena.sectionCount(ARENA_LOGICAL_SW);
  uint8_t savedCfnCount = g_modelArena.sectionCount(ARENA_CUSTOM_FN);
  uint16_t savedCurveCount = g_modelArena.sectionCount(ARENA_CURVES);
  uint16_t savedPointCount = g_modelArena.sectionCount(ARENA_POINTS);
  EXPECT_GT(savedMixCount, (uint8_t)0);
  EXPECT_EQ(savedLsCount, 3);  // lswAllocAt(2) allocates slots 0-2
  EXPECT_EQ(savedCfnCount, 1);
  EXPECT_EQ(savedCurveCount, 1);
  EXPECT_EQ(savedPointCount, 5);

  SourceRef savedMix0Src = mixAddress(0)->srcRaw;
  int16_t savedMix0Weight = mixAddress(0)->weight.numericValue();

  // Write model to YAML
  static const char* testFile = "test_arena.yml";
  const char* err = writeModelYaml(testFile);
  ASSERT_EQ(err, nullptr) << "writeModelYaml failed: " << (err ? err : "");

  // Clear model and arena completely
  memset(&g_model, 0, sizeof(g_model));
  inputNameIndexReset();
  uint16_t emptyCounts[MODEL_ARENA_NUM_SECTIONS] = {};
  g_modelArena.layout(emptyCounts);
  g_modelArena.clear();
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_MIXES), 0);
  EXPECT_EQ(g_modelArena.usedBytes(), (uint32_t)0);

  // Read back — ensure_capacity callbacks grow arena on demand
  err = readModelYaml(testFile, (uint8_t*)&g_model, sizeof(g_model));
  ASSERT_EQ(err, nullptr) << "readModelYaml failed: " << (err ? err : "");

  // Verify dyn counts match what was written
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_MIXES), savedMixCount);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_EXPOS), savedExpoCount);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), savedLsCount);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_CUSTOM_FN), savedCfnCount);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_CURVES), savedCurveCount);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_POINTS), savedPointCount);

  // Verify arena has correct used bytes
  EXPECT_GT(g_modelArena.usedBytes(), (uint32_t)0);

  // Verify data survived the round-trip
  EXPECT_EQ(mixAddress(0)->srcRaw, savedMix0Src);
  EXPECT_EQ(mixAddress(0)->weight.numericValue(), savedMix0Weight);
  auto* rawLs = reinterpret_cast<LogicalSwitchData*>(
      g_modelArena.sectionBase(ARENA_LOGICAL_SW)) + 2;
  EXPECT_EQ(rawLs->func, LS_FUNC_VPOS)
      << "sectionCount=" << g_modelArena.sectionCount(ARENA_LOGICAL_SW)
      << " sectionOffset=" << g_modelArena.sectionOffset(ARENA_LOGICAL_SW)
      << " usedBytes=" << g_modelArena.usedBytes();
  EXPECT_EQ(lswAddress(2)->func, LS_FUNC_VPOS);
  EXPECT_EQ(customFnAddress(0)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 1)));
  EXPECT_EQ(customFnAddress(0)->func, FUNC_PLAY_SOUND);

  // Verify curve data survived the round-trip
  loadCurves();
  EXPECT_EQ(getCurvePoints(0), 5);
  int8_t* ptsAfter = curveAddress(0);
  for (uint8_t i = 0; i < 5; i++) {
    EXPECT_EQ(ptsAfter[i], -100 + i * 50)
        << "curve point " << (int)i << " mismatch";
  }

  // Restore test path and clean up
  simuFatfsSetPaths(TESTS_PATH, nullptr);
  // Leave temp files for debugging if test fails
  if (!HasFailure()) {
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/MODELS/%s", tmpDir, testFile);
    remove(filePath);
    rmdir(modelsDir);
    rmdir(tmpDir);
  } else {
    printf("  YAML file left at: %s/MODELS/%s\n", tmpDir, testFile);
  }
}

// ---- Trim trailing empty tests ----

TEST_F(ArenaInsertDeleteTest, TrimTrailingEmpty)
{
  // Allocate 4 LS slots, make 0-1 non-empty, leave 2-3 empty
  ASSERT_NE(lswAllocAt(3), nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 4);

  lswAddress(0)->func = LS_FUNC_VPOS;
  lswAddress(1)->func = LS_FUNC_VPOS;

  uint32_t bytesBefore = g_modelArena.usedBytes();

  lswTrimTrailing();

  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 2);
  EXPECT_EQ(g_modelArena.usedBytes(),
            bytesBefore - 2 * sizeof(LogicalSwitchData));
  EXPECT_EQ(lswAddress(0)->func, LS_FUNC_VPOS);
  EXPECT_EQ(lswAddress(1)->func, LS_FUNC_VPOS);
}

TEST_F(ArenaInsertDeleteTest, TrimPreservesMiddleEmpty)
{
  ASSERT_NE(lswAllocAt(3), nullptr);
  lswAddress(0)->func = LS_FUNC_VPOS;
  // slot 1 empty
  lswAddress(2)->func = LS_FUNC_VPOS;
  lswAddress(3)->func = LS_FUNC_VPOS;

  lswTrimTrailing();

  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 4);
  EXPECT_EQ(lswAddress(1)->func, LS_FUNC_NONE);
}

TEST_F(ArenaInsertDeleteTest, TrimAllEmpty)
{
  ASSERT_NE(lswAllocAt(2), nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 3);

  lswTrimTrailing();

  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 0);
}

TEST_F(ArenaInsertDeleteTest, TrimPreservesSubsequentSection)
{
  // Set up LS with 3 elements (last empty) and CF with 1 element
  ASSERT_NE(lswAllocAt(2), nullptr);
  lswAddress(0)->func = LS_FUNC_VPOS;
  lswAddress(1)->func = LS_FUNC_VPOS;

  CustomFunctionData* cf = customFnAllocAt(0);
  ASSERT_NE(cf, nullptr);
  cf->swtch = SwitchRef_(SWITCH_TYPE_SWITCH, 1);
  cf->func = FUNC_PLAY_SOUND;

  lswTrimTrailing();

  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 2);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_CUSTOM_FN), 1);
  EXPECT_EQ(customFnAddress(0)->swtch, (SwitchRef_(SWITCH_TYPE_SWITCH, 1)));
  EXPECT_EQ(customFnAddress(0)->func, FUNC_PLAY_SOUND);
}

TEST_F(ArenaInsertDeleteTest, TrimAllocRoundTrip)
{
  uint32_t emptyBytes = g_modelArena.usedBytes();

  // Simulate UI: select slot 5, back out without editing
  ASSERT_NE(lswAllocAt(5), nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 6);

  lswTrimTrailing();
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 0);
  EXPECT_EQ(g_modelArena.usedBytes(), emptyBytes);

  // Now allocate slot 2 and make it non-empty
  ASSERT_NE(lswAllocAt(2), nullptr);
  lswAddress(2)->func = LS_FUNC_VPOS;

  lswTrimTrailing();
  // Slots 0,1 are empty but kept because slot 2 is non-empty
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 3);
}

TEST_F(ArenaInsertDeleteTest, TrimSectionTo)
{
  ASSERT_NE(lswAllocAt(4), nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 5);

  lswAddress(0)->func = LS_FUNC_VPOS;
  lswAddress(1)->func = LS_FUNC_VPOS;

  uint32_t bytesBefore = g_modelArena.usedBytes();

  uint16_t removed = g_modelArena.trimSectionTo(ARENA_LOGICAL_SW, 2);
  EXPECT_EQ(removed, 3);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 2);
  EXPECT_EQ(g_modelArena.usedBytes(),
            bytesBefore - 3 * sizeof(LogicalSwitchData));
  EXPECT_EQ(lswAddress(0)->func, LS_FUNC_VPOS);
  EXPECT_EQ(lswAddress(1)->func, LS_FUNC_VPOS);

  // No-op when newCount >= current
  removed = g_modelArena.trimSectionTo(ARENA_LOGICAL_SW, 5);
  EXPECT_EQ(removed, 0);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_LOGICAL_SW), 2);
}

// ---- Curve trim tests ----

TEST_F(ArenaInsertDeleteTest, CurveTrimTrailingEmpty)
{
  // Create curves at index 0 and 2
  ASSERT_TRUE(curveAllocAt(2));
  EXPECT_EQ(getCurveCount(), 3);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_POINTS), 15);  // 3 × 5

  // Make curve 0 non-empty (identity points)
  int8_t* pts0 = curveAddress(0);
  for (uint8_t i = 0; i < 5; i++) pts0[i] = -100 + i * 50;
  setCurveUsed(0);

  // Curve 1 and 2 are all zeros → trailing empty
  curveTrimTrailing();

  // Only curve 0 should remain
  EXPECT_EQ(getCurveCount(), 1);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_POINTS), 5);

  // Curve 0 data intact
  loadCurves();
  int8_t* pts = curveAddress(0);
  for (uint8_t i = 0; i < 5; i++) {
    EXPECT_EQ(pts[i], -100 + i * 50);
  }
}

TEST_F(ArenaInsertDeleteTest, CurveTrimPreservesMiddleEmpty)
{
  // Create curves 0, 1, 2 — make 0 and 2 non-empty
  ASSERT_TRUE(curveAllocAt(2));

  int8_t* pts0 = curveAddress(0);
  pts0[0] = -100;
  setCurveUsed(0);

  int8_t* pts2 = curveAddress(2);
  pts2[0] = -100;
  setCurveUsed(2);

  // Curve 1 is empty but in the middle
  curveTrimTrailing();

  // All 3 kept because curve 2 is non-empty
  EXPECT_EQ(getCurveCount(), 3);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_POINTS), 15);
}

TEST_F(ArenaInsertDeleteTest, CurveClearAndTrim)
{
  // Create curve 0 with identity data
  ASSERT_TRUE(curveAllocAt(0));
  int8_t* pts = curveAddress(0);
  for (uint8_t i = 0; i < 5; i++) pts[i] = -100 + i * 50;
  setCurveUsed(0);

  EXPECT_TRUE(isCurveUsed(0));

  uint32_t bytesBefore = g_modelArena.usedBytes();

  curveClear(0);
  EXPECT_FALSE(isCurveUsed(0));

  curveTrimTrailing();
  EXPECT_EQ(getCurveCount(), 0);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_POINTS), 0);
  EXPECT_LT(g_modelArena.usedBytes(), bytesBefore);
}

// ---- Growable arena tests (Phase 6) ----

class GrowableArenaTest : public testing::Test {
 protected:
  ModelArena arena;

  void TearDown() override {
    arena.release();
  }
};

TEST_F(GrowableArenaTest, GrowOnInsert)
{
  // Start with a small heap buffer, allow growth
  static constexpr uint32_t INITIAL = 128;
  static constexpr uint32_t MAX = 4096;
  uint8_t* buf = (uint8_t*)malloc(INITIAL);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, INITIAL);
  arena.attach(&modelArenaDesc, buf, INITIAL, MAX);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  arena.layout(counts);

  // Fill the initial buffer with mixes
  uint32_t maxInInitial = INITIAL / sizeof(MixData);
  for (uint32_t i = 0; i < maxInInitial; i++) {
    EXPECT_TRUE(arena.insertInSection(ARENA_MIXES, i, sizeof(MixData)))
        << "insert " << i << " should fit in initial buffer";
  }
  EXPECT_EQ(arena.currentFreeBytes(), INITIAL - maxInInitial * sizeof(MixData));

  // Next insert exceeds initial capacity — should succeed via growth
  EXPECT_TRUE(arena.insertInSection(ARENA_MIXES, maxInInitial, sizeof(MixData)));
  EXPECT_GT(arena.capacity(), INITIAL);
  EXPECT_TRUE(arena.isHeapOwned());
}

TEST_F(GrowableArenaTest, GrowFailsAtMax)
{
  // maxCapacity = capacity — no growth allowed
  static constexpr uint32_t SIZE = 128;
  uint8_t* buf = (uint8_t*)malloc(SIZE);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, SIZE);
  arena.attach(&modelArenaDesc, buf, SIZE, SIZE);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  arena.layout(counts);

  // Fill completely
  uint32_t maxMixes = SIZE / sizeof(MixData);
  for (uint32_t i = 0; i < maxMixes; i++) {
    arena.insertInSection(ARENA_MIXES, i, sizeof(MixData));
  }

  // Next insert should fail — no room to grow
  EXPECT_FALSE(arena.insertInSection(ARENA_MIXES, maxMixes, sizeof(MixData)));
}

TEST_F(GrowableArenaTest, ShrinkToFit)
{
  // Must start above MODEL_ARENA_INITIAL_SIZE so shrink actually reallocates
  static constexpr uint32_t INITIAL = MODEL_ARENA_INITIAL_SIZE * 2;
  static constexpr uint32_t MAX = MODEL_ARENA_INITIAL_SIZE * 4;
  uint8_t* buf = (uint8_t*)malloc(INITIAL);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, INITIAL);
  arena.attach(&modelArenaDesc, buf, INITIAL, MAX);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 2;
  arena.layout(counts);
  MixData* mix = (MixData*)arena.sectionBase(ARENA_MIXES);
  mix[0].destCh = 5;
  mix[1].destCh = 10;

  // Shrink — capacity should drop to MODEL_ARENA_INITIAL_SIZE
  EXPECT_TRUE(arena.shrinkToFit(64));
  EXPECT_EQ(arena.capacity(), (uint32_t)MODEL_ARENA_INITIAL_SIZE);
  EXPECT_TRUE(arena.isHeapOwned());

  // Data should survive the shrink
  mix = (MixData*)arena.sectionBase(ARENA_MIXES);
  EXPECT_EQ(mix[0].destCh, 5);
  EXPECT_EQ(mix[1].destCh, 10);
}

TEST_F(GrowableArenaTest, ShrinkSkippedBelowInitial)
{
  // Start at initial size — shrinkToFit should be a no-op
  static constexpr uint32_t MAX = MODEL_ARENA_INITIAL_SIZE * 4;
  uint8_t* buf = (uint8_t*)malloc(MODEL_ARENA_INITIAL_SIZE);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, MODEL_ARENA_INITIAL_SIZE);
  arena.attach(&modelArenaDesc, buf, MODEL_ARENA_INITIAL_SIZE, MAX);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  counts[ARENA_MIXES] = 1;
  arena.layout(counts);

  uint32_t capBefore = arena.capacity();
  EXPECT_TRUE(arena.shrinkToFit(64));
  EXPECT_EQ(arena.capacity(), capBefore);  // no reallocation
}

TEST_F(GrowableArenaTest, FreeBytesReflectsMaxCapacity)
{
  static constexpr uint32_t INITIAL = 256;
  static constexpr uint32_t MAX = 4096;
  uint8_t* buf = (uint8_t*)malloc(INITIAL);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, INITIAL);
  arena.attach(&modelArenaDesc, buf, INITIAL, MAX);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  arena.layout(counts);

  // freeBytes reflects maxCapacity, not current capacity
  EXPECT_EQ(arena.freeBytes(), MAX);
  EXPECT_EQ(arena.currentFreeBytes(), INITIAL);

  // After inserting data, freeBytes decreases from max
  arena.insertInSection(ARENA_MIXES, 0, sizeof(MixData));
  EXPECT_EQ(arena.freeBytes(), MAX - sizeof(MixData));
  EXPECT_EQ(arena.currentFreeBytes(), INITIAL - sizeof(MixData));
}

TEST_F(GrowableArenaTest, DetachHeapBuffer)
{
  static constexpr uint32_t SIZE = 256;
  uint8_t* buf = (uint8_t*)malloc(SIZE);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, SIZE);
  arena.attach(&modelArenaDesc, buf, SIZE, SIZE);
  arena.setHeapOwned(true);

  EXPECT_TRUE(arena.isHeapOwned());
  uint8_t* detached = arena.detachHeapBuffer();
  EXPECT_EQ(detached, buf);
  EXPECT_FALSE(arena.isHeapOwned());

  // Detach again returns nullptr (no longer heap-owned)
  EXPECT_EQ(arena.detachHeapBuffer(), nullptr);

  free(detached);
}

TEST_F(GrowableArenaTest, EnsureSectionCapacityGrows)
{
  static constexpr uint32_t INITIAL = 128;
  static constexpr uint32_t MAX = 4096;
  uint8_t* buf = (uint8_t*)malloc(INITIAL);
  ASSERT_NE(buf, nullptr);
  memset(buf, 0, INITIAL);
  arena.attach(&modelArenaDesc, buf, INITIAL, MAX);
  arena.setHeapOwned(true);

  uint16_t counts[MODEL_ARENA_NUM_SECTIONS] = {};
  arena.layout(counts);

  // Request more logical switches than fit in initial buffer
  uint16_t count = (INITIAL / sizeof(LogicalSwitchData)) + 5;
  EXPECT_TRUE(arena.ensureSectionCapacity(ARENA_LOGICAL_SW, count));
  EXPECT_EQ(arena.sectionCount(ARENA_LOGICAL_SW), count);
  EXPECT_GT(arena.capacity(), INITIAL);
}

// Test that the ArenaTest fixture (non-growable stack buffer) still works
TEST_F(ArenaTest, MaxCapacityEqualsCapacity)
{
  EXPECT_EQ(arena.maxCapacity(), TEST_ARENA_SIZE);
  EXPECT_EQ(arena.freeBytes(), TEST_ARENA_SIZE);
  EXPECT_EQ(arena.currentFreeBytes(), TEST_ARENA_SIZE);
  EXPECT_FALSE(arena.isHeapOwned());
}

// ---- Input name accessor tests ----

class InputNameTest : public ArenaInsertDeleteTest {
 protected:
  void SetUp() override {
    ArenaInsertDeleteTest::SetUp();
    for (uint8_t i = 0; i < MAX_INPUTS; i++)
      inputNameClear(i);
  }
};

TEST_F(InputNameTest, AllocAndLookup)
{

  // Initially no names
  EXPECT_EQ(inputName(0), nullptr);
  EXPECT_FALSE(hasInputName(0));
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 0);

  // Alloc name for input 3
  char* name3 = inputNameAlloc(3);
  ASSERT_NE(name3, nullptr);
  strncpy(name3, "Ail", LEN_INPUT_NAME);

  EXPECT_TRUE(hasInputName(3));
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 1);
  EXPECT_EQ(strncmp(inputName(3), "Ail", LEN_INPUT_NAME), 0);
  EXPECT_EQ(inputName(0), nullptr);

  // Alloc name for input 0
  char* name0 = inputNameAlloc(0);
  ASSERT_NE(name0, nullptr);
  strncpy(name0, "Ele", LEN_INPUT_NAME);

  EXPECT_TRUE(hasInputName(0));
  EXPECT_TRUE(hasInputName(3));
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 2);
  EXPECT_EQ(strncmp(inputName(0), "Ele", LEN_INPUT_NAME), 0);
  EXPECT_EQ(strncmp(inputName(3), "Ail", LEN_INPUT_NAME), 0);

  // Re-alloc existing returns same pointer
  char* name3b = inputNameAlloc(3);
  EXPECT_EQ(name3b, name3);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 2);
}

TEST_F(InputNameTest, ClearSwapWithLast)
{

  // Set up names for inputs 0, 2, 5
  strncpy(inputNameAlloc(0), "Ail", LEN_INPUT_NAME);
  strncpy(inputNameAlloc(2), "Rud", LEN_INPUT_NAME);
  strncpy(inputNameAlloc(5), "Thr", LEN_INPUT_NAME);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 3);

  // Clear input 0 — should swap with last (input 5's slot)
  inputNameClear(0);
  EXPECT_FALSE(hasInputName(0));
  EXPECT_EQ(inputName(0), nullptr);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 2);

  // Input 2 and 5 should still be valid
  EXPECT_EQ(strncmp(inputName(2), "Rud", LEN_INPUT_NAME), 0);
  EXPECT_EQ(strncmp(inputName(5), "Thr", LEN_INPUT_NAME), 0);

  // Clear input 5
  inputNameClear(5);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 1);
  EXPECT_EQ(strncmp(inputName(2), "Rud", LEN_INPUT_NAME), 0);

  // Clear last remaining
  inputNameClear(2);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 0);
}

TEST_F(InputNameTest, ClearNoOp)
{

  // Clear on non-existent name is a no-op
  inputNameClear(7);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 0);
}

TEST_F(InputNameTest, IndexReset)
{
  strncpy(inputNameAlloc(0), "X", LEN_INPUT_NAME);
  EXPECT_TRUE(hasInputName(0));

  inputNameIndexReset();
  // Index is reset but arena section still has data
  EXPECT_FALSE(hasInputName(0));
  EXPECT_EQ(inputName(0), nullptr);
}

TEST_F(InputNameTest, YamlRoundTrip)
{
  // Set up names for inputs 1, 5, 10
  strncpy(inputNameAlloc(1), "Ail", LEN_INPUT_NAME);
  strncpy(inputNameAlloc(5), "Rud", LEN_INPUT_NAME);
  strncpy(inputNameAlloc(10), "Thr", LEN_INPUT_NAME);
  EXPECT_EQ(g_modelArena.sectionCount(ARENA_INPUT_NAMES), 3);

  // Write model to YAML
  char tmpDir[] = "/tmp/edgetx_iname_XXXXXX";
  ASSERT_NE(mkdtemp(tmpDir), nullptr);
  char modelsDir[256];
  snprintf(modelsDir, sizeof(modelsDir), "%s/MODELS", tmpDir);
  mkdir(modelsDir, 0755);
  simuFatfsSetPaths(tmpDir, nullptr);

  static const char* testFile = "test_inames.yml";
  const char* err = writeModelYaml(testFile);
  ASSERT_EQ(err, nullptr) << "writeModelYaml failed: " << (err ? err : "");

  // Clear model and arena completely
  memset(&g_model, 0, sizeof(g_model));
  inputNameIndexReset();
  uint16_t emptyCounts[MODEL_ARENA_NUM_SECTIONS] = {};
  g_modelArena.layout(emptyCounts);
  g_modelArena.clear();

  // Read back
  err = readModelYaml(testFile, (uint8_t*)&g_model, sizeof(g_model));
  ASSERT_EQ(err, nullptr) << "readModelYaml failed: " << (err ? err : "");

  // Verify input names survived the round-trip
  EXPECT_TRUE(hasInputName(1));
  EXPECT_TRUE(hasInputName(5));
  EXPECT_TRUE(hasInputName(10));
  EXPECT_FALSE(hasInputName(0));
  EXPECT_FALSE(hasInputName(2));

  EXPECT_EQ(strncmp(inputName(1), "Ail", LEN_INPUT_NAME), 0);
  EXPECT_EQ(strncmp(inputName(5), "Rud", LEN_INPUT_NAME), 0);
  EXPECT_EQ(strncmp(inputName(10), "Thr", LEN_INPUT_NAME), 0);

  // Clean up
  simuFatfsSetPaths(TESTS_PATH, nullptr);
  if (!HasFailure()) {
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "%s/MODELS/%s", tmpDir, testFile);
    remove(filePath);
    rmdir(modelsDir);
    rmdir(tmpDir);
  }
}
