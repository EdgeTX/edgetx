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

#include <QSlider>

class AutoSlider : public QSlider, public AutoWidget
{
  Q_OBJECT

  public:
    explicit AutoSlider(QWidget * parent = nullptr);
    explicit AutoSlider(Qt::Orientation orientation, QWidget * parent = nullptr);
    virtual ~AutoSlider();

    virtual void updateValue() override;

    void setField(int & field, int min, int max, GenericPanel * panel = nullptr);
    void setField(unsigned int & field, int min, int max, GenericPanel * panel = nullptr);
    void setTick(int interval, QSlider::TickPosition position);

  signals:
    void currentDataChanged(int value);

  protected slots:
    void onValueChanged(int value);

  private:
    int *m_field = nullptr;

    void init();
    void setFieldInit(int min, int max, GenericPanel * panel);
};
