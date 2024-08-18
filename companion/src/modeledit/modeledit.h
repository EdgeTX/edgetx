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

#ifndef _MODELEDIT_H_
#define _MODELEDIT_H_

#include <QtWidgets>
#include "genericpanel.h"

class RadioData;
class CompoundItemModelFactory;

namespace Ui {
  class ModelEdit;
}

class ModelPanel : public GenericPanel
{
  public:
    ModelPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
      GenericPanel(parent, &model, generalSettings, firmware)
    {
    }
};

class ModelEdit : public QDialog
{
    Q_OBJECT

  public:
    ModelEdit(QWidget * parent, RadioData & radioData, int modelId, Firmware * firmware);
    ~ModelEdit();

    int getModelId() { return modelId; }

  protected:
    void closeEvent(QCloseEvent *event);

  signals:
    void modified();

  private slots:
    void onTabIndexChanged(int index);

  private:
    Ui::ModelEdit *ui;
    int modelId;
    RadioData &radioData;
    Firmware *firmware;
    QVector<GenericPanel *> panels;
    CompoundItemModelFactory *sharedItemModels;

    void addTab(GenericPanel * panel, QString text);
    void launchSimulation();

};

#endif // _MODELEDIT_H_
