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

#include "storage/yaml/yaml_tree_walker.h"
#include "storage/yaml/yaml_parser.h"
#include "storage/yaml/yaml_datastructs.h"
#include "storage/yaml/yaml_bits.h"
#include "mixes.h"
#include "gvars.h"

#include <string>

extern void setDefaultGVars();

static const char* _model_config[] =
  {
    // As written by radio firmware - always enclosed in double quotes
    "header: \n"
    "   name: \"Tst Name\"\n",         // no embedded double quote

    "header: \n"
    "   name: \"Tst \\x22 Name\"\n",   // embedded and encoded double quote

    // As written by Companion - only enclosed in double quotes when necessary
    "header: \n"
    "   name: Tst Name\n",             // no embedded double quote

    "header: \n"
    "   name: Tst \" Name\n",          // embedded double quote in string

    "header: \n"
    "   name: \"\\\"Tst Name\"\n",     // embedded double quote at start of string
  };

static void loadModelYamlStr(const char* str)
{
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);

  YamlParser yp;
  yp.init(YamlTreeWalker::get_parser_calls(), &tree);

  size_t len = strlen(str);
  yp.parse(str, len);
}

static char* modelName()
{
  static char name[LEN_MODEL_NAME + 1];
  strncpy(name, g_model.header.name, LEN_MODEL_NAME);
  name[LEN_MODEL_NAME] = 0;
  return name;
}

TEST(Model, testModelNameParse)
{
  loadModelYamlStr(_model_config[0]);
  EXPECT_STREQ(modelName(), "Tst Name");
  loadModelYamlStr(_model_config[1]);
  EXPECT_STREQ(modelName(), "Tst \" Name");
  loadModelYamlStr(_model_config[2]);
  EXPECT_STREQ(modelName(), "Tst Name");
  loadModelYamlStr(_model_config[3]);
  EXPECT_STREQ(modelName(), "Tst \" Name");
  loadModelYamlStr(_model_config[4]);
  EXPECT_STREQ(modelName(), "\"Tst Name");
}

// Legacy GVar format: GV1..GV9 (1-based) and -GV1..-GV9
TEST(Model, testLegacyGVarWeight)
{
  modelArenaInit();

  // Positive GVar: GV6 → 0-based index 5
  loadModelYamlStr(
    "mixData:\n"
    " -\n"
    "   weight: GV6\n"
    "   destCh: 0\n"
  );

  MixData* mix = mixAddress(0);
  ASSERT_NE(mix, nullptr);
  EXPECT_TRUE(mix->weight.isSource);
  EXPECT_EQ(mix->weight.srcType, SOURCE_TYPE_GVAR);

  SourceRef ref = mix->weight.toSourceRef();
  EXPECT_EQ(ref.type, SOURCE_TYPE_GVAR);
  EXPECT_EQ(ref.index, 5);  // GV6 is 1-based → index 5
  EXPECT_FALSE(ref.isInverted());

  // Negative GVar: -GV6 → 0-based index 5, inverted
  modelArenaInit();
  loadModelYamlStr(
    "mixData:\n"
    " -\n"
    "   weight: -GV6\n"
    "   destCh: 0\n"
  );

  mix = mixAddress(0);
  ASSERT_NE(mix, nullptr);
  EXPECT_TRUE(mix->weight.isSource);
  EXPECT_EQ(mix->weight.srcType, SOURCE_TYPE_GVAR);

  ref = mix->weight.toSourceRef();
  EXPECT_EQ(ref.type, SOURCE_TYPE_GVAR);
  EXPECT_EQ(ref.index, 5);  // GV6 is 1-based → index 5
  EXPECT_TRUE(ref.isInverted());

  // New format: gv(5) → 0-based index 5
  modelArenaInit();
  loadModelYamlStr(
    "mixData:\n"
    " -\n"
    "   weight: \"gv(5)\"\n"
    "   destCh: 0\n"
  );

  mix = mixAddress(0);
  ASSERT_NE(mix, nullptr);
  EXPECT_TRUE(mix->weight.isSource);

  ref = mix->weight.toSourceRef();
  EXPECT_EQ(ref.type, SOURCE_TYPE_GVAR);
  EXPECT_EQ(ref.index, 5);
  EXPECT_FALSE(ref.isInverted());

  // New format inverted: !gv(5) → 0-based index 5, inverted
  modelArenaInit();
  loadModelYamlStr(
    "mixData:\n"
    " -\n"
    "   weight: \"!gv(5)\"\n"
    "   destCh: 0\n"
  );

  mix = mixAddress(0);
  ASSERT_NE(mix, nullptr);
  EXPECT_TRUE(mix->weight.isSource);

  ref = mix->weight.toSourceRef();
  EXPECT_EQ(ref.type, SOURCE_TYPE_GVAR);
  EXPECT_EQ(ref.index, 5);
  EXPECT_TRUE(ref.isInverted());
}

