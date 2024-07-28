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

#include <QSpinBox>
#include <QString>

class QRegExpValidator;

class AutoHexSpinBox : public QSpinBox, public AutoWidget
{
  Q_OBJECT

  public:
    constexpr static unsigned int AUTOHEXSPINBOX_MAX_VALUE {65535};

    explicit AutoHexSpinBox(QWidget * parent = nullptr);
    virtual ~AutoHexSpinBox();

    virtual void updateValue();

    void setField(unsigned int & field, const unsigned int min = 0, const unsigned int max = AUTOHEXSPINBOX_MAX_VALUE, GenericPanel * panel = nullptr);
    void setField(unsigned int & field, GenericPanel * panel = nullptr);
    void setRange(unsigned int min = 0, unsigned int max = AUTOHEXSPINBOX_MAX_VALUE);

  protected:
    QValidator::State validate(QString &text, int &pos) const;
    int valueFromText(const QString &text) const;
    QString textFromValue(int value) const;

  signals:
    void currentDataChanged(int value);

  protected slots:
    void onValueChanged(int value);

  private:
    unsigned int *m_field;
    QRegExpValidator *m_validator;
    unsigned int m_length;
};
