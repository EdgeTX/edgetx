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

#include "modeledit.h"
#include "eeprominterface.h"

#include <QtCore>

class UserInterfacePanel : public ModelPanel
{
    Q_OBJECT

  public:
    UserInterfacePanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~UserInterfacePanel();

  private:
    QGridLayout * grid;
    int currindex = -1;
    QVector<QPushButton *> widgetbtns;
    QVector<QFrame *> optswidgets;
    QVector<QGridLayout *> optsgrids;
};

class CustomScreenPanel : public ModelPanel
{
    Q_OBJECT

  public:
    CustomScreenPanel(QWidget *parent, ModelData & model, int index, GeneralSettings & generalSettings, Firmware * firmware);
    virtual ~CustomScreenPanel();

  private:
    QGridLayout * grid;
    int currindex = -1;
    QVector<QPushButton *> widgetbtns;
    QVector<QFrame *> optswidgets;
    QVector<QGridLayout *> optsgrids;
};

class ColorCustomScreensPanel : public ModelPanel
{
    Q_OBJECT

  public:
    ColorCustomScreensPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                  CompoundItemModelFactory * sharedItemModels);
    virtual ~ColorCustomScreensPanel();

  private:
    QTabWidget *tabWidget;
    QGridLayout * grid;
    QVector<GenericPanel *> panels;

    QString getTabName(int index);
};

void addGridLabel(QGridLayout * grid, QString text, int row, int col, int colspan = 1);
void addGridBlankRow(QGridLayout * grid, int & row);
void addGridLine(QGridLayout * grid, int row, int col, int colspan);

template <class T>
QGridLayout * addOptionsLayout(T & persistentData, int optioncnt, QString title = "");
