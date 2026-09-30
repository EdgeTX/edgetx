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

#include "gtests.h"

#include <string>

#include "storage/yaml/yaml_datastructs.h"
#include "storage/yaml/yaml_parser.h"
#include "storage/yaml/yaml_tree_walker.h"

static bool yaml_string_writer(void* opaque, const char* str, size_t len)
{
  static_cast<std::string*>(opaque)->append(str, len);
  return true;
}

static std::string writeModelToString()
{
  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);
  EXPECT_TRUE(tree.generate(yaml_string_writer, &yaml));
  return yaml;
}

static void readModelFromString(const std::string& yaml)
{
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);
  YamlParser yp;
  yp.init(YamlTreeWalker::get_parser_calls(), &tree);
  yp.set_eof();
  yp.parse(yaml.c_str(), yaml.size());
}

static void replaceOnce(std::string& yaml, const std::string& from,
                        const std::string& to)
{
  auto pos = yaml.find(from);
  ASSERT_NE(std::string::npos, pos) << from;
  EXPECT_EQ(std::string::npos, yaml.find(from, pos + 1)) << from;
  yaml.replace(pos, from.size(), to);
}

class TelemetryCalcTest : public EdgeTxTest
{
 protected:
  void SetUp() override
  {
    EdgeTxTest::SetUp();
    for (auto& item : telemetryItems) item.clear();
  }

  static TelemetrySensor& calcSensor(uint8_t idx, uint8_t formula,
                                     uint8_t prec = 2)
  {
    TelemetrySensor& sensor = g_model.telemetrySensors[idx];
    sensor.init("CALC", UNIT_VOLTS, prec);
    sensor.type = TELEM_TYPE_CALCULATED;
    sensor.formula = formula;
    return sensor;
  }

  // Custom sensor with a fresh value (volts, prec 2)
  static void customSensor(uint8_t idx, int32_t value)
  {
    TelemetrySensor& sensor = g_model.telemetrySensors[idx];
    sensor.init("VFAS", UNIT_VOLTS, 2);
    telemetryItems[idx].setValue(sensor, value, UNIT_VOLTS, 2);
  }
};

TEST_F(TelemetryCalcTest, SourceEncoding)
{
  EXPECT_FALSE(calcSourceIsGVar(0));
  EXPECT_FALSE(calcSourceIsGVar(MAX_TELEMETRY_SENSORS));
  EXPECT_FALSE(calcSourceIsGVar(-MAX_TELEMETRY_SENSORS));

  for (uint8_t i = 0; i < MAX_GVARS; i++) {
    for (bool neg : {false, true}) {
      int8_t src = calcSourceFromGVar(i, neg);
      EXPECT_TRUE(calcSourceIsGVar(src));
      EXPECT_EQ(i, calcSourceGVarIndex(src));
      EXPECT_EQ(neg, src < 0);
    }
  }
}

TEST_F(TelemetryCalcTest, YamlWriteSources)
{
  auto& sensor = calcSensor(0, TELEM_FORMULA_ADD);
  sensor.calc.sources[0] = 3;
  sensor.calc.sources[1] = -4;
  sensor.calc.sources[2] = calcSourceFromGVar(0, false);
  sensor.calc.sources[3] = calcSourceFromGVar(MAX_GVARS - 1, true);

  auto yaml = writeModelToString();
  EXPECT_NE(std::string::npos, yaml.find("val: 3\r\n"));
  EXPECT_NE(std::string::npos, yaml.find("val: -4\r\n"));
  EXPECT_NE(std::string::npos, yaml.find("val: GV1\r\n"));
  EXPECT_NE(std::string::npos,
            yaml.find("val: -GV" + std::to_string(MAX_GVARS) + "\r\n"));

  // round trip
  memset(sensor.calc.sources, 0, sizeof(sensor.calc.sources));
  readModelFromString(yaml);
  EXPECT_EQ(3, sensor.calc.sources[0]);
  EXPECT_EQ(-4, sensor.calc.sources[1]);
  EXPECT_EQ(calcSourceFromGVar(0, false), sensor.calc.sources[2]);
  EXPECT_EQ(calcSourceFromGVar(MAX_GVARS - 1, true), sensor.calc.sources[3]);
}

TEST_F(TelemetryCalcTest, YamlReadSources)
{
  auto& sensor = calcSensor(0, TELEM_FORMULA_ADD);
  sensor.calc.sources[0] = 31;
  sensor.calc.sources[1] = -32;
  sensor.calc.sources[2] = 33;
  sensor.calc.sources[3] = 34;
  auto yaml = writeModelToString();

  // Existing sensor sources read as-is, GVn / -GVn as GVars
  replaceOnce(yaml, "val: 31\r\n", "val: GV3\r\n");
  replaceOnce(yaml, "val: -32\r\n", "val: -GV2\r\n");

  memset(sensor.calc.sources, 0, sizeof(sensor.calc.sources));
  readModelFromString(yaml);
  EXPECT_EQ(calcSourceFromGVar(2, false), sensor.calc.sources[0]);
  EXPECT_EQ(calcSourceFromGVar(1, true), sensor.calc.sources[1]);
  EXPECT_EQ(33, sensor.calc.sources[2]);
  EXPECT_EQ(34, sensor.calc.sources[3]);
}

