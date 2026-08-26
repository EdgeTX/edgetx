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

// Targeted tests for the PartialModel/ModelHeader restructuring in PR #7709
// ("fix(color): modifying model labels may corrupt model yaml files").
//
// These deliberately avoid FatFS/SD-card I/O and instead exercise the two
// things that PR touches which can silently drift out of sync across the
// 27 hand-edited yaml_datastructs_*.cpp files: the PartialModel YAML node
// table, and the strAppend() length constants used to write header.labels.

#include "gtests.h"

#include "storage/yaml/yaml_tree_walker.h"
#include "storage/yaml/yaml_parser.h"
#include "storage/yaml/yaml_datastructs.h"

// Find an immediate child of an array/struct YamlNode by tag.
static const YamlNode* findChild(const YamlNode* root, const char* tag)
{
  const YamlNode* child = root->u._array.child;
  for (; child->type != YDT_NONE; child++) {
    if (child->tag && strcmp(child->tag, tag) == 0) return child;
  }
  return nullptr;
}

// The PR hand-edited struct_PartialModel's "moduleData" YAML_ARRAY entry in
// 27 generated files to read `YAML_ARRAY("moduleData", <bits>, <NUM_MODULES>,
// struct_ModuleData, NULL)`, replacing the old "timers" entry. <bits> must
// equal sizeof(ModuleData)*8 and the element count must equal NUM_MODULES,
// or the YAML (de)serializer will misalign every field after moduleData in
// any file that uses struct_PartialModel (readModelYaml/writeModelLabels).
TEST(PartialModel, ModuleDataNodeMatchesStructLayout)
{
  const YamlNode* root = get_partialmodel_nodes();
  const YamlNode* moduleData = findChild(root, "moduleData");
  ASSERT_NE(moduleData, nullptr);

  EXPECT_EQ(moduleData->elmts, NUM_MODULES);
  EXPECT_EQ(moduleData->size, sizeof(ModuleData) * 8);
}

// Same idea for the "header" entry: its declared bit size must equal
// sizeof(ModelHeader)*8, or moduleData (and thus every array element after
// it) parses starting at the wrong bit offset.
TEST(PartialModel, HeaderNodeMatchesStructLayout)
{
  const YamlNode* root = get_partialmodel_nodes();
  const YamlNode* header = findChild(root, "header");
  ASSERT_NE(header, nullptr);

  EXPECT_EQ(header->size, sizeof(ModelHeader) * 8);
}

// PartialModel must be layout-compatible with the start of ModelData, per
// the comment added at datastructs_private.h ModelData::header ("Must match
// start of PartialModel") -- readModelCell()/updateModelCell() rely on this
// so that data read into a PartialModel (a stack buffer) reflects the same
// bytes that would land in the corresponding fields of a full ModelData.
TEST(PartialModel, HeaderOffsetMatchesModelData)
{
  EXPECT_EQ(offsetof(PartialModel, header), offsetof(ModelData, header));
}

// End-to-end: parse the same YAML "header:" block once into a PartialModel
// and once into a full ModelData, and check they agree field-for-field.
// This is the strongest guard against the two generated tables silently
// drifting apart, because it doesn't hardcode any bit offsets itself -- it
// just requires both parsers to produce the same result for the same input.
TEST(PartialModel, HeaderParsesIdenticallyToModelData)
{
  static const char yaml[] =
      "header: \n"
      "   name: \"Tst Name\"\n"
      "   modelId: [3, 5]\n"
#if LEN_BITMAP_NAME > 0
      "   bitmap: \"pic.bmp\"\n"
#endif
      "   labels: \"alpha,bravo\"\n";

  PartialModel partial;
  memclear(&partial, sizeof(partial));
  {
    YamlTreeWalker tree;
    tree.reset(get_partialmodel_nodes(), (uint8_t*)&partial);
    YamlParser yp;
    yp.init(YamlTreeWalker::get_parser_calls(), &tree);
    yp.parse(yaml, sizeof(yaml) - 1);
  }

  ModelData model;
  memclear(&model, sizeof(model));
  {
    YamlTreeWalker tree;
    tree.reset(get_modeldata_nodes(), (uint8_t*)&model);
    YamlParser yp;
    yp.init(YamlTreeWalker::get_parser_calls(), &tree);
    yp.parse(yaml, sizeof(yaml) - 1);
  }

  EXPECT_STREQ(partial.header.name, model.header.name);
  EXPECT_EQ(0, memcmp(partial.header.modelId, model.header.modelId,
                       sizeof(model.header.modelId)));
#if LEN_BITMAP_NAME > 0
  EXPECT_STREQ(partial.header.bitmap, model.header.bitmap);
#endif
  EXPECT_STREQ(partial.header.labels, model.header.labels);

  // Sanity: the fixture actually populated something non-zero, otherwise a
  // parser that silently no-ops on both sides would pass trivially.
  EXPECT_STREQ("Tst Name", partial.header.name);
}

// Behavioural contract for ModelHeader::labels: it holds the CSV-joined
// list of *every* label attached to a model (declared as
// char[LABELS_LENGTH], currently 100 bytes) -- as distinct from the length
// of one individual label name (LABEL_LENGTH, 16 bytes). A model with
// several labels attached should keep all of them when this field gets
// (re)written, up to the field's own declared capacity, not just as much
// as would fit in a single label name.
TEST(PartialModel, LabelsFieldRetainsFullCsvUpToItsOwnCapacity)
{
  PartialModel partial;
  memclear(&partial, sizeof(partial));

  // A multi-label CSV comfortably inside LABELS_LENGTH, but longer than a
  // single LABEL_LENGTH-sized name -- i.e. more than one label attached.
  const char* csv = "alpha,bravo,charlie,delta,echo";
  ASSERT_GT(strlen(csv), (size_t)LABEL_LENGTH);
  ASSERT_LT(strlen(csv), (size_t)LABELS_LENGTH);

  strAppend(partial.header.labels, csv, LABELS_LENGTH - 1);

  EXPECT_STREQ(csv, partial.header.labels)
      << "header.labels should retain the full label list up to "
         "LABELS_LENGTH-1 characters, not just a single label name's "
         "worth.";
}
