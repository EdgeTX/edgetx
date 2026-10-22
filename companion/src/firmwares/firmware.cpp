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

/*  Only use for options that are firmware related or are after market hardware mods
    Additonal firmware and board definitions can be created for common mods though it does
    increase the list of available firmwares

    tooltip translation cannot be performed at runtime
    key:   name    (Do not translate)
    value: tooltip
*/

const Firmware::OptionTooltip Firmware::registeredOptions = {
  { "eu",               QT_TRANSLATE_NOOP("Firmware", "Removes D8 FrSky protocol support which is not legal for use in the EU on radios sold after Jan 1st, 2015") },
  { "faichoice",        QT_TRANSLATE_NOOP("Firmware", "Possibility to enable FAI MODE (no telemetry) at field") },
  { "faimode",          QT_TRANSLATE_NOOP("Firmware", "FAI MODE (no telemetry) always enabled") },
  { "flexr9m",          QT_TRANSLATE_NOOP("Firmware", "Enable non certified firmwares") },
  { "flyskygimbals",    QT_TRANSLATE_NOOP("Firmware", "Support hardware mod: FlySky Paladin EV Gimbals") },
  { "horussticks",      QT_TRANSLATE_NOOP("Firmware", "Horus gimbals installed (Hall sensors)") },
  { "lua",              QT_TRANSLATE_NOOP("Firmware", "Enable Lua custom scripts screen") },
  { "nogvars",          QT_TRANSLATE_NOOP("Firmware", "Disable Global variables") },
  { "noheli",           QT_TRANSLATE_NOOP("Firmware", "Disable HELI menu and cyclic mix support") },
  { "nooverridech",     QT_TRANSLATE_NOOP("Firmware", "No OverrideCH functions available") },
  { "noras",            QT_TRANSLATE_NOOP("Firmware", "Disable RAS (SWR)") },
  { "pcbdev",           QT_TRANSLATE_NOOP("Firmware", "Use ONLY with first DEV pcb version") },
  { "shutdownconfirm",  QT_TRANSLATE_NOOP("Firmware", "Confirmation before radio shutdown") },
  { "sqt5font",         QT_TRANSLATE_NOOP("Firmware", "Use alternative SQT5 font") },
  { "danger",           QT_TRANSLATE_NOOP("Firmware", "Enable dangerous functions") }
};

