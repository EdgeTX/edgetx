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

#include "generalsettings.h"

#include "appdata.h"
#include "eeprominterface.h"
#include "radiodataconversionstate.h"
#include "compounditemmodels.h"
#include "yaml_ops.h"

const QList<int> channelsAir = { // First number is 0..23 -> template setup,  Second is relevant channel out
  1,2,3,4 , 1,2,4,3 , 1,3,2,4 , 1,3,4,2 , 1,4,2,3 , 1,4,3,2,
  2,1,3,4 , 2,1,4,3 , 2,3,1,4 , 2,3,4,1 , 2,4,1,3 , 2,4,3,1,
  3,1,2,4 , 3,1,4,2 , 3,2,1,4 , 3,2,4,1 , 3,4,1,2 , 3,4,2,1,
  4,1,2,3 , 4,1,3,2 , 4,2,1,3 , 4,2,3,1 , 4,3,1,2 , 4,3,2,1
};

const QList<QChar> controlsAir = { 'R', 'E', 'T', 'A' }; //  channelsAir[x] - 1] entry mapping

const QList<int> channelsSurface = { // First number is 0..1 -> template setup,  Second is relevant channel out
  1,2 , 2,1
};

const QList<QChar> controlsSurface = { 'S', 'T' }; //  channelsSurface[x] - 1] entry mapping

const QList<int> useChannels(bool isBoardAir)
{
  return isBoardAir ? channelsAir : channelsSurface;
}

const QList<QChar> useControls(bool isBoardAir)
{
  return isBoardAir ? controlsAir : controlsSurface;
}

const int channelsCount(bool isBoardAir)
{
  return isBoardAir ? channelsAir.size() : channelsSurface.size();
}

const int controlsCount(bool isBoardAir)
{
  return isBoardAir ? controlsAir.size() : controlsSurface.size();
}

bool GeneralSettings::switchPositionAllowed(int index) const
{
  if (index == 0)
    return true;

  div_t qr = div(abs(index) - 1, 3);

  if (index < 0 && switchConfig[qr.quot].type != Board::SWITCH_3POS)
    return false;
  else if (qr.rem == 1)
    return switchConfig[qr.quot].type == Board::SWITCH_3POS;
  else
    return switchConfig[qr.quot].type != Board::SWITCH_NOT_AVAILABLE;
}

bool GeneralSettings::switchSourceAllowed(int index) const
{
  return switchConfig[index].type != Board::SWITCH_NOT_AVAILABLE;
}

bool GeneralSettings::isInputAvailable(int index) const
{
  Board::Type board = getCurrentBoard();

  if (index < 0 || index >= Boards::getCapability(board, Board::Inputs))
    return false;

  const InputConfig &config = inputConfig[index];

  return (config.type == Board::AIT_STICK ||
          (config.type == Board::AIT_FLEX && config.flexType != Board::FLEX_NONE));
}

bool GeneralSettings::isInputFlexSwitchAvailable(int index) const
{
  Board::Type board = getCurrentBoard();

  if (index < 0 || index >= Boards::getCapability(board, Board::Inputs))
    return false;

  const InputConfig &config = inputConfig[index];

  return (config.type == Board::AIT_FLEX && config.flexType == Board::FLEX_SWITCH);
}

bool GeneralSettings::isInputMultiPosPot(int index) const
{
  if (isInputAvailable(index)) {
    const InputConfig &config = inputConfig[index];

    return (config.type == Board::AIT_FLEX &&
            config.flexType == Board::FLEX_MULTIPOS);
  }

  return false;
}

bool GeneralSettings::isInputPot(int index) const
{
  if (isInputAvailable(index)) {
    const InputConfig &config = inputConfig[index];

    return (config.type == Board::AIT_FLEX &&
           (config.flexType == Board::FLEX_POT ||
            config.flexType == Board::FLEX_POT_CENTER ||
            config.flexType == Board::FLEX_MULTIPOS));
  }

  return false;
}

bool GeneralSettings::isInputSlider(int index) const
{
  if (isInputAvailable(index)) {
    const InputConfig &config = inputConfig[index];

    return (config.type == Board::AIT_FLEX &&
            config.flexType == Board::FLEX_SLIDER);
  }
  return false;
}

bool GeneralSettings::isInputStick(int index) const
{
  if (isInputAvailable(index)) {
    const InputConfig &config = inputConfig[index];

    return config.type == Board::AIT_STICK;
  }

  return false;
}

