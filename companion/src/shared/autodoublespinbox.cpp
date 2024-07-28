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

#include "autodoublespinbox.h"

AutoDoubleSpinBox::AutoDoubleSpinBox(QWidget * parent):
  QDoubleSpinBox(parent),
  AutoWidget(),
  m_field(nullptr),
  m_offset(0)
{
  connect(this, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &AutoDoubleSpinBox::onValueChanged);
}

AutoDoubleSpinBox::~AutoDoubleSpinBox()
{
}

void AutoDoubleSpinBox::setField(int & field, GenericPanel * panel)
{
  m_field = &field;
  setPanel(panel);
  updateValue();
}

void AutoDoubleSpinBox::setField(unsigned int & field, GenericPanel * panel)
{
  m_field = (int *)&field;
  setPanel(panel);
  updateValue();
}

void AutoDoubleSpinBox::setDecimals(int prec)
{
  QDoubleSpinBox::setDecimals(prec);
  updateValue();
}

void AutoDoubleSpinBox::setOffset(int offset)
{
  m_offset = offset;
  updateValue();
}

void AutoDoubleSpinBox::updateValue()
{
  if (m_field) {
    setLock(true);
    setValue(float(*m_field + m_offset) / multiplier());
    setLock(false);
  }
}

int AutoDoubleSpinBox::multiplier()
{
  switch (decimals()) {
    case 1:
      return 10;
    case 2:
      return 100;
    case 3:
      return 1000;
    default:
      return 1;
   }
 }

void AutoDoubleSpinBox::onValueChanged(double value)
{
  if (m_field && !lock()) {
    *m_field = round(value * multiplier() - m_offset);
    emit currentDataChanged(*m_field);
    dataChanged();
  }
}
