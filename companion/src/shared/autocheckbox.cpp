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

#include "autocheckbox.h"

AutoCheckBox::AutoCheckBox(QWidget * parent):
  QCheckBox(parent),
  AutoWidget(),
  m_field(nullptr),
  m_invert(false)
{
  connect(this, &QCheckBox::toggled, this, &AutoCheckBox::onToggled);
}

AutoCheckBox::~AutoCheckBox()
{
}

void AutoCheckBox::setField(bool & field, GenericPanel * panel, bool invert)
{
  m_field = &field;
  m_invert = invert;
  setPanel(panel);
  updateValue();
}

void AutoCheckBox::setInvert(bool invert)
{
  m_invert = invert;
  updateValue();
}

void AutoCheckBox::onToggled(bool checked)
{
  if (m_field && !lock()) {
    const bool val = m_invert ? !checked : checked;
    *m_field = val;
    emit currentDataChanged(val);
    dataChanged();
  }
}

void AutoCheckBox::updateValue()
{
  if (m_field) {
    setLock(true);
    setChecked(m_invert ? !(*m_field) : *m_field);
    setLock(false);
  }
}
