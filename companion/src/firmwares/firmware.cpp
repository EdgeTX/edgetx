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

#include "firmware.h"
#include "firmwarefactories.h"
#include "appdata.h"

// static
QList<const char *> Firmware::m_languages = {
  "cn",
  "cz",
  "da",
  "de",
  "en",
  "es",
  "fi",
  "fr",
  "he",
  "hu",
  "it",
  "jp",
  "ko",
  "nl",
  "pl",
  "pt",
  "ru",
  "se",
  "sk",
  "tw",
  "ua"
};

// static
const Firmware::OptionTooltip Firmware::registeredOptions = {
  { "opt1", QT_TRANSLATE_NOOP("Firmware", "This is option 1") },
  { "opt2", QT_TRANSLATE_NOOP("Firmware", "This is option 2") },
  { "opt3", QT_TRANSLATE_NOOP("Firmware", "This is option 3") },
  { "opt4", QT_TRANSLATE_NOOP("Firmware", "This is option 4") },
  { "opt5", QT_TRANSLATE_NOOP("Firmware", "This is option 5") }
};

// depreciated see note in .h
QString Firmware::getLanguage() const {
  QStringList strl = getId().split('-');
  return strl.size() > 2 ? strl.last() : QString();
  // TODO replace above lines with
  // return g.currentProfile.fwLang();
}

// depreciated v3.0
QList<Firmware*> Firmware::getRegisteredFirmwares()
{
  return gFirmwareFactories->getRegisteredFirmwares();
}

Firmware::Firmware(const QString & id, const QString & path, const bool isSupported) :
  m_id(id),
  m_path(path),
  m_supported(isSupported),
  m_loaded(false),
  m_valid(true)
{
  m_defn = FirmwareDefn();

  QJsonDocument *doc = new QJsonDocument();

  if (load(doc, path)) {
    QJsonObject obj = doc->object();
    // ignore intermediate definitions
    if (!getValue(obj, "hidden", false).toBool()) {
      m_defn.id = getValueString(obj, "id", "unknown");
      m_defn.name = getValueString(obj, "name", "unknown");

      if (m_defn.id == "unknown") {
        m_valid = false;
        qCritical() << "Error - file:" << path << "does not contain an id";
      }
    } else {
      qDebug() << "ignoring" << path;
    }
  }

  delete doc;
}

