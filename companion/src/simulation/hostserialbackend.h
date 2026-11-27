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

#include <QByteArray>
#include <QObject>
#include <QString>

// Abstract host-side transport for a simulator aux serial port.
//
// The simulator firmware sees a generic byte stream; on the host side
// the bytes can come from a real QSerialPort device, a local socket,
// or any other source. Each backend implementation hides the
// transport details behind this interface so HostSerialConnector can
// stay transport-agnostic.
//
// Backends are owned by HostSerialConnector and live on the same
// thread as the connector. Implementations should emit dataReceived
// for incoming bytes and errorOccurred (with a user-facing message)
// when something goes wrong opening or operating the transport.
class HostSerialBackend : public QObject
{
    Q_OBJECT

  public:
    explicit HostSerialBackend(QObject * parent = nullptr) : QObject(parent) {}
    ~HostSerialBackend() override = default;

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    virtual void write(const QByteArray & data) = 0;

    // No-ops for transports without a notion of bit-level encoding /
    // baudrate (e.g. local sockets). Serial backends override these.
    virtual void setBaudrate(quint32 /*baudrate*/) {}
    virtual void setEncoding(quint8 /*encoding*/) {}

    // Human-readable identifier (port name, socket path, ...) shown
    // in dialogs and log output.
    virtual QString displayName() const = 0;

  signals:
    void dataReceived(const QByteArray & data);
    void errorOccurred(const QString & message);
};
