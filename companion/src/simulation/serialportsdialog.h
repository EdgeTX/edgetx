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

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets>

#include "simulatorinterface.h"
#include "hostserialconnector.h"

class QComboBox;

namespace Ui {
    class SerialPortsDialog;
}

class SerialPortsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SerialPortsDialog(QWidget *parent, SimulatorInterface *simulator, HostSerialConnector *connector);
    ~SerialPortsDialog();
    QString aux1;
    QString aux2;
    SimulatorInterface *simulator;
    HostSerialConnector *connector;

  private:
    Ui::SerialPortsDialog *ui;

  private slots:
    void populateSerialPortCombo(QComboBox * cb, QString currentPortName);
    void on_cancelButton_clicked();
    void on_okButton_clicked();
    void on_refreshButton_clicked();
    void on_aux1Combo_currentIndexChanged(int index);
    void on_aux2Combo_currentIndexChanged(int index);
};
