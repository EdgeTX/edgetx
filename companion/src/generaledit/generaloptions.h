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

#include "generaledit.h"

class QGridLayout;

class GeneralOptionsPanel : public GeneralPanel
{
    Q_OBJECT

  public:
    GeneralOptionsPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~GeneralOptionsPanel();

  private:
    Board::Type board;
    QGridLayout *grid;
    QList<QWidget *> *params;
    int row;
    int col;

    void addLabel(QString text);
    void addLine();
    void addParams();
    void addSection(QString text);
};
