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

// Custom-screen and topbar-widget layout data (CustomScreenData /
// TopBarPersistentData) is stored per-process in static globals rather than
// inside ModelData itself (see ModelData::getScreenData()/getTopbarData()),
// because that data can be large and only the currently loaded model's copy
// needs to be resident. Any code that reads or writes a *full* ModelData
// buffer -- as opposed to the header-only PartialModel -- for a model other
// than the one currently loaded therefore risks reading/writing those
// globals on behalf of the wrong model.
//
// Label management (renaming/adding/removing labels) routinely touches
// models other than the currently loaded one. These tests exercise that
// label-management path through its public ModelMap API and assert the
// currently loaded model's screen/topbar data is left alone, regardless of
// what label edits are made to other models on disk.

#include "gtests.h"
#include "location.h"

#include <filesystem>

#include "storage/modelslist.h"
#include "storage/sdcard_common.h"
#include "storage/sdcard_yaml.h"
#include "storage/yaml/yaml_datastructs.h"

#if defined(COLORLCD)

namespace fs = std::filesystem;

class ModelMapFsTest : public ::testing::Test
{
 protected:
  fs::path scratchDir;

  void SetUp() override
  {
    scratchDir = fs::temp_directory_path() /
                 fs::path("edgetx-gtest-modelslist-labels");
    std::error_code ec;
    fs::remove_all(scratchDir, ec);
    fs::create_directories(scratchDir / "MODELS", ec);
    ASSERT_FALSE(ec) << "could not create scratch MODELS directory";

    simuFatfsSetPaths(scratchDir.string().c_str(), nullptr);

    modelslist.clear();
    modelslabels.clear();
    memclear(&g_model, sizeof(g_model));
  }

  void TearDown() override
  {
    modelslist.clear();
    modelslabels.clear();

    simuFatfsSetPaths(TESTS_PATH, nullptr);

    std::error_code ec;
    fs::remove_all(scratchDir, ec);
  }

  // Writes a standalone model YAML file to <scratchDir>/MODELS/<filename>
  // with the given label list and screen/topbar marker values. This must be
  // called *before* the active model's own screen/topbar markers are set on
  // g_model: since getScreenData()/getTopbarData() are shared globals, using
  // them here to build this "other" model's fixture is itself writing
  // through the same storage g_model uses, exactly like the code under test
  // does. The test bodies set the active model's markers afterwards, which
  // simulates that model actually being loaded, and is what the assertions
  // check survives the label edit untouched.
  void writeFixtureModel(const char* filename, const char* labels,
                          const char* screenLayoutId, const char* widgetName)
  {
    ModelData model;
    memclear(&model, sizeof(model));
    // A real model always has a name; an entirely-default header (no name,
    // no labels, zeroed modelId) is omitted from the written YAML altogether
    // as a compaction optimisation, which would leave this fixture without
    // a "header:" section at all. Giving it a name keeps the fixture
    // realistic and avoids that unrelated edge case.
    strAppend(model.header.name, filename, LEN_MODEL_NAME);
    strAppend(model.header.labels, labels, LABELS_LENGTH - 1);
    model.getScreenData(0)->LayoutId = screenLayoutId;
    model.getTopbarData()->zones[0].widgetName = widgetName;

    char path[256];
    getModelPath(path, filename);
    ASSERT_EQ(writeFileYaml(path, get_modeldata_nodes(), (uint8_t*)&model, 0),
              (const char*)nullptr)
        << "failed writing fixture model " << filename;
  }
};

TEST_F(ModelMapFsTest, RenamingLabelOnOtherModelLeavesActiveScreenDataUntouched)
{
  // Seed a second, non-active model on disk with a label and screen/topbar
  // data distinct from the active model's.
  writeFixtureModel("model0002.yml", "Foo", "OtherLayout", "OtherWidget");
  ModelCell* other = modelslist.addModel("model0002.yml", false);
  ASSERT_NE(other, nullptr);
  ASSERT_FALSE(modelslabels.addLabelToModel("Foo", other, false));

  // Now "load" the active model: register its cell and mark it current, and
  // set its screen/topbar data to its own values.
  ModelCell* active = modelslist.addModel("model0001.yml", false);
  ASSERT_NE(active, nullptr);
  modelslist.setCurrentModel(active);
  g_model.getScreenData(0)->LayoutId = "ActiveLayout";
  g_model.getTopbarData()->zones[0].widgetName = "ActiveWidget";

  // Rename a label that only exists on the other, non-active model.
  modelslabels.renameLabel("Foo", "Bar");

  // The active model's screen/topbar data must be unaffected by editing an
  // unrelated model's labels.
  EXPECT_STREQ(g_model.getScreenData(0)->LayoutId.c_str(), "ActiveLayout");
  EXPECT_STREQ(g_model.getTopbarData()->zones[0].widgetName.c_str(),
               "ActiveWidget");

  // The other model's file on disk should reflect the renamed label. Read
  // back only the header (PartialModel), not a full ModelData, so this
  // verification step doesn't itself touch the shared screen/topbar globals.
  PartialModel partial;
  memclear(&partial, sizeof(partial));
  readModelYaml("model0002.yml", (uint8_t*)&partial, sizeof(PartialModel));
  EXPECT_STREQ(partial.header.labels, "Bar");
}

TEST_F(ModelMapFsTest,
       AddingLabelToOtherModelWithFileUpdateLeavesActiveScreenDataUntouched)
{
  writeFixtureModel("model0002.yml", "", "OtherLayout", "OtherWidget");
  ModelCell* other = modelslist.addModel("model0002.yml", false);
  ASSERT_NE(other, nullptr);

  ModelCell* active = modelslist.addModel("model0001.yml", false);
  ASSERT_NE(active, nullptr);
  modelslist.setCurrentModel(active);
  g_model.getScreenData(0)->LayoutId = "ActiveLayout";
  g_model.getTopbarData()->zones[0].widgetName = "ActiveWidget";

  // update=true drives ModelMap::updateModelFile(), the second call site
  // that reads/writes a non-active model's file on disk.
  EXPECT_FALSE(modelslabels.addLabelToModel("Baz", other, true));

  EXPECT_STREQ(g_model.getScreenData(0)->LayoutId.c_str(), "ActiveLayout");
  EXPECT_STREQ(g_model.getTopbarData()->zones[0].widgetName.c_str(),
               "ActiveWidget");

  PartialModel partial;
  memclear(&partial, sizeof(partial));
  readModelYaml("model0002.yml", (uint8_t*)&partial, sizeof(PartialModel));
  EXPECT_STREQ(partial.header.labels, "Baz");
}

#endif // defined(COLORLCD)
