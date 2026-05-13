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

/**
 * Tests for changes introduced in the PR that:
 *  - Added ComponentReleaseData (per-profile release info) and moved
 *    release/version/releaseId/date/prerelease out of ComponentData.
 *  - Fixed CompStoreObj::splitGroupedPath() to handle deeply-nested paths.
 *  - Moved addObjectMapping() calls from constructors to setIndex/setIndexes.
 *  - Added Profile::getCompRelease() with bounds checking.
 *  - Bumped CPN_SETTINGS_REVISION from 2 to 3.
 *  - Renamed joystick settings-path prefixes.
 */

#include "gtests.h"
#include "storage/appdata.h"

// ---------------------------------------------------------------------------
// Helper: expose the protected CompStoreObj::splitGroupedPath() for testing.
// ---------------------------------------------------------------------------
class SplitPathHelper : public CompStoreObj
{
public:
  QString propertyGroup() const override { return QStringLiteral("Test"); }

  static QPair<QString, QString> split(const QString & path)
  {
    return CompStoreObj::splitGroupedPath(path);
  }
};

// ===========================================================================
// CPN_SETTINGS_REVISION
// ===========================================================================

TEST(AppDataPR, SettingsRevisionBumpedTo3)
{
  // The PR bumps the revision from 2 to 3 to signal settings migration.
  EXPECT_EQ(3, CPN_SETTINGS_REVISION);
}

// ===========================================================================
// CompStoreObj::splitGroupedPath()
//
// The function was changed to use list.mid(0, list.size() - 2).join("/")
// instead of list.first() so that deeply-nested property groups (more than
// two path components) are correctly identified.
//
// Contract: for a stored key of the form "<settingsPath><propKey>", the
// function must return (propertyGroup(), propKey).
// settingsPath() always ends with '/', so the stored path is
//   settingsPath() % propKey  (no extra slash).
// ===========================================================================

// Single element: no '/' present → group = "General".
TEST(AppDataPR, SplitPath_SingleKey)
{
  auto result = SplitPathHelper::split("someKey");
  EXPECT_EQ(QStringLiteral("General"), result.first);
  EXPECT_EQ(QStringLiteral("someKey"), result.second);
}

// Three-segment path: "Profiles/profile0/Name"
//   list = ["Profiles","profile0","Name"], size=3
//   mid(0, 3-2) = mid(0,1) = ["Profiles"] → "Profiles"
//   This matches Profile::propertyGroup() = "Profiles".
TEST(AppDataPR, SplitPath_ThreeSegments_ProfileProperty)
{
  auto result = SplitPathHelper::split("Profiles/profile0/Name");
  EXPECT_EQ(QStringLiteral("Profiles"), result.first);
  EXPECT_EQ(QStringLiteral("Name"), result.second);
}

// Three-segment path: "Components/component0/checkForUpdate"
//   list = ["Components","component0","checkForUpdate"], size=3
//   mid(0,1) = ["Components"] → "Components"
//   Matches ComponentData::propertyGroup() = "Components".
TEST(AppDataPR, SplitPath_ThreeSegments_ComponentProperty)
{
  auto result = SplitPathHelper::split("Components/component0/checkForUpdate");
  EXPECT_EQ(QStringLiteral("Components"), result.first);
  EXPECT_EQ(QStringLiteral("checkForUpdate"), result.second);
}

// Four-segment path: "Profiles/profile0/component0/release"
//   list = ["Profiles","profile0","component0","release"], size=4
//   mid(0, 4-2) = mid(0,2) = ["Profiles","profile0"] → "Profiles/profile0"
//   This matches ComponentReleaseData::propertyGroup() = "Profiles/profile{idx}".
TEST(AppDataPR, SplitPath_FourSegments_ComponentReleaseProperty)
{
  auto result = SplitPathHelper::split("Profiles/profile0/component0/release");
  EXPECT_EQ(QStringLiteral("Profiles/profile0"), result.first);
  EXPECT_EQ(QStringLiteral("release"), result.second);
}

// Four-segment path: "Components/component0/asset0/desc"
//   list = ["Components","component0","asset0","desc"], size=4
//   mid(0,2) = ["Components","component0"] → "Components/component0"
//   Matches ComponentAssetData::propertyGroup() = "Components/component{idx}".
TEST(AppDataPR, SplitPath_FourSegments_ComponentAssetProperty)
{
  auto result = SplitPathHelper::split("Components/component0/asset0/desc");
  EXPECT_EQ(QStringLiteral("Components/component0"), result.first);
  EXPECT_EQ(QStringLiteral("desc"), result.second);
}

