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

#include "bineeprom.h"
#include "eeprominterface.h"
#include <QFile>
#include <bitset>

// TODO should be RleFormat, RawFormat etc.

bool BinEepromFormat::load(RadioData & radioData)
{
  QFile file(filename);

  int size = file.size();

  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "Unable to open" << filename << file.errorString();
    return false;
  }

  QByteArray eeprom(size, 0);
  int result = file.read((char *)eeprom.data(), size);
  if (result != size) {
    setError(tr("Error reading %1: %2").arg(filename).arg(file.errorString()));
    return false;
  }

  //TODO: show warnings if any
  return extract(radioData, eeprom);
}

bool BinEepromFormat::write(const RadioData & radioData)
{
  bool result;
  EEPROMInterface * eepromInterface = getCurrentEEpromInterface();
  int size = Boards::getEEpromSize(eepromInterface->getBoard());
  if (size == 0) {
    return false;
  }
  uint8_t * eeprom = (uint8_t *)malloc(size);
  int eeprom_size = eepromInterface->save(eeprom, radioData, 0, getCurrentFirmware()->getVariantNumber());
  if (eeprom_size) {
    result = writeToFile(eeprom, eeprom_size);
  }
  else {
    // TODO here we could call setError(eepromInterface->errors())
    setError(tr("Cannot save EEPROM"));
    result = false;
  }
  free(eeprom);
  return result;
}

bool BinEepromFormat::writeToFile(const uint8_t * eeprom, uint32_t size)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    setError(tr("Cannot open file %1:\n%2.").arg(filename).arg(file.errorString()));
    return false;
  }

  QTextStream outputStream(&file);
  qint64 len = file.write((char *)eeprom, size);
  if (len != qint64(size)) {
    setError(tr("Error writing file %1:\n%2.").arg(filename).arg(file.errorString()));
    return false;
  }

  return true;
}

bool BinEepromFormat::extract(RadioData & radioData, const QByteArray & eeprom)
{
  std::bitset<NUM_ERRORS> errors;

  foreach(EEPROMInterface * eepromInterface, eepromInterfaces) {
    std::bitset<NUM_ERRORS> result((unsigned long long)eepromInterface->load(radioData, (uint8_t *)eeprom.data(), eeprom.size()));
    if (result.test(ALL_OK)) {
      if (errors.test(HAS_WARNINGS)) {
        setWarning(EEPROMInterface::getEepromWarnings(errors.to_ulong()));
      }
      board = eepromInterface->getBoard();
      return true;
    }
    else {
      errors |= result;
    }
  }

  setError(tr("Invalid binary EEPROM file %1").arg(filename));
  return false;
}
