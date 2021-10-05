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

#include "ersky9xinterface.h"
#include "ersky9xeeprom.h"
#include "rlefile.h"
#include <iostream>
#include <bitset>

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
#define FILE_MODEL(n) (1+n)
#define FILE_TMP      (1+16)

Ersky9xInterface::Ersky9xInterface():
EEPROMInterface(Board::BOARD_SKY9X),
efile(new RleFile())
{
}

Ersky9xInterface::~Ersky9xInterface()
{
  delete efile;
}

const char * Ersky9xInterface::getName()
{
  return "Ersky9x";
}

inline void applyStickModeToModel(Ersky9xModelData_v10 & model, unsigned int mode)
{
  for (int i=0; i<2; i++) {
    int stick = applyStickMode(i+1, mode) - 1;
    {
      int tmp = model.trim[i];
      model.trim[i] = model.trim[stick];
      model.trim[stick] = tmp;
    }
    {
      Ersky9xExpoData tmp = model.expoData[i];
      model.expoData[i] = model.expoData[stick];
      model.expoData[stick] = tmp;
    }
  }
  for (int i=0; i<ERSKY9X_MAX_MIXERS_V10; i++)
    model.mixData[i].srcRaw = applyStickMode(model.mixData[i].srcRaw, mode);
  for (int i=0; i<ERSKY9X_NUM_CSW_V10; i++) {
    switch (LogicalSwitchData(model.logicalSw[i].func).getFunctionFamily()) {
      case LS_FAMILY_VCOMP:
        model.logicalSw[i].v2 = applyStickMode(model.logicalSw[i].v2, mode);
        // no break
      case LS_FAMILY_VOFS:
        model.logicalSw[i].v1 = applyStickMode(model.logicalSw[i].v1, mode);
        break;
      default:
        break;
    }
  }
  model.swashCollectiveSource = applyStickMode(model.swashCollectiveSource, mode);
}

inline void applyStickModeToModel(Ersky9xModelData_v11 & model, unsigned int mode)
{
  for (int i=0; i<2; i++) {
    int stick = applyStickMode(i+1, mode) - 1;
    {
      int tmp = model.trim[i];
      model.trim[i] = model.trim[stick];
      model.trim[stick] = tmp;
    }
    {
      Ersky9xExpoData tmp = model.expoData[i];
      model.expoData[i] = model.expoData[stick];
      model.expoData[stick] = tmp;
    }
  }
  for (int i=0; i<ERSKY9X_MAX_MIXERS_V11; i++)
    model.mixData[i].srcRaw = applyStickMode(model.mixData[i].srcRaw, mode);
  for (int i=0; i<ERSKY9X_NUM_CSW_V11; i++) {
    switch (LogicalSwitchData(model.logicalSw[i].func).getFunctionFamily()) {
      case LS_FAMILY_VCOMP:
        model.logicalSw[i].v2 = applyStickMode(model.logicalSw[i].v2, mode);
        // no break
      case LS_FAMILY_VOFS:
        model.logicalSw[i].v1 = applyStickMode(model.logicalSw[i].v1, mode);
        break;
      default:
        break;
    }
  }
  model.swashCollectiveSource = applyStickMode(model.swashCollectiveSource, mode);
}

#if 0
unsigned long Ersky9xInterface::loadxml(RadioData &radioData, QDomDocument &doc)
{
  QDebug dbg = qDebug();
  dbg.setAutoInsertSpaces(false);
  dbg << "trying ersky9x xml import... ";

  std::bitset<NUM_ERRORS> errors;

  Ersky9xGeneral ersky9xGeneral;
  memset(&ersky9xGeneral,0,sizeof(ersky9xGeneral));
  if(!loadRadioSettingsDataXML(&doc, &ersky9xGeneral)) {
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();
  }
  else {
    radioData.generalSettings=ersky9xGeneral;
    dbg << "version " << (unsigned int)ersky9xGeneral.myVers << " ";
  }
  for(int i=0; i<getCapability(Models); i++) {
    if (ersky9xGeneral.myVers == 10) {
      if (!loadModelDataXML<Ersky9xModelData_v10>(&doc, &radioData.models[i], i, radioData.generalSettings.stickMode+1)) {
        dbg << "ko";
        errors.set(UNKNOWN_ERROR);
        return errors.to_ulong();
      }
    }
    else {
      if (!loadModelDataXML<Ersky9xModelData_v11>(&doc, &radioData.models[i], i, radioData.generalSettings.stickMode+1)) {
        dbg << "ko";
        errors.set(UNKNOWN_ERROR);
        return errors.to_ulong();
      }
    }
  }
  dbg << "ok";
  errors.set(ALL_OK);
  return errors.to_ulong();
}
#endif

