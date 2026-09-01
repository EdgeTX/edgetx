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

#include "boards.h"
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

static const StringTagMappingTable switchTypesLookupTable = {
    {std::to_string(Board::SWITCH_NOT_AVAILABLE), "NONE"},
    {std::to_string(Board::SWITCH_TOGGLE),        "TOGGLE"},
    {std::to_string(Board::SWITCH_2POS),          "2POS"},
    {std::to_string(Board::SWITCH_3POS),          "3POS"},
    {std::to_string(Board::SWITCH_ADC),           "ADC"},
};

static const StringTagMappingTable stickNamesLookupTable = {
    {QCoreApplication::translate("Boards", "Rud").toStdString(), "LH"},  // air
    {QCoreApplication::translate("Boards", "Ele").toStdString(), "LV"},  // air
    {QCoreApplication::translate("Boards", "Thr").toStdString(), "RV"},  // air
    {QCoreApplication::translate("Boards", "Ail").toStdString(), "RH"},  // air
    {QCoreApplication::translate("Boards", "ST").toStdString(),  "ST"},  // surface
    {QCoreApplication::translate("Boards", "TH").toStdString(),  "TH"},  // surface
};

Boards::Boards(const Board::Type & board, const QString & hwdefn, const QString & bddefn) :
  JsonBase(),
  m_id(board),
  m_hwdefn(hwdefn),
  m_bddefn(bddefn),
  m_inputs(new InputsTable),
  m_switches(new SwitchesTable),
  m_trims(new TrimsTable),
  m_keys(new KeysTable),
  m_display(new DisplayDefn),
  m_cfs(new CustomSwitchesDefn),
  m_hardware(new HardwareDefn),
  m_hwextra(new BoardDefn),
  m_inputCnt({0, 0, 0, 0, 0, 0, 0, 0, 0}),
  m_switchCnt({0, 0, 0}),
  legacyTrimSourcesLookupTable(legacyTrimSourcesLut),
  trimSwitchesLookupTable(trimSwitchesLut),
  rawSwitchTypesLookupTable(RawSwitch::getRawSwitchTypesLookupTable()),
  rawSourceSpecialTypesLookupTable(RawSource::getSpecialTypesLookupTable())
{
}

Boards::~Boards()
{
  delete m_inputs;
  delete m_switches;
  delete m_trims;
  delete m_keys;
  delete m_display;
  delete m_cfs;
  delete m_hardware;
  delete m_hwextra;
}

// static
int Boards::getCapability(const Board::Type & id, const Capability capability)
{
  // TODO investigate usage of any that should be covered in Boards::getCapability or are no longer required
  //      some could be used when importing pre v2.10 configurations
  auto board = getBoard(id);

  switch (capability) {
    case Capability::BacklightLevelMin:
      return board->m_hwextra->backlightLevelMin;

    case Capability::HasAuxSerialMode:
      return board->m_hwextra->auxSerialMode;

    case Capability::HasAux2SerialMode:
      return board->m_hwextra->aux2SerialMode;

    case Capability::HasBluetooth:
      return board->m_hwextra->bluetooth;

    case Capability::HasExternalAntenna:
      return board->m_hwextra->externalAntenna;

    case Capability::HasHardwareAntennaSwitch:
      return board->m_hwextra->hardwareAntennaSwitch;

    case Capability::HasIMU:
      return board->m_hwextra->imu;

    case HasInternalGPS:
      return board->m_hwextra->internalGPS;

    case Capability::HasSoftwareSerialPower:
      return board->m_hwextra->softwareSerialPower;

    case Capability::HasSwitchableJack:
      return board->m_hwextra->switchableJack;

    case Capability::HasTrainerModuleCPPM:
      return board->m_hwextra->trainerModuleCPPM;

    case Capability::HasTrainerModuleSBUS:
      return board->m_hwextra->trainerModuleSBUS;

    case Capability::HasVCPSerialMode:
      return board->m_hwextra->vcpSerialMode;

    case Capability::MaxContrast:
      return board->m_hwextra->contrast.max;

    case Capability::MaxVolume:
      return board->m_hwextra->maxVolume;

    case Capability::MinContrast:
      return board->m_hwextra->contrast.min;

    case Capability::PwrButtonPress:
      return board->m_hwextra->pwrButtonPress;

    case Capability::RotaryEncoderNavigation:
      return board->m_hwextra->rotaryEncoderNavigation;

    case Capability::Air:
      return !board->m_hardware->surface;

    case Capability::FlexInputs:
      return (board->m_inputCnt.flexGyroAxes +
              board->m_inputCnt.flexJoystickAxes +
              board->m_inputCnt.flexPots +
              board->m_inputCnt.flexSliders +
              board->m_inputCnt.flexSwitches);

    case Capability::FlexSwitches:
      return board->m_switchCnt.flex;

    case Capability::FunctionSwitchColors:
      return board->m_cfs->rgb_led;

    case Capability::FunctionSwitches:
      return board->m_switchCnt.func;

    case Capability::FunctionSwitchGroups:
      return board->m_cfs->groups;

    case Capability::GyroAxes:
      return board->m_inputCnt.flexGyroAxes;

    case Capability::Gyros:
      return board->getCapability(Capability::GyroAxes) / 2;

    case Capability::HasAudioMuteGPIO:
      return board->m_hardware->has_audio_mute;

    case Capability::HasBacklightColor:
      return board->m_display->backlight_color;

    case Capability::HasBlingLEDS:
      return board->m_hardware->has_bling_leds;

    case Capability::HasColorLcd:
      return board->m_display->color;

    case Capability::HasExternalModuleSupport:
      return board->m_hardware->has_ext_module_support;

    case Capability::HasInternalModuleSupport:
      return board->m_hardware->has_int_module_support;

    case Capability::HasRTC:
      return board->m_inputCnt.rtcbat;

    case Capability::HasSDCard:
      return true;

    case Capability::HasVBat:
      return board->m_inputCnt.vbat;

    case Capability::Inputs:
      return board->m_inputs->size();

    case Capability::InputSwitches:
      return board->m_inputCnt.switches;

    case Capability::IsF4:
      return board->m_hardware->cpu_type == "STM32F4";

    case Capability::IsH5:
      return board->m_hardware->cpu_type == "STM32H5";

    case Capability::IsH7:
      return board->m_hardware->cpu_type == "STM32H7";

    case Capability::JoystickAxes:
      return board->m_inputCnt.flexJoystickAxes;

    case Capability::Keys:
      return board->m_keys->size();

    case Capability::LcdDepth:
      return board->m_display->depth;

    case Capability::LcdHeight:
      return board->m_display->h;

    case Capability::LcdOLED:
      return board->m_display->oled;

    case Capability::LcdWidth:
      return board->m_display->w;

    case Capability::MultiposPots:
      // assumes every input has potential to be one
      // index used for mapping 6 pos switches back to input
      return board->getCapability(Capability::Inputs);

    case Capability::MultiposPotsPositions:
      return 6;

    case Capability::NumFunctionSwitchesPositions:
       return board->getCapability(Capability::FunctionSwitches) * 3;

    case Capability::NumTrims:
      return board->m_trims->size();

    case Capability::NumTrimSwitches:
      return board->getCapability(Capability::NumTrims) * 2;

    case Capability::Pots:
      return board->m_inputCnt.flexPots;

    case Capability::Sliders:
      return board->m_inputCnt.flexSliders;

    case Capability::SportMaxBaudRate:
      return board->m_hardware->sport_max_baudrate;

    case Capability::StandardSwitches:
      return board->m_switchCnt.std;

    case Capability::Sticks:
      return board->m_inputCnt.sticks;

    case Capability::Surface:
      return board->m_hardware->surface;

    case Capability::Switches:
      return (board->m_switchCnt.std +
              board->m_switchCnt.flex +
              board->m_switchCnt.func);

    case Capability::SwitchesPositions:
      return board->getCapability(Capability::Switches) * 3;

    default:
      qWarning() << "Warning: no value returned for:" << capability;
      return 0;
  }
}

