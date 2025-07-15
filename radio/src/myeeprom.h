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

#ifndef _MYEEPROM_H_
#define _MYEEPROM_H_

#include "datastructs.h"
#include "bitfield.h"
#include "storage/yaml/yaml_defs.h"
#include "hal/switch_driver.h"

#define GET_MODULE_PPM_POLARITY(idx)             g_model.moduleData[idx].ppm.pulsePol
#define GET_TRAINER_PPM_POLARITY()               g_model.trainerData.pulsePol
#define GET_SBUS_POLARITY(idx)                   g_model.moduleData[idx].sbus.noninverted
#define GET_MODULE_PPM_DELAY(idx)                (g_model.moduleData[idx].ppm.delay * 50 + 300)
#define GET_TRAINER_PPM_DELAY()                  (g_model.trainerData.delay * 50 + 300)

#define IS_PLAY_FUNC(func)             ((func) >= FUNC_PLAY_SOUND && func <= FUNC_PLAY_VALUE)

#if defined(GVARS)
  #define IS_ADJUST_GV_FUNC(func)      ((func) == FUNC_ADJUST_GVAR)
#else
  #define IS_ADJUST_GV_FUNC(func)      (0)
#endif

#if defined(HAPTIC)
  #define IS_HAPTIC_FUNC(func)         ((func) == FUNC_HAPTIC)
#else
  #define IS_HAPTIC_FUNC(func)         (0)
#endif

#define HAS_REPEAT_PARAM(func)         (IS_PLAY_FUNC(func) || IS_HAPTIC_FUNC(func) || func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED || func == FUNC_SET_SCREEN)

#define CFN_EMPTY(p)                   (!(p)->swtch)
#define CFN_SWITCH(p)                  ((p)->swtch)
#define CFN_FUNC(p)                    ((p)->func)
#define CFN_ACTIVE(p)                  ((p)->active)
#define CFN_CH_INDEX(p)                ((p)->all.param)
#define CFN_CS_INDEX(p)                ((p)->all.param)
#define CFN_GVAR_INDEX(p)              ((p)->all.param)
#define CFN_TIMER_INDEX(p)             ((p)->all.param)
#define CFN_PLAY_REPEAT(p)             ((p)->repeat)
#define CFN_PLAY_REPEAT_MUL            1
#define CFN_PLAY_REPEAT_NOSTART        -1
#define CFN_GVAR_MODE(p)               ((p)->all.mode)
#define CFN_PARAM(p)                   ((p)->all.val)
#define CFN_VAL2(p)                    ((p)->all.val2)
#define CFN_RESET(p)                   ((p)->active=0, (p)->clear.val1=0, (p)->clear.val2=0)
#define CFN_GVAR_CST_MIN               -GVAR_MAX
#define CFN_GVAR_CST_MAX               GVAR_MAX
#define MODEL_GVAR_MIN(idx)            (CFN_GVAR_CST_MIN + g_model.gvars[idx].min)
#define MODEL_GVAR_MAX(idx)            (CFN_GVAR_CST_MAX - g_model.gvars[idx].max)

// pots config
#define POT_CFG_TYPE_BITS              3
#define POT_CFG_INV_BITS               1
#define POT_CFG_BITS                   (POT_CFG_TYPE_BITS + POT_CFG_INV_BITS)
#define POT_CFG_MASK                   ((1 << POT_CFG_BITS) - 1)
#define POT_CONFIG_POS(x)              (POT_CFG_BITS * (x))
#define POT_CONFIG_MASK(x)             (POT_CFG_MASK << POT_CONFIG_POS(x))
#define POT_CONFIG_DISABLE_MASK(x)     (~POT_CONFIG_MASK(x))

#define SW_CFG_BITS                    2
#define SW_CFG_MASK                    ((1 << SW_CFG_BITS) - 1)
#define SWITCH_CONFIG_MASK(x)          ((swconfig_t)SW_CFG_MASK << (SW_CFG_BITS * (x)))

