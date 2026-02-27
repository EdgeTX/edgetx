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
#include <QTimer>

AutoSourceListNum::AutoSourceListNum(QWidget * parent) :
  QWidget(parent),
  AutoSource()
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // minimise padding around widget
  setContentsMargins(0, 0, 0, 0);
  // layout to group the requested ui objects
  QHBoxLayout *layout = new QHBoxLayout();
  // minimise padding around sub-widgets
  layout->setContentsMargins(0, 0, 0, 0);

  m_chkType = new QCheckBox();
  //m_chkType->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  //m_chkType->setMinimumSize(200, 20);
  connect(m_chkType, &QCheckBox::checkStateChanged, this, &AutoSourceListNum::typeChanged);
  layout->addWidget(m_chkType);

  m_stack = new QStackedWidget();

  m_sourceDSB = new AutoSourceDSB();
  m_stack->addWidget(m_sourceDSB);

  m_sourceCB = new AutoSourceCB();
  m_stack->addWidget(m_sourceCB);

  m_stack->setCurrentIndex(0);
  layout->addWidget(m_stack);
  setLayout(layout);
  shrink();
}

AutoSourceListNum::~AutoSourceListNum()
{
}

void AutoSourceListNum::setField(RawSource * field, RawSource dflt, GenericPanel * panel,
                                 CompoundItemModelFactory * itemModels,
                                 int filter, bool isAvailable, QString typeLabel,
                                 int min, int max, int precision,
                                 QString prefix, QString suffix)
{
  setField(field, dflt, panel, itemModels->getItemModel(AbstractItemModel::IMID_RawSource),
           filter, isAvailable, typeLabel, min, max, precision, prefix, suffix);
}

void AutoSourceListNum::setField(RawSource * field, RawSource dflt, GenericPanel * panel,
                                 AbstractItemModel * itemModel,
                                 int filter, bool isAvailable, QString typeLabel,
                                 int min, int max, int precision,
                                 QString prefix, QString suffix)
{
  setLock(true);
  AutoSource::setField(field, RawSource(SOURCE_TYPE_NUMBER), panel);
  m_sourceCB->setField(field, RawSource(SOURCE_TYPE_NONE), panel, itemModel, filter, isAvailable);
  m_sourceDSB->setField(field, RawSource(SOURCE_TYPE_NUMBER), panel, min, max, precision, prefix, suffix);
  m_chkType->setText(typeLabel);
  m_chkType->setChecked(field->type != SOURCE_TYPE_NUMBER);
  m_stack->setCurrentIndex(field->type != SOURCE_TYPE_NUMBER ? 1 : 0);
  setLock(false);
  updateValue();
}

void AutoSourceListNum::setVisible(bool state)
{
  m_chkType->setVisible(state);
  m_sourceCB->setVisible(state);
  m_sourceDSB->setVisible(state);
}

void AutoSourceListNum::shrink()
{
  updateGeometry();
  QTimer::singleShot(0, this, &AutoSourceListNum::adjustSize);
  //adjustSize();
  //resize(0, 0);
  emit resized();
}

void AutoSourceListNum::typeChanged(int state)
{
  if (!lock()) {
    if (state == Qt::Checked) {
      m_sourceCB->setValueToDefault();
      m_stack->setCurrentIndex(1);
    } else {
      m_sourceDSB->setValueToDefault();
      m_stack->setCurrentIndex(0);
    }

    updateValue();
  }
}

void AutoSourceListNum::updateValue()
{
  if (m_stack->currentIndex() == 0)
    m_sourceDSB->updateValue();
  else
    m_sourceCB->updateValue();

  shrink();
}

