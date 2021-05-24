/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "autoprecisioncombobox.h"

constexpr unsigned int APCB_DECIMALS_MIN {0};
constexpr unsigned int APCB_DECIMALS_MAX {6};

AutoPrecisionComboBox::AutoPrecisionComboBox(QWidget * parent):
  QComboBox(parent),
  AutoWidget()
{
  setFieldInit();
  init();
}

AutoPrecisionComboBox::AutoPrecisionComboBox(unsigned int & field, unsigned int minDecimals, unsigned int maxDecimals, bool padding, QString suffix, QWidget * parent, GenericPanel * panel):
  QComboBox(parent),
  AutoWidget()
{
  m_field = &field;
  setFieldInit(minDecimals, maxDecimals, padding, suffix, panel);
  init();
}

AutoPrecisionComboBox::AutoPrecisionComboBox(int & field, unsigned int minDecimals, unsigned int maxDecimals, bool padding, QString suffix, QWidget * parent, GenericPanel * panel):
  QComboBox(parent),
  AutoWidget()
{
  m_field = (unsigned int *)&field;
  setFieldInit(minDecimals, maxDecimals, padding, suffix, panel);
  init();
}

AutoPrecisionComboBox::~AutoPrecisionComboBox()
{
}

void AutoPrecisionComboBox::setField(int & field, unsigned int minDecimals, unsigned int maxDecimals, bool padding, QString suffix, GenericPanel * panel)
{
  m_field = (unsigned int *)&field;
  setFieldInit(minDecimals, maxDecimals, padding, suffix, panel);
  setPanel(panel);
}

void AutoPrecisionComboBox::setField(unsigned int & field, unsigned int minDecimals, unsigned int maxDecimals, bool padding, QString suffix, GenericPanel * panel)
{
  m_field = &field;
  setFieldInit(minDecimals, maxDecimals, padding, suffix, panel);
  setPanel(panel);
}

void AutoPrecisionComboBox::setMinDecimals(unsigned int minDecimals)
{
  m_minDecimals = rangecheckDecimals(minDecimals);
  if (m_minDecimals > m_maxDecimals)
    m_minDecimals = m_maxDecimals;
  if (!lock())
    updateList();
}

void AutoPrecisionComboBox::setMaxDecimals(unsigned int maxDecimals)
{
  m_maxDecimals = rangecheckDecimals(maxDecimals);
  if (m_maxDecimals < m_minDecimals)
    m_maxDecimals = m_minDecimals;
  if (!lock())
    updateList();
}

void AutoPrecisionComboBox::setDecimalsRange(unsigned int min, unsigned int max)
{
  setMinDecimals(min);
  setMaxDecimals(max);
}

void AutoPrecisionComboBox::setPadding(bool padding)
{
  m_padding = padding;
  if (!lock())
    updateList();
}

void AutoPrecisionComboBox::setSuffix(QString suffix)
{
  m_suffix = suffix.trimmed();
  if (!lock())
    updateList();
}

void AutoPrecisionComboBox::setValue(int value)
{
  if (!m_field)
    return;

  const unsigned int val = (unsigned int)value;
  if (*m_field != val) {
    *m_field = rangecheckDecimals(val);
    updateValue();
    emit currentDataChanged(value);
    dataChanged();
  }
}

void AutoPrecisionComboBox::setValue(unsigned int value)
{
  if (!m_field)
    return;

  if (*m_field != value) {
    *m_field = rangecheckDecimals(value);
    updateValue();
    emit currentDataChanged((int)value);
    dataChanged();
  }
}

void AutoPrecisionComboBox::updateValue()
{
  if (!m_field)
    return;

  if (!isValidDecimals(*m_field))
    return;

  if (*m_field > m_maxDecimals)
    setMaxDecimals(*m_field);

  setLock(true);
  setCurrentIndex(findData(*m_field));
  setLock(false);
}

void AutoPrecisionComboBox::setAutoIndexes()
{
  for (int i = 0; i < count(); ++i)
    setItemData(i, i);
  updateValue();
}

void AutoPrecisionComboBox::init()
{
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoPrecisionComboBox::onCurrentIndexChanged);
}

void AutoPrecisionComboBox::setFieldInit(unsigned int minDecimals, unsigned int maxDecimals, bool padding, QString suffix, GenericPanel * panel)
{
  setLock(true);
  setMinDecimals(minDecimals);
  setMaxDecimals(maxDecimals);
  setPadding(padding);
  setSuffix(suffix);
  setLock(false);
  setPanel(panel);
  updateList();
}

void AutoPrecisionComboBox::updateList()
{
  setLock(true);
  unsigned int i;
  int j;
  clear();

  for (i = APCB_DECIMALS_MIN, j = (int)m_maxDecimals; j >= 0; i++, j--) {
    if (i >= m_minDecimals) {
      QString str = QString("0.%1").arg(QString(i, '0'));
      if (m_padding)
        str.append(QString(j, '_'));
      if (m_suffix != "")
        str.append(QString(" %1").arg(m_suffix));
      addItem(str, i);
    }
  }

  setSizeAdjustPolicy(QComboBox::AdjustToContents);
  setLock(false);
}

void AutoPrecisionComboBox::onCurrentIndexChanged(int index)
{
  if (!m_field || index < 0 || lock())
    return;

  if (*m_field != itemData(index).toUInt()) {
    *m_field = itemData(index).toUInt();
    emit currentDataChanged(index);
    dataChanged();
  }
}

unsigned int AutoPrecisionComboBox::rangecheckDecimals(unsigned int decimals)
{
  unsigned int ret;
  if (decimals < APCB_DECIMALS_MIN)
    ret = APCB_DECIMALS_MIN;
  else if (decimals > APCB_DECIMALS_MAX)
    ret = APCB_DECIMALS_MAX;
  else
    ret = decimals;
  return ret;
}

bool AutoPrecisionComboBox::isValidDecimals(unsigned int value)
{
  if (value >= APCB_DECIMALS_MIN && value <= APCB_DECIMALS_MAX)
    return true;
  else
    return false;
}
