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

#include <QSerialPort>
#include <QMutex>

#define MAX_HOST_SERIAL 2

class HostSerialConnector : public QObject
{
    Q_OBJECT

  public:
    explicit HostSerialConnector(QObject * parent, SimulatorInterface * simulator);
    ~HostSerialConnector();

    QString getConnectedSerialPortName(int index);

  public slots:
    void connectSerialPort(int index, QString portName);
    void sendSerialData(const quint8 index, const QByteArray & data);
    void setSerialEncoding(const quint8 index, const quint8 encoding);
    void setSerialBaudRate(const quint8 index, const quint32 baudrate);
    void serialStart(const quint8 index);
    void serialStop(const quint8 index);

 private:
    SimulatorInterface * simulator;

    QRecursiveMutex hostAuxPortsMutex;
    QSerialPort * hostAuxPorts[MAX_HOST_SERIAL];
    quint8 hostAuxPortsEncoding[MAX_HOST_SERIAL];
    quint32 hostAuxPortsBaudRate[MAX_HOST_SERIAL];
    bool hostAuxPortsOpen[MAX_HOST_SERIAL];
};
