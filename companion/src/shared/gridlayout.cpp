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

#include <QFrame>

GridLayout::GridLayout(QWidget * parent) :
  QGridLayout(parent),
  m_lastRow(0),
  m_lastCol(0)
{
}

GridLayout::~GridLayout()
{
}

gridIndex GridLayout::addLabel(QString & text, int col, int colSpan, int row, int rowSpan)
{
  QLabel *label = new QLabel(text);
  setIndexes(col, row);
  addWidget(label, col, colSpan, row, rowSpan);
  return { m_lastRow, m_lastCol };
}

gridIndex GridLayout::addLine(int col, int colSpan, int row)
{
  QFrame *line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  setIndexes(col, row);
  QGridLayout::addWidget(line, m_lastRow, m_lastCol, 1, (colSpan > 0 ? colSpan : columnCount()));
  return { m_lastRow, m_lastCol };
}

gridIndex GridLayout::addLayout(QLayout * layout, int col, int colSpan, int row, int rowSpan)
{
  setIndexes(col, row);
  QGridLayout::addLayout(layout, m_lastRow, m_lastCol, rowSpan, colSpan);
  return { m_lastRow, m_lastCol };
}

gridIndex GridLayout::addWidget(QWidget * wgt, int col, int colSpan, int row, int rowSpan)
{
  setIndexes(col, row);
  QGridLayout::addWidget(wgt, m_lastRow, m_lastCol, rowSpan, colSpan);
  return { m_lastRow, m_lastCol };
}

gridIndex GridLayout::addSection(QString & text, int col, int row)
{
  QString str(QString("<b>%1</b>").arg(text));
  return addLabel(str, col, row);
}

gridIndex GridLayout::newRow()
{
  return { ++m_lastRow, (m_lastCol = 0) };
}

void GridLayout::setIndexes(int col, int row)
{
  m_lastCol = (col = -1 ? m_lastCol + 1 : col);
  // do not auto increment row
  m_lastRow = (row = -1 ? m_lastRow : row);
}
