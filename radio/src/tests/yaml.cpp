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

#include <storage/yaml/yaml_node.h>
#include <storage/yaml/yaml_parser.h>
#include <storage/yaml/yaml_tree_walker.h>

struct TestStruct {
  uint8_t foo;
  uint8_t bar;

  TestStruct() : foo(0), bar(0) {}
};

static const struct YamlNode struct_TestStruct[] = {
  YAML_UNSIGNED( "foo", 8 ),
  YAML_UNSIGNED( "bar", 8 ),
  YAML_END
};

static const struct YamlNode struct_test[] = {
  YAML_STRUCT("testStruct", sizeof(TestStruct) * 8, struct_TestStruct, NULL),
  YAML_END
};

static const struct YamlNode _root_node = YAML_ROOT( struct_test );
    
TEST(Yaml, SkipBlankLines)
{
  TestStruct t;

  YamlTreeWalker tree;
  tree.reset(&_root_node, (uint8_t*)&t);

  const char chunk_1[] = "testStruct:\n  foo: 12\n";
  const char chunk_2[] = "\n  bar: 34\n\n  fo";
  const char chunk_3[] = "o: 45";
  
  YamlParser yp;
  yp.init(YamlTreeWalker::get_parser_calls(), &tree);
  EXPECT_EQ(YamlParser::CONTINUE_PARSING, yp.parse(chunk_1, sizeof(chunk_1) - 1));
  EXPECT_EQ(12, t.foo);
  
  EXPECT_EQ(YamlParser::CONTINUE_PARSING, yp.parse(chunk_2, sizeof(chunk_2) - 1));
  EXPECT_EQ(34, t.bar);

  yp.set_eof();
  EXPECT_EQ(YamlParser::CONTINUE_PARSING, yp.parse(chunk_3, sizeof(chunk_3) - 1));
  EXPECT_EQ(45, t.foo);
}
