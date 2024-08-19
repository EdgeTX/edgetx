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

#include "modelslist.h"

#include <algorithm>

using std::list;

#include "edgetx.h"
#include "storage/sdcard_yaml.h"
#include "yaml/yaml_datastructs.h"
#include "yaml/yaml_labelslist.h"
#include "yaml/yaml_modelslist.h"
#include "yaml/yaml_parser.h"

#if defined(USBJ_EX)
#include "usb_joystick.h"
#endif

#include <cstring>

#include "datastructs.h"
#include "myeeprom.h"
#include "pulses/modules_helpers.h"
#include "strhelpers.h"

#ifdef DEBUG_LABELS
#define TRACE_LABELS(...) TRACE(__VA_ARGS__)
#else
#define TRACE_LABELS(...)
#endif

ModelsList modelslist;
ModelMap modelslabels;

ModelCell::ModelCell(const char *fileName) : valid_rfData(false)
{
  strncpy(modelFilename, fileName, sizeof(modelFilename) - 1);
  modelFilename[sizeof(modelFilename) - 1] = '\0';
}

ModelCell::ModelCell(const char *fileName, uint8_t len) : valid_rfData(false)
{
  if (len > sizeof(modelFilename) - 1) len = sizeof(modelFilename) - 1;

  memcpy(modelFilename, fileName, len);
  modelFilename[len] = '\0';
}

void ModelCell::setModelName(char *name)
{
  strncpy(modelName, name, LEN_MODEL_NAME);
  modelName[LEN_MODEL_NAME] = '\0';

  if (modelName[0] == '\0') {
    char *tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *)memchr(modelName, '.', LEN_MODEL_NAME);
    if (tmp != nullptr) *tmp = 0;
  }
}

void ModelCell::setModelName(char *name, uint8_t len)
{
  if (len > LEN_MODEL_NAME - 1) len = LEN_MODEL_NAME - 1;

  memcpy(modelName, name, len);
  modelName[len] = '\0';

  if (modelName[0] == '\0') {
    char *tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *)memchr(modelName, '.', LEN_MODEL_NAME);
    if (tmp != nullptr) *tmp = 0;
  }
}

void ModelCell::setModelId(uint8_t moduleIdx, uint8_t id)
{
  modelId[moduleIdx] = id;
}

void ModelCell::setRfData(ModelData *model)
{
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    modelId[i] = model->header.modelId[i];
    setRfModuleData(i, &(model->moduleData[i]));
    TRACE("<%s/%i> : %X,%X,%X", strlen(modelName) ? modelName : modelFilename,
          i, moduleData[i].type, moduleData[i].subType, modelId[i]);
  }
  valid_rfData = true;
}

void ModelCell::setRfModuleData(uint8_t moduleIdx, ModuleData *modData)
{
  moduleData[moduleIdx].type = modData->type;
  if (modData->type != MODULE_TYPE_MULTIMODULE) {
    moduleData[moduleIdx].subType = (uint8_t)modData->subType;
  } else {
    // do we care here about MM_RF_CUSTOM_SELECTED? probably not...
    moduleData[moduleIdx].subType = modData->multi.rfProtocol;
  }
}

//-----------------------------------------------------------------------------
/**
 * @brief Gets all models which don't have any labels selected
 *
 * @param sortby
 * @return ModelsVector vector<ModelCell>
 */

ModelsVector ModelMap::getUnlabeledModels()
{
  ModelsVector unlabeledModels;
  for (auto model : modelslist) {
    if (modelslabels.getLabelsByModel(model).size() == 0)
      unlabeledModels.emplace_back(model);
  }
  sortModelsBy(unlabeledModels, _sortOrder);
  return unlabeledModels;
}

/**
 * @brief Returns a sorted list of all models
 */

ModelsVector ModelMap::getAllModels()
{
  ModelsVector all = modelslist;
  sortModelsBy(all, _sortOrder);
  return all;
}

/**
 * @brief Returns all models that have a sepecified label
 *
 * @param lbl Label to search
 * @return ModelsVector aka vector<ModelCell*> of all models belonging to a
 * label
 */

ModelsVector ModelMap::getModelsByLabel(const std::string &lbl)
{
  int index = getIndexByLabel(lbl);
  if (index < 0) return ModelsVector();
  ModelsVector rv;
  for (auto it = begin(); it != end(); ++it) {
    if (it->first == index) rv.push_back(it->second);
  }
  sortModelsBy(rv, _sortOrder);
  return rv;
}

/**
 * @brief Returns all models that are in multiple labels (OR function)
 *
 * @param lbls Labels to search
 * @return ModelsVector aka vector<ModelCell*> of all models belonging to a
 * label
 */

ModelsVector ModelMap::getModelsByLabels(const LabelsVector &lbls)
{
  bool addunlabeled = false;
  // Build a list of the requested indexes
  std::vector<int> idxvect;
  for (const auto &lbl : lbls) {
    if (lbl == STR_UNLABELEDMODEL) addunlabeled = true;
    int index = getIndexByLabel(lbl);
    if (index >= 0) idxvect.push_back(index);
  }

  ModelsVector rv;
  for (auto it = begin(); it != end(); ++it) {
    for (auto idx : idxvect) {
      if (it->first == idx) rv.push_back(it->second);
    }
  }

  if (addunlabeled) {
    ModelsVector unlabeled = getUnlabeledModels();
    rv.insert(rv.end(), unlabeled.begin(), unlabeled.end());
  }

  sortModelsBy(rv, _sortOrder);
  return rv;
}

/**
 * @brief Returns all models that match the selected labels
 *
 * @param lbls Labels to search
 * @return ModelsVector aka vector<ModelCell*> of all models belonging to a
 * label
 */

