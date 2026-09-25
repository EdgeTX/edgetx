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

#include "board.h"
#include "macros.h"
#include "compounditemmodels.h"
#include "helpers.h"
#include "boardfactories.h"
#include "generalsettings.h"
#include "modeldata.h"

#include <QMessageBox>

// TODO remove all those constants
// Update: These are now all only used within this class.
//  External access is only via getEEpromSize() and getFlashSize()

#define EESIZE_TARANIS                 (32*1024)
#define EESIZE_MAX                     EESIZE_TARANIS

// getFlashSize() (and these macros) is only used by radiointerface::getDfuArgs (perhaps can find a better way?)

#define FSIZE_512KB                    (512*1024)
#define FSIZE_1MB                      (1024*1024)
#define FSIZE_2MB                      (2048*1024)
#define FSIZE_MAX                      FSIZE_2MB

// pre v2.10
static const StringTagMappingTable legacyTrimSourcesLut = {
    {std::to_string(Board::TRIM_AXIS_LH), "TrimRud"},
    {std::to_string(Board::TRIM_AXIS_LV), "TrimEle"},
    {std::to_string(Board::TRIM_AXIS_RV), "TrimThr"},
    {std::to_string(Board::TRIM_AXIS_RH), "TrimAil"},
    {std::to_string(Board::TRIM_AXIS_T5), "TrimT5"},
    {std::to_string(Board::TRIM_AXIS_T6), "TrimT6"},
    {std::to_string(Board::TRIM_AXIS_T7), "TrimT7"},
    {std::to_string(Board::TRIM_AXIS_T8), "TrimT8"},
};

static const StringTagMappingTable trimSwitchesLut = {
    {std::to_string(Board::TRIM_SW_LH_DEC), "TrimRudLeft"},
    {std::to_string(Board::TRIM_SW_LH_INC), "TrimRudRight"},
    {std::to_string(Board::TRIM_SW_LV_DEC), "TrimEleDown"},
    {std::to_string(Board::TRIM_SW_LV_INC), "TrimEleUp"},
    {std::to_string(Board::TRIM_SW_RV_DEC), "TrimThrDown"},
    {std::to_string(Board::TRIM_SW_RV_INC), "TrimThrUp"},
    {std::to_string(Board::TRIM_SW_RH_DEC), "TrimAilLeft"},
    {std::to_string(Board::TRIM_SW_RH_INC), "TrimAilRight"},
    {std::to_string(Board::TRIM_SW_T5_DEC), "TrimT5Down"},
    {std::to_string(Board::TRIM_SW_T5_INC), "TrimT5Up"},
    {std::to_string(Board::TRIM_SW_T6_DEC), "TrimT6Down"},
    {std::to_string(Board::TRIM_SW_T6_INC), "TrimT6Up"},
    {std::to_string(Board::TRIM_SW_T7_INC), "TrimT7Up"},
    {std::to_string(Board::TRIM_SW_T7_DEC), "TrimT7Down"},
    {std::to_string(Board::TRIM_SW_T8_INC), "TrimT8Up"},
    {std::to_string(Board::TRIM_SW_T8_DEC), "TrimT8Down"},
};

static const StringTagMappingTable inputTypesLookupTable = {
    {std::to_string(Board::AIT_NONE),    "NONE"},
    {std::to_string(Board::AIT_STICK),   "STICK"},
    {std::to_string(Board::AIT_FLEX),    "FLEX"},
    {std::to_string(Board::AIT_VBAT),    "VBAT"},
    {std::to_string(Board::AIT_RTC_BAT), "RTC_BAT"},
    {std::to_string(Board::AIT_LUX),     "LUX"},
    {std::to_string(Board::AIT_SWITCH),  "SWITCH"},
};

static const StringTagMappingTable flexTypesLookupTable = {
    {std::to_string(Board::FLEX_NONE),        "NONE"},
    {std::to_string(Board::FLEX_POT),         "POT"},
    {std::to_string(Board::FLEX_POT_CENTER),  "POT_CENTER"},
    {std::to_string(Board::FLEX_SLIDER),      "SLIDER"},
    {std::to_string(Board::FLEX_MULTIPOS),    "MULTIPOS"},
    {std::to_string(Board::FLEX_AXIS_X),      "AXIS_X"},
    {std::to_string(Board::FLEX_AXIS_Y),      "AXIS_Y"},
    {std::to_string(Board::FLEX_SWITCH),      "SWITCH"},
};

static const StringTagMappingTable stickNamesLookupTable = {
    {QCoreApplication::translate("Board", "Rud").toStdString(), "LH"},  // air
    {QCoreApplication::translate("Board", "Ele").toStdString(), "LV"},  // air
    {QCoreApplication::translate("Board", "Thr").toStdString(), "RV"},  // air
    {QCoreApplication::translate("Board", "Ail").toStdString(), "RH"},  // air
    {QCoreApplication::translate("Board", "ST").toStdString(),  "ST"},  // surface
    {QCoreApplication::translate("Board", "TH").toStdString(),  "TH"},  // surface
};

static const StringTagMappingTable switchTypesLookupTable = {
    {std::to_string(Board::SWITCH_NOT_AVAILABLE), "NONE"},
    {std::to_string(Board::SWITCH_TOGGLE),        "TOGGLE"},
    {std::to_string(Board::SWITCH_2POS),          "2POS"},
    {std::to_string(Board::SWITCH_3POS),          "3POS"},
    {std::to_string(Board::SWITCH_ADC),           "ADC"},
};

// not used for yaml
static const StringTagMappingTable extModuleSizeLookupTable = {
    {std::to_string(Board::EXTMODSIZE_NONE),   "none"},
    {std::to_string(Board::EXTMODSIZE_STD),    "std"},
    {std::to_string(Board::EXTMODSIZE_SMALL),  "small"},
    {std::to_string(Board::EXTMODSIZE_BOTH),   "both"},
};

// not used for yaml
static const StringTagMappingTable intModuleTypesLookupTable = {
    {std::to_string(MODULE_TYPE_NONE),              "none"},
    {std::to_string(MODULE_TYPE_CROSSFIRE),         "crsf"},
    {std::to_string(MODULE_TYPE_DSM2),              "dsm2"},
    {std::to_string(MODULE_TYPE_CROSSFIRE),         "elrs"},
    {std::to_string(MODULE_TYPE_FLYSKY_AFHDS2A),    "flysky-afhds2a"},
    {std::to_string(MODULE_TYPE_FLYSKY_AFHDS3),     "flysky-afhds3"},
    {std::to_string(MODULE_TYPE_GHOST),             "ghost"},
    {std::to_string(MODULE_TYPE_LEMON_DSMP),        "lemon-dsmp"},
    {std::to_string(MODULE_TYPE_MULTIMODULE),       "multi"},
    {std::to_string(MODULE_TYPE_PPM),               "ppm"},
    {std::to_string(MODULE_TYPE_SBUS),              "sbus"},

    {std::to_string(MODULE_TYPE_ISRM_PXX2),         "isrm-pxx2"},
    {std::to_string(MODULE_TYPE_R9M_PXX1),          "r9m-pxx1"},
    {std::to_string(MODULE_TYPE_R9M_PXX2),          "r9m-pxx2"},
    {std::to_string(MODULE_TYPE_R9M_LITE_PXX1),     "r9m-lite-pxx1"},
    {std::to_string(MODULE_TYPE_R9M_LITE_PXX2),     "r9m-lite-pxx2"},
    {std::to_string(MODULE_TYPE_R9M_LITE_PRO_PXX2), "r9m-lite-pro-pxx1"},
    {std::to_string(MODULE_TYPE_XJT_PXX1),          "xjt-pxx1"},
    {std::to_string(MODULE_TYPE_XJT_LITE_PXX2),     "xjt-lite-pxx2"},
};

