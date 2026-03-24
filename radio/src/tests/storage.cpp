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

// Storage fault-injection tests.
// These tests exercise the radio-settings write path (writeGeneralSettings)
// and the storageCheck retry loop by injecting FatFS errors via the simulator's
// simuSetStorageError() / simuClearStorageError() API.
//
// Prerequisites: radio/src/tests/RADIO/ directory must exist (it is tracked in
// git with a .gitignore that suppresses its contents).

#include "gtests.h"
#include "storage/sdcard_common.h"
#include "sdcard.h"
#include "ff.h"

// ── helpers ────────────────────────────────────────────────────────────────

// Remove any leftover temp / live settings files from a previous run.
static void cleanSettingsFiles()
{
  f_unlink(RADIO_SETTINGS_YAML_PATH);
  f_unlink(RADIO_SETTINGS_TMPFILE_YAML_PATH);
  f_unlink(RADIO_SETTINGS_ERRORFILE_YAML_PATH);
  // f_unlink returns an error if the file doesn't exist, which is fine here.
}

// ── fixture ────────────────────────────────────────────────────────────────

class StorageTest : public EdgeTxTest
{
protected:
  void SetUp() override
  {
    EdgeTxTest::SetUp();
    simuClearStorageError();
    cleanSettingsFiles();
    // Reset the storageCheck retry counters by clearing the dirty mask and
    // making one successful write (which resets the internal static counters).
    storageDirtyMsk = 0;
  }

  void TearDown() override
  {
    simuClearStorageError();
    cleanSettingsFiles();
  }
};

// ── tests ──────────────────────────────────────────────────────────────────

// Baseline: a clean write succeeds and leaves no error string.
TEST_F(StorageTest, writeGeneralSettingsSucceeds)
{
  const char* err = writeGeneralSettings();
  EXPECT_EQ(nullptr, err) << "writeGeneralSettings should succeed: " << (err ? err : "");
}

// Injecting a write error makes writeGeneralSettings return a non-null error.
TEST_F(StorageTest, writeFaultFailsGeneralSettings)
{
  simuSetStorageError(true, SIMU_STORAGE_OP_WRITE);
  const char* err = writeGeneralSettings();
  EXPECT_NE(nullptr, err) << "Expected write error to propagate";
}

// Injecting a rename error (after the file write succeeds) also fails.
TEST_F(StorageTest, renameFaultFailsGeneralSettings)
{
  simuSetStorageError(true, SIMU_STORAGE_OP_RENAME);
  const char* err = writeGeneralSettings();
  EXPECT_NE(nullptr, err) << "Expected rename error to propagate";
}

// After clearing the fault, writes succeed again.
TEST_F(StorageTest, clearFaultRestoresWriteSuccess)
{
  simuSetStorageError(true, SIMU_STORAGE_OP_WRITE);
  EXPECT_NE(nullptr, writeGeneralSettings());

  simuClearStorageError();
  cleanSettingsFiles();

  const char* err = writeGeneralSettings();
  EXPECT_EQ(nullptr, err) << "Write should succeed after fault is cleared";
}

// storageCheck leaves the dirty bit set when the write fails, so the
// firmware will retry on the next call.
TEST_F(StorageTest, storageCheckRetainsDirectyBitOnWriteError)
{
  storageDirtyMsk |= EE_GENERAL;
  simuSetStorageError(true, SIMU_STORAGE_OP_WRITE);

  storageCheck(true);

  EXPECT_TRUE(storageDirtyMsk & EE_GENERAL)
      << "EE_GENERAL dirty bit should remain set after a failed write";

  // Clean up: clear fault and let storageCheck succeed so the static retry
  // counter is reset to zero for subsequent tests.
  simuClearStorageError();
  storageCheck(true);
}

// storageCheck clears the dirty bit once the write succeeds.
TEST_F(StorageTest, storageCheckClearsDirtyBitOnSuccess)
{
  storageDirtyMsk |= EE_GENERAL;

  storageCheck(true);

  EXPECT_FALSE(storageDirtyMsk & EE_GENERAL)
      << "EE_GENERAL dirty bit should be cleared after a successful write";
}

// Writing radio settings then reading them back must preserve a known field
// value — verifying the full write→rename→read YAML round trip.
TEST_F(StorageTest, writeAndReadBackGeneralSettings)
{
  // Use stickMode: a simple 2-bit field that is written to YAML and is not
  // overwritten by postRadioSettingsLoad() or adcCalibDefaults().
  // Valid values are 0-3; pick one that differs from the generalDefault value.
  const uint8_t sentinel = (g_eeGeneral.stickMode == 1) ? 2 : 1;
  g_eeGeneral.stickMode = sentinel;

  ASSERT_EQ(nullptr, writeGeneralSettings()) << "Write should succeed";

  // Wipe the in-memory struct so we can prove the value came from disk.
  memset(&g_eeGeneral, 0, sizeof(g_eeGeneral));

  const char* err = loadRadioSettings();
  ASSERT_EQ(nullptr, err) << "Load should succeed: " << (err ? err : "");

  EXPECT_EQ(sentinel, g_eeGeneral.stickMode)
      << "stickMode should survive a write→read round trip";
}

// A failed f_unlink should not prevent a successful write: writeGeneralSettings
// ignores the unlink return value and relies on f_rename to replace the file.
// On the host filesystem, fs::rename atomically replaces the destination, so
// even if the unlink "fails" (via our injector) the rename still succeeds.
TEST_F(StorageTest, unlinkFaultDoesNotPreventWrite)
{
  // First write creates radio.yml so the second write's unlink has something
  // to fail on.
  ASSERT_EQ(nullptr, writeGeneralSettings());

  simuSetStorageError(true, SIMU_STORAGE_OP_UNLINK);
  const char* err = writeGeneralSettings();
  EXPECT_EQ(nullptr, err)
      << "Write should succeed despite unlink fault (rename replaces atomically)";
}
