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

// Behavioural tests for the ModelCell / ModelsList (modelCellManager) label
// and duplicate-name management: unique-name generation, label rename /
// delete / reorder, LabelsMap index bookkeeping, and labels.yml round trips.

#include "gtests.h"
#include "location.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "storage/modelslist.h"
#include "storage/sdcard_common.h"
#include "storage/sdcard_yaml.h"
#include "storage/storage.h"
#include "storage/yaml/yaml_datastructs.h"

#if defined(COLORLCD)

#include "dialog.h"
#include "lcd.h"
#include "mainwindow.h"
#include "view_main.h"

namespace fs = std::filesystem;

namespace
{

std::vector<std::string> sorted(LabelsVector v)
{
  std::sort(v.begin(), v.end());
  return v;
}

std::vector<std::string> sorted(std::initializer_list<const char*> l)
{
  std::vector<std::string> v(l.begin(), l.end());
  std::sort(v.begin(), v.end());
  return v;
}

std::vector<std::string> splitCSV(const char* csv)
{
  std::vector<std::string> out;
  std::string cur;
  for (const char* c = csv; *c; ++c) {
    if (*c == ',') {
      out.push_back(cur);
      cur.clear();
    } else {
      cur += *c;
    }
  }
  if (!cur.empty()) out.push_back(cur);
  return out;
}

std::string joinCSV(const std::vector<std::string>& v)
{
  std::string s;
  for (auto& e : v) {
    if (!s.empty()) s += ',';
    s += e;
  }
  return s;
}

int countOf(const LabelsVector& v, const std::string& s)
{
  return (int)std::count(v.begin(), v.end(), s);
}

// Split a model yaml file into its "header:" block and everything else
void splitModelYaml(const std::string& text, std::string& header,
                    std::string& body)
{
  std::istringstream in(text);
  std::string line;
  bool inHeader = false, done = false;
  while (std::getline(in, line)) {
    std::string bare = line;
    while (!bare.empty() && (bare.back() == '\r' || bare.back() == ' '))
      bare.pop_back();
    if (!done && bare == "header:") {
      inHeader = true;
      header += line + "\n";
      continue;
    }
    if (inHeader && !done) {
      if (!bare.empty() && bare[0] == ' ') {
        header += line + "\n";
        continue;
      }
      done = true;
    }
    body += line + "\n";
  }
}

}  // namespace

class ModelCellManagerFsTest : public ::testing::Test
{
 protected:
  fs::path scratchDir;

  void SetUp() override
  {
    scratchDir = fs::temp_directory_path() /
                 fs::path("edgetx-gtest-modelcellmanager");
    std::error_code ec;
    fs::remove_all(scratchDir, ec);
    fs::create_directories(scratchDir / "MODELS", ec);
    ASSERT_FALSE(ec) << "could not create scratch MODELS directory";

    simuFatfsSetPaths(scratchDir.string().c_str(), nullptr);

    modelCellManager.clear();
    modelCellManager.clearFilter();
    modelCellManager.setSortOrder(DEFAULT_MODEL_SORT);
    memclear(&g_model, sizeof(g_model));
    memclear(g_eeGeneral.currModelFilename, sizeof(g_eeGeneral.currModelFilename));
    storageDirtyMsk = 0;
  }

  void TearDown() override
  {
    modelCellManager.clear();
    modelCellManager.clearFilter();
    modelCellManager.setSortOrder(DEFAULT_MODEL_SORT);
    storageDirtyMsk = 0;

    simuFatfsSetPaths(TESTS_PATH, nullptr);

    std::error_code ec;
    fs::remove_all(scratchDir, ec);
  }

  // ---- helpers -----------------------------------------------------------

  static std::string realPath(const char* file)
  {
    return simuFatfsGetRealPath(std::string(MODELS_PATH) + "/" + file);
  }

  static void setCurrentFilename(const char* file)
  {
    strAppend(g_eeGeneral.currModelFilename, file, LEN_MODEL_FILENAME);
  }

  // Add an in-memory cell (no file on disk needed).
  ModelCell* addCell(const char* file, const char* name,
                     std::initializer_list<const char*> labels = {})
  {
    ModelCell* cell = modelCellManager.addModel(file, false);
    cell->setModelName(name);
    for (auto l : labels) modelCellManager.addLabelToModel(l, cell, false);
    return cell;
  }