bool GeneralSettings::isSwitchAvailable(int index) const
{
  if (index < 0 || index >= Boards::getCapability(getCurrentBoard(), Board::Switches))
    return false;

  const SwitchConfig &config = switchConfig[index];

  return config.type != Board::SWITCH_NOT_AVAILABLE;
}

bool GeneralSettings::isSwitchFlex(int index) const
{
  return Boards::isSwitchFlex(index);
}

bool GeneralSettings::unassignedInputFlexSwitches() const
{
  Board::Type board = getCurrentBoard();
  int cnt = 0;

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (inputConfig[i].flexType == Board::FLEX_SWITCH)
      cnt++;
  }
  return cnt < Boards::getCapability(board, Board::FlexSwitches);
}

void GeneralSettings::clear()
{
  memset(reinterpret_cast<void *>(this), 0, sizeof(GeneralSettings));
  setDefaultControlTypes(getCurrentBoard());
  init();
}

void GeneralSettings::init()
{
  Firmware * firmware = Firmware::getCurrentVariant();
  Board::Type board = firmware->getBoard();

  // vBatWarn is voltage in 100mV, vBatMin is in 100mV but with -9V offset, vBatMax has a -12V offset
  Boards::getBattRange(board, vBatMin, vBatMax, vBatWarn);

  backlightMode = 3; // keys and sticks
  backlightDelay = 2; // 2 * 5 = 10 secs
  inactivityTimer = 10;

  hatsMode = HATSMODE_SWITCHABLE;

  // backlightBright = 0; // 0 = 100%

  if (IS_FAMILY_HORUS_OR_T16(board)) {
    backlightOffBright = 20;
  }

  speakerVolume = 12;
  wavVolume = 2;
  backgroundVolume = 1;

  if (IS_TARANIS(board))
    contrast = 25;

  if (IS_JUMPER_T16(board))
    strcpy(bluetoothName, "t16");
  else if (IS_FLYSKY_NV14(board))
    strcpy(bluetoothName, "nv14");
  else if (IS_FLYSKY_PL18(board))
    strcpy(bluetoothName, "pl18");
  else if (IS_FAMILY_HORUS_OR_T16(board))
    strcpy(bluetoothName, "horus");
  else if (IS_TARANIS_X9E(board) || IS_TARANIS_SMALL(board))
    strcpy(bluetoothName, "taranis");

  ttsLanguage[0] = 'e';
  ttsLanguage[1] = 'n';

  templateSetup = g.profile[g.sessionId()].channelOrder();
  stickMode = g.profile[g.sessionId()].defaultMode();

  QString t_calib = g.profile[g.sessionId()].stickPotCalib();
  if (!t_calib.isEmpty()) {
    QString t_trainercalib=g.profile[g.sessionId()].trainerCalib();
    int8_t t_txVoltageCalibration=(int8_t)g.profile[g.sessionId()].txVoltageCalibration();
    int8_t t_txCurrentCalibration=(int8_t)g.profile[g.sessionId()].txCurrentCalibration();
    int8_t t_PPM_Multiplier=(int8_t)g.profile[g.sessionId()].ppmMultiplier();
    uint8_t t_stickMode=(uint8_t)g.profile[g.sessionId()].gsStickMode();
    uint8_t t_vBatWarn=(uint8_t)g.profile[g.sessionId()].vBatWarn();
    QString t_DisplaySet=g.profile[g.sessionId()].display();
    QString t_BeeperSet=g.profile[g.sessionId()].beeper();
    QString t_HapticSet=g.profile[g.sessionId()].haptic();
    QString t_SpeakerSet=g.profile[g.sessionId()].speaker();
    QString t_CountrySet=g.profile[g.sessionId()].countryCode();

    if ((t_calib.length() == (Boards::getInputsCalibrated() * 12)) && (t_trainercalib.length() == 16)) {
      QString Byte;
      int16_t byte16;
      bool ok;
      for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
        if (Boards::isInputCalibrated(i)) {
          Byte = t_calib.mid(i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) inputConfig[i].calib.mid = byte16;
          Byte = t_calib.mid(4 + i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) inputConfig[i].calib.spanNeg = byte16;
          Byte = t_calib.mid(8 + i * 12, 4);
          byte16 = (int16_t)Byte.toInt(&ok, 16);
          if (ok) inputConfig[i].calib.spanPos = byte16;
        }
      }
      for (int i = 0; i < Boards::getCapability(board, Board::Sticks); i++) {
        Byte = t_trainercalib.mid(i * 4, 4);
        byte16 = (int16_t)Byte.toInt(&ok, 16);
        if (ok) trainer.calib[i] = byte16;
      }
      txCurrentCalibration = t_txCurrentCalibration;
      txVoltageCalibration = t_txVoltageCalibration;
      vBatWarn = t_vBatWarn;
      PPM_Multiplier = t_PPM_Multiplier;
      stickMode = t_stickMode;
    }
    if ((t_DisplaySet.length()==6) && (t_BeeperSet.length()==4) && (t_HapticSet.length()==6) && (t_SpeakerSet.length()==6)) {
      uint8_t byte8u;
      int8_t byte8;
      bool ok;
      byte8=(int8_t)t_DisplaySet.mid(0,2).toInt(&ok,16);
      if (ok)
        optrexDisplay=(byte8==1 ? true : false);
      byte8u=(uint8_t)t_DisplaySet.mid(2,2).toUInt(&ok,16);
      if (ok)
        contrast=byte8u;
      byte8u=(uint8_t)t_DisplaySet.mid(4,2).toUInt(&ok,16);
      if (ok)
        backlightBright=byte8u;
      byte8=(int8_t)t_BeeperSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        beeperMode=(BeeperMode)byte8;
      byte8=(int8_t)t_BeeperSet.mid(2,2).toInt(&ok,16);
      if (ok)
        beeperLength=byte8;
      byte8=(int8_t)t_HapticSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        hapticMode=(BeeperMode)byte8;
      byte8=(int8_t)t_HapticSet.mid(2,2).toInt(&ok,16);
      if (ok)
        hapticStrength=byte8;
      byte8=(int8_t)t_HapticSet.mid(4,2).toInt(&ok,16);
      if (ok)
        hapticLength=byte8;
      byte8u=(uint8_t)t_SpeakerSet.mid(0,2).toUInt(&ok,16);
      if (ok)
        speakerMode=byte8u;
      byte8u=(uint8_t)t_SpeakerSet.mid(2,2).toUInt(&ok,16);
      if (ok)
        speakerPitch=byte8u;
      byte8u=(uint8_t)t_SpeakerSet.mid(4,2).toUInt(&ok,16);
      if (ok)
        speakerVolume=byte8u;
      if (t_CountrySet.length()==6) {
        byte8u=(uint8_t)t_CountrySet.mid(0,2).toUInt(&ok,16);
        if (ok)
          countryCode=byte8u;
        byte8u=(uint8_t)t_CountrySet.mid(2,2).toUInt(&ok,16);
        if (ok)
          imperial=byte8u;
        QString chars = t_CountrySet.mid(4, 2);
        ttsLanguage[0] = chars[0].toLatin1();
        ttsLanguage[1] = chars[1].toLatin1();
      }
    }
  }

  internalModule = g.profile[g.sessionId()].defaultInternalModule();

  if (IS_FLYSKY_NV14(board) || IS_FLYSKY_PL18(board))
    stickDeadZone = 2;

}

