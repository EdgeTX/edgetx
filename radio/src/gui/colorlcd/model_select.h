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

#ifndef _MODEL_SELECT_H_
#define _MODEL_SELECT_H_

#include <algorithm>
#include <functional>

#include "libopenui.h"
#include "listbox.h"
#include "storage/modelslist.h"
#include "tabsgroup.h"

class ModelButton;

class ModelsPageBody : public FormWindow
{
 public:
  ModelsPageBody(Window *parent, const rect_t &rect);

  void update();

  void setLabels(LabelsVector labels)
  {
    selectedLabels = labels;
    update();
  }

  inline void setSortOrder(ModelsSortBy sortOrder)
  {
    modelslabels.setSortOrder(sortOrder);
    update();
  }

  void setLblRefreshFunc(std::function<void()> fnc)
  {
    refreshLabels = std::move(fnc);
  }

 protected:
  ModelsSortBy _sortOrder;
  bool isDirty = false;
  bool refresh = false;
  std::string selectedLabel;
  LabelsVector selectedLabels;
  ModelCell *focusedModel = nullptr;
  std::function<void()> refreshLabels = nullptr;

  void openMenu();
  void selectModel(ModelCell* model);
  void duplicateModel(ModelCell* model);
  void deleteModel(ModelCell* model);
  void editLabels(ModelCell* model);
  void saveAsTemplate(ModelCell *model);
};

class ModelLabelsWindow : public Page
{
 public:
  ModelLabelsWindow();

 protected:
  ModelsSortBy sort = DEFAULT_MODEL_SORT;
  char tmpLabel[LABEL_LENGTH + 1] = "\0";
  ListBox *lblselector;
  ModelsPageBody *mdlselector;
  std::string currentLabel;

  LabelsVector getLabels()
  {
    auto labels = modelslabels.getLabels();
    if (modelslabels.getUnlabeledModels().size() > 0)
      labels.emplace_back(STR_UNLABELEDMODEL);
    return labels;
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

  void newModel();
  void newLabel();
  void buildHead(PageHeader *window);
  void buildBody(FormWindow *window);
  void updateFilteredLabels(std::set<uint32_t> selected, bool setdirty = true);
  void labelRefreshRequest();
  void setTitle();
};

#endif  // _MODEL_SELECT_H_
