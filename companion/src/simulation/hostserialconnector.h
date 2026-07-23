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

#include "simulatorinterface.h"

#include <QMutex>

class HostSerialBackend;

#define MAX_HOST_SERIAL 2

// Owns one HostSerialBackend per simulator aux serial port and
// shuttles bytes between the simulator and whichever transport the
// user picked (real serial port, local socket, ...). Per-port
// encoding / baudrate state is cached here so it survives backend
// swaps and reconnects.
class HostSerialConnector : public QObject
{
    Q_OBJECT

  public:
    enum BackendKind {
      BackendNone = 0,
      BackendSerialPort,
      BackendLocalSocket,
    };
    Q_ENUM(BackendKind)

    explicit HostSerialConnector(QObject * parent, SimulatorInterface * simulator);
    ~HostSerialConnector();

    BackendKind getBackendKind(int index) const;
    QString getBackendSpec(int index) const;

    // Convenience: only meaningful when the active backend is a
    // QLocalSocketBackend. Returns an empty string otherwise.
    QString getLocalSocketFullName(int index) const;

  public slots:
    // kind=BackendNone or empty spec disconnects the port. For
    // BackendSerialPort, spec is the device name (e.g. "ttyUSB0");
    // for BackendLocalSocket, spec is the QLocalServer listen name.
    void connectBackend(int index, int kind, const QString & spec);
    void sendSerialData(const quint8 index, const QByteArray & data);
    void setSerialEncoding(const quint8 index, const quint8 encoding);
    void setSerialBaudRate(const quint8 index, const quint32 baudrate);
    void serialStart(const quint8 index);
    void serialStop(const quint8 index);

  signals:
    // Surfaced to the UI layer (e.g. SimulatorMainWindow) so it can
    // show the user a message box. Index identifies the aux port.
    void backendError(int index, const QString & message);

  private:
    void setBackend(int index, HostSerialBackend * backend, BackendKind kind);

    SimulatorInterface * simulator;

    mutable QRecursiveMutex hostAuxPortsMutex;
    HostSerialBackend * hostAuxBackends[MAX_HOST_SERIAL];
    BackendKind hostAuxBackendKinds[MAX_HOST_SERIAL];
    quint8 hostAuxPortsEncoding[MAX_HOST_SERIAL];
    quint32 hostAuxPortsBaudRate[MAX_HOST_SERIAL];
    bool hostAuxPortsOpen[MAX_HOST_SERIAL];
};