ModelsVector ModelMap::getModelsInLabels(const LabelsVector &lbls)
{
  if (lbls.size() == 0) return ModelsVector();

  // Requesting only Unlabeled models
  if (lbls.size() == 1 && lbls.at(0) == STR_UNLABELEDMODEL)
    return getUnlabeledModels();

  ModelsVector rv;

  for (const auto &mdl : modelslist) {
    bool hasAllLabels = true;
    bool hasAnyLabels = false;
    bool favLabelIncluded = false;
    bool hasFavLabel = false;
    LabelsVector mdllables = getLabelsByModel(mdl);
    for (const auto &lbl : lbls) {
      if (lbl == STR_UNLABELEDMODEL)  // If requesting unlabeled model ignore it
        break;
      bool hasLabel = std::find(mdllables.begin(), mdllables.end(), lbl) != mdllables.end();
      if (lbl == STR_FAVORITE_LABEL) {
        favLabelIncluded = true;
        hasFavLabel = hasLabel;
      } else {
        if (hasLabel) {
          hasAnyLabels = true;
        } else {
          hasAllLabels = false;
        }
      }
    }
    if (favLabelIncluded) {
      if (g_eeGeneral.favMultiMode == 0) {
        hasAnyLabels = hasAnyLabels && hasFavLabel;
        hasAllLabels = hasAllLabels && hasFavLabel;
      } else if (g_eeGeneral.favMultiMode == 1) {
        hasAnyLabels = hasAnyLabels || hasFavLabel;
        hasAllLabels = hasAllLabels && hasFavLabel;
      }
    }
    if (((g_eeGeneral.labelMultiMode == 0) && hasAllLabels) ||
        ((g_eeGeneral.labelMultiMode == 1) && hasAnyLabels))
      rv.push_back(mdl);
  }

  sortModelsBy(rv, _sortOrder);
  return rv;
}

/**
 * @brief Gets all labels that a model currently has selected
 *
 * @param mdl Model to search
 * @return LabelsVector aka vector<string> of all labels selected by a model
 */

LabelsVector ModelMap::getLabelsByModel(ModelCell *mdl)
{
  if (mdl == nullptr) return LabelsVector();
  LabelsVector rv;
  for (auto it = begin(); it != end(); ++it) {
    if (it->second == mdl) {
      rv.push_back(getLabelByIndex(it->first));
    }
  }
  return rv;
}

/**
 * @brief Get a map of all labels and their selection status
 * @details Returns a map of all the labels and if they are selected in a model.
 *
 * @param cell ModelCell to search
 * @return std::map<std::string, bool> Map Containing all labels with a boolean
 * if they are selected
 */

std::map<std::string, bool> ModelMap::getSelectedLabels(ModelCell *cell)
{
  std::map<std::string, bool> rval;
  for (auto lbl : labels) {  // Add all labels
    if (lbl == "") continue;
    rval[lbl] = false;
  }
  LabelsVector modlbl = getLabelsByModel(cell);
  for (const auto &ml : modlbl) {  // Set to true if selected by the model
    rval[ml] = true;
  }
  return rval;
}

/**
 * @brief Returns if a label is selected in the model
 *
 * @param label Label to search
 * @param cell Model to search
 * @return true Label is selected
 * @return false Label not seleced OR label not found
 */

bool ModelMap::isLabelSelected(const std::string &label, ModelCell *cell)
{
  auto lbm = getLabelsByModel(cell);
  if (std::find(lbm.begin(), lbm.end(), label) == lbm.end()) {
    return false;
  }
  return true;
}

/**
 * @brief Returns a list of the current labels
 *
 * @return LabelsVector vector<string> of all labels
 */

LabelsVector ModelMap::getLabels()
{
  LabelsVector rv;
  for (auto &label : labels) {
    // Skip the internal empty (removed) ones
    if (label.size() > 0) rv.emplace_back(label);
  }
  return rv;
}

/**
 * @brief Adds a label
 * @details  Checks if the label already exists. If it does it returns the
 *           index to it. If label doesn't exist it adds it at the end of the
 *           list and returns the new index
 *           Won't allow creation of the special case label "Unlabeled" STR_UNLABELEDMODEL
 *
 * @param lbl Adds a label to the list
 * @return int -1 on failure, label index on success
 */

int ModelMap::addLabel(std::string lbl)
{
  if (lbl == STR_UNLABELEDMODEL) return -1;

  // Limit maximum label length, TODO... Truncate UTF8 Properly
  lbl = lbl.substr(0, LABEL_LENGTH);
  removeYAMLChars(lbl);
  if (lbl.size() == 0) return -1;

  // Add a new label if it doesn't already exist in the list
  // Returns the index to the label
  int ind = getIndexByLabel(lbl);
  if (ind < 0) {
    labels.push_back(lbl);
    setDirty();
    TRACE_LABELS("Added a label %s", lbl.c_str());
    return labels.size() - 1;
  }
  return ind;
}

/**
 * @brief Adds a label to a model
 *
 * @param lbl Label to be added
 * @param cell Model to add the label
 * @param update If true will update the model file
 * @return true Couldn't add label, not enough memory available in labels string
 * @return false Success
 */

bool ModelMap::addLabelToModel(const std::string &lbl, ModelCell *cell,
                               bool update)
{
  // First check that there aren't too many labels on this model
  LabelsVector lbs = getLabelsByModel(cell);
  lbs.push_back(lbl);
  if(ModelMap::toCSV(lbs).size() > LABELS_LENGTH - 1) {
    TRACE("Cannot add the %s label to the model. Too many labels", lbl.c_str());
    return true;
  }

  setDirty();
  int labelindex = addLabel(lbl);
  insert(std::pair<int, ModelCell *>(labelindex, cell));

  if (update) updateModelFile(cell);  // Write labels into model

  return false;
}

/**
 * @brief Adds a label to the filter, used in yaml_labelslist on load
 *
 * @param label Label to be added
 */

void ModelMap::addFilteredLabel(const std::string &lbl)
{
  int ind = getIndexByLabel(lbl);
  if (ind >= 0) filtlbls.insert(ind);
}

bool ModelMap::isLabelFiltered(const std::string &lbl)
{
  if (filtlbls.find(getIndexByLabel(lbl)) != filtlbls.end()) return true;
  return false;
}

