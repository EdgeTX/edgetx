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

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>

SerialPortsDialog::SerialPortsDialog(QWidget *parent, SimulatorInterface *simulator, HostSerialConnector *connector) :
  QDialog(parent),
  aux1Kind(HostSerialConnector::BackendNone),
  aux2Kind(HostSerialConnector::BackendNone),
  simulator(simulator),
  connector(connector),
  ui(new Ui::SerialPortsDialog)
{
  ui->setupUi(this);

  buildPortRow(0, tr("AUX1 Port"));
  buildPortRow(1, tr("AUX2 Port"));

  loadPortState(0);
  loadPortState(1);

  // Disable rows whose AUX is not supported by this simulator build.
  const bool aux1Supported =
      simulator->getCapability(SimulatorInterface::Capability::CAP_SERIAL_AUX1);
  const bool aux2Supported =
      simulator->getCapability(SimulatorInterface::Capability::CAP_SERIAL_AUX2);
  auxRows[0].typeCombo->setEnabled(aux1Supported);
  auxRows[0].specStack->setEnabled(aux1Supported);
  auxRows[1].typeCombo->setEnabled(aux2Supported);
  auxRows[1].specStack->setEnabled(aux2Supported);
}

SerialPortsDialog::~SerialPortsDialog()
{
  delete ui;
}

QString SerialPortsDialog::defaultSocketName(int index)
{
  return QStringLiteral("edgetx-sim-aux%1").arg(index + 1);
}

void SerialPortsDialog::buildPortRow(int index, const QString & labelText)
{
  PortRow & row = auxRows[index];

  auto * label = new QLabel(labelText, this);

  row.typeCombo = new QComboBox(this);
  row.typeCombo->addItem(tr("Not assigned"), HostSerialConnector::BackendNone);
  row.typeCombo->addItem(tr("Serial port"), HostSerialConnector::BackendSerialPort);
  row.typeCombo->addItem(tr("Local socket"), HostSerialConnector::BackendLocalSocket);

  row.specStack = new QStackedWidget(this);

  // Page: none
  auto * nonePage = new QWidget(this);
  auto * noneLayout = new QHBoxLayout(nonePage);
  noneLayout->setContentsMargins(0, 0, 0, 0);
  noneLayout->addStretch();
  row.specStack->insertWidget(PageNone, nonePage);

  // Page: serial port combo
  auto * serialPage = new QWidget(this);
  auto * serialLayout = new QHBoxLayout(serialPage);
  serialLayout->setContentsMargins(0, 0, 0, 0);
  row.serialCombo = new QComboBox(serialPage);
  row.serialCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  serialLayout->addWidget(row.serialCombo);
  row.specStack->insertWidget(PageSerial, serialPage);

  // Page: local socket name + resolved-path label
  auto * socketPage = new QWidget(this);
  auto * socketLayout = new QVBoxLayout(socketPage);
  socketLayout->setContentsMargins(0, 0, 0, 0);
  socketLayout->setSpacing(2);
  row.socketEdit = new QLineEdit(socketPage);
  row.socketEdit->setPlaceholderText(defaultSocketName(index));
  socketLayout->addWidget(row.socketEdit);
  row.socketStatusLabel = new QLabel(socketPage);
  row.socketStatusLabel->setStyleSheet(QStringLiteral("color: gray;"));
  row.socketStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
  socketLayout->addWidget(row.socketStatusLabel);
  row.specStack->insertWidget(PageSocket, socketPage);

  ui->portsLayout->addWidget(label, index, 0);
  ui->portsLayout->addWidget(row.typeCombo, index, 1);
  ui->portsLayout->addWidget(row.specStack, index, 2);
  ui->portsLayout->setColumnStretch(2, 1);

  connect(row.typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this, index](int comboIndex) { onTypeChanged(index, comboIndex); });
}

void SerialPortsDialog::loadPortState(int index)
{
  PortRow & row = auxRows[index];

  const auto kind = connector->getBackendKind(index);
  const QString spec = connector->getBackendSpec(index);

  // Always populate the serial combo so the user can switch to it
  // even when the current backend is something else.
  populateSerialPortCombo(row.serialCombo, kind == HostSerialConnector::BackendSerialPort ? spec : QString());

  if (kind == HostSerialConnector::BackendLocalSocket) {
    row.socketEdit->setText(spec);
    const QString resolved = connector->getLocalSocketFullName(index);
    row.socketStatusLabel->setText(resolved.isEmpty()
                                       ? tr("Not listening")
                                       : tr("Listening on %1").arg(resolved));
  } else {
    row.socketEdit->clear();
    row.socketStatusLabel->clear();
  }

  const int kindIndex = row.typeCombo->findData(static_cast<int>(kind));
  row.typeCombo->setCurrentIndex(kindIndex >= 0 ? kindIndex : 0);
  // Make sure the stack mirrors the (possibly unchanged) combo.
  onTypeChanged(index, row.typeCombo->currentIndex());
}

void SerialPortsDialog::onTypeChanged(int index, int comboIndex)
{
  PortRow & row = auxRows[index];
  const int kind = row.typeCombo->itemData(comboIndex).toInt();

  switch (kind) {
    case HostSerialConnector::BackendSerialPort:
      row.specStack->setCurrentIndex(PageSerial);
      break;
    case HostSerialConnector::BackendLocalSocket:
      row.specStack->setCurrentIndex(PageSocket);
      // Pre-fill with the default name if the field is empty so users
      // who just pick "Local socket" get a working setup on Ok.
      if (row.socketEdit->text().isEmpty())
        row.socketEdit->setText(defaultSocketName(index));
      break;
    default:
      row.specStack->setCurrentIndex(PageNone);
      break;
  }
}

void SerialPortsDialog::populateSerialPortCombo(QComboBox * cb, const QString & currentPortName)
{
  cb->clear();
  cb->addItem(tr("Not Assigned"), "");
  if (currentPortName.isEmpty())
    cb->setCurrentIndex(0);

  const auto serialPortInfos = QSerialPortInfo::availablePorts();
  for (int i = 0; i < serialPortInfos.size(); i++) {
    const auto portInfo = serialPortInfos[i];
    cb->addItem(portInfo.portName(), portInfo.portName());
    if (portInfo.portName() == currentPortName)
      cb->setCurrentIndex(i + 1);
  }
}

void SerialPortsDialog::writeOutputs()
{
  const auto readRow = [this](int index, int & outKind, QString & outSpec) {
    PortRow & row = auxRows[index];
    const int kind = row.typeCombo->currentData().toInt();
    outKind = kind;
    switch (kind) {
      case HostSerialConnector::BackendSerialPort:
        outSpec = row.serialCombo->currentData().toString();
        break;
      case HostSerialConnector::BackendLocalSocket: {
        QString name = row.socketEdit->text().trimmed();
        if (name.isEmpty())
          name = defaultSocketName(index);
        outSpec = name;
        break;
      }
      default:
        outSpec.clear();
        break;
    }
  };

  readRow(0, aux1Kind, aux1Spec);
  readRow(1, aux2Kind, aux2Spec);
}

void SerialPortsDialog::on_cancelButton_clicked()
{
  this->reject();
}

void SerialPortsDialog::on_okButton_clicked()
{
  writeOutputs();
  this->accept();
}

void SerialPortsDialog::on_refreshButton_clicked()
{
  // Preserve current selection across refresh.
  for (int i = 0; i < 2; i++) {
    PortRow & row = auxRows[i];
    const QString current = row.serialCombo->currentData().toString();
    populateSerialPortCombo(row.serialCombo, current);
  }
}
