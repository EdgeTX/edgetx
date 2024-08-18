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

#include <QComboBox>
#include <QString>

class AutoPrecisionComboBox : public QComboBox, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoPrecisionComboBox(QWidget * parent = nullptr);
    explicit AutoPrecisionComboBox(unsigned int & field, unsigned int minDecimals = 0, unsigned int maxDecimals = 1, bool padding = false, QString suffix = "", QWidget * parent = nullptr, GenericPanel * panel = nullptr);
    explicit AutoPrecisionComboBox(int & field, unsigned int minDecimals = 0, unsigned int maxDecimals = 1, bool padding = false, QString suffix = "", QWidget * parent = nullptr, GenericPanel * panel = nullptr);
    virtual ~AutoPrecisionComboBox();

    virtual void updateValue() override;

    void setField(int & field, unsigned int minDecimals = 0, unsigned int maxDecimals = 1, bool padding = false, QString suffix = "", GenericPanel * panel = nullptr);
    void setField(unsigned int & field, unsigned int minDecimals = 0, unsigned int maxDecimals = 1, bool padding = false, QString suffix = "", GenericPanel * panel = nullptr);
    void setMinDecimals(unsigned int minDecimals);
    void setMaxDecimals(unsigned int maxDecimals);
    void setDecimalsRange(unsigned int min, unsigned int max);
    void setPadding(bool padding);
    void setSuffix(QString suffix);
    void setValue(int value);
    void setValue(unsigned int value);
    void setAutoIndexes();

  signals:
    void currentDataChanged(int index);

  protected slots:
    void onCurrentIndexChanged(int index);

  private:
    unsigned int *m_field = nullptr;
    unsigned int m_minDecimals = 0;
    unsigned int m_maxDecimals = 1;
    bool m_padding = false;
    QString m_suffix = "";

    void init();
    void setFieldInit(unsigned int minDecimals = 0, unsigned int maxDecimals = 1, bool padding = false, QString suffix = "", GenericPanel * panel = nullptr);
    bool isValidDecimals(unsigned int value);
    unsigned int rangecheckDecimals(unsigned int decimals);
    void updateList();
};
