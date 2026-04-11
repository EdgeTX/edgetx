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

class QLocalServer;
class QLocalSocket;

// HostSerialBackend backed by a QLocalServer (Unix domain socket on
// macOS / Linux, named pipe on Windows). Lets external scripts stand
// in for hardware on a simulator aux serial port.
//
// Single-client semantics: while one client is connected, additional
// connection attempts are accepted briefly and then closed. We use
// last-write-wins for outgoing data — bytes from the simulator only
// reach the active client.
class QLocalSocketBackend : public HostSerialBackend
{
    Q_OBJECT

  public:
    QLocalSocketBackend(const QString & socketName, QObject * parent = nullptr);
    ~QLocalSocketBackend() override;

    bool open() override;
    void close() override;
    bool isOpen() const override;

    void write(const QByteArray & data) override;

    QString displayName() const override;

    // Resolved native path / pipe name (e.g.
    // /var/folders/.../edgetx-sim-aux1 or
    // \\.\pipe\edgetx-sim-aux1). Empty when the server is not
    // listening.
    QString fullServerName() const;

  private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

  private:
    QString socketName;
    QLocalServer * server;
    QLocalSocket * client;
};