TEST_F(TelemetryCalcTest, YamlReadInvalidSources)
{
  auto& sensor = calcSensor(0, TELEM_FORMULA_ADD);
  sensor.calc.sources[0] = 31;
  sensor.calc.sources[1] = 32;
  sensor.calc.sources[2] = 33;
  sensor.calc.sources[3] = 34;
  auto yaml = writeModelToString();

  replaceOnce(yaml, "val: 31\r\n",
              "val: GV" + std::to_string(MAX_GVARS + 1) + "\r\n");
  replaceOnce(yaml, "val: 32\r\n", "val: GV0\r\n");
  replaceOnce(yaml, "val: 33\r\n", "val: GV1x\r\n");
  replaceOnce(yaml, "val: 34\r\n", "val: 127\r\n");

  readModelFromString(yaml);
  for (int i = 0; i < 4; i++) EXPECT_EQ(0, sensor.calc.sources[i]) << i;
}

#if defined(GVARS)
// GVar values are used as displayed (with the GVar's own precision)

TEST_F(TelemetryCalcTest, AddGVars)
{
  customSensor(0, 1200);                       // 12.00V
  g_model.flightModeData[0].gvars[1] = 1;      // 1

  auto& sensor = calcSensor(1, TELEM_FORMULA_ADD);
  sensor.calc.sources[0] = 1;
  sensor.calc.sources[1] = calcSourceFromGVar(1, false);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(1300, telemetryItems[1].value);    // 12.00 + 1

  sensor.calc.sources[1] = calcSourceFromGVar(1, true);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(1100, telemetryItems[1].value);    // 12.00 - 1

  g_model.gvars[1].prec = 1;
  g_model.flightModeData[0].gvars[1] = 5;      // 0.5
  sensor.calc.sources[1] = calcSourceFromGVar(1, false);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(1250, telemetryItems[1].value);    // 12.00 + 0.5

  sensor.prec = 0;
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(12, telemetryItems[1].value);      // 12 + 0 (0.5 truncated)
}

TEST_F(TelemetryCalcTest, MinMaxGVarFirst)
{
  customSensor(0, 1200);                       // 12.00V
  g_model.flightModeData[0].gvars[0] = 3;      // 3

  auto& sensor = calcSensor(1, TELEM_FORMULA_MIN);
  sensor.calc.sources[0] = calcSourceFromGVar(0, false);
  sensor.calc.sources[1] = 1;
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(300, telemetryItems[1].value);

  sensor.formula = TELEM_FORMULA_MAX;
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(1200, telemetryItems[1].value);
}

TEST_F(TelemetryCalcTest, MultiplyDivideGVar)
{
  customSensor(0, 1200);                       // 12.00V
  g_model.flightModeData[0].gvars[0] = 3;      // 3 cells

  auto& sensor = calcSensor(1, TELEM_FORMULA_MULTIPLY);
  sensor.calc.sources[0] = 1;
  sensor.calc.sources[1] = calcSourceFromGVar(0, true);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(400, telemetryItems[1].value);     // 12.00 / 3

  sensor.calc.sources[1] = calcSourceFromGVar(0, false);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(3600, telemetryItems[1].value);    // 12.00 * 3

  g_model.gvars[0].prec = 1;
  g_model.flightModeData[0].gvars[0] = 25;     // 2.5
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(3000, telemetryItems[1].value);    // 12.00 * 2.5

  sensor.calc.sources[1] = calcSourceFromGVar(0, true);
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(480, telemetryItems[1].value);     // 12.00 / 2.5

  g_model.flightModeData[0].gvars[0] = 0;
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(0, telemetryItems[1].value);       // divide by zero
}

TEST_F(TelemetryCalcTest, DivideByGVarFirst)
{
  customSensor(0, 1200);                       // 12.00V
  g_model.flightModeData[0].gvars[0] = 2;

  auto& sensor = calcSensor(1, TELEM_FORMULA_MULTIPLY);
  sensor.calc.sources[0] = calcSourceFromGVar(0, true);
  sensor.calc.sources[1] = 1;
  telemetryItems[1].eval(sensor);
  EXPECT_EQ(600, telemetryItems[1].value);     // 1 / 2 * 12.00
}

TEST_F(TelemetryCalcTest, GVarOnlySensorStaysFresh)
{
  g_model.flightModeData[0].gvars[0] = 10;
  g_model.flightModeData[0].gvars[1] = 20;

  auto& sensor = calcSensor(0, TELEM_FORMULA_ADD);
  EXPECT_FALSE(sensor.isOfflineFresh());

  sensor.calc.sources[0] = calcSourceFromGVar(0, false);
  sensor.calc.sources[1] = calcSourceFromGVar(1, false);
  EXPECT_TRUE(sensor.isOfflineFresh());
  telemetryItems[0].eval(sensor);
  EXPECT_EQ(3000, telemetryItems[0].value);    // 10 + 20, prec 2

  sensor.calc.sources[2] = 2;
  EXPECT_FALSE(sensor.isOfflineFresh());
}
#endif