void Firmware::dumpCapabilities()
{
  for (int i = 0; i < Capability::FirstText; i++) {
    qDebug() << i << getCapability((Capability)i);
  }

  for (int i = Capability::FirstText; i < Capability::Count; i++) {
    qDebug() << i << getCapabilityStr((Capability)i);
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
    case Capability::BackgroundVolumeDefault:
      return m_defn.backgroundVolume.dflt;

    case Capability::BacklightLevelMin:
      return m_defn.backlight.min;

    case Capability::BacklightBright:
      return m_defn.backlight.onBrightness;

    case Capability::BacklightDelay:
      return m_defn.backlight.delay;

    case Capability::BacklightMode:
      return m_defn.backlight.mode;

    case Capability::BacklightOffBright:
      return m_defn.backlight.offBrightness;

    case Capability::ChannelsName:
      return m_defn.outputs.name;

    case Capability::ContrastDefault:
      return m_defn.contrast.dflt;

    case Capability::DangerousFunctions:
      return opts.contains("danger") ? true : false;

    case Capability::EepromSize:
      return m_defn.eepromSize;

    case Capability::ExtendedTrimsRange:
      return 512;

    case Capability::FlashSize:
      return m_defn.flashSize;

    case Capability::FlightModes:
      return m_defn.flightModes.cnt;

    case Capability::FlightModesName:
      return m_defn.flightModes.name;

    case Capability::FourCC:
      return m_defn.fourCC;

    case Capability::GlobalFunctions:
      return m_defn.globalFuncs.cnt;

    case Capability::Gvars:
      return opts.contains("nogvars") ? 0 : m_defn.globalVars.cnt;

    case Capability::GvarsName:
      return m_defn.globalVars.name;

    case Capability::HasBacklightKeys:
      return m_defn.backlight.keys;

    case Capability::HasExpoNames:
      return getCapability(Capability::InputsLength);

    case Capability::HasFailsafe:
      return 32;

    case Capability::HasFlySkyGimbals:
      return opts.contains("flyskygimbals") ||
             m_board->getCapability(Capability::HasFlySkyGimbals);

    case Capability::HasMixerNames:
      return m_defn.mixes.name;

    case Capability::HasModelImage:
      return m_defn.modelImage.image;

    case Capability::HasModelLabels:
      return m_defn.labels;

    case Capability::HasLCDtoVideo:
      return m_defn.lcdToVideo;

    case Capability::HasModelsList:
      return m_defn.modelsList;

    case Capability::HasModuleR9MFlex:
      return opts.contains("flexr9m");

    case Capability::HasPowerOnDelay:
      return m_defn.powerOnDelay;

    case Capability::HasVario:
      return getCapability(Capability::Air);

    case Capability::HasVarioSink:
      return getCapability(Capability::Air);

    case Capability::HatsModeDefault:
      return m_defn.hatsMode;

    case Capability::Heli:
      return !(opts.contains("noheli") ||
             m_board->getCapability(Capability::Surface));

    case Capability::InactivityTimerDefault:
      return m_defn.inactivityTimer.dflt;

    case Capability::InputsLength:
      return m_defn.inputs.name;

    case Capability::KeyShortcuts:
      return m_defn.keyShortcuts;

    case Capability::LogicalSwitches:
      return m_defn.logicalSwitches.cnt;

    case Capability::LuaInputsPerScript:
      return m_defn.luaScripts.maxInputs;

    case Capability::LuaOutputsPerScript:
      return m_defn.luaScripts.maxOutputs;

    case Capability::LuaScripts:
      return opts.contains("lua") ? m_defn.luaScripts.sizes.cnt : 0;

    // TODO remove as mavlink option not used but ui feature
    case Capability::MavlinkTelemetry:
      return opts.contains("mavlink");

    case Capability::MaxContrast:
      return m_defn.contrast.max;

    case Capability::MaxVolume:
      return m_defn.speakerVolume.max;

    case Capability::MinContrast:
      return m_defn.contrast.min;

    case Capability::Mixes:
      return m_defn.mixes.cnt;

    case Capability::ModelImageKeepExtn:
      return m_defn.modelImage.keepExtn;

    case Capability::ModelImageNameLen:
      return m_defn.modelImage.name;

    case Capability::ModelName:
      return m_defn.modelName;

    case Capability::Models:
      return m_defn.modelSlots;

    case Capability::NumCurves:
      return m_defn.curves.sizes.cnt;

    case Capability::NumCurvePoints:
      return 512; // Note: less than CPN_MAX_CURVES x CPN_MAX_POINTS

    case Capability::OffsetWeight:
      return 500;

    case Capability::Outputs:
      return m_defn.outputs.cnt;

    case Capability::PPMCenter:
      return 500;

    case Capability::PPMFrameLen:
      return 40;

    case Capability::QMFavourites:
      return m_defn.quickMenuFavs;

    case Capability::SafetyChannelCustomFunction:
      return opts.contains("nooverridech") ? 0 : 1;

    case Capability::Sensors:
      return m_defn.sensors.cnt;

    case Capability::SlowRange:
      return 250;

    case Capability::SlowScale:
      return 10;

    case Capability::SpeakerVolumeDefault:
      return m_defn.speakerVolume.dflt;

    case Capability::SpecialFunctions:
      return m_defn.specialFuncs.cnt;

    case Capability::TelemetryCustomScreens:
      return m_defn.teleCstmScrns.sizes.cnt;

    case Capability::TelemetryCustomScreensBars:
      return m_defn.teleCstmScrns.maxBars;

    case Capability::TelemetryCustomScreensFieldsPerLine:
      return m_defn.teleCstmScrns.maxPerLine;

    case Capability::TelemetryCustomScreensLines:
      return m_defn.teleCstmScrns.maxLines;

    case Capability::Timers:
      return m_defn.timers.cnt;

    case Capability::TimersName:
      return m_defn.timers.name;

    case Capability::TopBarZones:
      return m_defn.topBarZones;

    case Capability::TrainerInputs:
      return 16;

    case Capability::TrimsRange:
      return 128;

    // TODO use Inputs
    case VirtualInputs:
      return CPN_MAX_INPUTS;

    case Capability::VoicesMaxLength:
      return 8;

    case Capability::WavVolumeDefault:
      return m_defn.wavVolume.dflt;

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

    //JSON_KEY_VAL(it);

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

    else if (it.key() == "contrast")
      Json::loadRange(it, m_defn.contrast);

    else if (it.key() == "backgroundVolume")
      Json::loadRange(it, m_defn.backgroundVolume);

    else if (it.key() == "backlight")
      loadBacklight(it);

    else if (it.key() == "curves")
      loadCurves(it);

    else if (it.key() == "customScreens")
      loadCustomScreens(it);

    else if (it.key() == "eepromSize")
      m_defn.eepromSize = Json::valueInt(it, m_defn.eepromSize);

    else if (it.key() == "expos")
      Json::loadSizes(it, m_defn.expos, CPN_MAX_EXPOS, EXPODATA_NAME_LEN);

    else if (it.key() == "flashSize")
      m_defn.flashSize = Json::valueInt(it, m_defn.flashSize);

    else if (it.key() == "flightModes")
      Json::loadSizes(it, m_defn.flightModes, CPN_MAX_FLIGHT_MODES, FLIGHTMODE_NAME_LEN);

    else if (it.key() == "fourCC")
      m_defn.fourCC = Json::valueInt(it, m_defn.fourCC);

    else if (it.key() == "globalFuncs")
      Json::loadSizes(it, m_defn.globalFuncs, CPN_MAX_GLOBAL_FUNCTIONS, 0);

    else if (it.key() == "globalVars")
      Json::loadSizes(it, m_defn.globalVars, CPN_MAX_GVARS, GVAR_NAME_LEN);

    else if (it.key() == "inactivityTimer")
      Json::loadRange(it, m_defn.inactivityTimer);

    else if (it.key() == "inputs")
      Json::loadSizes(it, m_defn.inputs, CPN_MAX_INPUTS, INPUT_NAME_LEN);

    else if (it.key() == "lcdToVideo")
      m_defn.lcdToVideo = Json::valueBool(it, m_defn.lcdToVideo);

    else if (it.key() == "keyShortcuts")
      m_defn.keyShortcuts = Json::valueInt(it);

    else if (it.key() == "logicalSw")
      Json::loadSizes(it, m_defn.logicalSwitches, CPN_MAX_LOGICAL_SWITCHES, MIXDATA_NAME_LEN);

    else if (it.key() == "luaScripts")
      loadLuaScripts(it);

    else if (it.key() == "modelImage")
      loadModelImage(it);

    else if (it.key() == "modelName")
      m_defn.modelName = Json::valueInt(it);

    else if (it.key() == "modelSlots")
      m_defn.modelSlots = Json::valueInt(it);

    else if (it.key() == "mixes")
      Json::loadSizes(it, m_defn.mixes, CPN_MAX_MIXERS, MIXDATA_NAME_LEN);

    else if (it.key() == "outputs")
      Json::loadSizes(it, m_defn.outputs, CPN_MAX_CHNOUT, LIMITDATA_NAME_LEN);

    else if (it.key() == "powerOnDelay")
      m_defn.powerOnDelay = Json::valueBool(it);

    else if (it.key() == "quickMenuFavs")
      m_defn.quickMenuFavs = Json::valueInt(it);

    else if (it.key() == "sensors")
      Json::loadSizes(it, m_defn.sensors, CPN_MAX_SENSORS, SENSOR_LABEL_LEN);

    else if (it.key() == "speakerVolume")
      Json::loadRange(it, m_defn.speakerVolume);

    else if (it.key() == "specialFuncs")
      Json::loadSizes(it, m_defn.specialFuncs, CPN_MAX_SPECIAL_FUNCTIONS, 0);

    else if (it.key() == "teleCstmScrns")
      loadTeleCstmScrns(it);

    else if (it.key() == "timers")
      Json::loadSizes(it, m_defn.timers, CPN_MAX_TIMERS, TIMER_NAME_LEN);

    else if (it.key() == "topBarZones")
      m_defn.topBarZones = Json::valueInt(it);

    else if (it.key() == "wavVolume")
      Json::loadRange(it, m_defn.wavVolume);

    else
      JSON_NO_RULE(it);
  }

  delete doc;
  return true;
}

