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

#include "autoslider.h"

AutoSlider::AutoSlider(QWidget * parent):
  QSlider(parent),
  AutoWidget()
{
  init();
}

AutoSlider::AutoSlider(Qt::Orientation orientation, QWidget * parent):
  QSlider(orientation, parent),
  AutoWidget()
{
  init();
}

AutoSlider::~AutoSlider()
{
}

void AutoSlider::setField(int & field, int min, int max, GenericPanel * panel)
{
  m_field = &field;
  setFieldInit(min, max, panel);
}

void AutoSlider::setField(unsigned int & field, int min, int max, GenericPanel * panel)
{
  m_field = (int *)&field;
  setFieldInit(min, max, panel);
}

void AutoSlider::setTick(int interval, QSlider::TickPosition position)
{
  setTickInterval(interval);
  setTickPosition(position);
}

void AutoSlider::updateValue()
{
  if (m_field) {
    setLock(true);
    setValue(*m_field);
    setLock(false);
  }
}

void AutoSlider::init()
{
  connect(this, &QSlider::valueChanged, this, &AutoSlider::onValueChanged);
}

void AutoSlider::setFieldInit(int min, int max, GenericPanel * panel)
{
  setPanel(panel);
  setRange(min, max);
  updateValue();
}

void AutoSlider::onValueChanged(int value)
{
  if (m_field && !lock()) {
    if (*m_field != value) {
      *m_field = value;
      emit currentDataChanged(value);
      dataChanged();
    }
  }
}
