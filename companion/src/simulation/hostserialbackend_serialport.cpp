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

#include "hostserialbackend_serialport.h"

#include "simulatorinterface.h"

#include <QtDebug>

QSerialPortBackend::QSerialPortBackend(const QString & portName, QObject * parent)
  : HostSerialBackend(parent),
    port(new QSerialPort(portName, this))
{
  connect(port, &QSerialPort::readyRead, this, [this]() {
    emit dataReceived(port->readAll());
  });
}

QSerialPortBackend::~QSerialPortBackend()
{
  if (port->isOpen())
    port->close();
}

bool QSerialPortBackend::open()
{
  if (port->open(QIODevice::ReadWrite)) {
    qDebug() << "Opened host serial" << port->portName();
    return true;
  }
  emit errorOccurred(port->errorString());
  return false;
}

void QSerialPortBackend::close()
{
  port->close();
}

bool QSerialPortBackend::isOpen() const
{
  return port->isOpen();
}

void QSerialPortBackend::write(const QByteArray & data)
{
  port->write(data);
}

void QSerialPortBackend::setBaudrate(quint32 baudrate)
{
  if (!port->setBaudRate(baudrate))
    qDebug() << "Failed to set baudrate";
}

void QSerialPortBackend::setEncoding(quint8 encoding)
{
  switch (encoding) {
    case SERIAL_ENCODING_8N1:
      port->setDataBits(QSerialPort::Data8);
      port->setParity(QSerialPort::NoParity);
      port->setStopBits(QSerialPort::OneStop);
      break;
    case SERIAL_ENCODING_8E2:
      port->setDataBits(QSerialPort::Data8);
      port->setParity(QSerialPort::EvenParity);
      port->setStopBits(QSerialPort::TwoStop);
      break;
    default:
      // QSerialPort can't do SERIAL_ENCODING_PXX1_PWM
      break;
  }
}

QString QSerialPortBackend::displayName() const
{
  return port->portName();
}
