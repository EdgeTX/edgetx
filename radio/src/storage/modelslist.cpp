/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

ModelsList modelslist;

ModelCell::ModelCell(const char * name):
  valid_rfData(false)
{
  strncpy(modelFilename, name, sizeof(modelFilename));
}

ModelCell::ModelCell(const char * name, uint8_t len)
  : valid_rfData(false)
{
  if (len > sizeof(modelFilename)-1)
    len = sizeof(modelFilename)-1;

  memcpy(modelFilename, name, len);
  modelFilename[len] = '\0';
}

ModelCell::~ModelCell()
{
}

void ModelCell::setModelName(char * name)
{
  strncpy(modelName, name, LEN_MODEL_NAME);
  if (modelName[0] == 0) {
    char * tmp;
    strncpy(modelName, modelFilename, LEN_MODEL_NAME);
    tmp = (char *) memchr(modelName, '.',  LEN_MODEL_NAME);
    if (tmp)
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
    if (tmp != NULL)
      *tmp = 0;
  }

  //resetBuffer();
}

void ModelCell::setModelId(uint8_t moduleIdx, uint8_t id)
{
  modelId[moduleIdx] = id;
}

// void ModelCell::resetBuffer()
// {
//   if (buffer) {
//     delete buffer;
//     buffer = NULL;
//   }
// }

// const BitmapBuffer * ModelCell::getBuffer()
// {
//   if (!buffer) {
//     loadBitmap();
//   }
//   return buffer;
// }

// void ModelCell::loadBitmap()
// {
//   uint8_t version;

//   PACK(struct {
//          ModelHeader header;
//          TimerData timers[MAX_TIMERS];
//        }) partialmodel;

//   const char * error = NULL;
//   BitmapBuffer * tmp_buffer;

//   if (strncmp(modelFilename, g_eeGeneral.currModelFilename, LEN_MODEL_FILENAME) == 0) {
//     memcpy(&partialmodel.header, &g_model.header, sizeof(partialmodel));
//   }
//   else {
// #if !defined (SDCARD_YAML)
//     error = readModel(modelFilename, (uint8_t *)&partialmodel.header, sizeof(partialmodel), &version);
//     // LEN_BITMAP_NAME has now 4 bytes more
//     if (version <= 218) {
//       memmove(partialmodel.timers, &(partialmodel.header.bitmap[10]), sizeof(TimerData)*MAX_TIMERS);
//       memclear(&(partialmodel.header.bitmap[10]), 4);
//     }
// #else
//     memset(&partialmodel.header,0,sizeof(partialmodel));
// #endif
//   }

//   if ((modelName[0] == 0) && ! error)
//     setModelName(partialmodel.header.name); // resets buffer!!!

//   tmp_buffer = new BitmapBuffer(BMP_RGB565, MODELCELL_WIDTH, MODELCELL_HEIGHT);
//   if (tmp_buffer == NULL) {
//     return;
//   }
//   tmp_buffer->clear(TEXT_BGCOLOR);

//   if (error) {
//     tmp_buffer->drawText(5, 2, "(Invalid Model)", TEXT_COLOR);
//     tmp_buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
//   }
//   else {
//     char timer[LEN_TIMER_STRING];
//     tmp_buffer->drawSizedText(5, 2, modelName, /*LEN_MODEL_NAME*/strlen(modelName),
//                           SMLSIZE|TEXT_COLOR);

//     getTimerString(timer, 0);
//     for (uint8_t i = 0; i < MAX_TIMERS; i++) {
//       if (partialmodel.timers[i].mode != 0 && partialmodel.timers[i].persistent) {
//         getTimerString(timer, partialmodel.timers[i].value, 1);
//         break;
//       }
//     }
//     tmp_buffer->drawText(101, 40, timer, TEXT_COLOR);
//     for (int i=0; i<4; i++) {
//       tmp_buffer->drawBitmapPattern(104+i*11, 25, LBM_SCORE0, TITLE_BGCOLOR);
//     }
//     GET_FILENAME(filename, BITMAPS_PATH, partialmodel.header.bitmap, "");
//     const BitmapBuffer * bitmap = BitmapBuffer::load(filename);
//     if (bitmap) {
//       tmp_buffer->drawScaledBitmap(bitmap, 5, 24, 56, 32);
//       delete bitmap;
//     }
//     else {
//       tmp_buffer->drawBitmapPattern(5, 23, LBM_LIBRARY_SLOT, TEXT_COLOR);
//     }
//   }
//   tmp_buffer->drawSolidHorizontalLine(5, 19, 143, LINE_COLOR);

