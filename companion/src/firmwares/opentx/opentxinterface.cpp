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

#include "opentxinterface.h"
#include "appdata.h"
#include "constants.h"

#include <bitset>
#include <QMessageBox>
#include <QTime>
#include <QUrl>

#include "storage.h"  // does this need to be last include?

#define FILE_TYP_GENERAL 1
#define FILE_TYP_MODEL   2

/// fileId of general file
#define FILE_GENERAL   0
/// convert model number 0..MAX_MODELS-1  int fileId
#define FILE_MODEL(n) (1+n)

using namespace Board;

Firmware * OpenTxFirmware::getFirmwareVariant(const QString &id)
{
  if (id == getId()) {
    return this;
  }
  else if (id.contains(getId() + "-") || (!id.contains("-") && id.contains(getId()))) {
    Firmware * result = new OpenTxFirmware(id, this);
    // TODO result.variant = firmware->getVariant(id);
    return result;
  }
  else {
    return NULL;
  }
}

int OpenTxFirmware::getCapability(::Capability capability)
{
  switch (capability) {
    case Models:
      if (IS_FAMILY_HORUS_OR_T16(board))
        return 0;
      else
        return 60;
    case Imperial:
      return 0;
    case HasModelImage:
      return (board == BOARD_TARANIS_X9D || IS_TARANIS_PLUS(board) || board == BOARD_TARANIS_X9DP_2019 || IS_FAMILY_HORUS_OR_T16(board));
    case ModelImageNameLen:
      return (IS_FAMILY_HORUS_OR_T16(board) ? 14 : 10); //  including extension if saved and <= CPN_MAX_BITMAP_LEN
    case ModelImageKeepExtn:
      return (IS_FAMILY_HORUS_OR_T16(board) ? true : false);
    case HasBeeper:
      return false;
    case HasPxxCountry:
      return 1;
    case HasGeneralUnits:
      return true;
    case HasNegAndSwitches:
      return true;
    case PPMExtCtrl:
      return 1;
    case PPMFrameLength:
      return 40;
    case FlightModes:
      return 9;
    case FlightModesHaveFades:
      return 1;
    case Heli:
      if (Boards::getCapability(board, Board::Surface))
        return false;
      else if (IS_HORUS_OR_TARANIS(board))
        return id.contains("noheli") ? 0 : 1;
      else
        return id.contains("heli") ? 1 : 0;
    case Gvars:
      if (IS_HORUS_OR_TARANIS(board))
        return id.contains("nogvars") ? 0 : 9;
      else if (id.contains("gvars"))
        return 9;
      else
        return 0;
    case ModelName:
      return (IS_FAMILY_HORUS_OR_T16(board) ? 15 : (HAS_LARGE_LCD(board) ? 12 : 10));
    case FlightModesName:
      return (IS_HORUS_OR_TARANIS(board) ? 10 : 6);
    case GvarsName:
      return 3;
    case GvarsInCS:
    case HasFAIMode:
      return 1;
    case GvarsAreNamed:
    case GvarsFlightModes:
      return 1;
    case Mixes:
      return 64;
    case OffsetWeight:
      return 500;
    case Timers:
      return 3;
    case TimersName:
      if (HAS_LARGE_LCD(board))
        return 8;
      else
        return 3;
    case PermTimers:
      return true;
    case CustomFunctions:
      return 64;
    case SafetyChannelCustomFunction:
      return id.contains("nooverridech") ? 0 : 1;
    case LogicalSwitches:
      return 64;
    case CustomAndSwitches:
      return getCapability(LogicalSwitches);
    case LogicalSwitchesExt:
      return true;
    case RotaryEncoders:
        return 0;
    case Outputs:
      return 32;
    case NumCurvePoints:
      return 512;
    case VoicesAsNumbers:
      return 0;
    case VoicesMaxLength:
      return 8;
    case MultiLangVoice:
      return 1;
    case SoundPitch:
      return 1;
    case Haptic:
      return board != Board::BOARD_TARANIS_X9D || id.contains("haptic");
    case ModelTrainerEnable:
      if (IS_HORUS_OR_TARANIS(board) && board!=Board::BOARD_TARANIS_XLITE)
        return 1;
      else
        return 0;
    case MaxVolume:
      return 23;
    case MaxContrast:
      if (IS_TARANIS_SMALL(board))
        return 30;
      else
        return 45;
    case MinContrast:
      if (IS_TARANIS_X9(board))
        return 0;
      else
        return 10;
    case HasSoundMixer:
      return 1;
    case ExtraInputs:
      return 1;
    case TrimsRange:
      return 128;
    case ExtendedTrimsRange:
      return 512;
    case Simulation:
      return 1;
    case NumCurves:
      return 32;
    case HasMixerNames:
      return (IS_TARANIS_X9(board) ? 8 : 6);
    case HasExpoNames:
      return (IS_TARANIS_X9(board) ? 8 : 6);
    case HasNoExpo:
      return (IS_HORUS_OR_TARANIS(board) ? false : true);
    case ChannelsName:
      return (HAS_LARGE_LCD(board) ? 6 : 4);
    case HasCvNames:
      return 1;
    case Telemetry:
      return 1;
    case TelemetryBars:
      return 1;
    case TelemetryCustomScreens:
      if (IS_FAMILY_HORUS_OR_T16(board))
        return 0;
      else
        return 4;
    case TelemetryCustomScreensBars:
      return (getCapability(TelemetryCustomScreens) ? 4 : 0);
    case TelemetryCustomScreensLines:
      return (getCapability(TelemetryCustomScreens) ? 4 : 0);
    case TelemetryCustomScreensFieldsPerLine:
      return HAS_LARGE_LCD(board) ? 3 : 2;
    case NoTelemetryProtocol:
      return IS_HORUS_OR_TARANIS(board) ? 1 : 0;
    case TelemetryUnits:
      return 0;
    case TelemetryMaxMultiplier:
      return 32;
    case PPMCenter:
      return (IS_HORUS_OR_TARANIS(board) ? 500 : (id.contains("ppmca") ? 125 : 0));
    case SYMLimits:
      return 1;
    case OptrexDisplay:
      return (board == BOARD_SKY9X ? true : false);
    case HasVario:
      return Boards::isAir(board);
    case HasVarioSink:
      return Boards::isAir(board);
    case HasFailsafe:
      return 32;
    case NumModules:
      return 2;
    case NumFirstUsableModule:
      return (IS_JUMPER_T12(board) && !id.contains("internalmulti") ? 1 : 0);
    case HasModuleR9MFlex:
      return id.contains("flexr9m");
    case HasModuleR9MMini:
      return IS_TARANIS_XLITE(board) && !id.contains("stdr9m");
    case HasPPMStart:
      return true;
    case HastxCurrentCalibration:
      return (IS_SKY9X(board) ? true : false);
    case HasVolume:
      return true;
    case HasBrightness:
      return true;
    case PerModelTimers:
      return 1;
    case SlowScale:
      return 10;
    case SlowRange:
      return 250;
    case CSFunc:
      return 18;
    case HasSDLogs:
      return true;
    case GetThrSwitch:
      return (IS_HORUS_OR_TARANIS(board) ? SWITCH_SF1 : SWITCH_THR);
    case HasDisplayText:
      return 1;
    case HasTopLcd:
      return IS_TARANIS_X9E(board) ? 1 : 0;
    case GlobalFunctions:
      return 64;
    case VirtualInputs:
      return 32;
    case InputsLength:
      return HAS_LARGE_LCD(board) ? 4 : 3;
    case TrainerInputs:
      return 16;
    case LuaScripts:
      return IS_HORUS_OR_TARANIS(board) && id.contains("lua") ? 7 : 0;
    case LuaInputsPerScript:
      return IS_HORUS_OR_TARANIS(board) ? 10 : 0;
    case LuaOutputsPerScript:
      return IS_HORUS_OR_TARANIS(board) ? 6 : 0;
    case LimitsPer1000:
    case EnhancedCurves:
      return 1;
    case HasFasOffset:
      return true;
    case HasMahPersistent:
      return true;
    case MavlinkTelemetry:
      return id.contains("mavlink") ? 1 : 0;
    case SportTelemetry:
      return 1;
    case HasInputDiff:
    case HasMixerExpo:
      return (IS_HORUS_OR_TARANIS(board) ? true : false);
    case HasBatMeterRange:
      return (IS_HORUS_OR_TARANIS(board) ? true : id.contains("battgraph"));
    case DangerousFunctions:
      return id.contains("danger") ? 1 : 0;
    case HasModelLabels:
      return IS_FAMILY_HORUS_OR_T16(board);
    case HasSwitchableJack:
      return IS_TARANIS_XLITES(board);
    case HasSportConnector:
      return IS_ACCESS_RADIO(board, id) || IS_TARANIS_X7(board) || IS_HORUS_X10(board) || IS_TARANIS_XLITE(board);
    case PwrButtonPress:
      return IS_HORUS_OR_TARANIS(board) && (board!=Board::BOARD_TARANIS_X9D) && (board!=Board::BOARD_TARANIS_X9DP);
    case Sensors:
      if (IS_FAMILY_HORUS_OR_T16(board) || IS_TARANIS_X9(board))
        return 60;
      else
        return 40;
    case HasAuxSerialMode:
      return (IS_FAMILY_HORUS_OR_T16(board) && !(IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board))) ||
             (IS_TARANIS_X9(board) && !IS_TARANIS_X9DP_2019(board)) ||
             IS_RADIOMASTER_ZORRO(board) || IS_RADIOMASTER_TX12_MK2(board) || IS_RADIOMASTER_MT12(board);
    case HasAux2SerialMode:
      return IS_FAMILY_T16(board);
    case HasVCPSerialMode:
      return IS_FAMILY_HORUS_OR_T16(board) || IS_RADIOMASTER_ZORRO(board) ||
             IS_JUMPER_TPRO(board) || IS_RADIOMASTER_TX12_MK2(board) ||
             IS_RADIOMASTER_BOXER(board) || IS_RADIOMASTER_POCKET(board) ||
             IS_RADIOMASTER_MT12(board);
    case HasBluetooth:
      return (IS_FAMILY_HORUS_OR_T16(board) || IS_TARANIS_X7(board) || IS_TARANIS_XLITE(board)|| IS_TARANIS_X9E(board) ||
              IS_TARANIS_X9DP_2019(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board) || IS_FLYSKY_PL18(board)) ? true : false;
    case HasADCJitterFilter:
      return IS_HORUS_OR_TARANIS(board);
    case HasTelemetryBaudrate:
      return IS_HORUS_OR_TARANIS(board);
    case TopBarZones:
      return Boards::getCapability(board, Board::LcdWidth) > Boards::getCapability(board, Board::LcdHeight) ? 4 : 2;
    case HasModelsList:
      return IS_FAMILY_HORUS_OR_T16(board);
    case HasFlySkyGimbals:
      return (IS_RADIOMASTER_TX16S(board) && id.contains("flyskygimbals"));
    case RotaryEncoderNavigation:
      return (IS_TARANIS_X9E(board) || IS_TARANIS_X9DP_2019(board) ||
              IS_TARANIS_X7(board) || IS_JUMPER_TPRO(board) ||
              IS_TARANIS_X9LITE(board) || IS_RADIOMASTER_TX12(board) ||
              IS_RADIOMASTER_TX12_MK2(board) || IS_RADIOMASTER_ZORRO(board) ||
              IS_RADIOMASTER_BOXER(board) || IS_RADIOMASTER_TX16S(board) ||
              IS_JUMPER_T18(board) || IS_JUMPER_T20(board) ||
              IS_RADIOMASTER_POCKET(board) || IS_RADIOMASTER_MT12(board));
    case HasSoftwareSerialPower:
      return IS_RADIOMASTER_TX16S(board);
    case HasIntModuleMulti:
      return id.contains("internalmulti") || IS_RADIOMASTER_TX16S(board) || IS_JUMPER_T18(board) ||
              IS_RADIOMASTER_TX12(board) || IS_JUMPER_TLITE(board) || IS_BETAFPV_LR3PRO(board) ||
              (IS_RADIOMASTER_ZORRO(board) && !id.contains("internalelrs")) ||
              (IS_RADIOMASTER_BOXER(board) && !id.contains("internalelrs")) ||
              (IS_RADIOMASTER_POCKET(board) && !id.contains("internalelrs")) ||
              (IS_RADIOMASTER_MT12(board) && !id.contains("internalelrs"));
    case HasIntModuleCRSF:
      return id.contains("internalcrsf");
    case HasIntModuleELRS:
      return id.contains("internalelrs") || IS_RADIOMASTER_TX12_MK2(board) ||
             IS_IFLIGHT_COMMANDO8(board) || IS_RADIOMASTER_BOXER(board) ||
             IS_RADIOMASTER_POCKET(board) || IS_JUMPER_T20(board) ||
             IS_RADIOMASTER_MT12(board);
    case HasIntModuleFlySky:
      return  id.contains("afhds2a") || id.contains("afhds3") ||
              IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board);
    case BacklightLevelMin:
      if (IS_HORUS_X12S(board))
        return 5;
      if (IS_FAMILY_T16(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board))
        return 1;
      return 46;
    default:
      return 0;
  }
}

