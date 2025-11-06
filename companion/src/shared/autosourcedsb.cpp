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

#include "autosourcedsb.h"
#include "helpers.h"

#include <cmath>

AutoSourceDSB::AutoSourceDSB(QWidget * parent) :
  AutoSource(),
  QDoubleSpinBox(parent),
  m_dsbValue(nullptr)
{
  m_dsbValue = new QDoubleSpinBox(this);
  m_dsbValue->setAccelerated(true);
  connect(m_dsbValue, &QDoubleSpinBox::editingFinished, this,
          &AutoSourceDSB::on_editingFinished);
}

AutoSourceDSB::~AutoSourceDSB()
{
}

void AutoSourceDSB::setField(RawSource * field,
                             RawSource dflt, QString typeLabel, int min, int max,
                             int precision, QString prefix, QString suffix, GenericPanel * panel)
{
  AutoSource::setField(field, panel);
  setField(dflt, typeLabel, min, max, precision, prefix, suffix);

  setLock(true);
  m_dsbValue->setDecimals(m_precision);
  m_dsbValue->setMinimum(min / m_scalingFactor);
  m_dsbValue->setMaximum(max / m_scalingFactor);
  m_dsbValue->setSingleStep(1 / m_scalingFactor);
  m_dsbValue->setSuffix(suffix);
  m_dsbValue->setValue(dflt.index / m_scalingFactor);
  setLock(false);

  updateValue();
}

void AutoSourceDSB::setField(RawSource dflt, QString typeLabel, int min, int max,
                             int precision, QString prefix, QString suffix)
{
  m_dflt = dflt;
  m_typeLabel = typeLabel;
  m_min = min;
  m_max = max;
  m_precision = precision;
  m_prefix = prefix;
  m_suffix = suffix;

  m_scalingFactor = pow(10, (double)m_precision);
}

void AutoSourceDSB::on_editingFinished()
{
  if (!lock() && m_dsbValue) {
    setSource(RawSource(SOURCE_TYPE_NUMBER,
                        (int)round(m_dsbValue->value() * m_scalingFactor)));
    emit dataChanged();
    AutoWidget::dataChanged();
  }
}

void AutoSourceDSB::setValueDefault()
{
  setSource(m_dflt);
}

void AutoSourceDSB::setMinMax(const int min, const int max)
{
  if (m_dsbValue) {
    m_min = min;
    m_max = max;

    if (getSource().type == SOURCE_TYPE_NUMBER &&
        (getSource().index < m_min || getSource().index > m_max)) {
      setSource(RawSource(SOURCE_TYPE_NUMBER,
                          (getSource().index < m_min ? m_min : m_max)));
      updateValue();
    }
  }
}

void AutoSourceDSB::updateValue()
{
  if (getSource().type == SOURCE_TYPE_NUMBER) {
    setLock(true);
    setValue(getSource().index / m_precision);
    setLock(false);
  }
}
