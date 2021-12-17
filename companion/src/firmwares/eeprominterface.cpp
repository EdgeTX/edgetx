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

#include "eeprominterface.h"
#include "firmwares/opentx/opentxeeprom.h"
#include "firmwareinterface.h"

#include <QtCore>
#include <QMessageBox>
#include <bitset>

float ValToTim(int value)
{
   return ((value < -109 ? 129+value : (value < 7 ? (113+value)*5 : (53+value)*10))/10.0);
}

int TimToVal(float value)
{
  int temp;
  if (value>60) {
    temp=136+round((value-60));
  }
  else if (value>2) {
    temp=20+round((value-2.0)*2.0);
  }
  else {
    temp=round(value*10.0);
  }
  return (temp-129);
}


/*
 * EEpromInterface globals
 */

// TODO: No globals

QList<EEPROMInterface *> eepromInterfaces;

void unregisterEEpromInterfaces()
{
  foreach(EEPROMInterface * intf, eepromInterfaces) {
    // qDebug() << "UnregisterEepromInterfaces(): deleting " <<  QString::number( reinterpret_cast<uint64_t>(intf), 16 );
    delete intf;
  }
  OpenTxEepromCleanup();
}

#if 0  // TODO: remove if unused, currently commented out in mdiChild.cpp
// TODO: No GUI here, e.g. return string list instead
void EEPROMInterface::showEepromErrors(QWidget *parent, const QString &title, const QString &mainMessage, unsigned long errorsFound)
{
  std::bitset<NUM_ERRORS> errors((unsigned long long)errorsFound);
  QStringList errorsList;

  errorsList << tr("Possible causes for this:");

  if (errors.test(UNSUPPORTED_NEWER_VERSION)) { errorsList << tr("- Eeprom is from a newer version of OpenTX"); }
  if (errors.test(NOT_OPENTX)) { errorsList << tr("- Eeprom is not from OpenTX"); }
  if (errors.test(NOT_ERSKY9X)) { errorsList << tr("- Eeprom is not from ErSky9X"); }
  if (errors.test(WRONG_SIZE)) { errorsList << tr("- Eeprom size is invalid"); }
  if (errors.test(WRONG_FILE_SYSTEM)) { errorsList << tr("- Eeprom file system is invalid"); }
  if (errors.test(UNKNOWN_BOARD)) { errorsList << tr("- Eeprom is from a unknown board"); }
  if (errors.test(WRONG_BOARD)) { errorsList << tr("- Eeprom is from the wrong board"); }
  if (errors.test(BACKUP_NOT_SUPPORTED)) { errorsList << tr("- Eeprom backup not supported"); }

  if (errors.test(UNKNOWN_ERROR)) { errorsList << tr("- Something that couldn't be guessed, sorry"); }

  if (errors.test(HAS_WARNINGS)) {
    errorsList << tr("Warning:");
    if (errors.test(WARNING_WRONG_FIRMWARE)) { errorsList << tr("- Your radio probably uses a wrong firmware,\n eeprom size is 4096 but only the first 2048 are used"); }
  }

  QMessageBox msgBox(parent);
  msgBox.setWindowTitle(title);
  msgBox.setIcon(QMessageBox::Critical);
  msgBox.setText(mainMessage);
  msgBox.setInformativeText(errorsList.join("\n"));
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
}
#endif

// TODO: No GUI here, e.g. return string list instead
QString EEPROMInterface::getEepromWarnings(unsigned long errorsFound)
{
  std::bitset<NUM_ERRORS> errors((unsigned long long)errorsFound);
  QStringList warningsList;
  if (errors.test(WARNING_WRONG_FIRMWARE)) {
    warningsList << tr("- Your radio probably uses a wrong firmware,\n eeprom size is 4096 but only the first 2048 are used");
  }
  if (errors.test(OLD_VERSION)) {
    warningsList << tr("- Your eeprom is from an old version of OpenTX, upgrading!\n To keep your original file as a backup, please choose File -> Save As specifying a different name.");
  }

  return warningsList.join("\n");
}


/*
 * Firmware
 */

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
  QStringList options = id.mid(base->getId().length()+1).split("-", QString::SkipEmptyParts);
  foreach(QString option, options) {
    foreach(OptionsGroup group, base->opts) {
      foreach(Option opt, group) {
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