QString Boards::getCapabilityStr(const Board::Type & id, const Capability capability)
{
  auto board = getBoard(id);

  switch (capability) {
    case Capability::CPU:
      return board->m_hardware->cpu.c_str();
    case Capability::CPUType:
      return board->m_hardware->cpu_type.c_str();
    default:
      return QString();
  }
}

QString Boards::getAxisName(int index)
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

bool Boards::isBoardCompatible(Board::Type board1, Board::Type board2)
{
  return (getBoard(board1)->getFourCC() == getBoard(board2)->getFourCC());
}

QString Boards::getBoardName(Board::Type board)
{
  return getBoard(board)->name();
}

//  static
QString Boards::switchTypeToString(int value)
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
AbstractStaticItemModel * Boards::switchTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_SWITCH_TYPE);

  for (int i = 0; i < Board::SWITCH_GLOBAL; i++) { // Function not required in lists
    mdl->appendToItemList(switchTypeToString(i), i, true, 0,
                          (i == Board::SWITCH_NOT_AVAILABLE ?
                           Board::SwitchTypeFlagNone :
                           (i < Board::SWITCH_3POS ? Board::SwitchTypeFlag2Pos : Board::SwitchTypeFlag3Pos)));
  }

  mdl->loadItemList();
  return mdl;
}

QList<int> Boards::getSupportedInternalModules(Board::Type board)
{
  QList<int> modules;
  modules.append((int)MODULE_TYPE_NONE);
  if (IS_TARANIS_X9DP_2019(board) || IS_TARANIS_X7_ACCESS(board)) {
    modules.append((int)MODULE_TYPE_ISRM_PXX2);
  } else if (IS_FLYSKY_NV14(board)) {
    modules.append((int)MODULE_TYPE_FLYSKY_AFHDS2A);
  } else if (IS_FLYSKY_EL18(board)) {
    modules.append((int)MODULE_TYPE_FLYSKY_AFHDS3);
    modules.append((int)MODULE_TYPE_CROSSFIRE);
  } else if (IS_RADIOMASTER_MT12(board)) {
    modules.append((int)MODULE_TYPE_CROSSFIRE);
    modules.append((int)MODULE_TYPE_MULTIMODULE);
  } else if (IS_FAMILY_HORUS_OR_T16(board) || IS_FAMILY_T12(board) ||
             (IS_TARANIS_SMALL(board) && IS_ACCESS_RADIO(board))) {
    modules.append((int)MODULE_TYPE_XJT_PXX1);
    modules.append((int)MODULE_TYPE_ISRM_PXX2);
    modules.append((int)MODULE_TYPE_CROSSFIRE);
    modules.append((int)MODULE_TYPE_MULTIMODULE);
  } else if (IS_TARANIS(board)) {
    modules.append((int)MODULE_TYPE_XJT_PXX1);
  }

  return modules;
}

