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
