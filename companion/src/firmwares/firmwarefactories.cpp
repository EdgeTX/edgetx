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

FirmwareFactories* gFirmwareFactories = nullptr;

FirmwareFactory::FirmwareFactory(const QString & id) :
  m_firmware(new Firmware(id))
{}

FirmwareFactories::FirmwareFactories()
{
  registerAllFirmwares();
}

FirmwareFactories::~FirmwareFactories()
{
}

Firmware * FirmwareFactories::getFirmware(const QString & id) const
{
  for (auto *registeredFactory : registeredFactories) {
    if (registeredFactory->getFirmware()->getId() == id)
      return registeredFactory->getFirmware();
  }

  return Firmware::getDefault();
}

QList<Firmware *> FirmwareFactories::getRegisteredFirmwares() const
{
  QList<Firmware *> ret;

  for (auto *registeredFactory : registeredFactories) {
    Firmware *firmware = registeredFactory->getFirmware();
    if (firmware->isSupported())
      ret.append(firmware);
  }

  return ret;
}

bool FirmwareFactories::isAvailable(const QString & id) const
{
  for (auto *registeredFactory : registeredFactories) {
    if (registeredFactory->getFirmware()->getId() == id)
      return true;
  }

  return false;
}

bool FirmwareFactories::loadDefinition(const QString & id)
{
  Firmware *firmware = getFirmware(id);
  return firmware ? firmware->loadDefinition() : false;
}

void FirmwareFactories::registerAllFirmwares()
{
  QStringList filters = { "*.json" };

  QDirIterator it(QString("%1/").arg(FWDEFNSDIR), filters, QDir::Files);

  while (it.hasNext()) {
    QString path = it.next();
    //qDebug() << "found file:" << path;
    QJsonDocument *doc = new QJsonDocument();

    if (Json::load(doc, path)) {
      QJsonObject obj = doc->object();
      // ignore intermediate definitions
      if (!Json::getValueBool(obj, "hidden", false)) {
        registerFirmware(QFileInfo(path).baseName());
      } else {
        qInfo() << "Ignoring hidden file:" << path;
      }
    }

    delete doc;
  }

  Firmware::setDefault(registeredFactories.first()->getFirmware());
}

QMap<QString, QString> FirmwareFactories::registeredFirmwares() const
{
  QMap<QString, QString> ret;

  for (auto *registeredFactory : registeredFactories) {
    Firmware *firmware = registeredFactory->getFirmware();
    if (firmware->isSupported())
      ret.insert(firmware->getId(), firmware->getName());
  }

  return ret;
}

bool FirmwareFactories::registerFirmware(const QString & id)
{
  Firmware* firmware = getFirmware(id);

  if (firmware) {
    qWarning() << "Error - Firmware id:" << id << "already registered";
    return false;
  }

  FirmwareFactory *ff = new FirmwareFactory(id);

  if (registerFactory(ff)) {
    qDebug() << "Registered firmware:" << ff->getFirmware()->getId() << ff->getFirmware()->getName();
    return true;
  }

  delete ff;
  return false;
}

bool FirmwareFactories::registerFactory(FirmwareFactory * factory)
{
  registeredFactories.append(factory);
  return true;
}

void FirmwareFactories::unregisterFactories()
{
  for (auto *registeredFactory : registeredFactories)
    delete registeredFactory;
}