// not used for yaml
static const StringTagMappingTable trainerModuleLookupTable = {
    {std::to_string(0), "none"},
    {std::to_string(1), "ccpm"},
    {std::to_string(2), "sbus"},
};

// mapping json tag (1st entry) to legacy tag (2nd entry)
// json tag then used to find inputs index
// only used to decode pre v2.10 yaml configs
StringTagMappingTable Board::getLegacyAnalogsLookupTable(QString boardId)
{
  Board *board = Board::getBoardForId(boardId);
  StringTagMappingTable tbl;

  tbl.insert(tbl.end(), {
                              {tr("LH").toStdString(), "Rud"},
                              {tr("LV").toStdString(), "Ele"},
                              {tr("RV").toStdString(), "Thr"},
                              {tr("RH").toStdString(), "Ail"},
  });

  tbl.insert(tbl.end(), {

                              {tr("JSx").toStdString(), "MOUSE1"},
                              {tr("JSy").toStdString(), "MOUSE2"},
                              {tr("JSx").toStdString(), "JSx"},
                              {tr("JSy").toStdString(), "JSy"},
                              {tr("TILT_X").toStdString(), "TILT_X"},
                              {tr("TILT_Y").toStdString(), "TILT_Y"},
                              {tr("TILT_X").toStdString(), "GYRO1"},
                              {tr("TILT_Y").toStdString(), "GYRO2"},
  });

  if (board->getId() == "x9lite") {
    tbl.insert(tbl.end(), {
                              {tr("SL1").toStdString(), "S1"},
                              {tr("P1").toStdString(), "POT1"},
                            });
  } else if (board->getId() == "x9e") {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                              {tr("P4").toStdString(), "POT4"},
                              {tr("SL1").toStdString(), "SLIDER1"},
                              {tr("SL2").toStdString(), "SLIDER2"},
                              {tr("SL3").toStdString(), "SLIDER3"},
                              {tr("SL4").toStdString(), "SLIDER4"},
                          });
  } else if (IS_TARANIS_XLITES(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                          });
  } else if (IS_RADIOMASTER_BOXER(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                          });
  } else if (IS_RADIOMASTER_POCKET(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "P1"},
                          });
  } else if ((IS_TARANIS_SMALL(board) && !IS_JUMPER_TLITE(board) && !IS_JUMPER_T20(board)) || IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                          });
  } else if (IS_TARANIS_X9(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                              {tr("SL1").toStdString(), "SLIDER1"},
                              {tr("SL2").toStdString(), "SLIDER2"},
                          });
  } else if (IS_HORUS_X12S(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "S1"},
                              {tr("P2").toStdString(), "6POS"},
                              {tr("P3").toStdString(), "S2"},
                              {tr("P4").toStdString(), "S3"},
                              {tr("P5").toStdString(), "S4"},
                              {tr("SL1").toStdString(), "LS"},
                              {tr("SL2").toStdString(), "RS"},
                          });
  } else if (IS_FLYSKY_PL18(board) || IS_FLYSKY_PL18U(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                              {tr("SL1").toStdString(), "LS"},
                              {tr("SL2").toStdString(), "RS"},
                          });
  } else if (IS_FLYSKY_PL18EV(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                              {tr("SL1").toStdString(), "LS"},
                              {tr("SL2").toStdString(), "RS"},
                              {tr("EXT1").toStdString(), "EXT1"},
                              {tr("EXT2").toStdString(), "EXT2"},
                              {tr("EXT3").toStdString(), "EXT3"},
                              {tr("EXT4").toStdString(), "EXT4"},
                          });
  } else if (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "S1"},
                              {tr("P2").toStdString(), "6POS"},
                              {tr("P3").toStdString(), "S2"},
                              {tr("EXT1").toStdString(), "EXT1"},
                              {tr("EXT2").toStdString(), "EXT2"},
                              {tr("EXT3").toStdString(), "EXT3"},
                              {tr("EXT4").toStdString(), "EXT4"},
                              {tr("SL1").toStdString(), "LS"},
                              {tr("SL2").toStdString(), "RS"},
                          });
  } else if (IS_JUMPER_T20(board)) {
      tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "P1"},
                              {tr("P2").toStdString(), "P2"},
                              {tr("SL1").toStdString(), "SL1"},
                              {tr("SL2").toStdString(), "SL2"},
                              {tr("SL3").toStdString(), "SL3"},
                              {tr("SL4").toStdString(), "SL4"},
                            });
  }

  return tbl;
}

std::string Board::getLegacyAnalogMappedInputTag(const char * legacytag, const QString & boardId)
{
  return DataHelpers::getStringTagMappingName(Board::getLegacyAnalogsLookupTable(boardId == Board::BOARD_UNKNOWN ? getCurrentBoard()->getId() : boardId), legacytag);
}

Board::Board(const QString & id, const QString & hwdefn, const bool isSupported) :
  JsonBase(),
  m_id(id),
  m_hwdefn(hwdefn),
  m_name("unknown"),
  m_manufacturer("unknown"),
  m_loaded(false),
  m_valid(true),
  m_inputCnt({0, 0, 0, 0, 0, 0, 0, 0, 0}),
  m_switchCnt({0, 0, 0}),
  trimSwitchesLookupTable(trimSwitchesLut),
  rawSwitchTypesLookupTable(RawSwitch::getRawSwitchTypesLookupTable()),
  rawSourceSpecialTypesLookupTable(RawSource::getSpecialTypesLookupTable())
{
  QJsonDocument *doc = new QJsonDocument();

  if (load(doc, hwdefn)) {
    QJsonObject obj = doc->object();
    // ignore intermediate definitions
    if (!getValue(obj, "hidden", false).toBool()) {
      m_id = getValueString(obj, "id", "unknown");
      m_name = getValue(obj, "name", "unknown").toString();

      if (m_id == "unknown") {
        m_valid = false;
        qCritical() << "Error - file:" << hwdefn << "does not contain an id";
      }
    } else {
      qDebug() << "ignoring" << hwdefn;
    }
  }

  delete doc;
}

Board::~Board()
{
}

Board * Board::getBoardForId(const QString & id)
{
  return gBoardFactories->boardForId(id);
}

