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
  if (rawSource.type == SOURCE_TYPE_NONE)
    return QString::number(rawSource.index);
  else
    return rawSource.toString(model, generalSettings, board, prefixCustomName);
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
 * RawSourceNumberUIManager
*/

RawSourceNumberUIManager::RawSourceNumberUIManager(RawSource & rawSource,
                            QCheckBox * chkUseSource, QSpinBox * sbxValue,
                            QComboBox * cboValue,
                            int defValue, int minValue, int maxValue, int step,
                            ModelData & model,
                            FilteredItemModel * sourceItemModel,
                            QObject * parent) :
  QObject(parent),
  rawSource(rawSource),
  chkUseSource(chkUseSource),
  sbxValue(sbxValue),
  cboValue(cboValue),
  defValue(defValue),
  model(model),
  lock(false)
{
  if (chkUseSource)
    connect(chkUseSource, &QCheckBox::checkStateChanged, this,
              &RawSourceNumberUIManager::chkUseSourceChanged);

  if (sbxValue) {
    sbxValue->setMinimum(minValue);
    sbxValue->setMaximum(maxValue);
    sbxValue->setSingleStep(step);
    sbxValue->setValue(defValue);
    connect(sbxValue, &QSpinBox::editingFinished, this,
              &RawSourceNumberUIManager::sbxValueChanged);
  }

  if (cboValue) {
    cboValue->setModel(sourceItemModel);
    cboValue->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    cboValue->setMaxVisibleItems(10);
    cboValue->setCurrentIndex(Helpers::getFirstPosValueIndex(cboValue));
    connect(cboValue, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              &RawSourceNumberUIManager::cboValueChanged);
  }

  update();
}

void RawSourceNumberUIManager::chkUseSourceChanged(int state)
{
  if (!lock) {
    rawSource = RawSourceNumber::getDefault(state, defValue);

    if (state == Qt::Checked) {
      cboValue->setCurrentIndex(cboValue->findData(rawSource.toValue()));
      if (cboValue->currentIndex() < 0)
        cboValue->setCurrentIndex(Helpers::getFirstPosValueIndex(cboValue));
    }
    else
      sbxValue->setValue(rawSource.toValue());

    update();
  }
}

void RawSourceNumberUIManager::sbxValueChanged()
{
  if (!lock) {
    rawSource.index = sbxValue->value();
    update();
  }
}

void RawSourceNumberUIManager::cboValueChanged(int index)
{
  if (!lock) {
    rawSource = cboValue->itemData(index).toInt();
    update();
  }
}

void RawSourceNumberUIManager::setVisible(bool state)
{
  chkUseSource->setVisible(state);
  sbxValue->setVisible(state);
}

void RawSourceNumberUIManager::update()
{
  lock = true;

  if (rawSource.isNumber()) {
    if (chkUseSource)
      chkUseSource->setChecked(false);

    if (sbxValue) {
      sbxValue->setValue(rawSource.index);
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
      cboValue->setCurrentIndex(cboValue->findData(rawSource.toValue()));
      cboValue->setVisible(true);
    }
  }

  emit resized();

  lock = false;
}