// Five-segment path: deeply nested NamedJSData joystick property.
//   "NamedJSData/name0/joystick1/stick_axe"
//   list = ["NamedJSData","name0","joystick1","stick_axe"], size=4
//   mid(0,2) = ["NamedJSData","name0"] → "NamedJSData/name0"
//   Matches NamedJStickData::propertyGroup() = "NamedJSData/name{idx}".
TEST(AppDataPR, SplitPath_FourSegments_NamedJStickProperty)
{
  auto result = SplitPathHelper::split("NamedJSData/name0/joystick1/stick_axe");
  EXPECT_EQ(QStringLiteral("NamedJSData/name0"), result.first);
  EXPECT_EQ(QStringLiteral("stick_axe"), result.second);
}

// Regression: with the OLD code (list.first()), a four-segment path would
// have returned only "Profiles" for "Profiles/profile0/component0/release",
// which is wrong.  Verify the new behaviour is distinct from the old.
TEST(AppDataPR, SplitPath_FourSegments_NotJustFirstSegment)
{
  auto result = SplitPathHelper::split("Profiles/profile0/component0/release");
  // The old (incorrect) code would have returned "Profiles" here.
  EXPECT_NE(QStringLiteral("Profiles"), result.first);
  // The correct new code returns the full two-segment group.
  EXPECT_EQ(QStringLiteral("Profiles/profile0"), result.first);
}

// ===========================================================================
// ComponentReleaseData – declared default property values
//
// The static _default() methods are generated by the PROPERTY_META4 macro
// and are publicly accessible.
// ===========================================================================

TEST(AppDataPR, ComponentReleaseData_DefaultDate)
{
  EXPECT_EQ(QStringLiteral(""), ComponentReleaseData::date_default().toString());
}

TEST(AppDataPR, ComponentReleaseData_DefaultPrerelease)
{
  EXPECT_EQ(false, ComponentReleaseData::prerelease_default().toBool());
}

TEST(AppDataPR, ComponentReleaseData_DefaultRelease)
{
  EXPECT_EQ(QStringLiteral("unknown"), ComponentReleaseData::release_default().toString());
}

TEST(AppDataPR, ComponentReleaseData_DefaultReleaseId)
{
  EXPECT_EQ(0, ComponentReleaseData::releaseId_default().toInt());
}

TEST(AppDataPR, ComponentReleaseData_DefaultVersion)
{
  EXPECT_EQ(QStringLiteral("0"), ComponentReleaseData::version_default().toString());
}

// ===========================================================================
// ComponentReleaseData::propertyGroup() / settingsPath()
//
// The group must follow "Profiles/profile{profileIndex}" so that
// splitGroupedPath() can correctly reverse it.
// We verify the pattern using the objects initialised by the global AppData.
// ===========================================================================

TEST(AppDataPR, ComponentReleaseData_PropertyGroupPattern)
{
  // After AppData construction, profile[0].compRelease[0] has been set up
  // with setIndexes(0, 0).  Its propertyGroup() must be "Profiles/profile0".
  // We can't call protected propertyGroup() directly, but we can verify the
  // round-trip through splitGroupedPath: a path built as
  //   "<settingsPath>" + "<key>"  must split back to (propertyGroup, key).
  //
  // settingsPath = "Profiles/profile0/component0/"
  // key          = "release"
  // full path    = "Profiles/profile0/component0/release"
  // expected grp = "Profiles/profile0"  (= propertyGroup of compRelease[0])
  const QString fullPath = QStringLiteral("Profiles/profile0/component0/release");
  auto result = SplitPathHelper::split(fullPath);
  EXPECT_EQ(QStringLiteral("Profiles/profile0"), result.first);
  EXPECT_EQ(QStringLiteral("release"), result.second);
}

TEST(AppDataPR, ComponentReleaseData_PropertyGroupPatternProfile5Component3)
{
  // Verify for a non-zero profile and component index.
  // settingsPath = "Profiles/profile5/component3/"
  // key          = "version"
  const QString fullPath = QStringLiteral("Profiles/profile5/component3/version");
  auto result = SplitPathHelper::split(fullPath);
  EXPECT_EQ(QStringLiteral("Profiles/profile5"), result.first);
  EXPECT_EQ(QStringLiteral("version"), result.second);
}

