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

#include "autospinbox.h"

AutoSpinBox::AutoSpinBox(QWidget * parent):
  QSpinBox(parent),
  AutoWidget(),
  m_field(nullptr)
{
  connect(this, QOverload<int>::of(&QSpinBox::valueChanged), this, &AutoSpinBox::onValueChanged);
}

AutoSpinBox::~AutoSpinBox()
{
}

void AutoSpinBox::setField(int & field, GenericPanel * panel)
{
  m_field = &field;
  setFieldInit(panel);
}

void AutoSpinBox::setField(unsigned int & field, GenericPanel * panel)
{
  m_field = (int *)&field;
  setFieldInit(panel);
}

void AutoSpinBox::setFieldInit(GenericPanel * panel)
{
  setPanel(panel);
  updateValue();
}

void AutoSpinBox::updateValue()
{
  if (m_field) {
    setLock(true);
    setValue(*m_field);
    setLock(false);
  }
}

void AutoSpinBox::onValueChanged(int value)
{
  if (m_field && !lock()) {
    if (*m_field != value) {
      *m_field = value;
      emit currentDataChanged(value);
      dataChanged();
    }
  }
}