unsigned long Ersky9xInterface::load(RadioData &radioData, const uint8_t *eeprom, int size)
{
  QDebug dbg = qDebug();
  dbg.setAutoInsertSpaces(false);
  dbg << "trying ersky9x import... ";

  std::bitset<NUM_ERRORS> errors;

  if (size != Boards::getEEpromSize(Board::BOARD_SKY9X)) {
    dbg << "wrong size";
    errors.set(WRONG_SIZE);
    return errors.to_ulong();
  }

  if (!efile->EeFsOpen((uint8_t *)eeprom, size, Board::BOARD_SKY9X)) {
    dbg << "wrong file system";
    errors.set(WRONG_FILE_SYSTEM);
    return errors.to_ulong();
  }

  efile->openRd(FILE_GENERAL);
  Ersky9xGeneral ersky9xGeneral;

  if (efile->readRlc2((uint8_t*)&ersky9xGeneral, 1) != 1) {
    dbg << "no";
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();
  }

  dbg << "version " << (unsigned int)ersky9xGeneral.myVers << " ";

  switch(ersky9xGeneral.myVers) {
    case 10:
      break;
    case 11:
      break;
    default:
      dbg << "not ersky9x";
      errors.set(NOT_ERSKY9X);
      return errors.to_ulong();
  }
  efile->openRd(FILE_GENERAL);
  if (!efile->readRlc2((uint8_t*)&ersky9xGeneral, sizeof(Ersky9xGeneral))) {
    dbg << "ko";
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();
  }
  radioData.generalSettings = ersky9xGeneral;

  for (int i=0; i<getCapability(Models); i++) {
    uint8_t buffer[4096];
    uint size;
    memset(buffer,0,sizeof(buffer));
    efile->openRd(FILE_MODEL(i));

//    if (!efile->readRlc2((uint8_t*)&ersky9xModel, sizeof(Ersky9xModelData))) {
    size = efile->readRlc2(buffer, 4096);
    if (!size) {
      radioData.models[i].clear();
    }
    else {
      if (size<720) {
        Ersky9xModelData_v10 ersky9xModel;
        memcpy(&ersky9xModel, buffer, sizeof(ersky9xModel));
        applyStickModeToModel(ersky9xModel, radioData.generalSettings.stickMode+1);
        radioData.models[i] = ersky9xModel;
      } else {
        Ersky9xModelData_v11 ersky9xModel;
        memcpy(&ersky9xModel, buffer, sizeof(ersky9xModel));
        applyStickModeToModel(ersky9xModel, radioData.generalSettings.stickMode+1);
        radioData.models[i] = ersky9xModel;
      }
    }
  }

  dbg << "ok";
  errors.set(ALL_OK);
  return errors.to_ulong();
}

unsigned long Ersky9xInterface::loadBackup(RadioData &radioData, const uint8_t *eeprom, int esize, int index)
{
  std::bitset<NUM_ERRORS> errors;
  errors.set(UNKNOWN_ERROR);
  return errors.to_ulong();
}

int Ersky9xInterface::getSize(const ModelData & model)
{
  return 0;
}

int Ersky9xInterface::getSize(const GeneralSettings & settings)
{
  return 0;
}

bool Ersky9xInterface::isAvailable(PulsesProtocol prot, int port)
{
  switch (prot) {
    case PULSES_PPM:
    case PULSES_DSM2:
    case PULSES_PXX_DJT:
    case PULSES_PPM16:
      return 1;
    default:
      return 0;
  }
}

void Ersky9xInterface::appendTextElement(QDomDocument * qdoc, QDomElement * pe, QString name, QString value)
{
    QDomElement e = qdoc->createElement(name);
    QDomText t = qdoc->createTextNode(name);
    t.setNodeValue(value);
    e.appendChild(t);
    pe->appendChild(e);
}

void Ersky9xInterface::appendNumberElement(QDomDocument * qdoc, QDomElement * pe,QString name, int value, bool forceZeroWrite)
{
  if(value || forceZeroWrite) {
    QDomElement e = qdoc->createElement(name);
    QDomText t = qdoc->createTextNode(name);
    t.setNodeValue(QString("%1").arg(value));
    e.appendChild(t);
    pe->appendChild(e);
  }
}

