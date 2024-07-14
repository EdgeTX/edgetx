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

#include <QtCore>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

class FilteredItemModel;

/*
  This class takes advantage of RawSource's assumption that small values must be SOURCE_TYPE_NONE ie zero
  albeit anything other than value zero is unexpected. However there is no setter validation to stop unknown values.

  Also SOURCE_TYPE_NONE must be excluded from selection lists.
  It would not make sense anyway as it equates to value 0 and thus ambiguous.
*/

class SourceNumRef {

  Q_DECLARE_TR_FUNCTIONS(SourceNumRef)

  public:
    SourceNumRef() : srcNum(RawSource()) {}
    explicit SourceNumRef(int value) : srcNum(RawSource(value)) {}
    SourceNumRef(RawSourceType type, int index = 0) : srcNum(RawSource(type, index)) {}

    virtual ~SourceNumRef() {}

    int toValue() { return srcNum.toValue(); }

    QString toString(const ModelData * model = nullptr, const GeneralSettings * const generalSettings = nullptr,
                     Board::Type board = Board::BOARD_UNKNOWN, bool prefixCustomName = true) const;

    const bool isNumber() const { return srcNum.type == SOURCE_TYPE_NONE; }
    const bool isSource() const { return srcNum.type != SOURCE_TYPE_NONE; }
    static int getDefault(int useSource, int dflt);

  private:
    RawSource srcNum;
};

class SourceNumRefEditor : public QObject {

  Q_OBJECT

  public:
    explicit SourceNumRefEditor(int & srcNumValue, QCheckBox * chkUseSource, QSpinBox * sbxValue, QComboBox * cboValue,
                                int defValue, int minValue, int maxValue, int step,
                                ModelData & model, FilteredItemModel * sourceItemModel, QObject * parent = nullptr);

    virtual ~SourceNumRefEditor() {}

    void setLock(bool state) { lock = state; }
    void setVisible(bool state);
    void update();

  signals:
    void resized();

  protected slots:
    void chkUseSourceChanged(int state);
    void sbxValueChanged();
    void cboValueChanged(int index);

  protected:
    int &srcNumValue;
    QCheckBox *chkUseSource;
    QSpinBox *sbxValue;
    QComboBox *cboValue;
    int defValue;
    ModelData &model;
    bool lock;
};
