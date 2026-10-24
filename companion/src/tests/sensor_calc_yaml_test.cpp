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

// Calculated sensor sources: sensors are stored as signed numbers,
// GVars as GVn / -GVn. tx16s has 9 GVars.

#include "gtests.h"

#include "firmwares/eeprominterface.h"
#include "firmwares/edgetx/yaml_sensordata.h"

namespace {

class SensorCalcYaml : public ::testing::Test
{
 protected:
  void SetUp() override
  {
    Firmware::setCurrentVariant(Firmware::getFirmwareForFlavour("tx16s"));
    ASSERT_NE(getCurrentFirmware(), nullptr)
        << "tx16s firmware must be registered for this test";
  }

  static SensorData calcSensor()
  {
    SensorData sd;
    sd.type = SensorData::TELEM_TYPE_CALCULATED;
    sd.formula = SensorData::TELEM_FORMULA_ADD;
    strcpy(sd.label, "CALC");
    return sd;
  }

  static SensorData decode(const char* yaml)
  {
    return YAML::Load(yaml).as<SensorData>();
  }
};

}  // namespace

TEST_F(SensorCalcYaml, WritesSensorsAsNumbersAndGVarsAsNames)
{
  SensorData sd = calcSensor();
  sd.sources[0] = 3;
  sd.sources[1] = -4;
  sd.sources[2] = SensorData::gvarSource(0, false);
  sd.sources[3] = SensorData::gvarSource(8, true);

  YAML::Node node;
  node = sd;
  YAML::Node sources = node["cfg"]["calc"]["sources"];
  EXPECT_EQ("3", sources["0"]["val"].Scalar());
  EXPECT_EQ("-4", sources["1"]["val"].Scalar());
  EXPECT_EQ("GV1", sources["2"]["val"].Scalar());
  EXPECT_EQ("-GV9", sources["3"]["val"].Scalar());

  SensorData rt = node.as<SensorData>();
  for (int i = 0; i < 4; i++) EXPECT_EQ(sd.sources[i], rt.sources[i]) << i;
}

TEST_F(SensorCalcYaml, ReadsSensorNumbersAndGVarNames)
{
  SensorData sd = decode(
      "type: TYPE_CALCULATED\n"
      "label: CALC\n"
      "id2:\n  formula: FORMULA_MULTIPLY\n"
      "cfg:\n  calc:\n    sources:\n"
      "      0:\n        val: 12\n"
      "      1:\n        val: -GV3\n"
      "      2:\n        val: -7\n"
      "      3:\n        val: GV9\n");

  EXPECT_EQ(12, sd.sources[0]);
  EXPECT_EQ(SensorData::gvarSource(2, true), sd.sources[1]);
  EXPECT_EQ(-7, sd.sources[2]);
  EXPECT_EQ(SensorData::gvarSource(8, false), sd.sources[3]);
}

TEST_F(SensorCalcYaml, InvalidSourcesReadAsNone)
{
  SensorData sd = decode(
      "type: TYPE_CALCULATED\n"
      "label: CALC\n"
      "id2:\n  formula: FORMULA_ADD\n"
      "cfg:\n  calc:\n    sources:\n"
      "      0:\n        val: GV10\n"
      "      1:\n        val: GV0\n"
      "      2:\n        val: GV1x\n"
      "      3:\n        val: 127\n");

  for (int i = 0; i < 4; i++) EXPECT_EQ(0, sd.sources[i]) << i;
}

TEST_F(SensorCalcYaml, GVarSourceLabels)
{
  ModelData model;
  model.clear();

  EXPECT_EQ("GV1", SensorData::calcSourceToString(
                       &model, SensorData::gvarSource(0, false),
                       SensorData::TELEM_FORMULA_ADD)
                       .toStdString());
  EXPECT_EQ("-GV2", SensorData::calcSourceToString(
                        &model, SensorData::gvarSource(1, true),
                        SensorData::TELEM_FORMULA_ADD)
                        .toStdString());
  EXPECT_EQ("/GV2", SensorData::calcSourceToString(
                        &model, SensorData::gvarSource(1, true),
                        SensorData::TELEM_FORMULA_MULTIPLY)
                        .toStdString());
}
