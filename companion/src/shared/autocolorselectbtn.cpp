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

#include "autocolorselectbtn.h"

AutoColorSelectBtn::AutoColorSelectBtn(QWidget * parent) :
  AutoPushButton(parent),
  m_sample(nullptr)
{
  connect(this, &QPushButton::released, [&] ()
  {
    QColor color = QColorDialog::getColor(m_color, this);

    if (color.isValid() && color != m_color) {
      m_color = color;

      if (m_sample) {
        m_sample->setPalette(QPalette(m_color));
        m_sample->repaint();
      }

      emit colorChanged(m_color);
    }
  });
}

AutoColorSelectBtn::~AutoColorSelectBtn()
{
}

void AutoColorSelectBtn::setup(QColor color, QWidget * sampleWidget)
{
  m_color = color;
  m_sample = sampleWidget;
}