void GeneralSettings::setDefaultControlTypes(Board::Type board)
{
  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); i++) {
    if (!Boards::isInputIgnored(i, board)) {
      Board::InputInfo info =  Boards::getInputInfo(i, board);
      inputConfig[i].type = info.type;
      inputConfig[i].flexType = info.flexType;
      inputConfig[i].inverted = false; //info.inverted;
    }
  }

  for (int i = 0; i < Boards::getCapability(board, Board::Inputs); ++i) {
    if (!Boards::isInputCalibrated(i, board))
      continue;

    Board::InputInfo info = Boards::getInputInfo(i, board);

    if (info.type == Board::AIT_FLEX && info.flexType == Board::FLEX_MULTIPOS) {
      inputConfig[i].calib.mid     = 773;
      inputConfig[i].calib.spanNeg = 5388;
      inputConfig[i].calib.spanPos = 9758;
    } else {
      inputConfig[i].calib.mid     = 0x200;
      inputConfig[i].calib.spanNeg = 0x180;
      inputConfig[i].calib.spanPos = 0x180;
    }
  }

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    Board::SwitchInfo info =  Boards::getSwitchInfo(i, board);
    switchConfig[i].type = info.dflt;
    switchConfig[i].inverted = info.inverted;
    switchConfig[i].inputIdx = SWITCH_INPUTINDEX_NONE;
  }

  for (uint8_t i = 0; i < 4; i++) {
    trainer.mix[i].mode = TrainerMix::TRN_MIX_MODE_SUBST;
    trainer.mix[i].src = i;
    trainer.mix[i].weight = 100;
  }
}

int GeneralSettings::getDefaultStick(unsigned int channel) const
{
  if (channel >= CPN_MAX_STICKS)
    return -1;
  else {
    return useChannels(Boards::isAir())[controlsCount(Boards::isAir()) * templateSetup + channel] - 1;
  }
}

