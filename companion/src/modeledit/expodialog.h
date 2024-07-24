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

#include "eeprominterface.h"

#include <QtWidgets>

class CompoundItemModelFactory;
class FilteredItemModelFactory;
class CurveRefFilteredFactory;
class CurveReferenceUIManager;
class SourceNumRefEditor;

namespace Ui {
  class ExpoDialog;
}

class ExpoDialog : public QDialog {
    Q_OBJECT
  public:
    ExpoDialog(QWidget *parent, ModelData & model, ExpoData *expodata, GeneralSettings & generalSettings,
                Firmware * firmware, QString & inputName, CompoundItemModelFactory * sharedItemModels);
    virtual ~ExpoDialog();

  protected:
    void updateScale();

  private slots:
    void valuesChanged();
    void label_phases_customContextMenuRequested(const QPoint & pos);
    void fmClearAll();
    void fmSetAll();
    void fmInvertAll();

  private:
    Ui::ExpoDialog * ui;
    ModelData & model;
    GeneralSettings & generalSettings;
    Firmware * firmware;
    ExpoData * ed;
    QString & inputName;
    SourceNumRefEditor * weightEditor;
    SourceNumRefEditor * offsetEditor;
    CurveReferenceUIManager * curveGroup;
    bool lock;
    QCheckBox * cb_fp[CPN_MAX_FLIGHT_MODES];
    FilteredItemModelFactory *dialogFilteredItemModels;
    CurveRefFilteredFactory *curveRefFilteredItemModels;
    int carryTrimFilterFlags = 0;

    void shrink();
};
