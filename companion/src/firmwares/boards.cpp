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

#include "boards.h"
#include "macros.h"
#include "compounditemmodels.h"
#include "moduledata.h"

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

using namespace Board;

void Boards::setBoardType(const Type & board)
{
  if (board >= BOARD_UNKNOWN && board <= BOARD_TYPE_MAX)
    m_boardType = board;
  else
    m_boardType = BOARD_UNKNOWN;
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
    case BOARD_JUMPER_T12:
      return 0x3D78746F;
    case BOARD_JUMPER_TLITE:
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
    case BOARD_RADIOMASTER_ZORRO:
      return 0x4778746F;
    case BOARD_RADIOMASTER_T8:
      return 0x4378746F;
    case BOARD_FLYSKY_NV14:
      return 0x3A78746F;
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
    case BOARD_JUMPER_T12:
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TPRO:
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_T8:
    case BOARD_RADIOMASTER_ZORRO:
      return EESIZE_TARANIS;
    case BOARD_UNKNOWN:
      return EESIZE_MAX;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
    case BOARD_FLYSKY_NV14:
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
    case BOARD_JUMPER_T12:
    case BOARD_JUMPER_TLITE:
    case BOARD_JUMPER_TPRO:
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_ZORRO:
    case BOARD_RADIOMASTER_T8:
      return FSIZE_TARANIS;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
    case BOARD_FLYSKY_NV14:
      return FSIZE_HORUS;
    case BOARD_UNKNOWN:
      return FSIZE_MAX;
    default:
      return 0;
  }
}

