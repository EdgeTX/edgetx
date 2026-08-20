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
#include "helpers_json.h"
#include "appdata.h"

Firmware::Firmware(const QString & id) :
  m_id(id),
  m_supported(false),
  m_loaded(false),
  m_valid(false),
  m_defn(FirmwareDefn()),
  m_board(nullptr)
{
  QJsonDocument *doc = new QJsonDocument();

  if (Json::load(doc, QString("%1/%2.json").arg(FWDEFNSDIR).arg(id))) {
    QJsonObject obj = doc->object();
    // ignore intermediate definitions
    if (!Json::value(obj, "hidden", false, false).toBool()) {
      m_supported = Json::value(obj, "supported", false, true).toBool();
      m_defn.name = Json::value(obj, "name", false, "unknown").toString().toStdString();
      m_defn.boardId = Json::value(obj, "board", false, id).toString().toStdString();

      if (Board::isAvailable(m_defn.boardId.c_str()))
        m_board = Board::getBoard(m_defn.boardId.c_str(), false);
      else
        qDebug() << "Error: cannot find board:" << m_defn.boardId;
    } else {
      qDebug() << "Error: attempt to register hidden firmware id:" << id;
    }
  }

  delete doc;
}

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

// TODO some of these options are redundant as they are handled by firmware and board defs
//      and others will be removed in Modules refactor so they are left here until cleaned up
//
// tooltip translation cannot be performed at runtime
// so translate tooltip only and load mapping at compile time
// key:   name    (Do not translate)
// value: tooltip
const Firmware::OptionTooltip Firmware::registeredOptions = {
  { "afhds2a",          QT_TRANSLATE_NOOP("Firmware", "Enable AFHDS2A support") },
  { "afhds3",           QT_TRANSLATE_NOOP("Firmware", "Enable AFHDS3 support") },
  { "bluetooth",        QT_TRANSLATE_NOOP("Firmware", "Support for bluetooth module") },
  { "eu",               QT_TRANSLATE_NOOP("Firmware", "Removes D8 FrSky protocol support which is not legal for use in the EU on radios sold after Jan 1st, 2015") },
  { "faichoice",        QT_TRANSLATE_NOOP("Firmware", "Possibility to enable FAI MODE (no telemetry) at field") },
  { "faimode",          QT_TRANSLATE_NOOP("Firmware", "FAI MODE (no telemetry) always enabled") },
  { "flexr9m",          QT_TRANSLATE_NOOP("Firmware", "Enable non certified firmwares") },
  { "flyskygimbals",    QT_TRANSLATE_NOOP("Firmware", "Support hardware mod: FlySky Paladin EV Gimbals") },
  { "haptic",           QT_TRANSLATE_NOOP("Firmware", "Haptic module installed") },
  { "horussticks",      QT_TRANSLATE_NOOP("Firmware", "Horus gimbals installed (Hall sensors)") },
  { "internalaccess",   QT_TRANSLATE_NOOP("Firmware", "Support for ACCESS internal module replacement") },
  { "internalelrs",     QT_TRANSLATE_NOOP("Firmware", "Select if internal ELRS module is installed") },
  { "internalgps",      QT_TRANSLATE_NOOP("Firmware", "Support internal GPS") },
  { "internalmulti",    QT_TRANSLATE_NOOP("Firmware", "Support for MULTI internal module") },
  { "lua",              QT_TRANSLATE_NOOP("Firmware", "Enable Lua custom scripts screen") },
  { "nogvars",          QT_TRANSLATE_NOOP("Firmware", "Disable Global variables") },
  { "noheli",           QT_TRANSLATE_NOOP("Firmware", "Disable HELI menu and cyclic mix support") },
  { "nooverridech",     QT_TRANSLATE_NOOP("Firmware", "No OverrideCH functions available") },
  { "noras",            QT_TRANSLATE_NOOP("Firmware", "Disable RAS (SWR)") },
  { "pcbdev",           QT_TRANSLATE_NOOP("Firmware", "Use ONLY with first DEV pcb version") },
  { "shutdownconfirm",  QT_TRANSLATE_NOOP("Firmware", "Confirmation before radio shutdown") },
  { "sqt5font",         QT_TRANSLATE_NOOP("Firmware", "Use alternative SQT5 font") }
};

