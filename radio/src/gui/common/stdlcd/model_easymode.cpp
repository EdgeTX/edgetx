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

#include "edgetx.h"
#include "easymode.h"
#include "easymode_convert.h"

enum MenuModelEasyModeItems {
  ITEM_EASYMODE_MODEL_TYPE,
  ITEM_EASYMODE_WING_TYPE,
  ITEM_EASYMODE_TAIL_TYPE,
  ITEM_EASYMODE_MOTOR_TYPE,
  ITEM_EASYMODE_CH_HEADER,
  ITEM_EASYMODE_CH_AIL,
  ITEM_EASYMODE_CH_AIL2,
  ITEM_EASYMODE_CH_ELE,
  ITEM_EASYMODE_CH_ELE2,
  ITEM_EASYMODE_CH_THR,
  ITEM_EASYMODE_CH_RUD,
  ITEM_EASYMODE_CH_STEER,
  ITEM_EASYMODE_CH_FLAP,
  ITEM_EASYMODE_SRC_FLAP,
  ITEM_EASYMODE_CH_FLAP2,
  ITEM_EASYMODE_CH_FLAP3,
  ITEM_EASYMODE_CH_FLAP4,
  ITEM_EASYMODE_CROW,
  ITEM_EASYMODE_SRC_MOTOR,
  ITEM_EASYMODE_CH_AUX1,
  ITEM_EASYMODE_SRC_AUX1,
  ITEM_EASYMODE_CH_AUX2,
  ITEM_EASYMODE_SRC_AUX2,
  ITEM_EASYMODE_OPT_HEADER,
  ITEM_EASYMODE_OPT_EXPO_AIL,
  ITEM_EASYMODE_OPT_EXPO_ELE,
  ITEM_EASYMODE_OPT_EXPO_RUD,
  ITEM_EASYMODE_OPT_DR_LOW,
  ITEM_EASYMODE_OPT_AIL_DIFF,
  ITEM_EASYMODE_OPT_AIL2RUD,
  ITEM_EASYMODE_OPT_FLAP2ELE,
  ITEM_EASYMODE_OPT_CH_ORDER,
  ITEM_EASYMODE_MIX_HEADER,
  ITEM_EASYMODE_MIX1_SRC,
  ITEM_EASYMODE_MIX1_CH,
  ITEM_EASYMODE_MIX1_WT,
  ITEM_EASYMODE_MIX2_SRC,
  ITEM_EASYMODE_MIX2_CH,
  ITEM_EASYMODE_MIX2_WT,
  ITEM_EASYMODE_MIX3_SRC,
  ITEM_EASYMODE_MIX3_CH,
  ITEM_EASYMODE_MIX3_WT,
  ITEM_EASYMODE_CONVERT,
  ITEM_EASYMODE_MAX
};

#define EASYMODE_COL2  (12*FW)

static const char* const modelTypeValues[] = {
  "---",
  STR_EASYMODE_AIRPLANE,
  STR_EASYMODE_HELICOPTER,
  STR_EASYMODE_GLIDER,
  STR_EASYMODE_MULTIROTOR,
  STR_EASYMODE_CAR,
  STR_EASYMODE_BOAT,
  nullptr
};

static const char* const wingTypeValues[] = {
  STR_EASYMODE_WING_SINGLE_AIL,
  STR_EASYMODE_WING_DUAL_AIL,
  STR_EASYMODE_WING_FLAPERON,
  STR_EASYMODE_WING_1A1F,
  STR_EASYMODE_WING_2A1F,
  STR_EASYMODE_WING_2A2F,
  STR_EASYMODE_WING_2A4F,
  STR_EASYMODE_WING_ELEVON,
  STR_EASYMODE_WING_DELTA,
  nullptr
};

static const char* const tailTypeValues[] = {
  STR_EASYMODE_TAIL_NORMAL,
  STR_EASYMODE_TAIL_VTAIL,
  STR_EASYMODE_TAIL_TAILLESS,
  STR_EASYMODE_TAIL_DUAL_ELE,
  STR_EASYMODE_TAIL_AILEVATOR,
  nullptr
};

static const char* const motorTypeValues[] = {
  STR_EASYMODE_MOTOR_NONE,
  STR_EASYMODE_MOTOR_ELECTRIC,
  STR_EASYMODE_MOTOR_NITRO,
  nullptr
};

