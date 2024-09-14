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

#pragma once

#include "eeprominterface.h"

#include <QtCore>

class OpenTxFirmware: public Firmware
{
  Q_DECLARE_TR_FUNCTIONS(OpenTxFirmware)

  public:
    OpenTxFirmware(const QString & id, OpenTxFirmware * parent):
      Firmware(parent, id, parent->getName(), parent->getBoard(),
               parent->getDownloadId(), parent->getSimulatorId(), parent->getHwDefnId())
    {
    }

    OpenTxFirmware(const QString & id, const QString & name, const Board::Type board,
                   const QString & downloadId = QString(), const QString & simulatorId = QString(), const QString & hwdefnId = QString()):
      Firmware(id, name, board, downloadId, simulatorId, hwdefnId)
    {
      //  Note: these align with the radio NOT computer locales - TODO harmonise with ISO and one list!!!
      addLanguage("en");
      addLanguage("cn");
      addLanguage("cz");
      addLanguage("da");
      addLanguage("de");
      addLanguage("es");
      addLanguage("fi");
      addLanguage("fr");
      addLanguage("he");
      addLanguage("hu");
      addLanguage("it");
      addLanguage("jp");
      addLanguage("nl");
      addLanguage("pl");
      addLanguage("pt");
      addLanguage("ru");
      addLanguage("se");
      addLanguage("sk");
      addLanguage("tw");
      addLanguage("ua");
    }

    virtual Firmware * getFirmwareVariant(const QString & id);

    virtual QString getStampUrl() { return ""; }  //  depreciated

    virtual QString getReleaseNotesUrl();

    virtual QString getFirmwareUrl() { return ""; }  //  depreciated;

    virtual int getCapability(Capability);

    virtual QString getCapabilityStr(Capability);

    virtual QString getAnalogInputName(unsigned int index)
    {
      qDebug() << "WARNING: Depreciate function called. Always returns empty string!";
      return QString();
    }

    virtual QTime getMaxTimerStart();

    //  moved to ModuleData EdgeTX v2.9
    virtual bool isAvailable(PulsesProtocol proto, int port=0)
    {
      qDebug() << "WARNING: Depreciate function called. Always returns false!";
      return false;
    }

  protected:

    QString getFirmwareBaseUrl() { return ""; }  //  depreciated;

};

void registerOpenTxFirmwares();
void unregisterOpenTxFirmwares();
