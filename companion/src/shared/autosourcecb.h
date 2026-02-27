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

#include "autosource.h"

#include <QComboBox>

class AbstractItemModel;
class CompoundItemModelFactory;
class FilteredItemModel;

class AutoSourceCB : public QComboBox, public AutoSource
{
  Q_OBJECT

  public:
    explicit AutoSourceCB(QWidget * parent = nullptr);
    virtual ~AutoSourceCB();

    // AutoWidget
    virtual void updateValue() override;

    void setField(RawSource * field, RawSource dflt = RawSource(),
                  GenericPanel * panel = nullptr,
                  CompoundItemModelFactory * itemModels = nullptr, int filter = 0,
                  bool isAvailable = true);

    void setField(RawSource * field, RawSource dflt = RawSource(),
                  GenericPanel * panel = nullptr,
                  AbstractItemModel * itemModel = nullptr, int filter = 0,
                  bool isAvailable = true);

    void setFilter(int filter, bool isAvailable = true);

    void setModelFilter(CompoundItemModelFactory * itemModels, int filter = 0,
                        bool isAvailable = true);

    void setModelFilter(AbstractItemModel * itemModel, int filter = 0,
                        bool isAvailable = true);

  signals:
    void dataChanged(int value);

  protected slots:
    void onCurrentIndexChanged(int index);

  private:
    AbstractItemModel *m_itemModel;
    FilteredItemModel *m_filteredSource;

    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
