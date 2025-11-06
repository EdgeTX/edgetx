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

class AutoSourceCB : public QComboBox, virtual public AutoSource
{
  Q_OBJECT

  public:
    explicit AutoSourceCB(QWidget * parent = nullptr);
    virtual ~AutoSourceCB();

    // QComboBox
    virtual void setModel(QAbstractItemModel * model) override;

    // AutoWidget
    virtual void updateValue() override;

    void setDefault(RawSource dflt);

    void setField(RawSource * field, RawSource dflt, GenericPanel * panel = nullptr);

    void setField(RawSource * field, CompoundItemModelFactory * itemModels, int filter,
                  bool isAvailable = true, RawSource dflt = RawSource(),
                  GenericPanel * panel = nullptr);

    void setField(RawSource * field, AbstractItemModel * itemModel, int filter,
                  bool isAvailable = true, RawSource dflt = RawSource(),
                  GenericPanel * panel = nullptr);

    void setFilter(int filter, bool isAvailable = true);

    void setModelFilter(CompoundItemModelFactory * itemModels, int filter,
                        bool isAvailable = true);

    void setModelFilter(AbstractItemModel * itemModel, int filter,
                        bool isAvailable = true);

    void setValueDefault();

  signals:
    void dataChanged(int value);

  protected slots:
    void onCurrentIndexChanged(int index);

  private:
    RawSource m_dflt;
    AbstractItemModel *m_itemModel;
    FilteredItemModel *m_filteredSource;

    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
