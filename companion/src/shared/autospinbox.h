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

#include <QSpinBox>

class AutoSpinBox : public QSpinBox, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoSpinBox(QWidget * parent = nullptr);
    virtual ~AutoSpinBox();

    virtual void updateValue() override;

    void setField(int & field, GenericPanel * panel = nullptr);
    void setField(unsigned int & field, GenericPanel * panel = nullptr);

  signals:
    void currentDataChanged(int value);

  protected slots:
    void onValueChanged(int value);

  private:
    int *m_field;

    void setFieldInit(GenericPanel * panel);
};