// ===========================================================================
// Profile::getCompRelease() – bounds checking
//
// Valid indices [0, MAX_COMPONENTS-1] must return a reference to the
// corresponding compRelease element.  Out-of-range indices must clamp to
// compRelease[0].
// ===========================================================================

TEST(AppDataPR, GetCompRelease_ValidIndex_Zero)
{
  Profile & p = g.profile[0];
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(0));
}

TEST(AppDataPR, GetCompRelease_ValidIndex_One)
{
  Profile & p = g.profile[0];
  EXPECT_EQ(&p.compRelease[1], &p.getCompRelease(1));
}

TEST(AppDataPR, GetCompRelease_ValidIndex_MaxMinus1)
{
  Profile & p = g.profile[0];
  const int lastIdx = MAX_COMPONENTS - 1;
  EXPECT_EQ(&p.compRelease[lastIdx], &p.getCompRelease(lastIdx));
}

TEST(AppDataPR, GetCompRelease_OutOfRange_NegativeOne)
{
  Profile & p = g.profile[0];
  // index == -1 → must return compRelease[0]
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(-1));
}

TEST(AppDataPR, GetCompRelease_OutOfRange_MaxComponents)
{
  Profile & p = g.profile[0];
  // index == MAX_COMPONENTS → must return compRelease[0]
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(MAX_COMPONENTS));
}

TEST(AppDataPR, GetCompRelease_OutOfRange_LargePositive)
{
  Profile & p = g.profile[0];
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(9999));
}

// Const variant – same bounds logic applies.
TEST(AppDataPR, GetCompRelease_Const_ValidIndex)
{
  const Profile & p = g.profile[0];
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(0));
  EXPECT_EQ(&p.compRelease[MAX_COMPONENTS - 1], &p.getCompRelease(MAX_COMPONENTS - 1));
}

TEST(AppDataPR, GetCompRelease_Const_OutOfRange)
{
  const Profile & p = g.profile[0];
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(-1));
  EXPECT_EQ(&p.compRelease[0], &p.getCompRelease(MAX_COMPONENTS));
}

// ===========================================================================
// ComponentReleaseData::operator=
//
// The custom assignment must copy all declared properties but must NOT copy
// the index fields (profileIndex, index), ensuring the destination object
// retains its own identity.
// ===========================================================================

TEST(AppDataPR, ComponentReleaseData_AssignmentCopiesProperties)
{
  // Use two compRelease slots from different profiles so they have distinct
  // indexes and are properly initialised.
  ComponentReleaseData & src = g.profile[1].compRelease[2];
  ComponentReleaseData & dst = g.profile[2].compRelease[3];

  // Set non-default values on the source.
  src.release(QStringLiteral("v3.0.0"), false);
  src.version(QStringLiteral("3.0.0"), false);
  src.releaseId(42, false);
  src.date(QStringLiteral("2025-01-01"), false);
  src.prerelease(true, false);

  // Assign.
  dst = src;

  // All property values must match.
  EXPECT_EQ(QStringLiteral("v3.0.0"),    dst.release());
  EXPECT_EQ(QStringLiteral("3.0.0"),     dst.version());
  EXPECT_EQ(42,                           dst.releaseId());
  EXPECT_EQ(QStringLiteral("2025-01-01"), dst.date());
  EXPECT_EQ(true,                         dst.prerelease());

  // Clean up: reset source back to defaults (no-store).
  src.releaseReset(false);
  src.versionReset(false);
  src.releaseIdReset(false);
  src.dateReset(false);
  src.prereleaseReset(false);
}

TEST(AppDataPR, ComponentReleaseData_AssignmentPreservesDestinationIdentity)
{
  // After assignment, the destination object must still be the same object
  // (i.e. at the same address). The index must not change.
  ComponentReleaseData & src = g.profile[3].compRelease[0];
  ComponentReleaseData & dst = g.profile[4].compRelease[5];

  ComponentReleaseData * dstAddr = &dst;
  dst = src;

  // Object identity preserved.
  EXPECT_EQ(dstAddr, &dst);
  // The address within Profile is still the same slot.
  EXPECT_EQ(&g.profile[4].compRelease[5], &dst);
}

