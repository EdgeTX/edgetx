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

#include "hostserialconnector.h"

HostSerialConnector::HostSerialConnector(QObject *parent, SimulatorInterface *simulator)
  : simulator(simulator)
{
  for (int i = 0; i < MAX_HOST_SERIAL; i++)
    hostAuxPorts[i] = nullptr;
}

HostSerialConnector::~HostSerialConnector()
{
  for (int i = 0; i < MAX_HOST_SERIAL; i++) {
    if (hostAuxPorts[i] != nullptr) {
      hostAuxPorts[i]->close();
      hostAuxPorts[i]->deleteLater();
    }
  }
}

void HostSerialConnector::connectSerialPort(int index, QString portName)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  QSerialPort * port = hostAuxPorts[index];
  if (port != nullptr) {
    port->close();
    port->deleteLater();
  }

  if (portName.isEmpty()) {
    hostAuxPorts[index] = nullptr;
    return;
  }

  port = new QSerialPort(portName, this);
  hostAuxPorts[index] = port;

  setSerialEncoding(index, hostAuxPortsEncoding[index]);
  setSerialBaudRate(index, hostAuxPortsBaudRate[index]);

  connect(port, &QSerialPort::readyRead, [this, index, port]() {
    QByteArray data = port->readAll();
    simulator->receiveAuxSerialData(index, data);
  });

  if (hostAuxPortsOpen[index])
    serialStart(index);
}

void HostSerialConnector::sendSerialData(const quint8 index, const QByteArray & data)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  QSerialPort * port = hostAuxPorts[index];
  if (port == nullptr)
    return;

  port->write(data);
}

void HostSerialConnector::setSerialEncoding(const quint8 index, const SimulatorInterface::SerialEncoding encoding)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsEncoding[index] = encoding;

  QSerialPort * port = hostAuxPorts[index];
  if (port == nullptr)
    return;
      
  switch(encoding) {
  case SimulatorInterface::SerialEncoding::SERIAL_ENCODING_8N1:
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    break;
  case SimulatorInterface::SerialEncoding::SERIAL_ENCODING_8E2:
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::EvenParity);
    port->setStopBits(QSerialPort::TwoStop);
    break;
  default:
    // Do nothing, QSerialPort can't do SERIAL_ENCODING_PXX1_PWM
    break;
  }
}

void HostSerialConnector::setSerialBaudRate(const quint8 index, const quint32 baudrate)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsBaudRate[index] = baudrate;

  QSerialPort * port = hostAuxPorts[index];
  if (port == nullptr)
    return;

  if (!port->setBaudRate(baudrate))
    qDebug() << "Failed to set baudrate";
}

void HostSerialConnector::serialStart(const quint8 index)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsOpen[index] = true;

  QSerialPort * port = hostAuxPorts[index];
  if (port == nullptr)
    return;

  if (!port->open(QIODevice::ReadWrite))
    qDebug() << "Failed to open host serial " << index;
}

void HostSerialConnector::serialStop(const quint8 index)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsOpen[index] = false;

  QSerialPort * port = hostAuxPorts[index];
  if (port == nullptr)
    return;

  port->close();
}