QString OpenTxFirmware::getCapabilityStr(::Capability capability)
{
  switch (capability) {
    case ModelImageFilters:
      return IS_FAMILY_HORUS_OR_T16(board) ? "*.bmp|*.jpg|*.png" : "*.bmp";
    default:
      return QString();
  }
}

QTime OpenTxFirmware::getMaxTimerStart()
{
  if (IS_HORUS_OR_TARANIS(board))
    return QTime(23, 59, 59);
  else
    return QTime(8, 59, 59);
}

template<typename T, size_t SIZE>
size_t getSizeA(T (&)[SIZE])
{
  return SIZE;
}

QString OpenTxFirmware::getReleaseNotesUrl()
{
  return QString("%1/downloads").arg(EDGETX_HOME_PAGE_URL);
}

// Firmware registrations
// NOTE: "recognized" build options are defined in /radio/util/fwoptions.py

void registerOpenTxFirmware(OpenTxFirmware * firmware, bool deprecated = false)
{
  if (!deprecated)
    Firmware::addRegisteredFirmware(firmware);
}

void addOpenTxCommonOptions(OpenTxFirmware * firmware)
{
  static const Firmware::OptionsGroup fai_options = {
    Firmware::Option("faichoice", Firmware::tr("Possibility to enable FAI MODE (no telemetry) at field")),
    Firmware::Option("faimode",   Firmware::tr("FAI MODE (no telemetry) always enabled"))
  };
  firmware->addOptionsGroup(fai_options);
  firmware->addOption("nooverridech", Firmware::tr("No OverrideCH functions available"));
}

