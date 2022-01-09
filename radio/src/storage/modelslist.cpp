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
using std::list;

#if defined(SDCARD_YAML)
#include "yaml/yaml_parser.h"
#include "yaml/yaml_modelslist.h"
#endif

#include "myeeprom.h"
#include "datastructs.h"
#include "pulses/modules_helpers.h"
#include "strhelpers.h"

#include <cstring>

ModelsList modelslist;

ModelCell::ModelCell(const char* name) : valid_rfData(false)
{
  strncpy(modelFilename, name, sizeof(modelFilename)-1);
  modelFilename[sizeof(modelFilename)-1] = '\0';
}

ModelCell::ModelCell(const char* name, uint8_t len) : valid_rfData(false)
{
  if (len > sizeof(modelFilename) - 1) len = sizeof(modelFilename) - 1;

  memcpy(modelFilename, name, len);
  modelFilename[len] = '\0';
}

ModelCell::~ModelCell()
{
}

void ModelCell::setModelName(char * name)
{
  strncpy(modelName, name, LEN_MODEL_NAME);
  modelName[LEN_MODEL_NAME] = '\0';

  if (modelName[0] == '\0') {
    char * tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *) memchr(modelName, '.',  LEN_MODEL_NAME);
    if (tmp != nullptr)
      *tmp = 0;
  }
}

void ModelCell::setModelName(char* name, uint8_t len)
{
  if (len > LEN_MODEL_NAME-1)
    len = LEN_MODEL_NAME-1;

  memcpy(modelName, name, len);
  modelName[len] = '\0';

  if (modelName[0] == 0) {
    char * tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *) memchr(modelName, '.',  LEN_MODEL_NAME);
    if (tmp != nullptr)
      *tmp = 0;
  }
}

void ModelCell::setModelId(uint8_t moduleIdx, uint8_t id)
{
  modelId[moduleIdx] = id;
}


void ModelCell::save(VfsFile& file)
{
#if !defined(SDCARD_YAML)
  file.puts(modelFilename);
  file.putc('\n');
#else
  file.puts("  - filename: \"");
  file.puts(modelFilename);
  file.puts("\"\n");

  file.puts("    name: \"");
  file.puts(modelName);
  file.puts("\"\n");
#endif
}

void ModelCell::setRfData(ModelData* model)
{
  for (uint8_t i = 0; i < NUM_MODULES; i++) {
    modelId[i] = model->header.modelId[i];
    setRfModuleData(i, &(model->moduleData[i]));
    TRACE("<%s/%i> : %X,%X,%X",
          strlen(modelName) ? modelName : modelFilename,
          i, moduleData[i].type, moduleData[i].rfProtocol, modelId[i]);
  }
  valid_rfData = true;
}

void ModelCell::setRfModuleData(uint8_t moduleIdx, ModuleData* modData)
{
  moduleData[moduleIdx].type = modData->type;
  if (modData->type != MODULE_TYPE_MULTIMODULE) {
    moduleData[moduleIdx].rfProtocol = (uint8_t)modData->rfProtocol;
  }
  else {
    // do we care here about MM_RF_CUSTOM_SELECTED? probably not...
    moduleData[moduleIdx].rfProtocol = modData->getMultiProtocol();
  }
}

bool ModelCell::fetchRfData()
{
#if !defined(SDCARD_YAML)
  //TODO: use g_model in case fetching data for current model
  //
  char buf[256];
  getModelPath(buf, modelFilename);

  FIL      file;
  uint16_t size;
  uint8_t  version;

  const char * err = openFileBin(buf, &file, &size, &version);
  if (err || version != EEPROM_VER) return false;

  FSIZE_t start_offset = f_tell(&file);

  UINT read;
  if ((f_read(&file, buf, LEN_MODEL_NAME, &read) != FR_OK) || (read != LEN_MODEL_NAME))
    goto error;

  setModelName(buf);

  // 1. fetch modelId: NUM_MODULES @ offsetof(ModelHeader, modelId)
  // if (f_lseek(&file, start_offset + offsetof(ModelHeader, modelId)) != FR_OK)
  //   goto error;
  if ((f_read(&file, modelId, NUM_MODULES, &read) != FR_OK) || (read != NUM_MODULES))
    goto error;

  // 2. fetch ModuleData: sizeof(ModuleData)*NUM_MODULES @ offsetof(ModelData, moduleData)
  if (f_lseek(&file, start_offset + offsetof(ModelData, moduleData)) != FR_OK)
    goto error;

  for(uint8_t i=0; i<NUM_MODULES; i++) {
    ModuleData modData;
    if ((f_read(&file, &modData, NUM_MODULES, &read) != FR_OK) || (read != NUM_MODULES))
      goto error;

    setRfModuleData(i, &modData);
  }

  valid_rfData = true;  
  f_close(&file);
  return true;
  
 error:
  f_close(&file);
  return false;

#else
  return false;
#endif
}

