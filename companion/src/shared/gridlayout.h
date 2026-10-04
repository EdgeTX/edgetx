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

#include "autowidgets.h"

#include <QGridLayout>

struct gridIndex {
  int row;
  int col;
};

class GridLayout : public QGridLayout
{
  Q_OBJECT

  public:
    explicit GridLayout(QWidget * parent = nullptr);
    virtual ~GridLayout();

    // creates and adds a label
    gridIndex addLabel(QString & text, int col = -1, int colSpan = 1, int row = -1, int rowSpan = 1);
    // creates and adds a horizontal line
    gridIndex addLine(int col = -1, int colSpan = -1, int row = -1);
    // adds a layout
    gridIndex addLayout(QLayout * layout, int col = -1, int colSpan = 1, int row = -1, int rowSpan = 1);
    // adds a fixed or collapsible section
    gridIndex addSection(QString & text, int col = -1, int row = -1);
    // adds a widget
    gridIndex addWidget(QWidget * wgt, int col = -1, int colSpan = 1, int row = -1, int rowSpan = 1);
    // increments the current row
    gridIndex newRow();
    // increments the current column
    gridIndex skipCol();

  private:
    int m_lastRow;
    int m_lastCol;

    void setIndexes(int col, int row);
};
