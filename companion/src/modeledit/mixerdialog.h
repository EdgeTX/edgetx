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

#include <QtWidgets>
#include "eeprominterface.h"

class CompoundItemModelFactory;
class FilteredItemModelFactory;
class CurveRefFilteredFactory;
class CurveReferenceUIManager;
class SourceNumRefEditor;

namespace Ui {
  class MixerDialog;
}

class MixerDialog : public QDialog {
    Q_OBJECT
  public:
    MixerDialog(QWidget *parent, ModelData & model, MixData *mixdata, int index, GeneralSettings & generalSettings, Firmware * firmware,
                CompoundItemModelFactory * sharedItemModels);
    ~MixerDialog();

  protected:
    void changeEvent(QEvent *e);

  private slots:
    void valuesChanged();
    void label_phases_customContextMenuRequested(const QPoint & pos);
    void fmClearAll();
    void fmSetAll();
    void fmInvertAll();

  private:
    Ui::MixerDialog *ui;
    ModelData & model;
    GeneralSettings & generalSettings;
    Firmware * firmware;
    MixData *md;
    bool lock;
    SourceNumRefEditor * weightEditor;
    SourceNumRefEditor * offsetEditor;
    CurveReferenceUIManager * curveGroup;
    QCheckBox * cb_fp[CPN_MAX_FLIGHT_MODES];
    FilteredItemModelFactory *dialogFilteredItemModels;
    CurveRefFilteredFactory *curveRefFilteredItemModels;

    void shrink();
};