  // Write a model yaml file to MODELS/<file>.
  void writeFixtureModel(const char* file, const char* name,
                         const char* labels)
  {
    ModelData model;
    memclear(&model, sizeof(model));
    strAppend(model.header.name, name, LEN_MODEL_NAME);
    strAppend(model.header.labels, labels, LABELS_LENGTH);

    char path[256];
    getModelPath(path, file);
    ASSERT_EQ(writeFileYaml(path, get_modeldata_nodes(), (uint8_t*)&model, 0),
              (const char*)nullptr)
        << "failed writing fixture model " << file;
  }

  static PartialModel readHeader(const char* file)
  {
    PartialModel partial;
    memclear(&partial, sizeof(partial));
    readModelYaml(file, (uint8_t*)&partial, sizeof(PartialModel));
    return partial;
  }

  static std::string readFile(const char* file)
  {
    std::ifstream in(realPath(file), std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
  }

  static std::string bodyOf(const char* file)
  {
    std::string h, b;
    splitModelYaml(readFile(file), h, b);
    return b;
  }

  static LabelsVector labelsOf(const char* file)
  {
    ModelCell* c = modelCellManager.getModel(file);
    return c ? c->getLabels() : LabelsVector();
  }

  // Register labels "L0".."L<n-1>" in the manager (index == number)
  static void registerLabels(int n)
  {
    modelCellManager.clear();
    for (int i = 0; i < n; i++)
      modelCellManager.addLabel("L" + std::to_string(i));
  }

  static std::string fileName(int n)
  {
    char fn[32];
    snprintf(fn, sizeof(fn), "model%04d.yml", n);
    return fn;
  }

  // Duplicate a cell `copies` times the way the model page does and check
  // every generated name is unique and fits.
  void checkDuplicateNames(size_t len, int copies)
  {
    std::string name = std::string("ABCDEFGHIJKLMNO").substr(0, len);
    ModelCell* src = addCell("model0001.yml", name.c_str());
    ASSERT_STREQ(src->modelName, name.c_str());

    for (int n = 1; n <= copies; n++) {
      std::string fn = fileName(n + 1);
      ModelCell* dup = modelCellManager.addModel(fn.c_str(), false, src);
      dup->setUniqueName();

      size_t l = strlen(dup->modelName);
      EXPECT_LE(l, (size_t)LEN_MODEL_NAME) << "len " << len << " copy " << n;
      EXPECT_GT(l, 0u);
      EXPECT_EQ(dup->modelName[l - 1], ')')
          << "len " << len << " copy " << n << " -> " << dup->modelName;

      int same = 0;
      for (auto* m : modelCellManager)
        if (strcmp(m->modelName, dup->modelName) == 0) same++;
      EXPECT_EQ(same, 1) << "duplicate name " << dup->modelName;
    }
  }
};

// ---------------------------------------------------------------------------
// Default / unique names
// ---------------------------------------------------------------------------

TEST(ModelCellName, DefaultNameFromNumericFilename)
{
  ModelCell a("model0003.yml");
  a.setModelName("");
  EXPECT_STREQ(a.modelName, "MODEL03");

  ModelCell b("model12.yml");
  b.setModelName(nullptr);
  EXPECT_STREQ(b.modelName, "MODEL12");

  ModelCell c("custom.yml");
  c.setModelName("");
  EXPECT_STREQ(c.modelName, "custom");

  ModelCell d("model0003.yml");
  d.setModelName("Named");
  EXPECT_STREQ(d.modelName, "Named");
}

TEST_F(ModelCellManagerFsTest, UniqueNameFirstDuplicate)
{
  ModelCell* src = addCell("model0001.yml", "Alpha");
  ModelCell* dup = modelCellManager.addModel("model0002.yml", false, src);
  EXPECT_STREQ(dup->modelName, "Alpha");  // copied verbatim until made unique
  dup->setUniqueName();
  EXPECT_STREQ(dup->modelName, "Alpha(1)");
  EXPECT_STREQ(src->modelName, "Alpha");
}

TEST_F(ModelCellManagerFsTest, UniqueNameSkipsTakenSuffixes)
{
  ModelCell* src = addCell("model0001.yml", "Alpha");
  addCell("model0002.yml", "Alpha(1)");
  addCell("model0003.yml", "Alpha(2)");
  ModelCell* dup = modelCellManager.addModel("model0004.yml", false, src);
  dup->setUniqueName();
  EXPECT_STREQ(dup->modelName, "Alpha(3)");
}

TEST_F(ModelCellManagerFsTest, UniqueNameShortNameStaysWithinBounds)
{
  checkDuplicateNames(1, 12);
}

TEST_F(ModelCellManagerFsTest, UniqueNameElevenCharNameStaysWithinBounds)
{
  checkDuplicateNames(11, 12);
}

// A name of exactly LEN_MODEL_NAME - 3 characters is not truncated, so from
// the 10th duplicate on "(10)" no longer fits. The overflow lands inside the
// ModelCell / stack frame, so it shows up as a 16 character name here rather
// than (necessarily) as an ASan report.
TEST_F(ModelCellManagerFsTest, UniqueNameTwelveCharNameTenthDuplicate)
{
  checkDuplicateNames(12, 12);
}

TEST_F(ModelCellManagerFsTest, UniqueNameThirteenCharNameStaysWithinBounds)
{
  checkDuplicateNames(13, 12);
}

TEST_F(ModelCellManagerFsTest, UniqueNameFullLengthNameStaysWithinBounds)
{
  checkDuplicateNames(LEN_MODEL_NAME, 12);
}

// ---------------------------------------------------------------------------
// LabelsMap bookkeeping
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, LabelsMapInsertsInSortedOrderWithoutDuplicates)
{
  registerLabels(5);
  LabelsMap m;
  m.addLabel(3);
  m.addLabel(1);
  m.addLabel(2);
  m.addLabel(2);
  EXPECT_EQ(m.getLabels(), (LabelsVector{"L1", "L2", "L3"}));
  EXPECT_TRUE(m.hasLabel(2));
  EXPECT_FALSE(m.hasLabel(0));
  EXPECT_FALSE(m.hasLabel(255));
}