#define SWITCH_CONFIG(x)              (bfGet<swconfig_t>(g_eeGeneral.switchConfig, SW_CFG_BITS * (x), SW_CFG_BITS))
#if defined(FUNCTION_SWITCHES)
  #define FSW_CFG_BITS                2
  #define FSWITCH_CONFIG(x)           (bfGet<swconfig_t>(g_model.functionSwitchConfig, FSW_CFG_BITS * (x), FSW_CFG_BITS))
  #define FSWITCH_SET_CONFIG(x,v)     g_model.functionSwitchConfig = bfSet<swconfig_t>(g_model.functionSwitchConfig, v, FSW_CFG_BITS * (x), FSW_CFG_BITS)
  #define FSWITCH_GROUP(x)            (bfGet<swconfig_t>(g_model.functionSwitchGroup, FSW_CFG_BITS * (x), FSW_CFG_BITS))
  #define FSWITCH_SET_GROUP(x,v)      g_model.functionSwitchGroup = bfSet<swconfig_t>(g_model.functionSwitchGroup, v, FSW_CFG_BITS * (x), FSW_CFG_BITS)
  #define FSWITCH_STARTUP(x)          (bfGet<swconfig_t>(g_model.functionSwitchStartConfig, FSW_CFG_BITS * (x), FSW_CFG_BITS))
  #define FSWITCH_SET_STARTUP(x,v)    g_model.functionSwitchStartConfig = bfSet<swconfig_t>(g_model.functionSwitchStartConfig, v, FSW_CFG_BITS * (x), FSW_CFG_BITS)
  #define IS_FSWITCH_GROUP_ON(x)      (bfGet<swconfig_t>(g_model.functionSwitchGroup, FSW_CFG_BITS * NUM_FUNCTIONS_SWITCHES + x, 1))
  #define SET_FSWITCH_GROUP_ON(x,v)   g_model.functionSwitchGroup = bfSet<swconfig_t>(g_model.functionSwitchGroup, v, FSW_CFG_BITS * NUM_FUNCTIONS_SWITCHES + x, 1)
  #define IS_SWITCH_FS(x)             (x >= switchGetMaxSwitches() && x < (switchGetMaxSwitches() + switchGetMaxFctSwitches()))
  #define SWITCH_EXISTS(x)            (IS_SWITCH_FS(x)  ? true : (SWITCH_CONFIG(x) != SWITCH_NONE))
  #define IS_CONFIG_3POS(x)           (IS_SWITCH_FS(x)  ? (FSWITCH_CONFIG(x - switchGetMaxSwitches()) == SWITCH_3POS) : (SWITCH_CONFIG(x) == SWITCH_3POS))
  #define IS_CONFIG_TOGGLE(x)         (IS_SWITCH_FS(x)  ? (FSWITCH_CONFIG(x - switchGetMaxSwitches()) == SWITCH_TOGGLE) : (SWITCH_CONFIG(x) == SWITCH_TOGGLE))
#else
  #define SWITCH_EXISTS(x)            (SWITCH_CONFIG(x) != SWITCH_NONE)
  #define IS_CONFIG_3POS(x)           (SWITCH_CONFIG(x) == SWITCH_3POS)
  #define IS_CONFIG_TOGGLE(x)         (SWITCH_CONFIG(x) == SWITCH_TOGGLE)
  #define IS_SWITCH_FS(x)             (false)
#endif
#define SWITCH_WARNING_ALLOWED(x)     (SWITCH_EXISTS(x) && !(IS_CONFIG_TOGGLE(x) || IS_SWITCH_FS(x)))

#define ALTERNATE_VIEW                0x10

#if defined(COLORLCD) && !defined(BOOT)
  #include "layout.h"
  #include "topbar.h"
#endif

#define SWITCHES_DELAY()            uint8_t(15+g_eeGeneral.switchesDelay)
#define SWITCHES_DELAY_NONE         (-15)
#define HAPTIC_STRENGTH()           (3+g_eeGeneral.hapticStrength)

enum CurveRefType {
  CURVE_REF_DIFF,
  CURVE_REF_EXPO,
  CURVE_REF_FUNC,
  CURVE_REF_CUSTOM
};

#define EXPO_VALID(ed)          ((ed)->mode)
#define EXPO_MODE_ENABLE(ed, v) (((v)<0 && ((ed)->mode&1)) || ((v)>=0 && ((ed)->mode&2)))

#define limit_min_max_t     int16_t
#define LIMIT_EXT_PERCENT   150
#define LIMIT_STD_PERCENT   100
#define LIMIT_EXT_MAX       (LIMIT_EXT_PERCENT*10)
#define LIMIT_STD_MAX       (LIMIT_STD_PERCENT*10)
#define PPM_CENTER_MAX      500
#define LIMIT_MAX(lim)                                            \
  (GV_IS_GV_VALUE(lim->max)                                       \
       ? GET_GVAR_PREC1(lim->max, -LIMIT_EXT_MAX, +LIMIT_EXT_MAX, \
                        mixerCurrentFlightMode)                   \
       : lim->max + LIMIT_STD_MAX)
#define LIMIT_MIN(lim)                                            \
  (GV_IS_GV_VALUE(lim->min)                                       \
       ? GET_GVAR_PREC1(lim->min, -LIMIT_EXT_MAX, +LIMIT_EXT_MAX, \
                        mixerCurrentFlightMode)                   \
       : lim->min - LIMIT_STD_MAX)
#define LIMIT_OFS(lim)                                               \
  (GV_IS_GV_VALUE(lim->offset)                                       \
       ? GET_GVAR_PREC1(lim->offset, -LIMIT_STD_MAX, +LIMIT_STD_MAX, \
                        mixerCurrentFlightMode)                      \
       : lim->offset)