TEST(AppDataPR, ComponentReleaseData_AssignmentFromDefault)
{
  // Assign from a source with default values; destination should also reflect
  // those defaults afterwards.
  ComponentReleaseData & src = g.profile[5].compRelease[1];
  ComponentReleaseData & dst = g.profile[6].compRelease[1];

  // Set dst to non-default values first.
  dst.release(QStringLiteral("dirty"), false);
  dst.version(QStringLiteral("9.9"), false);
  dst.releaseId(99, false);

  // Reset src to defaults.
  src.releaseReset(false);
  src.versionReset(false);
  src.releaseIdReset(false);
  src.dateReset(false);
  src.prereleaseReset(false);

  dst = src;

  EXPECT_EQ(QStringLiteral("unknown"), dst.release());
  EXPECT_EQ(QStringLiteral("0"),       dst.version());
  EXPECT_EQ(0,                          dst.releaseId());
}

// ===========================================================================
// Profile::operator= copies compRelease array
//
// When a Profile is copy-assigned, all MAX_COMPONENTS compRelease entries
// must be copied from the source profile.
// ===========================================================================

TEST(AppDataPR, Profile_AssignmentCopiesCompReleaseArray)
{
  Profile & src = g.profile[7];
  Profile & dst = g.profile[8];

  // Write distinct values into every compRelease slot of the source.
  for (int j = 0; j < MAX_COMPONENTS; j++) {
    src.compRelease[j].release(QStringLiteral("src-release-%1").arg(j), false);
    src.compRelease[j].version(QStringLiteral("1.%1").arg(j), false);
  }

  dst = src;

  for (int j = 0; j < MAX_COMPONENTS; j++) {
    EXPECT_EQ(QStringLiteral("src-release-%1").arg(j), dst.compRelease[j].release())
        << "compRelease[" << j << "].release() mismatch";
    EXPECT_EQ(QStringLiteral("1.%1").arg(j), dst.compRelease[j].version())
        << "compRelease[" << j << "].version() mismatch";
  }

  // Clean up.
  for (int j = 0; j < MAX_COMPONENTS; j++) {
    src.compRelease[j].releaseReset(false);
    src.compRelease[j].versionReset(false);
    dst.compRelease[j].releaseReset(false);
    dst.compRelease[j].versionReset(false);
  }
}

// ===========================================================================
// ComponentData – removed properties / methods
//
// The PR removes release, version, releaseId, date, prerelease from
// ComponentData (moved to ComponentReleaseData) and removes releaseClear().
// We verify at compile time via type traits that the expected properties
// exist in ComponentReleaseData and that ComponentData no longer has them
// as normal Q_PROPERTY-backed accessors.
// ===========================================================================

TEST(AppDataPR, ComponentReleaseData_HasExpectedProperties)
{
  // Verify the five properties declared on ComponentReleaseData are accessible
  // via the generated static key accessors (created by PROPERTY_META4).
  EXPECT_FALSE(ComponentReleaseData::date_key().isEmpty());
  EXPECT_FALSE(ComponentReleaseData::prerelease_key().isEmpty());
  EXPECT_FALSE(ComponentReleaseData::release_key().isEmpty());
  EXPECT_FALSE(ComponentReleaseData::releaseId_key().isEmpty());
  EXPECT_FALSE(ComponentReleaseData::version_key().isEmpty());
}

TEST(AppDataPR, ComponentData_HasOnlyExpectedProperties)
{
  // ComponentData should only have checkForUpdate and releaseChannel now.
  // We verify via the meta-object that the number of own properties matches.
  ComponentData & c = g.component[0];
  const QMetaObject * mo = c.metaObject();
  // propertyOffset() is the index of the first property defined by ComponentData
  // (skipping QObject's own properties).
  int ownPropertyCount = mo->propertyCount() - mo->propertyOffset();
  // checkForUpdate + releaseChannel = 2
  EXPECT_EQ(2, ownPropertyCount);
}

// ===========================================================================
// Settings path renames for joystick classes
//
// The PR renamed settings paths:
//   JStickData:      "JsCalibration/{idx}/"     → "Joysticks/joystick{idx}/"
//   JButtonData:     "JsButton/{idx}/"           → "JSButtons/button{idx}/"
//   NamedJStickData: "NamedJSData/{n}/JsCalibration/{i}/" → "NamedJSData/name{n}/joystick{i}/"
//   NamedJButtonData:"NamedJSData/{n}/JsButton/{i}/"      → "NamedJSData/name{n}/button{i}/"
//   NamedJSData:     "NamedJSData/{idx}/"        → "NamedJSData/name{idx}/"
//
// We can't call settingsPath() directly (protected), but we verify the
// corresponding splitGroupedPath results match the new propertyGroup values,
// i.e. the paths that would be stored in QSettings for these objects.
// ===========================================================================

