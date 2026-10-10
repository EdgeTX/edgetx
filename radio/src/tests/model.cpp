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

TEST(Model, limitGVarLegacyParse)
{
  memclear(&g_model, sizeof(g_model));
  loadModelYamlStr(
      "limitData:\n"
      "  0:\n"
      "    min: GV1\n"
      "    max: -GV10\n"
      "    offset: -15\n"
      "  1:\n"
      "    min: -250\n"
      "    max: 120\n"
      "    offset: GV2\n");

  LimitNumVal v;
  v.rawValue = g_model.limitData[0].min;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, MIXSRC_FIRST_GVAR);
  v.rawValue = g_model.limitData[0].max;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, -(MIXSRC_FIRST_GVAR + 9));
  v.rawValue = g_model.limitData[0].offset;
  EXPECT_FALSE(v.isSource);
  EXPECT_EQ(v.value, -15);

  v.rawValue = g_model.limitData[1].min;
  EXPECT_FALSE(v.isSource);
  EXPECT_EQ(v.value, -250);
  v.rawValue = g_model.limitData[1].max;
  EXPECT_FALSE(v.isSource);
  EXPECT_EQ(v.value, 120);
  v.rawValue = g_model.limitData[1].offset;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, MIXSRC_FIRST_GVAR + 1);
}

static bool stringWriter(void* opaque, const char* str, size_t len)
{
  static_cast<std::string*>(opaque)->append(str, len);
  return true;
}

TEST(Model, limitSourcesRoundTrip)
{
  memclear(&g_model, sizeof(g_model));
  ModelData src;
  memclear(&src, sizeof(src));
  src.limitData[0].min = makeLimitNumVal(MIXSRC_FIRST_GVAR + 2, true);
  src.limitData[0].max = makeLimitNumVal(-(MIXSRC_FIRST_GVAR + 1), true);
  src.limitData[0].offset = makeLimitNumVal(MIXSRC_FIRST_CH + 3, true);
  src.limitData[1].min = makeLimitNumVal(-250);
  src.limitData[1].max = makeLimitNumVal(499);
  src.limitData[1].offset = makeLimitNumVal(-1000);

  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&src);
  ASSERT_TRUE(tree.generate(stringWriter, &yaml));
  EXPECT_NE(yaml.find("gv(2)"), std::string::npos);

  ModelData dst;
  memclear(&dst, sizeof(dst));
  loadModelYamlStr(yaml.c_str());
  for (int i = 0; i < 2; i++) {
    EXPECT_EQ(g_model.limitData[i].min, src.limitData[i].min) << i;
    EXPECT_EQ(g_model.limitData[i].max, src.limitData[i].max) << i;
    EXPECT_EQ(g_model.limitData[i].offset, src.limitData[i].offset) << i;
  }
}

TEST(Model, legacyGVarWeightsKeepTheirIndex)
{
  memclear(&g_model, sizeof(g_model));
  // "GV10" and above used to be read as GV1
  loadModelYamlStr(
      "mixData:\n"
      "  - weight: GV10\n"
      "    offset: -GV15\n"
      "    destCh: 0\n"
      "    srcRaw: Rud\n");

  SourceNumVal v;
  v.rawValue = g_model.mixData[0].weight;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, MIXSRC_FIRST_GVAR + 9);
  v.rawValue = g_model.mixData[0].offset;
  EXPECT_TRUE(v.isSource);
  EXPECT_EQ(v.value, -(MIXSRC_FIRST_GVAR + 14));
}

TEST(Model, limitNumberBoundariesRoundTrip)
{
  memclear(&g_model, sizeof(g_model));
  ModelData src;
  memclear(&src, sizeof(src));
  const int vals[] = {-1000, -500, -1, 0, 1, 500, 1000};
  for (int i = 0; i < 7; i++) {
    src.limitData[i].min = makeLimitNumVal(vals[i]);
    src.limitData[i].max = makeLimitNumVal(-vals[i]);
    src.limitData[i].offset = makeLimitNumVal(vals[i]);
  }

  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&src);
  ASSERT_TRUE(tree.generate(stringWriter, &yaml));

  loadModelYamlStr(yaml.c_str());
  for (int i = 0; i < 7; i++) {
    EXPECT_EQ(g_model.limitData[i].min, src.limitData[i].min) << i;
    EXPECT_EQ(g_model.limitData[i].max, src.limitData[i].max) << i;
    EXPECT_EQ(g_model.limitData[i].offset, src.limitData[i].offset) << i;
  }
}

