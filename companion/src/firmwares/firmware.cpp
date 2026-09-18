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
#include "../boards/boardfactories.h"
#include "appdata.h"

// static
QList<QString> Firmware::m_languages = {
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
      m_defn.id = getValueStdString(obj, "id", "unknown");
      m_defn.name = getValueStdString(obj, "name", "unknown");
      m_defn.boardId = getValueStdString(obj, "boardId", "unknown");

      if (m_defn.id == "unknown") {
        m_valid = false;
        qCritical() << "Error - file:" << path << "does not contain an id";
      }

      if (m_defn.boardId == "unknown") {
        m_valid = false;
        qCritical() << "Error - file:" << path << "does not contain a boardId";
      } else {
        m_board = gBoardFactories->boardForId(m_defn.boardId.c_str());
      }
    } else {
      qDebug() << "ignoring" << path;
    }
  }

  delete doc;
}

const int Firmware::getCapability(Capability value) const
{
  QStringList opts = g.currentProfile().fwOptions().split("-");

  switch (value) {
    case Capability::ChannelsName:
      return m_defn.outputs.limits.max_name_len;

    case Capability::SpecialFunctions:
      return m_defn.max_specialFuncs;

    case Capability::DangerousFunctions:
      return opts.contains("danger") ? true : false;

    case Capability::ExtendedTrimsRange:
      return m_defn.extTrimsRange;

    case Capability::Modes:
      return m_defn.modes.max_count;

    case Capability::ModesName:
      return m_defn.modes.max_name_len;

    case Capability::FlightModes:
      return getCapability(Capability::Modes);

    case Capability::FlightModesName:
      return getCapability(Capability::ModesName);

    case Capability::GlobalFunctions:
      return m_defn.max_globalFuncs;

    case Capability::Gvars:
      return opts.contains("nogvars") ? 0 : m_defn.gvars.max_count;

    case Capability::GvarsName:
      return m_defn.gvars.max_name_len;

    case Capability::HasExpoNames:
      return getCapability(Capability::InputsName);

    case Capability::FailsafeChannels:
      return m_defn.failsafeChans;

    case Capability::HasFailsafe:
      return m_defn.failsafeChans;

    case Capability::HasFlySkyGimbals:
      return opts.contains("flyskygimbals") || m_board->getCapability(Capability::HasFlySkyGimbals);

    case Capability::HasMixerNames:
      return m_defn.mixes.max_name_len;

    case Capability::HasModelImage:
      return m_defn.modelImage.image;

    case Capability::HasModelLabels:
      return m_defn.labels;

    case Capability::HasModelsList:
      return m_defn.modelsList;

    case Capability::HasVario:
      return getCapability(Capability::Air);

    case Capability::HasVarioSink:
      return getCapability(Capability::Air);

    case Capability::Heli:
      return !(opts.contains("noheli") || getCapability(Capability::Surface));

    case Capability::Inputs:
      return m_defn.inputs.max_count;

    case Capability::InputsName:
      return m_defn.inputs.max_name_len;

    case Capability::InputsLength:
      return getCapability(Capability::InputsName);

    case Capability::KeyShortcuts:
      return m_defn.max_keyShortcuts;

    case Capability::LogicalSwitches:
      return m_defn.logicalSW.max_count;

    case Capability::LuaInputsPerScript:
      return m_defn.luaScripts.max_inputs;

    case Capability::LuaOutputsPerScript:
      return m_defn.luaScripts.max_outputs;

    case Capability::LuaScripts:
      return opts.contains("lua") ? m_defn.luaScripts.limits.max_count : 0;

    case Capability::Mixes:
      return m_defn.mixes.max_count;

    case Capability::ModelImageKeepExtn:
      return m_defn.modelImage.keepExtn;

    case Capability::ModelImageNameLen:
      return m_defn.modelImage.limits.max_name_len;

    case Capability::ModelName:
      return m_defn.max_modelName;

    case Capability::Models:
      return m_defn.max_modelSlots;

    case Capability::NumCurvePoints:
      return m_defn.curves.max_points;

    case Capability::NumCurves:
      return m_defn.curves.limits.max_count;

    case Capability::OffsetWeight:
      return m_defn.offsetWeight;

    case Capability::Outputs:
      return m_defn.outputs.limits.max_count;

    case Capability::PPMCenter:
      return m_defn.outputs.ppmCenter;

    case Capability::PPMFrameLength:
      return m_defn.outputs.ppmFrameLen;

    case Capability::QMFavourites:
      return m_defn.max_quickMenuFavs;

    case Capability::SafetyChannelCustomFunction:
      return opts.contains("nooverridech") ? 0 : 1;

    case Capability::Sensors:
      return m_defn.sensors.max_count;

    case Capability::SlowRange:
      return m_defn.slowRange;

    case Capability::SlowScale:
      return m_defn.slowScale;

    case Capability::TelemetryCustomScreens:
      return m_defn.teleCstmScrns.limits.max_count;

    case Capability::TelemetryCustomScreensBars:
      return m_defn.teleCstmScrns.max_bars;

    case Capability::TelemetryCustomScreensFieldsPerLine:
      return m_defn.teleCstmScrns.max_perLine;

    case Capability::TelemetryCustomScreensLines:
      return m_defn.teleCstmScrns.max_lines;

    case Capability::Timers:
      return m_defn.timers.max_count;

    case Capability::TimersName:
      return m_defn.timers.max_name_len;

    case Capability::TopBarZones:
      return m_defn.max_topBarZones;

    case Capability::TrainerInputs:
      return m_defn.max_trainerInputs;

    case Capability::TrimsRange:
      return m_defn.trimsRange;

    case Capability::VoicesMaxLength:
      return m_defn.max_voicesFileLen;

    // drop thru to Board
    default:
      return m_board->getCapability(value);
  }
}

