/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _EXPODIALOG_H_
#define _EXPODIALOG_H_

#include <QtWidgets>
#include "eeprominterface.h"
#include "modelprinter.h"

class GVarGroup;
class CompoundItemModelFactory;
class FilteredItemModelFactory;
class CurveRefFilteredFactory;
class CurveReferenceUIManager;

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
    void shrink();
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
    GVarGroup * gvWeightGroup;
    GVarGroup * gvOffsetGroup;
    CurveReferenceUIManager * curveGroup;
    ModelPrinter modelPrinter;
    bool lock;
    QCheckBox * cb_fp[CPN_MAX_FLIGHT_MODES];
    FilteredItemModelFactory *dialogFilteredItemModels;
    CurveRefFilteredFactory *curveRefFilteredItemModels;
};

#endif // _EXPODIALOG_H_