int Firmware::getCapability(Capability value) const
{
  // TODO data moved to a new field in Preferences refactor
  // this contains edgetx-<firmware id>[-option[-option]...]-<language>
  QStringList opts = g.currentProfile().fwType().split("-");

  // TODO until Boards refactor
  Board::Type board = Boards::getBoardForHwDefn(m_defn.hwdefn);

  switch (value) {
    case Capability::ChannelsName:
      return m_defn.outputs.nameLen;
    case Capability::CustomFunctions:
      return m_defn.customFuncs;
    case Capability::DangerousFunctions:
      return opts.contains("danger") ? true : false;
    case Capability::ExtendedTrimsRange:
      return m_defn.extTrimsRange;
    case Capability::Modes:
      return m_defn.modes.cnt;
    case Capability::ModesName:
      return m_defn.modes.nameLen;
    case Capability::FlightModes:
      return getCapability(Capability::Modes);
    case Capability::FlightModesName:
      return getCapability(Capability::ModesName);
    case Capability::GlobalFunctions:
      return m_defn.globalFuncs;
    case Capability::Gvars:
      return opts.contains("nogvars") ? 0 : m_defn.gvars.cnt;
    case Capability::GvarsName:
      return m_defn.gvars.nameLen;
    case Capability::HasExpoNames:
      return getCapability(Capability::InputsName);
    case Capability::FailsafeChannels:
      return m_defn.failsafeChans;
    case Capability::HasFailsafe:
      return m_defn.failsafeChans;
    case Capability::HasFlySkyGimbals:
      return opts.contains("flyskygimbals");  // || Boards::getCapability(board, Boards::HasFlyskyGimbals); TODO Need a solution
    case Capability::HasMixerNames:
      return m_defn.mixes.nameLen;
    case Capability::HasModelImage:
      return m_defn.modelImage.image;
    case Capability::HasModelLabels:
      return m_defn.labels;
    case Capability::HasModelsList:
      return m_defn.modelsList;
    case Capability::HasVario:
      return Boards::isAir(board);
    case Capability::HasVarioSink:
      return Boards::isAir(board);
    case Capability::Heli:
      return !(opts.contains("noheli") || Boards::getCapability(board, Board::Surface));
    case Capability::Inputs:
      return m_defn.inputs.cnt;
    case Capability::InputsName:
      return m_defn.inputs.nameLen;
    case Capability::InputsLength:
      return getCapability(Capability::InputsName);
    case Capability::KeyShortcuts:
      return m_defn.keyShortcuts;
    case Capability::LogicalSwitches:
      return m_defn.logicalSW.cnt;
    case Capability::LuaInputsPerScript:
      return m_defn.luaScripts.inputs;
    case Capability::LuaOutputsPerScript:
      return m_defn.luaScripts.outputs;
    case Capability::LuaScripts:
      return opts.contains("lua") ? m_defn.luaScripts.cnt : 0;
    case Capability::Mixes:
      return m_defn.mixes.cnt;
    case Capability::ModelImageKeepExtn:
      return m_defn.modelImage.keepExtn;
    case Capability::ModelImageNameLen:
      return m_defn.modelImage.nameLen;
    case Capability::ModelName:
      return m_defn.modelNameLen;
    case Capability::Models:
      return m_defn.modelSlots;
    case Capability::NumCurvePoints:
      return m_defn.curves.points;
    case Capability::NumCurves:
      return m_defn.curves.cnt;
    case Capability::OffsetWeight:
      return m_defn.offsetWeight;
    case Capability::Outputs:
      return m_defn.outputs.cnt;
    case Capability::PPMCenter:
      return m_defn.outputs.ppmCenter;
    case Capability::PPMFrameLength:
      return m_defn.outputs.ppmFrameLen;
    case Capability::QMFavourites:
      return m_defn.quickMenuFavs;
    case Capability::SafetyChannelCustomFunction:
      return opts.contains("nooverridech") ? 0 : 1;
    case Capability::Sensors:
      return m_defn.sensors.cnt;
    case Capability::SlowRange:
      return m_defn.slowRange;
    case Capability::SlowScale:
      return m_defn.slowScale;
    case Capability::TelemetryCustomScreens:
      return m_defn.teleCstmScrns.cnt;
    case Capability::TelemetryCustomScreensBars:
      return m_defn.teleCstmScrns.bars;
    case Capability::TelemetryCustomScreensFieldsPerLine:
      return m_defn.teleCstmScrns.perLine;
    case Capability::TelemetryCustomScreensLines:
      return m_defn.teleCstmScrns.lines;
    case Capability::Timers:
      return m_defn.timers.cnt;
    case Capability::TimersName:
      return m_defn.timers.nameLen;
    case Capability::TopBarZones:
      return m_defn.topBarZones;
    case Capability::TrainerInputs:
      return m_defn.trainerInputs;
    case Capability::TrimsRange:
      return m_defn.trimsRange;
    case Capability::VoicesMaxLength:
      return m_defn.voicesFileLen;

    // depreciated v3.0 so call replacement Capability

    case Capability::VirtualInputs:
      return getCapability(Capability::Inputs);

    default:
      return 0;
  }
}

QString Firmware::getCapabilityStr(Capability value) const
{
  switch (value) {
    case Capability::ModelImageFilters:
      return m_defn.modelImage.filters;
    default:
      return QString();
  }
}

Firmware * Firmware::getFirmwareForId(const QString & id)
{
  QStringList parts = id.split("-");
  return gFirmwareFactories->firmware(parts.at(0) % "-" % parts.at(1));
}

bool Firmware::isOptionDuplicate(const OptionsGroup & grp, const QString & val)
{
  // qDebug() << val;

  for (OptionsGroup::const_iterator itg = grp.cbegin(); itg != grp.cend(); ++itg) {
    if (val == itg->name)
      return true;
  }

  return false;
}

bool Firmware::isOptionDuplicate(const OptionsList & options, const QString & val)
{
  // qDebug() << val;

  for (OptionsList::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
    for (OptionsGroup::const_iterator itg = it->cbegin(); itg != it->cend(); ++itg) {
      if (val == itg->name)
        return true;
    }
  }

  return false;
}

bool Firmware::loadDefinition()
{
  if (m_loaded)
    return true;

  // load default.json first and allow subsequent file values to override
  // this avoids having to include default in basedOn tree
  if (loadDefinition(QString("%1/%2.json").arg(FWDEFNSDIR).arg("default"))) {
    if (loadDefinition(m_path)) {
      m_loaded = true;
      qDebug() << "Definition loaded:" << m_id;
      return true;
    }
  }

  return false;
}

