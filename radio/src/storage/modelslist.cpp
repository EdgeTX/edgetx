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

#include "edgetx.h"
#include "storage/sdcard_yaml.h"
#include "yaml/yaml_datastructs.h"
#include "yaml/yaml_labelslist.h"
#include "yaml/yaml_modelslist.h"
#include "yaml/yaml_parser.h"
#include "strhelpers.h"

#ifdef DEBUG_LABELS
#define TRACE_LABELS(...) TRACE(__VA_ARGS__)
#else
#define TRACE_LABELS(...)
#endif

LAYOUT_SIZE(LABEL_TRUNCATE_LENGTH, 21, 16)

ModelsList modelCellManager;

//-----------------------------------------------------------------------------

// TODO - Fix me, ideally there should be no limitations
static void escapeCSV(std::string &str)
{
  strReplaceAll(str, "/", "//");
  strReplaceAll(str, ",", "/c");
}

// TODO - Fix me, ideally there should be no limitations
static void unEscapeCSV(std::string &str)
{
  strReplaceAll(str, "//", "/");
  strReplaceAll(str, "/c", ",");
}

// TODO - Fix me, ideally there should be no limitations
static void removeYAMLChars(std::string &str)
{
  std::string s;
  for (int i = 0; labelExcludedChars[i]; i += 1) {
    s = labelExcludedChars[i];
    strReplaceAll(str, s, "");
  }
}

/**
 * @brief Convert a Comma separated string list to LabelsVector
 *
 * @param str Comma separated string of labels
 * @return vector of all labels
 */

static LabelsVector fromCSV(const char* str)
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

/**
 * @brief Create a string from a vector of labels joined with a given seperator
 *
 * @param labels Vector of the labels
 * @param seperator String to seperate labels in teh result
 * @return labels list as a string
 */

static std::string joinLabels(const LabelsVector &labels, const char* seperator)
{
  // If creating a CSV escape each label character for saving to YAML
  bool escape = (strcmp(seperator, ",") == 0);
  std::string res;
  for (auto lbl: labels) {
    if (lbl.size() > 0) {
      if (escape)
        escapeCSV(lbl);
      if (res.size() > 0)
        res += seperator;
      res += lbl;
    }
  }
  return res;
}

/**
 * @brief Convert a vector of labels to a comma separated list
 *
 * @param labels Vector of the labels
 * @return labels list in CSV format
 */

static std::string toCSV(const LabelsVector &labels)
{
  return joinLabels(labels, ",");
}

/**
 * @brief Creates a Hash based on the file information
 *
 * @param buffer Buffer to store the output data
 * @param finfo Files info handle
 * @return char* Pointer to buffer supplied
 */

static char *FILInfoToHexStr(char buffer[FILE_HASH_LENGTH + 1], FILINFO *finfo)
{
  char *str = buffer;
  for (unsigned int i = 0; i < sizeof(FInfoH); i++) {
    sprintf(str, "%02x", *((uint8_t *)finfo + i));
    str += 2;
  }
  return buffer;
}

//-----------------------------------------------------------------------------

void LabelsMap::addLabel(uint8_t idx)
{
  // Add a label - maintain sort order
  if (!hasLabel(idx)) {
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
      if (*it > idx) {
        labelMap.insert(it, idx);
        return;
      }
    }
    labelMap.emplace_back(idx);
  }
}

void LabelsMap::removeLabel(uint8_t idx)
{
  // Remove a label from the model (if present)
  if (hasLabel(idx))
    labelMap.erase(std::remove(labelMap.begin(), labelMap.end(), idx), labelMap.end());
}

bool LabelsMap::deleteLabel(uint8_t idx)
{
  // Remove a label that has been deleted from the labels list
  // Update label indexes to account for deletion
  bool changed = false;
  // Remove label if present - return changed state
  if (hasLabel(idx)) {
    removeLabel(idx);
    changed = true;
  }
  // Fix label indexes after a label was deleted
  for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
    if (*it > idx)
      *it = *it - 1;
  }
  return changed;
}

