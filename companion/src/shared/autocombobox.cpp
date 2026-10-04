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
  AutoWidget(parent),
  m_next(0),
  m_hasModel(false),
  m_useFindData(true)
{
  initField();
  setSizeAdjustPolicy(QComboBox::AdjustToContents);
  connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AutoComboBox::onCurrentIndexChanged);
}

AutoComboBox::~AutoComboBox()
{
}

void AutoComboBox::initField()
{
  m_int = nullptr;
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
  m_int = (int *)&field;
  updateValue();
}

void AutoComboBox::setField(int & field, AbstractPanel * panel)
{
  setFieldInit(panel);
  m_int = &field;
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
  if (model) {
    setLock(true);
    QComboBox::setModel(model);
    setLock(false);
    m_hasModel = true;
    updateValue();
  }
}

void AutoComboBox::setValue(QVariant value)
{
  m_value = value;
  updateValue();
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

  if (m_int)
    setCurrentIndex(findData(*m_int));
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
  int ival = 0;
  QString sval;

  if (m_qString || m_stdString || m_value.typeId() != QMetaType::Int) {
    ok = true;
    if (m_useFindData)
      sval = itemData(index).toString();
    else
      sval = itemText(index);
  } else {
    ival = itemData(index).toInt(&ok);
  }

  if (ok) {
    if (m_int && *m_int != ival) {
      *m_int = ival;
    } else if (m_rawSource && m_rawSource->toValue() != ival) {
      *m_rawSource = RawSource(ival);
    } else if (m_rawSwitch && m_rawSwitch->toValue() != ival) {
      *m_rawSwitch = RawSwitch(ival);
    } else if (m_curveType && *m_curveType != ival) {
      *m_curveType = (CurveData::CurveType)ival;
    } else if (m_flexType && *m_flexType != ival) {
      *m_flexType = (Board::FlexType)ival;
    } else if (m_switchType && *m_switchType != ival) {
      *m_switchType = (Board::SwitchType)ival;
    } else if (m_qString && *m_qString != sval) {
      *m_qString = sval;
    } else if (m_stdString && *m_stdString != sval.toStdString()) {
      *m_stdString = sval.toStdString();
      // default using m_value
    } else if (m_value.typeId() == QMetaType::Int && m_value.toInt() != ival) {
      m_value = ival;
    } else if (m_value.typeId() != QMetaType::Int && m_value.toString() != sval) {
      m_value = sval;
    }

    emit currentDataChanged(ival);  // TODO change to QVariant
    runPostChanged();
  }
}
