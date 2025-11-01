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

#include "../firmwares/radiodataconversionstate.h"

#include <QDialog>
#include <QString>
#include <QGridLayout>

class AbstractStaticItemModel;
class RadioDataConvesionState;

class ConvMapDialog : public QDialog
{
    Q_OBJECT

  public:
    ConvMapDialog(QWidget * parent, RadioDataConversionState & cstate);
    virtual ~ConvMapDialog() {}

  private:
    RadioDataConversionState &cstate;
    QList<QWidget *> *params;
    int row;
    QGridLayout *grid;
    AbstractStaticItemModel *toSticksItemModel;
    AbstractStaticItemModel *toInputsItemModel;
    AbstractStaticItemModel *toSwitchesItemModel;

    void addFlex(int index);
    void addHeading();
    void addLabel(QString text);
    void addLine();
    void addParams();
    void addSection(QString text);
    void addStick(int index);
    void addSwitch(int index);
    void buildToInputsItemModel();
    void buildToSticksItemModel();
    void buildToSwitchesItemModel();

};
