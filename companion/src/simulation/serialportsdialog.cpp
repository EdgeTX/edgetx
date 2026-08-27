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

#include "serialportsdialog.h"
#include "ui_serialportsdialog.h"

SerialPortsDialog::SerialPortsDialog(QWidget *parent, SimulatorInterface *simulator, HostSerialConnector *connector) :
  QDialog(parent),
  simulator(simulator),
  connector(connector),
  ui(new Ui::SerialPortsDialog)
{
  ui->setupUi(this);

  aux1 = connector->getConnectedSerialPortName(0);
  aux2 = connector->getConnectedSerialPortName(1);

  populateSerialPortCombo(ui->aux1Combo, aux1);
  populateSerialPortCombo(ui->aux2Combo, aux2);

  ui->aux1Combo->setEnabled(simulator->getCapability(SimulatorInterface::Capability::CAP_SERIAL_AUX1));
  ui->aux2Combo->setEnabled(simulator->getCapability(SimulatorInterface::Capability::CAP_SERIAL_AUX2));
}

SerialPortsDialog::~SerialPortsDialog()
{
  delete ui;
}

void SerialPortsDialog::populateSerialPortCombo(QComboBox * cb, QString currentPortName)
{
  cb->clear();
  cb->addItem(tr("Not Assigned"), "");
  if (currentPortName == "") {
    cb->setCurrentIndex(0);
  }

  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  for (int i = 0; i < serialPortInfos.size(); i++) {
    const auto portInfo = serialPortInfos[i];
    cb->addItem(portInfo.portName(), portInfo.portName());
    if (portInfo.portName() == currentPortName)
      cb->setCurrentIndex(i + 1);
  }
}

void SerialPortsDialog::on_cancelButton_clicked()
{
  this->reject();
}

void SerialPortsDialog::on_okButton_clicked()
{
  this->accept();
}

void SerialPortsDialog::on_refreshButton_clicked()
{
  populateSerialPortCombo(ui->aux1Combo, aux1);
  populateSerialPortCombo(ui->aux2Combo, aux2);
}

void SerialPortsDialog::on_aux1Combo_currentIndexChanged(int index)
{
  aux1 = ui->aux1Combo->itemData(index).toString();
}

void SerialPortsDialog::on_aux2Combo_currentIndexChanged(int index)
{
  aux2 = ui->aux2Combo->itemData(index).toString();
}
