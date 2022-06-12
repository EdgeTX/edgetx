/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "bineeprom.h"
#include "eepe.h"
#include "otx.h"
#include "etx.h"
#include "sdcard.h"
#include "yaml.h"
#include "firmwareinterface.h"
#include "eeprominterface.h"
#include <QFileInfo>

StorageType getStorageType(const QString & filename)
{
  QString suffix = QFileInfo(filename).suffix().toUpper();
  if (suffix == "HEX")
    return STORAGE_TYPE_HEX;
  else if (suffix == "BIN")
    return STORAGE_TYPE_BIN;
  else if (suffix == "EEPM")
    return STORAGE_TYPE_EEPM;
  else if (suffix == "EEPE")
    return STORAGE_TYPE_EEPE;
  else if (suffix == "XML")
    return STORAGE_TYPE_XML;
  else if (suffix == "OTX")
    return STORAGE_TYPE_OTX;
  else if (suffix == "ETX")
    return STORAGE_TYPE_ETX;
  else if (suffix == "YML")
    return STORAGE_TYPE_YML;
  else
    return STORAGE_TYPE_UNKNOWN;
}

void registerStorageFactory(StorageFactory * factory);

QList<StorageFactory *> registeredStorageFactories;

void registerStorageFactory(StorageFactory * factory)
{
  qDebug() << "register storage" << factory->name();
  registeredStorageFactories.push_back(factory);
}

void registerStorageFactories()
{
  registerStorageFactory(new DefaultStorageFactory<BinEepromFormat>("bin"));
  registerStorageFactory(new DefaultStorageFactory<EepeFormat>("eepe"));
  registerStorageFactory(new DefaultStorageFactory<HexEepromFormat>("hex"));
  registerStorageFactory(new DefaultStorageFactory<OtxFormat>("otx"));
  registerStorageFactory(new DefaultStorageFactory<EtxFormat>("etx"));
  registerStorageFactory(new DefaultStorageFactory<YamlFormat>("yml"));
  registerStorageFactory(new SdcardStorageFactory());
}

void unregisterStorageFactories()
{
  foreach (StorageFactory * factory, registeredStorageFactories)
    delete factory;
}

bool Storage::load(RadioData & radioData)
{
  QFile file(filename);
  if (!file.exists()) {
    setError(tr("Unable to find file %1!").arg(filename));
    return false;
  }

  bool ret = false;
  foreach(StorageFactory * factory, registeredStorageFactories) {
    if (factory->probe(filename)) {
      StorageFormat * format = factory->instance(filename);
      if (format->load(radioData)) {
        board = format->getBoard();
        setWarning(format->warning());
        ret = true;
        delete format;
        break;
      }
      else {
        setError(format->error());
      }
      delete format;
    }
  }

  return ret;
}

bool Storage::write(const RadioData & radioData)
{
  bool ret = false;
  foreach(StorageFactory * factory, registeredStorageFactories) {
    if (factory->probe(filename)) {
      StorageFormat * format = factory->instance(filename);
      ret = format->write(radioData);
      delete format;
      break;
    }
  }
  return ret;
}

bool convertEEprom(const QString & sourceEEprom, const QString & destinationEEprom, const QString & firmwareFilename)
{
  FirmwareInterface firmware(firmwareFilename);
  if (!firmware.isValid())
    return false;

  uint8_t version = firmware.getEEpromVersion();
  unsigned int variant = firmware.getEEpromVariant();

  QSharedPointer<RadioData> radioData = QSharedPointer<RadioData>(new RadioData());
  Storage storage(sourceEEprom);
  if (!storage.load(*radioData))
    return false;

  QByteArray eeprom(Boards::getEEpromSize(Board::BOARD_UNKNOWN), 0);
  int size = getCurrentEEpromInterface()->save((uint8_t *)eeprom.data(), *radioData, version, variant);
  if (size == 0) {
    return false;
  }

  QFile destinationFile(destinationEEprom);
  if (!destinationFile.open(QIODevice::WriteOnly)) {
    return false;
  }

  int result = destinationFile.write(eeprom.constData(), size);
  destinationFile.close();
  return (result == size);
}

#if 0
unsigned long LoadBackup(RadioData & radioData, uint8_t * eeprom, int size, int index)
{
  std::bitset<NUM_ERRORS> errors;

    foreach(EEPROMInterface *eepromInterface, eepromInterfaces) {
      std::bitset<NUM_ERRORS> result((unsigned long long)eepromInterface->loadBackup(radioData, eeprom, size, index));
      if (result.test(ALL_OK)) {
        return result.to_ulong();
      }
      else {
        errors |= result;
      }
    }

  if (errors.none()) {
    errors.set(UNKNOWN_ERROR);
  }
  return errors.to_ulong();
}


unsigned long LoadEepromXml(RadioData & radioData, QDomDocument & doc)
{
  std::bitset<NUM_ERRORS> errors;

    foreach(EEPROMInterface *eepromInterface, eepromInterfaces) {
      std::bitset<NUM_ERRORS> result((unsigned long long)eepromInterface->loadxml(radioData, doc));
      if (result.test(ALL_OK)) {
        return result.to_ulong();
      }
      else {
        errors |= result;
      }
    }

  if (errors.none()) {
    errors.set(UNKNOWN_ERROR);
  }
  return errors.to_ulong();
}
#endif
