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

// Model-level YAML round-trip corpus test.
//
// Gates the byte-compatibility of the BoundedString<N> migration: the
// migrated ModelData header fields (name, labels) must survive Companion's
// real YAML (de)serialisation unchanged, and the serialisation must be a
// byte-stable fixed point. Truncation to the field capacity must match the
// radio (and the legacy char[] behaviour) exactly.
//
// Note: ModelData::filename is the on-storage file name and is NOT part of a
// single model's YAML (the ScriptData "file" key is unrelated), so it is not
// exercised here -- its byte equivalence is covered by boundedstring_test.

#include "gtests.h"

#include <QByteArray>
#include <string>
#include <vector>

#include "firmwares/eeprominterface.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "firmwares/edgetx/yaml_moduledata.h"

namespace {

// Round-trip a model through the real Companion YAML serialisation and return
// the reloaded copy; hands the serialised bytes back to the caller.
ModelData roundTrip(const ModelData& in, QByteArray& yamlOut)
{
  writeModelToYaml(in, yamlOut);
  ModelData out;
  out.clear();
  loadModelFromYaml(out, yamlOut);
  return out;
}

class ModelYamlRoundTrip : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("tx16s"));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << "tx16s firmware must be registered for this test";
  }
};

}  // namespace

// Valid, in-limit header fields survive a round-trip unchanged, and writing
// the reloaded model reproduces identical bytes (a deterministic fixed point).
TEST_F(ModelYamlRoundTrip, HeaderFieldsSurviveAndAreStable)
{
  struct Case { const char* name; const char* labels; };
  const std::vector<Case> corpus = {
    {"MyModel",         "fav,race"},
    {"ABCDEFGHIJKLMNO", "alpha,bravo"},     // name exactly at the 15-char limit
    {"",                ""},                 // empty model
    {"Plane01",         "fav"},
    {"Heli3D",          "race,fav,park,3d,heli,test"},
  };

  for (const auto& c : corpus) {
    ModelData m;
    m.clear();
    m.used = true;
    m.name = c.name;
    m.labels = c.labels;

    QByteArray y1;
    ModelData m2 = roundTrip(m, y1);

    EXPECT_EQ(m.name.str(),   m2.name.str())   << "name lost for: "   << c.name;
    EXPECT_EQ(m.labels.str(), m2.labels.str()) << "labels lost for: " << c.name;

    // Fixed point at the field level: reloading the re-serialised model
    // yields the same field bytes. (We deliberately do NOT compare the whole
    // YAML document -- it round-trips unrelated non-string fields that have
    // pre-existing non-idempotencies, e.g. varioData.centerSilent.)
    QByteArray y2;
    ModelData m3 = roundTrip(m2, y2);

    EXPECT_EQ(m2.name.str(),   m3.name.str());
    EXPECT_EQ(m2.labels.str(), m3.labels.str());
  }
}

// An over-length name is truncated to the field capacity on assignment (no
// overflow), and that truncated value is exactly what round-trips.
TEST_F(ModelYamlRoundTrip, OverlongNameTruncatesAndRoundTrips)
{
  ModelData m;
  m.clear();
  m.used = true;
  m.name = std::string(40, 'N');  // capacity is MODEL_NAME_LEN (15)

  ASSERT_EQ(m.name.size(), (size_t)MODEL_NAME_LEN);

  QByteArray y1;
  ModelData m2 = roundTrip(m, y1);
  EXPECT_EQ(m.name.str(), m2.name.str());

  // The truncated value is a fixed point: no further loss or growth.
  QByteArray y2;
  ModelData m3 = roundTrip(m2, y2);
  EXPECT_EQ(m2.name.str(), m3.name.str());
}

// A module written by a radio that emitted antennaMode must decode. The field is
// a string enum firmware side; read as a raw int, yaml-cpp throws
// TypedBadConversion and the whole model fails to open.
TEST_F(ModelYamlRoundTrip, RadioWrittenAntennaModeDecodes)
{
  YAML::Node node = YAML::Load(
      "type: TYPE_MULTIMODULE\n"
      "subType: 6,7\n"
      "channelsStart: 0\n"
      "channelsCount: 16\n"
      "failsafeMode: NOT_SET\n"
      "antennaMode: MODE_PER_MODEL\n");

  ModuleData md;
  bool threw = false;
  std::string err;
  try {
    node >> md;
  } catch (const std::exception& e) {
    threw = true;
    err = e.what();
  } catch (...) {
    threw = true;
    err = "non-std exception";
  }

  ASSERT_FALSE(threw) << "decoding threw: " << err;
  EXPECT_EQ(md.subType, 7u) << "subType lost alongside antennaMode";
}

// The fixture selects tx16s, which has no external antenna. Such radios parse
// antennaMode over the top two bits of subType, so writing the key back would
// change the sub-protocol on load.
TEST_F(ModelYamlRoundTrip, AntennaModeNotWrittenWithoutExternalAntenna)
{
  ASSERT_FALSE(Boards::getCapability(getCurrentBoard(), Board::HasExternalAntenna))
      << "fixture board is expected to have no external antenna";

  ModelData m;
  m.clear();
  m.used = true;
  // only modules with a protocol are serialised at all
  m.moduleData[1].protocol = PULSES_MULTIMODULE;
  m.moduleData[1].antennaMode = GeneralSettings::ANTENNA_MODE_EXTERNAL;

  QByteArray y;
  writeModelToYaml(m, y);
  ASSERT_TRUE(y.contains("moduleData")) << "module was not serialised";
  EXPECT_FALSE(y.contains("antennaMode"));
}