int Board::getCapability(const Capability capability) const
{
  switch (capability) {
    case Capability::BacklightLevelMin:
      return m_hardware.backlightLevelMin;

    case Capability::DefaultInternalModule:
      return m_hardware.intModules.dflt;

    // TODO remove when Modules refactored and use extModules
    case Capability::DefaultExternalModuleSize:
      return getCapability(Capability::HasExternalModuleSupport) ?
             m_hardware.defExtModSz : Board::EXTMODSIZE_NONE;

    case Capability::HasAuxSerialMode:
      return m_hardware.auxSerialMode;

    case Capability::HasAux2SerialMode:
      return m_hardware.aux2SerialMode;

    case Capability::HasBluetooth:
      return m_hardware.bluetooth;

    case Capability::HasExternalAntenna:
      return m_hardware.externalAntenna;

    case Capability::HasHardwareAntennaSwitch:
      return m_hardware.hardwareAntennaSwitch;

    case Capability::HasIMU:
      return m_hardware.imu;

    case HasInternalGPS:
      return m_hardware.internalGPS;

    case Capability::HasSoftwareSerialPower:
      return m_hardware.softwareSerialPower;

    case Capability::HasSwitchableJack:
      return m_hardware.switchableJack;

    case Capability::HasTrainerModuleCPPM:
      return m_hardware.trainerModule == Board::TR_MOD_TYPE_CPPM;

    case Capability::HasTrainerModuleSBUS:
      return m_hardware.trainerModule == Board::TR_MOD_TYPE_SBUS;

    case Capability::HasVCPSerialMode:
      return m_hardware.vcpSerialMode;

    case Capability::MaxContrast:
      return m_hardware.contrast.max;

    case Capability::MaxVolume:
      return m_hardware.maxVolume;

    case Capability::MinContrast:
      return m_hardware.contrast.min;

    case Capability::PwrButtonPress:
      return m_hardware.pwrButtonPress;

    case Capability::RotaryEncoderNavigation:
      return m_hardware.rotaryEncNav;

    case Capability::Air:
      return !m_hardware.surface;

    case Capability::FlexInputs:
      return (m_inputCnt.flexGyroAxes +
              m_inputCnt.flexJoystickAxes +
              m_inputCnt.flexPots +
              m_inputCnt.flexSliders +
              m_inputCnt.flexSwitches);

    case Capability::FlexSwitches:
      return m_switchCnt.flex;

    case Capability::FunctionSwitchColors:
      return m_cfs.rgb_led;

    case Capability::FunctionSwitches:
      return m_switchCnt.func;

    case Capability::FunctionSwitchGroups:
      return m_cfs.groups;

    case Capability::GyroAxes:
      return m_inputCnt.flexGyroAxes;

    case Capability::Gyros:
      return getCapability(Capability::GyroAxes) / 2;

    case Capability::HasAudioMuteGPIO:
      return m_hardware.has_audio_mute;

    case Capability::HasBacklightColor:
      return m_display.backlight_color;

    case Capability::HasBlingLEDS:
      return m_hardware.has_bling_leds;

    case Capability::HasColorLcd:
      return m_display.color;

    case Capability::HasExternalModuleSupport:
      return m_hardware.has_ext_module_support;

    case Capability::HasInternalModuleSupport:
      return m_hardware.has_int_module_support;

    case Capability::HasKeyLockCombo:
      return m_hardware.hasKeyLockCombo;

    case Capability::HasRTC:
      return m_inputCnt.rtcbat;

    case Capability::HasSDCard:
      return true;

    case Capability::HasVBat:
      return m_inputCnt.vbat;

    case Capability::Inputs:
      return m_inputs.size();

    case Capability::InputSwitches:
      return m_inputCnt.switches;

    case Capability::IsF4:
      return m_hardware.cpu_type == "STM32F4";

    case Capability::IsH5:
      return m_hardware.cpu_type == "STM32H5";

    case Capability::IsH7:
      return m_hardware.cpu_type == "STM32H7";

    case Capability::JoystickAxes:
      return m_inputCnt.flexJoystickAxes;

    case Capability::Keys:
      return m_keys.size();

    case Capability::LcdDepth:
      return m_display.depth;

    case Capability::LcdHeight:
      return m_display.h;

    case Capability::LcdOLED:
      return m_display.oled;

    case Capability::LcdWidth:
      return m_display.w;

    case Capability::MultiposPots:
      // assumes every input has potential to be one
      // index used for mapping 6 pos switches back to input
      return getCapability(Capability::Inputs);

    case Capability::MultiposPotsPositions:
      return 6;

    case Capability::NumFunctionSwitchesPositions:
       return getCapability(Capability::FunctionSwitches) * 3;

    case Capability::NumTrims:
      return m_trims.size();

    case Capability::NumTrimSwitches:
      return getCapability(Capability::NumTrims) * 2;

    case Capability::Pots:
      return m_inputCnt.flexPots;

    case Capability::Sliders:
      return m_inputCnt.flexSliders;

    case Capability::SportMaxBaudRate:
      return m_hardware.sport_max_baudrate;

    case Capability::StandardSwitches:
      return m_switchCnt.std;

    case Capability::Sticks:
      return m_inputCnt.sticks;

    case Capability::Surface:
      return m_hardware.surface;

    case Capability::Switches:
      return (m_switchCnt.std +
              m_switchCnt.flex +
              m_switchCnt.func);

    case Capability::SwitchesPositions:
      return getCapability(Capability::Switches) * 3;

    default:
      qWarning() << "Warning: unknown capability:" << capability << "returned 0";
      return 0;
  }
}

QString Board::getCapabilityStr(const Capability capability) const
{
  switch (capability) {
    case Capability::BluetoothName:
      return m_hardware.bluetoothName.c_str();
    case Capability::CPU:
      return m_hardware.cpu.c_str();
    case Capability::CPUType:
      return m_hardware.cpu_type.c_str();
    default:
      qWarning() << "Warning: unknown capability:" << capability << "returned ''";
      return QString();
  }
}

// static
QString Board::getAxisName(int index)
{
  const QString axes[] = {
    tr("Left Horizontal"),
    tr("Left Vertical"),
    tr("Right Vertical"),
    tr("Right Horizontal"),
    tr("Aux. 1"),
    tr("Aux. 2"),
  };

  if (index < (int)DIM(axes))
    return axes[index];
  else
    return CPN_STR_UNKNOWN_ITEM;
}

bool Board::isAvailable(const QString & id)
{
  return gBoardFactories->isAvailable(id);
}

//  static
QString Board::switchTypeToString(int value)
{
  switch(value) {
    case Board::SWITCH_NOT_AVAILABLE:
      return tr("None");
    case Board::SWITCH_TOGGLE:
      return tr("2 Positions Toggle");
    case Board::SWITCH_2POS:
      return tr("2 Positions");
    case Board::SWITCH_3POS:
      return tr("3 Positions");
    case Board::SWITCH_GLOBAL:
      return tr("Global");
    case Board::SWITCH_FUNC:
      return tr("Function");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * Board::switchTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARD_SWITCH_TYPE);

  for (int i = 0; i < Board::SWITCH_GLOBAL; i++) { // Function not required in lists
    mdl->appendToItemList(switchTypeToString(i), i, true, 0,
                          (i == Board::SWITCH_NOT_AVAILABLE ?
                           Board::SwitchTypeFlagNone :
                           (i < Board::SWITCH_3POS ? Board::SwitchTypeFlag2Pos : Board::SwitchTypeFlag3Pos)));
  }

  mdl->loadItemList();
  return mdl;
}