TEST_F(ModelCellManagerFsTest, LabelsMapRemoveOnlyRemovesThatLabel)
{
  registerLabels(5);
  LabelsMap m;
  m.addLabel(1);
  m.addLabel(3);
  m.removeLabel(2);  // not present: no-op
  EXPECT_EQ(m.getLabels(), (LabelsVector{"L1", "L3"}));
  m.removeLabel(1);
  EXPECT_EQ(m.getLabels(), (LabelsVector{"L3"}));
}

TEST_F(ModelCellManagerFsTest, LabelsMapDeleteShiftsHigherIndexes)
{
  registerLabels(5);
  LabelsMap m;
  m.addLabel(1);
  m.addLabel(3);

  EXPECT_FALSE(m.deleteLabel(2));  // not present, but 3 shifts down to 2
  EXPECT_EQ(m.getLabels(), (LabelsVector{"L1", "L2"}));

  EXPECT_TRUE(m.deleteLabel(1));  // present: removed, nothing above left to shift but 2->1
  EXPECT_EQ(m.getLabels(), (LabelsVector{"L1"}));
}

TEST_F(ModelCellManagerFsTest, LabelsMapMoveUpSwapsMembership)
{
  registerLabels(4);
  LabelsMap both, first, second, neither;
  both.addLabel(1); both.addLabel(2);
  first.addLabel(1);
  second.addLabel(2);

  both.moveLabelUp(2);
  EXPECT_EQ(both.getLabels(), (LabelsVector{"L1", "L2"}));

  first.moveLabelUp(2);  // 1 -> 2
  EXPECT_EQ(first.getLabels(), (LabelsVector{"L2"}));

  second.moveLabelUp(2);  // 2 -> 1
  EXPECT_EQ(second.getLabels(), (LabelsVector{"L1"}));

  neither.moveLabelUp(2);
  EXPECT_TRUE(neither.getLabels().empty());

  second.moveLabelUp(0);  // top label can't move
  EXPECT_EQ(second.getLabels(), (LabelsVector{"L1"}));
}

// ---------------------------------------------------------------------------
// addLabelToModel edge cases
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, RejectedLabelDoesNotMakeModelLabelled)
{
  ModelCell* cell = addCell("model0001.yml", "One");

  // "Unlabeled" is reserved and a label made only of excluded characters is
  // empty once sanitised: addLabel() refuses both, so nothing may be attached.
  modelCellManager.addLabelToModel(STR_UNLABELEDMODEL, cell, false);
  modelCellManager.addLabelToModel("\"", cell, false);

  EXPECT_TRUE(cell->getLabels().empty());
  auto unlabeled = modelCellManager.getUnlabeledModels();
  EXPECT_EQ(unlabeled.size(), 1u);
}

