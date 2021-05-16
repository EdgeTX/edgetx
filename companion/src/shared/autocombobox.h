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

#pragma once

#include "autowidget.h"
#include "rawsource.h"
#include "rawswitch.h"

#include <QComboBox>

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

    void setAutoIndexes();
    void setModel(QAbstractItemModel * model);

  signals:
    void currentDataChanged(int value);

  protected slots:
    void onCurrentIndexChanged(int index);

  private:
    int *m_field;
    int m_next;
    bool m_hasModel;
    RawSource *m_rawSource;
    RawSwitch *m_rawSwitch;

    void setFieldInit(GenericPanel * panel);
};
