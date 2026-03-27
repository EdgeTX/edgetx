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

#pragma once

#include "autowidget.h"
#include "rawsource.h"
#include "rawswitch.h"
#include "curvedata.h"
#include "boards.h"

#include <QComboBox>

#include <string.h>

/*
    NOTE: Q_OBJECT classes cannot be templated and since we use signals we have no choice but to
          take this approach or create a class per data type
*/
class AutoComboBox : public QComboBox, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoComboBox(QWidget * parent = nullptr);
    virtual ~AutoComboBox();

    // QComboBox
    virtual void addItem(const QString & item);
    virtual void addItem(const QString & item, int value);
    virtual void insertItems(int index, const QStringList & items);
    // AutoWidget
    virtual void updateValue() override;

    void clear();

    void setField(unsigned int & field, GenericPanel * panel = nullptr);
    void setField(int & field, GenericPanel * panel = nullptr);
    void setField(RawSource & field, GenericPanel * panel = nullptr);
    void setField(RawSwitch & field, GenericPanel * panel = nullptr);
    void setField(CurveData::CurveType & field, GenericPanel * panel = nullptr);
    void setField(Board::FlexType & field, GenericPanel * panel = nullptr);
    void setField(Board::SwitchType & field, GenericPanel * panel = nullptr);
    void setField(QString & field, GenericPanel * panel = nullptr);
    void setField(std::string & field, GenericPanel * panel = nullptr);

    void setAutoIndexes();
    void setModel(QAbstractItemModel * model) override;

  signals:
    void currentDataChanged(int value);

  protected slots:
    void onCurrentIndexChanged(int index);

  private:
    int m_next;
    bool m_hasModel;
    int *m_field;
    RawSource *m_rawSource;
    RawSwitch *m_rawSwitch;
    CurveData::CurveType *m_curveType;
    Board::FlexType *m_flexType;
    Board::SwitchType *m_switchType;
    QString *m_qString;
    std::string *m_stdString;

    void initField();
    void setFieldInit(GenericPanel * panel);
};
