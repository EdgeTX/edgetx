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

#ifndef _HELI_H_
#define _HELI_H_

#include "modeledit.h"

class CompoundItemModelFactory;
class FilteredItemModel;

namespace Ui {
  class Heli;
}

class HeliPanel : public ModelPanel
{
    Q_OBJECT

  public:
    HeliPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~HeliPanel();
    void update();

  private slots:
    void edited();
    void onItemModelAboutToBeUpdated();
    void onItemModelUpdateComplete();

  private:
    Ui::Heli *ui;
    FilteredItemModel * rawSourceFilteredModel;
    void connectItemModelEvents(const FilteredItemModel * itemModel);
};

#endif // _HELI_H_
