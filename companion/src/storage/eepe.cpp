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

#include "eepe.h"
#include <QFile>
#include "eeprominterface.h"
#include "hexinterface.h"

bool EepeFormat::load(RadioData & radioData)
{
  QFile file(filename);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setError(tr("Unable to open %1: %2").arg(filename).arg(file.errorString()));
    return false;
  }

  QTextStream inputStream(&file);
  QString hline = inputStream.readLine();
  if (hline != EEPE_EEPROM_FILE_HEADER) {
    qDebug() << qPrintable("[eepe] No EEPE header");
    return false;
  }

  qDebug() << qPrintable("[eepe] EEPE header found");

  QByteArray eeprom(Boards::getEEpromSize(Board::BOARD_UNKNOWN), 0);
  int eeprom_size = HexInterface(inputStream).load((uint8_t *)eeprom.data(), Boards::getEEpromSize(Board::BOARD_UNKNOWN));
  if (!eeprom_size) {
    setError(tr("Invalid EEPROM file %1").arg(filename));
    return false;
  }
  eeprom.resize(eeprom_size);

  return extract(radioData, eeprom);
}
