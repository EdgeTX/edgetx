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

#include "er9xinterface.h"
#include "er9xeeprom.h"
#include "rlefile.h"
// #include "appdata.h"
// #include <iostream>
#include <bitset>

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

#define FILE_GENERAL   0
#define FILE_MODEL(n) (1+n)

Er9xInterface::Er9xInterface():
  EEPROMInterface(Board::BOARD_9X_M64),
  efile(new RleFile())
{
}

Er9xInterface::~Er9xInterface()
{
  delete efile;
}

const char * Er9xInterface::getName()
{
  return "Er9x";
}

inline void applyStickModeToModel(Er9xModelData & model, unsigned int mode)
{
  for (int i=0; i<2; i++) {
    int stick = applyStickMode(i+1, mode) - 1;
    {
      int tmp = model.trim[i];
      model.trim[i] = model.trim[stick];
      model.trim[stick] = tmp;
    }
    {
      Er9xExpoData tmp = model.expoData[i];
      model.expoData[i] = model.expoData[stick];
      model.expoData[stick] = tmp;
    }
  }
  for (int i=0; i<ER9X_MAX_MIXERS; i++)
    model.mixData[i].srcRaw = applyStickMode(model.mixData[i].srcRaw, mode);
  for (int i=0; i<ER9X_NUM_CSW; i++) {
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
unsigned long Er9xInterface::loadxml(RadioData &radioData, QDomDocument &doc)
{
  QDebug dbg = qDebug();
  dbg.setAutoInsertSpaces(false);
  dbg << "trying er9x xml import... ";

  std::bitset<NUM_ERRORS> errors;

  Er9xGeneral er9xGeneral;
  memset(&er9xGeneral,0,sizeof(er9xGeneral));
  if (!loadRadioSettingsDataXML(&doc, &er9xGeneral)) {
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();
  }
  else {
    radioData.generalSettings=er9xGeneral;
    dbg << "version " << (unsigned int)er9xGeneral.myVers << " ";
  }
  for (int i=0; i<getCapability(Models); i++) {
    Er9xModelData er9xModel;
    memset(&er9xModel,0,sizeof(er9xModel));
    if(loadModelDataXML(&doc, &er9xModel, i)) {
      applyStickModeToModel(er9xModel, radioData.generalSettings.stickMode+1);
      radioData.models[i] = er9xModel;
    }
  }
  dbg << "ok";
  errors.set(ALL_OK);
  return errors.to_ulong();
}
#endif

unsigned long Er9xInterface::load(RadioData &radioData, const uint8_t *eeprom, int size)
{
  QDebug dbg = qDebug();
  dbg.setAutoInsertSpaces(false);
  dbg << "trying er9x import... ";

  std::bitset<NUM_ERRORS> errors;

  if (size != Boards::getEEpromSize(Board::BOARD_9X_M64)) {
    dbg << "wrong size";
    errors.set(WRONG_SIZE);
    return errors.to_ulong();
  }

  if (!efile->EeFsOpen((uint8_t *)eeprom, size, Board::BOARD_9X_M64)) {
    dbg << "wrong file system";
    errors.set(WRONG_FILE_SYSTEM);
    return errors.to_ulong();
  }

  efile->openRd(FILE_GENERAL);
  Er9xGeneral er9xGeneral;

  if (efile->readRlc1((uint8_t*)&er9xGeneral, 1) != 1) {
    dbg << "no";
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();
  }

  dbg << "version " << (unsigned int)er9xGeneral.myVers << " ";

  switch(er9xGeneral.myVers) {
    case 3:
      dbg << "(old gruvin9x) ";
    case 4:
//    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      break;
    default:
      dbg << "not er9x";
      errors.set(NOT_ER9X);
      return errors.to_ulong();
  }

  efile->openRd(FILE_GENERAL);
  if (!efile->readRlc1((uint8_t*)&er9xGeneral, sizeof(Er9xGeneral))) {
    dbg << "ko";
    errors.set(UNKNOWN_ERROR);
    return errors.to_ulong();

  }
  radioData.generalSettings = er9xGeneral;

  for (int i=0; i<getCapability(Models); i++) {
    Er9xModelData er9xModel;
    efile->openRd(FILE_MODEL(i));
    if (!efile->readRlc1((uint8_t*)&er9xModel, sizeof(Er9xModelData))) {
      radioData.models[i].clear();
    }
    else {
      applyStickModeToModel(er9xModel, radioData.generalSettings.stickMode+1);
      radioData.models[i] = er9xModel;
    }
  }

  dbg << "ok";
  errors.set(ALL_OK);
  return errors.to_ulong();
}

unsigned long Er9xInterface::loadBackup(RadioData &radioData, const uint8_t *eeprom, int esize, int index)
{
  std::bitset<NUM_ERRORS> errors;
  errors.set(UNKNOWN_ERROR);
  return errors.to_ulong();
}

int Er9xInterface::getSize(const ModelData &model)
{
  return 0;
}

int Er9xInterface::getSize(const GeneralSettings &settings)
{
  return 0;
}

bool Er9xInterface::isAvailable(PulsesProtocol prot, int port)
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

void Er9xInterface::appendTextElement(QDomDocument * qdoc, QDomElement * pe, QString name, QString value)
{
  QDomElement e = qdoc->createElement(name);
  QDomText t = qdoc->createTextNode(name);
  t.setNodeValue(value);
  e.appendChild(t);
  pe->appendChild(e);
}

void Er9xInterface::appendNumberElement(QDomDocument * qdoc, QDomElement * pe,QString name, int value, bool forceZeroWrite)
{
  if(value || forceZeroWrite) {
    QDomElement e = qdoc->createElement(name);
    QDomText t = qdoc->createTextNode(name);
    t.setNodeValue(QString("%1").arg(value));
    e.appendChild(t);
    pe->appendChild(e);
  }
}

void Er9xInterface::appendCDATAElement(QDomDocument * qdoc, QDomElement * pe,QString name, const char * data, int size)
{
  QDomElement e = qdoc->createElement(name);
  QDomCDATASection t = qdoc->createCDATASection(name);
  t.setData(QByteArray(data, size).toBase64());
  e.appendChild(t);
  pe->appendChild(e);
}

QDomElement Er9xInterface::getGeneralDataXML(QDomDocument * qdoc, Er9xGeneral * tgen)
{
  QDomElement gd = qdoc->createElement("GENERAL_DATA");
  appendNumberElement(qdoc, &gd, "Version", tgen->myVers, true); // have to write value here
  appendTextElement(qdoc, &gd, "Owner", QString::fromLatin1(tgen->ownerName,sizeof(tgen->ownerName)).trimmed());
  appendCDATAElement(qdoc, &gd, "Data", (const char *)tgen,sizeof(Er9xGeneral));
  return gd;
}

QDomElement Er9xInterface::getModelDataXML(QDomDocument * qdoc, Er9xModelData * tmod, int modelNum, int mdver)
{
  QDomElement md = qdoc->createElement("MODEL_DATA");
  md.setAttribute("number", modelNum);
  appendNumberElement(qdoc, &md, "Version", mdver, true); // have to write value here
  appendTextElement(qdoc, &md, "Name", QString::fromLatin1(tmod->name,sizeof(tmod->name)).trimmed());
  appendCDATAElement(qdoc, &md, "Data", (const char *)tmod,sizeof(Er9xModelData));
  return md;
}

bool Er9xInterface::loadRadioSettingsDataXML(QDomDocument * qdoc, Er9xGeneral * tgen)
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
      memcpy(tgen, data, sizeof(Er9xGeneral));
      break;
    }
    n = n.nextSibling();
  }
  // check version?
  return true;
}

bool Er9xInterface::loadModelDataXML(QDomDocument * qdoc, Er9xModelData * tmod, int modelNum)
{
  // look for MODEL_DATA with modelNum attribute.
  // if modelNum = -1 then just pick the first one
  QDomNodeList ndl = qdoc->elementsByTagName("MODEL_DATA");

  // cycle through nodes to find correct model number
  QDomNode k = ndl.at(0);
  if(modelNum>=0) {
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
      memcpy(tmod, data, sizeof(Er9xModelData));
      break;
    }
    n = n.nextSibling();
  }
  // check version?
  return true;
}

int Er9xInterface::getCapability(Capability capability)
{
  switch (capability) {
    case Models:
      return 16;
    default:
      return 0;
  }
}

QString Er9xInterface::getCapabilityStr(Capability capability)
{
  return QString();
}
