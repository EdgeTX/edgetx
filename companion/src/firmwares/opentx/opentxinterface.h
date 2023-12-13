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

#ifndef _OPENTXINTERFACE_H_
#define _OPENTXINTERFACE_H_

#include "eeprominterface.h"

#include <QtCore>

class RleFile;
class OpenTxFirmware;

class OpenTxEepromInterface : public EEPROMInterface
{
  Q_DECLARE_TR_FUNCTIONS(OpenTxEepromInterface)

  public:

    OpenTxEepromInterface(OpenTxFirmware * firmware);

    virtual ~OpenTxEepromInterface();

    virtual unsigned long load(RadioData &, const uint8_t * eeprom, int size);

    bool loadModelFromBackup(ModelData & model, const uint8_t * data, unsigned int size, uint8_t version, uint32_t variant);

    virtual unsigned long loadBackup(RadioData &, const uint8_t * eeprom, int esize, int index);

    virtual int save(uint8_t * eeprom, const RadioData & radioData, uint8_t version=0, uint32_t variant=0);

    virtual int getSize(const ModelData &);

    virtual int getSize(const GeneralSettings &);

  protected:

    const char * getName();

    EepromLoadErrors checkVersion(unsigned int version);

    bool checkVariant(unsigned int version, unsigned int variant);

    template <class T, class M>
    bool loadFromByteArray(T & dest, const QByteArray & data, uint8_t version, uint32_t variant=0);

  public:
    template <class T, class M>
    bool loadFromByteArray(T & dest, const QByteArray & data);

    template <class T, class M>
    bool saveToByteArray(const T & src, QByteArray & data, uint8_t version=0);

    bool loadRadioSettingsFromRLE(GeneralSettings & settings, RleFile * rleFile, uint8_t version);

    bool loadModelFromRLE(ModelData & model, RleFile * rleFile, unsigned int index, uint8_t version, uint32_t variant);

    void showErrors(const QString & title, const QStringList & errors);

    uint8_t getLastDataVersion(Board::Type board);

    RleFile * efile;

    OpenTxFirmware * firmware;

};

class OpenTxFirmware: public Firmware
{
  Q_DECLARE_TR_FUNCTIONS(OpenTxFirmware)

  public:
    OpenTxFirmware(const QString & id, OpenTxFirmware * parent):
      Firmware(parent, id, parent->getName(), parent->getBoard(),
               parent->getDownloadId(), parent->getSimulatorId(), parent->getHwDefnId())
    {
      setEEpromInterface(parent->getEEpromInterface());
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

extern QList<OpenTxEepromInterface *> opentxEEpromInterfaces;

OpenTxEepromInterface * loadModelFromByteArray(ModelData & model, const QByteArray & data);
OpenTxEepromInterface * loadRadioSettingsFromByteArray(GeneralSettings & settings, const QByteArray & data);

bool writeModelToByteArray(const ModelData & model, QByteArray & data);
bool writeRadioSettingsToByteArray(const GeneralSettings & settings, QByteArray & data);

#endif // _OPENTXINTERFACE_H_
