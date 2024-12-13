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
#include "moduledata.h"
#include "helpers.h"
#include "boardfactories.h"
#include "generalsettings.h"

// TODO remove all those constants
// Update: These are now all only used within this class.
//  External access is only via getEEpromSize() and getFlashSize()

#define EESIZE_TARANIS                 (32*1024)
#define EESIZE_SKY9X                   (128*4096)
#define EESIZE_9XRPRO                  (128*4096)
#define EESIZE_MAX                     EESIZE_9XRPRO

// getFlashSize() (and these macros) is only used by radiointerface::getDfuArgs (perhaps can find a better way?)

#define FSIZE_TARANIS                  (512*1024)
#define FSIZE_SKY9X                    (256*1024)
#define FSIZE_9XRPRO                   (512*1024)
#define FSIZE_HORUS                    (2048*1024)
#define FSIZE_MAX                      FSIZE_HORUS

// pre v2.10
static const StringTagMappingTable legacyTrimSourcesLut = {
    {std::to_string(TRIM_AXIS_LH), "TrimRud"},
    {std::to_string(TRIM_AXIS_LV), "TrimEle"},
    {std::to_string(TRIM_AXIS_RV), "TrimThr"},
    {std::to_string(TRIM_AXIS_RH), "TrimAil"},
    {std::to_string(TRIM_AXIS_T5), "TrimT5"},
    {std::to_string(TRIM_AXIS_T6), "TrimT6"},
    {std::to_string(TRIM_AXIS_T7), "TrimT7"},
    {std::to_string(TRIM_AXIS_T8), "TrimT8"},
};

static const StringTagMappingTable trimSwitchesLut = {
    {std::to_string(TRIM_SW_LH_DEC), "TrimRudLeft"},
    {std::to_string(TRIM_SW_LH_INC), "TrimRudRight"},
    {std::to_string(TRIM_SW_LV_DEC), "TrimEleDown"},
    {std::to_string(TRIM_SW_LV_INC), "TrimEleUp"},
    {std::to_string(TRIM_SW_RV_DEC), "TrimThrDown"},
    {std::to_string(TRIM_SW_RV_INC), "TrimThrUp"},
    {std::to_string(TRIM_SW_RH_DEC), "TrimAilLeft"},
    {std::to_string(TRIM_SW_RH_INC), "TrimAilRight"},
    {std::to_string(TRIM_SW_T5_DEC), "TrimT5Down"},
    {std::to_string(TRIM_SW_T5_INC), "TrimT5Up"},
    {std::to_string(TRIM_SW_T6_DEC), "TrimT6Down"},
    {std::to_string(TRIM_SW_T6_INC), "TrimT6Up"},
    {std::to_string(TRIM_SW_T7_INC), "TrimT7Up"},
    {std::to_string(TRIM_SW_T7_DEC), "TrimT7Down"},
    {std::to_string(TRIM_SW_T8_INC), "TrimT8Up"},
    {std::to_string(TRIM_SW_T8_DEC), "TrimT8Down"},
};

using namespace Board;

Boards::Boards(Board::Type board) :
  legacyTrimSourcesLookupTable(legacyTrimSourcesLut),
  trimSwitchesLookupTable(trimSwitchesLut),
  rawSwitchTypesLookupTable(RawSwitch::getRawSwitchTypesLookupTable()),
  rawSourceSpecialTypesLookupTable(RawSource::getSpecialTypesLookupTable())
{
  setBoardType(board);
}

void Boards::setBoardType(const Type & board)
{
  if (board < BOARD_TYPE_MAX)
    m_boardType = board;
  else
    m_boardType = BOARD_UNKNOWN;

  m_boardJson = getBoardJson(m_boardType);
}