void Firmware::loadBacklight(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  BacklightDefn & bd = m_defn.backlight;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments" || it.key() == "backlight_bdtr")
      continue;

    //JSON_KEY_VAL(it);
    else if (it.key() == "delay")
      bd.delay = Json::valueInt(it, bd.delay);

    else if (it.key() == "keys")
      bd.keys = Json::valueBool(it, bd.keys);

    else if (it.key() == "min")
      bd.min = Json::valueInt(it, bd.min);

    else if (it.key() == "mode")
      bd.mode = Json::valueInt(it, bd.mode);

    else if (it.key() == "offBrightness")
      bd.offBrightness = Json::valueInt(it, bd.offBrightness);

    else if (it.key() == "onBrightness")
      bd.onBrightness = Json::valueInt(it, bd.onBrightness);

    else
      JSON_NO_RULE(it);
  }
}

void Firmware::loadCurves(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  CurvesDefn & crv = m_defn.curves;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments")
      continue;

      //JSON_KEY_VAL(it);

    else if (it.key() == "cnt")
      crv.sizes.cnt = Json::valueInt(it, crv.sizes.cnt, CPN_MAX_CURVES);

    else if (it.key() == "name")
      crv.sizes.name = Json::valueInt(it, crv.sizes.name, CURVEDATA_NAME_LEN);

    else if (it.key() == "points") {
      JSON_OBJ_INIT_C(it, po);

      for (QJsonObject::const_iterator pit = po.constBegin(); pit != po.constEnd(); ++pit) {
        if (pit.key() == "comments")
          continue;

        //JSON_KEY_VAL(pit);

        else if (pit.key() == "min")
          crv.minPoints = Json::valueInt(pit, crv.minPoints, 2);

        else if (pit.key() == "max")
          crv.maxPoints = Json::valueInt(pit, crv.maxPoints, CPN_MAX_POINTS);

        else if (pit.key() == "total")
          crv.totalPoints = Json::valueInt(pit, crv.totalPoints, 512);
        else
        JSON_NO_RULE(pit);
      }
    } else
      JSON_NO_RULE(it);
  }
}