static bool isAirType(EasyModelType t)
{
  return t == EASYMODE_AIRPLANE || t == EASYMODE_GLIDER ||
         t == EASYMODE_HELICOPTER || t == EASYMODE_MULTIROTOR;
}

static bool isSurfaceType(EasyModelType t)
{
  return t == EASYMODE_CAR || t == EASYMODE_BOAT;
}

static bool needsWingType(EasyModelType t)
{
  return t == EASYMODE_AIRPLANE || t == EASYMODE_GLIDER;
}

static bool needsTailType(EasyModelType t)
{
  return t == EASYMODE_AIRPLANE || t == EASYMODE_GLIDER || t == EASYMODE_HELICOPTER;
}

static bool needsAileron2(const EasyModeData& em)
{
  return (em.wingType == EASYWING_DUAL_AIL || em.wingType == EASYWING_FLAPERON ||
          em.wingType == EASYWING_2AIL_1FLAP || em.wingType == EASYWING_2AIL_2FLAP ||
          em.wingType == EASYWING_2AIL_4FLAP || em.wingType == EASYWING_ELEVON ||
          em.wingType == EASYWING_DELTA) && needsWingType(em.modelType);
}

static bool needsFlap(const EasyModeData& em)
{
  return em.wingType == EASYWING_1AIL_1FLAP || em.wingType == EASYWING_2AIL_1FLAP ||
         em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP;
}

static bool needsFlap2(const EasyModeData& em)
{
  return em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP;
}

static bool needsEle2(const EasyModeData& em)
{
  return (em.tailType == EASYTAIL_DUAL_ELEVATOR || em.tailType == EASYTAIL_AILEVATOR) &&
         em.channels.elevator2 >= 0;
}

static bool needsCrow(const EasyModeData& em)
{
  return (em.modelType == EASYMODE_GLIDER || em.modelType == EASYMODE_AIRPLANE) &&
         needsFlap(em);
}

static bool needsMotorSwitch(const EasyModeData& em)
{
  return em.modelType == EASYMODE_GLIDER && em.motorType != EASYMOTOR_NONE;
}