TEST_F(ModelCellManagerFsTest, LabelsFieldLengthLimitIsEnforced)
{
  ModelCell* cell = addCell("model0001.yml", "One");
  int accepted = 0;
  for (int i = 0; i < 20; i++) {
    std::string lbl = std::string(15, 'A' + i);
    if (!modelCellManager.addLabelToModel(lbl, cell, false)) accepted++;
  }
  EXPECT_GT(accepted, 0);
  EXPECT_LT(accepted, 20);
  std::string csv = joinCSV(cell->getLabels());
  EXPECT_LE(csv.size(), (size_t)LABELS_LENGTH);
  EXPECT_EQ(cell->getLabels().size(), (size_t)accepted);
}

// ---------------------------------------------------------------------------
// renameLabel
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, RenameLabelPatchesOtherModelHeaderKeepsBody)
{
  writeFixtureModel("model0001.yml", "One", "Keep");
  writeFixtureModel("model0002.yml", "Two", "Foo,Keep");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  ASSERT_EQ(sorted(labelsOf("model0002.yml")), sorted({"Foo", "Keep"}));
  std::string bodyBefore = bodyOf("model0002.yml");
  ASSERT_FALSE(bodyBefore.empty());

  modelCellManager.renameLabel("Foo", "Bar");

  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"Bar", "Keep"}));
  PartialModel hdr = readHeader("model0002.yml");
  EXPECT_EQ(sorted(splitCSV(hdr.header.labels)), sorted({"Bar", "Keep"}));
  EXPECT_STREQ(hdr.header.name, "Two");
  EXPECT_EQ(bodyOf("model0002.yml"), bodyBefore);
  EXPECT_EQ(modelCellManager.getIndexByLabel("Foo"), -1);
  EXPECT_GE(modelCellManager.getIndexByLabel("Bar"), 0);
}

TEST_F(ModelCellManagerFsTest, RenameLabelOnCurrentModelUpdatesRamNotFile)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(modelCellManager.getCurrentModel(),
            modelCellManager.getModel("model0001.yml"));
  storageDirtyMsk = 0;

  modelCellManager.renameLabel("Foo", "Bar");

  EXPECT_STREQ(g_model.header.labels, "Bar");
  EXPECT_TRUE(storageDirtyMsk & EE_MODEL);
  // The file is only rewritten by the normal storage flush
  EXPECT_STREQ(readHeader("model0001.yml").header.labels, "Foo");
}

TEST_F(ModelCellManagerFsTest, RenameLabelToSameNameKeepsSingleLabel)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  modelCellManager.renameLabel("Foo", "Foo");

  EXPECT_EQ(countOf(modelCellManager.getLabels(), "Foo"), 1);
  EXPECT_EQ(labelsOf("model0001.yml"), (LabelsVector{"Foo"}));
}

TEST_F(ModelCellManagerFsTest, RenameLabelOntoExistingLabelMerges)
{
  writeFixtureModel("model0001.yml", "One", "Bar");
  writeFixtureModel("model0002.yml", "Two", "Foo,Bar");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  modelCellManager.renameLabel("Foo", "Bar");

  EXPECT_EQ(countOf(modelCellManager.getLabels(), "Bar"), 1)
      << "label list now has duplicate entries";
  EXPECT_EQ(countOf(labelsOf("model0002.yml"), "Bar"), 1);
  EXPECT_EQ(splitCSV(readHeader("model0002.yml").header.labels),
            (std::vector<std::string>{"Bar"}));
}

