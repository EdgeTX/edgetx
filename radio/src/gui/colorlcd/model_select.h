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

#include "tabsgroup.h"
#include "storage/modelslist.h"
#include "libopenui.h"
#include <algorithm>
#include "listbox.h"

constexpr int MAX_LABEL_SIZE = 30;

class ModelButton;

class ModelsPageBody : public FormWindow
{
  public:
    ModelsPageBody(Window *parent, const rect_t &rect);

    void setLabels(LabelsVector labels) {selectedLabels = labels; update();}
    void update(int selected = -1);

    inline void setRefresh() { refresh = true; }
    inline void setSortOrder(ModelsSortBy sortOrder) { _sortOrder = sortOrder; setRefresh(); }

    void checkEvents() override;

    void deleteLater(bool detach = true, bool trash = true) override
    {
      innerWindow.deleteLater(true, false);

      Window::deleteLater(detach, trash);
    }

    void paint(BitmapBuffer *dc) override;
    void setLblRefreshFunc(std::function<void()> fnc) {refreshLabels = std::move(fnc);}

  protected:
    ModelsSortBy _sortOrder;
    bool isDirty = false;
    bool refresh = false;
    FormWindow innerWindow;
    void initPressHandlers(ModelButton *button, ModelCell *model, int index);
    std::string selectedLabel;
    LabelsVector selectedLabels;
    std::function<void()> refreshLabels = nullptr;
};

class ModelLabelsWindow : public Page {
  public:
    ModelLabelsWindow();

#if defined (HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

  protected:
    ModelsSortBy sort = NAME_ASC;
    char tmpLabel[MAX_LABEL_SIZE + 1] = "\0";
    ListBox *lblselector;
    ModelsPageBody *mdlselector;
    TextButton *newButton;
    std::string currentLabel;

    LabelsVector getLabels()
    {
      auto labels = modelsLabels.getLabels();
      if(modelsLabels.getUnlabeledModels().size() > 0)
        labels.emplace_back(STR_UNLABELEDMODEL);
      return labels;
    }

    void buildHead(PageHeader *window);
    void buildBody(FormWindow *window);
    void labelRefreshRequest();
};


#endif // _MODEL_SELECT_H_
