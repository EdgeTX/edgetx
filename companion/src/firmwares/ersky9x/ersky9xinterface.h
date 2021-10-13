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

#ifndef _ERSKY9XINTERFACE_H_
#define _ERSKY9XINTERFACE_H_

#include <QtXml>
#include "eeprominterface.h"
#include "ersky9xeeprom.h"

class RleFile;

class Ersky9xInterface : public EEPROMInterface
{
  public:

    Ersky9xInterface();

    virtual ~Ersky9xInterface();

    virtual const char * getName();


    virtual unsigned long load(RadioData &, const uint8_t * eeprom, int size);

    virtual unsigned long loadBackup(RadioData &, const uint8_t * eeprom, int esize, int index);

    virtual int save(uint8_t * eeprom, const RadioData & radioData, uint8_t version=0, uint32_t variant=0)
    {
      return 0;
    }

    virtual int getSize(const ModelData &);

    virtual int getSize(const GeneralSettings & settings);

    virtual bool isAvailable(PulsesProtocol proto, int port=0);

    virtual int getCapability(Capability capability);

    virtual QString getCapabilityStr(Capability);

  protected:

    RleFile * efile;

  private:
    void appendTextElement(QDomDocument * qdoc, QDomElement * pe, QString name, QString value);

    void appendNumberElement(QDomDocument * qdoc, QDomElement * pe,QString name, int value, bool forceZeroWrite = false);

    void appendCDATAElement(QDomDocument * qdoc, QDomElement * pe,QString name, const char * data, int size);

    QDomElement getGeneralDataXML(QDomDocument * qdoc, Ersky9xGeneral * tgen);   //parse out data to XML format

    QDomElement getModelDataXML(QDomDocument * qdoc, Ersky9xModelData_v11 * tmod, int modelNum, int mdver); //parse out data to XML format

    bool loadRadioSettingsDataXML(QDomDocument * qdoc, Ersky9xGeneral * tgen); // get data from XML

    template <class T>
    bool loadModelDataXML(QDomDocument * qdoc, ModelData * model, int modelNum, int stickMode); // get data from XML

};

#endif // _ERSKY9XINTERFACE_H_
