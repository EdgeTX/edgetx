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

#include "autohexspinbox.h"

AutoHexSpinBox::AutoHexSpinBox(QWidget * parent):
  QSpinBox(parent),
  AutoWidget(),
  m_field(nullptr)
{
  setRange(0, AUTOHEXSPINBOX_MAX_VALUE);
  connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoHexSpinBox::onValueChanged);
}

AutoHexSpinBox::~AutoHexSpinBox()
{
}

void AutoHexSpinBox::setField(unsigned int & field, const unsigned int min, const unsigned int max, GenericPanel * panel)
{
  m_field = &field;
  setRange(min, max);
  setPanel(panel);
  updateValue();
}

void AutoHexSpinBox::setField(unsigned int & field, GenericPanel * panel)
{
  m_field = &field;
  setRange(0, AUTOHEXSPINBOX_MAX_VALUE);
  setPanel(panel);
  updateValue();
}

void AutoHexSpinBox::setRange(const unsigned int min, const unsigned int max)
{
  QSpinBox::setRange(min, max);

  m_length = QString("%1").arg(max, 0, 16).size();

  m_validator = new QRegExpValidator(QRegExp(QString("[0-9A-Fa-f]{1,%1}").arg(m_length)), this);
}

void AutoHexSpinBox::updateValue()
{
  if (m_field) {
    setLock(true);
    setValue(*m_field);
    setLock(false);
  }
}

void AutoHexSpinBox::onValueChanged(int value)
{
  if (m_field && !lock()) {
    if (*m_field != (unsigned)value) {
      *m_field = (unsigned)value;
      emit currentDataChanged(value);
      dataChanged();
    }
  }
}

// reimplemented
QValidator::State AutoHexSpinBox::validate(QString &text, int &pos) const
{
  return m_validator->validate(text, pos);
}

int AutoHexSpinBox::valueFromText(const QString &text) const
{
  return text.toInt(0, 16);
}

QString AutoHexSpinBox::textFromValue(int value) const
{
  return QString("%1").arg(value, m_length, 16, QChar('0')).toUpper();
}
