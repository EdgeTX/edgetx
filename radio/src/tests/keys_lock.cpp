/*
 * Copyright (C) EdgeTX
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

#if defined(KEYS_LOCK_KEY1) && defined(KEYS_LOCK_KEY2)

// keys.cpp local; must match.
static constexpr int LOCK_POLL_DELAY = 32;
// A few extra polling cycles past the threshold to be safe against off-by-ones.
static constexpr int LOCK_POLL_HOLD  = LOCK_POLL_DELAY + 4;

static void releaseAllKeys()
{
  for (int i = 0; i < MAX_KEYS; i++) simuSetKey(i, false);
  // Drain any pending events from prior tests.
  while (getEvent()) {}
  // Two polling cycles to let the Key debouncers see "released".
  keysPollingCycle();
  keysPollingCycle();
  while (getEvent()) {}
  (void)consumeKeysLockToggleEvent();
}

static void holdComboFor(int cycles)
{
  simuSetKey(KEYS_LOCK_KEY1, true);
  simuSetKey(KEYS_LOCK_KEY2, true);
  for (int i = 0; i < cycles; i++) keysPollingCycle();
}

class KeyLockTest : public EdgeTxTest
{
 protected:
  void SetUp() override
  {
    EdgeTxTest::SetUp();
    g_eeGeneral.keyLockEnabled = 1;
    releaseAllKeys();
  }
};

TEST_F(KeyLockTest, ComboTogglesLock)
{
  EXPECT_FALSE(areKeysLocked());

  holdComboFor(LOCK_POLL_HOLD);
  EXPECT_TRUE(areKeysLocked());
  EXPECT_TRUE(consumeKeysLockToggleEvent());
  EXPECT_FALSE(consumeKeysLockToggleEvent());  // consumed once

  releaseAllKeys();

  holdComboFor(LOCK_POLL_HOLD);
  EXPECT_FALSE(areKeysLocked());
  EXPECT_TRUE(consumeKeysLockToggleEvent());
}

TEST_F(KeyLockTest, BriefComboDoesNotToggle)
{
  // Hold for fewer cycles than the long-press threshold.
  holdComboFor(LOCK_POLL_DELAY - 2);
  EXPECT_FALSE(areKeysLocked());
  EXPECT_FALSE(consumeKeysLockToggleEvent());
}

TEST_F(KeyLockTest, DisabledFlagPreventsLocking)
{
  g_eeGeneral.keyLockEnabled = 0;
  holdComboFor(LOCK_POLL_HOLD);
  EXPECT_FALSE(areKeysLocked());
  EXPECT_FALSE(consumeKeysLockToggleEvent());
}

TEST_F(KeyLockTest, DisablingFlagWhileLockedReleasesLock)
{
  holdComboFor(LOCK_POLL_HOLD);
  ASSERT_TRUE(areKeysLocked());
  (void)consumeKeysLockToggleEvent();
  releaseAllKeys();

  // Turning the feature off must never leave the radio stuck locked.
  g_eeGeneral.keyLockEnabled = 0;
  keysPollingCycle();
  EXPECT_FALSE(areKeysLocked());
}

// Find a key not part of the lock combo to use for input-suppression tests.
static int pickProbeKey()
{
  for (int i = 0; i < MAX_KEYS; i++) {
    if (i == KEYS_LOCK_KEY1 || i == KEYS_LOCK_KEY2) continue;
    if (keyIsSupported((EnumKeys)i)) return i;
  }
  return -1;
}

TEST_F(KeyLockTest, LockedKeyPressIsSuppressed)
{
  int probe = pickProbeKey();
  if (probe < 0) GTEST_SKIP() << "no spare key to probe with";

  // Sanity check: while unlocked, a key press emits an event.
  simuSetKey(probe, true);
  keysPollingCycle();
  simuSetKey(probe, false);
  keysPollingCycle();
  bool sawEvent = false;
  while (event_t e = getEvent()) { (void)e; sawEvent = true; }
  EXPECT_TRUE(sawEvent);

  // Lock the keys.
  holdComboFor(LOCK_POLL_HOLD);
  ASSERT_TRUE(areKeysLocked());
  (void)consumeKeysLockToggleEvent();
  releaseAllKeys();

  // Now key presses must not push any events.
  simuSetKey(probe, true);
  keysPollingCycle();
  simuSetKey(probe, false);
  keysPollingCycle();
  EXPECT_EQ(getEvent(), (event_t)0);

  // A key press while locked re-arms the toggle-event notification so the
  // UI can re-show the "Keys locked" toast.
  EXPECT_TRUE(consumeKeysLockToggleEvent());
}

#endif  // KEYS_LOCK_KEY1 && KEYS_LOCK_KEY2