const QString Firmware::getCapabilityStr(Capability value) const
{
  switch (value) {
    case Capability::ModelImageFilters:
      return m_defn.modelImage.filters.c_str();

    // drop thru to Board
    default:
      return m_board->getCapabilityStr(value);
  }
}

Firmware * Firmware::getFirmwareForId(const QString & id)
{
  QStringList parts = id.split("-");
  return gFirmwareFactories->getFirmware(parts.at(0) % "-" % parts.at(1));
}

QList<Firmware *> Firmware::getRegisteredFirmwares()
{
  return gFirmwareFactories->getRegisteredFirmwares();
}

bool Firmware::isOptionDuplicate(const OptionsGroup & grp, const QString & val)
{
  // qDebug() << val;
  return grp.contains(val);
}

bool Firmware::isOptionDuplicate(const OptionsList & options, const QString & val)
{
  // qDebug() << val;

  for (OptionsList::const_iterator it = options.cbegin(); it != options.cend(); ++it) {
    for (OptionsGroup::const_iterator itg = it->cbegin(); itg != it->cend(); ++itg) {
      if (itg->contains(val))
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
      m_defn.id = getValueStdString(it);

    else if (it.key() == "name")
      m_defn.name = getValueStdString(it);

    else if (it.key() == "board")
      m_defn.boardId = getValueStdString(it);

    else if (it.key() == "dwnldId")
      m_defn.dwnldId = getValueStdString(it);

    else if (it.key() == "simulatorId")
      m_defn.simuId = getValueStdString(it);

    else if (it.key() == "categories")
      m_defn.categories = getValueBool(it, m_defn.categories);

    else if (it.key() == "gvars")
      loadGroup(it, m_defn.gvars, CPN_MAX_GVARS, 3);

    else if (it.key() == "inputs")
      loadGroup(it, m_defn.inputs, CPN_MAX_INPUTS, 3);

    else if (it.key() == "keyShortcuts")
      m_defn.max_keyShortcuts = getValueInt(it);

    else if (it.key() == "logicalSW")
      loadGroup(it, m_defn.logicalSW, CPN_MAX_LOGICAL_SWITCHES, 3);

    else if (it.key() == "luaScripts")
      loadLuaScripts(it);

    else if (it.key() == "modelImage")
      loadModelImage(it);

    else if (it.key() == "modelNameLen")
      m_defn.max_modelName = getValueInt(it);

    else if (it.key() == "modelSlots")
      m_defn.max_modelSlots = getValueInt(it);

    else if (it.key() == "modes")
      loadGroup(it, m_defn.modes, CPN_MAX_FLIGHT_MODES, 3);

    else if (it.key() == "mixes")
      loadGroup(it, m_defn.mixes, CPN_MAX_MIXERS, 3);

    else if (it.key() == "outputs")
      loadOutputs(it);

    else if (it.key() == "quickMenuFavs")
      m_defn.max_quickMenuFavs = getValueInt(it);

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
        crv.limits.max_count = getValueInt(it, crv.limits.max_count, CPN_MAX_CURVES);
      else if (it.key() == "nameLen")
        crv.limits.max_name_len = getValueInt(it, crv.limits.max_name_len, 5);
      else if (it.key() == "points")
        crv.max_points = getValueInt(it, crv.max_points, 512);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: curves is not an object";
}

void Firmware::loadGroup(QJsonObject::const_iterator & grpit, Limits & grp,
                           const int cntMax, const int nameLenMax,
                           const int cntMin, const int nameLenMin)
{
  if (grpit->isObject()) {
    QJsonObject o = grpit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "cnt")
        grp.max_count = getValueInt(it, grp.max_count, cntMax, cntMin);
      else if (it.key() == "nameLen")
        grp.max_name_len = getValueInt(it, grp.max_name_len, nameLenMax, nameLenMin);
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
        lua.limits.max_count = getValueInt(it, lua.limits.max_count, CPN_MAX_SCRIPTS);
      else if (it.key() == "inputs")
        lua.max_inputs = getValueInt(it, lua.max_inputs, CPN_MAX_SCRIPT_INPUTS);
      else if (it.key() == "outputs")
        lua.max_outputs = getValueInt(it, lua.max_outputs, CPN_MAX_SCRIPT_OUTPUTS);
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
        mi.filters = getValueStdString(it, mi.filters);
      else if (it.key() == "image")
        mi.image = getValueBool(it, mi.image);
      else if (it.key() == "keepExtn")
        mi.keepExtn = getValueBool(it, mi.keepExtn);
      else if (it.key() == "nameLen")
        mi.limits.max_name_len = getValueInt(it, mi.limits.max_name_len, 14);
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
        grp.append(opt);
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
        out.limits.max_count = getValueInt(it, out.limits.max_count, CPN_MAX_CHNOUT);
      else if (it.key() == "nameLen")
        out.limits.max_name_len = getValueInt(it, out.limits.max_name_len, 6);
      else if (it.key() == "ppmCenter")
        out.ppmCenter = getValueInt(it, out.ppmCenter, 512);
      else if (it.key() == "ppmFrameLen")
        out.ppmFrameLen = getValueInt(it, out.ppmFrameLen, 40);
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
        tele.limits.max_count = getValueInt(it, tele.limits.max_count, 3);
      else if (it.key() == "bars")
        tele.max_bars = getValueInt(it, tele.max_bars, 4);
      else if (it.key() == "perLine")
        tele.max_perLine = getValueInt(it, tele.max_perLine, 3);
      else if (it.key() == "lines")
        tele.max_lines = getValueInt(it, tele.max_lines, 4);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: teleCstmScrns is not an object";
}

bool Firmware::postLoad()
{
  // set defaults here to avoid business rules elsewhere
  if (m_defn.boardId.empty())
    m_defn.boardId = m_defn.id;

  if (m_defn.dwnldId.empty())
    m_defn.dwnldId = m_defn.id;

  if (m_defn.simuId.empty())
    m_defn.simuId = m_defn.id;

  return true;
}

void Firmware::setCurrent(const QString & id)
{
  setCurrent(gFirmwareFactories->getFirmware(id));
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
    qCritical() << "ERROR - Set current firmware to instance:" << (firmware ? firmware->getId() : "unknown");

}