void Ersky9xInterface::appendCDATAElement(QDomDocument * qdoc, QDomElement * pe,QString name, const char * data, int size)
{
  QDomElement e = qdoc->createElement(name);
  QDomCDATASection t = qdoc->createCDATASection(name);
  t.setData(QByteArray(data, size).toBase64());
  e.appendChild(t);
  pe->appendChild(e);
}

QDomElement Ersky9xInterface::getGeneralDataXML(QDomDocument * qdoc, Ersky9xGeneral * tgen)
{
  QDomElement gd = qdoc->createElement("GENERAL_DATA");
  appendNumberElement(qdoc, &gd, "Version", tgen->myVers, true); // have to write value here
  appendTextElement(qdoc, &gd, "Owner", QString::fromLatin1(tgen->ownerName,sizeof(tgen->ownerName)).trimmed());
  appendCDATAElement(qdoc, &gd, "Data", (const char *)tgen,sizeof(Ersky9xGeneral));
  return gd;
}

QDomElement Ersky9xInterface::getModelDataXML(QDomDocument * qdoc, Ersky9xModelData_v11 * tmod, int modelNum, int mdver)
{
  QDomElement md = qdoc->createElement("MODEL_DATA");
  md.setAttribute("number", modelNum);
  appendNumberElement(qdoc, &md, "Version", mdver, true); // have to write value here
  appendTextElement(qdoc, &md, "Name", QString::fromLatin1(tmod->name,sizeof(tmod->name)).trimmed());
  appendCDATAElement(qdoc, &md, "Data", (const char *)tmod,sizeof(Ersky9xModelData_v11));
  return md;
}

bool Ersky9xInterface::loadRadioSettingsDataXML(QDomDocument * qdoc, Ersky9xGeneral * tgen)
{
  // look for "GENERAL_DATA" tag
  QDomElement gde = qdoc->elementsByTagName("GENERAL_DATA").at(0).toElement();

  if(gde.isNull()) // couldn't find
    return false;

  // load cdata into tgen
  QDomNode n = gde.elementsByTagName("Data").at(0).firstChild();// get all children in Data
  while (!n.isNull()) {
    if (n.isCDATASection()) {
      QString ds = n.toCDATASection().data();
      QByteArray ba = QByteArray::fromBase64(ds.toLatin1());
      const char * data = ba.data();
      memcpy(tgen, data, std::min((unsigned int)ba.size(), (unsigned int)sizeof(Ersky9xGeneral)));
      break;
    }
    n = n.nextSibling();
  }
  // check version?
  return true;
}

template <class T>
bool Ersky9xInterface::loadModelDataXML(QDomDocument * qdoc, ModelData *model, int modelNum, int stickMode)
{
  T ersky9xModel;
  memset(&ersky9xModel, 0, sizeof(ersky9xModel));

  // look for MODEL_DATA with modelNum attribute.
  //if modelNum = -1 then just pick the first one
  QDomNodeList ndl = qdoc->elementsByTagName("MODEL_DATA");

  // cycle through nodes to find correct model number
  QDomNode k = ndl.at(0);
  if (modelNum>=0) {
    while(!k.isNull()) {
      int a = k.toElement().attribute("number").toInt();
      if(a==modelNum)
        break;
      k = k.nextSibling();
    }
  }

  if (k.isNull()) // couldn't find
    return false;

  // load cdata into tgen
  QDomNode n = k.toElement().elementsByTagName("Data").at(0).firstChild();// get all children in Data
  while (!n.isNull()) {
    if (n.isCDATASection()) {
      QString ds = n.toCDATASection().data();
      QByteArray ba = QByteArray::fromBase64(ds.toLatin1());
      const char * data = ba.data();
      memcpy(&ersky9xModel, data, std::min(ba.size(), (int)sizeof(ersky9xModel)));
      break;
    }
    n = n.nextSibling();
  }

  applyStickModeToModel(ersky9xModel, stickMode);
  *model = ersky9xModel;
  return true;
}

int Ersky9xInterface::getCapability(Capability capability)
{
  switch (capability) {
    case Models:
      return 20;
    default:
      return 0;
  }
}

QString Ersky9xInterface::getCapabilityStr(Capability capability)
{
  return QString();
}
