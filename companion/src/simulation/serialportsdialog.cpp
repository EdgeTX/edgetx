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

#include "eeprominterface.h"
#include "boards.h"
#include "constants.h"

SerialPortsDialog::SerialPortsDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SerialPortsDialog)
{
  ui->setupUi(this);

  aux1 = QString("");
  aux2 = QString("");

  populateSerialPortCombo(ui->aux1Combo);
  populateSerialPortCombo(ui->aux2Combo);
}

SerialPortsDialog::~SerialPortsDialog()
{
  delete ui;
}

void SerialPortsDialog::populateSerialPortCombo(QComboBox * cb)
{
  cb->clear();
  cb->addItem(tr("Not Assigned"), "");

  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  for (int i = 0; i < serialPortInfos.size(); i++) {
    const auto portInfo = serialPortInfos[i];
    cb->addItem(portInfo.systemLocation(), portInfo.portName());
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
  populateSerialPortCombo(ui->aux1Combo);
  populateSerialPortCombo(ui->aux2Combo);
}

void SerialPortsDialog::on_aux1Combo_currentIndexChanged(int index)
{
  aux1 = ui->aux1Combo->itemData(index).toString();
}

void SerialPortsDialog::on_aux2Combo_currentIndexChanged(int index)
{
  aux2 = ui->aux2Combo->itemData(index).toString();
}
