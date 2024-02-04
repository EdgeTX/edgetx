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

#include <QComboBox>

class AutoBitMappedComboBox : public QComboBox, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoBitMappedComboBox(QWidget * parent = nullptr);
    virtual ~AutoBitMappedComboBox();

    // QComboBox
    virtual void addItem(const QString & item);
    virtual void addItem(const QString & item, int value);
    virtual void insertItems(int index, const QStringList & items);
    // AutoWidget
    virtual void updateValue() override;

    void clear();

    void setBits(const unsigned int numBits = 1, const unsigned int offsetBits = 0, const unsigned int index = 0,
                 const unsigned int indexBits = 1);
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
    unsigned int m_index;
    unsigned int m_indexBits;
    unsigned int m_bits;
    unsigned int m_offsetBits;

    unsigned int shiftbits();
    unsigned int bitmask();

    void setFieldInit(GenericPanel * panel);
};
