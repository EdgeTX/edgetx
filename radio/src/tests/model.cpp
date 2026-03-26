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

// YAML round-trip for a LogicalSwitchData entry.
// Parses a FUNC_STICKY logical switch with "L2,L3" operands and "L1" as
// AND-switch; verifies func, v1, v2, andsw, delay, and duration fields.
// Based on the common rc-soar pattern of using STICKY with an AND-switch to
// latch launch detection until the pilot resets it.
TEST(Model, LogicalSwitchYamlParse)
{
  MODEL_RESET();

  // The indexed format (0:) is what firmware writes for arrays with YAML_IDX.
  // "L1" → SWSRC_FIRST_LOGICAL_SWITCH+0
  // "L2" → SWSRC_FIRST_LOGICAL_SWITCH+1
  // "L3" → SWSRC_FIRST_LOGICAL_SWITCH+2
  loadModelYamlStr(
    "logicalSw:\n"
    "  0:\n"
    "    func: FUNC_STICKY\n"
    "    def: L2,L3\n"
    "    andsw: L1\n"
    "    delay: 5\n"
    "    duration: 30\n"
  );

  LogicalSwitchData * ls = lswAddress(0);
  EXPECT_EQ(ls->func, LS_FUNC_STICKY) << "func should be STICKY (17)";
  EXPECT_EQ(ls->v1, SWSRC_FIRST_LOGICAL_SWITCH + 1) << "v1 should be L2";
  EXPECT_EQ(ls->v2, SWSRC_FIRST_LOGICAL_SWITCH + 2) << "v2 should be L3";
  EXPECT_EQ(ls->andsw, SWSRC_FIRST_LOGICAL_SWITCH + 0) << "andsw should be L1";
  EXPECT_EQ(ls->delay, 5) << "delay field mismatch";
  EXPECT_EQ(ls->duration, 30) << "duration field mismatch";
}

// YAML round-trip for a MixData entry with non-default flightModes and mltpx.
// Parses a REPL mix active only in FM1 ("101111111" = 509 decimal) and
// verifies both the multiplex and the 9-bit flight-modes bitmask.
TEST(Model, MixDataFlightModesYamlParse)
{
  MODEL_RESET();

  // mixData uses list (-) format (no YAML_IDX in struct_MixData).
  // "101111111": bit0=1, bit1=0, bits2-8=1 → 1+4+8+16+32+64+128+256 = 509.
  loadModelYamlStr(
    "mixData:\n"
    "  - destCh: 0\n"
    "    srcRaw: MAX\n"
    "    weight: 100\n"
    "    mltpx: REPL\n"
    "    flightModes: 101111111\n"
    "    offset: 0\n"
    "    swtch: NONE\n"
    "    delayUp: 0\n"
    "    delayDown: 0\n"
    "    speedUp: 0\n"
    "    speedDown: 0\n"
  );

  EXPECT_EQ(g_model.mixData[0].mltpx, (uint8_t)MLTPX_REPL)
      << "mltpx should be REPL (2)";
  EXPECT_EQ(g_model.mixData[0].flightModes, 509u)
      << "flightModes should be 509 (all-except-FM1)";
  EXPECT_EQ(g_model.mixData[0].srcRaw, MIXSRC_MAX)
      << "srcRaw should be MIXSRC_MAX";
}

// YAML round-trip for FlightModeData with a GVar value in a non-base FM.
// Parses FM1 with gvars[0]=80, then verifies that FM0 and FM2 are
// unaffected (per-FM GVar isolation at the YAML parsing level).
TEST(Model, FlightModeGVarYamlParse)
{
  MODEL_RESET();

  // flightModeData uses indexed (0:) format.
  // gvars array also uses indexed format with struct_signed_16 {val: N}.
  loadModelYamlStr(
    "flightModeData:\n"
    "  1:\n"
    "    name: CAL\n"
    "    fadeIn: 0\n"
    "    fadeOut: 0\n"
    "    gvars:\n"
    "      0:\n"
    "        val: 80\n"
  );

  EXPECT_EQ(g_model.flightModeData[1].gvars[0], 80)
      << "FM1 gvars[0] should be 80 after YAML parse";
  // FM0 and FM2 are not mentioned in the YAML; they stay at 0.
  EXPECT_EQ(g_model.flightModeData[0].gvars[0], 0)
      << "FM0 gvars[0] should be untouched";
  EXPECT_EQ(g_model.flightModeData[2].gvars[0], 0)
      << "FM2 gvars[0] should be untouched";
  EXPECT_STREQ(g_model.flightModeData[1].name, "CAL")
      << "FM1 name should be CAL";
}

// YAML round-trip for LimitData with ppmCenter and revert.
// ahi_110 rc-soar model has ppmCenter: 18 on several outputs;
// revert: 1 is used on reversed servo channels.
TEST(Model, LimitDataPpmCenterYamlParse)
{
  MODEL_RESET();

  // limitData uses indexed (0:) format.
  loadModelYamlStr(
    "limitData:\n"
    "  0:\n"
    "    min: -100\n"
    "    max: 100\n"
    "    ppmCenter: 18\n"
    "    offset: 0\n"
    "    symetrical: 0\n"
    "    revert: 1\n"
    "    curve: 0\n"
  );

  EXPECT_EQ(g_model.limitData[0].ppmCenter, 18)
      << "ppmCenter should be 18 after YAML parse";
  EXPECT_EQ(g_model.limitData[0].revert, 1)
      << "revert should be 1 after YAML parse";
  // Adjacent output must be unaffected.
  EXPECT_EQ(g_model.limitData[1].ppmCenter, 0)
      << "limitData[1] ppmCenter should be 0 (untouched)";
}