// Helper: write model YAML to a std::string
static std::string writeModelYamlToString()
{
  std::string result;
  auto writer = [](void* opaque, const char* str, size_t len) -> bool {
    auto* s = reinterpret_cast<std::string*>(opaque);
    s->append(str, len);
    return true;
  };

  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&g_model);
  tree.generate(writer, &result);
  return result;
}

// Test reading old YAML format: gvar values nested inside flightModeData
TEST(Model, readGVarsNestedInFlightModes)
{
  modelArenaInit();
  setDefaultGVars();

  loadModelYamlStr(
    "flightModeData:\n"
    "   0:\n"
    "      gvars:\n"
    "         0:\n"
    "            val: 100\n"
    "         2:\n"
    "            val: -50\n"
    "   1:\n"
    "      name: \"FM1\"\n"
    "      gvars:\n"
    "         0:\n"
    "            val: 200\n"
  );

  // FM0 gvar values
  EXPECT_EQ(GVAR_VALUE(0, 0), 100);
  EXPECT_EQ(GVAR_VALUE(1, 0), 0);      // default
  EXPECT_EQ(GVAR_VALUE(2, 0), -50);

  // FM1 gvar values
  EXPECT_EQ(GVAR_VALUE(0, 1), 200);

  // FM1 name
  FlightModeData* fmd = flightModeAddress(1);
  EXPECT_EQ(strncmp(fmd->name, "FM1", 3), 0);

  // Unmodified FMs should keep defaults
  EXPECT_EQ(GVAR_VALUE(0, 2), GVAR_MAX + 1);
}

// Test writing old YAML format: gvar values appear nested inside flightModeData
TEST(Model, writeGVarsNestedInFlightModes)
{
  modelArenaInit();
  setDefaultGVars();

  // Set some gvar values
  GVAR_VALUE(0, 0) = 42;     // FM0, GV0
  GVAR_VALUE(1, 1) = 123;    // FM1, GV1

  std::string yaml = writeModelYamlToString();

  // Should contain gvars nested under flightModeData
  EXPECT_NE(yaml.find("flightModeData:"), std::string::npos)
      << "flightModeData not found in YAML output";

  // FM0 should have gvar 0 with val 42
  EXPECT_NE(yaml.find("val: 42"), std::string::npos)
      << "FM0 GV0 value not found in YAML output";

  // FM1 should have gvar 1 with val 123
  EXPECT_NE(yaml.find("val: 123"), std::string::npos)
      << "FM1 GV1 value not found in YAML output";

  // Should NOT contain top-level gvarValues
  EXPECT_EQ(yaml.find("gvarValues:"), std::string::npos)
      << "Top-level gvarValues should not appear in YAML output";
}

// Test full round-trip: write → read → verify
TEST(Model, gvarYamlRoundTrip)
{
  modelArenaInit();
  setDefaultGVars();

  // Set diverse gvar values across multiple FMs
  GVAR_VALUE(0, 0) = 500;     // FM0, GV0
  GVAR_VALUE(2, 0) = -100;    // FM0, GV2
  GVAR_VALUE(0, 1) = 200;     // FM1, GV0
  GVAR_VALUE(3, 2) = -1024;   // FM2, GV3

  // Write to YAML string
  std::string yaml = writeModelYamlToString();

  // Clear model and reinitialize
  memset(&g_model, 0, sizeof(g_model));
  inputNameIndexReset();
  modelArenaInit();
  setDefaultGVars();

  // Verify all gvar values are at defaults
  EXPECT_EQ(GVAR_VALUE(0, 0), 0);
  EXPECT_EQ(GVAR_VALUE(0, 1), GVAR_MAX + 1);

  // Parse the YAML back
  loadModelYamlStr(yaml.c_str());

  // Verify all values survived the round-trip
  EXPECT_EQ(GVAR_VALUE(0, 0), 500);
  EXPECT_EQ(GVAR_VALUE(2, 0), -100);
  EXPECT_EQ(GVAR_VALUE(0, 1), 200);
  EXPECT_EQ(GVAR_VALUE(3, 2), -1024);

  // Unset values should remain at defaults
  EXPECT_EQ(GVAR_VALUE(1, 0), 0);           // FM0 default = 0
  EXPECT_EQ(GVAR_VALUE(1, 1), GVAR_MAX + 1); // FM1+ default = inherit
}