void LabelsMap:: moveLabelUp(uint8_t idx)
{
  // Swap position of label at idx with idx - 1
  // Label position was moved so index has changed

  // Can't move top label
  if (idx == 0) return;
  // See which labels are present
  bool hasIdx1 = hasLabel(idx);
  bool hasIdx2 = hasLabel(idx - 1);

  // Both set so nothing to do
  if (hasIdx1 && hasIdx2) return;
  // Neither set so nothing to do
  if (!hasIdx1 && !hasIdx2) return;

  // Remove set label and add the other one
  if (hasIdx1) {
    removeLabel(idx);
    addLabel(idx - 1);
  } else {
    removeLabel(idx - 1);
    addLabel(idx);
  }
}

bool LabelsMap::hasLabel(uint8_t idx)
{
  return std::find(labelMap.begin(), labelMap.end(), idx) != labelMap.end();
}

/**
 * @brief Create a list of all the label names
 *
 * @param mdl Model to search
 * @return LabelsVector aka vector<string> of all labels selected by a model
 */

LabelsVector LabelsMap::getLabels()
{
  LabelsVector rv;
  for (auto it = labelMap.begin(); it != labelMap.end(); ++it) {
    rv.push_back(modelCellManager.getLabelByIndex(*it));
  }
  return rv;
}

//-----------------------------------------------------------------------------

ModelCell::ModelCell(const char *fileName)
{
  strAppend(modelFilename, fileName, LEN_MODEL_FILENAME);
}

void ModelCell::setModelName(const char *name)
{
  if (name && name[0]) {
    strAppend(modelName, name, LEN_MODEL_NAME);
  } else {
    setDefaultName();
  }
}

void ModelCell::setRfData(ModelHeader *header, ModuleData* modData)
{
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    modelId[i] = header->modelId[i];
    moduleData[i].type = modData[i].type;
    if (modData[i].type != MODULE_TYPE_MULTIMODULE) {
      moduleData[i].subType = (uint8_t)modData[i].subType;
    } else {
      // do we care here about MM_RF_CUSTOM_SELECTED? probably not...
      moduleData[i].subType = modData[i].multi.rfProtocol;
    }
    TRACE("<%s/%i> : %X,%X,%X", strlen(modelName) ? modelName : modelFilename,
          i, moduleData[i].type, moduleData[i].subType, modelId[i]);
  }
  valid_rfData = true;
}

void ModelCell::setUniqueName()
{
  // Generate a new unique name from the current name
  char s[LEN_MODEL_NAME + 1];
  strAppend(s, modelName, LEN_MODEL_NAME);
  int endPos = strlen(s);
  bool truncate = endPos > LEN_MODEL_NAME - 3;
  if (truncate) endPos -= 3;
  for (int i = 1; i < 99; i += 1) {
    if (i == 10 && truncate) endPos -= 1;
    sprintf(s + endPos, "(%d)", i);
    if (modelCellManager.getModelWithName(s) == nullptr) {
      strAppend(modelName, s);
      return;
    }
  }

  // Fallback
  setDefaultName();
}

void ModelCell::setDefaultName()
{
  // Reset name back to default 'MODELxx' where 'xx' is the model number
  // from the file name
  int idx = -1;
  sscanf(modelFilename, "model%d", &idx);
  if (idx > 0)
    sprintf(modelName, "MODEL%02d" MODEL_FILENAME_SUFFIX, idx);
  else
    strAppend(modelName, modelFilename, LEN_MODEL_NAME);
  char* tmp = (char *)strrchr(modelName, '.');
  if (tmp != nullptr) *tmp = 0;
}

void ModelCell::duplicateFrom(ModelCell* src)
{
  // Copy everything except the filename and hash from the src model
  strAppend(modelName, src->modelName, LEN_MODEL_NAME);
#if LEN_BITMAP_NAME > 0
  strAppend(modelBitmap, src->modelBitmap, LEN_BITMAP_NAME);
#endif
  lastOpened = src->lastOpened;
  _isDirty = src->_isDirty;
  valid_rfData = src->valid_rfData;
  for (int i = 0; i < NUM_MODULES; i += 1) {
    moduleData[i].type = src->moduleData[i].type;
    moduleData[i].subType = src->moduleData[i].subType;
  }
  labelsMap = src->labelsMap;
}