void Firmware::dumpCapabilities()
{
  for (int i = 0; i < Capability::Count; i++) {
    qDebug() << i << getCapability((Capability)i);
  }
}

QString Firmware::getOptionTooltip(const QString & opt)
{
  return registeredOptions.value(opt, tr("No tooltip available for this option"));
}

Board * Firmware::getBoard(bool forceLoad) const
{
  if (!m_board->isLoaded() && forceLoad)
    m_board->loadDefinition();

  return m_board;
}

int Firmware::getCapability(Capability value) const
{
  const QStringList opts = g.currentProfile().fwOptions().split("-");

  switch (value) {
    case Capability::ChannelsName:
      return m_defn.outputs.limits.name;

    case Capability::SpecialFunctions:
      return m_defn.specialFuncs.max;

    case Capability::DangerousFunctions:
      return opts.contains("danger") ? true : false;

    case Capability::ExtendedTrimsRange:
      return m_defn.extTrimsRange;

    case Capability::FlightModes:
      return m_defn.flightModes.max;

    case Capability::FlightModesName:
      return m_defn.flightModes.name;

    case Capability::GlobalFunctions:
      return m_defn.globalFuncs.max;

    case Capability::Gvars:
      return opts.contains("nogvars") ? 0 : m_defn.globalVars.max;

    case Capability::GvarsName:
      return m_defn.globalVars.name;

    case Capability::HasExpoNames:
      return getCapability(Capability::InputsName);

    case Capability::FailsafeChannels:
      return m_defn.failsafeChans;

    case Capability::HasFailsafe:
      return m_defn.failsafeChans;

    case Capability::HasFlySkyGimbals:
      return opts.contains("flyskygimbals") || m_board->getCapability(Capability::HasFlySkyGimbals);

    case Capability::HasMixerNames:
      return m_defn.mixes.name;

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
      return m_defn.inputs.max;

    case Capability::InputsName:
      return m_defn.inputs.name;

    case Capability::InputsLength:
      return getCapability(Capability::InputsName);

    case Capability::KeyShortcuts:
      return m_defn.maxKeyShortcuts;

    case Capability::LogicalSwitches:
      return m_defn.logicalSwitches.max;

    case Capability::LuaInputsPerScript:
      return m_defn.luaScripts.maxInputs;

    case Capability::LuaOutputsPerScript:
      return m_defn.luaScripts.maxOutputs;

    case Capability::LuaScripts:
      return opts.contains("lua") ? m_defn.luaScripts.limits.max : 0;

    case Capability::Mixes:
      return m_defn.mixes.max;

    case Capability::ModelImageKeepExtn:
      return m_defn.modelImage.keepExtn;

    case Capability::ModelImageNameLen:
      return m_defn.modelImage.limits.name;

    case Capability::ModelName:
      return m_defn.maxModelName;

    case Capability::Models:
      return m_defn.maxModelSlots;

    case Capability::NumCurvePoints:
      return m_defn.curves.maxPoints;

    case Capability::NumCurves:
      return m_defn.curves.limits.max;

    case Capability::OffsetWeight:
      return m_defn.offsetWeight;

    case Capability::Outputs:
      return m_defn.outputs.limits.max;

    case Capability::PPMCenter:
      return m_defn.outputs.ppmCenter;

    case Capability::PPMFrameLength:
      return m_defn.outputs.ppmFrameLen;

    case Capability::QMFavourites:
      return m_defn.maxQuickMenuFavs;

    case Capability::SafetyChannelCustomFunction:
      return opts.contains("nooverridech") ? 0 : 1;

    case Capability::Sensors:
      return m_defn.sensors.max;

    case Capability::SlowRange:
      return m_defn.slowRange;

    case Capability::SlowScale:
      return m_defn.slowScale;

    case Capability::TelemetryCustomScreens:
      return m_defn.teleCstmScrns.limits.max;

    case Capability::TelemetryCustomScreensBars:
      return m_defn.teleCstmScrns.maxBars;

    case Capability::TelemetryCustomScreensFieldsPerLine:
      return m_defn.teleCstmScrns.maxPerLine;

    case Capability::TelemetryCustomScreensLines:
      return m_defn.teleCstmScrns.maxLines;

    case Capability::Timers:
      return m_defn.timers.max;

    case Capability::TimersName:
      return m_defn.timers.name;

    case Capability::TopBarZones:
      return m_defn.topBarZones;

    case Capability::TrainerInputs:
      return m_defn.maxTrainerInputs;

    case Capability::TrimsRange:
      return m_defn.trimsRange;

    case Capability::VoicesMaxLength:
      return m_defn.maxVoicesFileLen;

    // drop thru to Board
    default:
      return m_board->getCapability(value);
  }
}