void Firmware::loadCustomScreens(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  CustomScreensDefn & cs = m_defn.customScreens;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments")
      continue;

    //JSON_KEY_VAL(it);

    else if (it.key() == "cnt")
      cs.sizes.cnt = Json::valueInt(it, cs.sizes.cnt, CPN_MAX_SCRIPTS);

    else if (it.key() == "layoutOptions")
      cs.layoutOptions = Json::valueInt(it, cs.layoutOptions, MAX_LAYOUT_OPTIONS);

    else if (it.key() == "layoutZones")
      cs.layoutZones = Json::valueInt(it, cs.layoutZones, MAX_LAYOUT_ZONES);

    else if (it.key() == "topBarOptions")
      cs.topBarOptions = Json::valueInt(it, cs.topBarOptions, MAX_TOPBAR_OPTIONS);

    else if (it.key() == "topBarZones")
      cs.topBarZones = Json::valueInt(it, cs.topBarZones, MAX_TOPBAR_ZONES);

    else if (it.key() == "themeOptions")
      cs.themeOptions = Json::valueInt(it, cs.themeOptions, MAX_THEME_OPTIONS);

    else if (it.key() == "widgetOptions")
      cs.widgetOptions = Json::valueInt(it, cs.widgetOptions, MAX_WIDGET_OPTIONS);

    else
      JSON_NO_RULE(it);
  }
}

