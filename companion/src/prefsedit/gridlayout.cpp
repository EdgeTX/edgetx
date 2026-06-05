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

#include "gridlayout.h"
#include "autocollapsiblesection.h"

#include <QFrame>

GridLayout::GridLayout(QWidget * parent) :
  QGridLayout(parent),
  wgts(new QList<QWidget *>()),
  row(0)
{
}

void GridLayout::addLabel(QString text)
{
  QLabel *label = new QLabel();
  label->setText(text);
  addWidget(label);
}

void GridLayout::addLine()
{
  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  QGridLayout::addWidget(line, row++, 0, 1, columnCount());
}

void GridLayout::addWidget(QWidget * wgt)
{
  wgts->append(wgt);
}

void GridLayout::addSection(QString text, bool collapsible)
{
  addLabel(QString("<b>%1</b>").arg(text));
  append();
}

void GridLayout::append()
{
  for (int i = 0; i < wgts->size(); i++) {
    QGridLayout::addWidget(wgts->at(i), row, i);
  }

  row += 1;
  wgts->clear();
}