int Boards::getDefaultInternalModules(Board::Type board)
{
  switch(board) {
  case BOARD_TARANIS_X7:
  case BOARD_TARANIS_X9D:
  case BOARD_TARANIS_X9DP:
  case BOARD_TARANIS_X9E:
  case BOARD_HORUS_X12S:
  case BOARD_X10:
  case BOARD_TARANIS_XLITE:
    return (int)MODULE_TYPE_XJT_PXX1;

  case BOARD_TARANIS_X7_ACCESS:
  case BOARD_TARANIS_X9DP_2019:
  case BOARD_X10_EXPRESS:
  case BOARD_TARANIS_XLITES:
  case BOARD_TARANIS_X9LITE:
  case BOARD_TARANIS_X9LITES:
    return (int)MODULE_TYPE_ISRM_PXX2;

  case BOARD_JUMPER_T12:
  case BOARD_JUMPER_T16:
  case BOARD_RADIOMASTER_TX16S:
  case BOARD_JUMPER_T18:
  case BOARD_RADIOMASTER_TX12:
  case BOARD_RADIOMASTER_T8:
  case BOARD_JUMPER_TLITE:
  case BOARD_JUMPER_TLITE_F4:
  case BOARD_JUMPER_TPRO:
  case BOARD_JUMPER_TPROV2:
  case BOARD_FLYSKY_PL18:
  case BOARD_FLYSKY_PL18EV:
  case BOARD_FLYSKY_NB4P:
    return (int)MODULE_TYPE_MULTIMODULE;

  case BOARD_BETAFPV_LR3PRO:
  case BOARD_FATFISH_F16:
  case BOARD_HELLORADIOSKY_V12:
  case BOARD_HELLORADIOSKY_V14:
  case BOARD_HELLORADIOSKY_V14LCD:
  case BOARD_HELLORADIOSKY_V16:
  case BOARD_RADIOMASTER_TX15:
  case BOARD_RADIOMASTER_GX15:
  case BOARD_RADIOMASTER_TX16SMK3:
  case BOARD_IFLIGHT_COMMANDO8:
  case BOARD_IFLIGHT_COMMANDO14:
  case BOARD_JUMPER_BUMBLEBEE:
  case BOARD_JUMPER_T12MAX:
  case BOARD_JUMPER_T14:
  case BOARD_JUMPER_T15:
  case BOARD_JUMPER_T15PRO:
  case BOARD_JUMPER_T22:
  case BOARD_JUMPER_T20:
  case BOARD_JUMPER_TPROS:
  case BOARD_JUMPER_T20V2:
  case BOARD_RADIOMASTER_BOXER:
  case BOARD_RADIOMASTER_GX12:
  case BOARD_RADIOMASTER_MT12:
  case BOARD_RADIOMASTER_POCKET:
  case BOARD_RADIOMASTER_TX12_MK2:
  case BOARD_RADIOMASTER_ZORRO:
    return (int)MODULE_TYPE_CROSSFIRE;

  case BOARD_FLYSKY_NV14:
    return (int)MODULE_TYPE_FLYSKY_AFHDS2A;

  case BOARD_FLYSKY_EL18:
  case BOARD_FLYSKY_PL18U:
  case BOARD_FLYSKY_PA01: // ANT
  case BOARD_FLYSKY_ST16: // ANT
    return (int)MODULE_TYPE_FLYSKY_AFHDS3;

  default:
    return (int)MODULE_TYPE_NONE;
  }
}

#define BR(min, max, warn) vmin = min - 90; vmax = max - 120; vwarn = warn;

void Boards::getBattRange(const Board::Type & boardType, int & vmin, int & vmax, unsigned int & vwarn)
{
  auto bd = gBoardFactories->board(boardType);
  vmin = bd->m_hwextra->battery.min;
  vmax = bd->m_hwextra->battery.max;
  vwarn = bd->m_hwextra->battery.warn;
}

// static
int Boards::getDefaultExternalModuleSize(Board::Type board)
{
  if (!getCapability(board, HasExternalModuleSupport))
    return Board::EXTMODSIZE_NONE;

  if (getCapability(board, HasColorLcd)) {
    if (IS_FLYSKY_EL18(board))
      return Board::EXTMODSIZE_BOTH;
    else if (IS_HELLORADIOSKY_V12(board))
      return Board::EXTMODSIZE_SMALL;
    else
      return Board::EXTMODSIZE_STD;
  }

  if (IS_TARANIS_X9LITE(board)     ||
      IS_RADIOMASTER_ZORRO(board)  ||
      IS_RADIOMASTER_MT12(board)   ||
      IS_RADIOMASTER_POCKET(board) ||
      IS_JUMPER_TLITE(board)       ||
      IS_JUMPER_TPRO(board)        ||
      IS_JUMPER_T20(board)         ||
      IS_JUMPER_BUMBLEBEE(board)  ||
      IS_BETAFPV_LR3PRO(board))
    return Board::EXTMODSIZE_SMALL;

  return Board::EXTMODSIZE_STD;
}

//  static
QString Boards::externalModuleSizeToString(int value)
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
AbstractStaticItemModel * Boards::externalModuleSizeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_MODULE_SIZE);

  for (int i = 0; i < Board::EXTMODSIZE_COUNT; i++) {
    mdl->appendToItemList(externalModuleSizeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString Boards::flexTypeToString(int value)
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

AbstractStaticItemModel * Boards::flexTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_FLEX_TYPE);

  for (int i = 0; i < Board::FLEX_TYPE_COUNT; i++) {
    mdl->appendToItemList(flexTypeToString(i), i, true, 0,
                          (i == (int)Board::FLEX_SWITCH ?
                           Board::FlexTypeFlagSwitch :
                           Board::FlexTypeFlagNotSwitch));
  }

  mdl->loadItemList();
  return mdl;
}

Boards* Boards::getBoard(const Board::Type & boardType)
{
  return gBoardFactories->board(boardType == Board::BOARD_UNKNOWN ? getCurrentBoard() : boardType);
}

QString Boards::getRadioModeString(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Capability::Air) ? tr("Flight") : tr("Drive");
}

bool Boards::isAir(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Capability::Air);
}

bool Boards::isSurface(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Capability::Surface);
}

// temporary until boards refactored
// only called from Firmware::getBoard()
Board::Type Boards::getBoardForHwDefn(const QString & hwdefn)
{
  return gBoardFactories->board(hwdefn)->boardType();
}

const int Boards::getInputIndex(const QString val, Board::LookupValueType lvt) const
{
  return getInputIndex(m_inputs, val, lvt);
}

