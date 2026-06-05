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

#include "autocheckbox.h"
#include "autocombobox.h"
#include "autofldrselectbtn.h"
#include "autolabel.h"
#include "autolineedit.h"

#include <QGridLayout>
#include <QList>

class GridLayout : public QGridLayout
{
  Q_OBJECT

  public:
    explicit GridLayout(QWidget * parent = nullptr);
    virtual ~GridLayout();

    // creates and adds a label to the widgets list
    void addLabel(QString text);
    // adds a horizontal line to the grid
    void addLine();
    // adds the widget to the widgets list Note: overides QGridLayout::addWidget
    void addWidget(QWidget * wgt);
    // adds a fixed or collapsible section to the grid
    void addSection(QString text, bool collapsible = false);
    // appends the widgets list as a row to the grid and clears the list
    void append();

  private:
    QList<QWidget *> *wgts;
    int row;
};