uint32_t Boards::getFourCC(Type board)
{
  switch (board) {
    case BOARD_HORUS_X12S:
      return 0x3478746F;
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
      return 0x3778746F;
    case BOARD_TARANIS_XLITE:
      return 0x3978746F;
    case BOARD_TARANIS_XLITES:
      return 0x3B78746F;
    case BOARD_TARANIS_X7:
    case BOARD_TARANIS_X7_ACCESS:
      return 0x3678746F;
    case BOARD_TARANIS_X9E:
      return 0x3578746F;
    case BOARD_TARANIS_X9D:
    case BOARD_TARANIS_X9DP:
    case BOARD_TARANIS_X9DP_2019:
      return 0x3378746F;
    case BOARD_TARANIS_X9LITE:
      return 0x3C78746F;
    case BOARD_TARANIS_X9LITES:
      return 0x3E78746F;
    case BOARD_SKY9X:
    case BOARD_AR9X:
    case BOARD_9XRPRO:
      return 0x3278746F;
    case BOARD_BETAFPV_LR3PRO:
      return 0x4578746F;
    case BOARD_IFLIGHT_COMMANDO8:
      return 0x4B78746F; // TODO : Check this value
    case BOARD_JUMPER_T12:
      return 0x3D78746F;
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TLITE_F4:
      return 0x4278746F;
    case BOARD_JUMPER_TPRO:
      return 0x4678746F;
    case BOARD_JUMPER_T16:
      return 0x3F78746F;
    case BOARD_JUMPER_T18:
      return 0x4078746F;
    case BOARD_RADIOMASTER_TX16S:
      return 0x3878746F;
    case BOARD_RADIOMASTER_TX12:
      return 0x4178746F;
    case BOARD_RADIOMASTER_TX12_MK2:
      return 0x4878746F;
    case BOARD_RADIOMASTER_ZORRO:
      return 0x4778746F;
    case BOARD_RADIOMASTER_BOXER:
      return 0x4778746F;
    case BOARD_RADIOMASTER_T8:
      return 0x4378746F;
    case BOARD_FLYSKY_NV14:
      return 0x3A78746F;
    case BOARD_FLYSKY_EL18:
      return 0x3A78746F;
    case BOARD_FLYSKY_PL18:
      return 0x4878746F;
    default:
      return 0;
  }
}

int Boards::getEEpromSize(Board::Type board)
{
  switch (board) {
    case BOARD_SKY9X:
      return EESIZE_SKY9X;
    case BOARD_9XRPRO:
    case BOARD_AR9X:
      return EESIZE_9XRPRO;
    case BOARD_TARANIS_XLITES:
    case BOARD_TARANIS_XLITE:
    case BOARD_TARANIS_X7:
    case BOARD_TARANIS_X7_ACCESS:
    case BOARD_TARANIS_X9LITE:
    case BOARD_TARANIS_X9LITES:
    case BOARD_TARANIS_X9D:
    case BOARD_TARANIS_X9DP:
    case BOARD_TARANIS_X9DP_2019:
    case BOARD_TARANIS_X9E:
    case BOARD_BETAFPV_LR3PRO:
    case BOARD_IFLIGHT_COMMANDO8:
    case BOARD_JUMPER_T12:
    case BOARD_JUMPER_T12MAX:
    case BOARD_JUMPER_T14:
    case BOARD_JUMPER_T20:
    case BOARD_JUMPER_T20V2:
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TLITE_F4:
    case BOARD_JUMPER_TPRO:
    case BOARD_JUMPER_TPROV2:
    case BOARD_JUMPER_BUMBLEBEE:
    case BOARD_JUMPER_TPROS:
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_TX12_MK2:
    case BOARD_RADIOMASTER_T8:
    case BOARD_RADIOMASTER_ZORRO:
    case BOARD_RADIOMASTER_BOXER:
    case BOARD_RADIOMASTER_POCKET:
    case BOARD_RADIOMASTER_MT12:
    case BOARD_RADIOMASTER_GX12:
      return EESIZE_TARANIS;
    case BOARD_UNKNOWN:
      return EESIZE_MAX;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T15:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
    case BOARD_FLYSKY_NV14:
    case BOARD_FLYSKY_EL18:
    case BOARD_FLYSKY_PL18:
    case BOARD_FATFISH_F16:
    case BOARD_HELLORADIOSKY_V16:
      return 0;
    default:
      return 0;
  }
}

