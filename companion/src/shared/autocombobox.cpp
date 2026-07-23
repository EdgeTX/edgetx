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

#include "autocombobox.h"

AutoComboBox::AutoComboBox(QWidget * parent):
  QComboBox(parent),
  AutoWidget(),
  m_next(0),
  m_hasModel(false),
  m_useFindData(true)
{
  initField();
  setSizeAdjustPolicy(QComboBox::AdjustToContents);
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AutoComboBox::onCurrentIndexChanged);
}

AutoComboBox::~AutoComboBox()
{
}

void AutoComboBox::initField()
{
  m_field = nullptr;
  m_rawSource = nullptr;
  m_rawSwitch = nullptr;
  m_curveType = nullptr;
  m_flexType = nullptr;
  m_switchType = nullptr;
  m_qString = nullptr;
  m_stdString = nullptr;
  m_value = 0;

}

void AutoComboBox::clear()
{
  if (!m_hasModel) {
    setLock(true);
    QComboBox::clear();
    m_next = 0;
    setLock(false);
  }
}

void AutoComboBox::insertItems(int index, const QStringList & items)
{
  if (!m_hasModel) {
    foreach(QString item, items)
      addItem(item);

  }
}

void AutoComboBox::addItem(const QString & item)
{
  if (!m_hasModel)
    addItem(item, m_next++);
}

void AutoComboBox::addItem(const QString & item, int value)
{
  if (!m_hasModel) {
    setLock(true);
    QComboBox::addItem(item, value);
    setLock(false);
    updateValue();
  }
}

void AutoComboBox::addItems(const QStringList & items)
{
  if (!m_hasModel) {
    setLock(true);

    foreach(QString item, items)
      QComboBox::addItem(item, m_next++);

    setLock(false);
    updateValue();
  }
}

void AutoComboBox::setAutoModel(QAbstractItemModel * model)
{
  AutoComboBox::setModel(model);
}

void AutoComboBox::setField(unsigned int & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_field = (int *)&field;
  updateValue();
}

void AutoComboBox::setField(int & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_field = &field;
  updateValue();
}

void AutoComboBox::setField(RawSource & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_rawSource = &field;
  updateValue();
}

void AutoComboBox::setField(RawSwitch & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_rawSwitch = &field;
  updateValue();
}

void AutoComboBox::setField(CurveData::CurveType & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_curveType = &field;
  updateValue();
}

void AutoComboBox::setField(Board::FlexType & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_flexType = &field;
  updateValue();
}

void AutoComboBox::setField(Board::SwitchType & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_switchType = &field;
  updateValue();
}

void AutoComboBox::setField(QString & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_qString = &field;
  updateValue();
}

void AutoComboBox::setField(std::string & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_stdString = &field;
  updateValue();
}

void AutoComboBox::setFieldInit(AbstractPanel * panel)
{
  initField();
  setPanel(panel);
}

void AutoComboBox::setModel(QAbstractItemModel * model)
{
  if (model && QComboBox::model() != model) {
    setLock(true);
    QComboBox::setModel(model);
    setLock(false);
    m_hasModel = true;
    updateValue();
  }
}

void AutoComboBox::setValue(QVariant value, AbstractPanel * panel, bool useFindData)
{
  setFieldInit(panel);
  m_value = value;
  m_useFindData = useFindData;
  updateValue();
}

void AutoComboBox::setAutoIndexes()
{
  if (!m_hasModel) {
    for (int i = 0; i < count(); ++i)
      setItemData(i, i);

    updateValue();
  }
}

void AutoComboBox::updateValue()
{
  setLock(true);

  if (m_field)
    setCurrentIndex(findData(*m_field));
  else if (m_rawSource)
    setCurrentIndex(findData(m_rawSource->toValue()));
  else if (m_rawSwitch)
    setCurrentIndex(findData(m_rawSwitch->toValue()));
  else if (m_curveType)
    setCurrentIndex(findData(*m_curveType));
  else if (m_flexType)
    setCurrentIndex(findData(*m_flexType));
  else if (m_switchType)
    setCurrentIndex(findData(*m_switchType));
  else if (m_qString)
    setCurrentIndex(findText(*m_qString));
  else if (m_stdString)
    setCurrentIndex(findText(QString(m_stdString->c_str())));
  // default using m_value
  else if (m_useFindData)
    setCurrentIndex(findData(m_value));
  else
    setCurrentIndex(findText(m_value.toString()));

  setLock(false);
}

void AutoComboBox::onCurrentIndexChanged(int index)
{
  if (lock() || index < 0 || index >= count())
    return;

  bool ok;
  int val = 0;
  QString str(itemText(index));

  if (m_qString || m_stdString || !isValueInt()) {
    ok = true;
  } else {
    val = itemData(index).toInt(&ok);
  }

  if (ok) {
    if (m_field && *m_field != val)
      *m_field = val;
    else if (m_rawSource && m_rawSource->toValue() != val)
      *m_rawSource = RawSource(val);
    else if (m_rawSwitch && m_rawSwitch->toValue() != val)
      *m_rawSwitch = RawSwitch(val);
    else if (m_curveType && *m_curveType != val)
      *m_curveType = (CurveData::CurveType)val;
    else if (m_flexType && *m_flexType != val)
      *m_flexType = (Board::FlexType)val;
    else if (m_switchType && *m_switchType != val)
      *m_switchType = (Board::SwitchType)val;
    else if (m_qString && *m_qString != str)
      *m_qString = str;
    else if (m_stdString && *m_stdString != str.toStdString())
      *m_stdString = str.toStdString();
    // default using m_value
    else if (isValueInt() && m_value.toInt() != val)
      m_value = val;
    else if (!isValueInt() && m_value.toString() != str)
      m_value = str;

    emit currentDataChanged(val);
    runPostChanged();
  }
}

bool AutoComboBox::isValueInt()
{
  bool ok;
  m_value.toInt(&ok);
  return ok;
}
