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

#include <QFileInfo>

FirmwareFactories* gFirmwareFactories = nullptr;

FirmwareFactory::FirmwareFactory(const QString & id, const QString & path, const bool isSupported) :
  m_firmware(new Firmware(id, path, isSupported))
{}

FirmwareFactories::FirmwareFactories()
{
  registerAllFirmwares();
}

FirmwareFactories::~FirmwareFactories()
{
}

QList<Firmware*> FirmwareFactories::getRegisteredFirmwares()
{
  QList<Firmware*> ret;

  for (auto *registeredFactory : registeredFactories)
    ret.append(registeredFactory->firmware());

  return ret;
}

Firmware * FirmwareFactories::firmware(const QString & id) const
{
  for (auto *registeredFactory : registeredFactories) {
    if (registeredFactory->firmware()->id() == id)
      return registeredFactory->firmware();
  }

  return Firmware::getDefault();
}

bool FirmwareFactories::loadDefinition(const QString & id)
{
  Firmware *fw = firmware(id);
  return fw ? fw->loadDefinition() : false;
}

void FirmwareFactories::registerAllFirmwares()
{
  QStringList filters = { "*.json" };

  QDirIterator it(QString("%1/").arg(FWDEFNSDIR), filters, QDir::Files);

  while (it.hasNext()) {
    QString path = it.next();
    //qDebug() << "found file:" << path;
    QJsonDocument *doc = new QJsonDocument();

    if (Firmware::load(doc, path)) {
      QJsonObject obj = doc->object();
      // ignore intermediate definitions
      if (!Firmware::getValueBool(obj, "hidden", false)) {
        QString id = Firmware::getValueString(obj, "id", QFileInfo(path).baseName());
        registerFirmware("edgetx-" % id, path, Firmware::getValueBool(obj, "supported", true));
      } else {
        //qDebug() << "ignoring file:" << path;
      }
    }

    delete doc;
  }

  Firmware::setDefault(registeredFactories.first()->firmware());
}

QMap<QString, QString> FirmwareFactories::registeredFirmware()
{
  QMap<QString, QString> ret;

  for (auto *registeredFactory : registeredFactories) {
    if (registeredFactory->firmware()->isSupported())
      ret.insert(registeredFactory->firmware()->id(), registeredFactory->firmware()->name());
  }

  return ret;
}

bool FirmwareFactories::registerFirmware(const QString & id, const QString & path, const bool isSupported)
{
  Firmware* regfirmware = firmware(id);

  if (regfirmware) {
    qWarning() << "Error - Firmware id:" << id << "file:" << path << "already registered";
    return false;
  }

  FirmwareFactory *ff = new FirmwareFactory(id, path, isSupported);

  if (registerFactory(ff)) {
    qDebug() << "Registered firmware:" << ff->firmware()->id() << ff->firmware()->name();
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
