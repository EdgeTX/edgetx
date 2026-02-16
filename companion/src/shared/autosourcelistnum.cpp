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

#include "autosourcelistnum.h"
#include "compounditemmodels.h"

#include <QHBoxLayout>

AutoSourceListNum::AutoSourceListNum(QWidget * parent) :
  QWidget(parent),
  AutoSource(),
  m_chkType(nullptr),
  m_sourceCB(nullptr),
  m_sourceDSB(nullptr),
  m_stack(nullptr)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // minimise padding around widget
  setContentsMargins(0, 0, 0, 0);
  // layout to group the requested ui objects
  QHBoxLayout *layout = new QHBoxLayout();
  // minimise padding around sub-widgets
  layout->setContentsMargins(0, 0, 0, 0);

  m_chkType = new QCheckBox(this);
  m_chkType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  m_chkType->setMinimumSize(200, 20);
  connect(m_chkType, &QCheckBox::checkStateChanged, this,
          &AutoSourceListNum::typeChanged);
  layout->addWidget(m_chkType);

  m_stack = new QStackedLayout();
  m_sourceCB = new AutoSourceCB();
  m_stack->addWidget(m_sourceCB);

  m_sourceDSB = new AutoSourceDSB();
  m_stack->addWidget(m_sourceDSB);

  layout->addLayout(m_stack);
  setLayout(layout);

  updateGeometry();
}

AutoSourceListNum::~AutoSourceListNum()
{
}

void AutoSourceListNum::setField(RawSource * field, CompoundItemModelFactory * itemModels,
                             int filter, bool isAvailable,
                             RawSource dflt, QString typeLabel, int min, int max,
                             int precision, QString prefix, QString suffix,
                             GenericPanel * panel)
{
  setField(field, itemModels->getItemModel(AbstractItemModel::IMID_RawSource),
           filter, isAvailable, dflt, typeLabel, min, max, precision, prefix, suffix, panel);
}

void AutoSourceListNum::setField(RawSource * field, AbstractItemModel * itemModel,
                             int filter, bool isAvailable,
                             RawSource dflt, QString typeLabel, int min, int max,
                             int precision, QString prefix, QString suffix,
                             GenericPanel * panel)
{
  AutoSource::setField(field, panel);
  m_sourceCB->setField(field, itemModel, filter, isAvailable, dflt, panel);
  m_sourceDSB->setField(field, dflt, min, max, precision, prefix, suffix, panel);
  m_chkType->setText(typeLabel);
  m_chkType->setChecked((getSource().type == SOURCE_TYPE_NUMBER) ? false : true);
  updateValue();
}

void AutoSourceListNum::setVisible(bool state)
{
  if (m_chkType)
    m_chkType->setVisible(state);

  if (m_sourceCB)
    m_sourceCB->setVisible(state);

  if (m_sourceDSB)
    m_sourceDSB->setVisible(state);
}

void AutoSourceListNum::shrink()
{
  updateGeometry();
  adjustSize();
  //resize(0, 0);
  emit resized();
}

void AutoSourceListNum::typeChanged(int state)
{
  if (!lock()) {
    if (state == Qt::Checked) {
      if (m_sourceCB) {
        m_sourceCB->setValueDefault();
      }
    } else {
      if (m_sourceDSB) {
        m_sourceDSB->setValueDefault();
      }
    }

    updateValue();
  }
}

void AutoSourceListNum::updateValue()
{
  if (getSource().type == SOURCE_TYPE_NUMBER) {
    if (m_sourceCB) {
      m_sourceCB->setVisible(false);
    }

    if (m_sourceDSB) {
      m_sourceDSB->setVisible(true);
      m_sourceDSB->updateValue();
      m_stack->setCurrentIndex(1);

    }
  } else {
    if (m_sourceCB) {
      m_sourceCB->setVisible(true);
      m_sourceCB->updateValue();
      m_stack->setCurrentIndex(0);
    }

    if (m_sourceDSB) {
      m_sourceDSB->setVisible(false);
    }
  }

  shrink();
}