//   buffer = tmp_buffer;
// }

void ModelCell::save(FIL* file)
{
#if !defined(SDCARD_YAML)
  f_puts(modelFilename, file);
  f_putc('\n', file);
#else
  f_puts("  - filename: ", file);
  f_puts(modelFilename, file);
  f_putc('\n', file);

  f_puts("    name: ", file);
  f_puts(modelName, file);
  f_putc('\n', file);
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

  const char * err = openFile(buf, &file, &size, &version);
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

void ModelsCategory::save(FIL * file)
{
#if !defined(SDCARD_YAML)
  f_puts("[", file);
  f_puts(name, file);
  f_puts("]", file);
  f_putc('\n', file);
#else
  f_puts("- ", file);
  f_puts(name, file);
  f_putc('\n', file);
  //f_puts("  models:\n", file);
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

bool ModelsList::load()
{
  char line[LEN_MODELS_IDX_LINE+1];
  ModelsCategory * category = nullptr;
  ModelCell * model = nullptr;

  if (loaded)
    return true;

#if !defined(SDCARD_YAML)
  FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_OPEN_EXISTING | FA_READ);
#else
  FRESULT result = f_open(&file, RADIO_MODELSLIST_YAML_PATH, FA_OPEN_EXISTING | FA_READ);
#endif
  if (result == FR_OK) {

#if !defined(SDCARD_YAML)
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
#else
    // YAML reader
    TRACE("YAML modelslist reader");

    YamlParser yp; //TODO: move to re-usable buffer
    yp.init(get_modelslist_parser_calls(), get_modelslist_iter());

    UINT bytes_read=0;
    while (f_read(&file, line, sizeof(line), &bytes_read) == FR_OK) {

      // reached EOF?
      if (bytes_read == 0)
        break;

      if (yp.parse(line, bytes_read) != YamlParser::CONTINUE_PARSING)
        break;
    }

    // debug output
    //modelslist.dump();
#endif

    f_close(&file);
  }

  if (!currentModel) {
    if (model) {
      currentModel = model;
    }
    else {
      category = new ModelsCategory("Models");
      categories.push_back(category);
    }
    currentCategory = category;
  }

  loaded = true;
  return true;
}

void ModelsList::save()
{
#if !defined(SDCARD_YAML)
  FRESULT result = f_open(&file, RADIO_MODELSLIST_PATH, FA_CREATE_ALWAYS | FA_WRITE);
#else
  FRESULT result = f_open(&file, RADIO_MODELSLIST_YAML_PATH, FA_CREATE_ALWAYS | FA_WRITE);
#endif

  if (result != FR_OK) {
    return;
  }

  for (list<ModelsCategory *>::iterator it = categories.begin();
       it != categories.end(); ++it) {
    (*it)->save(&file);
  }

  f_close(&file);
}

void ModelsList::setCurrentCategory(ModelsCategory * cat)
{
  currentCategory = cat;
}

void ModelsList::setCurrentModel(ModelCell * cell)
{
  currentModel = cell;
  if (!currentModel->valid_rfData)
    currentModel->fetchRfData();
}

bool ModelsList::readNextLine(char * line, int maxlen)
{
  if (f_gets(line, maxlen, &file) != NULL) {
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

ModelCell * ModelsList::addModel(ModelsCategory * category, const char * name)
{
  ModelCell * result = category->addModel(name);
  modelsCount++;
  save();
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