bool Firmware::loadDefinition(const QString & path)
{
  bool success = true;
  QJsonDocument *doc = new QJsonDocument();
  QJsonObject o;
  QStringList depends;

  if (load(doc, path)) {
    if (doc->isObject()) {
      o = doc->object();

      if (isArray(o,"basedOn")) {
        QJsonArray a = o.value("basedOn").toArray();

        for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
          if ((*it).isString()) {
            QString p = QString("%1/%2.json").arg(FWDEFNSDIR).arg((*it).toString());

            if (!depends.contains(p)) {
              depends.append(p);

              if (!loadDefinition(p))
                success = false;
            } else {
              qCritical() << "ERROR: circular dependency chain detected";
              success = false;
            }
          }
        }
      }
    }
  } else {
    success = false;
  }

  if (!success) {
    qCritical() << "CRITICAL: Load definition" << path << "unsuccessful";
    delete doc;
    return false;
  }

  //qDebug() << "loading values from:" << m_path;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    //qDebug() << "key:" << it.key() << "value:" << it.value();

    if (it.key() == "hidden" || it.key() == "basedOn")
      continue;

    if (it.key() == "id")
      m_defn.id = getValueString(o, it.key());
    else if (it.key() == "name")
      m_defn.name = getValueString(o, it.key());
    else if (it.key() == "board")
      m_defn.bddefn = getValueString(o, it.key());
    else if (it.key() == "dwnldId")
      m_defn.dwnldId = getValueString(o, it.key());
    else if (it.key() == "simulatorId")
      m_defn.simuId = getValueString(o, it.key());
    else if (it.key() == "hwm_defnId")
      m_defn.hwdefn = getValueString(o, it.key());
    else if (it.key() == "categories")
      m_defn.categories = getValueBool(o, it.key(), m_defn.categories);
    else if (it.key() == "gvars")
      loadGroup(it, m_defn.gvars, CPN_MAX_GVARS, 3);
    else if (it.key() == "inputs")
      loadGroup(it, m_defn.inputs, CPN_MAX_INPUTS, 3);
    else if (it.key() == "keyShortcuts")
      m_defn.keyShortcuts = getValueInt(o, it.key());
    else if (it.key() == "logicalSW")
      loadGroup(it, m_defn.logicalSW, CPN_MAX_LOGICAL_SWITCHES, 3);
    else if (it.key() == "luaScripts")
      loadLuaScripts(it);
    else if (it.key() == "modelImage")
      loadModelImage(it);
    else if (it.key() == "modelNameLen")
      m_defn.modelNameLen = getValueInt(o, it.key());
    else if (it.key() == "modelSlots")
      m_defn.modelSlots = getValueInt(o, it.key());
    else if (it.key() == "modes")
      loadGroup(it, m_defn.modes, CPN_MAX_FLIGHT_MODES, 3);
    else if (it.key() == "mixes")
      loadGroup(it, m_defn.mixes, CPN_MAX_MIXERS, 3);
    else if (it.key() == "outputs")
      loadOutputs(it);
    else if (it.key() == "quickMenuFavs")
      m_defn.quickMenuFavs = getValueInt(o, it.key());
    else if (it.key() == "sensors")
      loadGroup(it, m_defn.sensors, CPN_MAX_SENSORS, 3);
    else if (it.key() == "timers")
      loadGroup(it, m_defn.timers, CPN_MAX_TIMERS, 3);
    else if (isArray(o, "options"))
      loadOptions(it);
    else
      qWarning() << "Warning: No rule to process - path:" << path << "name:" << it.key() << "value:" << it.value();
  }

  delete doc;
  return postLoad();
}

void Firmware::loadCurves(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    Curves & crv = m_defn.curves;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        crv.cnt = getValueInt(o, it.key(), crv.cnt, CPN_MAX_CURVES);
      else if (it.key() == "nameLen")
        crv.nameLen = getValueInt(o, it.key(), crv.nameLen, 5);
      else if (it.key() == "points")
        crv.points = getValueInt(o, it.key(), crv.points, 512);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: curves is not an object";
}

void Firmware::loadGroup(QJsonObject::const_iterator & grpit, BaseGrp & grp,
                           const int cntMax, const int nameLenMax,
                           const int cntMin, const int nameLenMin)
{
  if (grpit->isObject()) {
    QJsonObject o = grpit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        grp.cnt = getValueInt(o, it.key(), grp.cnt, cntMax, cntMin);
      else if (it.key() == "nameLen")
        grp.nameLen = getValueInt(o, it.key(), grp.nameLen, nameLenMax, nameLenMin);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: option group is not an object";
}

void Firmware::loadLuaScripts(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    LuaScripts & lua = m_defn.luaScripts;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        lua.cnt = getValueInt(o, it.key(), lua.cnt, CPN_MAX_SCRIPTS);
      else if (it.key() == "inputs")
        lua.inputs = getValueInt(o, it.key(), lua.inputs, CPN_MAX_SCRIPT_INPUTS);
      else if (it.key() == "outputs")
        lua.outputs = getValueInt(o, it.key(), lua.outputs, CPN_MAX_SCRIPT_OUTPUTS);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: luaScripts is not an object";
}

void Firmware::loadModelImage(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    ModelImage & mi = m_defn.modelImage;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "filters")
        mi.filters = getValueString(o, it.key(), mi.filters);
      else if (it.key() == "image")
        mi.image = getValueBool(o, it.key(), mi.image);
      else if (it.key() == "keepExtn")
        mi.keepExtn = getValueBool(o, it.key(), mi.keepExtn);
      else if (it.key() == "nameLen")
        mi.nameLen = getValueInt(o, it.key(), mi.nameLen, 14);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: modelImage is not an object";
}

