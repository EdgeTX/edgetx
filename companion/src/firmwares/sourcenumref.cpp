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

#include "sourcenumref.h"
#include "filtereditemmodels.h"
#include "helpers.h"

QString SourceNumRef::toString(const ModelData * model, const GeneralSettings * const generalSettings,
                     Board::Type board, bool prefixCustomName) const
{
  if (srcNum.type == SOURCE_TYPE_NONE)
    return QString::number(srcNum.index);
  else
    return srcNum.toString(model, generalSettings, board, prefixCustomName);
}

// static
int SourceNumRef::getDefault(int useSource, int dflt)
{
  if (useSource)
    return 0;
  else
    return dflt;
}


/*
 * SourceNumRefEditor
*/

SourceNumRefEditor::SourceNumRefEditor(int & srcNumValue, QCheckBox * chkUseSource, QSpinBox * sbxValue, QComboBox * cboValue,
                                       int defValue, int minValue, int maxValue, int step,
                                       ModelData & model, FilteredItemModel * sourceItemModel, QObject * parent) :
  QObject(parent),
  srcNumValue(srcNumValue),
  chkUseSource(chkUseSource),
  sbxValue(sbxValue),
  cboValue(cboValue),
  defValue(defValue),
  model(model),
  lock(false)
{
  if (chkUseSource) {
    connect(chkUseSource, &QCheckBox::stateChanged, this, &SourceNumRefEditor::chkUseSourceChanged);
  }

  if (sbxValue) {
    sbxValue->setMinimum(minValue);
    sbxValue->setMaximum(maxValue);
    sbxValue->setSingleStep(step);
    sbxValue->setValue(defValue);
    connect(sbxValue, &QSpinBox::editingFinished, this, &SourceNumRefEditor::sbxValueChanged);
  }

  if (cboValue) {
    cboValue->setModel(sourceItemModel);
    cboValue->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboValue->setMaxVisibleItems(10);
    cboValue->setCurrentIndex(Helpers::getFirstPosValueIndex(cboValue));
    connect(cboValue, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SourceNumRefEditor::cboValueChanged);
  }

  update();
}

void SourceNumRefEditor::chkUseSourceChanged(int state)
{
  if (!lock) {
    srcNumValue = SourceNumRef::getDefault(state, defValue);

    if (state == Qt::Checked) {
      cboValue->setCurrentIndex(cboValue->findData(srcNumValue));
      if (cboValue->currentIndex() < 0)
        cboValue->setCurrentIndex(Helpers::getFirstPosValueIndex(cboValue));
    }
    else
      sbxValue->setValue(srcNumValue);

    update();
  }
}

void SourceNumRefEditor::sbxValueChanged()
{
  if (!lock) {
    srcNumValue = sbxValue->value();
    update();
  }
}

void SourceNumRefEditor::cboValueChanged(int index)
{
  if (!lock) {
    srcNumValue = cboValue->itemData(index).toInt();
    update();
  }
}

void SourceNumRefEditor::setVisible(bool state)
{
  chkUseSource->setVisible(state);
  cboValue->setVisible(state);
  sbxValue->setVisible(state);
}

void SourceNumRefEditor::update()
{
  lock = true;

  if (SourceNumRef(srcNumValue).isNumber()) {
    if (chkUseSource)
      chkUseSource->setChecked(false);

    if (sbxValue) {
      sbxValue->setValue(srcNumValue);
      sbxValue->setVisible(true);
    }

    if (cboValue)
      cboValue->setVisible(false);
  }
  else {
    if (chkUseSource)
      chkUseSource->setChecked(true);

    if (sbxValue)
      sbxValue->setVisible(false);

    if (cboValue) {
      cboValue->setCurrentIndex(cboValue->findData(srcNumValue));
      cboValue->setVisible(true);
    }
  }

  emit resized();

  lock = false;
}