void ModelMap::setSortOrder(ModelsSortBy sortby)
{
  if (sortby < SORT_COUNT && sortby >= NO_SORT) {
    _sortOrder = sortby;
    setDirty();
  }
}

/**
 * @brief Convert a vector of labels to a comma separated list
 *
 * @param labels Vector of the labels
 * @return labels list in CSV format
 */

std::string ModelMap::toCSV(const LabelsVector &labels)
{
  std::string csv;
  bool comma=false;
  for(auto lbl: labels) {
    if(lbl == "")
      continue;
    if(comma) {
      csv += ',';
    }
    // Escape out commas
    escapeCSV(lbl);
    csv += lbl;
    comma = true;
  }
  return csv;
}

/**
 * @brief Convert a Comma separated string list to LabelsVector
 *
 * @param str Comma separated string of labels
 * @return vector of all labels
 */

LabelsVector ModelMap::fromCSV(const char* str)
{
  LabelsVector lbls;
  const char* prev_c = str;
  const char* c = strchr(prev_c, ',');
  while(c != nullptr) {
    std::string lbl(prev_c, c - prev_c);
    unEscapeCSV(lbl);
    lbls.push_back(lbl);
    prev_c = ++c;
    c = strchr(c, ',');
  }

  std::string lbl(prev_c);
  if (!lbl.empty()) {
    unEscapeCSV(lbl);
    lbls.push_back(lbl);
  }

  return lbls;
}

// TODO - Fix me, ideally there should be no limitations
void ModelMap::escapeCSV(std::string &str)
{
  replace_all(str, "/", "//");
  replace_all(str, ",", "/c");
}

// TODO - Fix me, ideally there should be no limitations
void ModelMap::unEscapeCSV(std::string &str)
{
  replace_all(str, "//", "/");
  replace_all(str, "/c", ",");
}

// TODO - Fix me, ideally there should be no limitations
void ModelMap::removeYAMLChars(std::string &str)
{
  replace_all(str, "\\", "");
  replace_all(str, "\"", "");
  replace_all(str, ":", "");
  replace_all(str, "\'", "");
  replace_all(str, "-", "");
}

/**
 * @brief Replace all occurances of <from> to <to> in <str>
 *
 * @param str String
 * @param from String to search
 * @param to String to replace
 */

