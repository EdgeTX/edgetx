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

#include "eeprominterface.h"
#include "firmwareinterface.h"
#include "boardfactories.h"

#include <QtCore>
#include <QMessageBox>
#include <bitset>

float ValToTim(int value)
{
   return ((value < -109 ? 129 + value : (value < 7 ? (113 + value) * 5 : (53 + value) * 10)) / 10.0);
}

int TimToVal(float value)
{
  int temp;
  if (value > 60) {
    temp = 136 + round((value - 60));
  }
  else if (value > 2) {
    temp = 20 + round((value - 2.0) * 2.0);
  }
  else {
    temp = round(value * 10.0);
  }
  return (temp - 129);
}

Firmware::Firmware(Firmware * base, const QString & id, const QString & name, Board::Type board,
                  const QString & downloadId, const QString & simulatorId, const QString & hwdefnId) :
  id(id),
  name(name),
  board(board),
  variantBase(0),
  base(base),
  downloadId(downloadId),
  simulatorId(simulatorId),
  hwdefnId(hwdefnId)
{
  gBoardFactories->registerBoard(board, hwdefnId);
}

// static
QVector<Firmware *> Firmware::registeredFirmwares;
Firmware * Firmware::defaultVariant = nullptr;
Firmware * Firmware::currentVariant = nullptr;

// static
Firmware * Firmware::getFirmwareForId(const QString & id)
{
  foreach(Firmware * firmware, registeredFirmwares) {
    Firmware * result = firmware->getFirmwareVariant(id);
    if (result) {
      return result;
    }
  }

  return defaultVariant;
}

unsigned int Firmware::getVariantNumber()
{
  unsigned int result = 0;
  const Firmware * base = getFirmwareBase();
  QStringList options = id.mid(base->getId().length() + 1).split("-", Qt::SkipEmptyParts);
  foreach (QString option, options) {
    foreach (OptionsGroup group, base->opts) {
      foreach (Option opt, group) {
        if (opt.name == option) {
          result += opt.variant;
        }
      }
    }
  }
  return result;
}

void Firmware::addLanguage(const char * lang)
{
  languages.push_back(lang);
}

//void Firmware::addTTSLanguage(const char * lang)
//{
//  ttslanguages.push_back(lang);
//}

void Firmware::addOption(const char * option, const QString & tooltip, unsigned variant)
{
  addOption(Option(option, tooltip, variant));
}

void Firmware::addOption(const Option & option)
{
  addOptionsGroup({option});
}

void Firmware::addOptionsGroup(const OptionsGroup & options)
{
  this->opts.append(options);
}

QString Firmware::getFlavour()
{
  QStringList strl = id.split("-");
  return strl.count() >= 2 ? strl.at(1) : QString();
}
