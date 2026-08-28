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

#pragma once

#include <algorithm>
#include <functional>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "sdcard.h"

#include "rtc.h"

#define DEFAULT_MODEL_SORT NAME_ASC

// Characters not (currently) allowed in labels
constexpr const char* labelExcludedChars = "\\\":'-";

struct ModelHeader;
struct ModuleData;

typedef enum {
  NO_SORT,
  NAME_ASC,
  NAME_DES,
  DATE_ASC,
  DATE_DES,
  SORT_COUNT
} ModelsSortBy;

struct SimpleModuleData {
  uint8_t type = 0;
  uint8_t subType = 0;
};

// Must match start of the FILINFO struct
typedef struct PACKED {
  FSIZE_t fsize; /* File size */
  WORD fdate;    /* Modified date */
  WORD ftime;    /* Modified time */
} FInfoH;

#define FILE_HASH_LENGTH (sizeof(FInfoH) * 2)  // Hex string output

typedef std::vector<std::string> LabelsVector;

struct LabelsMap
{
 public:
  LabelsMap() {}

  void addLabel(uint8_t idx);
  void removeLabel(uint8_t idx);
  bool deleteLabel(uint8_t idx);
  void moveLabelUp(uint8_t idx);
  bool hasLabel(uint8_t idx);
  LabelsVector getLabels();

 protected:
  std::vector<uint16_t> labelMap;
};

class ModelCell
{
 public:
  ModelCell(const char *fileName);

  char modelFilename[LEN_MODEL_FILENAME + 1] = "";
  char modelName[LEN_MODEL_NAME + 1] = "";
  char modelFinfoHash[FILE_HASH_LENGTH + 1] = "";
#if LEN_BITMAP_NAME > 0
  char modelBitmap[LEN_BITMAP_NAME + 1] = "";
#endif
  gtime_t lastOpened = 0;
  bool _isDirty = true;
  bool valid_rfData = false;
  uint8_t modelId[NUM_MODULES] = {0, 0};
  SimpleModuleData moduleData[NUM_MODULES];

  void setModelName(const char *name);
  void setRfData(ModelHeader *header, ModuleData* moduleData);

  void addLabel(uint8_t idx);
  void removeLabel(uint8_t idx);
  void deleteLabel(uint8_t idx);
  void moveLabelUp(uint8_t idx);
  bool hasLabel(uint8_t idx) { return labelsMap.hasLabel(idx); }
  bool hasLabel(const std::string& label);

  void updateModelFile();
  void updateModelCell();
  void updateFinfoHash();

  void duplicateFrom(ModelCell* src);

  bool writeModelLabels(const char*);

  LabelsVector getLabels() { return labelsMap.getLabels(); }

  std::string getBulletLabelString(const char *noresults = "");

 protected:
  LabelsMap labelsMap;
};

typedef std::vector<ModelCell *> ModelsVector;

class ModelsList : public ModelsVector
{
 public:
  ModelsList();
  ~ModelsList();

  bool load();
  const char *save();
  void clear();

  void setCurrentModel(ModelCell *cell);
  void updateCurrentModelCell();

  ModelCell* getCurrentModel() const { return currentModel; }
  ModelCell* getModel(const char* filename);

  ModelCell *addModel(const char *name, bool save = true, ModelCell *copyCell = nullptr);
  bool removeModel(ModelCell *model);

  bool isModelIdUnique(uint8_t moduleIdx, char *warn_buf, size_t warn_buf_len);
  uint8_t findNextUnusedModelId(uint8_t moduleIdx);

  void setSortOrder(ModelsSortBy sortby);
  ModelsSortBy sortOrder() {return _sortOrder;}

  void sortModelsBy(ModelsVector &mv, ModelsSortBy sortby);

  ModelsVector getUnlabeledModels();
  ModelsVector getAllModels();
  ModelsVector getFilteredModelList(const LabelsVector &lbls);

  int getIndexByLabel(const std::string &str);
  std::string getLabelByIndex(uint16_t index);
  const LabelsVector& getLabels() const { return labels; }

  int addLabel(std::string lbl);
  void removeLabel(const std::string &,
      std::function<void(const char *file, int progress)> progress = nullptr);
  void moveLabelUp(uint16_t idx);
  void renameLabel(const std::string &from, const std::string& to,
      std::function<void(const char *file, int progress)> progress = nullptr);

  bool addLabelToModel(const std::string &, ModelCell *, bool update = true);
  void addLabelsToModel(const char* labels, ModelCell* cell);
  void removeLabelFromModel(const std::string &, ModelCell *);

  void setFilteredLabels(std::set<uint32_t> filtlbls);
  void clearFilter() { filtlbls.clear(); }
  void addFilteredLabel(const std::string &lbl);
  bool isLabelFiltered(const std::string &lbl);
  std::set<uint32_t> filteredLabels() { return filtlbls; }

 protected:
  ModelCell *currentModel = nullptr;
  ModelsSortBy _sortOrder = DEFAULT_MODEL_SORT;
  LabelsVector labels;  // Storage space for discovered labels
  std::set<uint32_t> filtlbls;

  bool loadYaml();
};

extern ModelsList modelCellManager;