RawSource GeneralSettings::getDefaultSource(unsigned int channel) const
{
  int stick = getDefaultStick(channel);

  if (stick >= 0)
    return RawSource(SOURCE_TYPE_INPUT, stick + 1);
  else
    return RawSource(SOURCE_TYPE_NONE);
}

int GeneralSettings::getDefaultChannel(unsigned int stick) const
{
  for (int i = 0; i < controlsCount(Boards::isAir()); i++) {
    if (getDefaultStick(i) == (int)stick)
      return i;
  }
  return -1;
}

void GeneralSettings::convert(RadioDataConversionState & cstate)
{
  // Here we can add explicit conversions when moving from one board to another

  cstate.setOrigin(tr("Radio Settings"));

  cstate.setComponent(tr("Hardware"));
  cstate.setSubComp(tr("Internal Module"));
  RadioDataConversionState::LogField oldData(internalModule, ModuleData::typeToString(internalModule));

  if (internalModule != MODULE_TYPE_NONE && (int)internalModule != g.currentProfile().defaultInternalModule()) {
    internalModule = MODULE_TYPE_NONE;
    cstate.setInvalid(oldData);
  }

  //  Try to intelligently copy any custom controls
  //  step 1 clear current config
  memset(&inputConfig[0], 0, sizeof(InputConfig) * CPN_MAX_INPUTS);
  memset(&switchConfig[0], 0, sizeof(SwitchConfig) * CPN_MAX_SWITCHES);
  //  step 2 load default config
  setDefaultControlTypes(cstate.toType);
  //  step 3 copy matching config based on tags
  cstate.setSubComp(tr("Axis & Pots"));

  for (int i = 0; i < Boards::getCapability(cstate.fromType, Board::Inputs); i++) {
    if (Boards::isInputConfigurable(i, cstate.fromType)) {
      cstate.withComponentField("");
      cstate.setItemType(Boards::isInputStick(i, cstate.fromType) ? tr("Axis") : tr("Pot"));
      RadioDataConversionState::LogField oldData(i, Boards::getInputName(i, cstate.fromType));
      const int idx = Boards::getInputIndex(Boards::getInputTag(i, cstate.fromType), Board::LVT_TAG, cstate.toType);

      if (idx > -1) {
        const InputConfig &fromcfg = cstate.fromGS()->inputConfig[i];
        InputConfig &tocfg = inputConfig[idx];
        strncpy(tocfg.name, fromcfg.name, sizeof(inputConfig[0].name));
        tocfg.type = fromcfg.type;

        if (tocfg.type == Board::AIT_FLEX && !Boards::getCapability(cstate.toType, Board::FlexSwitches) &&
            fromcfg.flexType == Board::FLEX_SWITCH) {
          cstate.withComponentField(Boards::getInputName(i, cstate.fromType));
          RadioDataConversionState::LogField oldFT(i, Boards::flexTypeToString(fromcfg.flexType));
          tocfg.flexType = Board::FLEX_NONE;
          cstate.setConverted(oldFT, RadioDataConversionState::LogField(i, Boards::flexTypeToString(tocfg.flexType)));
        }
        else
          tocfg.flexType = fromcfg.flexType;

        tocfg.inverted = fromcfg.inverted;
        // do not copy calibration - use defaults as safer
      }
      else if (cstate.fromGS()->inputConfig[i].type == Board::AIT_FLEX && cstate.fromGS()->inputConfig[i].flexType != Board::FLEX_NONE) {
        cstate.setUnsupported(oldData);
      }
    }
  }

  cstate.setSubComp(tr("Switches"));

  for (int i = 0; i < Boards::getCapability(cstate.fromType, Board::Switches); i++) {
    if (Boards::isSwitchConfigurable(i, cstate.fromType)) {
      cstate.withComponentField("");
      cstate.setItemType(Boards::isSwitchFlex(i, cstate.fromType) ? tr("Flex Switch") :
                         Boards::isSwitchFunc(i, cstate.fromType) ? tr("Function Switch") : tr("Switch"));
      RadioDataConversionState::LogField oldData(i, Boards::getSwitchName(i, cstate.fromType));
      const int idx = Boards::getSwitchIndex(Boards::getSwitchTag(i, cstate.fromType), Board::LVT_TAG, cstate.toType);

      if (idx > -1) {
        const SwitchConfig &fromcfg = cstate.fromGS()->switchConfig[i];
        SwitchConfig &tocfg = switchConfig[idx];
        strncpy(tocfg.name, fromcfg.name, sizeof(switchConfig[0].name));

        if (Boards::getSwitchInfo(i, cstate.fromType).type > Boards::getSwitchInfo(idx, cstate.toType).type) {
          cstate.withComponentField(Boards::getSwitchName(i, cstate.fromType));
          cstate.setItemType(Boards::isSwitchFlex(i, cstate.fromType) ? tr("Flex Switch") :
                             Boards::isSwitchFunc(i, cstate.fromType) ? tr("Function Switch") : tr("Switch"));
          RadioDataConversionState::LogField oldSWT(i, Boards::switchTypeToString(fromcfg.type));
          // switch type not supported on to profile so leave as hw default eg from 3 Pos and to 2 Pos
          // if switch position not supported it will be reported where used on each model
          cstate.setConverted(oldSWT, RadioDataConversionState::LogField(i, Boards::switchTypeToString(tocfg.type)));
        }
        else
          tocfg.type = fromcfg.type;

        tocfg.inverted = fromcfg.inverted;

        if (fromcfg.inputIdx != SWITCH_INPUTINDEX_NONE) {
          if (!Boards::getCapability(cstate.toType, Board::FlexSwitches) ||
              Boards::getInputIndex(Boards::getInputTag(fromcfg.inputIdx, cstate.fromType), Board::LVT_TAG, cstate.toType) < 0) {
            cstate.withComponentField(Boards::getSwitchName(i, cstate.fromType));
            RadioDataConversionState::LogField oldFT(i, Boards::getInputName(fromcfg.inputIdx, cstate.fromType));
            tocfg.inputIdx = SWITCH_INPUTINDEX_NONE;
            cstate.setConverted(oldFT, RadioDataConversionState::LogField(i, tr("None")));
          }
          else
            tocfg.inputIdx = fromcfg.inputIdx;
        }
      }
      else if (cstate.fromGS()->switchConfig[i].type != Board::SWITCH_NOT_AVAILABLE)
        cstate.setUnsupported(oldData);
    }
  }

  if (IS_TARANIS(cstate.toType)) {
    contrast = qBound<int>(getCurrentFirmware()->getCapability(MinContrast), contrast, getCurrentFirmware()->getCapability(MaxContrast));
  }

  // TODO: Would be nice at this point to have GUI pause and ask the user to set up any custom hardware they have on the destination radio.

  // Convert all global functions (do this after HW adjustments)
  for (int i = 0; i < CPN_MAX_SPECIAL_FUNCTIONS; i++) {
    customFn[i].convert(cstate.withComponentIndex(i));
  }

}