#define LIMIT_MAX_RESX(lim) calc1000toRESX(LIMIT_MAX(lim))
#define LIMIT_MIN_RESX(lim) calc1000toRESX(LIMIT_MIN(lim))
#define LIMIT_OFS_RESX(lim) calc1000toRESX(LIMIT_OFS(lim))

#define LIMITS_MIN_MAX_OFFSET LIMIT_STD_MAX

#define TRIM_OFF    (1)
#define TRIM_ON     (0)

enum MixerMultiplex {
  MLTPX_ADD  = 0,
  MLTPX_MUL  = 1,
  MLTPX_REPL = 2,
};

enum TrainerMultiplex {
  TRAINER_OFF  = 0,
  TRAINER_ADD  = 1,
  TRAINER_REPL = 2,
};

#define DELAY_MAX       250 /* 25 seconds */
#define SLOW_MAX        250 /* 25 seconds */

#define MD_WEIGHT(md) (md->weight)
#define MD_WEIGHT_TO_UNION(md, var) var.word = md->weight
#define MD_UNION_TO_WEIGHT(var, md) md->weight = var.word

#define MD_OFFSET(md) (md->offset)
#define MD_OFFSET_TO_UNION(md, var) var.word = md->offset
#define MD_UNION_TO_OFFSET(var, md) md->offset = var.word

enum LogicalSwitchesFunctions {
  LS_FUNC_NONE,
  LS_FUNC_VEQUAL, // v==offset
  LS_FUNC_VALMOSTEQUAL, // v~=offset
  LS_FUNC_VPOS,   // v>offset
  LS_FUNC_VNEG,   // v<offset
  LS_FUNC_APOS,   // |v|>offset
  LS_FUNC_ANEG,   // |v|<offset
  LS_FUNC_AND,
  LS_FUNC_OR,
  LS_FUNC_XOR,
  LS_FUNC_EDGE,
  LS_FUNC_EQUAL,
  LS_FUNC_GREATER,
  LS_FUNC_LESS,
  LS_FUNC_DIFFEGREATER,
  LS_FUNC_ADIFFEGREATER,
  LS_FUNC_TIMER,
  LS_FUNC_STICKY,
  LS_FUNC_COUNT SKIP,
  LS_FUNC_MAX SKIP = LS_FUNC_COUNT-1
};

#define MAX_LS_DURATION 250 /*25s*/
#define MAX_LS_DELAY    250 /*25s*/
#define MAX_LS_ANDSW    SWSRC_LAST

enum TelemetrySensorType
{
  TELEM_TYPE_CUSTOM,
  TELEM_TYPE_CALCULATED
};

enum TelemetrySensorFormula
{
  TELEM_FORMULA_ADD,
  TELEM_FORMULA_AVERAGE,
  TELEM_FORMULA_MIN,
  TELEM_FORMULA_MAX,
  TELEM_FORMULA_MULTIPLY,
  TELEM_FORMULA_TOTALIZE,
  TELEM_FORMULA_CELL,
  TELEM_FORMULA_CONSUMPTION,
  TELEM_FORMULA_DIST,
  TELEM_FORMULA_LAST SKIP = TELEM_FORMULA_DIST
};

enum SwashType {
  SWASH_TYPE_NONE,
  SWASH_TYPE_120,
  SWASH_TYPE_120X,
  SWASH_TYPE_140,
  SWASH_TYPE_90,
  SWASH_TYPE_MAX SKIP = SWASH_TYPE_90
};

#define TRIM_EXTENDED_MAX 512
#define TRIM_EXTENDED_MIN (-TRIM_EXTENDED_MAX)
#define TRIM_MAX 128
#define TRIM_MIN (-TRIM_MAX)

#define TRIMS_ARRAY_SIZE  8
#define TRIM_MODE_NONE  0x1F  // 0b11111
#define TRIM_MODE_3POS  (2 * MAX_FLIGHT_MODES)

#define IS_MANUAL_RESET_TIMER(idx)     (g_model.timers[idx].persistent == 2)

#define TIMER_COUNTDOWN_START(x)       (g_model.timers[x].countdownStart == 0 ? 20 : (g_model.timers[x].countdownStart == 1 ? 30 : (g_model.timers[x].countdownStart == -1 ? 10 : 5)))

#include "pulses/modules_constants.h"

enum DisplayTrims
{
  DISPLAY_TRIMS_NEVER,
  DISPLAY_TRIMS_CHANGE,
  DISPLAY_TRIMS_ALWAYS
};

extern RadioData g_eeGeneral;
extern ModelData g_model;

constexpr uint8_t EE_GENERAL = 0x01;
constexpr uint8_t EE_MODEL = 0x02;
constexpr uint8_t EE_LABELS = 0x04;

#endif // _MYEEPROM_H_