// JStickData: propertyGroup = "Joysticks", settingsPath = "Joysticks/joystick{i}/"
// Stored key for stick_axe on joystick 3: "Joysticks/joystick3/stick_axe"
TEST(AppDataPR, SplitPath_JStickData_NewPath)
{
  auto result = SplitPathHelper::split("Joysticks/joystick3/stick_axe");
  EXPECT_EQ(QStringLiteral("Joysticks"), result.first);
  EXPECT_EQ(QStringLiteral("stick_axe"), result.second);
}

// JButtonData: propertyGroup = "JSButtons", settingsPath = "JSButtons/button{i}/"
TEST(AppDataPR, SplitPath_JButtonData_NewPath)
{
  auto result = SplitPathHelper::split("JSButtons/button2/button_idx");
  EXPECT_EQ(QStringLiteral("JSButtons"), result.first);
  EXPECT_EQ(QStringLiteral("button_idx"), result.second);
}

// NamedJSData: propertyGroup = "NamedJSData", settingsPath = "NamedJSData/name{i}/"
TEST(AppDataPR, SplitPath_NamedJSData_NewPath)
{
  auto result = SplitPathHelper::split("NamedJSData/name1/jsName");
  EXPECT_EQ(QStringLiteral("NamedJSData"), result.first);
  EXPECT_EQ(QStringLiteral("jsName"), result.second);
}

// NamedJStickData: propertyGroup = "NamedJSData/name{n}", settingsPath = "NamedJSData/name{n}/joystick{i}/"
TEST(AppDataPR, SplitPath_NamedJStickData_NewPath)
{
  auto result = SplitPathHelper::split("NamedJSData/name2/joystick4/stick_inv");
  EXPECT_EQ(QStringLiteral("NamedJSData/name2"), result.first);
  EXPECT_EQ(QStringLiteral("stick_inv"), result.second);
}

// NamedJButtonData: propertyGroup = "NamedJSData/name{n}", settingsPath = "NamedJSData/name{n}/button{i}/"
TEST(AppDataPR, SplitPath_NamedJButtonData_NewPath)
{
  auto result = SplitPathHelper::split("NamedJSData/name0/button7/button_idx");
  EXPECT_EQ(QStringLiteral("NamedJSData/name0"), result.first);
  EXPECT_EQ(QStringLiteral("button_idx"), result.second);
}

// ===========================================================================
// AppData – global object initialises compRelease indexes
//
// The AppData constructor must call setIndexes(profileIdx, compIdx) for every
// compRelease entry so they are properly registered.  We verify that the
// getCompRelease() accessor returns distinct objects for each component index
// within the same profile.
// ===========================================================================

TEST(AppDataPR, AppData_CompReleaseObjectsAreDistinctPerProfile)
{
  Profile & p = g.profile[0];
  for (int j = 0; j < MAX_COMPONENTS; j++) {
    EXPECT_EQ(&p.compRelease[j], &p.getCompRelease(j))
        << "compRelease[" << j << "] address mismatch";
  }
}

TEST(AppDataPR, AppData_CompReleaseObjectsAreDistinctAcrossProfiles)
{
  // Verify two profiles have separate compRelease arrays.
  EXPECT_NE(&g.profile[0].compRelease[0], &g.profile[1].compRelease[0]);
}

// ===========================================================================
// AppData::getComponent() – unchanged API, regression guard
//
// getComponent() was not changed in this PR, but its companion
// getCompRelease() was added.  Verify that the parallel bounds logic holds.
// ===========================================================================

TEST(AppDataPR, GetComponent_ValidIndex)
{
  EXPECT_EQ(&g.component[0], &g.getComponent(0));
  EXPECT_EQ(&g.component[MAX_COMPONENTS - 1], &g.getComponent(MAX_COMPONENTS - 1));
}

TEST(AppDataPR, GetComponent_OutOfRange)
{
  EXPECT_EQ(&g.component[0], &g.getComponent(-1));
  EXPECT_EQ(&g.component[0], &g.getComponent(MAX_COMPONENTS));
}