void Firmware::loadOptions(QJsonObject::const_iterator & it)
{
  if (it->isArray()) {
    QJsonArray arrOptions = it->toArray();

    for (QJsonArray::const_iterator itOptions = arrOptions.constBegin(); itOptions != arrOptions.constEnd(); ++itOptions) {
      //qDebug() << "value:" << (*itOptions);
      OptionsGroup grp;

      if ((*itOptions).isArray()) {
        QJsonArray arrOptGrp = itOptions->toArray();

        for (QJsonArray::const_iterator itOptGrp = arrOptGrp.constBegin(); itOptGrp != arrOptGrp.constEnd(); ++itOptGrp) {
          if (!isOptionDuplicate(grp,(*itOptGrp).toString()))
            loadOptionGroup(itOptGrp, grp);
          else
            qWarning() << "Warning: duplicate option:" << *itOptGrp;
        }
      } else
        loadOptionGroup(itOptions, grp);

      if (grp.count())
        m_defn.options.append(grp);
    }
  } else
    qWarning() << "Warning: options is not an array";
}

void Firmware::loadOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp)
{
  if ((*it).isString()) {
    QString opt((*it).toString());

    if (registeredOptions.contains((*it).toString())) {
      if (!isOptionDuplicate(m_defn.options, opt)) {
        // TODO do not store a duplicate copy of the tooltip
        grp.append( { Option(opt,
                      registeredOptions.value(opt)) });
      } else {
        qWarning() << "Duplicate option:" << opt;
      }
    } else {
      qWarning() << "Invalid option:" << opt;
    }
  } else
    qWarning() << "option is not a string:" << (*it);
}

void Firmware::loadOutputs(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    Outputs & out = m_defn.outputs;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        out.cnt = getValueInt(o, it.key(), out.cnt, CPN_MAX_CHNOUT);
      else if (it.key() == "nameLen")
        out.nameLen = getValueInt(o, it.key(), out.nameLen, 6);
      else if (it.key() == "ppmCenter")
        out.ppmCenter = getValueInt(o, it.key(), out.ppmCenter, 512);
      else if (it.key() == "ppmFrameLen")
        out.ppmFrameLen = getValueInt(o, it.key(), out.ppmFrameLen, 40);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: outputs is not an object";
}

void Firmware::loadTeleCstmScrns(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    TeleCstmScrns & tele = m_defn.teleCstmScrns;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        tele.cnt = getValueInt(o, it.key(), tele.cnt, 3);
      else if (it.key() == "bars")
        tele.bars = getValueInt(o, it.key(), tele.bars, 4);
      else if (it.key() == "perLine")
        tele.perLine = getValueInt(o, it.key(), tele.perLine, 3);
      else if (it.key() == "lines")
        tele.lines = getValueInt(o, it.key(), tele.lines, 4);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: teleCstmScrns is not an object";
}

bool Firmware::postLoad()
{
  // set defaults here to avoid business rules elsewhere
  if (m_defn.bddefn.isEmpty())
    m_defn.bddefn = m_defn.id;

  if (m_defn.hwdefn.isEmpty())
    m_defn.hwdefn = m_defn.id;

  if (m_defn.dwnldId.isEmpty())
    m_defn.dwnldId = m_defn.id;

  if (m_defn.simuId.isEmpty())
    m_defn.simuId = m_defn.id;

  return true;
}

void Firmware::setCurrent(const QString & id)
{
  setCurrent(gFirmwareFactories->firmware(id));
}

void Firmware::setCurrent(Firmware * firmware)
{
  bool result = false;

  if (firmware && firmware->loadDefinition()) {
    result = true;
    m_current = firmware;
    // TODO as part of Boards refactor
    // force the linked board to load if not already
    //m_current->board()->loadDefinition;
  }

  if (!result)
    qCritical() << "ERROR - Set current firmware to instance:" << (firmware ? firmware->id() : "unknown");

}