QString GeneralSettings::antennaModeToString() const
{
  return antennaModeToString(antennaMode);
}

QString GeneralSettings::bluetoothModeToString() const
{
  return bluetoothModeToString(bluetoothMode);
}

QString GeneralSettings::serialPortModeToString(int port_nr) const
{
  if (port_nr < 0 || port_nr >= SP_COUNT)
    return QString();

  return serialModeToString(serialPort[port_nr]);
}

QString GeneralSettings::internalModuleBaudrateToString() const
{
  return moduleBaudrateToString(internalModuleBaudrate);
}

//  static
QString GeneralSettings::antennaModeToString(int value)
{
  Board::Type board = getCurrentBoard();

  switch(value) {
    case ANTENNA_MODE_INTERNAL:
      return tr("Internal");
    case ANTENNA_MODE_ASK:
      return tr("Ask");
    case ANTENNA_MODE_PER_MODEL:
      return tr("Per model");
    case ANTENNA_MODE_EXTERNAL:
    // case ANTENNA_MODE_INTERNAL_EXTERNAL:
      return IS_HORUS_X12S(board) ? tr("Internal + External") : tr("External");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString GeneralSettings::bluetoothModeToString(int value)
{
  Board::Type board = getCurrentBoard();

  switch(value) {
    case BLUETOOTH_MODE_OFF:
      return tr("OFF");
    case BLUETOOTH_MODE_ENABLED:
    // case BLUETOOTH_MODE_TELEMETRY:
      return IS_TARANIS_X9E(board) ? tr("Enabled") : tr("Telemetry");
    case BLUETOOTH_MODE_TRAINER:
      return tr("Trainer");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString GeneralSettings::serialModeToString(int value)
{
  switch(value) {
    case AUX_SERIAL_OFF:
      return tr("OFF");
    case AUX_SERIAL_TELE_MIRROR:
      return tr("Telemetry Mirror");
    case AUX_SERIAL_TELE_IN:
      return tr("Telemetry In");
    case AUX_SERIAL_SBUS_TRAINER:
      return tr("SBUS Trainer");
    case AUX_SERIAL_LUA:
      return tr("LUA");
    case AUX_SERIAL_CLI:
      return tr("CLI");
    case AUX_SERIAL_GPS:
      return tr("GPS");
    case AUX_SERIAL_DEBUG:
      return tr("Debug");
    case AUX_SERIAL_SPACEMOUSE:
      return tr("SpaceMouse");
    case AUX_SERIAL_EXT_MODULE:
      return tr("External module");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString GeneralSettings::moduleBaudrateToString(int value)
{
  return moduleBaudratesList.value(value, CPN_STR_UNKNOWN_ITEM);
}

//  static
FieldRange GeneralSettings::getPPM_MultiplierRange()
{
  FieldRange result;

  result.min = 0;
  result.max = 5;
  result.decimals = 1;
  result.step = 0.1;
  result.offset = 10;

  return result;
}

//  static
FieldRange GeneralSettings::getTxCurrentCalibration()
{
  FieldRange result;

  result.max = 49;
  result.min = -result.max;
  result.unit = tr("mA");

  return result;
}

//  static
AbstractStaticItemModel * GeneralSettings::antennaModeItemModel(bool model_setup)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_ANTENNAMODE);

  for (int i = ANTENNA_MODE_FIRST; i <= ANTENNA_MODE_LAST; i++) {
    mdl->appendToItemList(antennaModeToString(i), i, model_setup ? i != ANTENNA_MODE_PER_MODEL : true);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * GeneralSettings::bluetoothModeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_BLUETOOTHMODE);

  for (int i = 0; i < BLUETOOTH_MODE_COUNT; i++) {
    mdl->appendToItemList(bluetoothModeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * GeneralSettings::serialModeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_SERIALMODE);


  for (int i = 0; i < AUX_SERIAL_COUNT; i++) {
    int contexts = AllAuxSerialModeContexts;

    if (i == AUX_SERIAL_EXT_MODULE) {
      contexts &= ~(AUX2Context | VCPContext);
    }
    else if (i == AUX_SERIAL_TELE_IN ||
             i == AUX_SERIAL_SBUS_TRAINER ||
             i == AUX_SERIAL_GPS ||
             i == AUX_SERIAL_SPACEMOUSE ||
             i == AUX_SERIAL_EXT_MODULE) {
      contexts &= ~VCPContext;
    }

    mdl->appendToItemList(serialModeToString(i), i, true, 0, contexts);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * GeneralSettings::internalModuleBaudrateItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_INTMODULEBAUDRATE);

  for (int i = 0; i < moduleBaudratesList.size(); i++) {
    mdl->appendToItemList(moduleBaudrateToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * GeneralSettings::stickDeadZoneItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_STICKDEADZONE);

  for (int i = 0; i <= 7; i++) {
    mdl->appendToItemList(i ? QString::number((int)(2 << (i - 1))) : QString::number(0), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString GeneralSettings::uartSampleModeToString() const
{
  return uartSampleModeToString(uartSampleMode);
}

//  static
QString GeneralSettings::uartSampleModeToString(int value)
{
  switch(value) {
    case UART_SAMPLE_MODE_NORMAL:
      return tr("Normal");
    case UART_SAMPLE_MODE_ONEBIT:
      return tr("OneBit");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * GeneralSettings::uartSampleModeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_UARTSAMPLEMODE);

  for (int i = 0; i < UART_SAMPLE_MODE_COUNT; i++) {
    mdl->appendToItemList(uartSampleModeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString GeneralSettings::hatsModeToString() const
{
  return hatsModeToString(hatsMode);
}

bool GeneralSettings::fix6POSCalibration()
{
  bool changed = false;
  // Fix default 6POS calibration
  for (int i = CPN_MAX_STICKS; i < CPN_MAX_STICKS + CPN_MAX_POTS; i += 1) {
    if ((potConfig[i-CPN_MAX_STICKS] == Board::POT_MULTIPOS_SWITCH) && (calibMid[i] == 0x200) && (calibSpanNeg[i] == 0x180) && (calibSpanPos[i] == 0x180)) {
      calibMid[i] = 773;;
      calibSpanNeg[i] = 5388;
      calibSpanPos[i] = 9758;
      changed = true;
    }
  }
  return changed;
}

//  static
QString GeneralSettings::hatsModeToString(int value)
{
  switch(value) {
    case HATSMODE_TRIMS_ONLY:
      return tr("Trims only");
    case HATSMODE_KEYS_ONLY:
      return tr("Keys only");
    case HATSMODE_SWITCHABLE:
      return tr("Switchable");
    case HATSMODE_GLOBAL:
      return tr("Global");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * GeneralSettings::hatsModeItemModel(bool radio_setup)
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_HATSMODE);

  for (int i = 0; i < HATSMODE_COUNT; i++) {
    mdl->appendToItemList(hatsModeToString(i), i, i == HATSMODE_GLOBAL && radio_setup ? false : true);
  }

  mdl->loadItemList();
  return mdl;
}

// Legacy mapping
static const StringTagMappingTable potTypesConversionTable = {
    {std::to_string(Board::POT_NONE),               std::to_string(Board::FLEX_NONE)},
    {std::to_string(Board::POT_WITH_DETENT),        std::to_string(Board::FLEX_POT_CENTER)},
    {std::to_string(Board::POT_MULTIPOS_SWITCH),    std::to_string(Board::FLEX_MULTIPOS)},
    {std::to_string(Board::POT_WITHOUT_DETENT),     std::to_string(Board::FLEX_POT)},
    {std::to_string(Board::POT_SLIDER_WITH_DETENT), std::to_string(Board::FLEX_SLIDER)},
};

static const StringTagMappingTable sliderTypesConversionTable = {
    {std::to_string(Board::SLIDER_NONE),            std::to_string(Board::FLEX_NONE)},
    {std::to_string(Board::SLIDER_WITH_DETENT),     std::to_string(Board::FLEX_SLIDER)},
};

// This copies the pre v2.10 config to the new structs
bool GeneralSettings::convertLegacyConfiguration(Board::Type board)
{
  for (int i = 0; i < CPN_MAX_STICKS && i < Boards::getCapability(board, Board::Sticks); i++) {
    inputConfig[i].type = Board::AIT_STICK;
    YamlValidateName(stickName[i], board);
    strncpy(inputConfig[i].name, stickName[i], HARDWARE_NAME_LEN);
    inputConfig[i].calib.mid = calibMid[i];
    inputConfig[i].calib.spanNeg = calibSpanNeg[i];
    inputConfig[i].calib.spanPos = calibSpanPos[i];
  }

  for (int i = 0; i < CPN_MAX_POTS && i < Boards::getCapability(board, Board::Pots); i++) {
    int idx = 0;
    if (IS_FAMILY_HORUS_OR_T16(board)) {
      //  0 - 2 P1 - P3
      //  3 - 6 EXT1 - EXT4
      //  7 - 8 S1 - S2
      if (i < 3)
        idx = Boards::getInputPotIndex(i + 1, board);
      else
        idx = Boards::getInputExtIndex(i - 3 + 1, board);
    }
    else
      idx = Boards::getInputPotIndex(i + 1, board);

    // qDebug() << "i:" << i << "idx:" << idx << "pot name:" << potName[i];

    if (idx >= 0) {
      inputConfig[idx].type = Board::AIT_FLEX;
      YamlValidateName(potName[i], board);
      strncpy(inputConfig[idx].name, potName[i], HARDWARE_NAME_LEN);
      inputConfig[idx].calib.mid = calibMid[i];
      inputConfig[idx].calib.spanNeg = calibSpanNeg[i];
      inputConfig[idx].calib.spanPos = calibSpanPos[i];
      int ft = std::stoi(DataHelpers::getStringTagMappingTag(potTypesConversionTable, potConfig[i]));
      if (ft > -1)
        inputConfig[idx].flexType = (Board::FlexType)ft;
    }
  }

  for (int i = 0; i < CPN_MAX_SLIDERS && i < Boards::getCapability(board, Board::Sliders); i++) {
    int offset = 0;
    if (IS_HORUS_X12S(board)) {
      // 0 - 1  extra sliders eg L1 - L2 or S1 - S2
      // 2 - 3  LS - RS
      // hw json flips the pairs
      if (i < 2)
        offset = 2;
      else
        offset = -2;
    }

    int idx = Boards::getInputSliderIndex(i + offset + 1, board);

    // qDebug() << "i:" << i << "idx:" << idx << "slider name:" << sliderName[i];

    if (idx >= 0) {
      inputConfig[idx].type = Board::AIT_FLEX;
      YamlValidateName(sliderName[i], board);
      strncpy(inputConfig[idx].name, sliderName[i], HARDWARE_NAME_LEN);
      inputConfig[idx].calib.mid = calibMid[i];
      inputConfig[idx].calib.spanNeg = calibSpanNeg[i];
      inputConfig[idx].calib.spanPos = calibSpanPos[i];
      int ft = std::stoi(DataHelpers::getStringTagMappingTag(sliderTypesConversionTable, sliderConfig[i]));
      if (ft > -1)
        inputConfig[idx].flexType = (Board::FlexType)ft;
    }
  }

  for (int i = 0; i < CPN_MAX_SWITCHES && i < Boards::getCapability(board, Board::Switches); i++) {
    switchConfig[i].type = (Board::SwitchType)swtchConfig[i];
    YamlValidateName(swtchName[i], board);
    strncpy(switchConfig[i].name, swtchName[i], HARDWARE_NAME_LEN);
  }

  return true;
}

void GeneralSettings::validateFlexSwitches()
{
  for (int i = 0; i < CPN_MAX_SWITCHES_FLEX; i++) {
    if (inputConfig[switchConfig[i].inputIdx].flexType != Board::FLEX_SWITCH)
      switchConfig[i].inputIdx = -1;

    int idx = Boards::getSwitchIndex(QString("FL%1").arg(i), Board::LVT_TAG);
    if (idx >= 0) {
      if (switchConfig[idx].type == Board::SWITCH_NOT_AVAILABLE)
        switchConfig[i].inputIdx = -1;
    }
  }
}

/*
    TrainerMix
*/

QString TrainerMix::modeToString() const
{
  return modeToString(mode);
}

QString TrainerMix::srcToString() const
{
  return srcToString(src);
}

//  static
FieldRange TrainerMix::getWeightRange()
{
  FieldRange result;

  result.decimals = 0;
  result.max = 125;
  result.min = -result.max;
  result.step = 1;

  return result;
}

//  static
QString TrainerMix::modeToString(int value)
{
  switch(value) {
    case TRN_MIX_MODE_OFF:
      return tr("OFF");
    case TRN_MIX_MODE_ADD:
      return tr("+= (Sum)");
    case TRN_MIX_MODE_SUBST:
      return tr(":= (Replace)");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString TrainerMix::srcToString(int value)
{
  return tr("CH%1").arg(value + 1);
}

//  static
AbstractStaticItemModel * TrainerMix::modeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TRAINERMIX_MODE);

  for (int i = 0; i < TRN_MIX_MODE_COUNT; i++) {
    mdl->appendToItemList(modeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * TrainerMix::srcItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_TRAINERMIX_SRC);

  for (int i = 0; i < Boards::getCapability(getCurrentBoard(), Board::Sticks); i++) {
    mdl->appendToItemList(srcToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString GeneralSettings::stickModeToString() const
{
  return stickModeToString(stickMode);
}

//  static
QString GeneralSettings::stickModeToString(int value)
{
  switch(value) {
    case STICK_MODE_1:
      return tr("Mode 1 (RUD ELE THR AIL)");
    case STICK_MODE_2:
      return tr("Mode 2 (RUD THR ELE AIL)");
    case STICK_MODE_3:
      return tr("Mode 3 (AIL ELE THR RUD)");
    case STICK_MODE_4:
      return tr("Mode 4 (AIL THR ELE RUD)");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * GeneralSettings::stickModeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_STICKMODE);

  for (int i = 0; i < STICK_MODE_COUNT; i++) {
    mdl->appendToItemList(stickModeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString GeneralSettings::templateSetupToString() const
{
  return templateSetupToString(templateSetup, Boards::isAir());
}

//  static
QString GeneralSettings::templateSetupToString(int value, bool isBoardAir)
{
  QString ret;
  const QList<int> channels = useChannels(isBoardAir);
  const QList<QChar> controls = useControls(isBoardAir);

  if (value < (channels.size() / controls.size())) {
    for (int i = 0; i < controls.size(); i++) {
      if (i > 0)
        ret.append(" ");

      int idx = channels[controls.size() * value + i] - 1;
      if (idx >= 0 && idx < controls.size())
        ret.append(controls[channels[controls.size() * value + i] - 1]);
    }
  }

  return ret;
}

//  static
AbstractStaticItemModel * GeneralSettings::templateSetupItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_GS_TEMPLATESETUP);

  for (int i = 0; i < (channelsCount(true) / controlsCount(true)); i++) {
    mdl->appendToItemList(templateSetupToString(i, true), i, true, 0, RadioTypeContextAir);
  }

  for (int i = 0; i < (channelsCount(false) / controlsCount(false)); i++) {
    mdl->appendToItemList(templateSetupToString(i, false), i, true, 0, RadioTypeContextSurface);
  }

  mdl->loadItemList();
  return mdl;
}
