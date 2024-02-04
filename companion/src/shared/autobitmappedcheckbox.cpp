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

#include "autobitmappedcheckbox.h"

AutoBitMappedCheckBox::AutoBitMappedCheckBox(QWidget * parent):
  QCheckBox(parent),
  AutoWidget(),
  m_field(nullptr),
  m_invert(false)
{
  setBits();
  connect(this, &QCheckBox::toggled, this, &AutoBitMappedCheckBox::onToggled);
}

AutoBitMappedCheckBox::~AutoBitMappedCheckBox()
{
}

void AutoBitMappedCheckBox::setField(int & field, GenericPanel * panel, bool invert)
{
  m_field = &field;
  m_invert = invert;
  setPanel(panel);
  updateValue();
}

void AutoBitMappedCheckBox::setField(unsigned int & field, GenericPanel * panel, bool invert)
{
  m_field = (int *)&field;
  m_invert = invert;
  setPanel(panel);
  updateValue();
}

void AutoBitMappedCheckBox::setInvert(bool invert)
{
  m_invert = invert;
  updateValue();
}

void AutoBitMappedCheckBox::setBits(const unsigned int numBits, const unsigned int offsetBits,
                                    const unsigned int index, const unsigned int indexBits)
{
  m_bits = numBits;
  m_offsetBits = offsetBits;
  m_index = index;
  m_indexBits = indexBits;

  if (m_offsetBits + m_bits > m_indexBits)
    m_indexBits = m_offsetBits + m_bits;
}

void AutoBitMappedCheckBox::onToggled(bool checked)
{
  if (m_field && !lock()) {
    const bool val = m_invert ? !checked : checked;
    unsigned int fieldmask = (bitmask() << shiftbits());
    *m_field = (*m_field & ~fieldmask) | (val << shiftbits());
    emit currentDataChanged(val);
    dataChanged();
  }
}

void AutoBitMappedCheckBox::updateValue()
{
  if (m_field) {
    setLock(true);
    bool val = (bool)((*m_field >> shiftbits()) & bitmask());
    setChecked(m_invert ? !val : val);
    setLock(false);
  }
}

unsigned int AutoBitMappedCheckBox::shiftbits()
{
  return m_indexBits * m_index + m_offsetBits;
}

unsigned int AutoBitMappedCheckBox::bitmask()
{
  int mask = -1;
  mask = ~(mask << m_bits);
  return (unsigned int)mask;
}
