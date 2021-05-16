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

#include "autolineedit.h"

AutoLineEdit::AutoLineEdit(QWidget * parent, bool updateOnChange):
  QLineEdit(parent),
  AutoWidget(),
  m_charField(NULL),
  m_strField(nullptr)
{
  if (updateOnChange)
    connect(this, &QLineEdit::textChanged, this, &AutoLineEdit::onEdited);
  else
    connect(this, &QLineEdit::editingFinished, this, &AutoLineEdit::onEdited);
}

AutoLineEdit::~AutoLineEdit()
{
}

void AutoLineEdit::setField(char * field, int len, GenericPanel * panel)
{
  m_charField = field;
  setPanel(panel);
  setValidator(new QRegExpValidator(QRegExp("[ A-Za-z0-9_.-,]*"), this));
  if (len)
    setMaxLength(len);
  updateValue();
}

void AutoLineEdit::setField(QString & field, int len, GenericPanel * panel)
{
  m_strField = &field;
  if (len)
    setMaxLength(len);
  setPanel(panel);
  updateValue();
}

void AutoLineEdit::updateValue()
{
  setLock(true);
  if (m_strField)
    setText(*m_strField);
  else if (m_charField)
    setText(m_charField);
  setLock(false);
}

void AutoLineEdit::onEdited()
{
  if (lock())
    return;

  if (m_strField && *m_strField != text())
    *m_strField = text();
  else if (m_charField && strcmp(m_charField, text().toLatin1()))
    strcpy(m_charField, text().toLatin1());
  else
    return;

  emit currentDataChanged();
  dataChanged();
}