void ModelCell::updateModelFile()
{
  // Update labels and write model file
  std::string csv = toCSV(getLabels());
  if (this == modelCellManager.getCurrentModel()) {
    strAppend(g_model.header.labels, csv.c_str(), LABELS_LENGTH - 1);
    storageDirty(EE_MODEL);
  } else {
    writeModelLabels(csv.c_str());
    updateFinfoHash();
  }
}

/**
 * @brief Opens a YAML file, reads the data and updates the ModelCell
 */

void ModelCell::updateModelCell()
{
  TRACE("Labels: Updating model %s", modelFilename);

  PartialModel partial;
  memclear(&partial, sizeof(PartialModel));
  readModelYaml(modelFilename, (uint8_t*)&partial, sizeof(PartialModel));

  strAppend(modelName, partial.header.name, LEN_MODEL_NAME);
  strAppend(modelBitmap, partial.header.bitmap, LEN_BITMAP_NAME);

  modelCellManager.addLabelsToModel(partial.header.labels, this);

  // Save Module Data
  setRfData(&partial.header, partial.moduleData);

  _isDirty = false;
}

void ModelCell::updateFinfoHash()
{
  // Currently the file hashes are only synchronised between the labels.yml file
  // and the model files on startup. Any updates to model files while running
  // do not update the labels.yml copy of the file hash.

  // This function can be used to re-synch a model file to the labels file hash
  // Currently disabled to maintain compatability with existing firmware.

#if 0
  // Recalculate file info hash and save to SD
  FILINFO finfo;
  char path[256];
  getModelPath(path, modelFilename);
  FRESULT result = f_stat(path, &finfo);
  if (result == FR_OK) {
    FILInfoToHexStr(modelFinfoHash, &finfo);
    storageDirty(EE_LABELS);
  }
#endif
}

void ModelCell::addLabel(uint8_t idx)
{
  labelsMap.addLabel(idx);
}

void ModelCell::removeLabel(uint8_t idx)
{
  labelsMap.removeLabel(idx);
}

void ModelCell::deleteLabel(uint8_t idx)
{
  if (labelsMap.deleteLabel(idx))
    updateModelFile();
}

void ModelCell::moveLabelUp(uint8_t idx)
{
  labelsMap.moveLabelUp(idx);
}

bool ModelCell::hasLabel(const std::string& label)
{
  int idx = modelCellManager.getIndexByLabel(label);
  return labelsMap.hasLabel(idx);
}

/**
 * @brief Update the labels in an existing model yaml file on SD card
 * 
 * @param labels new labels
 * @return true Success
 * @return false Failure
 */
