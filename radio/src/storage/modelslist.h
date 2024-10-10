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

#include <stdint.h>

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "sdcard.h"

#include "dataconstants.h"
#include "rtc.h"

// modelXXXXXXX.bin F,FF F,3F,FF\r\n
#define LEN_MODELS_IDX_LINE \
  (LEN_MODEL_FILENAME + sizeof(" F,FF F,3F,FF\r\n") - 1)

#define DEFAULT_MODEL_SORT NAME_ASC

#if !PORTRAIT_LCD // Landscape
#define LABEL_TRUNCATE_LENGTH 21
#else
#define LABEL_TRUNCATE_LENGTH 16
#endif

struct ModelData;
struct ModuleData;

struct SimpleModuleData {
  uint8_t type = 0;
  uint8_t subType = 0;
};

typedef struct PACKED {
  FSIZE_t fsize; /* File size */
  WORD fdate;    /* Modified date */
  WORD ftime;    /* Modified time */
} FInfoH;

#define FILE_HASH_LENGTH (sizeof(FInfoH) * 2)  // Hex string output

class ModelCell
{
 public:
  char modelFilename[LEN_MODEL_FILENAME + 1] = "";
  char modelName[LEN_MODEL_NAME + 1] = "";
  char modelFinfoHash[FILE_HASH_LENGTH + 1] = "";
#if LEN_BITMAP_NAME > 0
  char modelBitmap[LEN_BITMAP_NAME + 1] = "";
#endif
  gtime_t lastOpened = 0;
  bool _isDirty = true;

  bool valid_rfData;
  uint8_t modelId[NUM_MODULES] = {0, 0};
  SimpleModuleData moduleData[NUM_MODULES];

  explicit ModelCell(const char *fileName);
  explicit ModelCell(const char *fileName, uint8_t len);

  void setModelName(char *name);
  void setModelName(char *name, uint8_t len);
  void setRfData(ModelData *model);

  void setModelId(uint8_t moduleIdx, uint8_t id);
  void setRfModuleData(uint8_t moduleIdx, ModuleData *modData);
  bool fetchRfData();
};

typedef struct {
  std::string icon;
  // Anything else?
} SLabelDetail;

typedef std::vector<std::pair<uint16_t, ModelCell *>> ModelLabelsVector;
typedef std::vector<std::string> LabelsVector;
typedef std::vector<ModelCell *> ModelsVector;
typedef enum {
  NO_SORT,
  NAME_ASC,
  NAME_DES,
  DATE_ASC,
  DATE_DES,
  SORT_COUNT
} ModelsSortBy;

/**
 * @brief ModelMap is a multimap of all models and their cooresponding
 *        labels. Lables are referenced by index, stored in var labels
 */

class ModelMap : protected std::multimap<uint16_t, ModelCell *>
{
 public:
  ModelsVector getUnlabeledModels();
  ModelsVector getAllModels();
  ModelsVector getModelsByLabel(const std::string &);
  ModelsVector getModelsByLabels(const LabelsVector &);
  ModelsVector getModelsInLabels(const LabelsVector &lbls);
  LabelsVector getLabelsByModel(ModelCell *);
  std::map<std::string, bool> getSelectedLabels(ModelCell *);
  bool isLabelSelected(const std::string &, ModelCell *);
  LabelsVector getLabels();
  int addLabel(std::string lbl);
  bool addLabelToModel(const std::string &, ModelCell *, bool update = false);
  bool removeLabelFromModel(const std::string &label, ModelCell *,
                            bool update = false);
  bool removeLabel(
      const std::string &,
      std::function<void(const char *file, int progress)> progress = nullptr);
  bool moveLabelTo(unsigned current, unsigned newind);
  bool renameLabel(const std::string &from, std::string to,
      std::function<void(const char *file, int progress)> progress = nullptr);
  std::string getCurrentLabel() { return currentlabel; };
  void setCurrentLabel(const std::string &lbl)
  {
    currentlabel = lbl;
    setDirty();
  }
  std::string getBulletLabelString(ModelCell *, const char *noresults = "");
  void setDirty(bool save = false);
  bool isDirty() { return _isDirty; }

  // Currently selected labels in the GUI
  void setFilteredLabels(std::set<uint32_t> filtlbls)
  {
    this->filtlbls = std::move(filtlbls);
  }
  void clearFilter() { filtlbls.clear(); }
  void addFilteredLabel(const std::string &lbl);
  bool isLabelFiltered(const std::string &lbl);
  std::set<uint32_t> filteredLabels() { return filtlbls; }

  void setSortOrder(ModelsSortBy sortby);
  ModelsSortBy sortOrder() {return _sortOrder;}

  static std::string toCSV(const LabelsVector &labels);
  static LabelsVector fromCSV(const char *str);
  static void escapeCSV(std::string &str);
  static void unEscapeCSV(std::string &str);
  static void removeYAMLChars(std::string &str);
  static void replace_all(std::string &str,
                          const std::string &from,
                          const std::string &to);

 protected:
  ModelsSortBy _sortOrder = DEFAULT_MODEL_SORT;
  bool _isDirty = true;
  std::set<uint32_t> filtlbls;
  std::string currentlabel = "";

  void updateModelCell(ModelCell *);
  bool removeModels(
      ModelCell *);  // Should only be called from ModelsList remove model
  bool updateModelFile(ModelCell *);
  void sortModelsBy(ModelsVector &mv, ModelsSortBy sortby);

  void clear()
  {
    _isDirty = true;
    labels.clear();
    std::multimap<uint16_t, ModelCell *>::clear();
  }

  int getIndexByLabel(const std::string &str)
  {
    auto a = std::find(labels.begin(), labels.end(), str);
    return a == labels.end() ? -1 : a - labels.begin();
  }

  std::string getLabelByIndex(uint16_t index)
  {
    if (index < (uint16_t)labels.size())
      return labels.at(index);
    else
      return std::string();
  }

  friend class ModelsList;

 private:
  LabelsVector labels;  // Storage space for discovered labels
};

class ModelsList : public ModelsVector
{
  bool loaded;

  ModelCell *currentModel;

  void init();

 public:
  ModelsList();
  ~ModelsList();

  bool load();
  const char *save(LabelsVector newOrder=LabelsVector());
  void clear();

  void setCurrentModel(ModelCell *cell);
  void updateCurrentModelCell();

  ModelCell *getCurrentModel() const { return currentModel; }

  unsigned int getModelsCount() const
  {
    return std::vector<ModelCell *>::size();
  }

  ModelCell *addModel(const char *name, bool save = true, ModelCell *copyCell = nullptr);
  bool removeModel(ModelCell *model);
  bool moveModelTo(unsigned curindex, unsigned toindex);

  bool isModelIdUnique(uint8_t moduleIdx, char *warn_buf, size_t warn_buf_len);
  uint8_t findNextUnusedModelId(uint8_t moduleIdx);

  typedef struct _filedat {
    std::string name;
    char hash[FILE_HASH_LENGTH + 1];
    bool curmodel = false;
    bool celladded = false;
  } filedat;
  std::vector<filedat> fileHashInfo;

 protected:
  FIL file;

  bool loadYaml();
  bool loadYamlDirScanner();
};

ModelLabelsVector getUniqueLabels();

extern ModelsList modelslist;
extern ModelMap modelslabels;