QString Firmware::getCapabilityStr(Capability value) const
{
  switch (value) {
    case Capability::ModelImageFilters:
      return m_defn.modelImage.filters.c_str();

    // drop thru to Board
    default:
      return m_board->getCapabilityStr(value);
  }
}

Firmware * Firmware::getFirmware(const QString & id, bool forceLoad)
{
  //qDebug() << "id:" << id << "forceLoad:" << forceLoad;
  auto firmware = FirmwareFactories::getInstance().getFirmware(id, forceLoad);

  if (forceLoad)
    firmware->getBoard(forceLoad);

  return firmware;
}

const QString Firmware::getFullName() const
{
  return m_board->getCapabilityStr(Capability::Manufacturer) % " " % m_defn.name.c_str();
}

QList<Firmware *> Firmware::getRegisteredFirmwares()
{
  return FirmwareFactories::getInstance().getRegisteredFirmwares();
}

bool Firmware::isAvailable(const QString & id)
{
  return FirmwareFactories::getInstance().isAvailable(id);
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
  if (m_loaded) return true;

  // load default.json first and allow subsequent file values to override
  // this avoids having to include default in basedOn tree

  if (loadDefinition(QString("%1/%2.json").arg(FWDEFNSDIR).arg("default"))) {
    if (loadDefinition(QString("%1/%2.json").arg(FWDEFNSDIR).arg(m_id))) {
      if (postLoad()) {
        m_loaded = true;
        m_valid = true;
        qDebug() << "Definition loaded:" << m_id;
        return true;
      }
    }
  }

  return false;
}

