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

#include "autolabel.h"

#include <QFontMetrics>

AutoLabel::AutoLabel(QWidget * parent):
  QLabel(parent),
  AutoWidget(),
  m_charField(NULL),
  m_strField(nullptr)
{
}

AutoLabel::~AutoLabel()
{
}

void AutoLabel::setField(char * field, GenericPanel * panel)
{
  m_charField = field;
  setPanel(panel);
  updateValue();
}

void AutoLabel::setField(QString & field, GenericPanel * panel)
{
  m_strField = &field;
  setPanel(panel);
  updateValue();
}

void AutoLabel::setWidth(int numChars)
{
  QFontMetrics *f = new QFontMetrics(this->font());
  QSize sz = f->size(Qt::TextSingleLine, QString(numChars, QChar('X')));
  setMinimumWidth(sz.width());
  delete f;
}

void AutoLabel::updateValue()
{
  setLock(true);
  if (m_strField)
    setText(*m_strField);
  else if (m_charField)
    setText(m_charField);
  setLock(false);
}
