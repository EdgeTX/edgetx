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

#include "autobitsetcheckbox.h"

AutoBitsetCheckBox::AutoBitsetCheckBox(QWidget * parent) :
  AutoBitsetCheckBox(QString(), parent)
{
}

AutoBitsetCheckBox::AutoBitsetCheckBox(const QString & text, QWidget * parent) :
  QCheckBox(text, parent),
  AutoWidget()
{
  init();
}

// int field constructors
AutoBitsetCheckBox::AutoBitsetCheckBox(int & field, int bitmask, const QString & text, QWidget * parent) :
  AutoBitsetCheckBox(field, bitmask, false, text, parent)
{
}

AutoBitsetCheckBox::AutoBitsetCheckBox(int & field, int bitmask, bool invert, const QString & text, QWidget * parent) :
  QCheckBox(text, parent),
  AutoWidget()
{
  setField(field, bitmask, invert);
  init();
}

AutoBitsetCheckBox::AutoBitsetCheckBox(int & field, int bitmask, int toggleMask, const QString & text, QWidget * parent) :
  QCheckBox(text, parent),
  AutoWidget()
{
  setField(field, bitmask, false, toggleMask);
  init();
}

// unsigned field constructors
AutoBitsetCheckBox::AutoBitsetCheckBox(unsigned & field, int bitmask, const QString & text, QWidget * parent) :
  AutoBitsetCheckBox(field, bitmask, false, text, parent)
{
}

AutoBitsetCheckBox::AutoBitsetCheckBox(unsigned & field, int bitmask, bool invert, const QString & text, QWidget * parent) :
  QCheckBox(text, parent),
  AutoWidget()
{
  setField(field, bitmask, invert);
  init();
}

AutoBitsetCheckBox::AutoBitsetCheckBox(unsigned & field, int bitmask, int toggleMask, const QString & text, QWidget * parent) :
  QCheckBox(text, parent),
  AutoWidget()
{
  setField(field, bitmask, false, toggleMask);
  init();
}

AutoBitsetCheckBox::~AutoBitsetCheckBox()
{
}

void AutoBitsetCheckBox::setField(int & field, int bitmask, bool invert, int toggleMask)
{
  m_field = &field;
  initField(bitmask, invert, toggleMask);
}

void AutoBitsetCheckBox::setField(unsigned & field, int bitmask, bool invert, int toggleMask)
{
  m_field = (int *)&field;
  initField(bitmask, invert, toggleMask);
}

void AutoBitsetCheckBox::setBitmask(int bitmask)
{
  m_bitmask = bitmask;
  updateValue();
}

void AutoBitsetCheckBox::setToggleMask(int toggleMask)
{
  m_toggleMask = toggleMask;
  updateValue();
}

void AutoBitsetCheckBox::setInverted(bool invert)
{
  m_invert = invert;
  updateValue();
}

void AutoBitsetCheckBox::updateValue()
{
  if (m_field)
    setLock(true);
    setChecked((m_invert && !m_toggleMask) != bool(*m_field & m_bitmask));
    setLock(false);
}

void AutoBitsetCheckBox::onToggled(bool checked)
{
  if (!m_field || lock())
    return;

  if (m_toggleMask) {
    if (checked) {
      *m_field &= ~m_toggleMask;
      *m_field |= m_bitmask;
    }
    else {
      *m_field &= ~m_bitmask;
      *m_field |= m_toggleMask;
    }
  }
  else {
    if (m_invert)
      checked = !checked;
    if (checked)
      *m_field |= m_bitmask;
    else
      *m_field &= ~m_bitmask;
  }

  emit currentDataChanged(checked);
  dataChanged();
}

void AutoBitsetCheckBox::init()
{
  connect(this, &QCheckBox::toggled, this, &AutoBitsetCheckBox::onToggled);
}

void AutoBitsetCheckBox::initField(int bitmask, bool invert, int toggleMask)
{
  m_bitmask = bitmask;
  m_toggleMask = toggleMask;
  m_invert = toggleMask ? false : invert;
  updateValue();
}