SwitchInfo Boards::getSwitchInfo(Board::Type board, int index)
{
  if (index < 0)
    return {SWITCH_NOT_AVAILABLE, CPN_STR_UNKNOWN_ITEM};

  if (IS_TARANIS_XLITES(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_2POS,   "SC"},
      {SWITCH_2POS,   "SD"},
      {SWITCH_TOGGLE, "SE"},
      {SWITCH_TOGGLE, "SF"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_TARANIS_XLITE(board) || IS_JUMPER_TLITE(board) || IS_JUMPER_TPRO(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_2POS,   "SC"},
      {SWITCH_2POS,   "SD"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (board == BOARD_TARANIS_X7_ACCESS) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_3POS,   "SC"},
      {SWITCH_3POS,   "SD"},
      {SWITCH_2POS,   "SF"},
      {SWITCH_TOGGLE, "SH"},
      {SWITCH_2POS,   "SI"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (board == BOARD_TARANIS_X7) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_3POS,   "SC"},
      {SWITCH_3POS,   "SD"},
      {SWITCH_2POS,   "SF"},
      {SWITCH_TOGGLE, "SH"},
      {SWITCH_2POS,   "SI"},
      {SWITCH_2POS,   "SJ"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_RADIOMASTER_TX12(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_TOGGLE,   "SA"},
      {SWITCH_3POS,     "SB"},
      {SWITCH_3POS,     "SC"},
      {SWITCH_TOGGLE,   "SD"},
      {SWITCH_3POS,     "SE"},
      {SWITCH_3POS,     "SF"},
      {SWITCH_2POS,     "SI"},
      {SWITCH_2POS,     "SJ"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_RADIOMASTER_ZORRO(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_TOGGLE,   "SA"},
      {SWITCH_3POS,     "SB"},
      {SWITCH_3POS,     "SC"},
      {SWITCH_TOGGLE,   "SD"},
      {SWITCH_2POS,     "SE"},
      {SWITCH_2POS,     "SF"},
      {SWITCH_TOGGLE,   "SG"},
      {SWITCH_TOGGLE,   "SH"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_RADIOMASTER_T8(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_TOGGLE,   "SA"},
      {SWITCH_3POS,     "SB"},
      {SWITCH_3POS,     "SC"},
      {SWITCH_TOGGLE,   "SD"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_JUMPER_T12(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS, "SA"},
      {SWITCH_3POS, "SB"},
      {SWITCH_3POS, "SC"},
      {SWITCH_3POS, "SD"},
      {SWITCH_2POS, "SG"},
      {SWITCH_2POS, "SH"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_FLYSKY_NV14(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_2POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_TOGGLE, "SC"},
      {SWITCH_2POS,   "SD"},
      {SWITCH_TOGGLE, "SE"},
      {SWITCH_3POS,   "SF"},
      {SWITCH_3POS,   "SG"},
      {SWITCH_TOGGLE, "SH"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else if (IS_FAMILY_HORUS_OR_T16(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_3POS,   "SC"},
      {SWITCH_3POS,   "SD"},
      {SWITCH_3POS,   "SE"},
      {SWITCH_2POS,   "SF"},
      {SWITCH_3POS,   "SG"},
      {SWITCH_TOGGLE, "SH"},
      {SWITCH_2POS,   "SI"},
      {SWITCH_2POS,   "SJ"}
  };
  if (index < DIM(switches))
    return switches[index];
  }
  else if (IS_TARANIS(board)) {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "SA"},
      {SWITCH_3POS,   "SB"},
      {SWITCH_3POS,   "SC"},
      {SWITCH_3POS,   "SD"},
      {SWITCH_3POS,   "SE"},
      {SWITCH_2POS,   "SF"},
      {SWITCH_3POS,   "SG"},
      {SWITCH_TOGGLE, "SH"},
      {board == Board::BOARD_TARANIS_X9DP_2019 ? SWITCH_TOGGLE : SWITCH_3POS,   "SI"},
      {SWITCH_3POS,   "SJ"},
      {SWITCH_3POS,   "SK"},
      {SWITCH_3POS,   "SL"},
      {SWITCH_3POS,   "SM"},
      {SWITCH_3POS,   "SN"},
      {SWITCH_3POS,   "SO"},
      {SWITCH_3POS,   "SP"},
      {SWITCH_3POS,   "SQ"},
      {SWITCH_3POS,   "SR"}
    };
    if (index < DIM(switches))
      return switches[index];
  }
  else {
    const Board::SwitchInfo switches[] = {
      {SWITCH_3POS,   "3POS"},
      {SWITCH_2POS,   "THR"},
      {SWITCH_2POS,   "RUD"},
      {SWITCH_2POS,   "ELE"},
      {SWITCH_2POS,   "AIL"},
      {SWITCH_2POS,   "GEA"},
      {SWITCH_TOGGLE, "TRN"}
    };
    if (index < DIM(switches))
      return switches[index];
  }

  return {SWITCH_NOT_AVAILABLE, CPN_STR_UNKNOWN_ITEM};
}

int Boards::getCapability(Board::Type board, Board::Capability capability)
{
  switch (capability) {
    case Sticks:
      return 4;

    case Pots:
      if (IS_TARANIS_X9LITE(board))
        return 1;
      else if (IS_JUMPER_TLITE(board))
        return 0;
      else if (IS_TARANIS_SMALL(board) || IS_JUMPER_TPRO(board))
        return 2;
      else if (IS_TARANIS_X9E(board))
        return 4;
      else if (IS_HORUS_X10(board) || IS_FAMILY_T16(board))
        return 7;
      else if (IS_HORUS_X12S(board))
        return 3;
      else if (IS_FLYSKY_NV14(board))
        return 2;
      else
        return 3;

    case FactoryInstalledPots:
      if (IS_TARANIS_X9(board))
        return 2;
      else
        return getCapability(board, Pots);

    case Sliders:
      if (IS_HORUS_X12S(board) || IS_TARANIS_X9E(board))
        return 4;
      else if (IS_TARANIS_X9D(board) || IS_HORUS_X10(board) || IS_FAMILY_T16(board))
        return 2;
      else
        return 0;

    case MouseAnalogs:
      if (IS_HORUS_X12S(board))
        return 2;
      else
        return 0;

    case GyroAnalogs:
      if (IS_TARANIS_XLITES(board))
        return 2;
      else
        return 0;

    case MaxAnalogs:
      return getCapability(board, Board::Sticks) + getCapability(board, Board::Pots) + getCapability(board, Board::Sliders) + getCapability(board, Board::MouseAnalogs) + getCapability(board, Board::GyroAnalogs);

    case MultiposPots:
      if (IS_HORUS_OR_TARANIS(board) && !IS_FLYSKY_NV14(board))
        return getCapability(board, Board::Pots);
      else
        return 0;

    case MultiposPotsPositions:
      return IS_HORUS_OR_TARANIS(board) ? 6 : 0;

    case Board::Switches:
      if (IS_TARANIS_X9E(board))
        return 18;
      else if (board == Board::BOARD_TARANIS_X9LITE)
        return 5;
      else if (board == Board::BOARD_TARANIS_X9LITES)
        return 7;
      else if (board == BOARD_TARANIS_X7_ACCESS)
        return 7;
      else if (board == BOARD_TARANIS_X7)
        return 8;
      else if (board == BOARD_JUMPER_TLITE || board == BOARD_JUMPER_TPRO)
        return 4;
      else if (board == BOARD_FLYSKY_NV14)
        return 8;
      else if (IS_FAMILY_T12(board))
        return 8;
      else if (IS_TARANIS_XLITE(board))
        return 6;
      else if (board == Board::BOARD_TARANIS_X9DP_2019)
        return 9;
      else if (IS_TARANIS(board))
        return 8;
      else if (IS_FAMILY_HORUS_OR_T16(board))
        return 10;
      else
        return 7;

    case FunctionSwitches:
       return (IS_JUMPER_TPRO(board) ? 6 : 0);

    case FactoryInstalledSwitches:
      if (IS_TARANIS_X9E(board))
        return 8;
      else if (IS_JUMPER_TLITE(board) || IS_JUMPER_TPRO(board))
        return 4;
      else if(IS_RADIOMASTER_ZORRO(board))
        return 8;
      else if (IS_FAMILY_T12(board))
        return 6;
      else if (IS_HORUS_X12S(board))
        return 8;
      else
        return getCapability(board, Board::Switches);

    case SwitchPositions:
      if (IS_HORUS_OR_TARANIS(board) || IS_FLYSKY_NV14(board))
        return getCapability(board, Board::Switches) * 3;
      else
        return 9;

    case NumFunctionSwitchesPositions:
       return getCapability(board, Board::FunctionSwitches) * 3;


    case NumTrims:
      if (IS_FAMILY_HORUS_OR_T16(board) && !IS_FLYSKY_NV14(board))
        return 6;
      else
        return 4;

    case NumTrimSwitches:
      return getCapability(board, Board::NumTrims) * 2;

    case HasRTC:
      return IS_STM32(board) ? true : false;

    case HasColorLcd:
      return IS_FAMILY_HORUS_OR_T16(board);

    case HasSDCard:
      return IS_STM32(board);

    case HasInternalModuleSupport:
      return (IS_STM32(board) && !IS_TARANIS_X9(board));

    case HasExternalModuleSupport:
      return (IS_STM32(board) && !IS_RADIOMASTER_T8(board));

    case SportMaxBaudRate:
      if (IS_FAMILY_T16(board) || IS_FLYSKY_NV14(board) || IS_TARANIS_X7_ACCESS(board) ||
         (IS_TARANIS(board) && !IS_TARANIS_XLITE(board) && !IS_TARANIS_X7(board) && !IS_TARANIS_X9LITE(board)))
        return 400000;  //  400K and higher
      else
        return 250000;  //  less than 400K

    default:
      return 0;
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

StringTagMappingTable Boards::getAnalogNamesLookupTable(Board::Type board)
{
  StringTagMappingTable tbl;

  if (getBoardCapability(board, Board::Sticks)) {
    tbl.insert(tbl.end(), {"Rud", "Ele", "Thr", "Ail"});
  }

  if (IS_SKY9X(board)) {
    tbl.insert(tbl.end(), {"P1", "P2", "P3"});
  } else if (IS_TARANIS_X9E(board)) {
    tbl.insert(tbl.end(), {
                              {"F1", "POT1"},
                              {"F2", "POT2"},
                              {"F3", "POT3"},
                              {"F4", "POT4"},
                              {"S1", "SLIDER1"},
                              {"S2", "SLIDER2"},
                              {"LS", "SLIDER3"},
                              {"RS", "SLIDER4"},
                          });
  } else if (IS_TARANIS_XLITE(board)) {
    tbl.insert(tbl.end(), {
                              {"S1", "POT1"},
                              {"S2", "POT2"},
                              {"GyrX", "GYRO1"},
                              {"GyrY", "GYRO2"},
                          });
  } else if (IS_TARANIS(board)) {
    tbl.insert(tbl.end(), {
                              {"S1", "POT1"},
                              {"S2", "POT2"},
                              {"S3", "POT3"},
                              {"LS", "SLIDER1"},
                              {"RS", "SLIDER2"},
                          });
  } else if (IS_HORUS_X12S(board)) {
    tbl.insert(tbl.end(), {
                              {"S1", "S1"},
                              {"6P", "6POS"},
                              {"S2", "S2"},
                              {"L1", "S3"},
                              {"L2", "S4"},
                              {"LS", "LS"},
                              {"RS", "RS"},
                              {"JSx", "MOUSE1"},
                              {"JSy", "MOUSE2"},
                          });
  } else if (IS_FLYSKY_NV14(board)) {
    tbl.insert(tbl.end(), {
                              {"S1", "POT1"},
                              {"S2", "POT2"},
                          });
  } else if (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) {
    tbl.insert(tbl.end(), {
                              {"S1", "S1"},
                              {"6P", "6POS"},
                              {"S2", "S2"},
                              {"EX1", "EXT1"},
                              {"EX2", "EXT2"},
                              {"EX3", "EXT3"},
                              {"EX4", "EXT4"},
                              {"LS", "LS"},
                              {"RS", "RS"},
                          });
  }

  return tbl;
}

QString Boards::getAnalogInputName(Board::Type board, int index)
{
  const StringTagMappingTable& lut = getAnalogNamesLookupTable(board);
  if (index < (int)lut.size())
    return QString::fromStdString(lut[index].name);

  return CPN_STR_UNKNOWN_ITEM;
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
    case BOARD_JUMPER_TPRO:
      return "Jumper T-Pro";
    case BOARD_JUMPER_T16:
      return "Jumper T16";
    case BOARD_JUMPER_T18:
      return "Jumper T18";
    case BOARD_RADIOMASTER_TX16S:
      return "Radiomaster TX16S";
    case BOARD_RADIOMASTER_TX12:
      return "Radiomaster TX12";
    case BOARD_RADIOMASTER_ZORRO:
      return "Radiomaster Zorro";
    case BOARD_RADIOMASTER_T8:
      return "Radiomaster T8";
    case BOARD_FLYSKY_NV14:
      return "FlySky NV14";
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
QString Boards::potTypeToString(int value)
{
  switch(value) {
    case POT_NONE:
      return tr("None");
    case POT_WITH_DETENT:
      return tr("Pot with detent");
    case POT_MULTIPOS_SWITCH:
      return tr("Multi pos switch");
    case POT_WITHOUT_DETENT:
      return tr("Pot without detent");
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}


//  static
QString Boards::sliderTypeToString(int value)
{
  switch(value) {
    case SLIDER_NONE:
      return tr("None");
    case SLIDER_WITH_DETENT:
      return tr("Slider with detent");
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
    default:
      return CPN_STR_UNKNOWN_ITEM;
  }
}

//  static
AbstractStaticItemModel * Boards::potTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_POT_TYPE);

  for (int i = 0; i < POT_TYPE_COUNT; i++) {
    mdl->appendToItemList(potTypeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * Boards::sliderTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_SLIDER_TYPE);

  for (int i = 0; i < SLIDER_TYPE_COUNT; i++) {
    mdl->appendToItemList(sliderTypeToString(i), i);
  }

  mdl->loadItemList();
  return mdl;
}

//  static
AbstractStaticItemModel * Boards::switchTypeItemModel()
{
  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->setName(AIM_BOARDS_SWITCH_TYPE);

  for (int i = 0; i < SWITCH_TYPE_COUNT; i++) {
    mdl->appendToItemList(switchTypeToString(i), i, true, 0, (i < SWITCH_3POS ? SwitchTypeFlag2Pos : SwitchTypeFlag3Pos));
  }

  mdl->loadItemList();
  return mdl;
}

//  static
StringTagMappingTable Boards::getSwitchesLookupTable(Board::Type board)
{
  StringTagMappingTable tbl;

  for (int i = 0; i < Boards::getCapability(board, Board::Switches); i++) {
    SwitchInfo si = Boards::getSwitchInfo(board, i);
    if (si.config != SWITCH_NOT_AVAILABLE) //  safety check in case Switches does not align
      tbl.insert(tbl.end(), si.name.toStdString());
  }

  return tbl;
}

// static
StringTagMappingTable Boards::getTrimSwitchesLookupTable(Board::Type board)
{
  StringTagMappingTable tbl;

  tbl.insert(tbl.end(), {
                          {std::to_string(TRIM_SW_LH_DEC), "TrimRudLeft"},
                          {std::to_string(TRIM_SW_LH_INC), "TrimRudRight"},
                          {std::to_string(TRIM_SW_LV_DEC), "TrimEleDown"},
                          {std::to_string(TRIM_SW_LV_INC), "TrimEleUp"},
                          {std::to_string(TRIM_SW_RV_DEC), "TrimThrDown"},
                          {std::to_string(TRIM_SW_RV_INC), "TrimThrUp"},
                          {std::to_string(TRIM_SW_RH_DEC), "TrimAilLeft"},
                          {std::to_string(TRIM_SW_RH_INC), "TrimAilRight"},
                        });

  if (getCapability(board, Board::NumTrims) > 4)
    tbl.insert(tbl.end(), {
                            {std::to_string(TRIM_SW_T5_DEC), "TrimT5Down"},
                            {std::to_string(TRIM_SW_T5_INC), "TrimT5Up"},
                            {std::to_string(TRIM_SW_T6_DEC), "TrimT6Down"},
                            {std::to_string(TRIM_SW_T6_INC), "TrimT6Up"},
                          });

  return tbl;
}

// static
StringTagMappingTable Boards::getTrimSourcesLookupTable(Board::Type board)
{
  StringTagMappingTable tbl;

  tbl.insert(tbl.end(), {
                          {std::to_string(TRIM_AXIS_LH), "TrimRud"},
                          {std::to_string(TRIM_AXIS_LV), "TrimEle"},
                          {std::to_string(TRIM_AXIS_RV), "TrimThr"},
                          {std::to_string(TRIM_AXIS_RH), "TrimAil"},
                        });

  if (getCapability(board, Board::NumTrims) > 4)
    tbl.insert(tbl.end(), {
                            {std::to_string(TRIM_AXIS_T5), "TrimT5"},
                            {std::to_string(TRIM_AXIS_T6), "TrimT6"},
                          });

  return tbl;
}

QList<int> Boards::getSupportedInternalModules(Board::Type board)
{
  QList<int> modules;
  modules = {(int)MODULE_TYPE_NONE};
  if (IS_TARANIS_X9DP_2019(board) || IS_TARANIS_X7_ACCESS(board)) {
    modules.append({(int)MODULE_TYPE_ISRM_PXX2});
  } else if (IS_FLYSKY_NV14(board)) {
    modules.append({(int)MODULE_TYPE_FLYSKY});
  } else if (IS_FAMILY_HORUS_OR_T16(board) || IS_FAMILY_T12(board)
             || (IS_TARANIS_SMALL(board) && IS_ACCESS_RADIO(board))) {
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
  case BOARD_JUMPER_TPRO:
    return (int)MODULE_TYPE_MULTIMODULE;

  case BOARD_RADIOMASTER_ZORRO:
    return (int)MODULE_TYPE_CROSSFIRE;

  case BOARD_FLYSKY_NV14:
    return (int)MODULE_TYPE_FLYSKY;

  default:
    return (int)MODULE_TYPE_NONE;
  }
}
