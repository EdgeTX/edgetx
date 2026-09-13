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

#include "hostserialbackend.h"
#include "hostserialbackend_serialport.h"
#include "hostserialbackend_localsocket.h"

HostSerialConnector::HostSerialConnector(QObject *parent, SimulatorInterface *simulator)
  : QObject(parent),
    simulator(simulator)
{
  for (int i = 0; i < MAX_HOST_SERIAL; i++) {
    hostAuxBackends[i] = nullptr;
    hostAuxBackendKinds[i] = BackendNone;
    hostAuxPortsEncoding[i] = SERIAL_ENCODING_8N1;
    hostAuxPortsBaudRate[i] = 9600;
    hostAuxPortsOpen[i] = false;
  }
}

HostSerialConnector::~HostSerialConnector()
{
  for (int i = 0; i < MAX_HOST_SERIAL; i++) {
    if (hostAuxBackends[i] != nullptr) {
      hostAuxBackends[i]->close();
      hostAuxBackends[i]->deleteLater();
    }
  }
}

HostSerialConnector::BackendKind HostSerialConnector::getBackendKind(int index) const
{
  if (index >= MAX_HOST_SERIAL)
    return BackendNone;

  QMutexLocker locker(&hostAuxPortsMutex);
  return hostAuxBackendKinds[index];
}

QString HostSerialConnector::getBackendSpec(int index) const
{
  if (index >= MAX_HOST_SERIAL)
    return QString();

  QMutexLocker locker(&hostAuxPortsMutex);

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return QString();

  return backend->displayName();
}

QString HostSerialConnector::getLocalSocketFullName(int index) const
{
  if (index >= MAX_HOST_SERIAL)
    return QString();

  QMutexLocker locker(&hostAuxPortsMutex);

  if (hostAuxBackendKinds[index] != BackendLocalSocket)
    return QString();

  auto * backend = qobject_cast<QLocalSocketBackend *>(hostAuxBackends[index]);
  return backend != nullptr ? backend->fullServerName() : QString();
}

void HostSerialConnector::setBackend(int index, HostSerialBackend * backend, BackendKind kind)
{
  // Caller holds hostAuxPortsMutex.
  HostSerialBackend * old = hostAuxBackends[index];
  if (old != nullptr) {
    old->close();
    old->deleteLater();
  }

  hostAuxBackends[index] = backend;
  hostAuxBackendKinds[index] = backend != nullptr ? kind : BackendNone;

  if (backend == nullptr)
    return;

  connect(backend, &HostSerialBackend::dataReceived, this,
          [this, index](const QByteArray & data) {
            simulator->receiveAuxSerialData(index, data);
          });
  connect(backend, &HostSerialBackend::errorOccurred, this,
          [this, index](const QString & message) {
            emit backendError(index, message);
          });

  // Re-apply cached settings to the freshly created backend.
  backend->setEncoding(hostAuxPortsEncoding[index]);
  backend->setBaudrate(hostAuxPortsBaudRate[index]);
}

void HostSerialConnector::connectBackend(int index, int kind, const QString & spec)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  if (kind == BackendNone || spec.isEmpty()) {
    setBackend(index, nullptr, BackendNone);
    return;
  }

  HostSerialBackend * backend = nullptr;
  switch (kind) {
    case BackendSerialPort:
      backend = new QSerialPortBackend(spec, this);
      break;
    case BackendLocalSocket:
      backend = new QLocalSocketBackend(spec, this);
      break;
    default:
      return;
  }

  setBackend(index, backend, static_cast<BackendKind>(kind));

  if (hostAuxPortsOpen[index])
    serialStart(index);
}

void HostSerialConnector::sendSerialData(const quint8 index, const QByteArray & data)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return;

  backend->write(data);
}

void HostSerialConnector::setSerialEncoding(const quint8 index, const quint8 encoding)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsEncoding[index] = encoding;

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return;

  backend->setEncoding(encoding);
}

void HostSerialConnector::setSerialBaudRate(const quint8 index, const quint32 baudrate)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsBaudRate[index] = baudrate;

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return;

  backend->setBaudrate(baudrate);
}

void HostSerialConnector::serialStart(const quint8 index)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsOpen[index] = true;

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return;

  backend->open();
}

void HostSerialConnector::serialStop(const quint8 index)
{
  if (index >= MAX_HOST_SERIAL)
    return;

  QMutexLocker locker(&hostAuxPortsMutex);

  hostAuxPortsOpen[index] = false;

  HostSerialBackend * backend = hostAuxBackends[index];
  if (backend == nullptr)
    return;

  backend->close();
}