const QList<int> Board::supportedInternalModules() const
{
  QList<int> modules(m_hardware.intModules.available.begin(), m_hardware.intModules.available.end());
  modules.prepend((int)MODULE_TYPE_NONE);

  return modules;
}

#define BR(min, max, warn) vmin = min - 90; vmax = max - 120; vwarn = warn;

void Board::getBatteryRange(int & vmin, int & vmax, unsigned int & vwarn) const
{
  vmin = m_hardware.battery.min;
  vmax = m_hardware.battery.max;
  vwarn = m_hardware.battery.warn;
}

//  static
QString Board::externalModuleSizeToString(int value)
{
  switch(value) {
    case Board::EXTMODSIZE_NONE:
      return tr("None");
    case Board::EXTMODSIZE_STD:
      return tr("Standard");
    case Board::EXTMODSIZE_SMALL:
      return tr("Small");
    case Board::EXTMODSIZE_BOTH:
      return tr("Both");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
int Board::externalModuleStringToSize(const QString & value)
{
  for (int i = 0; i < Board::EXTMODSIZE_COUNT; i++) {
    if (externalModuleSizeToString(i).toLower() == value)
      return i;
  }

  return -1;
}

//  static
AbstractStaticItemModel * Board::externalModuleSizeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARD_MODULE_SIZE);

  for (int i = 0; i < Board::EXTMODSIZE_COUNT; i++) {
    mdl->appendToItemList(externalModuleSizeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString Board::flexTypeToString(int value)
{
  switch(value) {
    case Board::FLEX_NONE:
      return tr("None");
    case Board::FLEX_POT:
      return tr("Pot");
    case Board::FLEX_POT_CENTER:
      return tr("Pot with detent");
    case Board::FLEX_SLIDER:
      return tr("Slider");
    case Board::FLEX_MULTIPOS:
      return tr("Multipos Switch");
    case Board::FLEX_AXIS_X:
      return tr("Axis X");
    case Board::FLEX_AXIS_Y:
      return tr("Axis Y");
    case Board::FLEX_SWITCH:
      return tr("Switch");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

AbstractStaticItemModel * Board::flexTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARD_FLEX_TYPE);

  for (int i = 0; i < Board::FLEX_TYPE_COUNT; i++) {
    mdl->appendToItemList(flexTypeToString(i), i, true, 0,
                          (i == (int)Board::FLEX_SWITCH ?
                           Board::FlexTypeFlagSwitch :
                           Board::FlexTypeFlagNotSwitch));
  }

  mdl->loadItemList();
  return mdl;
}

const QString Board::radioModeString() const
{
  return getCapability(Capability::Air) ? tr("Flight") : tr("Drive");
}

const int Board::getInputIndex(QString val, Board::LookupValueType lvt) const
{
  for (int i = 0; i < (int)m_inputs.size(); i++) {
    if ((lvt == Board::LVT_TAG && m_inputs.at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && m_inputs.at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const QString Board::getInputName(int index) const
{
  try {
    return m_inputs.at(index).name.c_str();
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const QString Board::getInputTag(int index) const
{
  try {
    return m_inputs.at(index).tag.c_str();
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const int Board::getInputYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_inputs.size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_inputs.at(i).cfgYaml : m_inputs.at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getInputName(i) : getInputTag(i));

    if (val == tmp)
      return getInputIndex(val, type);
  }

  return -1;
}

const QString Board::getInputYamlName(int index, YamlLookupType ylt) const
{
  try {
    return ((ylt == YLT_CONFIG && m_inputs.at(index).cfgYaml == Board::LVT_NAME) ||
            m_inputs.at(index).refYaml == Board::LVT_NAME) ?
            getInputName(index) :
            getInputTag(index);
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const int Board::getInputsCalibrated() const
{
  unsigned int cnt = 0;

  for (int i = 0; i < (int)m_inputs.size(); i++) {
     cnt += (int)isInputCalibrated(i);
  }

  return cnt;
}

const int Board::getInputTagOffset(QString tag) const
{
  for (int i = 0; i < (int)m_inputs.size(); i++) {
    if (tag == m_inputs.at(i).tag.c_str())
      return i;
  }

  return -1;
}

const int Board::getInputExtIndex(int index) const
{
  if (getCapability(Capability::Pots) > 0)
    return getInputTagOffset(QString("EXT%1").arg(index));

  return -1;
}

const int Board::getInputPotIndex(int index) const
{
  if (getCapability(Capability::Pots) > 0)
    return getInputTagOffset(QString("P%1").arg(index));

  return -1;
}

const int Board::getInputSliderIndex(int index) const
{
  if (getCapability(Capability::Sliders) > 0)
    return getInputTagOffset(QString("SL%1").arg(index));

  return -1;
}

const int Board::getInputThrottleIndex() const
{
  if (getCapability(Capability::Sticks) > 0)
    return getInputTagOffset(Board::getCapability(Capability::Air) ? "RV" : "TH");

  return -1;
}

const int Board::getInputTypeOffset(Board::AnalogInputType type) const
{
  for (int i = 0; i < (int)m_inputs.size(); i++) {
    if (type == m_inputs.at(i).type)
      return i;
  }

  return -1;
}

const Board::InputInfo Board::getInputInfo(int index) const
{
  InputInfo info;

  try {
    InputDefn defn = m_inputs.at(index);
    info.type = defn.type;
    info.tag = defn.tag;
    info.name = defn.name;
    info.shortName = defn.shortName;
    info.flexType = defn.flexType;
    info.inverted = defn.inverted;
  } catch (const std::out_of_range& e) {
  }

  return info;
}

const int Board::getKeyIndex(QString key) const
{
  for (int i = 0; i < (int)m_keys.size(); i++) {
    if (m_keys.at(i).key.c_str() == key)
      return i;
  }

  return -1;
}

const Board::KeyInfo Board::getKeyInfo(int index) const
{
  KeyInfo info;

  try {
    KeyDefn defn = m_keys.at(index);
    info.name = defn.name;
    info.key = defn.key;
    info.label = defn.label;
  } catch (const std::out_of_range& e) {
  }

  return info;
}

// static
int Board::getNumericSuffix(const std::string str)
{
  std::string suffix = std::string();

  for (int i = 0; i < (int)str.size(); i++) {
    if (str.substr(i, 1) >= "0" && str.substr(i, 1) <= "9")
      suffix.append(str.substr(i, 1));
  }

  if (!suffix.empty())
    return std::stoi(suffix);

  return -1;
}

const int Board::getCFSIndexForSwitch(int swIdx) const
{
  try {
    return m_switches.at(swIdx).isCustomSwitch ? m_switches.at(swIdx).customSwitchIdx : -1;
  } catch (const std::out_of_range& e) {
  return -1;
  }
}

const int Board::getSwitchIndexForCFS(int cfsIdx) const
{
  for (int i = 0; i < (int)m_switches.size(); i++) {
    if (m_switches.at(i).isCustomSwitch && m_switches.at(i).customSwitchIdx == cfsIdx)
      return i;
  }

  return -1;
}

const int Board::getCFSOffsetForCFSIndex(const int index) const
{
  int cnt = 0;

  for (int i = 0; i < (int)m_switches.size(); i++) {
    if (m_switches.at(i).isCustomSwitch) {
      if (m_switches.at(i).customSwitchIdx == index)
        return cnt;
      else
        cnt++;
    }
  }

  return -1;
}

const int Board::getSwitchIndexForCFSOffset(const int offset) const
{
  int cnt = 0;

  for (int i = 0; i < (int)m_switches.size(); i++) {
    if (m_switches.at(i).isCustomSwitch) {
      if (cnt == offset)
        return i;
      else
        cnt++;
    }
  }

  return -1;
}

const int Board::getSwitchIndex(QString val, Board::LookupValueType lvt) const
{
  for (int i = 0; i < (int)m_switches.size(); i++) {
    if ((lvt == Board::LVT_TAG && m_switches.at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && m_switches.at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const Board::SwitchInfo Board::getSwitchInfo(int index) const
{
  Board::SwitchInfo info;

  try {
    SwitchDefn defn = m_switches.at(index);
    info.type = defn.type;
    info.tag = defn.tag;
    info.name = defn.name;
    info.dflt = defn.dflt;
    info.inverted = defn.inverted;
  } catch (const std::out_of_range& e) {
  }

  return info;
}

const QString Board::getSwitchName(int index) const
{
  try {
    return m_switches.at(index).name.c_str();
  } catch (const std::out_of_range& e) {
  return CPN_STR_UNKNOWN_ITEM;
  }
}

const QString Board::getSwitchTag(int index) const
{
  try {
    return m_switches.at(index).tag.c_str();
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const int Board::getSwitchTagNum(int index) const
{
  try {
    return getNumericSuffix(m_switches.at(index).tag.c_str());
  } catch (const std::out_of_range& e) {
    return -1;
  }
}

const int Board::getSwitchTypeOffset(Board::SwitchType type) const
{
  for (int i = 0; i < (int)m_switches.size(); i++) {
    if (type == m_switches.at(i).type)
      return i;
  }

  return -1;
}

const int Board::getSwitchYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_switches.size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_switches.at(i).cfgYaml : m_switches.at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getSwitchName(i) : getSwitchTag(i));
    if (val == tmp)
      return getSwitchIndex(val, type);
  }

  return -1;
}

const QString Board::getSwitchYamlName(int index, YamlLookupType ylt) const
{
  try {
  return ((ylt == YLT_CONFIG && m_switches.at(index).cfgYaml == Board::LVT_NAME) ||
          m_switches.at(index).refYaml == Board::LVT_NAME) ?
          getSwitchName(index) :
          getSwitchTag(index);
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const int Board::getTrimIndex(QString val, Board::LookupValueType lvt) const
{
  for (int i = 0; i < (int)m_trims.size(); i++) {
    if ((lvt == Board::LVT_TAG && m_trims.at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && m_trims.at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const QString Board::getTrimName(int index) const
{
  try {
    return m_trims.at(index).name.c_str();
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const QString Board::getTrimTag(int index) const
{
  try {
    return m_trims.at(index).tag.c_str();
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const int Board::getTrimYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_trims.size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_trims.at(i).cfgYaml : m_trims.at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getTrimName(i) : getTrimTag(i));
    if (val == tmp)
      return getTrimIndex(val, type);
  }

  return -1;
}

const QString Board::getTrimYamlName(int index, YamlLookupType ylt) const
{
  try {
    return ((ylt == YLT_CONFIG && m_trims.at(index).cfgYaml == Board::LVT_NAME) ||
            m_trims.at(index).refYaml == Board::LVT_NAME) ?
            getTrimName(index) :
            getTrimTag(index);
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const bool Board::isInputAvailable(int index) const
{
  try {
    return (m_inputs.at(index).type == Board::AIT_STICK ||
           (m_inputs.at(index).type == Board::AIT_FLEX &&
            m_inputs.at(index).flexType != Board::FLEX_NONE &&
            !isInputFlexJoystickAxis(index)));
  } catch (const std::out_of_range& e) {
    return CPN_STR_UNKNOWN_ITEM;
  }
}

const bool Board::isInputCalibrated(int index) const
{
  return (isInputStick(index) || isInputFlexPot(index) || isInputFlexSlider(index));
}

const bool Board::isInputConfigurable(int index) const
{
  return (isInputStick(index) || isInputFlexPot(index) || isInputFlexSlider(index));
}

const bool Board::isInputIgnored(int index) const
{
  return (isInputFlexJoystickAxis(index) || isInputSwitch(index));
}

const bool Board::isInputFlex(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_FLEX;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexGyroAxis(int index) const
{
  try {
    const char* val = m_inputs.at(index).tag.data();

    return (m_inputs.at(index).type == Board::AIT_FLEX && m_inputs.at(index).tag.size() > 5 &&
            val[0] == 'T' && val[1] == 'I'  && val[2] == 'L' && val[3] == 'T' && val[4] == '_' && (val[5] == 'X' || val[5] == 'Y'));
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexJoystickAxis(int index) const
{
  try {
    const char* val = m_inputs.at(index).tag.data();

    return (m_inputs.at(index).type == Board::AIT_FLEX && m_inputs.at(index).tag.size() > 2 &&
            val[0] == 'J' && val[1] == 'S' && (val[2] == 'x' || val[2] == 'y'));
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexPot(int index) const
{
  try {
    const char* val = m_inputs.at(index).tag.data();
    size_t len = m_inputs.at(index).tag.size();

    return (m_inputs.at(index).type == Board::AIT_FLEX &&
            ((len > 1 && val[0] == 'P' && val[1] >= '0' && val[1] <= '9') ||
              (len > 3 && val[0] == 'E' && val[1] == 'X' && val[2] == 'T' && val[3] >= '0' && val[3] <= '9')));
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexPotMultipos(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_FLEX && m_inputs.at(index).flexType == Board::FLEX_MULTIPOS;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexSlider(int index) const
{
  try {
    const char* val = m_inputs.at(index).tag.data();

    return (m_inputs.at(index).type == Board::AIT_FLEX && m_inputs.at(index).tag.size() > 2 &&
            val[0] == 'S' && val[1] == 'L' && val[2] >= '0' && val[2] <= '9');
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputFlexSwitch(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_FLEX && m_inputs.at(index).flexType == Board::FLEX_SWITCH;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputRTCBat(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_RTC_BAT;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputStick(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_STICK;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputSwitch(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_SWITCH;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isInputVBat(int index) const
{
  try {
    return m_inputs.at(index).type == Board::AIT_VBAT;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isSwitchConfigurable(int index) const
{
  try {
    if (index >= 0 && index < getCapability(Capability::Switches)) {
      if (isSwitchStd(index) || isSwitchFunc(index))
        return true;

      if (isSwitchFlex(index)) {
        int sfx = getNumericSuffix(m_switches.at(index).tag);
        if (sfx > 0 && sfx <= getCapability(Capability::FlexSwitches))
          return true;
      }
    }
  } catch (const std::out_of_range& e) {
  }

  return false;
}

const bool Board::isSwitchStd(int index) const
{
  return !(isSwitchFlex(index) || isSwitchFunc(index));
}

const bool Board::isSwitchFlex(int index) const
{
  try {
    const char* val = m_switches.at(index).tag.data();

    return (m_switches.at(index).tag.size() > 2 &&
            val[0] == 'F' && val[1] == 'L' && val[2] >= '0' && val[2] <= '9');
  } catch (const std::out_of_range& e) {
    return false;
  }
}

const bool Board::isSwitchFunc(int index) const
{
  try {
    return m_switches.at(index).customSwitchIdx >= 0;
  } catch (const std::out_of_range& e) {
    return false;
  }
}

bool Board::loadDefinition()
{
  if (m_loaded)
    return true;

  // safety net for BoardFactory::instance
  if (m_id == Board::BOARD_UNKNOWN)
    return true;

  // required because of the way the Firmware class is used
  if (m_hwdefn.isEmpty()) {
    return false;
  }

  // load default.json first and allow subsequent file values to override
  // this avoids having to include default in basedOn tree
  if (loadDefinition(QString("%1/%2.json").arg(BDDEFNSDIR).arg("default"))) {
    if (loadDefinition(QString("%1/%2.json").arg(HWDEFNSDIR).arg(m_hwdefn))) {
      if (loadDefinition(QString("%1/%2.json").arg(BDDEFNSDIR).arg(m_hwdefn))) {
        qDebug() << "Definition loaded:" << m_id;
      } else
        return false;
    } else
      return false;
  } else
    return false;

  postLoadFixups();

  setInputCounts();
  setSwitchCounts();

  qDebug() << "Board:" << getName() <<
              "inputs:" << getCapability(Capability::Inputs) <<
              "sticks:" << getCapability(Capability::Sticks) <<
              "pots:" << getCapability(Capability::Pots) <<
              "sliders:" << getCapability(Capability::Sliders) <<
              "gyro axes:" << getCapability(Capability::GyroAxes) <<
              "joystick axes:" << getCapability(Capability::JoystickAxes) <<
              "flex inputs:" << getCapability(Capability::FlexInputs) <<
              "input switches:" << getCapability(Capability::InputSwitches) <<
              "trims:" << getCapability(Capability::NumTrims) <<
              "keys:" << getCapability(Capability::Keys) <<
              "std switches:" << getCapability(Capability::StandardSwitches) <<
              "flex switches:" << getCapability(Capability::FlexSwitches) <<
              "func switches:" << getCapability(Capability::FunctionSwitches) <<
              "rtcbat:" << getCapability(Capability::HasRTC) <<
              "vbat:" << getCapability(Capability::HasVBat);

  m_loaded = true;
  return true;
}

bool Board::loadDefinition(const QString & path)
{
  bool success = true;
  QJsonDocument *doc = new QJsonDocument();
  QJsonObject o;
  QStringList depends;

  if (load(doc, path)) {
    if (doc->isObject()) {
      o = doc->object();

      // hwdefs are flat so not used
      if (isArray(o,"basedOn")) {
        QJsonArray a = o.value("basedOn").toArray();

        for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
          if ((*it).isString()) {
            QString p = QString("%1/%2.json").arg(QFileInfo(path).path()).arg((*it).toString());

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
  } else
    success = false;

  if (!success) {
    qCritical() << "CRITICAL: Load definition" << path << "unsuccessful";
    delete doc;
    return false;
  }

  qDebug() << "loading values from:" << path;

  for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
    qDebug() << "key:" << it.key() << "value:" << it.value();

    // skips first to save processing time and no unknown key warning messages
    if (it.key() == "hidden" || it.key() == "basedOn" ||
        it.key() == "timers" || it.key() == "haptic")
      continue;

    else if (it.key() == "adc_inputs")
      loadADCInputs(it);

    else if (it.key() == "switches")
      loadSwitches(it);

    else if (it.key() == "trims")
      loadTrims(it);

    else if (it.key() == "keys")
      loadKeys(it);

    else if (it.key() == "display")
      loadDisplay(it);

    else if (it.key() == "hardware")
      loadHardware(it);

    else if (it.key() == "leds")
      loadLEDS(it);

    else if (it.key() == "backlight")
      m_display.backlight_color = getValueBool(it, m_display.backlight_color);

    else if (it.key() == "key_lock_combo")
      m_hardware.hasKeyLockCombo = (it->isArray() && it->toArray().size() == 2 ? true : m_hardware.hasKeyLockCombo);

    else if (it.key() == "manufacturer")
      m_manufacturer = getValueString(it, m_manufacturer);

    else if (it.key() == "battery")
      loadBattery(it);

    else if (it.key() == "backlightLevelMin")
      m_hardware.backlightLevelMin = getValueBool(it, m_hardware.backlightLevelMin);

    else if (it.key() == "auxSerialMode")
      m_hardware.auxSerialMode = getValueBool(it, m_hardware.auxSerialMode);

    else if (it.key() == "aux2SerialMode")
      m_hardware.aux2SerialMode = getValueBool(it, m_hardware.aux2SerialMode);

    else if (it.key() == "bluetooth")
      m_hardware.bluetooth = getValueBool(it, m_hardware.bluetooth);

    else if (it.key() == "externalAntenna")
      m_hardware.externalAntenna = getValueBool(it, m_hardware.externalAntenna);

    else if (it.key() == "hardwareAntennaSwitch")
      m_hardware.hardwareAntennaSwitch = getValueBool(it, m_hardware.hardwareAntennaSwitch);

    else if (it.key() == "imu")
      m_hardware.imu = getValueBool(it, m_hardware.imu);

    else if (it.key() == "internalGPS")
      m_hardware.internalGPS = getValueBool(it, m_hardware.internalGPS);

    else if (it.key() == "softwareSerialPower")
      m_hardware.softwareSerialPower = getValueBool(it, m_hardware.softwareSerialPower);

    else if (it.key() == "switchableJack")
      m_hardware.switchableJack = getValueBool(it, m_hardware.switchableJack);

    else if (it.key() == "trainerModule") {
      int idx = DataHelpers::getStringTagMappingIndex(trainerModuleLookupTable,
                                                      getValueStdString(it).c_str());
      m_hardware.trainerModule = idx > -1 ? idx : m_hardware.trainerModule;
    }
    else if (it.key() == "vcpSerialMode")
      m_hardware.vcpSerialMode = getValueBool(it, m_hardware.vcpSerialMode);

    else if (it.key() == "contrast")
      loadContrast(it);

    else if (it.key() == "maxVolume")
      m_hardware.maxVolume = getValueInt(it, m_hardware.maxVolume);

    else if (it.key() == "pwrButtonPress")
      m_hardware.pwrButtonPress = getValueBool(it, m_hardware.pwrButtonPress);

    else if (it.key() == "rotaryEncNav")
      m_hardware.rotaryEncNav = getValueBool(it, m_hardware.rotaryEncNav);

    else if (it.key() == "intModules")
      loadIntModules(it);

    else if (it.key() == "extModules") {
      loadExtModules(it);
    }
    else if (it.key() == "fourCC")
      m_hardware.fourCC = getValueInt(it, m_hardware.fourCC);

    else if (it.key() == "eepromSize")
      m_hardware.eepromSize = getValueInt(it, m_hardware.eepromSize);

    else if (it.key() == "flashSize")
      m_hardware.flashSize = getValueInt(it, m_hardware.flashSize);

    else
      qWarning() << "Warning: No rule to process - path:" << path << "name:" << it.key() << "value:" << it.value();
  }

  delete doc;
  return true;
}

void Board::loadADCInputs(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    const QJsonObject &o = oit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "acds")
        continue;
      else if (it.key() == "inputs")
        loadInputs(it);
      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: adc_inputs is not an object" << *oit;
}

void Board::loadBattery(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    const QJsonObject &o = oit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "min")
        m_hardware.battery.min = getValueInt(it, m_hardware.battery.min);
      else if (it.key() == "max")
        m_hardware.battery.max = getValueInt(it, m_hardware.battery.max);
      else if (it.key() == "warn")
        m_hardware.battery.warn = getValueInt(it, m_hardware.battery.warn);
      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: battery is not an object" << *oit;
}

void Board::loadContrast(QJsonObject::const_iterator & oit)
{
  if (oit->isObject()) {
    const QJsonObject &o = oit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "min")
        m_hardware.contrast.min = getValueInt(it, m_hardware.contrast.min);
      else if (it.key() == "max")
        m_hardware.contrast.max = getValueInt(it, m_hardware.contrast.max);
      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: contrast is not an object" << *oit;
}

void Board::loadInputs(QJsonObject::const_iterator & oit)
{
  if (oit->isArray()) {
    const QJsonArray &a = oit->toArray();

    for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
      if (it->isObject()) {
        const QJsonObject &o = it->toObject();
        InputDefn defn;

        for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
          //qDebug() << "key:" << it.key() << "value:" << it.value();
          if (it.key() == "name")
            defn.name = getValueStdString(it, defn.name);

          else if (it.key() == "type") {
            std::string type = getValueStdString(it);
            defn.type = (Board::AnalogInputType)DataHelpers::getStringTagMappingIndex(inputTypesLookupTable, type.c_str());
            if (defn.type == Board::AIT_STICK)
              defn.refYaml = Board::LVT_NAME;
          }

          else if (it.key() == "inverted")
            defn.inverted = getValueBool(it, defn.inverted);

          else if (it.key() == "label")
            defn.name = getValueStdString(it, defn.name);

          else if (it.key() == "short_label")
            defn.shortName = getValueStdString(it, defn.shortName);

          else if (it.key() == "default") {
            std::string dflt = getValueStdString(it);

            if (defn.type == Board::AIT_FLEX) {
              int idx = DataHelpers::getStringTagMappingIndex(flexTypesLookupTable, dflt.c_str());
              defn.flexType = (Board::FlexType)(idx < 0 ? Board::FLEX_NONE : idx);
            }
          }

          else
            qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
        }

        m_inputs.insert(m_inputs.end(), defn);
      } else
        qWarning() << "Warning: input is not an object" << *it;
    }
  } else
        qWarning() << "Warning: inputs is not an array" << *oit;
}

void Board::loadExtModules(QJsonObject::const_iterator & it)
{
  loadModules(it, m_hardware.extModules);
}

void Board::loadIntModules(QJsonObject::const_iterator & it)
{
  loadModules(it, m_hardware.intModules);
}

void Board::loadModules(QJsonObject::const_iterator & oit, Board::Modules & modules)
{
  if (oit->isObject()) {
    const QJsonObject &o = oit->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      if (it.key() == "available") {
        if (it.value().isArray()) {
          for (const auto &mod : it.value().toArray()) {
            int idx = DataHelpers::getStringTagMappingIndex(intModuleTypesLookupTable,
                                                            mod.toString().toLatin1().constData());
            if (idx > -1) {
              modules.available.push_back(idx);
            } else {
              qWarning() << "Warning: unsupported module:" << mod.toString();
            }
          }
        }
      }
      else if (it.key() == "dflt") {
        int idx = DataHelpers::getStringTagMappingIndex(intModuleTypesLookupTable,
                                                        getValueStdString(it).c_str());
        modules.dflt = idx > -1 ? idx : MODULE_TYPE_NONE;
      }
      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: intModules is not an object" << *oit;
}

void Board::loadSwitches(QJsonObject::const_iterator & oit)
{
  if (oit->isArray()) {
    const QJsonArray &a = oit->toArray();

    for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
      if (it->isObject()) {
        const QJsonObject &o = it->toObject();
        SwitchDefn sw;

        for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
          //qDebug() << "key:" << it.key() << "value:" << it.value();
          if (it.key() == "name")
            sw.name = getValueStdString(it, sw.name);

          else if (it.key() == "type") {
            int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable,
                                                            getValueStdString(it).c_str());
            sw.type = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
          }

          else if (it.key() == "flags")
            sw.flags = o.value("flags").toInt();

          else if (it.key() == "inverted")
            sw.inverted = getValueBool(it);

          else if (it.key() == "default") {
            int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable,
                                                            getValueStdString(it).c_str());
            sw.dflt = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
          }

          else if (it.key() == "display") {
            if (it->isArray()) {
              const QJsonArray &d = it->toArray();

              if (d.size() >= 2) {
                if (d.at(0).isDouble())
                  sw.display.x = (unsigned int)d.at(0).toInt(0);
                if (d.at(1).isDouble())
                  sw.display.y = (unsigned int)d.at(1).toInt(0);
              }
            }
          }

          else if (it.key() == "is_cfs")
            sw.isCustomSwitch = getValueBool(it, sw.isCustomSwitch);

          else if (it.key() == "cfs_idx")
            sw.customSwitchIdx = getValueBool(it, sw.customSwitchIdx);

          else
            qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
        }

        // special handing for ADC
        if (sw.type == Board::SWITCH_ADC) {
          if (sw.dflt == Board::SWITCH_TOGGLE) {
            // this could be 2 or 3 position toggle so play safe
            // it therefore has an impact on configuring hardware, available switches, simulator widget, yaml encode and decode
            sw.dflt = Board::SWITCH_3POS;
          }
          // make the same
          sw.type = sw.dflt;
        }

        sw.tag = sw.name;
        m_switches.insert(m_switches.end(), sw);
      } else
        qWarning() << "Warning: switch is not an object" << *it;
    }
  } else
        qWarning() << "Warning: switches is not an array" << *oit;
}

void Board::loadKeys(QJsonObject::const_iterator & oit)
{
  if (oit->isArray()) {
    const QJsonArray &a = oit->toArray();

    for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
      if (it->isObject()) {
        const QJsonObject &o = it->toObject();
        KeyDefn k;

        for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
          //qDebug() << "key:" << it.key() << "value:" << it.value();
          if (it.key() == "name")
            k.name = getValueStdString(it, k.name);

          else if (it.key() == "key")
            k.key = getValueStdString(it, k.key);

          else if (it.key() == "label")
            k.label = getValueStdString(it, k.label);

          else
            qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
        }

        k.tag = k.name;
        m_keys.insert(m_keys.end(), k);
      } else
        qWarning() << "Warning: key is not an object" << *it;
    }
  } else
        qWarning() << "Warning: keys is not an array" << *oit;
}

void Board::loadTrims(QJsonObject::const_iterator & oit)
{
  if (oit->isArray()) {
    const QJsonArray &a = oit->toArray();

    for (QJsonArray::const_iterator it = a.constBegin(); it != a.constEnd(); ++it) {
      if (it->isObject()) {
        const QJsonObject &o = it->toObject();
        TrimDefn t;

        for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
          //qDebug() << "key:" << it.key() << "value:" << it.value();
          if (it.key() == "name")
            t.name = getValueStdString(it, t.name);

          else
            qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
        }

        t.tag = t.name;
        m_trims.insert(m_trims.end(), t);
      } else
        qWarning() << "Warning: trim is not an object" << *it;
    }
  } else
        qWarning() << "Warning: trims is not an array" << *oit;
}

void Board::loadDisplay(QJsonObject::const_iterator & it)
{
  if (it->isObject()) {
    const QJsonObject &o = it->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      //qDebug() << "key:" << it.key() << "value:" << it.value();
      if (it.key() == "lcd_depth")
        m_display.depth = getValueInt(it, m_display.depth);

      else if (it.key() == "lcd_h")
        m_display.h = getValueInt(it, m_display.h);

      else if (it.key() == "lcd_w")
        m_display.w = getValueInt(it, m_display.w);

      else if (it.key() == "lcd_phys_h")
        m_display.phys_h = getValueInt(it, m_display.phys_h);

      else if (it.key() == "lcd_phys_w")
        m_display.phys_w = getValueInt(it, m_display.phys_w);

      else if (it.key() == "oled_screen")
        m_display.oled = getValueBool(it, m_display.oled);

      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }

    m_display.color = m_display.depth == 16 ? 1 : 0;

  } else
        qWarning() << "Warning: display is not an object" << *it;
}

void Board::loadLEDS(QJsonObject::const_iterator & it)
{
  if (it->isObject()) {
    const QJsonObject &o = it->toObject();

    int cfs_led_strip_length = 0;
    int cfs_leds_per_switch = 0;

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      //qDebug() << "key:" << it.key() << "value:" << it.value();
      if (it.key() == "bling_led_strip_length")
        m_hardware.has_bling_leds = getValueInt(it, m_hardware.has_bling_leds);

      else if (it.key() == "cfs_leds_per_switch")
        cfs_leds_per_switch = getValueInt(it, 0);

      else if (it.key() == "cfs_led_strip_length")
        cfs_led_strip_length = getValueInt(it, 0);

      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }

    m_cfs.groups = cfs_leds_per_switch ? cfs_led_strip_length / (2 * cfs_leds_per_switch) : 0;
    m_cfs.rgb_led = m_cfs.groups > 0;

  } else
        qWarning() << "Warning: leds is not an object" << *it;
}

void Board::loadHardware(QJsonObject::const_iterator & it)
{
  if (it->isObject()) {
    const QJsonObject &o = it->toObject();

    for (QJsonObject::const_iterator it = o.constBegin(); it != o.constEnd(); ++it) {
      //qDebug() << "key:" << it.key() << "value:" << it.value();
      if (it.key() == "has_audio_mute")
        m_hardware.has_audio_mute = getValueBool(it, m_hardware.has_audio_mute);

      else if (it.key() == "has_ext_module_support")
        m_hardware.has_ext_module_support = getValueBool(it, m_hardware.has_ext_module_support);

      else if (it.key() == "has_int_module_support")
        m_hardware.has_int_module_support = getValueBool(it, m_hardware.has_int_module_support);

      else if (it.key() == "sport_max_baudrate")
        m_hardware.sport_max_baudrate = getValueInt(it, m_hardware.sport_max_baudrate);

      else if (it.key() == "surface")
        m_hardware.surface = getValueBool(it, m_hardware.surface);

      else if (it.key() == "cpu")
        m_hardware.cpu = getValueStdString(it, m_hardware.cpu);

      else if (it.key() == "cpu_type")
        m_hardware.cpu_type = getValueStdString(it, m_hardware.cpu_type);

      else
        qWarning() << "Warning: No rule to process - key:" << it.key() << "value:" << it.value();
    }
  } else
        qWarning() << "Warning: hardware is not an object" << *it;
}

void Board::postLoadFixups()
{
  // Set default labels for LUX inputs if not provided by JSON
  for (auto &defn : m_inputs) {
    if (defn.type == Board::AIT_LUX) {
      if (defn.name.empty())
        defn.name = "Ambient light";
      if (defn.shortName.empty())
        defn.shortName = "Light";
    }
  }

  // json files do not normally specify stick labels so load legacy labels
  for (auto &defn : m_inputs) {
    if (defn.type == Board::AIT_STICK && defn.name.empty())
      defn.name = DataHelpers::getStringTagMappingName(stickNamesLookupTable, defn.tag.c_str());
  }
}

void Board::setInputCounts()
{
  for (int i = 0; i < (int)m_inputs.size(); i++) {
    if (isInputStick(i))
      m_inputCnt.sticks++;
    else if (isInputFlexPot(i))
      m_inputCnt.flexPots++;
    else if (isInputFlexSlider(i))
      m_inputCnt.flexSliders++;
    else if (isInputFlexGyroAxis(i))
      m_inputCnt.flexGyroAxes++;
    else if (isInputFlexJoystickAxis(i))
      m_inputCnt.flexJoystickAxes++;
    else if (isInputFlexSwitch(i))
      m_inputCnt.flexSwitches++;
    else if (isInputRTCBat(i))
      m_inputCnt.rtcbat++;
    else if (isInputVBat(i))
      m_inputCnt.vbat++;
    else if (isInputSwitch(i))
      m_inputCnt.switches++;
  }
}

void Board::setSwitchCounts()
{
  for (int i = 0; i < (int)m_switches.size(); i++) {
    if (isSwitchStd(i))
      m_switchCnt.std++;
    else if (isSwitchFlex(i))
      m_switchCnt.flex++;
    else if (isSwitchFunc(i))
      m_switchCnt.func++;
  }
}