ModelsCategory::ModelsCategory(const char * name)
{
  strncpy(this->name, name, sizeof(this->name));
}

ModelsCategory::ModelsCategory(const char * name, uint8_t len)
{
  if (len > sizeof(this->name)-1)
      len = sizeof(this->name)-1;

  memcpy(this->name, name, len);
  this->name[len] = '\0';
}

ModelsCategory::~ModelsCategory()
{
  for (auto * model: *this) {
    delete model;
  }
}

ModelCell * ModelsCategory::addModel(const char * name)
{
  if (!name) return NULL;

  ModelCell * result = new ModelCell(name);
  push_back(result);
  return result;
}

void ModelsCategory::removeModel(ModelCell * model)
{
  delete model;
  remove(model);
}

void ModelsCategory::moveModel(ModelCell * model, int8_t step)
{
  ModelsCategory::iterator current = begin();
  for (; current != end(); current++) {
    if (*current == model) {
      break;
    }
  }

  ModelsCategory::iterator new_position = current;
  if (step > 0) {
    while (step >= 0 && new_position != end()) {
      new_position++;
      step--;
    }
  }
  else {
    while (step < 0 && new_position != begin()) {
      new_position--;
      step++;
    }
  }

  insert(new_position, 1, *current);
  erase(current);
}

int ModelsCategory::getModelIndex(const ModelCell* model)
{
  int idx = 0;
  for (auto m : *this) {
    if (model == m)
      return idx;

    ++idx;
  }

  return -1;
}

void ModelsCategory::save(VfsFile& file)
{
#if !defined(SDCARD_YAML)
  file.puts("[");
  file.puts(name);
  file.puts("]");
  file.putc('\n');
#else
  file.puts("- \"");
  file.puts(name);
  file.puts("\":\n");
#endif
  for (list<ModelCell *>::iterator it = begin(); it != end(); ++it) {
    (*it)->save(file);
  }
}

ModelsList::ModelsList()
{
  init();
}

ModelsList::~ModelsList()
{
  clear();
}

void ModelsList::init()
{
  loaded = false;
  currentCategory = nullptr;
  currentModel = nullptr;
  modelsCount = 0;
}

void ModelsList::clear()
{
  for (list<ModelsCategory *>::iterator it = categories.begin(); it != categories.end(); ++it) {
    delete *it;
  }
  categories.clear();
  init();
}

