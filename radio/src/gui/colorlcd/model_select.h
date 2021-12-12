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

constexpr int MAX_LABEL_SIZE = 20;

class ModelButton;

class ModelsPageBody : public FormWindow
{
 public:
  ModelsPageBody(Window *parent, const rect_t &rect);

  void setLabels(LabelsVector labels)
  {
    selectedLabels = labels;
    update();
  }
  void update(int selected = -1);

  inline void setSortOrder(ModelsSortBy sortOrder)
  {
    _sortOrder = sortOrder;
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
  std::function<void()> refreshLabels = nullptr;

  void modelMenu(ModelButton* btn);
  void selectModel(ModelButton* btn);
  void duplicateModel(ModelCell* model);
  void deleteModel(ModelButton* btn);
  void editLabels(ModelCell* model);
};

class ModelLabelsWindow : public Page
{
 public:
  ModelLabelsWindow();

 protected:
  ModelsSortBy sort = DEFAULT_MODEL_SORT;
  char tmpLabel[MAX_LABEL_SIZE + 1] = "\0";
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

  void newModel();
  void buildHead(PageHeader *window);
  void buildBody(FormWindow *window);
  void updateFilteredLabels(std::set<uint32_t> selected, bool setdirty = true);
  void labelRefreshRequest();
};

class ProgressDialog : public Dialog
{
  uint32_t lastUpdate = 0;
  Progress *progress;

  std::function<void()> onClose;

 public:
  ProgressDialog(Window *parent, std::string title,
                 std::function<void()> onClose);

  void updateProgress(const char *filename, int percentage);

  // disable keys
  void onEvent(event_t) override;

 protected:
  std::string _title;
};

#endif  // _MODEL_SELECT_H_
