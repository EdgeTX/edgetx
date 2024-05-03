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

#include "listbox.h"
#include "storage/modelslist.h"
#include "page.h"

class ModelsPageBody;
class ModelLayoutButton;

class ModelLabelsWindow : public Page
{
 public:
  ModelLabelsWindow();

  static LAYOUT_VAL(BUTTONS_HEIGHT, 36, 36)
  static LAYOUT_VAL(LABELS_WIDTH, 132, 0)
  static LAYOUT_VAL(LABELS_HEIGHT, 0, 152)
  static LAYOUT_VAL(LABELS_ROW, 0, 1)
  static LAYOUT_VAL(MODELS_COL, 1, 0)
  static LAYOUT_VAL(MODELS_ROW_CNT, 2, 1)
  static LAYOUT_VAL(BUTTONS_ROW, 1, 2)
  static LAYOUT_VAL(NEW_BTN_W, 60, 60)
  static LAYOUT_VAL(LAYOUT_BTN_XO, 105, 105)
  static LAYOUT_VAL(LAYOUT_BTN_YO, 6, 6)

 protected:
  ModelsSortBy sort = DEFAULT_MODEL_SORT;
  char tmpLabel[LABEL_LENGTH + 1] = "\0";
  ListBox *lblselector;
  ModelsPageBody *mdlselector;
  ModelLayoutButton *mdlLayout;
  std::string currentLabel;

  LabelsVector getLabels()
  {
    auto labels = modelslabels.getLabels();
    if (modelslabels.getUnlabeledModels().size() > 0)
      labels.emplace_back(STR_UNLABELEDMODEL);
    return labels;
  }

  void newModel();
  void newLabel();
  void buildHead(Window *window);
  void buildBody(Window *window);
  void updateFilteredLabels(std::set<uint32_t> selected, bool setdirty = true);
  void labelRefreshRequest();
  void setTitle();

  void moveLabel(int selected, int direction);

#if defined(HARDWARE_KEYS)
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPG(bool isNext);
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif
};