// static
int Boards::getInputIndex(const InputsTable * inputs, QString val, Board::LookupValueType lvt)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if ((lvt == Board::LVT_TAG && inputs->at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && inputs->at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const QString Boards::getInputName(int index) const
{
  return getInputName(m_inputs, index);
}

// static
QString Boards::getInputName(const InputsTable * inputs, int index)
{
  if (index > -1 && index < (int)inputs->size())
    return inputs->at(index).name.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const QString Boards::getInputTag(int index) const
{
  return getInputTag(m_inputs, index);
}

// static
QString Boards::getInputTag(const InputsTable * inputs, int index)
{
  if (index > -1 && index < (int)inputs->size())
    return inputs->at(index).tag.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const int Boards::getInputYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_inputs->size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_inputs->at(i).cfgYaml : m_inputs->at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getInputName(m_inputs, i) : getInputTag(m_inputs, i));
    if (val == tmp)
      return getInputIndex(m_inputs, val, type);
  }

  return -1;
}

const QString Boards::getInputYamlName(int index, YamlLookupType ylt) const
{
  if (index > -1 && index < (int)m_inputs->size()) {
    if (ylt == YLT_CONFIG)
      return m_inputs->at(index).cfgYaml == Board::LVT_NAME ? getInputName(m_inputs, index) : getInputTag(m_inputs, index);
    else
      return m_inputs->at(index).refYaml == Board::LVT_NAME ? getInputName(m_inputs, index) : getInputTag(m_inputs, index);
  }

  return CPN_STR_UNKNOWN_ITEM;
}

const int Boards::getInputsCalibrated() const
{
  return getInputsCalibrated(m_inputs);
}

// static
int Boards::getInputsCalibrated(const InputsTable * inputs)
{
  unsigned int cnt = 0;

  for (const auto &defn : *inputs) {
    if (isInputCalibrated(defn)) cnt++;
  }

  return cnt;
}

const int Boards::getInputTagOffset(QString tag)
{
  return getInputTagOffset(m_inputs, tag);
}

// static
int Boards::getInputTagOffset(const InputsTable * inputs, QString tag)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if (tag == inputs->at(i).tag.c_str())
      return i;
  }

  return -1;
}

const int Boards::getInputExtIndex(int index)
{
  if (getCapability(Capability::Pots) > 0)
    return getInputTagOffset(m_inputs, QString("EXT%1").arg(index));

  return -1;
}

const int Boards::getInputPotIndex(int index)
{
  if (getCapability(Capability::Pots) > 0)
    return getInputTagOffset(m_inputs, QString("P%1").arg(index));

  return -1;
}

const int Boards::getInputSliderIndex(int index)
{
  if (getCapability(Capability::Sliders) > 0)
    return getInputTagOffset(m_inputs, QString("SL%1").arg(index));

  return -1;
}

const int Boards::getInputThrottleIndex()
{
  if (getCapability(Capability::Sticks) > 0)
    return getInputTagOffset(m_inputs, Boards::getCapability(m_id, Capability::Air) ? "RV" : "TH");

  return -1;
}

const int Boards::getInputTypeOffset(Board::AnalogInputType type)
{
  return getInputTypeOffset(m_inputs, type);
}

// static
int Boards::getInputTypeOffset(const InputsTable * inputs, Board::AnalogInputType type)
{
  for (int i = 0; i < (int)inputs->size(); i++) {
    if (type == inputs->at(i).type)
      return i;
  }

  return -1;
}

const Board::InputInfo Boards::getInputInfo(int index) const
{
  return getInputInfo(m_inputs, index);
}

// static
Board::InputInfo Boards::getInputInfo(const InputsTable * inputs, int index)
{
  Board::InputInfo info;

  if (index >= 0 && index < (int)inputs->size()) {
    InputDefn defn = inputs->at(index);
    info.type = defn.type;
    info.tag = defn.tag;
    info.name = defn.name;
    info.shortName = defn.shortName;
    info.flexType = defn.flexType;
    info.inverted = defn.inverted;
  }

  return info;
}

const int Boards::getKeyIndex(const QString key) const
{
  return getKeyIndex(m_keys, key);
}

// static
int Boards::getKeyIndex(const KeysTable * keys, QString key)
{
  for (int i = 0; i < (int)keys->size(); i++) {
    if (keys->at(i).key.c_str() == key)
      return i;
  }

  return -1;
}

const Board::KeyInfo Boards::getKeyInfo(int index) const
{
  return getKeyInfo(m_keys, index);
}

// static
Board::KeyInfo Boards::getKeyInfo(const KeysTable * keys, int index)
{
  Board::KeyInfo info;

  if (index >= 0 && index < (int)keys->size()) {
    KeyDefn defn = keys->at(index);
    info.name = defn.name;
    info.key = defn.key;
    info.label = defn.label;
  }

  return info;
}

// static
int Boards::getNumericSuffix(const std::string str)
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

const int Boards::getCFSIndexForSwitch(int offset) const
{
  return getCFSIndexForSwitch(m_switches, offset);
}

// static
int Boards::getCFSIndexForSwitch(const SwitchesTable * switches, int sw)
{
  if (sw < (int)switches->size() && switches->at(sw).isCustomSwitch)
    return switches->at(sw).customSwitchIdx;

  return -1;
}

const int Boards::getSwitchIndexForCFS(int offset) const
{
  return getSwitchIndexForCFS(m_switches, offset);
}

// static
int Boards::getSwitchIndexForCFS(const SwitchesTable * switches, int cfsIdx)
{
  for (int i = 0; i < (int)switches->size(); i++) {
    if (switches->at(i).isCustomSwitch && switches->at(i).customSwitchIdx == cfsIdx)
      return i;
  }

  return -1;
}

const int Boards::getCFSOffsetForCFSIndex(int index) const
{
  return getCFSOffsetForCFSIndex(m_switches, index);
}