TEST(Model, limitInvertedSourceRoundTrip)
{
  memclear(&g_model, sizeof(g_model));
  ModelData src;
  memclear(&src, sizeof(src));
  src.limitData[0].min = makeLimitNumVal(-MIXSRC_MAX, true);
  src.limitData[0].max = makeLimitNumVal(-(MIXSRC_FIRST_GVAR + MAX_GVARS - 1), true);
  src.limitData[0].offset = makeLimitNumVal(MIXSRC_FIRST_GVAR + MAX_GVARS - 1, true);

  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&src);
  ASSERT_TRUE(tree.generate(stringWriter, &yaml));

  loadModelYamlStr(yaml.c_str());
  EXPECT_EQ(g_model.limitData[0].min, src.limitData[0].min);
  EXPECT_EQ(g_model.limitData[0].max, src.limitData[0].max);
  EXPECT_EQ(g_model.limitData[0].offset, src.limitData[0].offset);
}

// Text as written by Companion (sources unquoted unless they need it)
TEST(Model, limitCompanionFormatParses)
{
  memclear(&g_model, sizeof(g_model));
  loadModelYamlStr(
      "limitData:\n"
      "  0:\n"
      "    min: gv(2)\n"
      "    max: \"!gv(8)\"\n"
      "    revert: 0\n"
      "    offset: ch(3)\n"
      "  1:\n"
      "    min: 200\n"
      "    max: 200\n"
      "    offset: -35\n");

  EXPECT_EQ(g_model.limitData[0].min, makeLimitNumVal(MIXSRC_FIRST_GVAR + 2, true));
  EXPECT_EQ(g_model.limitData[0].max, makeLimitNumVal(-(MIXSRC_FIRST_GVAR + 8), true));
  EXPECT_EQ(g_model.limitData[0].offset, makeLimitNumVal(MIXSRC_FIRST_CH + 3, true));
  EXPECT_EQ(g_model.limitData[1].min, makeLimitNumVal(200));
  EXPECT_EQ(g_model.limitData[1].max, makeLimitNumVal(200));
  EXPECT_EQ(g_model.limitData[1].offset, makeLimitNumVal(-35));
}

// The highest valid sources, plain and inverted, must survive the YAML text
TEST(Model, limitSourceExtremesRoundTrip)
{
  const int srcs[] = {1, MIXSRC_FIRST_CH, MIXSRC_LAST_CH, MIXSRC_FIRST_GVAR,
                      MIXSRC_LAST_GVAR, MIXSRC_TX_VOLTAGE, MIXSRC_FIRST_TELEM,
                      MIXSRC_LAST_TELEM};
  for (int s : srcs) {
    for (int sign : {1, -1}) {
      ModelData src;
      memclear(&src, sizeof(src));
      src.limitData[0].max = makeLimitNumVal(sign * s, true);

      std::string yaml;
      YamlTreeWalker tree;
      tree.reset(get_modeldata_nodes(), (uint8_t*)&src);
      ASSERT_TRUE(tree.generate(stringWriter, &yaml));

      memclear(&g_model, sizeof(g_model));
      loadModelYamlStr(yaml.c_str());
      EXPECT_EQ(g_model.limitData[0].max, src.limitData[0].max) << sign * s;
    }
  }
}

// Stored values are relative to -/+100%: extended limits reach -/+500, subtrim -/+1000
TEST(Model, limitExtendedRangeRoundTrip)
{
  ModelData src;
  memclear(&src, sizeof(src));
  src.limitData[0].min = makeLimitNumVal(-LIMIT_EXT_MAX + LIMIT_STD_MAX);  // -150%
  src.limitData[0].max = makeLimitNumVal(LIMIT_EXT_MAX - LIMIT_STD_MAX);   // +150%
  src.limitData[0].offset = makeLimitNumVal(-LIMIT_STD_MAX);
  src.limitData[1].min = makeLimitNumVal(LIMIT_STD_MAX);                   // 0%
  src.limitData[1].max = makeLimitNumVal(-LIMIT_STD_MAX);
  src.limitData[1].offset = makeLimitNumVal(LIMIT_STD_MAX);

  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(get_modeldata_nodes(), (uint8_t*)&src);
  ASSERT_TRUE(tree.generate(stringWriter, &yaml));

  memclear(&g_model, sizeof(g_model));
  loadModelYamlStr(yaml.c_str());
  for (int i = 0; i < 2; i++) {
    EXPECT_EQ(LIMIT_MIN(&g_model.limitData[i]), LIMIT_MIN(&src.limitData[i])) << i;
    EXPECT_EQ(LIMIT_MAX(&g_model.limitData[i]), LIMIT_MAX(&src.limitData[i])) << i;
    EXPECT_EQ(LIMIT_OFS(&g_model.limitData[i]), LIMIT_OFS(&src.limitData[i])) << i;
  }
  EXPECT_EQ(LIMIT_MIN(&g_model.limitData[0]), -LIMIT_EXT_MAX);
  EXPECT_EQ(LIMIT_MAX(&g_model.limitData[0]), LIMIT_EXT_MAX);
}