bool ModelCell::writeModelLabels(const char* labels)
{
  TRACE("Updating labels in %s",modelFilename);

  UINT bytes_cnt;
  char buf[512];
  char tempPath[256];
  FIL out;
  FIL file;

  // Read exiting model header
  PartialModel partial;
  memclear(&partial, sizeof(PartialModel));
  readModelYaml(modelFilename, (uint8_t*)&partial, sizeof(PartialModel));

  // Update name (may have been changed when duplicating model)
  strAppend(partial.header.name, modelName, LEN_MODEL_NAME);
  // Update header with new labels
  strAppend(partial.header.labels, labels, LABELS_LENGTH - 1);
  // Remove module data - only want to write the header
  memclear(&partial.moduleData, sizeof(ModuleData) * NUM_MODULES);

  // Write new header to a temp file
  getModelPath(tempPath, "tmp.yml");
  if (writeFileYaml(tempPath, get_partialmodel_nodes(), (uint8_t *)&partial, 0) != NULL) {
    TRACE("ERROR writing temp model file");
    f_unlink(tempPath);
    return false;
  }

  // Open tmp file for appending
  FRESULT result = f_open(&out, tempPath, FA_OPEN_EXISTING | FA_WRITE | FA_OPEN_APPEND);
  if (result != FR_OK) {
    TRACE("ERROR opening temp file");
    f_unlink(tempPath);
    return false;
  }

  // Copy rest of model yaml from original file to temp file
  getModelPath(buf, modelFilename);
  result = f_open(&file, buf, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) {
    f_close(&out);
    f_unlink(tempPath);
    return false;
  }

  // Read old header - assumes header fits within first 512 bytes
  // header has not changed significantly for a long time so should be safe
  result = f_read(&file, buf, sizeof(buf), &bytes_cnt);
  if (result != FR_OK) {
    f_close(&out);
    f_close(&file);
    f_unlink(tempPath);
    return false;
  }

  // Bound searches/writes below by bytes actually read, not sizeof(buf) -
  // the rest of buf is uninitialized for a file shorter than 512 bytes.
  int len = (int)bytes_cnt;

  // Find header section
  int n = 0;
  while (n < len - 7 && strncmp(&buf[n], "header:", 7) != 0)
    n += 1;

  if (n >= len - 7) {
    TRACE("ERROR model header not found in %s", modelFilename);
    f_close(&out);
    f_close(&file);
    f_unlink(tempPath);
    return false;
  }

  // Skip header section - look for next section after 'header:'
  do {
    // Skip current line
    while (n < len && buf[n] != '\n') n += 1;
    n += 1;
  } while ((n < len) && buf[n] == ' ');

  if (n >= len) {
    TRACE("ERROR could not match model header in %s", modelFilename);
    f_close(&out);
    f_close(&file);
    f_unlink(tempPath);
    return false;
  }

  // Write remainder of first buffer after header - check for short
  // writes (e.g. SD full), which f_write() can return FR_OK for.
  UINT written;
  UINT to_write = (UINT)(len - n);
  result = f_write(&out, &buf[n], to_write, &written);
  bool short_write = (result == FR_OK && written != to_write);

  // Block copy the rest of the original file to the temp file
  while (result == FR_OK && !short_write && bytes_cnt != 0) {
    result = f_read(&file, buf, sizeof(buf), &bytes_cnt);
    if (result == FR_OK && bytes_cnt != 0) {
      result = f_write(&out, buf, bytes_cnt, &written);
      short_write = (result == FR_OK && written != bytes_cnt);
    }
  }

  f_close(&out);
  f_close(&file);

  if (result != FR_OK || short_write) {
    TRACE("ERROR copying to temp file");
    f_unlink(tempPath);
    return false;
  }

  // Delete original file and rename temp file
  getModelPath(buf, modelFilename);
  f_unlink(buf);
  if (f_rename(tempPath, buf) != FR_OK) {
    TRACE("ERROR renaming temp file to %s", modelFilename);
    return false;
  }

  return true;
}

/**
 * @brief Returns a bullet separated list of the labels, used in model setup
 *
 * @param curmod Model Cell
 * @param noresults String to return when no labels found
 * @return std::string of all Labels, if no results return
 */

std::string ModelCell::getBulletLabelString(const char *noresults)
{
  std::string lbls = joinLabels(getLabels(), "\u2022");

  if(lbls.size() == 0)
    return noresults;

  if(lbls.size() > LABEL_TRUNCATE_LENGTH) {
    lbls = lbls.substr(0, LABEL_TRUNCATE_LENGTH);
    lbls += "...";
  }

  return lbls;
}

//-----------------------------------------------------------------------------

ModelsList::ModelsList() {}

ModelsList::~ModelsList() { clear(); }