TEST_F(ModelCellManagerFsTest, RenameLabelRejectsYamlUnsafeCharacters)
{
  writeFixtureModel("model0001.yml", "One", "");
  writeFixtureModel("model0002.yml", "Two", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  modelCellManager.renameLabel("Foo", "a\"b:c'd-e\\f");

  for (const auto& lbl : modelCellManager.getLabels())
    for (const char* c = labelExcludedChars; *c; ++c)
      EXPECT_EQ(lbl.find(*c), std::string::npos)
          << "label '" << lbl << "' contains excluded char '" << *c << "'";

  PartialModel hdr = readHeader("model0002.yml");
  std::string csv = hdr.header.labels;
  for (const char* c = labelExcludedChars; *c; ++c)
    EXPECT_EQ(csv.find(*c), std::string::npos)
        << "model header labels '" << csv << "' contain '" << *c << "'";
}

TEST_F(ModelCellManagerFsTest, RenameLabelTruncatesToLabelLength)
{
  writeFixtureModel("model0001.yml", "One", "");
  writeFixtureModel("model0002.yml", "Two", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  modelCellManager.renameLabel("Foo", std::string(40, 'x'));

  for (const auto& lbl : modelCellManager.getLabels())
    EXPECT_LE(lbl.size(), (size_t)LABEL_LENGTH) << "label '" << lbl << "'";
}

// Renaming must never leave a model's label field holding a partial label
// (the CSV is capped at LABELS_LENGTH characters).
TEST_F(ModelCellManagerFsTest, RenameLabelNeverWritesPartialLabels)
{
  std::vector<std::string> many;
  for (int i = 0; i < 6; i++) many.push_back(std::string(15, 'A' + i));

  writeFixtureModel("model0001.yml", "One", "");
  writeFixtureModel("model0002.yml", "Two", joinCSV(many).c_str());
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(labelsOf("model0002.yml").size(), 6u);

  modelCellManager.renameLabel(many[0], std::string(30, 'Z'));

  auto known = modelCellManager.getLabels();
  for (const auto& tok : splitCSV(readHeader("model0002.yml").header.labels))
    EXPECT_EQ(countOf(known, tok), 1)
        << "header holds '" << tok << "' which is not a known label";
}

TEST_F(ModelCellManagerFsTest, RenameLabelReportsProgressToCompletion)
{
  writeFixtureModel("model0001.yml", "One", "");
  writeFixtureModel("model0002.yml", "Two", "Foo");
  writeFixtureModel("model0003.yml", "Three", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  int calls = 0, last = -1;
  modelCellManager.renameLabel("Foo", "Bar", [&](const char*, int pct) {
    calls++;
    last = pct;
  });
  EXPECT_GT(calls, 0);
  EXPECT_EQ(last, 100);
}

// Callers close the progress dialog at 100%
TEST_F(ModelCellManagerFsTest, RenameLabelOntoExistingLabelProgressEndsAt100)
{
  writeFixtureModel("model0001.yml", "One", "Bar");
  writeFixtureModel("model0002.yml", "Two", "Foo,Bar");
  writeFixtureModel("model0003.yml", "Three", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  std::vector<int> reported;
  modelCellManager.renameLabel("Foo", "Bar", [&](const char*, int pct) {
    reported.push_back(pct);
  });

  auto done = std::find(reported.begin(), reported.end(), 100);
  ASSERT_NE(done, reported.end());
  for (auto it = done; it != reported.end(); ++it)
    EXPECT_EQ(*it, 100) << "progress reported again after reaching 100%";
}

static lv_obj_t* findProgressBar(lv_obj_t* obj)
{
  if (lv_obj_has_class(obj, &lv_bar_class)) return obj;
  for (uint32_t i = 0; i < lv_obj_get_child_cnt(obj); i++) {
    auto bar = findProgressBar(lv_obj_get_child(obj, i));
    if (bar) return bar;
  }
  return nullptr;
}

// Callback as used in model_select.cpp. LVGL memory is not visible to ASAN,
// so check the progress bar is still valid before each update.
TEST_F(ModelCellManagerFsTest, RenameLabelOntoExistingLabelWithProgressDialog)
{
  writeFixtureModel("model0001.yml", "One", "Bar");
  writeFixtureModel("model0002.yml", "Two", "Foo,Bar");
  writeFixtureModel("model0003.yml", "Three", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  // ProgressDialog forces a refresh, no simulator front end to complete it
  lcdSetFlushCb([](lv_disp_drv_t* disp, uint16_t*, const rect_t&) {
    lv_disp_flush_ready(disp);
  });

  // Dialogs are always shown over the main view
  ViewMain::instance();
  MainWindow::instance()->run();

  auto rndialog = new ProgressDialog(STR_RENAME_LABEL, [=]() {});
  lv_obj_t* bar = findProgressBar(rndialog->getLvObj());
  ASSERT_NE(bar, nullptr);

  modelCellManager.renameLabel(
      "Foo", "Bar", [=](const char *name, int percentage) {
        if (!lv_obj_is_valid(bar)) {
          // 100% again is a no-op, anything else writes to the freed bar
          EXPECT_EQ(percentage, 100)
              << percentage << "% progress reported after the dialog was"
              << " closed and its progress bar freed";
          return;
        }

        rndialog->setTitle(std::string(STR_RENAME_LABEL) + " " + name);
        rndialog->updateProgress(percentage);
        if (percentage >= 100) rndialog->closeDialog();
      });

  MainWindow::instance()->run();
  EXPECT_EQ(countOf(modelCellManager.getLabels(), "Bar"), 1);

  // Restore simulator LCD callbacks
  lcdInit();
}

// ---------------------------------------------------------------------------
// removeLabel
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, RemoveLabelShiftsIndexesAcrossSaveAndReload)
{
  writeFixtureModel("model0001.yml", "One", "A,C");
  writeFixtureModel("model0002.yml", "Two", "B,D");
  writeFixtureModel("model0003.yml", "Three", "C,D");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(sorted(modelCellManager.getLabels()), sorted({"A", "B", "C", "D"}));

  modelCellManager.removeLabel("B");

  EXPECT_EQ(sorted(modelCellManager.getLabels()), sorted({"A", "C", "D"}));
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"A", "C"}));
  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"D"}));
  EXPECT_EQ(sorted(labelsOf("model0003.yml")), sorted({"C", "D"}));
  EXPECT_STREQ(readHeader("model0002.yml").header.labels, "D");

  // Persist and reload: nothing may drift after the index shift.
  ASSERT_EQ(modelCellManager.save(), (const char*)nullptr);
  modelCellManager.clear();
  modelCellManager.load();

  EXPECT_EQ(sorted(modelCellManager.getLabels()), sorted({"A", "C", "D"}));
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"A", "C"}));
  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"D"}));
  EXPECT_EQ(sorted(labelsOf("model0003.yml")), sorted({"C", "D"}));
}