void Firmware::loadLuaScripts(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  LuaScriptsDefn & lua = m_defn.luaScripts;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments")
      continue;

    //JSON_KEY_VAL(it);

    else if (it.key() == "cnt")
      lua.sizes.cnt = Json::valueInt(it, lua.sizes.cnt, CPN_MAX_SCRIPTS);

    else if (it.key() == "name")
      lua.sizes.name = Json::valueInt(it, lua.sizes.name, CPN_LEN_SCRIPT_NAME);

    else if (it.key() == "inputs")
      lua.maxInputs = Json::valueInt(it, lua.maxInputs, CPN_MAX_SCRIPT_INPUTS);

    else if (it.key() == "outputs")
      lua.maxOutputs = Json::valueInt(it, lua.maxOutputs, CPN_MAX_SCRIPT_OUTPUTS);

    else
      JSON_NO_RULE(it);
  }
}

void Firmware::loadModelImage(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  ModelImageDefn & mi = m_defn.modelImage;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments")
      continue;

    //JSON_KEY_VAL(it);

    else if (it.key() == "filters")
      mi.filters = Json::valueStdString(it, mi.filters);

    else if (it.key() == "image")
      mi.image = Json::valueBool(it, mi.image);

    else if (it.key() == "keepExtn")
      mi.keepExtn = Json::valueBool(it, mi.keepExtn);

    else if (it.key() == "name")
      mi.name = Json::valueInt(it, mi.name, CPN_MAX_BITMAP_LEN);

    else
      JSON_NO_RULE(it);
  }
}

void Firmware::loadBuildOptions(QJsonObject::const_iterator & it)
{
  JSON_ARR_INIT_R(it, arrOptions);

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
}

void Firmware::loadBuildOptionGroup(QJsonArray::const_iterator & it, OptionsGroup & grp)
{
  if ((*it).isString()) {
    QString opt((*it).toString());

    if (registeredOptions.contains((*it).toString())) {
      if (!isOptionDuplicate(m_defn.options, opt)) {
        grp.append(opt);
      } else {
        qWarning() << "Duplicate option ignored:" << opt;
      }
    } else {
      qWarning() << "Invalid option:" << opt;
    }
  } else
    qWarning() << "Option is not a string:" << (*it);
}

void Firmware::loadTeleCstmScrns(QJsonObject::const_iterator & oit)
{
  JSON_OBJ_INIT_R(oit, o);

  TeleCstmScrnsDefn & tele = m_defn.teleCstmScrns;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    if (it.key() == "comments")
      continue;

    //JSON_KEY_VAL(it);

    else if (it.key() == "cnt")
      tele.sizes.cnt = Json::valueInt(it, tele.sizes.cnt, CPN_TELE_CUST_SCRNS);

    else if (it.key() == "bars")
      tele.maxBars = Json::valueInt(it, tele.maxBars, CPN_TELE_CUST_SCRNS_BARS);

    else if (it.key() == "perLine")
      tele.maxPerLine = Json::valueInt(it, tele.maxPerLine, CPN_TELE_CUST_SCRNS_PERLINE);

    else if (it.key() == "lines")
      tele.maxLines = Json::valueInt(it, tele.maxLines, CPN_TELE_CUST_SCRNS_LINES);

    else
      JSON_NO_RULE(oit);
  }
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

bool Firmware::setCurrent(const QString & id)
{
  return setCurrent(FirmwareFactories::getInstance().getFirmware(id));
}

bool Firmware::setCurrent(Firmware * firmware)
{
  bool result = false;

  if (firmware && firmware->loadDefinition()) {
    result = true;
    m_current = firmware;
    m_current->getBoard(true);  // ensure board definition is loaded as it may only be a skeleton
  }

  // call after board definition loaded as Firmware::getCapabilities falls thru to Board::getCapabilities
  m_current->dumpCapabilities();

  if (!result)
    qCritical() << "ERROR - Setting current firmware:"
                << (firmware ? firmware->getId() : "unknown");

  return result;
}