int Boards::getFlashSize(Type board)
{
  switch (board) {
    case BOARD_SKY9X:
      return FSIZE_SKY9X;
    case BOARD_9XRPRO:
    case BOARD_AR9X:
      return FSIZE_9XRPRO;
    case BOARD_TARANIS_XLITES:
    case BOARD_TARANIS_XLITE:
    case BOARD_TARANIS_X7:
    case BOARD_TARANIS_X7_ACCESS:
    case BOARD_TARANIS_X9LITE:
    case BOARD_TARANIS_X9LITES:
    case BOARD_TARANIS_X9D:
    case BOARD_TARANIS_X9DP:
    case BOARD_TARANIS_X9DP_2019:
    case BOARD_TARANIS_X9E:
    case BOARD_BETAFPV_LR3PRO:
    case BOARD_IFLIGHT_COMMANDO8:
    case BOARD_JUMPER_T12:
    case BOARD_JUMPER_T12MAX:
    case BOARD_JUMPER_T14:
    case BOARD_JUMPER_T20:
    case BOARD_JUMPER_T20V2:
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TLITE_F4:
    case BOARD_JUMPER_TPRO:
    case BOARD_JUMPER_TPROV2:
    case BOARD_JUMPER_TPROS:
    case BOARD_JUMPER_BUMBLEBEE:
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_TX12_MK2:
    case BOARD_RADIOMASTER_ZORRO:
    case BOARD_RADIOMASTER_BOXER:
    case BOARD_RADIOMASTER_T8:
    case BOARD_RADIOMASTER_POCKET:
    case BOARD_RADIOMASTER_MT12:
    case BOARD_RADIOMASTER_GX12:
      return FSIZE_TARANIS;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T15:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
    case BOARD_FLYSKY_NV14:
    case BOARD_FLYSKY_EL18:
    case BOARD_FLYSKY_PL18:
    case BOARD_FATFISH_F16:
    case BOARD_HELLORADIOSKY_V16:
      return FSIZE_HORUS;
    case BOARD_UNKNOWN:
      return FSIZE_MAX;
    default:
      return 0;
  }
}

// static
int Boards::getCapability(Board::Type board, Board::Capability capability)
{
  // TODO investigate usage of any that should be covered in BoardJson::getCapability or are no longer required
  //      some could be used when importing pre v2.10 configurations
  switch (capability) {
    case Air:
      return !getCapability(board, Surface);

    case FactoryInstalledPots:
      if (IS_TARANIS_X9(board))
        return 2;
      else
        return getCapability(board, Pots);

    case FactoryInstalledSwitches:
      if (IS_TARANIS_X9E(board))
        return 8;
      else if (IS_JUMPER_TPROV2(board))
        return 6;
      else if (IS_JUMPER_TLITE(board) || IS_JUMPER_TPROV1(board) || IS_BETAFPV_LR3PRO(board) || IS_IFLIGHT_COMMANDO8(board) || IS_JUMPER_BUMBLEBEE(board))
        return 4;
      else if(IS_RADIOMASTER_ZORRO(board))
        return 8;
      else if (board == BOARD_RADIOMASTER_POCKET)
        return 5;
      else if (IS_FAMILY_T12(board))
        return 6;
      else if (IS_HORUS_X12S(board))
        return 8;
      else
        return getCapability(board, Board::Switches);

    case HasAudioMuteGPIO:
      // All color lcd (including NV14 and EL18) except Horus X12S
      // TX12, TX12MK2, ZORRO, BOXER, T8, TLITE, TPRO, LR3PRO, COMMANDO8
      return (IS_FAMILY_HORUS_OR_T16(board) && !IS_HORUS_X12S(board)) || IS_FAMILY_T12(board);

    case HasColorLcd:
      return IS_FAMILY_HORUS_OR_T16(board);

    case HasExternalModuleSupport:
      return (IS_STM32(board) && !IS_RADIOMASTER_T8(board));

    case HasIMU:
      return (IS_FAMILY_HORUS_OR_T16(board) || IS_TARANIS(board));

    case HasInternalModuleSupport:
      return (IS_STM32(board) && !IS_TARANIS_X9(board));

    case HasLedStripGPIO:
      return (IS_RADIOMASTER_MT12(board) || IS_FLYSKY_PL18(board) ||
              IS_HELLORADIOSKY_V16(board));

    case HasSDCard:
      return IS_STM32(board);

    case HasTrainerModuleCPPM:
      return (getCapability(board, HasTrainerModuleSBUS) || IS_FAMILY_HORUS_OR_T16(board));

    case HasTrainerModuleSBUS:
      return ((IS_TARANIS_X9LITE(board) || (IS_TARANIS_XLITE(board) && !IS_TARANIS_X9LITES(board)) ||
              IS_TARANIS_X9DP_2019(board) || IS_TARANIS_X7_ACCESS(board) || IS_RADIOMASTER_ZORRO(board) ||
              IS_RADIOMASTER_TX12_MK2(board) || IS_RADIOMASTER_BOXER(board) || IS_RADIOMASTER_POCKET(board)) ||
              IS_FAMILY_T16(board) || IS_FAMILY_HORUS(board) || 
              (getCapability(board, HasExternalModuleSupport) && (IS_TARANIS(board) && !IS_FAMILY_T12(board))));

    case LcdDepth:
      if (IS_FAMILY_HORUS_OR_T16(board))
        return 16;
      else if (IS_TARANIS_SMALL(board))
        return 1;
      else if (IS_TARANIS(board))
        return 4;
      else
        return 1;

    case LcdHeight:
      if (IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board))
        return 480;
      else if (IS_FLYSKY_PL18(board) || IS_JUMPER_T15(board))
        return 320;
      else if (IS_FAMILY_HORUS_OR_T16(board))
        return 272;
      else
        return 64;

    case LcdWidth:
      if (IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board))
        return 320;
      else if (IS_FLYSKY_PL18(board))
        return 480;
      else if (IS_FAMILY_HORUS_OR_T16(board))
        return 480;
      else if (IS_TARANIS_SMALL(board))
        return 128;
      else if (IS_TARANIS(board))
        return 212;
      else
        return 128;

    case MaxAnalogs:
      return getCapability(board, Board::Sticks) + getCapability(board, Board::Pots) + getCapability(board, Board::Sliders) +
             getCapability(board, Board::JoystickAxes) + getCapability(board, Board::GyroAxes);

    case SportMaxBaudRate:
      if (IS_FAMILY_T16(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board) || IS_TARANIS_X7_ACCESS(board) ||
         (IS_TARANIS(board) && !IS_TARANIS_XLITE(board) && !IS_TARANIS_X7(board) && !IS_TARANIS_X9LITE(board)))
        return 400000;  //  400K and higher
      else
        return 250000;  //  less than 400K

    case Surface:
      return IS_RADIOMASTER_MT12(board);

    default:
      return getBoardJson(board)->getCapability(capability);
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