TEST_F(ModelCellManagerFsTest, RemoveLastLabelAddsFavorites)
{
  writeFixtureModel("model0001.yml", "One", "Only");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(modelCellManager.getLabels(), (LabelsVector{"Only"}));

  modelCellManager.removeLabel("Only");

  EXPECT_EQ(modelCellManager.getLabels(), (LabelsVector{STR_FAVORITE_LABEL}));
  EXPECT_TRUE(labelsOf("model0001.yml").empty());
}

TEST_F(ModelCellManagerFsTest, RemoveLabelUsedByCurrentModelDirtiesModel)
{
  writeFixtureModel("model0001.yml", "One", "Foo,Keep");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  storageDirtyMsk = 0;

  modelCellManager.removeLabel("Foo");

  EXPECT_EQ(labelsOf("model0001.yml"), (LabelsVector{"Keep"}));
  EXPECT_STREQ(g_model.header.labels, "Keep");
  EXPECT_TRUE(storageDirtyMsk & EE_MODEL);
  EXPECT_TRUE(storageDirtyMsk & EE_LABELS);
}

TEST_F(ModelCellManagerFsTest, RemoveLabelReportsProgressToCompletion)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  writeFixtureModel("model0002.yml", "Two", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  int calls = 0, last = -1;
  modelCellManager.removeLabel("Foo", [&](const char*, int pct) {
    calls++;
    last = pct;
  });
  EXPECT_GT(calls, 0);
  EXPECT_EQ(last, 100);
}

TEST_F(ModelCellManagerFsTest, RemoveUnknownLabelIsNoop)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  modelCellManager.removeLabel("Nope");

  EXPECT_EQ(modelCellManager.getLabels(), (LabelsVector{"Foo"}));
  EXPECT_EQ(labelsOf("model0001.yml"), (LabelsVector{"Foo"}));
}

// ---------------------------------------------------------------------------
// moveLabelUp
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, MoveLabelUpKeepsMembershipAndSurvivesReload)
{
  writeFixtureModel("model0001.yml", "One", "A");
  writeFixtureModel("model0002.yml", "Two", "B");
  writeFixtureModel("model0003.yml", "Three", "A,C");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  // Label order depends on directory scan order, so swap whichever two
  // labels ended up at positions 0 and 1.
  auto order = modelCellManager.getLabels();
  ASSERT_EQ(order.size(), 3u);

  modelCellManager.moveLabelUp(1);

  EXPECT_EQ(modelCellManager.getIndexByLabel(order[0]), 1);
  EXPECT_EQ(modelCellManager.getIndexByLabel(order[1]), 0);
  EXPECT_EQ(modelCellManager.getIndexByLabel(order[2]), 2);
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"A"}));
  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"B"}));
  EXPECT_EQ(sorted(labelsOf("model0003.yml")), sorted({"A", "C"}));

  LabelsVector expectedOrder = modelCellManager.getLabels();
  ASSERT_EQ(modelCellManager.save(), (const char*)nullptr);
  modelCellManager.clear();
  modelCellManager.load();

  EXPECT_EQ(modelCellManager.getLabels(), expectedOrder);
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"A"}));
  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"B"}));
  EXPECT_EQ(sorted(labelsOf("model0003.yml")), sorted({"A", "C"}));
}

TEST_F(ModelCellManagerFsTest, MoveLabelUpOutOfRangeIsNoop)
{
  writeFixtureModel("model0001.yml", "One", "A,B");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  LabelsVector before = modelCellManager.getLabels();

  modelCellManager.moveLabelUp(0);
  EXPECT_EQ(modelCellManager.getLabels(), before);
  modelCellManager.moveLabelUp(200);
  EXPECT_EQ(modelCellManager.getLabels(), before);
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"A", "B"}));
}