bool Firmware::loadDefinition(const QString & path)
{
  /*
    Iterating is less efficient than looking for specific keys, especially at the top level.
    However, it does provide the benefit of allowing reporting of all key value pairs
    and any unexpected keys, which can make debugging json files easier.
    The overhead is not excessive since we only load the full definition for
    the firmware used.

    An alternative is to build a custom schema and validate files against it.
    As at Qt 6.9 there is no such a feature thus a third party
    utility such as nlohmann/json-schema-validator would need to be
    incorporated into the build process.
  */

  bool success = true;
  QJsonDocument *doc = new QJsonDocument();
  QJsonObject o;
  QStringList depends;  // stores basedOn tree and used to test for circular references

  if (Json::load(doc, path)) {
    if (doc->isObject()) {
      o = doc->object();

      // for each dependency walk to its root and retrace path loading definitions
      if (Json::isArray(o, "basedOn")) {
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

  qDebug() << "Loading values from:" << path;

  // key: supported is not manditory and if omitted it is assumed true
  // if an earlier definition contained "supported": false
  // we need to override with true if it is omitted in a later file
  m_supported = Json::value(o, "supported", false, true).toBool();

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      // skip early to save processing time and avoid unknown key warning messages
    if (it.key() == "hidden"    || it.key() == "basedOn"  ||
        it.key() == "supported" || it.key() == "comments" ||
        it.key() == "id"        || it.key() == "name"     ||
        it.key() == "board")
      continue;

    //qDebug() << "key:" << it.key() << "value:" << it.value();

    if (it.key() == "dwnldId")
      m_defn.dwnldId = Json::valueStdString(it);

    else if (it.key() == "simuId")
      m_defn.simuId = Json::valueStdString(it);

    else if (it.key() == "buildOpts")
      loadBuildOptions(it);

    else if (it.key() == "categories")
      m_defn.categories = Json::valueBool(it, m_defn.categories);

    // TODO is this the same as categories?
    else if (it.key() == "labels")
      m_defn.labels = Json::valueBool(it, m_defn.labels);

    // TODO is this still used - check what is generated by firmware
    else if (it.key() == "modelsList")
      m_defn.modelsList = Json::valueBool(it, m_defn.modelsList);

    else if (it.key() == "channels")
      loadArrayLimits(it, m_defn.channels, CPN_MAX_CHNOUT, 6);

    else if (it.key() == "curves")
      loadCurves(it);

    else if (it.key() == "expos")
      loadArrayLimits(it, m_defn.expos, CPN_MAX_EXPOS, 10);

    else if (it.key() == "flightModes")
      loadArrayLimits(it, m_defn.flightModes, CPN_MAX_FLIGHT_MODES, 4);

    else if (it.key() == "globalFuncs")
      loadArrayLimits(it, m_defn.globalFuncs, CPN_MAX_GLOBAL_FUNCTIONS, 0);

    else if (it.key() == "globalVars")
      loadArrayLimits(it, m_defn.globalVars, CPN_MAX_GVARS, 3);

    else if (it.key() == "inputs")
      loadArrayLimits(it, m_defn.inputs, CPN_MAX_INPUTS, 3);

    else if (it.key() == "lcdtoVideo")
      m_defn.lcdtoVideo = Json::valueBool(it, m_defn.lcdtoVideo);

    else if (it.key() == "keyShortcuts")
      m_defn.maxKeyShortcuts = Json::valueInt(it);

    else if (it.key() == "logicalSw")
      loadArrayLimits(it, m_defn.logicalSwitches, CPN_MAX_LOGICAL_SWITCHES, 3);

    else if (it.key() == "luaScripts")
      loadLuaScripts(it);

    else if (it.key() == "modelImage")
      loadModelImage(it);

    else if (it.key() == "modelNameLen")
      m_defn.maxModelName = Json::valueInt(it);

    else if (it.key() == "modelSlots")
      m_defn.maxModelSlots = Json::valueInt(it);

    else if (it.key() == "mixes")
      loadArrayLimits(it, m_defn.mixes, CPN_MAX_MIXERS, 3);

    else if (it.key() == "outputs")
      loadOutputs(it);

    else if (it.key() == "quickMenuFavs")
      m_defn.maxQuickMenuFavs = Json::valueInt(it);

    else if (it.key() == "sensors")
      loadArrayLimits(it, m_defn.sensors, CPN_MAX_SENSORS, 4);

    else if (it.key() == "specialFuncs")
      loadArrayLimits(it, m_defn.specialFuncs, CPN_MAX_SPECIAL_FUNCTIONS, 0);

    else if (it.key() == "teleCstmScrns")
      loadTeleCstmScrns(it);

    else if (it.key() == "timers")
      loadArrayLimits(it, m_defn.timers, CPN_MAX_TIMERS, 8);

    else if (it.key() == "topBarZones")
      m_defn.topBarZones = Json::valueInt(it);

    else
      qWarning() << "Warning: No rule to process - path:" << path << "name:" << it.key() << "value:" << it.value();
  }

  delete doc;
  return true;
}

void Firmware::loadCurves(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    Curves & crv = m_defn.curves;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "comments")
        continue;
      else if (it.key() == "max")
        crv.limits.max = Json::valueInt(it, crv.limits.max, CPN_MAX_CURVES);
      else if (it.key() == "name")
        crv.limits.name = Json::valueInt(it, crv.limits.name, 5);
      else if (it.key() == "points") {
        if (it->isObject()) {
          QJsonObject po = it->toObject();
          for (QJsonObject::const_iterator pit = po.constBegin(); pit != po.constEnd(); ++pit) {
            if (pit.key() == "comments")
              continue;
            else if (pit.key() == "min")
              crv.minPoints = Json::valueInt(pit, crv.minPoints, 2);
            else if (pit.key() == "max")
              crv.maxPoints = Json::valueInt(pit, crv.maxPoints, 17);
            else if (pit.key() == "total")
              crv.totalPoints = Json::valueInt(pit, crv.totalPoints, 512);
            else
            qWarning() << "Warning: No rule to process - name:" << pit.key() << "value:" << pit.value();
          }
        } else
          qWarning() << "Warning: curve points is not an object";
      } else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: curves is not an object";
}

void Firmware::loadArrayLimits(QJsonObject::const_iterator & grpit, ArrayLimits & grp,
                           const int cntMax, const int nameLenMax,
                           const int cntMin, const int nameLenMin)
{
  if (grpit->isObject()) {
    QJsonObject o = grpit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "comments")
        continue;
      else if (it.key() == "max")
        grp.max = Json::valueInt(it, grp.max, cntMax, cntMin);
      else if (it.key() == "name")
        grp.name = Json::valueInt(it, grp.name, nameLenMax, nameLenMin);
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
      if (it.key() == "comments")
        continue;
      else if (it.key() == "max")
        lua.limits.max = Json::valueInt(it, lua.limits.max, CPN_MAX_SCRIPTS);
      else if (it.key() == "inputs")
        lua.maxInputs = Json::valueInt(it, lua.maxInputs, CPN_MAX_SCRIPT_INPUTS);
      else if (it.key() == "outputs")
        lua.maxOutputs = Json::valueInt(it, lua.maxOutputs, CPN_MAX_SCRIPT_OUTPUTS);
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
      if (it.key() == "comments")
        continue;
      else if (it.key() == "filters")
        mi.filters = Json::valueStdString(it, mi.filters);
      else if (it.key() == "image")
        mi.image = Json::valueBool(it, mi.image);
      else if (it.key() == "keepExtn")
        mi.keepExtn = Json::valueBool(it, mi.keepExtn);
      else if (it.key() == "name")
        mi.limits.name = Json::valueInt(it, mi.limits.name, 14);
      else
        qWarning() << "Warning: No rule to process - name:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: modelImage is not an object";
}

void Firmware::loadBuildOptions(QJsonObject::const_iterator & it)
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
            loadBuildOptionGroup(itOptGrp, grp);
          else
            qWarning() << "Warning: duplicate option:" << *itOptGrp;
        }
      } else
        loadBuildOptionGroup(itOptions, grp);

      if (grp.count())
        m_defn.options.append(grp);
    }
  } else
    qWarning() << "Warning: buildOpts is not an array";
}

