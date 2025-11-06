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

#include "rawsource.h"

#include <QWidget>
#include <QComboBox>

class CompoundItemModelFactory;
class FilteredItemModel;

constexpr int UI_FLAG_NONE   { 0 };
constexpr int UI_FLAG_SOURCE { 1 << 1 };

class RawSourceWidget : public QWidget {

  Q_OBJECT

  public:
    explicit RawSourceWidget(QWidget * parent = nullptr,
                             ModelData * modelData = nullptr,
                             CompoundItemModelFactory * sharedItemModels = nullptr,
                             RawSource * rawSource = nullptr,
                             RawSource dflt = RawSource(),
                             int filterFlags = 0);

    virtual ~RawSourceWidget();

    void init(ModelData * modelData, CompoundItemModelFactory * sharedItemModels,
              RawSource * rawSource, RawSource dflt = RawSource(),
              int filterFlags = 0);
    void setDefault(RawSource value);
    void setFilterFlags(int flags);
    void setVisible(bool state);

  signals:
    void dataChanged();
    void resize();

  protected slots:

  protected:
    ModelData *modelData;
    RawSource *rawSource;
    RawSource dflt;
    bool lock;

    void update(bool notify = true);

  private:
    FilteredItemModel *fimRawSource;
    QComboBox *cboSource;

    void connectItemModelEvents(const FilteredItemModel * itemModel);
};
