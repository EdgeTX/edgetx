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

#include "radiowidget.h"
#include "sliderwidget.h"

class RadioFaderWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioFaderWidget(QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(parent, f)
    {
      init();
    }
    explicit RadioFaderWidget(const QString & labelText, int value = 0, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(labelText, value, parent, f)
    {
      init();
    }

    void init()
    {
      m_type = RADIO_WIDGET_FADER;

      SliderWidget * sl = new SliderWidget(this);
      sl->setOrientation(Qt::Vertical);
      sl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      sl->setMinimumHeight(75);
      //sl->setMaximumHeight(75);
      sl->setTickPosition(QSlider::TicksBothSides);
      sl->setPageStep(128);
      sl->setMinimum(-1024);
      sl->setMaximum(1024);
      sl->setTickInterval(65);

      connect(sl, &SliderWidget::valueChanged, this, &RadioWidget::setValue);
      connect(this, &RadioWidget::valueChanged, sl, &QSlider::setValue);

      setWidget(sl);
    }
};