void ModelsList::clear()
{
  labels.clear();

  for(ModelCell *mdl: *this) {
    delete(mdl);
  }
  std::vector<ModelCell *>::clear();

  currentModel = nullptr;
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
  clear();

  DEBUG_TIMER_START(debugTimerYamlScan);

  FRESULT result;
  FIL file;

  // Scan all models in folder
  DIR moddir;
  FILINFO finfo;
  if (f_opendir(&moddir, MODELS_PATH) == FR_OK) {
    for (;;) {
      result = f_readdir(&moddir, &finfo);
      if (result != FR_OK || finfo.fname[0] == 0) break;
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
      auto model = addModel(finfo.fname, false);
      FILInfoToHexStr(model->modelFinfoHash, &finfo);
      TRACE_LABELS("File - %s \r\n  HASH - %s", finfo.fname, model->modelFinfoHash);
    }
    f_closedir(&moddir);
  }

#if defined(DEBUG_TIMERS)
  DEBUG_TIMER_SAMPLE(debugTimerYamlScan);
  TRACE("Labels: Time to scan models folder %luus",
        debugTimers[debugTimerYamlScan].getLast());
#endif

  // Scan labels.yml
  result = f_open(&file, LABELSLIST_YAML_PATH, FA_OPEN_EXISTING | FA_READ);
  if (result == FR_OK) {
    char line[32];
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

  // Activate current model from radio settings
  auto mdl = getModel(g_eeGeneral.currModelFilename);
  if (mdl)
    setCurrentModel(mdl);

  // Scan all models, to see if update needed
  bool updatelabelsyml = false;
  for (auto &model : *this) {
    // Open and read each model if it's marked as dirty
    if (model->_isDirty) {
      updatelabelsyml = true;
      model->updateModelCell();
    }
  }

  // If no labels found. Add a favorites label
  if (getLabels().size() == 0) {
    addLabel(STR_FAVORITE_LABEL);
  }

  // If any items differed save the file
  if (updatelabelsyml == true) {
    TRACE_LABELS("LABELS.YML Wasn't in sync. Needs to be saved");
    storageDirty(EE_LABELS);
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
  bool res = loadYaml();

  if (!currentModel) {
    TRACE("ERROR no Current Model Found");
    if (size()) {
      setCurrentModel(at(0));
      strAppend(g_eeGeneral.currModelFilename, at(0)->modelFilename, LEN_MODEL_FILENAME);
      TRACE("  - Set current model to first available");
    } else {
      TRACE("  - No Models Found, making a new one");
      // No models found, make a new one
      auto model = addModel(createModel(), true);
      setCurrentModel(model);
      updateCurrentModelCell();
    }
  }

  return res;
}

/**
 * @brief Writes labels.yml file
 * @param newOrder vector<string> - Forces a save of this label order. leave empty to use current
 *                 order.
 * @return const char* NULL on success
 * @return const char* Error String on failure
 */

const char *ModelsList::save()
{
  FIL file;

  FRESULT result =
      f_open(&file, LABELSLIST_YAML_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if (result != FR_OK) return "Couldn't open labels.yml for writing";

  // Save current selection
  f_puts("Labels:\r\n", &file);

  LabelsVector newOrder = modelCellManager.getLabels();
  for (auto &lbl : newOrder) {
    f_printf(&file, "  \"%s\":\r\n", lbl.c_str());
    if (isLabelFiltered(lbl))
      f_printf(&file, "    selected: true\r\n", lbl.c_str());
  }

  // Save current sort order
  f_printf( &file, "Sort: %d\r\n", modelCellManager.sortOrder());

  f_puts("Models:\r\n", &file);
  for (auto &model : *this) {
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

    f_printf(&file, "    labels: \"%s\"\r\n", toCSV(model->getLabels()).c_str());

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
  storageDirty(EE_LABELS);

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
    strAppend(currentModel->modelBitmap, g_model.header.bitmap, LEN_BITMAP_NAME);
#endif
    strAppend(currentModel->modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME);
    currentModel->setModelName(g_model.header.name);
    currentModel->setRfData(&g_model.header, g_model.moduleData);
    storageDirty(EE_LABELS);
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
  if (copyCell != nullptr) {
    // Duplicate all data except filename and hash
    result->duplicateFrom(copyCell);
  }

  // Add to the ModelsList
  push_back(result);

  // Force save to labels.yml
  if (save) storageDirty(EE_LABELS);

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

  storageDirty(EE_LABELS);

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
  if (!currentModel || !currentModel->valid_rfData) {
    // in doubt, pretend it's unique
    return true;
  }

  uint8_t modelId = currentModel->modelId[moduleIdx];
  uint8_t type = currentModel->moduleData[moduleIdx].type;
  uint8_t subType = currentModel->moduleData[moduleIdx].subType;

  uint8_t additionalOnes = 0;
  char *curr = warn_buf;
  curr[0] = 0;

  bool hit_found = false;

  for (auto it = begin(); it != end(); ++it) {
    if (currentModel == *it) continue;

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
  if (!currentModel || !currentModel->valid_rfData) {
    return 0;
  }

  uint8_t type = currentModel->moduleData[moduleIdx].type;
  uint8_t subType = currentModel->moduleData[moduleIdx].subType;

  uint8_t usedModelIds[(MAX_RXNUM + 7) / 8];
  memset(usedModelIds, 0, sizeof(usedModelIds));

  for (auto it = begin(); it != end(); it++) {
    if (currentModel == *it) continue;

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

void ModelsList::setSortOrder(ModelsSortBy sortby)
{
  if (sortby < SORT_COUNT && sortby >= NO_SORT) {
    _sortOrder = sortby;
    storageDirty(EE_LABELS);
  }
}

/**
 * @brief Sorts a ModelsVector
 *
 * @param mv ModelsVector to sort
 * @param sortby NO_SORT, NAME_ASC, NAME_DES, DATE_ASC, DATE_DES,
 */

void ModelsList::sortModelsBy(ModelsVector &mv, ModelsSortBy sortby)
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
 * @brief Finds a ModelCell by filename
 *
 * @param filename Model file name
 * @return ModelCell pointer if found, else null
 */

ModelCell* ModelsList::getModel(const char* filename)
{
  for (auto model : *this)
    if (strncmp(model->modelFilename, filename, LEN_MODEL_FILENAME) == 0)
      return model;
  return nullptr;
}

/**
 * @brief Finds a ModelCell by model name
 *
 * @param filename Model file name
 * @return ModelCell pointer if found, else null
 */

ModelCell* ModelsList::getModelWithName(const char* name)
{
  for (auto model : *this)
    if (strncmp(model->modelName, name, LEN_MODEL_NAME) == 0)
      return model;
  return nullptr;
}

/**
 * @brief Gets all models which don't have any labels selected
 *
 * @param sortby
 * @return ModelsVector vector<ModelCell>
 */

ModelsVector ModelsList::getUnlabeledModels()
{
  ModelsVector unlabeledModels;
  for (auto model : *this) {
    if (model->getLabels().size() == 0)
      unlabeledModels.emplace_back(model);
  }
  sortModelsBy(unlabeledModels, sortOrder());
  return unlabeledModels;
}

/**
 * @brief Returns a sorted list of all models
 */

ModelsVector ModelsList::getAllModels()
{
  ModelsVector all = *this;
  sortModelsBy(all, sortOrder());
  return all;
}

/**
 * @brief Return the index for a label string
 */

int ModelsList::getIndexByLabel(const std::string &str)
{
  auto a = std::find(labels.begin(), labels.end(), str);
  return a == labels.end() ? -1 : a - labels.begin();
}

/**
 * @brief Return the string for a label index
 */

std::string ModelsList::getLabelByIndex(uint16_t index)
{
  if (index < (uint16_t)labels.size())
    return labels.at(index);
  else
    return std::string();
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

int ModelsList::addLabel(std::string lbl)
{
  if (lbl == STR_UNLABELEDMODEL) return -1;

  // Limit maximum label length, TODO... Truncate UTF8 Properly
  removeYAMLChars(lbl);
  lbl = lbl.substr(0, LABEL_LENGTH);
  if (lbl.size() == 0) return -1;

  // Add a new label if it doesn't already exist in the list
  // Returns the index to the label
  int ind = getIndexByLabel(lbl);
  if (ind < 0) {
    labels.push_back(lbl);
    storageDirty(EE_LABELS);
    TRACE_LABELS("Added a label %s", lbl.c_str());
    return labels.size() - 1;
  }
  return ind;
}

/**
 * @brief Removes a label
 * @details Remove a label from the list.
 *          The label is removed from any models that are using it.
 *
 * @param label Label to be removed
 * @param progress function to update progress bar (if needed)
 */

void ModelsList::removeLabel(
    const std::string &label,
    std::function<void(const char *file, int progress)> progress)
{
  int idx = getIndexByLabel(label);
  if (idx >= 0) {
    labels.erase(std::remove(labels.begin(), labels.end(), label), labels.end());

    int i = 1;
    for (auto it = begin(); it != end(); ++it, i += 1) {
      if (progress != nullptr) progress((*it)->modelFilename, (i * 100) / size());
      (*it)->deleteLabel(idx);
    }

    if (progress != nullptr) progress("", 100); // Kill progress dialog

    // If no more labels, add a favorite
    if (labels.size() == 0)
      addLabel(STR_FAVORITE_LABEL);

    storageDirty(EE_LABELS);
  }
}

/**
 * @brief Moves a labels order up one place
 *
 * @param idx Index of the label to move
 */

void ModelsList::moveLabelUp(uint16_t idx)
{
  if (idx == 0 || idx >= labels.size())
    return;

  std::swap(labels[idx - 1], labels[idx]);

  int i = 1;
  for (auto it = begin(); it != end(); ++it, i += 1) {
    (*it)->moveLabelUp(idx);
  }

  storageDirty(EE_LABELS);
}

/**
 * @brief Update a label string
 *
 * @param from label to rename
 * @param to new label string
 * @param progress function to update progress bar (if needed)
 */

void ModelsList::renameLabel(const std::string &from, const std::string& to,
        std::function<void(const char *file, int progress)> progress)
{
  int idx = getIndexByLabel(from);
  if (idx >= 0) {
    labels[idx] = to;
    int i = 1;
    for (auto it = begin(); it != end(); ++it, i += 1) {
      if (progress != nullptr) progress((*it)->modelFilename, (i * 100) / size());
      if ((*it)->hasLabel(idx))
        (*it)->updateModelFile();
    }
    storageDirty(EE_LABELS);
  }

  if (progress != nullptr) progress("", 100); // Kill progress dialog
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

bool ModelsList::addLabelToModel(const std::string &lbl, ModelCell *cell, bool update)
{
  // First check that there aren't too many labels on this model
  LabelsVector lbs = cell->getLabels();
  lbs.push_back(lbl);
  if(toCSV(lbs).size() > LABELS_LENGTH - 1) {
    TRACE("Cannot add the %s label to the model. Too many labels", lbl.c_str());
    return true;
  }

  int labelindex = addLabel(lbl);
  cell->addLabel(labelindex);
  if (update) cell->updateModelFile();

  storageDirty(EE_LABELS);

  return false;
}

/**
 * @brief Adds a set of labels (in a CSV string) to a model
 *
 * @param lbls Labels to be added
 * @param cell Model to add the labels
 */

void ModelsList::addLabelsToModel(const char* lbls, ModelCell* cell)
{
  LabelsVector vec = fromCSV(lbls);
  for(const auto &lbl : vec) {
    TRACE_LABELS("  Adding the label - %s", lbl.c_str());
    if (addLabelToModel(lbl, cell, false)) return;
  }
}

/**
 * @brief Removes a label from a model
 *
 * @param label Label to be removed
 * @param cell Model to remove the label from
 */

void ModelsList::removeLabelFromModel(const std::string &label, ModelCell *cell)
{
  int idx = modelCellManager.getIndexByLabel(label);
  if (idx >= 0) {
    cell->removeLabel(idx);
    cell->updateModelFile();  // Write labels into model
    storageDirty(EE_LABELS);
  }
}

/**
 * @brief Adds a label to the filter, used in yaml_labelslist on load
 *
 * @param label Label to be added
 */

void ModelsList::addFilteredLabel(const std::string &lbl)
{
  int ind = getIndexByLabel(lbl);
  if (ind >= 0) filtlbls.insert(ind);
}

bool ModelsList::isLabelFiltered(const std::string &lbl)
{
  return (filtlbls.find(getIndexByLabel(lbl)) != filtlbls.end());
}

/**
 * @brief Update label filter selection
 *
 * @param newFilter List of label indexes to filter on
 */

void ModelsList::setFilteredLabels(std::set<uint32_t> newFilter)
{
  filtlbls = std::move(newFilter);
  storageDirty(EE_LABELS);
}

/**
 * @brief Returns all models that match the selected labels
 *
 * @param lbls Labels to search
 * @return ModelsVector aka vector<ModelCell*> of all models matching the
 * label filters
 */

ModelsVector ModelsList::getFilteredModelList(const LabelsVector &lbls)
{
  if (lbls.size() == 0) return ModelsVector();

  // Requesting only Unlabeled models
  if (lbls.size() == 1 && lbls.at(0) == STR_UNLABELEDMODEL)
    return getUnlabeledModels();

  ModelsVector rv;

  for (const auto &mdl : *this) {
    bool hasAllLabels = true;
    bool hasAnyLabels = false;
    bool favLabelIncluded = false;
    bool hasFavLabel = false;
    for (const auto &lbl : lbls) {
      if (lbl == STR_UNLABELEDMODEL)  // If requesting unlabeled model ignore it
        break;
      bool hasLabel = mdl->hasLabel(lbl);
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

  sortModelsBy(rv, sortOrder());
  return rv;
}