bool ModelsList::loadTxt()
{
  char line[LEN_MODELS_IDX_LINE+1];
  ModelsCategory * category = nullptr;
  ModelCell * model = nullptr;

  VfsError result = VirtualFS::instance().openFile(file, RADIO_MODELSLIST_PATH, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
  if (result == VfsError::OK) {

    // TXT reader
    while (readNextLine(line, LEN_MODELS_IDX_LINE)) {
      int len = strlen(line); // TODO could be returned by readNextLine
      if (len > 2 && line[0] == '[' && line[len-1] == ']') {
        line[len-1] = '\0';
        category = new ModelsCategory(&line[1]);
        categories.push_back(category);
      }
      else if (len > 0) {
        model = new ModelCell(line);
        if (!category) {
          category = new ModelsCategory("Models");
          categories.push_back(category);
        }
        category->push_back(model);
        if (!strncmp(line, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME)) {
          currentCategory = category;
          currentModel = model;
        }
        model->fetchRfData();
        modelsCount += 1;
      }
    }

    file.close();
    return true;
  }

  return false;
}

#if defined(SDCARD_YAML)
bool ModelsList::loadYaml()
{
  char line[LEN_MODELS_IDX_LINE+1];
  VirtualFS& vfs = VirtualFS::instance();
  VfsError result = vfs.openFile(file, MODELSLIST_YAML_PATH, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);

  if (result != VfsError::OK) {
    // move the YaML models list from the old to the new place
    VfsFileInfo fno;
    if (vfs.fstat(FALLBACK_MODELSLIST_YAML_PATH, fno) == VfsError::OK) {
      if (vfs.copyFile(FALLBACK_MODELSLIST_YAML_PATH, MODELSLIST_YAML_PATH) != VfsError::OK) {
        f_unlink(FALLBACK_MODELSLIST_YAML_PATH);
        result =
            vfs.openFile(file, MODELSLIST_YAML_PATH, VfsOpenFlags::OPEN_EXISTING | VfsOpenFlags::READ);
      }
    }
  }

  if (result == VfsError::OK) {
    // YAML reader
    TRACE("YAML modelslist reader");

    YamlParser yp;
    void* ctx = get_modelslist_iter(
        g_eeGeneral.currModelFilename,
        strnlen(g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME));

    yp.init(get_modelslist_parser_calls(), ctx);

    UINT bytes_read=0;
    while (file.read(line, sizeof(line), bytes_read) == VfsError::OK) {

      // reached EOF?
      if (bytes_read == 0)
        break;

      if (f_eof(&file)) yp.set_eof();
      if (yp.parse(line, bytes_read) != YamlParser::CONTINUE_PARSING)
        break;
    }

    file.close();
    return true;
  }

  return false;
}
#endif

bool ModelsList::load(Format fmt)
{
  if (loaded)
    return true;

  VirtualFS& vfs = VirtualFS::instance();
  bool res = false;
#if !defined(SDCARD_YAML)
  (void)fmt;
  res = loadTxt();
#else
  VfsFileInfo fno;
  if (fmt == Format::txt ||
      (fmt == Format::yaml_txt &&
       vfs.fstat(MODELSLIST_YAML_PATH, fno) != VfsError::OK &&
       vfs.fstat(FALLBACK_MODELSLIST_YAML_PATH, fno) != VfsError::OK)) {
    res = loadTxt();
  } else {
    res = loadYaml();
  }
#endif

  if (!currentModel) {
    if (categories.empty()) {
      currentCategory = new ModelsCategory("Models");
      categories.push_back(currentCategory);
    } else {
      currentCategory = *categories.begin();
      if (!currentCategory->empty()) {
        currentModel = *currentCategory->begin();
      }
    }
  }

  loaded = true;
  return res;
}

void ModelsList::save()
{
#if !defined(SDCARD_YAML)
  VfsError result = VirtualFS::instance().openFile(file, RADIO_MODELSLIST_PATH, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
#else
  VfsError result = VirtualFS::instance().openFile(file, MODELSLIST_YAML_PATH, VfsOpenFlags::CREATE_ALWAYS | VfsOpenFlags::WRITE);
#endif
  if (result != VfsError::OK) return;

  for (list<ModelsCategory *>::iterator it = categories.begin();
       it != categories.end(); ++it) {
    (*it)->save(file);
  }

  file.close();
}

void ModelsList::setCurrentCategory(ModelsCategory * cat)
{
  currentCategory = cat;
}

int ModelsList::getCurrentCategoryIdx() const
{
  if (!currentCategory)
    return -1;
  
  int idx = 0;
  for (auto cat : categories) {
    if (currentCategory == cat)
      return idx;

    ++idx;
  }

  return -1;
}

void ModelsList::setCurrentModel(ModelCell * cell)
{
  currentModel = cell;
  if (!currentModel->valid_rfData)
    currentModel->fetchRfData();
}

bool ModelsList::readNextLine(char * line, int maxlen)
{
  if (file.gets(line, maxlen) != NULL) {
    int curlen = strlen(line) - 1;
    if (line[curlen] == '\n') { // remove unwanted chars if file was edited using windows
      if (line[curlen - 1] == '\r') {
        line[curlen - 1] = 0;
      }
      else {
        line[curlen] = 0;
      }
    }
    return true;
  }
  return false;
}

ModelsCategory * ModelsList::createCategory(bool save)
{
  return createCategory("Category", save);
}

ModelsCategory * ModelsList::createCategory(const char* name, bool save)
{
  ModelsCategory * result = new ModelsCategory(name);
  categories.push_back(result);
  if (save) this->save();
  return result;
}

ModelCell * ModelsList::addModel(ModelsCategory * category, const char * name, bool save)
{
  ModelCell * result = category->addModel(name);
  modelsCount++;
  if (save) this->save();
  return result;
}

void ModelsList::removeCategory(ModelsCategory * category)
{
  modelsCount -= category->size();
  delete category;
  categories.remove(category);
}

void ModelsList::removeModel(ModelsCategory * category, ModelCell * model)
{
  category->removeModel(model);
  modelsCount--;
  save();
}

void ModelsList::moveModel(ModelsCategory * category, ModelCell * model, int8_t step)
{
  category->moveModel(model, step);
  save();
}

void ModelsList::moveModel(ModelCell * model, ModelsCategory * previous_category, ModelsCategory * new_category)
{  
  previous_category->remove(model);
  new_category->push_back(model);
  save();
}

bool ModelsList::isModelIdUnique(uint8_t moduleIdx, char* warn_buf, size_t warn_buf_len)
{
  ModelCell* modelCell = modelslist.getCurrentModel();
  if (!modelCell || !modelCell->valid_rfData) {
    // in doubt, pretend it's unique
    return true;
  }

  uint8_t modelId = modelCell->modelId[moduleIdx];
  uint8_t type = modelCell->moduleData[moduleIdx].type;
  uint8_t rfProtocol = modelCell->moduleData[moduleIdx].rfProtocol;

  uint8_t additionalOnes = 0;
  char* curr = warn_buf;
  curr[0] = 0;

  bool hit_found = false;
  const std::list<ModelsCategory*>& cats = modelslist.getCategories();
  std::list<ModelsCategory*>::const_iterator catIt = cats.begin();
  for (;catIt != cats.end(); catIt++) {
    for (ModelsCategory::const_iterator it = (*catIt)->begin(); it != (*catIt)->end(); it++) {
      if (modelCell == *it)
        continue;

      if (!(*it)->valid_rfData)
        continue;

      if ((type != MODULE_TYPE_NONE) &&
          (type       == (*it)->moduleData[moduleIdx].type) &&
          (rfProtocol == (*it)->moduleData[moduleIdx].rfProtocol) &&
          (modelId    == (*it)->modelId[moduleIdx])) {

        // Hit found!
        hit_found = true;

        const char * modelName = (*it)->modelName;
        const char * modelFilename = (*it)->modelFilename;

        // you cannot rely exactly on WARNING_LINE_LEN so using WARNING_LINE_LEN-2 (-2 for the ",")
        if ((warn_buf_len - 2 - (curr - warn_buf)) > LEN_MODEL_NAME) {
          if (warn_buf[0] != 0)
            curr = strAppend(curr, ", ");
          if (modelName[0] == 0) {
            size_t len = min<size_t>(strlen(modelFilename),LEN_MODEL_NAME);
            curr = strAppendFilename(curr, modelFilename, len);
          }
          else
            curr = strAppend(curr, modelName, LEN_MODEL_NAME);
        }
        else {
          additionalOnes++;
        }
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

uint8_t ModelsList::findNextUnusedModelId(uint8_t moduleIdx)
{
  ModelCell * modelCell = modelslist.getCurrentModel();
  if (!modelCell || !modelCell->valid_rfData) {
    return 0;
  }

  uint8_t type = modelCell->moduleData[moduleIdx].type;
  uint8_t rfProtocol = modelCell->moduleData[moduleIdx].rfProtocol;

  uint8_t usedModelIds[(MAX_RXNUM + 7) / 8];
  memset(usedModelIds, 0, sizeof(usedModelIds));
  
  const std::list<ModelsCategory *> & cats = modelslist.getCategories();
  for (auto catIt = cats.begin(); catIt != cats.end(); catIt++) {
    for (auto it = (*catIt)->begin(); it != (*catIt)->end(); it++) {
      if (modelCell == *it)
        continue;

      if (!(*it)->valid_rfData)
        continue;

      // match module type and RF protocol
      if ((type != MODULE_TYPE_NONE) &&
          (type       == (*it)->moduleData[moduleIdx].type) &&
          (rfProtocol == (*it)->moduleData[moduleIdx].rfProtocol)) {

        uint8_t id = (*it)->modelId[moduleIdx];
        uint8_t mask = 1 << (id & 7u);
        usedModelIds[id >> 3u] |= mask;
      }
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

void ModelsList::onNewModelCreated(ModelCell* cell, ModelData* model)
{
  cell->setModelName(model->header.name);
  cell->setRfData(model);

  uint8_t new_id = findNextUnusedModelId(INTERNAL_MODULE);
  model->header.modelId[INTERNAL_MODULE] = new_id;
  cell->setModelId(INTERNAL_MODULE, new_id);
}