void ModelMap::replace_all(std::string &str,
                           const std::string &from,
                           const std::string &to)
{
  if(from.empty()) return;
  size_t pos = 0;
  while((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
  }
}

/**
 * @brief Removes a label from a model
 *
 * @param label Label to be removed
 * @param cell Model to remove the label from
 * @return true Couldn't find the label
 * @return false Success
 */

bool ModelMap::removeLabelFromModel(const std::string &label, ModelCell *cell,
                                    bool update)
{
  int lblind = getIndexByLabel(label);
  if (lblind < 0) return true;
  bool rv = true;
  // Erase items that match in the map
  for (ModelMap::const_iterator itr = cbegin(); itr != cend();) {
    itr = (itr->first == lblind && itr->second == cell) ? erase(itr)
                                                        : std::next(itr);
    setDirty();
    rv = false;
  }

  if (update) updateModelFile(cell);  // Write labels into model

  return rv;
}

/**
 * @brief Removes a label
 * @details Remove a label from the list, only if there are no models that have
 *          the label selected. The label is set blank, when writing this will be
 *          skipped. Labels are re-synced
 *
 * @param label Label to be removed
 * @return true Label wasn't found or not empty
 * @return false Success
 */

bool ModelMap::removeLabel(
    const std::string &label,
    std::function<void(const char *file, int progress)> progress)
{
  bool renameFault=true;
  renameLabel(label, "", std::move(progress));
  for (auto &lbl : labels) {
    if (lbl == label && getModelsByLabel(lbl).size() == 0) {
      lbl = "";
      setDirty();
      renameFault = false;
    }
  }

  // If no more labels, add a favorite
  if (!renameFault && getLabels().size() == 0) {
    addLabel(STR_FAVORITE_LABEL);
  }

  // Save and rescan
  modelslist.save();
  modelslist.clear();
  modelslist.load();

  return renameFault;
}

/**
 * @brief Moves a labels order
 *
 * @param current Index of the label to move
 * @param toindex Index where to place the new label
 * @return 1 Failure
 * @return 0 Success
 */

bool ModelMap::moveLabelTo(unsigned curind, unsigned newind)
{
  if (curind == newind || curind >= labels.size() || newind >= labels.size())
    return true;

  if (labels.at(curind) == "") return true;

  std::swap(labels[curind], labels[newind]);

  ModelMap newmap;
  newmap.labels = labels;

  for (auto &mm : modelslabels) {
    uint16_t ind = mm.first;
    if (ind == curind)
      ind = newind;
    else if (ind == newind)
      ind = curind;
    newmap.insert(std::make_pair(ind, mm.second));
  }

  modelslabels = newmap;

  modelslist.save(labels);
  setDirty();

  return false;
}

/**
 * @brief Rename a label
 * @details Opens all models which have a label that matches the <from> string.
 *          Renames the label with <to> string and saves file.
 *          If working on the current model it replaces the label string in
 *            g_model
 *
 * @param from Label to search
 * @param to Replacement label
 * @return true failure Label couldn't be found, YAML couldn't be read
 * @return false success
 */

bool ModelMap::renameLabel(const std::string &from, std::string to,
    std::function<void(const char *file, int progress)> progress)
{
  if (from == "") return true;
  DEBUG_TIMER_START(debugTimerYamlScan);

  if(to.size() > 0) { // Ignore check if deleting a label, size will be zero
    to = to.substr(0, LABEL_LENGTH); // Limit max label name. TODO: Warn user they entered too long of a string
    removeYAMLChars(to); // Remove special chars
    if(to.size() == 0 || from == to) { // Abort rename if no chars left or same
      if (progress != nullptr) progress("", 100); // Kill progress dialog
      return true;
    }
  }

  ModelData *modeldata = (ModelData *)malloc(sizeof(ModelData));
  if (!modeldata) {
    TRACE("Labels: Out Of Memory");
    if (progress != nullptr) progress("", 100); // Kill progress dialog
    return true;
  }

  // Force a write of any changes in memory
  storageCheck(true);

  bool fault = false;
  ModelsVector mods = getModelsByLabel(from);  // Find all models to be renamed

  // Scan all these models first, recombine their labels to a csv,
  // make sure re-size is going to fit before starting. Otherwise new partial
  // labels would be created on next scan.
  for(const auto &model: mods) {
    int curlen = toCSV(getLabelsByModel(model)).size();
    std::string csvto = to;
    escapeCSV(csvto);
    std::string csvfrom = from;
    escapeCSV(csvfrom);
    if(curlen + csvto.size() - csvfrom.size() > LABELS_LENGTH - 1) {
      TRACE("Labels: Rename Error! Labels too long on %s", model->modelName);
      if (progress != nullptr) progress("", 100); // Kill progress dialog
      free(modeldata);
      return true;
    }
  }

  int i = 0;
  for (const auto &modcell : mods) {
    if (progress != nullptr) {
      progress(modcell->modelFilename, (i++) * 100 / mods.size());
    }

    readModelYaml(modcell->modelFilename, (uint8_t *)modeldata,
                  sizeof(ModelData));

    // Separate Curent CSV
    LabelsVector lbls = ModelMap::fromCSV(modeldata->header.labels);

    // Replace from->to strings
    for (auto &lbl : lbls) {
      if (lbl == from) lbl = to;
    }

    // Remove possible duplicates
    std::sort(lbls.begin(), lbls.end());
    auto last = std::unique(lbls.begin(), lbls.end());
    lbls.erase(last, lbls.end());
    lbls.resize(std::distance(lbls.begin(), last));

    // Write back
    strncpy(modeldata->header.labels, ModelMap::toCSV(lbls).c_str(), LABELS_LENGTH);
    modeldata->header.labels[LABELS_LENGTH-1] = '\0';

    char path[256];
    getModelPath(path, modcell->modelFilename);

    if (modcell == modelslist.getCurrentModel()) {
      // If working on the current model, write current data to file instead
      memcpy(g_model.header.labels, modeldata->header.labels, LABELS_LENGTH);
      fault = (writeFileYaml(path, get_modeldata_nodes(),
                             (uint8_t *)&g_model, 0) != NULL);
    } else {
      fault = (writeFileYaml(path, get_modeldata_nodes(),
                             (uint8_t *)modeldata, 0) != NULL);
    }
#if defined(SIMU)
    if (SIMU_SLEEP_OR_EXIT_MS(100)) break;
#endif
  }

  // Rename the label
  for (auto &lbl : labels) {
    if (lbl == from) {
      lbl = to;
      setDirty(true);
    }
  }

  // Make sure to leave at 100, to kill rename dialog
  if (progress != nullptr) progress("", 100);

  free(modeldata);

  // Issue a rescan all of all models.
  modelslist.clear();
  modelslist.load();

#if defined(DEBUG_TIMERS)
  DEBUG_TIMER_SAMPLE(debugTimerYamlScan);
  TRACE("Labels: Time to rename %d labels %luus", mods.size(),
        debugTimers[debugTimerYamlScan].getLast());
#endif

  return fault;
}

/**
 * @brief Returns a bullet separated list of the labels, used in model_setup
 *
 * @param curmod Model Cell
 * @param noresults String to return when no labels found
 * @return std::string of all Labels, if no results return
 */

std::string ModelMap::getBulletLabelString(ModelCell *curmod, const char *noresults)
{
  std::string lbls = ModelMap::toCSV(getLabelsByModel(curmod));
  replace_all(lbls, ",", "\u2022");
  unEscapeCSV(lbls);

  if(lbls.size() == 0) {
    return noresults;
  }
  if(lbls.size() > LABEL_TRUNCATE_LENGTH) {
    lbls = lbls.substr(0, LABEL_TRUNCATE_LENGTH);
    lbls += "...";
  }
  return lbls;
}

/**
 * @brief Removes all models from the map
 *
 * @param cell Model to remove
 * @return true Failure finding a model
 * @return false Success
 */

bool ModelMap::removeModels(ModelCell *cell)
{
  bool rv = true;
  // Erase items that match in the map
  for (ModelMap::const_iterator itr = cbegin(); itr != cend();) {
    if (itr->second == cell) {
      itr = erase(itr);
      setDirty();
      rv = false;
    } else {
      itr = std::next(itr);
    }
  }
  return rv;
}

/**
 * @brief Opens a model.yml File and writes labels data into it.
 * @details If the cell is current model then write the labels data to g_model
 * and mark as dirty
 *
 * @param cell
 * @return true
 * @return false
 */

bool ModelMap::updateModelFile(ModelCell *cell)
{
  // Update memory copy if on current model
  if (cell == modelslist.getCurrentModel()) {
    strncpy(g_model.header.labels, ModelMap::toCSV(getLabelsByModel(cell)).c_str(),
            LABELS_LENGTH - 1);
    g_model.header.labels[LABELS_LENGTH - 1] = '\0';
    storageDirty(EE_MODEL);
    return false;
  }

  ModelData *modeldata = (ModelData *)malloc(sizeof(ModelData));
  if (!modeldata) {
    TRACE("Labels: Out Of Memory");
    return true;
  }

  bool fault = false;
  readModelYaml(cell->modelFilename, (uint8_t *)modeldata, sizeof(ModelData));

  strncpy(modeldata->header.labels, ModelMap::toCSV(getLabelsByModel(cell)).c_str(),
          LABELS_LENGTH - 1);
  modeldata->header.labels[LABELS_LENGTH - 1] = '\0';

  char path[256];
  getModelPath(path, cell->modelFilename);
  fault = (writeFileYaml(path, get_modeldata_nodes(), (uint8_t *)modeldata, 0) !=
           NULL);

  free(modeldata);

#if defined(DEBUG_TIMERS)
  DEBUG_TIMER_SAMPLE(debugTimerYamlScan);
  TRACE("Labels: Time to add/remove labels %luus",
        debugTimers[debugTimerYamlScan].getLast());
#endif

  return fault;
}

/**
 * @brief Sorts a ModelsVector by sortby
 *
 * @param mv ModelsVector to sort
 * @param sortby NO_SORT, NAME_ASC, NAME_DES, DATE_ASC, DATE_DES,
 */

void ModelMap::sortModelsBy(ModelsVector &mv, ModelsSortBy sortby)
{
  if (sortby == DATE_DES) {
    std::sort(mv.begin(), mv.end(), [](ModelCell *a, ModelCell *b) -> bool {
      return a->lastOpened > b->lastOpened;
    });
  } else if (sortby == DATE_ASC) {
    std::sort(mv.begin(), mv.end(), [](ModelCell *a, ModelCell *b) -> bool {
      return a->lastOpened < b->lastOpened;
    });
  } else if (sortby == NAME_ASC) {
    std::sort(mv.begin(), mv.end(), [](ModelCell *a, ModelCell *b) -> bool {
      return strcasecmp(a->modelName, b->modelName) < 0;
    });
  } else if (sortby == NAME_DES) {
    std::sort(mv.begin(), mv.end(), [](ModelCell *a, ModelCell *b) -> bool {
      return strcasecmp(a->modelName, b->modelName) > 0;
    });
  }
}

/**
 * @brief Sets the ModelMap to dirty.
 * @details Causes labels.yml to be written after a delay in
 * sdcard_common.cpp->storageCheck()
 */

void ModelMap::setDirty(bool save)
{
  _isDirty = true;
  storageDirty(EE_LABELS);
  if (save) storageCheck(true);
}

//-----------------------------------------------------------------------------

ModelsList::ModelsList() { init(); }

ModelsList::~ModelsList() { clear(); }

void ModelsList::init()
{
  loaded = false;
  currentModel = nullptr;
}

void ModelsList::clear()
{
  for(ModelCell *mdl: *this) {
    delete(mdl);
  }
  std::vector<ModelCell *>::clear();
  init();
}

/**
 * @brief Opens a YAML file, reads the data and updates the ModelCell
 *
 * @param cell Model to update
 */

void ModelMap::updateModelCell(ModelCell *cell)
{
  modelslabels.removeModels(cell);

  ModelData *model = (ModelData *)malloc(sizeof(ModelData));
  if (!model) {
    TRACE("Labels: Out Of Memory");
    return;
  }

  TRACE("Labels: Updating model %s", cell->modelFilename);
  readModelYaml(cell->modelFilename, (uint8_t *)model, sizeof(ModelData));
  strncpy(cell->modelName, model->header.name, LEN_MODEL_NAME);
  cell->modelName[LEN_MODEL_NAME] = '\0';
  strncpy(cell->modelBitmap, model->header.bitmap, LEN_BITMAP_NAME);
  cell->modelBitmap[LEN_BITMAP_NAME] = '\0';
  LabelsVector labels = ModelMap::fromCSV(model->header.labels);
  for(const auto &lbl : labels ) {
    modelslabels.addLabelToModel(lbl,cell);
  }

  // Save Module Data
  cell->setRfData(model);

  cell->_isDirty = false;
  free(model);
}

/**
 * @brief Creates a Hash based on the file information
 *
 * @param buffer Buffer to store the output data
 * @param finfo Files info handle
 * @return char* Pointer to buffer supplied
 */

char *FILInfoToHexStr(char buffer[17], FILINFO *finfo)
{
  char *str = buffer;
  for (unsigned int i = 0; i < sizeof(FInfoH); i++) {
    sprintf(str, "%02x", *((uint8_t *)finfo + i));
    str += 2;
  }
  return buffer;
}

/**
 * @brief Loads the Labels and Models from the labels.yml file
 *
 * @return true On success
 * @return false On failure
 */

bool ModelsList::loadYaml()
{
  // Clear labels + map
  modelslist.clear();
  modelslabels.clear();
  fileHashInfo.clear();

  DEBUG_TIMER_START(debugTimerYamlScan);

  // Scan all models in folder
  DIR moddir;
  FILINFO finfo;
  if (f_opendir(&moddir, MODELS_PATH) == FR_OK) {
    for (;;) {
      FRESULT res = f_readdir(&moddir, &finfo);
      if (res != FR_OK || finfo.fname[0] == 0) break;
      if (finfo.fattrib & AM_DIR) continue;
      unsigned int len = strlen(finfo.fname);

      // Only open model###.yml files
      bool modelNameInvalid = false;
      if (strncasecmp(finfo.fname, MODEL_FILENAME_PREFIX, sizeof(MODEL_FILENAME_PREFIX) - 1) == 0) {
        for (unsigned int i = sizeof(MODEL_FILENAME_PREFIX) - 1; i < len - 4; i++) {
          if(finfo.fname[i] < '0' || finfo.fname[i] > '9') {
            modelNameInvalid = true;
            break;
          }
        }
      } else {
        modelNameInvalid = true;
      }

      if (modelNameInvalid ||
          strcasecmp(finfo.fname + len - 4, YAML_EXT) ||  // Skip non .yml files
          (finfo.fattrib & AM_DIR)) {  // Skip sub dirs
        continue;
      }

      // Store hash & filename
      filedat cf;
      FILInfoToHexStr(cf.hash, &finfo);
      cf.name = finfo.fname;
      cf.celladded = false;
      if (!strncmp(finfo.fname, g_eeGeneral.currModelFilename,
                   LEN_MODEL_FILENAME))
        cf.curmodel = true;
      else
        cf.curmodel = false;
      fileHashInfo.push_back(cf);
      TRACE_LABELS("File - %s \r\n  HASH - %s - CM: %s", finfo.fname, cf.hash,
                   cf.curmodel ? "Y" : "N");
    }
    f_closedir(&moddir);
  }

  // Check if models.yml exists
  // Any files found above that are not listed in the file will be moved into
  // /MDOELS/UNUSED and removed from the discovered file hash list
  char line[LEN_MODELS_IDX_LINE + 1];
  FILINFO fno;
  FRESULT result;
  bool foundInModels = f_stat(MODELSLIST_YAML_PATH, &fno) == FR_OK;
  bool foundInRadio = f_stat(FALLBACK_MODELSLIST_YAML_PATH, &fno) == FR_OK;

  if(foundInModels) { // Default to /Models copy
    result = f_open(&file, MODELSLIST_YAML_PATH, FA_OPEN_EXISTING | FA_READ);
  } else if (foundInRadio) {
    result = f_open(&file, FALLBACK_MODELSLIST_YAML_PATH, FA_OPEN_EXISTING | FA_READ);
  }
  if((foundInModels || foundInRadio) && result == FR_OK) {
    // Create /Models/Unused if it doesn't exist
    bool moveRequired = false;
    DIR unusedFolder;
    FRESULT result = f_opendir(&unusedFolder, UNUSED_MODELS_PATH);
    if (result != FR_OK) {
      if (result == FR_NO_PATH) result = f_mkdir(UNUSED_MODELS_PATH);
      if (result != FR_OK) {
        TRACE("Unable to create unused models folder");
        f_close(&file);
        return false;
      }
    } else f_closedir(&unusedFolder);

    YamlParser ymp;
    std::vector<std::string> modfiles;
    void *ctx = get_modelslist_iter(&modfiles);
    ymp.init(get_modelslist_parser_calls(), ctx);
    UINT bytes_read = 0;
    while (f_read(&file, line, sizeof(line), &bytes_read) == FR_OK) {
      if (bytes_read == 0) break;
      if (f_eof(&file)) ymp.set_eof();
      if (ymp.parse(line, bytes_read) != YamlParser::CONTINUE_PARSING) break;
    }
    f_close(&file);

    // Loop through file hases, move any files found that don't exists to /unused
    std::vector<filedat> newFileHash;
    for(const auto &fhas: fileHashInfo) {
      bool found = false;
      for(const auto &file : modfiles) {
        if (file == fhas.name) {
          TRACE_LABELS("Found file %s in models.yml.. OK!", file);
          found = true;
          break;
        }
      }
      if(!found) {
        moveRequired = true;
        TRACE_LABELS("Model %s not in models.yml, moving to /UNUSED", fhas.name.c_str());
        // Move model into unused folder.
        const char *warning = sdMoveFile(fhas.name.c_str(), MODELS_PATH, fhas.name.c_str(), UNUSED_MODELS_PATH);
        if(warning)
          POPUP_WARNING(warning);
      } else {
        newFileHash.push_back(fhas); // File exists, keep it
      }
    }

    if(foundInRadio) {
      const char *warning = sdMoveFile(MODELS_FILENAME, RADIO_PATH, MODELS_FILENAME ".old", UNUSED_MODELS_PATH);
      if(warning)
        POPUP_WARNING(warning);
    }
    if(foundInModels) { // Will overwrite the copy from /radio if both existed, do last
      const char *warning = sdMoveFile(MODELS_FILENAME, MODELS_PATH, MODELS_FILENAME ".old", UNUSED_MODELS_PATH);
      if(warning)
        POPUP_WARNING(warning);
    }
    if(moveRequired) {
      fileHashInfo = newFileHash; // Update the new file list
      POPUP_WARNING(TR_MODELS_MOVED "\n" UNUSED_MODELS_PATH, "\n" TR_PRESS_ANY_KEY_TO_SKIP);
    }
  }

#if defined(DEBUG_TIMERS)
  DEBUG_TIMER_SAMPLE(debugTimerYamlScan);
  TRACE("Lables: Time to scan models folder %luus",
        debugTimers[debugTimerYamlScan].getLast());
#endif

  // Scan labels.yml
  result = f_open(&file, LABELSLIST_YAML_PATH, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    YamlParser yp;
    void *ctx = get_labelslist_iter();
    yp.init(get_labelslist_parser_calls(), ctx);
    UINT bytes_read = 0;
    while (f_read(&file, line, sizeof(line), &bytes_read) == FR_OK) {
      if (bytes_read == 0) break;
      if (f_eof(&file)) yp.set_eof();
      if (yp.parse(line, bytes_read) != YamlParser::CONTINUE_PARSING) break;
    }
    f_close(&file);
  }

#if defined(DEBUG_TIMERS)
  DEBUG_TIMER_SAMPLE(debugTimerYamlScan);
  TRACE("Lables: Time to scan labels.yml %luus",
        debugTimers[debugTimerYamlScan].getLast());
#endif

  // Add modelcells for any remaining models that weren't in labels.yml
  for (auto &filehash : fileHashInfo) {
    ModelCell *model = NULL;
    if (filehash.celladded == false) {
      TRACE_LABELS("  Created a modelcell for %s, not in labels.yml",
                   filehash.name.c_str());
      model = new ModelCell(filehash.name.c_str());
      strncpy(model->modelFinfoHash, filehash.hash, FILE_HASH_LENGTH);
      model->modelFinfoHash[FILE_HASH_LENGTH] = '\0';
      modelslist.push_back(model);
      filehash.celladded = true;
      model->_isDirty = true;
      if(filehash.curmodel == true)
        modelslist.setCurrentModel(model);
    }
  }

  // Scan all models, to see if update needed
  bool updatelabelsyml = false;
  for (auto &model : modelslist) {
    // Open and read each model if it's marked as dirty
    if (model->_isDirty) {
      updatelabelsyml = true;
      modelslabels.updateModelCell(model);
    }
  }

  fileHashInfo.clear();

  // If any items differed save the file
  if (updatelabelsyml == true) {
    TRACE_LABELS("LABELS.YML Wasn't in sync. Needs to be saved");
    modelslist.save();
  } else {
    TRACE_LABELS("LABELS.YML Is in Sync! No models were read");
  }

  // If no labels found. Add a favorites label
  if (modelslabels.getLabels().size() == 0) {
    modelslabels.addLabel(STR_FAVORITE_LABEL);
  }

  return true;
}

/**
 * @brief Called to load the model data from file
 *
 * @return true on success
 * @return false on failure
 */

bool ModelsList::load()
{
  if (loaded) return true;

  bool res = loadYaml();

  if (!currentModel) {
    TRACE("ERROR no Current Model Found");
    if (modelslist.size()) {
      modelslist.setCurrentModel(modelslist.at(0));
      strncpy(g_eeGeneral.currModelFilename, modelslist.at(0)->modelFilename,
              sizeof(g_eeGeneral.currModelFilename));
      g_eeGeneral.currModelFilename[sizeof(g_eeGeneral.currModelFilename) - 1] = '\0';
      TRACE("  - Set current model to first available");
    } else {
      TRACE("  - No Models Found, making a new one");
      // No models found, make a new one
      auto model = modelslist.addModel(createModel(), true);
      modelslist.setCurrentModel(model);
      updateCurrentModelCell();
    }
  }

  loaded = true;
  return res;
}

/**
 * @brief Writes labels.yml file
 * @param newOrder vector<string> - Forces a save of this label order. leave empty to use current
 *                 order.
 * @return const char* NULL on success
 * @return const char* Error String on failure
 */

const char *ModelsList::save(LabelsVector newOrder)
{
  FRESULT result =
      f_open(&file, LABELSLIST_YAML_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) return "Couldn't open labels.yml for writing";

  // Save current selection
  f_puts("Labels:\r\n", &file);

  std::string cursel = modelslabels.getCurrentLabel();
  if(newOrder.empty())
    newOrder = modelslabels.getLabels();
  for (auto &lbl : newOrder) {
    f_printf(&file, "  \"%s\":\r\n", lbl.c_str());
    if (modelslabels.isLabelFiltered(lbl))
      f_printf(&file, "    selected: true\r\n", lbl.c_str());
  }

  // Save current sort order
  f_printf( &file, "Sort: %d\r\n", modelslabels.sortOrder());

  f_puts("Models:\r\n", &file);
  for (auto &model : modelslist) {
    f_puts("  ", &file);
    f_puts(model->modelFilename, &file);
    f_puts(":\r\n", &file);

    f_puts("    hash: \"", &file);
    f_puts(model->modelFinfoHash, &file);
    f_puts("\"\r\n", &file);

    f_puts("    name: \"", &file);
    f_puts(model->modelName, &file);
    f_puts("\"\r\n", &file);

    for (int i = 0; i < NUM_MODULES; i++) {
      if (model->modelId[i])
        f_printf(&file, "    " MODULE_ID_STR ": %u\r\n", i,
                 (unsigned int)model->modelId[i]);
      if (model->moduleData[i].type)
        f_printf(&file, "    " MODULE_TYPE_STR ": %u\r\n", i,
                 (unsigned int)model->moduleData[i].type);
      if (model->moduleData[i].subType)
        f_printf(&file, "    " MODULE_RFPROTOCOL_STR ": %u\r\n", i,
                 (unsigned int)model->moduleData[i].subType);
    }

    f_printf(&file, "    labels: \"%s\"\r\n", ModelMap::toCSV(modelslabels.getLabelsByModel(model)).c_str());

#if LEN_BITMAP_NAME > 0
    f_puts("    bitmap: \"", &file);
    f_puts(model->modelBitmap, &file);
    f_puts("\"\r\n", &file);
#endif
    f_puts("    lastopen: ", &file);
    f_puts(std::to_string(model->lastOpened).c_str(), &file);
    f_puts("\r\n", &file);
  }

  f_puts("\r\n", &file);
  f_close(&file);
  modelslabels._isDirty = false;

  return NULL;
}

/**
 * @brief set the currently loaded model.
 *
 * @param cell Model to set as current
 */

void ModelsList::setCurrentModel(ModelCell *cell)
{
  currentModel = cell;
  struct gtm t;
  gettime(&t);
  cell->lastOpened = gmktime(&t);
  modelslabels.setDirty();

#if defined(USBJ_EX) && defined(STM32) && !defined(SIMU)
  onUSBJoystickModelChanged();
#endif
}

/**
 * @brief Takes the current g_model + filename data in memory and update
 *        the ModelCell data to match.
 */

void ModelsList::updateCurrentModelCell()
{
  if (currentModel) {
#if LEN_BITMAP_NAME > 0
    strncpy(currentModel->modelBitmap, g_model.header.bitmap, LEN_BITMAP_NAME);
    currentModel->modelBitmap[LEN_BITMAP_NAME] = '\0';
#endif
    strncpy(currentModel->modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
    currentModel->modelFilename[LEN_MODEL_FILENAME] = '\0';
    currentModel->setModelName(g_model.header.name);
    currentModel->setRfData(&g_model);
    modelslabels.setDirty();
  } else {
    TRACE("ModelList Error - No Current Model");
  }
}

/**
 * @brief Creates a new ModelCell
 *
 * @param name Model File Name
 * @param save True=Update labels.yml right away
 * @param copyCell If duplicating copy the data from this cell, otherwise leave null
 * @return ModelCell* New Model
 */

ModelCell *ModelsList::addModel(const char *fileName, bool save, ModelCell *copyCell)
{
  ModelCell *result = new ModelCell(fileName);
  if (copyCell != nullptr) { // Duplicate all data
    memcpy(result, copyCell, sizeof(ModelCell));
  }

  // Set the new File Name
  strncpy(result->modelFilename, fileName, LEN_MODEL_FILENAME);
  result->modelFilename[LEN_MODEL_FILENAME] = '\0';

  // Add to the ModelsList
  push_back(result);

  // Force save to labels.yml
  if (save) this->save();
  return result;
}

/**
 * @brief Removes a model from the list and the modelmap.
 * @details The models filename is moved into the sub folder DELETED_MODELS_PATH
 *          rather than deleting the model. If left in current location would
 *          be re-scanned on boot
 *
 * @param model Model to be deleted
 * @return false Successfully removed
 * @return true Failure
 */

bool ModelsList::removeModel(ModelCell *model)
{
  erase(std::remove(begin(), end(), model), end());
  modelslabels.removeModels(model);

  // Create deleted folder if it doesn't exist
  DIR deletedFolder;
  FRESULT result = f_opendir(&deletedFolder, DELETED_MODELS_PATH);
  if (result != FR_OK) {
    if (result == FR_NO_PATH) result = f_mkdir(DELETED_MODELS_PATH);
    if (result != FR_OK) {
      TRACE("Unable to create deleted models folder");
      return true;
    }
  } else f_closedir(&deletedFolder);

  // Move model into deleted folder. If not moved will be re-added on next
  // reboot
  TRACE_LABELS("Deleting Model %s", model->modelFilename);
  const char *warning = sdMoveFile(model->modelFilename, MODELS_PATH, model->modelFilename, DELETED_MODELS_PATH);
  if (warning) {
    TRACE("Labels: Unable to move file");
    return true;
  }

  // Free memory
  delete(model);

  return false;
}

/**
 * @brief Moves a model in the list for customization of order
 *
 * @param curindex Index of the model to move
 * @param toindex Destination index of the model
 * @return true Failure
 * @return false Success
 */

bool ModelsList::moveModelTo(unsigned curindex, unsigned toindex)
{
  if (curindex == toindex || curindex >= size() || toindex >= size())
    return true;

  if (curindex < toindex) {  // Move forward
    std::rotate(rend() - curindex - 1, rend() - curindex, rend() - toindex);
  } else {  // Move back
    std::rotate(begin() + curindex, begin() + curindex + 1,
                begin() + toindex + 1);
  }

  modelslabels.setDirty();
  return false;
}

/**
 * @brief Checks if a models RF module has a unique id
 *
 * @param moduleIdx Module number 0 <-> NUM_MODULES
 * @param warn_buf Buffer to store the warning string
 * @param warn_buf_len Length of buffer
 * @return true ID is unique
 * @return false ID is used
 */

bool ModelsList::isModelIdUnique(uint8_t moduleIdx, char *warn_buf,
                                 size_t warn_buf_len)
{
  ModelCell *modelCell = modelslist.getCurrentModel();
  if (!modelCell || !modelCell->valid_rfData) {
    // in doubt, pretend it's unique
    return true;
  }

  uint8_t modelId = modelCell->modelId[moduleIdx];
  uint8_t type = modelCell->moduleData[moduleIdx].type;
  uint8_t subType = modelCell->moduleData[moduleIdx].subType;

  uint8_t additionalOnes = 0;
  char *curr = warn_buf;
  curr[0] = 0;

  bool hit_found = false;

  for (auto it = begin(); it != end(); ++it) {
    if (modelCell == *it) continue;

    if (!(*it)->valid_rfData) continue;

    if ((type != MODULE_TYPE_NONE) &&
        (type == (*it)->moduleData[moduleIdx].type) &&
        (subType == (*it)->moduleData[moduleIdx].subType) &&
        (modelId == (*it)->modelId[moduleIdx])) {
      // Hit found!
      hit_found = true;

      const char *modelName = (*it)->modelName;
      const char *modelFilename = (*it)->modelFilename;

      // you cannot rely exactly on WARNING_LINE_LEN so using WARNING_LINE_LEN-2
      // (-2 for the ",")
      if ((int)(warn_buf_len - 2 - (curr - warn_buf)) > LEN_MODEL_NAME) {
        if (warn_buf[0] != 0) curr = strAppend(curr, ", ");
        if (modelName[0] == 0) {
          size_t len = min<size_t>(strlen(modelFilename), LEN_MODEL_NAME);
          curr = strAppendFilename(curr, modelFilename, len);
        } else
          curr = strAppend(curr, modelName, LEN_MODEL_NAME);
      } else {
        additionalOnes++;
      }
    }
  }

  if (additionalOnes && (warn_buf_len - (curr - warn_buf) >= 7)) {
    curr = strAppend(curr, " (+");
    curr = strAppendUnsigned(curr, additionalOnes);
    curr = strAppend(curr, ")");
  }

  return !hit_found;
}

/**
 * @brief Finds the next free RF Module ID
 *
 * @param moduleIdx RF Module to search (Internal/External) 0 <-> NUM_MODULES
 * @return 0 No unused ID found
 * @return uint8_t Next free ID
 */

uint8_t ModelsList::findNextUnusedModelId(uint8_t moduleIdx)
{
  ModelCell *modelCell = modelslist.getCurrentModel();
  if (!modelCell || !modelCell->valid_rfData) {
    return 0;
  }

  uint8_t type = modelCell->moduleData[moduleIdx].type;
  uint8_t subType = modelCell->moduleData[moduleIdx].subType;

  uint8_t usedModelIds[(MAX_RXNUM + 7) / 8];
  memset(usedModelIds, 0, sizeof(usedModelIds));

  for (auto it = begin(); it != end(); it++) {
    if (modelCell == *it) continue;

    if (!(*it)->valid_rfData) continue;

    // match module type and RF protocol
    if ((type != MODULE_TYPE_NONE) &&
        (type == (*it)->moduleData[moduleIdx].type) &&
        (subType == (*it)->moduleData[moduleIdx].subType)) {
      uint8_t id = (*it)->modelId[moduleIdx];
      uint8_t mask = 1 << (id & 7u);
      usedModelIds[id >> 3u] |= mask;
    }
  }

  for (uint8_t id = 1; id <= getMaxRxNum(moduleIdx); id++) {
    uint8_t mask = 1u << (id & 7u);
    if (!(usedModelIds[id >> 3u] & mask)) {
      // found free ID
      return id;
    }
  }

  // failed finding something...
  return 0;
}
