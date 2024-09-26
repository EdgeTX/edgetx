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

#include "simulator.h"
#include <QDialog>

namespace Ui {
class SimulatorStartupDialog;
}

class QSortFilterProxyModel;

class SimulatorStartupDialog : public QDialog
{
    Q_OBJECT

  public:

    enum ItemModelDataRoles {
      IMDR_Id = Qt::UserRole,
      IMDR_SimulatorId
    };
    Q_ENUM(ItemModelDataRoles)

    explicit SimulatorStartupDialog(SimulatorOptions * options, int * profId, QWidget *parent = 0);
    ~SimulatorStartupDialog();

    static bool usesCategorizedStorage(const QString & name);
    static QString findRadioId(const QString & str);
    static QString radioEepromFileName(const QString & simulatorId, QString folder = "");

    void updateContainerTypes();

  public slots:

    virtual void accept();
    void loadRadioProfile(int id);
    void onRadioProfileChanged(int index);
    void onRadioTypeChanged(int index);
    void onDataFileSelect(bool);
    void onDataFolderSelect(bool);
    void onSdPathSelect(bool);

  protected:

    bool usesCategorizedStorage();
    void changeEvent(QEvent *e);

  private:
    Ui::SimulatorStartupDialog *ui;
    SimulatorOptions * m_options;
    int * m_profileId;
    QSortFilterProxyModel * m_simProxy;

    void setGlobalFirmware(const QString & id);
};