// Compute which rows are visible
static uint8_t getRowVisibility(uint8_t k)
{
  auto& em = g_easyMode;
  bool isHeli = (em.modelType == EASYMODE_HELICOPTER);
  bool isMulti = (em.modelType == EASYMODE_MULTIROTOR);
  bool active = (em.modelType != EASYMODE_NONE);

  switch (k) {
    case ITEM_EASYMODE_MODEL_TYPE:   return 0;
    case ITEM_EASYMODE_WING_TYPE:    return needsWingType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_TAIL_TYPE:    return needsTailType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_MOTOR_TYPE:   return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_HEADER:    return active ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_AIL:       return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_AIL2:      return needsAileron2(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_ELE:       return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_ELE2:      return needsEle2(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_THR:       return (em.motorType != EASYMOTOR_NONE || isSurfaceType(em.modelType) || isMulti) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_RUD:       return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_STEER:     return isSurfaceType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_FLAP:      return needsFlap(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_SRC_FLAP:     return needsFlap(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_FLAP2:     return needsFlap2(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_FLAP3:     return (em.wingType == EASYWING_2AIL_4FLAP) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_FLAP4:     return (em.wingType == EASYWING_2AIL_4FLAP) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CROW:         return needsCrow(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_SRC_MOTOR:    return needsMotorSwitch(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_AUX1:      return (em.channels.aux1 >= 0) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_SRC_AUX1:     return (em.channels.aux1 >= 0 && (isHeli || isMulti)) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CH_AUX2:      return (em.channels.aux2 >= 0) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_SRC_AUX2:     return (em.channels.aux2 >= 0 && isMulti) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_HEADER:   return active ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_EXPO_AIL: return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_EXPO_ELE: return (isAirType(em.modelType) && !isSurfaceType(em.modelType)) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_EXPO_RUD: return isAirType(em.modelType) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_DR_LOW:   return active ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_AIL_DIFF: return needsAileron2(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_AIL2RUD:  return (isAirType(em.modelType) && em.channels.rudder >= 0) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_FLAP2ELE: return needsFlap(em) ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_OPT_CH_ORDER: return isMulti ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_MIX_HEADER:   return active ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_MIX1_SRC:
    case ITEM_EASYMODE_MIX1_CH:
    case ITEM_EASYMODE_MIX1_WT:
    case ITEM_EASYMODE_MIX2_SRC:
    case ITEM_EASYMODE_MIX2_CH:
    case ITEM_EASYMODE_MIX2_WT:
    case ITEM_EASYMODE_MIX3_SRC:
    case ITEM_EASYMODE_MIX3_CH:
    case ITEM_EASYMODE_MIX3_WT:      return active ? 0 : HIDDEN_ROW;
    case ITEM_EASYMODE_CONVERT:      return active ? 0 : HIDDEN_ROW;
    default: return HIDDEN_ROW;
  }
}

static int8_t editChannel(const char* label, coord_t y, int8_t ch, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, label);
  int val = ch + 1;  // display as 1-based
  lcdDrawNumber(EASYMODE_COL2, y, val, attr | LEFT);
  if (attr) {
    val = checkIncDec(event, val, 1, EASYCH_MAX, EE_MODEL);
  }
  return val - 1;
}

static uint16_t editSource(const char* label, coord_t y, uint16_t val,
                            uint16_t max, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, label);
  drawSource(EASYMODE_COL2, y, val, attr);
  if (attr) CHECK_INCDEC_MODELSOURCE(event, val, 0, max);
  return val;
}

void menuModelEasyMode(event_t event)
{
  auto& em = g_easyMode;

  uint8_t rowVisibility[ITEM_EASYMODE_MAX];
  for (uint8_t i = 0; i < ITEM_EASYMODE_MAX; i++)
    rowVisibility[i] = getRowVisibility(i);

  MENU(STR_EASYMODE, menuTabModel, MENU_MODEL_EASYMODE, HEADER_LINE + ITEM_EASYMODE_MAX,
       { (uint8_t)(NAVIGATION_LINE_BY_LINE | rowVisibility[0]),
         rowVisibility[1], rowVisibility[2], rowVisibility[3],
         rowVisibility[4], rowVisibility[5], rowVisibility[6],
         rowVisibility[7], rowVisibility[8], rowVisibility[9],
         rowVisibility[10], rowVisibility[11], rowVisibility[12],
         rowVisibility[13], rowVisibility[14], rowVisibility[15],
         rowVisibility[16], rowVisibility[17], rowVisibility[18],
         rowVisibility[19], rowVisibility[20], rowVisibility[21],
         rowVisibility[22], rowVisibility[23], rowVisibility[24],
         rowVisibility[25], rowVisibility[26], rowVisibility[27],
         rowVisibility[28], rowVisibility[29], rowVisibility[30],
         rowVisibility[31], rowVisibility[32], rowVisibility[33],
         rowVisibility[34], rowVisibility[35], rowVisibility[36],
         rowVisibility[37], rowVisibility[38], rowVisibility[39],
         rowVisibility[40], rowVisibility[41], rowVisibility[42] });

  uint8_t sub = menuVerticalPosition - HEADER_LINE;
  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (uint8_t i = 0; i < NUM_BODY_LINES; i++) {
    uint8_t k = i + menuVerticalOffset;
    for (int j = 0; j <= k; j++) {
      if (j < ITEM_EASYMODE_MAX && rowVisibility[j] == HIDDEN_ROW) {
        k++;
      }
    }
    if (k >= ITEM_EASYMODE_MAX) break;
    LcdFlags blink = (s_editMode > 0 ? BLINK | INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_EASYMODE_MODEL_TYPE: {
        choice_t val = editChoice(EASYMODE_COL2, y, STR_EASYMODE_MODEL_TYPE,
                                   modelTypeValues, (int)em.modelType,
                                   EASYMODE_NONE, EASYMODE_BOAT, attr, event);
        if (val != em.modelType) {
          em.modelType = (EasyModelType)val;
          if (em.modelType != EASYMODE_NONE) {
            easyModeSetDefaults(em);
            easyModeApply(em);
          }
          storageDirty(EE_MODEL);
        }
        break;
      }

      case ITEM_EASYMODE_WING_TYPE: {
        choice_t val = editChoice(EASYMODE_COL2, y, STR_EASYMODE_WING_TYPE,
                                   wingTypeValues, (int)em.wingType,
                                   0, EASYWING_COUNT - 1, attr, event);
        if (val != em.wingType) {
          em.wingType = (EasyWingType)val;
          easyModeApply(em);
          storageDirty(EE_MODEL);
        }
        break;
      }

      case ITEM_EASYMODE_TAIL_TYPE: {
        choice_t val = editChoice(EASYMODE_COL2, y, STR_EASYMODE_TAIL_TYPE,
                                   tailTypeValues, (int)em.tailType,
                                   0, EASYTAIL_COUNT - 1, attr, event);
        if (val != em.tailType) {
          em.tailType = (EasyTailType)val;
          easyModeApply(em);
          storageDirty(EE_MODEL);
        }
        break;
      }

      case ITEM_EASYMODE_MOTOR_TYPE: {
        choice_t val = editChoice(EASYMODE_COL2, y, STR_EASYMODE_MOTOR_TYPE,
                                   motorTypeValues, (int)em.motorType,
                                   0, EASYMOTOR_COUNT - 1, attr, event);
        if (val != em.motorType) {
          em.motorType = (EasyMotorType)val;
          easyModeApply(em);
          storageDirty(EE_MODEL);
        }
        break;
      }

      case ITEM_EASYMODE_CH_HEADER:
        lcdDrawTextAlignedLeft(y, STR_EASYMODE_CHANNEL_MAP);
        break;

      case ITEM_EASYMODE_CH_AIL:
        em.channels.aileron = editChannel(STR_EASYMODE_CH_AILERON, y, em.channels.aileron, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_AIL2:
        em.channels.aileron2 = editChannel(STR_EASYMODE_CH_AIL2, y, em.channels.aileron2, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_ELE:
        em.channels.elevator = editChannel(STR_EASYMODE_CH_ELEVATOR, y, em.channels.elevator, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_ELE2:
        em.channels.elevator2 = editChannel(STR_EASYMODE_CH_ELE2, y, em.channels.elevator2, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_THR:
        em.channels.throttle = editChannel(STR_EASYMODE_CH_THROTTLE, y, em.channels.throttle, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_RUD:
        em.channels.rudder = editChannel(STR_EASYMODE_CH_RUDDER, y, em.channels.rudder, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_STEER:
        em.channels.steering = editChannel(STR_EASYMODE_CH_STEERING, y, em.channels.steering, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_FLAP:
        em.channels.flap = editChannel(STR_EASYMODE_CH_FLAP, y, em.channels.flap, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_SRC_FLAP:
        em.sources.flapSource = editSource(STR_EASYMODE_SRC_FLAP, y,
                                            em.sources.flapSource, MIXSRC_LAST_CH, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_FLAP2:
        em.channels.flap2 = editChannel(STR_EASYMODE_CH_FLAP2, y, em.channels.flap2, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_FLAP3:
        em.channels.flap3 = editChannel(STR_EASYMODE_CH_BRAKEL, y, em.channels.flap3, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_FLAP4:
        em.channels.flap4 = editChannel(STR_EASYMODE_CH_BRAKER, y, em.channels.flap4, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CROW: {
        lcdDrawTextAlignedLeft(y, STR_EASYMODE_CROW);
        uint8_t val = em.options.crowEnabled;
        lcdDrawChar(EASYMODE_COL2, y, val ? '1' : '0', attr);
        if (attr) {
          val = checkIncDec(event, val, 0, 1, EE_MODEL);
          if (val != em.options.crowEnabled) {
            em.options.crowEnabled = val;
            easyModeApply(em);
            storageDirty(EE_MODEL);
          }
        }
        break;
      }

      case ITEM_EASYMODE_SRC_MOTOR:
        em.sources.motorSource = editSource(STR_EASYMODE_SRC_MOTOR, y,
                                             em.sources.motorSource, MIXSRC_LAST_CH, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_CH_AUX1:
        em.channels.aux1 = editChannel(em.modelType == EASYMODE_HELICOPTER ? STR_EASYMODE_CH_GYRO : STR_EASYMODE_CH_AUX1,
                                        y, em.channels.aux1, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_SRC_AUX1: {
        const char* label = (em.modelType == EASYMODE_HELICOPTER) ?
                            STR_EASYMODE_SRC_GYRO : STR_EASYMODE_SRC_ARM;
        em.sources.aux1Source = editSource(label, y,
                                            em.sources.aux1Source, MIXSRC_LAST_CH, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;
      }

      case ITEM_EASYMODE_CH_AUX2:
        em.channels.aux2 = editChannel(STR_EASYMODE_CH_AUX2, y, em.channels.aux2, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_SRC_AUX2:
        em.sources.aux2Source = editSource(STR_EASYMODE_SRC_MODE, y,
                                            em.sources.aux2Source, MIXSRC_LAST_CH, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_HEADER:
        lcdDrawTextAlignedLeft(y, STR_EASYMODE_OPTIONS);
        break;

      case ITEM_EASYMODE_OPT_EXPO_AIL:
        em.options.expoAileron = editNumberField("  Ail.expo", INDENT_WIDTH, EASYMODE_COL2, y,
                                                  em.options.expoAileron, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_EXPO_ELE:
        em.options.expoElevator = editNumberField("  Ele.expo", INDENT_WIDTH, EASYMODE_COL2, y,
                                                   em.options.expoElevator, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_EXPO_RUD:
        em.options.expoRudder = editNumberField("  Rud.expo", INDENT_WIDTH, EASYMODE_COL2, y,
                                                 em.options.expoRudder, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_DR_LOW:
        em.options.dualRateLow = editNumberField("  D/R low", INDENT_WIDTH, EASYMODE_COL2, y,
                                                  em.options.dualRateLow, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_AIL_DIFF:
        em.options.aileronDifferential = editNumberField("  Ail.diff", INDENT_WIDTH, EASYMODE_COL2, y,
                                                          em.options.aileronDifferential, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_AIL2RUD:
        em.options.aileronToRudderMix = editNumberField("  Ail>Rud", INDENT_WIDTH, EASYMODE_COL2, y,
                                                         em.options.aileronToRudderMix, 0, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_FLAP2ELE:
        em.options.flapToElevatorComp = editNumberField("  Flp>Ele", INDENT_WIDTH, EASYMODE_COL2, y,
                                                         em.options.flapToElevatorComp, -100, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;

      case ITEM_EASYMODE_OPT_CH_ORDER: {
        static const char* const chOrderValues[] = { "AETR", "TAER", "RETA", nullptr };
        choice_t val = editChoice(EASYMODE_COL2, y, STR_EASYMODE_CH_ORDER,
                                   chOrderValues, (int)em.options.multiChannelOrder,
                                   0, EASYMULTI_ORDER_COUNT - 1, attr, event);
        if (val != em.options.multiChannelOrder) {
          em.options.multiChannelOrder = val;
          easyModeApply(em);
          storageDirty(EE_MODEL);
        }
        break;
      }

      case ITEM_EASYMODE_MIX_HEADER:
        lcdDrawTextAlignedLeft(y, STR_EASYMODE_CUSTOM_MIX);
        break;

      case ITEM_EASYMODE_MIX1_SRC:
      case ITEM_EASYMODE_MIX2_SRC:
      case ITEM_EASYMODE_MIX3_SRC: {
        uint8_t idx = (k - ITEM_EASYMODE_MIX1_SRC) / 3;
        char label[10];
        snprintf(label, sizeof(label), "Mix %d", idx + 1);
        em.customMix[idx].source = editSource(label, y,
                                               em.customMix[idx].source, MIXSRC_LAST, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;
      }

      case ITEM_EASYMODE_MIX1_CH:
      case ITEM_EASYMODE_MIX2_CH:
      case ITEM_EASYMODE_MIX3_CH: {
        uint8_t idx = (k - ITEM_EASYMODE_MIX1_CH) / 3;
        int val = em.customMix[idx].destCh + 1;
        lcdDrawTextAlignedLeft(y, "  CH");
        lcdDrawNumber(EASYMODE_COL2, y, val, attr | LEFT);
        if (attr) {
          val = checkIncDec(event, val, 0, EASYCH_MAX, EE_MODEL);
          if (val - 1 != em.customMix[idx].destCh) {
            em.customMix[idx].destCh = val - 1;
            easyModeApply(em);
            storageDirty(EE_MODEL);
          }
        }
        break;
      }

      case ITEM_EASYMODE_MIX1_WT:
      case ITEM_EASYMODE_MIX2_WT:
      case ITEM_EASYMODE_MIX3_WT: {
        uint8_t idx = (k - ITEM_EASYMODE_MIX1_WT) / 3;
        em.customMix[idx].weight = editNumberField("  Weight", INDENT_WIDTH, EASYMODE_COL2, y,
                                                    em.customMix[idx].weight, -100, 100, attr, event);
        if (attr && s_editMode > 0) { easyModeApply(em); storageDirty(EE_MODEL); }
        break;
      }

      case ITEM_EASYMODE_CONVERT:
        lcdDrawTextAlignedLeft(y, STR_EASYMODE_CONVERT);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          POPUP_CONFIRMATION(STR_EASYMODE_CONVERT_CONFIRM, [](const char* result) {
            if (result == STR_OK) {
              easyModeConvertToExpert();
            }
          });
          s_editMode = 0;
        }
        break;
    }

    y += FH;
  }
}
