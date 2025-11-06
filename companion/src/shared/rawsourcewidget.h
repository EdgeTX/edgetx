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
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

class CompoundItemModelFactory;
class FilteredItemModel;
class CurveImageWidget;

/*
  Not all widgets are required to manage the source type and its value

  These flags control which widgets are created and
  to compact the ui space required for the dynamic group

  As a mimimum UI_FLAG_LIST or UI_FLAG_VALUE must be chosen

  Sequence:
  curve type combobox (curve reference)
  source list or value active checkbox (if both requested)
  source list combobox
  value (spinbox or double spinbox - based on parameters)
  curve image (double click opens editor)

  Note: source list and value are visibily mutually exclusive
*/

constexpr int UI_FLAG_NONE        { 0 };
constexpr int UI_FLAG_CURVE_TYPE  { 1 << 1 };
constexpr int UI_FLAG_LIST        { 1 << 2 };
constexpr int UI_FLAG_VALUE       { 1 << 3 };
constexpr int UI_FLAG_CURVE_IMAGE { 1 << 4 };
constexpr int UI_FLAG_DEFAULT     { UI_FLAG_LIST };
constexpr int UI_FLAG_LIST_VALUE  { UI_FLAG_LIST | UI_FLAG_VALUE };
constexpr int UI_FLAG_ALL         { UI_FLAG_CURVE_TYPE | UI_FLAG_LIST |
                                    UI_FLAG_VALUE | UI_FLAG_CURVE_IMAGE };

class RawSourceWidget : public QWidget {

  Q_OBJECT

  public:
    explicit RawSourceWidget(QWidget * parent,
                              ModelData * modelData,
                              CompoundItemModelFactory * sharedItemModels,
                              RawSource * src,
                              int listFilter,
                              int flags,
                              RawSource dflt = RawSource(),
                              QString typeLabel = "",
                              int min = -1000,
                              int max = 1000,
                              int precision = 1,
                              double step = -1.0,
                              QString prefix = "",
                              QString suffix = "",
                              bool isAvailable = true);

    explicit RawSourceWidget(QWidget * parent,
                             ModelData * modelData,
                             CompoundItemModelFactory * sharedItemModels,
                             RawSource * src,
                             int listFilter) :
      RawSourceWidget(parent, modelData, sharedItemModels, src, listFilter, UI_FLAG_LIST) {}

    // if the widget is included in a .ui definition file
    // only the parent widget is passed to the constructor
    explicit RawSourceWidget(QWidget * parent);

    virtual ~RawSourceWidget();

    void init();

    // if the widget is included in a .ui definition file
    // this function must be called after setupUi(this)
    // as only the parent widget is passed to the constructor
    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              RawSource * src,
              int listFilter,
              int flags,
              RawSource dflt = RawSource(),
              QString typeLabel = "",
              int min = -1000,
              int max = 1000,
              int precision = 1,
              double step = -1.0,
              QString prefix = "",
              QString suffix = "",
              bool isAvailable = true);

    void init(ModelData * modelData,
              CompoundItemModelFactory * sharedItemModels,
              RawSource * src,
              int listFilter) {
      init(modelData, sharedItemModels, src, listFilter, UI_FLAG_LIST); }

    void setDefault(RawSource value);
    void setFilter(int filter, bool isAvailable = true);
    void update();
    void updateMinMax(const int min, const int max);

  signals:
    void dataChanged(RawSource src);
    void resized();

  protected slots:
    void sourceChanged(int index);
    void typeChanged(int state);
    void valueChanged();

  protected:
    bool lock;

  private:
    ModelData *modelData;
    RawSource *src;
    RawSource dflt;
    int flags;
    bool isAvailable;
    int listFilter;
    QString typeLabel;
    int min;
    int max;
    int precision;
    double step;
    QString prefix;
    QString suffix;

    FilteredItemModel *fimSource;
    QCheckBox *chkType;
    QComboBox *cboSource;
    QSpinBox *sbValue;
    QDoubleSpinBox *dsbValue;
    CurveImageWidget *curveImage;

    void connectItemModelEvents(const FilteredItemModel * itemModel);
    void shrink();
};
