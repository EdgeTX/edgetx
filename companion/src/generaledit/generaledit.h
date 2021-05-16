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

#pragma once

#include <QtWidgets>
#include "eeprominterface.h"
#include "genericpanel.h"

class CompoundItemModelFactory;

namespace Ui {
  class GeneralEdit;
}

class GeneralPanel : public GenericPanel
{
  public:
    GeneralPanel(QWidget *parent, GeneralSettings & generalSettings, Firmware * firmware):
      GenericPanel(parent, nullptr, generalSettings, firmware)
    {
    }
};

class GeneralEdit : public QDialog
{
  Q_OBJECT

  public:
    GeneralEdit(QWidget * parent, RadioData & radioData, Firmware * firmware);
    ~GeneralEdit();

  private:
    Ui::GeneralEdit *ui;
    GeneralSettings & generalSettings;

    bool switchDefPosEditLock;
    bool voiceLangEditLock;
    bool mavbaudEditLock;
    void getGeneralSwitchDefPos(int i, bool val);
    void setSwitchDefPos();
    void updateVarioPitchRange();

  signals:
    void modified();

  private slots:
    void onTabModified();
    void on_tabWidget_currentChanged(int index);
    void on_calretrieve_PB_clicked();
    void on_calstore_PB_clicked();

  private:
    Firmware * firmware;
    QVector<GenericPanel *> panels;
    void addTab(GenericPanel *panel, QString text);
    void closeEvent(QCloseEvent *event);
    CompoundItemModelFactory *editorItemModels;
};