enum RfOptions {
  NONE = 0,
  EU = 1 << 0,
  FLEX = 1 << 1,
  AFHDS2A = 1 << 2,
  AFHDS3 = 1 << 3,
};

void addOpenTxRfOptions(OpenTxFirmware * firmware, uint8_t options)
{
  static const Firmware::Option opt_eu("eu", Firmware::tr("Removes D8 FrSky protocol support which is not legal for use in the EU on radios sold after Jan 1st, 2015"));
  static const Firmware::Option opt_fl("flexr9m", Firmware::tr("Enable non certified firmwares"));
  static const Firmware::Option opt_afhds2a("afhds2a", Firmware::tr("Enable AFHDS2A support"));
  static const Firmware::Option opt_afhds3("afhds3", Firmware::tr("Enable AFHDS3 support"));


  if ((options & (EU + FLEX)) == EU + FLEX)
    firmware->addOptionsGroup({opt_eu, opt_fl});
  else if ((options & EU) != 0)
    firmware->addOption(opt_eu);
  else if ((options & FLEX) != 0)
    firmware->addOption(opt_fl);
  if ((options & AFHDS2A) != 0)
    firmware->addOption(opt_afhds2a);
  if ((options & AFHDS3) != 0)
    firmware->addOption(opt_afhds3);
}

