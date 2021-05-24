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
    case BOARD_JUMPER_T16:
      return 0x3F78746F;
    case BOARD_JUMPER_T18:
      return 0x4078746F;
    case BOARD_RADIOMASTER_TX16S:
      return 0x3878746F;
    case BOARD_RADIOMASTER_TX12:
      return 0x4178746F;
    case BOARD_RADIOMASTER_T8:
      return 0x4378746F;
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
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_T8:
      return EESIZE_TARANIS;
    case BOARD_UNKNOWN:
      return EESIZE_MAX;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
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
    case BOARD_RADIOMASTER_TX12:
    case BOARD_RADIOMASTER_T8:
      return FSIZE_TARANIS;
    case BOARD_HORUS_X12S:
    case BOARD_X10:
    case BOARD_X10_EXPRESS:
    case BOARD_JUMPER_T16:
    case BOARD_JUMPER_T18:
    case BOARD_RADIOMASTER_TX16S:
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
  else if (IS_TARANIS_XLITE(board) || IS_JUMPER_TLITE(board)) {
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
      else if (IS_TARANIS_SMALL(board))
        return 2;
      else if (IS_TARANIS_X9E(board))
        return 4;
      else if (IS_HORUS_X10(board) || IS_FAMILY_T16(board))
        return 5;
      else if (IS_HORUS_X12S(board))
        return 3;
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
      return IS_HORUS_OR_TARANIS(board) ? getCapability(board, Board::Pots) : 0;

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
      else if (board == BOARD_JUMPER_TLITE)
        return 4;
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

    case FactoryInstalledSwitches:
      if (IS_TARANIS_X9E(board))
        return 8;
      else if (IS_JUMPER_TLITE(board))
        return 4;
      else if (IS_FAMILY_T12(board))
        return 6;
      else if (IS_HORUS_X12S(board))
        return 8;
      else
        return getCapability(board, Board::Switches);

    case SwitchPositions:
      if (IS_HORUS_OR_TARANIS(board))
        return getCapability(board, Board::Switches) * 3;
      else
        return 9;

    case NumTrims:
      if (IS_FAMILY_HORUS_OR_T16(board))
        return 6;
      else
        return 4;

    case NumTrimSwitches:
      return getCapability(board, Board::NumTrims) * 2;

    case HasRTC:
      return IS_STM32(board) ? true : false;

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

QString Boards::getAnalogInputName(Board::Type board, int index)
{
  if (index < 0)
    return CPN_STR_UNKNOWN_ITEM;

  if (index < getBoardCapability(board, Board::Sticks)) {
    const QString sticks[] = {
      tr("Rud"),
      tr("Ele"),
      tr("Thr"),
      tr("Ail")
    };
    return sticks[index];
  }

  index -= getCapability(board, Board::Sticks);

  if (IS_SKY9X(board)) {
    const QString pots[] = {
      "P1",
      "P2",
      "P3"
    };
    if (index < DIM(pots))
      return pots[index];
  }
  else if (IS_TARANIS_X9E(board)) {
    const QString pots[] = {
      "F1",
      "F2",
      "F3",
      "F4",
      "S1",
      "S2",
      "LS",
      "RS"
    };
    if (index < DIM(pots))
      return pots[index];
  }
  else if (IS_TARANIS_XLITE(board)) {
    const QString pots[] = {
      "S1",
      "S2",
      "GyrX",
      "GyrY"
    };
    if (index < DIM(pots))
      return pots[index];
  }
  else if (IS_TARANIS(board)) {
    const QString pots[] = {
      "S1",
      "S2",
      "S3",
      "LS",
      "RS"
    };
    if (index < DIM(pots))
      return pots[index];
  }
  else if (IS_HORUS_X12S(board)) {
    const QString pots[] = {
      "S1",
      "6P",
      "S2",
      "L1",
      "L2",
      "LS",
      "RS",
      "JSx",
      "JSy"
    };
    if (index < DIM(pots))
      return pots[index];
  }
  else if (IS_HORUS_X10(board) || IS_FAMILY_T16(board)) {
    const QString pots[] = {
      "S1",
      "6P",
      "S2",
      "EX1",
      "EX2",
      "LS",
      "RS"
    };
    if (index < DIM(pots))
      return pots[index];
  }

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
    case BOARD_JUMPER_T16:
      return "Jumper T16";
    case BOARD_JUMPER_T18:
      return "Jumper T18";
    case BOARD_RADIOMASTER_TX16S:
      return "Radiomaster TX16S";
    case BOARD_RADIOMASTER_TX12:
      return "Radiomaster TX12";
    case BOARD_RADIOMASTER_T8:
      return "Radiomaster T8";
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
