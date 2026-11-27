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

#include "hostserialbackend.h"

#include <QSerialPort>

// HostSerialBackend implementation backed by a real QSerialPort
// device (USB-serial adapter, native UART, ...). Mirrors the original
// HostSerialConnector behaviour 1:1.
class QSerialPortBackend : public HostSerialBackend
{
    Q_OBJECT

  public:
    QSerialPortBackend(const QString & portName, QObject * parent = nullptr);
    ~QSerialPortBackend() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;

    void write(const QByteArray & data) override;

    void setBaudrate(quint32 baudrate) override;
    void setEncoding(quint8 encoding) override;

    QString displayName() const override;

  private:
    QSerialPort * port;
};
