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

#include <QTimeEdit>

class AutoTimeEdit : public QTimeEdit, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoTimeEdit(QWidget * parent = nullptr);
    virtual ~AutoTimeEdit();

    virtual void updateValue() override;

    void setField(unsigned int & field, GenericPanel * panel = nullptr);
    void setTimeRange(const QTime min, const QTime max);

  signals:
    void currentDataChanged(unsigned int val);

  protected slots:
    void onTimeChanged(QTime time);

  private:
    unsigned int *m_field;
};

