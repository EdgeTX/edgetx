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

#include "firmwarefactories.h"
#include "helpers_json.h"

#include <QFileInfo>

FirmwareFactory::FirmwareFactory(const QString & id) :
  m_firmware(new Firmware(id))
{}

FirmwareFactories::FirmwareFactories()
{
  registerAllFirmwares();
}

FirmwareFactories::~FirmwareFactories()
{
  //qDebug() << "Unregister all firmware factories";
  for (auto *registeredFactory : registeredFactories)
    delete registeredFactory;
}

Firmware * FirmwareFactories::getFirmware(const QString & id, bool forceLoad) const
{
  for (auto *registeredFactory : registeredFactories) {
    auto firmware = registeredFactory->firmware();

    if (firmware->getId() == id) {
      if (!firmware->isLoaded() && forceLoad)
        firmware->loadDefinition();

      return firmware;
    }
  }

  qDebug() << "Error: firmware id:" << id << "not registered";
  return Firmware::getDefault();
}

QList<Firmware *> FirmwareFactories::getRegisteredFirmwares() const
{
  QList<Firmware *> ret;

  for (auto *registeredFactory : registeredFactories) {
    Firmware *firmware = registeredFactory->firmware();
    if (firmware->isSupported())
      ret.append(firmware);
  }

  return ret;
}

bool FirmwareFactories::isAvailable(const QString & id) const
{
  for (auto *registeredFactory : registeredFactories) {
    if (registeredFactory->firmware()->getId() == id)
      return true;
  }

  return false;
}

void FirmwareFactories::registerAllFirmwares()
{
  QStringList registered;
  QStringList filters = { "*.json" };

  QDirIterator it(QString("%1/").arg(FWDEFNSDIR), filters, QDir::Files);

  while (it.hasNext()) {
    QString path = it.next();
    //qDebug() << "found file:" << path;
    QJsonDocument *doc = new QJsonDocument();

    if (Json::load(doc, path)) {
      QJsonObject obj = doc->object();
      // ignore intermediate definitions
      if (!Json::value(obj, "hidden", false, false).toBool()) {
        if (registerFirmware(QFileInfo(path).baseName()))
          registered.append(getFirmware(QFileInfo(path).baseName(), false)->getName());
      } else {
        //qInfo() << "Ignoring hidden file:" << path;
      }
    }

    delete doc;
  }

  Firmware::setDefault(registeredFactories.first()->firmware());
  qDebug() << "Registered firmwares:" << registered;
}

bool FirmwareFactories::registerFirmware(const QString & id)
{
  if (isAvailable(id)) {
    qWarning() << "Error - Firmware id:" << id << "already registered";
    return false;
  }

  FirmwareFactory *factory = new FirmwareFactory(id);
  registeredFactories.append(factory);
  //qDebug() << "Registered firmware:" << factory->getFirmware()->getName();
  return true;
}

QMap<QString, QString> FirmwareFactories::registeredFirmwares() const
{
  QMap<QString, QString> ret;

  for (auto *registeredFactory : registeredFactories) {
    Firmware *firmware = registeredFactory->firmware();
    if (firmware->isSupported())
      ret.insert(firmware->getId(), firmware->getName());
  }

  return ret;
}
