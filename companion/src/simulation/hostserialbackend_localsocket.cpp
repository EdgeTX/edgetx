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

#include "hostserialbackend_localsocket.h"

#include <QLocalServer>
#include <QLocalSocket>
#include <QtDebug>

QLocalSocketBackend::QLocalSocketBackend(const QString & socketName, QObject * parent)
  : HostSerialBackend(parent),
    socketName(socketName),
    server(new QLocalServer(this)),
    client(nullptr)
{
  connect(server, &QLocalServer::newConnection,
          this, &QLocalSocketBackend::onNewConnection);
}

QLocalSocketBackend::~QLocalSocketBackend()
{
  close();
}

QString QLocalSocketBackend::resolveListenName(const QString & name)
{
#ifdef Q_OS_UNIX
  // Qt defaults to QStandardPaths::TempLocation, which on macOS is
  // a per-user $TMPDIR (e.g. /var/folders/.../T/) — fine for IPC
  // between Qt apps, painful when the other end is a Python script
  // a user is poking from a shell. Pin bare names to /tmp/<name>
  // so the resolved path is predictable on both macOS and Linux.
  // Absolute paths supplied by the user are passed through as-is.
  if (!name.startsWith(QLatin1Char('/')))
    return QStringLiteral("/tmp/") + name;
#endif
  return name;
}

bool QLocalSocketBackend::open()
{
  if (server->isListening())
    return true;

  const QString listenName = resolveListenName(socketName);

  // A previous run (or another process) may have left a stale socket
  // file behind on Unix; QLocalServer::removeServer is the documented
  // way to clear it before re-listening.
  QLocalServer::removeServer(listenName);

  if (!server->listen(listenName)) {
    emit errorOccurred(tr("Failed to listen on local socket \"%1\": %2")
                       .arg(listenName, server->errorString()));
    return false;
  }

  qDebug() << "Listening on local socket" << server->fullServerName();
  return true;
}

void QLocalSocketBackend::close()
{
  if (client != nullptr) {
    client->disconnect(this);
    client->disconnectFromServer();
    client->deleteLater();
    client = nullptr;
  }
  if (server->isListening())
    server->close();
}

bool QLocalSocketBackend::isOpen() const
{
  return server->isListening();
}

void QLocalSocketBackend::write(const QByteArray & data)
{
  if (client != nullptr && client->state() == QLocalSocket::ConnectedState)
    client->write(data);
}

QString QLocalSocketBackend::displayName() const
{
  return socketName;
}

QString QLocalSocketBackend::fullServerName() const
{
  return server->isListening() ? server->fullServerName() : QString();
}

void QLocalSocketBackend::onNewConnection()
{
  while (server->hasPendingConnections()) {
    QLocalSocket * incoming = server->nextPendingConnection();
    if (client != nullptr) {
      // Single-client backend: politely refuse extra connections.
      incoming->disconnectFromServer();
      incoming->deleteLater();
      continue;
    }

    client = incoming;
    connect(client, &QLocalSocket::readyRead,
            this, &QLocalSocketBackend::onClientReadyRead);
    connect(client, &QLocalSocket::disconnected,
            this, &QLocalSocketBackend::onClientDisconnected);
    qDebug() << "Local socket client connected on" << socketName;
  }
}

void QLocalSocketBackend::onClientReadyRead()
{
  if (client == nullptr)
    return;
  emit dataReceived(client->readAll());
}

void QLocalSocketBackend::onClientDisconnected()
{
  if (client == nullptr)
    return;
  client->deleteLater();
  client = nullptr;
  qDebug() << "Local socket client disconnected from" << socketName;
}
