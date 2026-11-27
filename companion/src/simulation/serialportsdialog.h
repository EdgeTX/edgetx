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

#include "hostserialconnector.h"
#include "simulatorinterface.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QtWidgets>

class QComboBox;
class QLabel;
class QLineEdit;
class QStackedWidget;

namespace Ui {
    class SerialPortsDialog;
}

class SerialPortsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SerialPortsDialog(QWidget *parent, SimulatorInterface *simulator, HostSerialConnector *connector);
    ~SerialPortsDialog();

    // Output state, read by SimulatorMainWindow on accept().
    int aux1Kind;
    QString aux1Spec;
    int aux2Kind;
    QString aux2Spec;

    SimulatorInterface *simulator;
    HostSerialConnector *connector;

  private:
    // Stack page indices for the per-port spec stack.
    enum SpecPage {
      PageNone = 0,
      PageSerial,
      PageSocket,
    };

    struct PortRow {
      QComboBox * typeCombo = nullptr;
      QStackedWidget * specStack = nullptr;
      QComboBox * serialCombo = nullptr;
      QLineEdit * socketEdit = nullptr;
      QLabel * socketStatusLabel = nullptr;
    };

    PortRow auxRows[2];

    Ui::SerialPortsDialog *ui;

    void buildPortRow(int row, const QString & label);
    void loadPortState(int index);
    void onTypeChanged(int index, int comboIndex);
    void populateSerialPortCombo(QComboBox * cb, const QString & currentPortName);
    static QString defaultSocketName(int index);
    void writeOutputs();

  private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();
    void on_refreshButton_clicked();
};