// static
int Boards::getCFSOffsetForCFSIndex(const SwitchesTable * switches, const int index)
{
  int cnt = 0;

  for (int i = 0; i < (int)switches->size(); i++) {
    if (switches->at(i).isCustomSwitch) {
      if (switches->at(i).customSwitchIdx == index)
        return cnt;
      else
        cnt++;
    }
  }

  return -1;
}

const int Boards::getSwitchIndexForCFSOffset(int offset) const
{
  return getSwitchIndexForCFSOffset(m_switches, offset);
}

// static
int Boards::getSwitchIndexForCFSOffset(const SwitchesTable * switches, const int offset)
{
  int cnt = 0;

  for (int i = 0; i < (int)switches->size(); i++) {
    if (switches->at(i).isCustomSwitch) {
      if (cnt == offset)
        return i;
      else
        cnt++;
    }
  }

  return -1;
}

const int Boards::getSwitchIndex(const QString val, Board::LookupValueType lvt) const
{
  return getSwitchIndex(m_switches, val, lvt);
}

// static
int Boards::getSwitchIndex(const SwitchesTable * switches, QString val, Board::LookupValueType lvt)
{
  for (int i = 0; i < (int)switches->size(); i++) {
    if ((lvt == Board::LVT_TAG && switches->at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && switches->at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const Board::SwitchInfo Boards::getSwitchInfo(int index) const
{
  return getSwitchInfo(m_switches, index);
}

// static
Board::SwitchInfo Boards::getSwitchInfo(const SwitchesTable * switches, int index)
{
  Board::SwitchInfo info;

  if (index >= 0 && index < (int)switches->size()) {
    SwitchDefn defn = switches->at(index);
    info.type = defn.type;
    info.tag = defn.tag;
    info.name = defn.name;
    info.dflt = defn.dflt;
    info.inverted = defn.inverted;
  }

  return info;
}

const QString Boards::getSwitchName(int index) const
{
  return getSwitchName(m_switches, index);
}

// static
QString Boards::getSwitchName(const SwitchesTable * switches, int index)
{
  if (index > -1 && index < (int)switches->size())
    return switches->at(index).name.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const QString Boards::getSwitchTag(int index) const
{
  return getSwitchTag(m_switches, index);
}

// static
QString Boards::getSwitchTag(const SwitchesTable * switches, int index)
{
  if (index > -1 && index < (int)switches->size())
    return switches->at(index).tag.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const int Boards::getSwitchTagNum(int index) const
{
  return getSwitchTagNum(m_switches, index);
}

// static
int Boards::getSwitchTagNum(const SwitchesTable * switches, int index)
{
  if (index > -1 && index < (int)switches->size())
    return getNumericSuffix(switches->at(index).tag.c_str());

  return -1;
}

const int Boards::getSwitchTypeOffset(Board::SwitchType type)
{
  return getSwitchTypeOffset(m_switches, type);
}

// static
int Boards::getSwitchTypeOffset(const SwitchesTable * switches, Board::SwitchType type)
{
  for (int i = 0; i < (int)switches->size(); i++) {
    if (type == switches->at(i).type)
      return i;
  }

  return -1;
}

const int Boards::getSwitchYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_switches->size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_switches->at(i).cfgYaml : m_switches->at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getSwitchName(m_switches, i) : getSwitchTag(m_switches, i));
    if (val == tmp)
      return getSwitchIndex(m_switches, val, type);
  }

  return -1;
}

const QString Boards::getSwitchYamlName(int index, YamlLookupType ylt) const
{
  if (index > -1 && index < (int)m_switches->size()) {
    if (ylt == YLT_CONFIG)
      return m_switches->at(index).cfgYaml == Board::LVT_NAME ? getSwitchName(m_switches, index) : getSwitchTag(m_switches, index);
    else
      return m_switches->at(index).refYaml == Board::LVT_NAME ? getSwitchName(m_switches, index) : getSwitchTag(m_switches, index);
  }

  return CPN_STR_UNKNOWN_ITEM;
}

const int Boards::getTrimIndex(const QString val, Board::LookupValueType lvt) const
{
  return getTrimIndex(m_trims, val, lvt);
}

// static
int Boards::getTrimIndex(const TrimsTable * trims, QString val, Board::LookupValueType lvt)
{
  for (int i = 0; i < (int)trims->size(); i++) {
    if ((lvt == Board::LVT_TAG && trims->at(i).tag.c_str() == val) ||
        (lvt == Board::LVT_NAME && trims->at(i).name.c_str() == val))
      return i;
  }

  return -1;
}

const QString Boards::getTrimName(int index) const
{
  return getTrimName(m_trims, index);
}

// static
QString Boards::getTrimName(const TrimsTable * trims, int index)
{
  if (index > -1 && index < (int)trims->size())
    return trims->at(index).name.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const QString Boards::getTrimTag(int index) const
{
  return getTrimTag(m_trims, index);
}

// static
QString Boards::getTrimTag(const TrimsTable * trims, int index)
{
  if (index > -1 && index < (int)trims->size())
    return trims->at(index).tag.c_str();

  return CPN_STR_UNKNOWN_ITEM;
}

const int Boards::getTrimYamlIndex(const QString val, YamlLookupType ylt) const
{
  for (int i = 0; i < (int)m_trims->size(); i++) {
    Board::LookupValueType type = (ylt == YLT_CONFIG ? m_trims->at(i).cfgYaml : m_trims->at(i).refYaml);
    QString tmp = (type == Board::LVT_NAME ? getTrimName(m_trims, i) : getTrimTag(m_trims, i));
    if (val == tmp)
      return getTrimIndex(m_trims, val, type);
  }

  return -1;
}

const QString Boards::getTrimYamlName(int index, YamlLookupType ylt) const
{
  if (index > -1 && index < (int)m_trims->size()) {
    if (ylt == YLT_CONFIG)
      return m_trims->at(index).cfgYaml == Board::LVT_NAME ? getTrimName(m_trims, index) : getTrimTag(m_trims, index);
    else
      return m_trims->at(index).refYaml == Board::LVT_NAME ? getTrimName(m_trims, index) : getTrimTag(m_trims, index);
  }

  return CPN_STR_UNKNOWN_ITEM;
}

const bool Boards::isInputAvailable(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputAvailable(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputAvailable(const InputDefn & defn)
{
  return (defn.type == Board::AIT_STICK ||
          (defn.type == Board::AIT_FLEX && defn.flexType != Board::FLEX_NONE && !isInputFlexJoystickAxis(defn)));
}

const bool Boards::isInputCalibrated(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputCalibrated(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputCalibrated(const InputDefn & defn)
{
  return (isInputStick(defn) || isInputFlexPot(defn) || isInputFlexSlider(defn));
}

const bool Boards::isInputConfigurable(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputConfigurable(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputConfigurable(const InputDefn & defn)
{
  return (isInputStick(defn) || isInputFlexPot(defn) || isInputFlexSlider(defn));
}

const bool Boards::isInputIgnored(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputIgnored(m_inputs->at(index)) : true;
}

// static
bool Boards::isInputIgnored(const InputDefn & defn)
{
  return (isInputFlexJoystickAxis(defn) || isInputSwitch(defn));
}

// static
bool Boards::isInputFlex(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX;
}

const bool Boards::isInputFlexGyroAxis(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputFlexGyroAxis(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputFlexGyroAxis(const InputDefn & defn)
{
  const char* val = defn.tag.data();

  return (defn.type == Board::AIT_FLEX && defn.tag.size() > 5 &&
          val[0] == 'T' && val[1] == 'I'  && val[2] == 'L' && val[3] == 'T' && val[4] == '_' && (val[5] == 'X' || val[5] == 'Y'));
}

const bool Boards::isInputFlexJoystickAxis(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputFlexJoystickAxis(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputFlexJoystickAxis(const InputDefn & defn)
{
  const char* val = defn.tag.data();

  return (defn.type == Board::AIT_FLEX && defn.tag.size() > 2 &&
          val[0] == 'J' && val[1] == 'S' && (val[2] == 'x' || val[2] == 'y'));
}

const bool Boards::isInputFlexPot(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputFlexPot(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputFlexPot(const InputDefn & defn)
{
  const char* val = defn.tag.data();
  size_t len = defn.tag.size();

  return (defn.type == Board::AIT_FLEX &&
          ((len > 1 && val[0] == 'P' && val[1] >= '0' && val[1] <= '9') ||
           (len > 3 && val[0] == 'E' && val[1] == 'X' && val[2] == 'T' && val[3] >= '0' && val[3] <= '9')));
}

// static
bool Boards::isInputFlexPotMultipos(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX && defn.flexType == Board::FLEX_MULTIPOS;
}

// static
bool Boards::isInputFlexSlider(const InputDefn & defn)
{
  const char* val = defn.tag.data();

  return (defn.type == Board::AIT_FLEX && defn.tag.size() > 2 &&
          val[0] == 'S' && val[1] == 'L' && val[2] >= '0' && val[2] <= '9');
}

// static
bool Boards::isInputFlexSwitch(const InputDefn & defn)
{
  return defn.type == Board::AIT_FLEX && defn.flexType == Board::FLEX_SWITCH;
}

// static
bool Boards::isInputRTCBat(const InputDefn & defn)
{
  return defn.type == Board::AIT_RTC_BAT;
}

const bool Boards::isInputStick(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputStick(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputStick(const InputDefn & defn)
{
  return defn.type == Board::AIT_STICK;
}

const bool Boards::isInputSwitch(int index) const
{
  return (index >=0 && index < (int)m_inputs->size()) ? isInputSwitch(m_inputs->at(index)) : false;
}

// static
bool Boards::isInputSwitch(const InputDefn & defn)
{
  return defn.type == Board::AIT_SWITCH;
}

// static
bool Boards::isInputVBat(const InputDefn & defn)
{
  return defn.type == Board::AIT_VBAT;
}

const bool Boards::isSwitchConfigurable(int index) const
{
  if (index >= 0 && index < getCapability(Capability::Switches)) {
    SwitchDefn &defn = m_switches->at(index);
    if (isSwitchStd(defn) || isSwitchFunc(defn))
      return true;

    if (isSwitchFlex(defn)) {
      int sfx = getNumericSuffix(defn.tag);
      if (sfx > 0 && sfx <= getCapability(Capability::FlexSwitches))
        return true;
    }
  }

  return false;
}

// static
bool Boards::isSwitchStd(const SwitchDefn & defn)
{
  return !(isSwitchFlex(defn) || isSwitchFunc(defn));
}

const bool Boards::isSwitchFlex(int index) const
{
  return (index >=0 && index < (int)m_switches->size()) ? isSwitchFlex(m_switches->at(index)) : false;
}

// static
bool Boards::isSwitchFlex(const SwitchDefn & defn)
{
  const char* val = defn.tag.data();

  return (defn.tag.size() > 2 &&
          val[0] == 'F' && val[1] == 'L' && val[2] >= '0' && val[2] <= '9');
}

const bool Boards::isSwitchFunc(int index) const
{
  return (index >=0 && index < (int)m_switches->size()) ? isSwitchFunc(m_switches->at(index)) : false;
}

// static
bool Boards::isSwitchFunc(const SwitchDefn & defn)
{
  return defn.customSwitchIdx >= 0;
}

bool Boards::loadDefinition()
{
  // safety net for BoardFactory::instance
  if (m_id == Board::BOARD_UNKNOWN)
    return true;

  if (!loadFile(m_id, m_hwdefn, m_inputs, m_switches, m_keys, m_trims, m_display, m_cfs, m_hardware, m_hwextra, m_hasKeyLockCombo))
    return false;

  afterLoadFixups(m_id, m_inputs, m_switches, m_keys, m_trims);

  setInputCounts(m_inputs, m_inputCnt);
  setSwitchCounts(m_switches, m_switchCnt);

  // json files do not normally specify stick labels so load legacy labels
  for (int i = 0; i < getCapability(Capability::Sticks); i++) {
    if (m_inputs->at(i).name.empty())
      m_inputs->at(i).name = DataHelpers::getStringTagMappingName(stickNamesLookupTable, m_inputs->at(i).tag.c_str());
  }

  qDebug() << "Board:" << Boards::getBoardName(m_id) <<
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

  return true;
}

// static
bool Boards::loadFile(Board::Type id, QString hwdefn, InputsTable * inputs, SwitchesTable * switches,
                         KeysTable * keys, TrimsTable * trims, DisplayDefn * display, CustomSwitchesDefn * cfs,
                         HardwareDefn * hardware, BoardDefn * hwextra, bool & hasKeyLockCombo)
{
  if (id == Board::BOARD_UNKNOWN) {
    return false;
  }

  // required because of the way the Firmware class is used
  if (hwdefn.isEmpty()) {
    return false;
  }

  QString path = QString(":/hwdefs/%1.json").arg(hwdefn);
  QFile file(path);

  if (!file.exists()) {
    QMessageBox::critical(nullptr, tr("Load Board Hardware Definition"),
                          tr("Board: %1\nError: Unable to load file %2").arg(Boards::getBoardName(id)).arg(path));
    return false;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(nullptr, tr("Load Board Hardware Definition"),
                          tr("Board: %1\nError: Unable to open file %2").arg(Boards::getBoardName(id)).arg(path));
    return false;
  }

  QByteArray buffer = file.readAll();
  file.close();

  if (buffer.isEmpty()) {
    QMessageBox::critical(nullptr, tr("Load Board Hardware Definition"),
                          tr("Board: %1\nError: Unable to read file %2").arg(Boards::getBoardName(id)).arg(path));
    return false;
  }

  QJsonParseError res;
  QJsonDocument *json = new QJsonDocument();
  *json = QJsonDocument::fromJson(buffer, &res);

  if (res.error || json->isNull() || !json->isObject()) {
    QMessageBox::critical(nullptr, tr("Load Board Hardware Definition"),
                          tr("Board: %1\nError: %2 is not a valid json formatted file.\nError code: %3\nError description: %4").arg(Boards::getBoardName(id)).arg(path).arg(res.error).arg(res.errorString()));
    delete json;
    return false;
  }

  const QJsonObject &obj = json->object();

  if (obj.value("adc_inputs").isObject()) {
    const QJsonObject &adcinputs = obj.value("adc_inputs").toObject();

    if (adcinputs.value("inputs").isArray()) {
      const QJsonArray &in = adcinputs.value("inputs").toArray();

      for (const QJsonValue &input : in)
      {
        if (input.isObject()) {
          const QJsonObject &o = input.toObject();
          InputDefn defn;

          if (!o.value("name").isUndefined())
            defn.tag = o.value("name").toString().toStdString();

          if (!o.value("type").isUndefined()) {
            std::string type = o.value("type").toString().toStdString();
            defn.type = (Board::AnalogInputType)DataHelpers::getStringTagMappingIndex(inputTypesLookupTable, type.c_str());
            if (defn.type == Board::AIT_STICK)
              defn.refYaml = Board::LVT_NAME;
          }

          if (!o.value("inverted").isUndefined())
            defn.inverted = o.value("inverted").toBool();

          if (!o.value("label").isUndefined())
            defn.name = o.value("label").toString().toStdString();

          if (!o.value("short_label").isUndefined())
            defn.shortName = o.value("short_label").toString().toStdString();

          if (!o.value("default").isUndefined()) {
            std::string dflt = o.value("default").toString().toStdString();
            if (defn.type == Board::AIT_FLEX) {
              int idx = DataHelpers::getStringTagMappingIndex(flexTypesLookupTable, dflt.c_str());
              defn.flexType = (Board::FlexType)(idx < 0 ? Board::FLEX_NONE : idx);
            }
          }

          inputs->insert(inputs->end(), defn);

//          qDebug() << "name:" << defn.name.c_str() <<
//                      "type:" << defn.stype.c_str() << ">" <<
//                      DataHelpers::getStringNameMappingTag(inputTypesLookupTable, std::to_string(defn.type).c_str()).c_str() <<
//                      "label:" << defn.label.c_str() << "short:" << defn.shortLabel.c_str() << "inverted:" << defn.inverted <<
//                      "dflt:" << defn.dflt.c_str() << ">" <<
//                      DataHelpers::getStringNameMappingTag(flexTypesLookupTable, std::to_string(defn.flexType).c_str()).c_str();
        }
      }
    }
  }

  if (obj.value("switches").isArray()) {
    const QJsonArray &swtchs = obj.value("switches").toArray();

    for (const QJsonValue &swtch : swtchs)
    {
      if (swtch.isObject()) {
        const QJsonObject &o = swtch.toObject();
        SwitchDefn sw;

        if (!o.value("name").isUndefined()) {
          sw.name = o.value("name").toString().toStdString();
          sw.tag = sw.name;
        }

        if (!o.value("type").isUndefined()) {
          std::string type = o.value("type").toString().toStdString();
          int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable, type.c_str());
          sw.type = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
        }

        if (!o.value("flags").isUndefined())
          sw.flags = o.value("flags").toInt();

        if (!o.value("inverted").isUndefined())
          sw.inverted = o.value("inverted").toBool();

        if (!o.value("default").isUndefined()) {
          std::string dflt = o.value("default").toString().toStdString();
          int idx = DataHelpers::getStringTagMappingIndex(switchTypesLookupTable, dflt.c_str());
          sw.dflt = idx < 0 ? Board::SWITCH_NOT_AVAILABLE : (Board::SwitchType)idx;
        }

        if (o.value("display").isArray()) {
          const QJsonArray &d = obj.value("display").toArray();
          sw.display.x = (unsigned int)d.at(0).toInt(0);
          sw.display.y = (unsigned int)d.at(1).toInt(0);
        }

        if (!o.value("is_cfs").isUndefined())
          sw.isCustomSwitch = o.value("is_cfs").toBool();

        if (!o.value("cfs_idx").isUndefined())
          sw.customSwitchIdx = o.value("cfs_idx").toInt();

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

        switches->insert(switches->end(), sw);

//        qDebug() << "tag:" << sw.tag.c_str() << "name:" << sw.name.c_str() << "type:" << sw.type << ">" << Boards::switchTypeToString(sw.type) <<
//                    "flags:" << sw.flags << "default:" << sw.dflt << ">" << Boards::switchTypeToString(sw.dflt) <<
//                    "inverted:" << sw.inverted << "display:" << QString("%1").arg(sw.display.x) << "," << QString("%1").arg(sw.display.y);
      }
    }
  }

  if (obj.value("keys").isArray()) {
    const QJsonArray &kys = obj.value("keys").toArray();

    for (const QJsonValue &key : kys)
    {
      if (key.isObject()) {
        const QJsonObject &o = key.toObject();
        KeyDefn k;

        if (!o.value("name").isUndefined()) {
          k.name = o.value("name").toString().toStdString();
          k.key = o.value("key").toString().toStdString();
          k.label = o.value("label").toString().toStdString();
          k.tag = k.name;
        }

        keys->insert(keys->end(), k);

//        qDebug() << "name:" << k.name.c_str() << "key:" << k.key.c_str() << "label:" << k.label.c_str();
      }
    }
  }

  hasKeyLockCombo = obj.value("key_lock_combo").isArray() &&
                    obj.value("key_lock_combo").toArray().size() == 2;

  if (obj.value("trims").isArray()) {
    const QJsonArray &trms = obj.value("trims").toArray();

    for (const QJsonValue &trm : trms)
    {
      if (trm.isObject()) {
        const QJsonObject &o = trm.toObject();
        TrimDefn t;

        if (!o.value("name").isUndefined()) {
          t.name = o.value("name").toString().toStdString();
          t.tag = t.name;
        }

        trims->insert(trims->end(), t);

//        qDebug() << "name:" << t.name.c_str();
      }
    }
  }

  if (obj.value("display").isObject()) {
    const QJsonObject &o = obj.value("display").toObject();

    display->w = o.value("lcd_w").toInt();
    display->h = o.value("lcd_h").toInt();
    display->phys_w = o.value("lcd_phys_w").toInt();
    display->phys_h = o.value("lcd_phys_h").toInt();
    display->depth = o.value("lcd_depth").toInt();
    display->color = display->depth == 16 ? 1 : 0;
    display->oled = o.value("oled_screen").toBool();
  }

  if (obj.value("backlight").isObject()) {
    const QJsonObject &o = obj.value("backlight").toObject();

    display->backlight_color = o.value("has_backlight_color").toBool();
  }

  if (obj.value("leds").isObject()) {
    const QJsonObject &o = obj.value("leds").toObject();

    int cfs_led_strip_length = o.value("cfs_led_strip_length").toInt();
    int cfs_leds_per_switch = o.value("cfs_leds_per_switch").toInt();
    cfs->groups = cfs_leds_per_switch ? cfs_led_strip_length / (2 * cfs_leds_per_switch) : 0;
    cfs->rgb_led = cfs->groups > 0;
    hardware->has_bling_leds = o.value("bling_led_strip_length").toInt();
  }

  if (obj.value("hardware").isObject()) {
    const QJsonObject &o = obj.value("hardware").toObject();

    hardware->has_audio_mute = o.value("has_audio_mute").toBool();
    hardware->has_ext_module_support = o.value("has_ext_module_support").toBool();
    hardware->has_int_module_support = o.value("has_int_module_support").toBool();
    hardware->sport_max_baudrate = o.value("sport_max_baudrate").toInt();
    hardware->surface = o.value("surface").toBool();
    hardware->cpu = o.value("cpu").toString().toStdString();
    hardware->cpu_type = o.value("cpu_type").toString().toStdString();
  }

  delete json;
  return true;
}

// static
void Boards::setInputCounts(const InputsTable * inputs, InputCounts & inputCounts)
{
  for (const auto &defn : *inputs) {
    if (isInputStick(defn))
      inputCounts.sticks++;
    else if (isInputFlexPot(defn))
      inputCounts.flexPots++;
    else if (isInputFlexSlider(defn))
      inputCounts.flexSliders++;
    else if (isInputFlexGyroAxis(defn))
      inputCounts.flexGyroAxes++;
    else if (isInputFlexJoystickAxis(defn))
      inputCounts.flexJoystickAxes++;
    else if (isInputFlexSwitch(defn))
      inputCounts.flexSwitches++;
    else if (isInputRTCBat(defn))
      inputCounts.rtcbat++;
    else if (isInputVBat(defn))
      inputCounts.vbat++;
    else if (isInputSwitch(defn))
      inputCounts.switches++;
  }
}

// static
void Boards::setSwitchCounts(const SwitchesTable * switches, SwitchCounts & switchCounts)
{
  for (const auto &swtch : *switches) {
    if (isSwitchStd(swtch))
      switchCounts.std++;
    else if (isSwitchFlex(swtch))
      switchCounts.flex++;
    else if (isSwitchFunc(swtch))
      switchCounts.func++;
  }
}