void Firmware::loadBuildOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp)
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
    qWarning() << "Option is not a string:" << (*it);
}

void Firmware::loadOutputs(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    QJsonObject o = oit->toObject();
    Outputs & out = m_defn.outputs;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "comments")
        continue;
      else if (it.key() == "max")
        out.limits.max = Json::valueInt(it, out.limits.max, CPN_MAX_CHNOUT);
      else if (it.key() == "name")
        out.limits.name = Json::valueInt(it, out.limits.name, 6);
      else if (it.key() == "ppmCenter")
        out.ppmCenter = Json::valueInt(it, out.ppmCenter, 512);
      else if (it.key() == "ppmFrameLen")
        out.ppmFrameLen = Json::valueInt(it, out.ppmFrameLen, 40);
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
      if (it.key() == "comments")
        continue;
      else if (it.key() == "max")
        tele.limits.max = Json::valueInt(it, tele.limits.max, 3);
      else if (it.key() == "bars")
        tele.maxBars = Json::valueInt(it, tele.maxBars, 4);
      else if (it.key() == "perLine")
        tele.maxPerLine = Json::valueInt(it, tele.maxPerLine, 3);
      else if (it.key() == "lines")
        tele.maxLines = Json::valueInt(it, tele.maxLines, 4);
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
  setCurrent(FirmwareFactories::getInstance().getFirmware(id));
}

void Firmware::setCurrent(Firmware * firmware)
{
  bool result = false;

  if (firmware && firmware->loadDefinition()) {
    result = true;
    m_current = firmware;
    // force loading board definition as it may only be a skeleton
    // if not loaded previously
    m_current->getBoard(true);
  }

  m_current->dumpCapabilities();

  if (!result)
    qCritical() << "ERROR - Set current firmware to instance:" << (firmware ? firmware->getId() : "unknown");

}