// ---------------------------------------------------------------------------
// Duplicating a model
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, DuplicateCopiesLabelsButNotFilenameOrHash)
{
  writeFixtureModel("model0001.yml", "Alpha", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ModelCell* src = modelCellManager.getModel("model0001.yml");
  ASSERT_NE(src, nullptr);
  ASSERT_GT(strlen(src->modelFinfoHash), 0u);

  ModelCell* dup = modelCellManager.addModel("model0002.yml", false, src);

  EXPECT_STREQ(dup->modelFilename, "model0002.yml");
  EXPECT_STRNE(dup->modelFinfoHash, src->modelFinfoHash);
  EXPECT_EQ(dup->getLabels(), src->getLabels());
  EXPECT_TRUE(dup->hasLabel("Foo"));
  EXPECT_NE(dup, src);
}

// The real "duplicate" sequence: copy the file, add the cell, make the name
// unique and patch the header of the copy.
TEST_F(ModelCellManagerFsTest, DuplicateWritesUniqueNameToFilePreservingBody)
{
  writeFixtureModel("model0001.yml", "Alpha", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ModelCell* src = modelCellManager.getModel("model0001.yml");
  ASSERT_NE(src, nullptr);

  fs::copy_file(realPath("model0001.yml"), realPath("model0003.yml"));
  ModelCell* dup = modelCellManager.addModel("model0003.yml", true, src);
  dup->setUniqueName();
  dup->updateModelFile();

  EXPECT_STREQ(dup->modelName, "Alpha(1)");

  PartialModel dupHdr = readHeader("model0003.yml");
  EXPECT_STREQ(dupHdr.header.name, "Alpha(1)");
  EXPECT_STREQ(dupHdr.header.labels, "Foo");

  // Source untouched, and the body of the copy is byte-identical
  EXPECT_STREQ(readHeader("model0001.yml").header.name, "Alpha");
  std::string body = bodyOf("model0001.yml");
  ASSERT_FALSE(body.empty());
  EXPECT_EQ(bodyOf("model0003.yml"), body);
}

TEST_F(ModelCellManagerFsTest, DuplicateOfCurrentModelDoesNotTouchCurrentName)
{
  writeFixtureModel("model0001.yml", "Alpha", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ModelCell* src = modelCellManager.getCurrentModel();
  ASSERT_NE(src, nullptr);

  fs::copy_file(realPath("model0001.yml"), realPath("model0002.yml"));
  ModelCell* dup = modelCellManager.addModel("model0002.yml", true, src);
  dup->setUniqueName();
  dup->updateModelFile();

  EXPECT_EQ(modelCellManager.getCurrentModel(), src);
  EXPECT_STREQ(src->modelName, "Alpha");
  EXPECT_STREQ(readHeader("model0002.yml").header.name, "Alpha(1)");
}

// ---------------------------------------------------------------------------
// labels.yml load / save
// ---------------------------------------------------------------------------

TEST_F(ModelCellManagerFsTest, NoLabelsAnywhereCreatesFavoritesLabel)
{
  writeFixtureModel("model0001.yml", "One", "");
  writeFixtureModel("model0002.yml", "Two", "");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  EXPECT_EQ(modelCellManager.getLabels(), (LabelsVector{STR_FAVORITE_LABEL}));
  EXPECT_EQ(modelCellManager.getUnlabeledModels().size(), 2u);
}

TEST_F(ModelCellManagerFsTest, LoadBuildsCellsFromModelFilesWithoutLabelsYml)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  writeFixtureModel("model0002.yml", "Two", "Foo,Bar");
  setCurrentFilename("model0002.yml");
  modelCellManager.load();

  ASSERT_EQ(modelCellManager.size(), 2u);
  EXPECT_STREQ(modelCellManager.getModel("model0001.yml")->modelName, "One");
  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"Foo", "Bar"}));
  EXPECT_EQ(modelCellManager.getCurrentModel(),
            modelCellManager.getModel("model0002.yml"));
}

