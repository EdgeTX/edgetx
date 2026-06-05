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

#include "autolineedit.h"

AutoLineEdit::AutoLineEdit(QWidget * parent, bool updateOnChange):
  QLineEdit(parent),
  AutoWidget(),
  m_charField(NULL),
  m_strField(nullptr),
  m_value("")
{
  setEditSignal(updateOnChange);
}

AutoLineEdit::~AutoLineEdit()
{
}

void AutoLineEdit::setField(char * field, int len, AbstractPanel * panel)
{
  m_charField = field;
  setFieldInit(len, panel);
}

void AutoLineEdit::setField(QString & field, int len, AbstractPanel * panel)
{
  m_strField = &field;
  setFieldInit(len, panel);
}

void AutoLineEdit::setFieldInit(int len, AbstractPanel * panel)
{
  setPanel(panel);
  setLength(len);
  updateValue();
}

void AutoLineEdit::setLength(int len)
{
  if (len) setMaxLength(len);
}

void AutoLineEdit::setEditSignal(bool onChange)
{
  disconnect(this, &QLineEdit::textChanged, this, &AutoLineEdit::onEdited);
  disconnect(this, &QLineEdit::editingFinished, this, &AutoLineEdit::onEdited);

  if (onChange)
    connect(this, &QLineEdit::textChanged, this, &AutoLineEdit::onEdited);
  else
    connect(this, &QLineEdit::editingFinished, this, &AutoLineEdit::onEdited);
}

void AutoLineEdit::updateValue()
{
  setLock(true);

  if (m_strField)
    QLineEdit::setText(*m_strField);
  else if (m_charField)
    QLineEdit::setText(m_charField);
  else
    QLineEdit::setText(m_value);

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
  else if (m_value != text())
    m_value = text();
  else
    return;

  emit currentDataChanged();
  runPostChanged();
}

void AutoLineEdit::setAutoText(QString text)
{
  setLock(true);
  QLineEdit::setText(text);
  setLock(false);
}

void AutoLineEdit::setValue(QString text, AbstractPanel * panel, int len)
{
  if (!m_strField && !m_charField) {
    m_value = text;
    setFieldInit(len, panel);
  }
}