// mapping json tag (1st entry) to legacy tag (2nd entry)
// json tag then used to find inputs index
// only used to decode pre v2.10 yaml configs
StringTagMappingTable Boards::getLegacyAnalogsLookupTable(Board::Type board)
{
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

  if (IS_TARANIS_X9LITE(board)) {
    tbl.insert(tbl.end(), {
                              {tr("SL1").toStdString(), "S1"},
                              {tr("P1").toStdString(), "POT1"},
                            });
  } else if (IS_TARANIS_X9E(board)) {
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
  } else if (IS_FLYSKY_PL18(board)) {
    tbl.insert(tbl.end(), {
                              {tr("P1").toStdString(), "POT1"},
                              {tr("P2").toStdString(), "POT2"},
                              {tr("P3").toStdString(), "POT3"},
                              {tr("SL1").toStdString(), "LS"},
                              {tr("SL2").toStdString(), "RS"},
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

std::string Boards::getLegacyAnalogMappedInputTag(const char * legacytag, Board::Type board)
{
  return DataHelpers::getStringTagMappingName(getLegacyAnalogsLookupTable(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board), legacytag);
}

bool Boards::isBoardCompatible(Type board1, Type board2)
{
  return (getFourCC(board1) == getFourCC(board2));
}

QString Boards::getBoardName(Board::Type board)
{
  switch (board) {
    case BOARD_TARANIS_X7:
      return "Taranis X7/X7S";
    case BOARD_TARANIS_X7_ACCESS:
      return "Taranis X7/X7S Access";
    case BOARD_TARANIS_XLITE:
      return "Taranis X-Lite";
    case BOARD_TARANIS_XLITES:
      return "Taranis X-Lite S/PRO";
    case BOARD_TARANIS_X9D:
      return "Taranis X9D";
    case BOARD_TARANIS_X9DP:
      return "Taranis X9D+";
    case BOARD_TARANIS_X9DP_2019:
      return "Taranis X9D+ 2019";
    case BOARD_TARANIS_X9E:
      return "Taranis X9E";
    case BOARD_TARANIS_X9LITE:
      return "Taranis X9-Lite";
    case BOARD_TARANIS_X9LITES:
      return "Taranis X9-Lite S";
    case BOARD_SKY9X:
      return "Sky9x";
    case BOARD_9XRPRO:
      return "9XR-PRO";
    case BOARD_AR9X:
      return "AR9X";
    case BOARD_HORUS_X12S:
      return "Horus X12S";
    case BOARD_X10:
      return "Horus X10/X10S";
    case BOARD_X10_EXPRESS:
      return "Horus X10/X10S Express";
    case BOARD_JUMPER_T12:
      return "Jumper T12";
    case BOARD_JUMPER_TLITE:
      return "Jumper T-Lite";
    case BOARD_JUMPER_TLITE_F4:
      return "Jumper T-Lite (F4 MCU)";
    case BOARD_JUMPER_TPRO:
      return "Jumper T-Pro";
    case BOARD_JUMPER_TPROV2:
      return "Jumper T-Pro V2";
    case BOARD_JUMPER_TPROS:
      return "Jumper T-Pro S";
    case BOARD_JUMPER_T12MAX:
      return "Jumper T12 MAX";
    case BOARD_JUMPER_BUMBLEBEE:
      return "Jumper Bumblebee";
    case BOARD_JUMPER_T14:
      return "Jumper T14";
    case BOARD_JUMPER_T15:
      return "Jumper T15";
    case BOARD_JUMPER_T16:
      return "Jumper T16";
    case BOARD_JUMPER_T18:
      return "Jumper T18";
    case BOARD_JUMPER_T20:
      return "Jumper T20";
    case BOARD_JUMPER_T20V2:
      return "Jumper T20 V2";
    case BOARD_RADIOMASTER_BOXER:
      return "Radiomaster Boxer";
    case BOARD_RADIOMASTER_POCKET:
      return "Radiomaster Pocket";
    case BOARD_RADIOMASTER_MT12:
      return "Radiomaster MT12";
    case BOARD_RADIOMASTER_T8:
      return "Radiomaster T8";
    case BOARD_RADIOMASTER_TX12:
      return "Radiomaster TX12";
    case BOARD_RADIOMASTER_TX12_MK2:
      return "Radiomaster TX12 Mark II";
    case BOARD_RADIOMASTER_TX16S:
      return "Radiomaster TX16S";
    case BOARD_RADIOMASTER_ZORRO:
      return "Radiomaster Zorro";
    case BOARD_RADIOMASTER_GX12:
      return "Radiomaster GX12";
    case BOARD_FLYSKY_NV14:
      return "FlySky NV14";
    case BOARD_FLYSKY_EL18:
      return "FlySky EL18";
    case BOARD_FLYSKY_PL18:
      return "FlySky PL18";
    case BOARD_BETAFPV_LR3PRO:
      return "BETAFPV LR3PRO";
    case BOARD_IFLIGHT_COMMANDO8:
      return "iFlight Commando 8";
    case BOARD_FATFISH_F16:
      return "Fatfish F16";
    case BOARD_HELLORADIOSKY_V16:
      return "HelloRadioSky V16";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString Boards::switchTypeToString(int value)
{
  switch(value) {
    case SWITCH_NOT_AVAILABLE:
      return tr("None");
    case SWITCH_TOGGLE:
      return tr("2 Positions Toggle");
    case SWITCH_2POS:
      return tr("2 Positions");
    case SWITCH_3POS:
      return tr("3 Positions");
    case SWITCH_FUNC:
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

  for (int i = 0; i < SWITCH_FUNC; i++) { // Function not required in lists
    mdl->appendToItemList(switchTypeToString(i), i, true, 0,
                          (i == SWITCH_NOT_AVAILABLE ? SwitchTypeFlagNone : (i < SWITCH_3POS ? SwitchTypeFlag2Pos : SwitchTypeFlag3Pos)));
  }

  mdl->loadItemList();
  return mdl;
}

QList<int> Boards::getSupportedInternalModules(Board::Type board)
{
  QList<int> modules;
  modules = {(int)MODULE_TYPE_NONE};
  if (IS_TARANIS_X9DP_2019(board) || IS_TARANIS_X7_ACCESS(board)) {
    modules.append({(int)MODULE_TYPE_ISRM_PXX2});
  } else if (IS_FLYSKY_NV14(board)) {
    modules.append({(int)MODULE_TYPE_FLYSKY_AFHDS2A});
  } else if (IS_FLYSKY_EL18(board)) {
    modules.append({
        (int)MODULE_TYPE_FLYSKY_AFHDS3,
        (int)MODULE_TYPE_CROSSFIRE,
    });
  } else if (IS_RADIOMASTER_MT12(board)) {
    modules.append({
        (int)MODULE_TYPE_CROSSFIRE,
        (int)MODULE_TYPE_MULTIMODULE,
    });
  } else if (IS_FAMILY_HORUS_OR_T16(board) || IS_FAMILY_T12(board) ||
             (IS_TARANIS_SMALL(board) && IS_ACCESS_RADIO(board))) {
    modules.append({
        (int)MODULE_TYPE_XJT_PXX1,
        (int)MODULE_TYPE_ISRM_PXX2,
        (int)MODULE_TYPE_CROSSFIRE,
        (int)MODULE_TYPE_MULTIMODULE,
    });
  } else if (IS_TARANIS(board)) {
    modules.append({(int)MODULE_TYPE_XJT_PXX1});
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
    return (int)MODULE_TYPE_MULTIMODULE;

  case BOARD_BETAFPV_LR3PRO:
  case BOARD_RADIOMASTER_ZORRO:
  case BOARD_RADIOMASTER_BOXER:
  case BOARD_RADIOMASTER_MT12:
  case BOARD_RADIOMASTER_POCKET:
  case BOARD_RADIOMASTER_GX12:
  case BOARD_RADIOMASTER_TX12_MK2:
  case BOARD_IFLIGHT_COMMANDO8:
  case BOARD_JUMPER_T12MAX:
  case BOARD_JUMPER_T14:
  case BOARD_JUMPER_T15:
  case BOARD_JUMPER_T20:
  case BOARD_JUMPER_T20V2:
  case BOARD_JUMPER_TPROS:
  case BOARD_JUMPER_BUMBLEBEE:
  case BOARD_FATFISH_F16:
  case BOARD_HELLORADIOSKY_V16:
    return (int)MODULE_TYPE_CROSSFIRE;

  case BOARD_FLYSKY_NV14:
    return (int)MODULE_TYPE_FLYSKY_AFHDS2A;

  case BOARD_FLYSKY_EL18:
    return (int)MODULE_TYPE_FLYSKY_AFHDS3;

  default:
    return (int)MODULE_TYPE_NONE;
  }
}

#define BR(min, max, warn) vmin = min - 90; vmax = max - 120; vwarn = warn;

void Boards::getBattRange(Board::Type board, int& vmin, int& vmax, unsigned int& vwarn)
{
  switch (board) {
    case BOARD_TARANIS_X7:
    case BOARD_TARANIS_X7_ACCESS:
    case BOARD_TARANIS_X9D:
    case BOARD_TARANIS_X9DP:
    case BOARD_TARANIS_X9DP_2019:
    case BOARD_TARANIS_X9LITE:
    case BOARD_TARANIS_X9LITES:
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_TX12_MK2:
    case BOARD_RADIOMASTER_BOXER:
    case BOARD_RADIOMASTER_POCKET:
    case BOARD_RADIOMASTER_ZORRO:
    case BOARD_RADIOMASTER_MT12:
    case BOARD_RADIOMASTER_GX12:
    case BOARD_JUMPER_T12:
    case BOARD_JUMPER_T14:
    case BOARD_JUMPER_TPRO:
    case BOARD_JUMPER_TPROV2:
    default:
      BR(60, 80, 65)
      break;
    case BOARD_TARANIS_X9E:
    case BOARD_HORUS_X12S:
      BR(85, 115, 87)
      break;
    case BOARD_TARANIS_XLITE:
    case BOARD_TARANIS_XLITES:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_RADIOMASTER_TX16S:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_JUMPER_T20:
    case BOARD_JUMPER_T20V2:
      BR(67, 83, 66)
      break;
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TLITE_F4:
    case BOARD_RADIOMASTER_T8:
    case BOARD_BETAFPV_LR3PRO:
      BR(34, 42, 36)
      break;
    case BOARD_FLYSKY_NV14:
    case BOARD_FLYSKY_EL18:
      BR(35, 42, 37)
      break;
    case BOARD_FLYSKY_PL18:
      BR(35, 43, 37)
      break;
    case BOARD_IFLIGHT_COMMANDO8:
      BR(30, 42, 32)
      break;
  }
}

// static
int Boards::getDefaultExternalModuleSize(Board::Type board)
{
  if (!getCapability(board, HasExternalModuleSupport))
    return EXTMODSIZE_NONE;

  if (getCapability(board, HasColorLcd)) {
    if (IS_FLYSKY_EL18(board))
      return EXTMODSIZE_BOTH;
    else
      return EXTMODSIZE_STD;
  }

  if (IS_TARANIS_X9LITE(board)    ||
      IS_RADIOMASTER_ZORRO(board) ||
      IS_RADIOMASTER_MT12(board) ||
      IS_RADIOMASTER_POCKET(board) ||
      IS_JUMPER_TLITE(board)      ||
      IS_JUMPER_TPRO(board)       ||
      IS_JUMPER_T20(board)       ||
      IS_BETAFPV_LR3PRO(board))
    return EXTMODSIZE_SMALL;

  return EXTMODSIZE_STD;
}

//  static
QString Boards::externalModuleSizeToString(int value)
{
  switch(value) {
    case EXTMODSIZE_NONE:
      return tr("None");
    case EXTMODSIZE_STD:
      return tr("Standard");
    case EXTMODSIZE_SMALL:
      return tr("Small");
    case EXTMODSIZE_BOTH:
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

  for (int i = 0; i < EXTMODSIZE_COUNT; i++) {
    mdl->appendToItemList(externalModuleSizeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

QString Boards::flexTypeToString(int value)
{
  switch(value) {
    case FLEX_NONE:
      return tr("None");
    case FLEX_POT:
      return tr("Pot");
    case FLEX_POT_CENTER:
      return tr("Pot with detent");
    case FLEX_SLIDER:
      return tr("Slider");
    case FLEX_MULTIPOS:
      return tr("Multipos Switch");
    case FLEX_AXIS_X:
      return tr("Axis X");
    case FLEX_AXIS_Y:
      return tr("Axis Y");
    case FLEX_SWITCH:
      return tr("Switch");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

AbstractStaticItemModel * Boards::flexTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_FLEX_TYPE);

  for (int i = 0; i < FLEX_TYPE_COUNT; i++) {
    mdl->appendToItemList(flexTypeToString(i), i, true, 0, (i == (int)FLEX_SWITCH ? FlexTypeFlagSwitch : FlexTypeFlagNotSwitch));
  }

  mdl->loadItemList();
  return mdl;
}

BoardJson* Boards::getBoardJson(Board::Type board)
{
  return gBoardFactories->instance(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board);
}

Board::InputInfo Boards::getInputInfo(int index, Board::Type board)
{
  return getBoardJson(board)->getInputInfo(index);
}

int Boards::getInputIndex(QString val, Board::LookupValueType lvt, Board::Type board)
{
  return getBoardJson(board)->getInputIndex(val, lvt);
}

QString Boards::getInputName(int index, Board::Type board)
{
  return getBoardJson(board)->getInputName(index);
}

int Boards::getInputExtIndex(int index, Board::Type board)
{
  return getBoardJson(board)->getInputExtIndex(index);
}

int Boards::getInputPotIndex(int index, Board::Type board)
{
  return getBoardJson(board)->getInputPotIndex(index);
}

int Boards::getInputSliderIndex(int index, Board::Type board)
{
  return getBoardJson(board)->getInputSliderIndex(index);
}

QString Boards::getInputTag(int index, Board::Type board)
{
  return getBoardJson(board)->getInputTag(index);
}

int Boards::getInputTagOffset(QString tag, Board::Type board)
{
  return getBoardJson(board)->getInputTagOffset(tag);
}

int Boards::getInputThrottleIndex(Board::Type board)
{
  return getBoardJson(board)->getInputThrottleIndex();
}

int Boards::getInputTypeOffset(Board::AnalogInputType type, Board::Type board)
{
  return getBoardJson(board)->getInputTypeOffset(type);
}

int Boards::getInputYamlIndex(QString val, int ylt, Board::Type board)
{
  return getBoardJson(board)->getInputYamlIndex(val, (BoardJson::YamlLookupType)ylt);
}

QString Boards::getInputYamlName(int index, int ylt, Board::Type board)
{
  return getBoardJson(board)->getInputYamlName(index, (BoardJson::YamlLookupType)ylt);
}

int Boards::getInputsCalibrated(Board::Type board)
{
  return getBoardJson(board)->getInputsCalibrated();
}

Board::KeyInfo Boards::getKeyInfo(int index, Board::Type board)
{
  return getBoardJson(board)->getKeyInfo(index);
}

int Boards::getKeyIndex(QString key, Board::Type board)
{
  return getBoardJson(board)->getKeyIndex(key);
}

Board::SwitchInfo Boards::getSwitchInfo(int index, Board::Type board)
{
  return getBoardJson(board)->getSwitchInfo(index);
}

int Boards::getSwitchIndex(QString val, Board::LookupValueType lvt, Board::Type board)
{
  return getBoardJson(board)->getSwitchIndex(val, lvt);
}

QString Boards::getSwitchName(int index, Board::Type board)
{
  return getBoardJson(board)->getSwitchName(index);
}

QString Boards::getSwitchTag(int index, Board::Type board)
{
  return getBoardJson(board)->getSwitchTag(index);
}

int Boards::getSwitchTagNum(int index, Board::Type board)
{
  return getBoardJson(board)->getSwitchTagNum(index);
}

int Boards::getSwitchTypeOffset(Board::SwitchType type, Board::Type board)
{
  return getBoardJson(board)->getSwitchTypeOffset(type);
}

int Boards::getSwitchYamlIndex(QString val, int ylt, Board::Type board)
{
  return getBoardJson(board)->getSwitchYamlIndex(val, (BoardJson::YamlLookupType)ylt);
}

QString Boards::getSwitchYamlName(int index, int ylt, Board::Type board)
{
  return getBoardJson(board)->getSwitchYamlName(index, (BoardJson::YamlLookupType)ylt);
}

int Boards::getTrimYamlIndex(QString val, int ylt, Board::Type board)
{
  return getBoardJson(board)->getTrimYamlIndex(val, (BoardJson::YamlLookupType)ylt);
}

QString Boards::getTrimYamlName(int index, int ylt, Board::Type board)
{
  return getBoardJson(board)->getTrimYamlName(index, (BoardJson::YamlLookupType)ylt);
}

bool Boards::isInputAvailable(int index, Board::Type board)
{
  return getBoardJson(board)->isInputAvailable(index);
}

bool Boards::isInputCalibrated(int index, Board::Type board)
{
  return getBoardJson(board)->isInputCalibrated(index);
}

bool Boards::isInputConfigurable(int index, Board::Type board)
{
  return getBoardJson(board)->isInputConfigurable(index);
}

bool Boards::isInputIgnored(int index, Board::Type board)
{
  return getBoardJson(board)->isInputIgnored(index);
}

bool Boards::isInputPot(int index, Board::Type board)
{
  return getBoardJson(board)->isInputFlexPot(index);
}

bool Boards::isInputStick(int index, Board::Type board)
{
  return getBoardJson(board)->isInputStick(index);
}

bool Boards::isSwitchConfigurable(int index, Board::Type board)
{
  return getBoardJson(board)->isSwitchConfigurable(index);
}

bool Boards::isSwitchFlex(int index, Board::Type board)
{
  return getBoardJson(board)->isSwitchFlex(index);
}

bool Boards::isSwitchFunc(int index, Board::Type board)
{
  return getBoardJson(board)->isSwitchFunc(index);
}

QString Boards::getRadioTypeString(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Board::Air) ? tr("Flight") : tr("Drive");
}

bool Boards::isAir(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Board::Air);
}

bool Boards::isSurface(Board::Type board)
{
  return getCapability(board == Board::BOARD_UNKNOWN ? getCurrentBoard() : board, Board::Surface);
}