void addOpenTxFontOptions(OpenTxFirmware * firmware)
{
  firmware->addOption("sqt5font", Firmware::tr("Use alternative SQT5 font"));
}

void addOpenTxFrskyOptions(OpenTxFirmware * firmware)
{
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
}

void addOpenTxTaranisOptions(OpenTxFirmware * firmware)
{
  addOpenTxFrskyOptions(firmware);
  addOpenTxFontOptions(firmware);
}

// TODO: refactoring OpenTxFirmware constructor - need to check if used elsewhere and how
#define FIRMWAREID(flavour)  std::string(FIRMWARE_ID_PREFIX).append(flavour).c_str()

void registerOpenTxFirmwares()
{
  OpenTxFirmware * firmware;

  static const Firmware::Option opt_bt("bluetooth", Firmware::tr("Support for bluetooth module"));
  static const Firmware::Option opt_internal_gps("internalgps", Firmware::tr("Support internal GPS"));

  /* BETAFPV LR3PRO board */
  firmware = new OpenTxFirmware(FIRMWAREID("lr3pro"), QCoreApplication::translate("Firmware", "BETAFPV LiteRadio3 Pro"), BOARD_BETAFPV_LR3PRO);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Fatfish F16 board */
  firmware = new OpenTxFirmware(FIRMWAREID("f16"), Firmware::tr("Fatfish F16"), BOARD_FATFISH_F16);
  addOpenTxFrskyOptions(firmware);
  addOpenTxRfOptions(firmware, FLEX);
  firmware->addOptionsGroup({opt_bt, opt_internal_gps});
  firmware->addOption("flyskygimbals", Firmware::tr("Support hardware mod: FlySky Paladin EV Gimbals"));
  registerOpenTxFirmware(firmware);

  /* FlySky NV14 board */
  firmware = new OpenTxFirmware(FIRMWAREID("nv14"), QCoreApplication::translate("Firmware", "FlySky NV14"), BOARD_FLYSKY_NV14);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption(opt_bt);
  addOpenTxRfOptions(firmware, FLEX + AFHDS2A + AFHDS3);
  registerOpenTxFirmware(firmware);

  /* FlySky EL18 board */
  firmware = new OpenTxFirmware(FIRMWAREID("el18"), QCoreApplication::translate("Firmware", "FlySky EL18"), BOARD_FLYSKY_EL18);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption(opt_bt);
  addOpenTxRfOptions(firmware, FLEX + AFHDS2A + AFHDS3);
  registerOpenTxFirmware(firmware);

  /* FlySky PL18 board */
  firmware = new OpenTxFirmware(FIRMWAREID("pl18"), Firmware::tr("FlySky PL18"), BOARD_FLYSKY_PL18);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption(opt_bt);
  addOpenTxRfOptions(firmware, FLEX + AFHDS3);
  registerOpenTxFirmware(firmware);

  /* FrSky Horus X10 board */
  firmware = new OpenTxFirmware(FIRMWAREID("x10"), Firmware::tr("FrSky Horus X10 / X10S"), BOARD_X10);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalaccess", Firmware::tr("Support for ACCESS internal module replacement"));
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX);

  /* FrSky Horus X10 Express board */
  firmware = new OpenTxFirmware(FIRMWAREID("x10express"), Firmware::tr("FrSky Horus X10 Express / X10S Express"), BOARD_X10_EXPRESS, "x10express");
  addOpenTxFrskyOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Horus X12 board */
  firmware = new OpenTxFirmware(FIRMWAREID("x12s"), Firmware::tr("FrSky Horus X12S"), BOARD_HORUS_X12S);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalaccess", Firmware::tr("Support for ACCESS internal module replacement"));
  firmware->addOption("pcbdev", Firmware::tr("Use ONLY with first DEV pcb version"));
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX);

  /* FrSky Taranis X7 board */
  firmware = new OpenTxFirmware(FIRMWAREID("x7"), Firmware::tr("FrSky Taranis X7 / X7S"), BOARD_TARANIS_X7);
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX);

  /* FrSky Taranis X7 Access board */
  firmware = new OpenTxFirmware(FIRMWAREID("x7access"), Firmware::tr("FrSky Taranis X7 / X7S Access"), BOARD_TARANIS_X7_ACCESS, "x7access");
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Taranis X-Lite board */
  firmware = new OpenTxFirmware(FIRMWAREID("xlite"), Firmware::tr("FrSky Taranis X-Lite"), BOARD_TARANIS_XLITE);
  // firmware->addOption("stdr9m", Firmware::tr("Use JR-sized R9M module"));
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX);

  /* FrSky Taranis X-Lite S/PRO board */
  firmware = new OpenTxFirmware(FIRMWAREID("xlites"), Firmware::tr("FrSky Taranis X-Lite S/PRO"), BOARD_TARANIS_XLITES);
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Taranis X9-Lite board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9lite"), Firmware::tr("FrSky Taranis X9-Lite"), BOARD_TARANIS_X9LITE);
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Taranis X9-LiteS board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9lites"), Firmware::tr("FrSky Taranis X9-Lite S"), BOARD_TARANIS_X9LITES);
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Taranis X9D board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9d"), Firmware::tr("FrSky Taranis X9D"), BOARD_TARANIS_X9D);
  firmware->addOption("noras", Firmware::tr("Disable RAS (SWR)"));
  firmware->addOption("haptic", Firmware::tr("Haptic module installed"));
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX + AFHDS2A + AFHDS3);

  /* FrSky Taranis X9D+ board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9d+"), Firmware::tr("FrSky Taranis X9D+"), BOARD_TARANIS_X9DP, "x9dp");
  firmware->addOption("noras", Firmware::tr("Disable RAS (SWR)"));
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX + AFHDS2A + AFHDS3);

  /* FrSky Taranis X9D+ 2019 board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9d+2019"), Firmware::tr("FrSky Taranis X9D+ 2019"), BOARD_TARANIS_X9DP_2019, "x9dp2019");
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* FrSky Taranis X9E board */
  firmware = new OpenTxFirmware(FIRMWAREID("x9e"), Firmware::tr("FrSky Taranis X9E"), BOARD_TARANIS_X9E);
  firmware->addOption("shutdownconfirm", Firmware::tr("Confirmation before radio shutdown"));
  firmware->addOption("horussticks", Firmware::tr("Horus gimbals installed (Hall sensors)"));
  addOpenTxTaranisOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, EU + FLEX);

  /* HelloRadioSky V16 board */
  firmware = new OpenTxFirmware(FIRMWAREID("v16"), Firmware::tr("HelloRadioSky V16"), BOARD_HELLORADIOSKY_V16);
  addOpenTxFrskyOptions(firmware);
  addOpenTxRfOptions(firmware, FLEX);
  firmware->addOptionsGroup({opt_bt, opt_internal_gps});
  registerOpenTxFirmware(firmware);

  /* iFlight Commando8 board */
  firmware = new OpenTxFirmware(FIRMWAREID("commando8"), QCoreApplication::translate("Firmware", "iFlight Commando8"), BOARD_IFLIGHT_COMMANDO8);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, NONE);

  /* Jumper T-Lite board */
  firmware = new OpenTxFirmware(FIRMWAREID("tlite"), QCoreApplication::translate("Firmware", "Jumper T-Lite"), BOARD_JUMPER_TLITE);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T-Pro board */
  firmware = new OpenTxFirmware(FIRMWAREID("tpro"), QCoreApplication::translate("Firmware", "Jumper T-Pro"), BOARD_JUMPER_TPRO);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T-Pro V2 board */
  firmware = new OpenTxFirmware(FIRMWAREID("tprov2"), QCoreApplication::translate("Firmware", "Jumper T-Pro V2"), BOARD_JUMPER_TPROV2);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T-Pro S board */
  firmware = new OpenTxFirmware(FIRMWAREID("tpros"), QCoreApplication::translate("Firmware", "Jumper T-Pro S"), BOARD_JUMPER_TPROS);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper Bumblebee board */
  firmware = new OpenTxFirmware(FIRMWAREID("bumblebee"), QCoreApplication::translate("Firmware", "Jumper Bumblebee"), BOARD_JUMPER_BUMBLEBEE);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T12 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t12"), QCoreApplication::translate("Firmware", "Jumper T12 / T12 Pro"), BOARD_JUMPER_T12);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  firmware->addOption("internalmulti", Firmware::tr("Support for MULTI internal module"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T12 MAX board */
  firmware = new OpenTxFirmware(FIRMWAREID("t12max"), Firmware::tr("Jumper T12 MAX"), BOARD_JUMPER_T12MAX);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalelrs", Firmware::tr("Select if internal ELRS module is installed"));
  addOpenTxRfOptions(firmware, NONE);
  registerOpenTxFirmware(firmware);

  /* Jumper T14 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t14"), Firmware::tr("Jumper T14"), BOARD_JUMPER_T14);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalelrs", Firmware::tr("Select if internal ELRS module is installed"));
  addOpenTxRfOptions(firmware, NONE);
  registerOpenTxFirmware(firmware);

  /* Jumper T15 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t15"), Firmware::tr("Jumper T15"), BOARD_JUMPER_T15);
  addOpenTxFrskyOptions(firmware);
  addOpenTxRfOptions(firmware, FLEX);
  registerOpenTxFirmware(firmware);

  /* Jumper T16 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t16"), Firmware::tr("Jumper T16 / T16+ / T16 Pro"), BOARD_JUMPER_T16);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalmulti", Firmware::tr("Support for MULTI internal module"));
  firmware->addOption(opt_bt);
  addOpenTxRfOptions(firmware, FLEX);
  registerOpenTxFirmware(firmware);

  /* Jumper T18 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t18"), Firmware::tr("Jumper T18"), BOARD_JUMPER_T18);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption(opt_bt);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Jumper T20 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t20"), Firmware::tr("Jumper T20"), BOARD_JUMPER_T20);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalelrs", Firmware::tr("Select if internal ELRS module is installed"));
  addOpenTxRfOptions(firmware, NONE);
  registerOpenTxFirmware(firmware);

  /* Jumper T20 V2 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t20v2"), Firmware::tr("Jumper T20 V2"), BOARD_JUMPER_T20V2);
  addOpenTxFrskyOptions(firmware);
  firmware->addOption("internalelrs", Firmware::tr("Select if internal ELRS module is installed"));
  addOpenTxRfOptions(firmware, NONE);
  registerOpenTxFirmware(firmware);

  /* Radiomaster Boxer board */
  firmware = new OpenTxFirmware(FIRMWAREID("boxer"), QCoreApplication::translate("Firmware", "Radiomaster Boxer"), Board::BOARD_RADIOMASTER_BOXER);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX + AFHDS2A + AFHDS3);

  /* Radiomaster Pocket board */
  firmware = new OpenTxFirmware(FIRMWAREID("pocket"), QCoreApplication::translate("Firmware", "Radiomaster Pocket"), Board::BOARD_RADIOMASTER_POCKET);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX + AFHDS2A + AFHDS3);

  /* Radiomaster MT12 board */
  firmware = new OpenTxFirmware(FIRMWAREID("mt12"), QCoreApplication::translate("Firmware", "Radiomaster MT12"), Board::BOARD_RADIOMASTER_MT12);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, NONE);

  /* Radiomaster T8 board */
  firmware = new OpenTxFirmware(FIRMWAREID("t8"), QCoreApplication::translate("Firmware", "Radiomaster T8"), BOARD_RADIOMASTER_T8);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, NONE);
  firmware->addOption("bindkey", Firmware::tr("Allow bind using bind key"));

  /* Radiomaster TX12 board */
  firmware = new OpenTxFirmware(FIRMWAREID("tx12"), QCoreApplication::translate("Firmware", "Radiomaster TX12"), BOARD_RADIOMASTER_TX12);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Radiomaster TX12 Mark II board */
  firmware = new OpenTxFirmware(FIRMWAREID("tx12mk2"), QCoreApplication::translate("Firmware", "Radiomaster TX12 Mark II"), BOARD_RADIOMASTER_TX12_MK2);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Radiomaster GX12 board */
  firmware = new OpenTxFirmware(FIRMWAREID("gx12"), QCoreApplication::translate("Firmware", "Radiomaster GX12"), BOARD_RADIOMASTER_GX12);
  addOpenTxCommonOptions(firmware);
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX);

  /* Radiomaster TX16S board */
  firmware = new OpenTxFirmware(FIRMWAREID("tx16s"), Firmware::tr("Radiomaster TX16S / SE / Hall / Masterfire"), BOARD_RADIOMASTER_TX16S);
  addOpenTxFrskyOptions(firmware);
  addOpenTxRfOptions(firmware, FLEX);
  firmware->addOptionsGroup({opt_bt, opt_internal_gps});
  firmware->addOption("flyskygimbals", Firmware::tr("Support hardware mod: FlySky Paladin EV Gimbals"));
  registerOpenTxFirmware(firmware);

  /* Radiomaster Zorro board */
  firmware = new OpenTxFirmware(FIRMWAREID("zorro"), QCoreApplication::translate("Firmware", "Radiomaster Zorro"), Board::BOARD_RADIOMASTER_ZORRO);
  addOpenTxCommonOptions(firmware);
  firmware->addOption("noheli", Firmware::tr("Disable HELI menu and cyclic mix support"));
  firmware->addOption("nogvars", Firmware::tr("Disable Global variables"));
  firmware->addOption("lua", Firmware::tr("Enable Lua custom scripts screen"));
  firmware->addOption("internalelrs", Firmware::tr("Select if internal ELRS module is installed"));
  addOpenTxFontOptions(firmware);
  registerOpenTxFirmware(firmware);
  addOpenTxRfOptions(firmware, FLEX + AFHDS2A + AFHDS3);

  Firmware::sortRegisteredFirmwares();
  Firmware::setDefaultVariant(Firmware::getFirmwareForFlavour("tx16s"));
  Firmware::setCurrentVariant(Firmware::getDefaultVariant());
}

void unregisterOpenTxFirmwares()
{
  foreach (Firmware * f, Firmware::getRegisteredFirmwares()) {
    delete f;
  }
}