TEST_F(ModelCellManagerFsTest, ChangedModelFileReplacesLabelsRatherThanMerging)
{
  writeFixtureModel("model0001.yml", "One", "Foo,Bar");
  writeFixtureModel("model0002.yml", "Two", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(modelCellManager.save(), (const char*)nullptr);

  // Edit a model file behind the radio's back (e.g. Companion). The new
  // content is longer so the size/date hash certainly changes.
  writeFixtureModel("model0002.yml", "Two", "Bar,Baz");

  modelCellManager.clear();
  modelCellManager.load();

  EXPECT_EQ(sorted(labelsOf("model0002.yml")), sorted({"Bar", "Baz"}))
      << "stale label from labels.yml survived a model file change";
  EXPECT_EQ(sorted(labelsOf("model0001.yml")), sorted({"Foo", "Bar"}));
}

TEST_F(ModelCellManagerFsTest, LabelsYmlEntryForMissingModelFileIsIgnored)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();
  ASSERT_EQ(modelCellManager.save(), (const char*)nullptr);

  {
    std::ofstream out(realPath(LABELS_FILENAME), std::ios::app);
    out << "  model0099.yml:\r\n"
        << "    hash: \"0000000000000000\"\r\n"
        << "    name: \"Ghost\"\r\n"
        << "    labels: \"Foo,Ghostly\"\r\n";
  }

  modelCellManager.clear();
  modelCellManager.load();

  EXPECT_EQ(modelCellManager.size(), 1u);
  EXPECT_EQ(modelCellManager.getModel("model0099.yml"), nullptr);
  EXPECT_EQ(labelsOf("model0001.yml"), (LabelsVector{"Foo"}));
}

TEST_F(ModelCellManagerFsTest, SaveLoadRoundTripKeepsSortOrderAndFilter)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  writeFixtureModel("model0002.yml", "Two", "Bar");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  int bar = modelCellManager.getIndexByLabel("Bar");
  ASSERT_GE(bar, 0);
  modelCellManager.setSortOrder(DATE_DES);
  modelCellManager.setFilteredLabels({(uint32_t)bar});
  ASSERT_EQ(modelCellManager.save(), (const char*)nullptr);

  // Change the in-memory state so only a real reload can restore it
  modelCellManager.setSortOrder(NAME_DES);
  modelCellManager.clearFilter();
  modelCellManager.clear();
  modelCellManager.load();

  EXPECT_EQ(modelCellManager.sortOrder(), DATE_DES);
  EXPECT_TRUE(modelCellManager.isLabelFiltered("Bar"));
  EXPECT_FALSE(modelCellManager.isLabelFiltered("Foo"));
}

TEST_F(ModelCellManagerFsTest, FilteredModelListHonoursLabelsAndUnlabeled)
{
  writeFixtureModel("model0001.yml", "One", "Foo");
  writeFixtureModel("model0002.yml", "Two", "Foo,Bar");
  writeFixtureModel("model0003.yml", "Three", "");
  setCurrentFilename("model0001.yml");
  modelCellManager.load();

  g_eeGeneral.labelMultiMode = 0;  // AND
  auto both = modelCellManager.getFilteredModelList({"Foo", "Bar"});
  ASSERT_EQ(both.size(), 1u);
  EXPECT_STREQ(both[0]->modelName, "Two");

  g_eeGeneral.labelMultiMode = 1;  // OR
  auto either = modelCellManager.getFilteredModelList({"Foo", "Bar"});
  EXPECT_EQ(either.size(), 2u);

  auto none = modelCellManager.getFilteredModelList({STR_UNLABELEDMODEL});
  ASSERT_EQ(none.size(), 1u);
  EXPECT_STREQ(none[0]->modelName, "Three");
}

TEST_F(ModelCellManagerFsTest, SortModelsByNameAndDate)
{
  ModelCell* b = addCell("model0001.yml", "bravo");
  ModelCell* a = addCell("model0002.yml", "Alpha");
  ModelCell* c = addCell("model0003.yml", "Charlie");
  a->lastOpened = 30;
  b->lastOpened = 20;
  c->lastOpened = 10;

  modelCellManager.setSortOrder(NAME_ASC);
  auto v = modelCellManager.getAllModels();
  EXPECT_EQ(v, (ModelsVector{a, b, c}));  // case-insensitive

  modelCellManager.setSortOrder(NAME_DES);
  v = modelCellManager.getAllModels();
  EXPECT_EQ(v, (ModelsVector{c, b, a}));

  modelCellManager.setSortOrder(DATE_DES);
  v = modelCellManager.getAllModels();
  EXPECT_EQ(v, (ModelsVector{a, b, c}));

  modelCellManager.setSortOrder(DATE_ASC);
  v = modelCellManager.getAllModels();
  EXPECT_EQ(v, (ModelsVector{c, b, a}));
}

#endif  // defined(COLORLCD)
