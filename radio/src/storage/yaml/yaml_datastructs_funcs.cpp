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
#include "edgetx_constants.h"
#include "yaml_bits.h"
#include "yaml_node.h"
#include "yaml_tree_walker.h"

#include "pulses/multi.h"
#include "switches.h"
#include "analogs.h"
#include "stamp.h"

#include "hal/switch_driver.h"
#include "hal/adc_driver.h"
#include "hal/audio_driver.h"

#if defined(COLORLCD)
#include "radio_tools.h"
#endif

//
// WARNING:
// ========
//
//  If any of these static_assert() fails, you need to check that
//  the functions below are still applicable.
//
//  Please note that the sizes used here are those from the v220 format
//  (see storage/conversions/yaml/datastructs_220.h)
//
static inline void check_yaml_funcs()
{
  static_assert(offsetof(ModuleData, ppm) == 4,"");
  check_size<ModuleData, 29>();
#if defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5)
  static_assert(MAX_GVARS == 15,"");
#else
  static_assert(MAX_GVARS == 9,"");
#endif
}

//
// Arena extern array get_ptr callbacks for YAML_EXTERN_ARRAY
//
#include "model_arena.h"
#include "expos.h"
#include "mixes.h"

// YAML extern array callbacks.
// get_ptr returns the section base and current element count.
// ensure_capacity grows the arena section on demand during parsing.

#define YAML_GET_PTR(name, section) \
static uint8_t* yaml_get_##name##_ptr(uint16_t* count) { \
  *count = g_modelArena.sectionCount(section); \
  return g_modelArena.sectionBase(section); \
}

#define YAML_ENSURE(name, section, hard_max) \
static bool yaml_ensure_##name##_capacity(uint16_t min_count) { \
  if (min_count > hard_max) return false; \
  return g_modelArena.ensureSectionCapacity(section, min_count); \
}

YAML_GET_PTR(mix, ARENA_MIXES)
YAML_ENSURE(mix, ARENA_MIXES, MAX_MIXERS_HARD)

YAML_GET_PTR(expo, ARENA_EXPOS)
YAML_ENSURE(expo, ARENA_EXPOS, MAX_EXPOS_HARD)

YAML_GET_PTR(curves, ARENA_CURVES)
YAML_ENSURE(curves, ARENA_CURVES, MAX_CURVES_HARD)

YAML_GET_PTR(points, ARENA_POINTS)
YAML_ENSURE(points, ARENA_POINTS, MAX_CURVE_POINTS_HARD)

YAML_GET_PTR(logical_sw, ARENA_LOGICAL_SW)
YAML_ENSURE(logical_sw, ARENA_LOGICAL_SW, MAX_LOGICAL_SWITCHES_HARD)

YAML_GET_PTR(custom_fn, ARENA_CUSTOM_FN)
YAML_ENSURE(custom_fn, ARENA_CUSTOM_FN, MAX_SPECIAL_FUNCTIONS_HARD)

YAML_GET_PTR(fmd, ARENA_FLIGHT_MODES)
YAML_ENSURE(fmd, ARENA_FLIGHT_MODES, MAX_FLIGHT_MODES)

YAML_GET_PTR(gvar_data, ARENA_GVAR_DATA)
YAML_ENSURE(gvar_data, ARENA_GVAR_DATA, MAX_GVARS)

YAML_GET_PTR(gvar_values, ARENA_GVAR_VALUES)
YAML_ENSURE(gvar_values, ARENA_GVAR_VALUES, MAX_FLIGHT_MODES * MAX_GVARS)

YAML_GET_PTR(input_names, ARENA_INPUT_NAMES)
YAML_ENSURE(input_names, ARENA_INPUT_NAMES, MAX_INPUTS)


#undef YAML_GET_PTR
#undef YAML_ENSURE

// Radio arena callbacks (separate arena for radio custom functions)
static uint8_t* yaml_get_radio_cfn_ptr(uint16_t* count) {
  *count = g_radioArena.sectionCount(RADIO_ARENA_CUSTOM_FN);
  return g_radioArena.sectionBase(RADIO_ARENA_CUSTOM_FN);
}

static bool yaml_ensure_radio_cfn_capacity(uint16_t min_count) {
  if (min_count > MAX_SPECIAL_FUNCTIONS_HARD) return false;
  return g_radioArena.ensureSectionCapacity(RADIO_ARENA_CUSTOM_FN, min_count);
}



static bool w_semver(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque)
{
  return wf(opaque, VERSION, sizeof(VERSION)-1);
}

static bool w_board(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque)
{
  return wf(opaque, FLAVOUR, sizeof(FLAVOUR)-1);
}

static uint32_t in_read_weight(const YamlNode* node, const char* val, uint8_t val_len)
{
  if ((strncmp(val, "GV", 2) == 0) || (strncmp(val, "-GV", 3) == 0)) {
    bool neg = false;
    int ofst = 2;
    if (val[0] == '-') {
      neg = true;
      ofst = 3;
    }
    int32_t idx = yaml_str2int(val + ofst, val_len - ofst);
    // Convert to range -MAX_GVARS .. MAX_GVARS - 1
    if (neg)
      idx = -idx;
    else
      idx = idx - 1;
    return GV_VALUE_FROM_INDEX(idx);
  }

  return (uint32_t)(int16_t)GV_ENCODE(yaml_str2int(val, val_len));
}

bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                     void* opaque)
{
  int32_t sval = yaml_to_signed(val, node->size);

  if (GV_IS_GV_VALUE(sval)) {
    char s[8] = "";
    int ofst = 0;
    int idx = GV_INDEX_FROM_VALUE(sval);
    if (idx < 0) {
      s[0] = '-';
      ofst = 1;
      idx = -idx;
    } else {
      idx = idx + 1;
    }
    strAppendStringWithIndex(s + ofst, "GV", idx);
    return wf(opaque, s, strlen(s));
  }

  char* s = yaml_signed2str(GV_DECODE(sval));
  return wf(opaque, s, strlen(s));
}

static int _legacy_input_idx(const char* val, uint8_t val_len)
{
  for (uint8_t i = 0; i < DIM(_legacy_inputs); i++) {
    if (!strncmp(_legacy_inputs[i].legacy, val, val_len))
      return i;
  }
  return -1;
}

static SourceRef _legacy_mix_src(const char* val, uint8_t val_len)
{
  int idx = _legacy_input_idx(val, val_len);
  if (idx >= 0) return _legacy_inputs[idx].ref;
  return {};
}

// These enums are no longer auto-generated (SourceRef/SwitchRef replaced the enum types in structs)
// but are still needed by the YAML custom read/write functions for backward compatibility.
const struct YamlIdStr enum_MixSources[] = {
  {  MIXSRC_NONE, "NONE"  },
  {  MIXSRC_MIN, "MIN"  },
  {  MIXSRC_MAX, "MAX"  },
  {  MIXSRC_TX_VOLTAGE, "TX_VOLTAGE"  },
  {  MIXSRC_TX_TIME, "TX_TIME"  },
  {  MIXSRC_TX_GPS, "TX_GPS"  },
  {  0, NULL  }
};

// Find next ',' separator, return length up to; but not inclding separator.
uint8_t find_sep(const char* val, uint8_t val_len)
{
  // find ","
  const char* sep = (const char *)memchr(val, ',', val_len);
  if (sep) {
    // Special case - check for '(x,y)' in string. If found skip past closing bracket
    const char* bkt = (const char *)memchr(val, '(', val_len);
    if (bkt && bkt < sep) {
      // Found '(' before ','
      bkt = (const char *)memchr(val, ')', val_len);
      if (bkt && bkt > sep) {
        // Found ')' after ','
        sep = (const char *)memchr(bkt, ',', val_len-(bkt-val));
      }
    }
  }
  // Return length up to ',' (or full length if not found)
  return sep ? sep - val : val_len;
}

// sources: parse/output
//  - lua(script#,n): LUA mix outputs
//  - ls(n): logical switches
//  - tr(n): trainer input
//  - ch(n): channels
//  - gv(n): gvars
//  - tele(n): telemetry
//
// --- SourceRef YAML parsing (text → SourceRef) ---

// Backward-compat enum table for singleton sources
static const struct {
  const char* name;
  uint8_t type;
} sourceEnumTable[] = {
  { "NONE",       SOURCE_TYPE_NONE },
  { "MIN",        SOURCE_TYPE_MIN },
  { "MAX",        SOURCE_TYPE_MAX },
  { "TX_VOLTAGE", SOURCE_TYPE_TX_VOLTAGE },
  { "TX_TIME",    SOURCE_TYPE_TX_TIME },
  { "TX_GPS",     SOURCE_TYPE_TX_GPS },
  { "LIGHT",      SOURCE_TYPE_LIGHT },
};

static SourceRef yaml_parse_source(const char* val, uint8_t val_len)
{
    if (val_len > 0 && val[0] == 'I') {
        return SourceRef_(SOURCE_TYPE_INPUT, (uint16_t)yaml_str2uint(val+1, val_len-1));
    } else if (val_len > 4 &&
               val[0] == 'l' && val[1] == 'u' && val[2] == 'a' && val[3] == '(') {
      val += 4; val_len -= 4;
      uint8_t script = yaml_str2uint_ref(val, val_len);
      if (!val_len) return {};
      val++; val_len--;
      uint16_t idx = script * MAX_SCRIPT_OUTPUTS + yaml_str2uint(val, val_len);
      return SourceRef_(SOURCE_TYPE_LUA, idx);
    } else if (val_len > 3 &&
               val[0] == 'l' && val[1] == 's' && val[2] == '(') {
      val += 3; val_len -= 3;
      return SourceRef_(SOURCE_TYPE_LOGICAL_SWITCH, (uint16_t)(yaml_str2uint(val, val_len) - 1));
    } else if (val_len > 3 &&
               val[0] == 't' && val[1] == 'r' && val[2] == '(') {
      val += 3; val_len -= 3;
      return SourceRef_(SOURCE_TYPE_TRAINER, (uint16_t)yaml_str2uint(val, val_len));
    } else if (val_len > 3 &&
               val[0] == 'c' && val[1] == 'h' && val[2] == '(') {
      val += 3; val_len -= 3;
      return SourceRef_(SOURCE_TYPE_CHANNEL, (uint16_t)yaml_str2uint(val, val_len));
    } else if (val_len > 3 &&
               val[0] == 'g' && val[1] == 'v' && val[2] == '(') {
      val += 3; val_len -= 3;
      return SourceRef_(SOURCE_TYPE_GVAR, (uint16_t)yaml_str2uint(val, val_len));
#if defined(FUNCTION_SWITCHES)
    } else if (val_len > 2 &&
               val[0] == 'G' && val[1] == 'R' &&
               val[2] >= '1' && val[2] <= '3') {
      return SourceRef_(SOURCE_TYPE_CUSTOM_SWITCH_GROUP, (uint16_t)(val[2] - '1'));
#endif
    } else if (val_len > 5 &&
               val[0] == 't' && val[1] == 'e' && val[2] == 'l' && val[3] == 'e' && val[4] == '(') {
      val += 5; val_len -= 5;
      uint8_t sign = 0;
      if (*val == '-') { sign = 1; val++; val_len--; }
      else if (*val == '+') { sign = 2; val++; val_len--; }
      uint16_t idx = yaml_str2uint(val, val_len) * 3 + sign;
      return SourceRef_(SOURCE_TYPE_TELEMETRY, idx);
    } else if (val_len > 3 &&
               val[0] == 'C' && val[1] == 'Y' && val[2] == 'C' &&
               val[3] >= '1' && val[3] <= '3') {
      return SourceRef_(SOURCE_TYPE_HELI, (uint16_t)(val[3] - '1'));
    } else if (val_len > 3 &&
               val[0] == 'T' && val[1] == 'm' && val[2] == 'r' &&
               val[3] >= '1' && val[3] <= ('0' + MAX_TIMERS)) {
      return SourceRef_(SOURCE_TYPE_TIMER, (uint16_t)(val[3] - '1'));
    } else if (val_len > 1 &&
               val[0] == 'T' && val[1] >= '1' && val[1] <= ('0' + MAX_TRIMS)) {
      return SourceRef_(SOURCE_TYPE_TRIM, (uint16_t)(val[1] - '1'));
    }

    auto idx = analogLookupCanonicalIdx(ADC_INPUT_MAIN, val, val_len);
    if (idx >= 0) return SourceRef_(SOURCE_TYPE_STICK, (uint16_t)idx);

    idx = analogLookupCanonicalIdx(ADC_INPUT_FLEX, val, val_len);
    if (idx >= 0) return SourceRef_(SOURCE_TYPE_POT, (uint16_t)idx);

    idx = switchLookupIdx(val, val_len);
    if (idx >= 0) return SourceRef_(SOURCE_TYPE_SWITCH, (uint16_t)idx);

    // Legacy input names (backward compat)
    SourceRef legacy = _legacy_mix_src(val, val_len);
    if (!legacy.isNone()) return legacy;

    // Singleton source enum names (NONE, MIN, MAX, TX_VOLTAGE, etc.)
    for (const auto& e : sourceEnumTable) {
      if (val_len == strlen(e.name) && strncmp(val, e.name, val_len) == 0)
        return {e.type, 0, 0};
    }

    return {};
}

// --- SourceRef YAML writing (SourceRef → text) ---

static constexpr char closing_parenthesis[] = ")";

static bool output_source_1_param(const char* src_prefix, size_t src_len, uint32_t n,
                           yaml_writer_func wf, void* opaque)
{
  if (!wf(opaque, src_prefix, src_len)) return false;
  const char* str = yaml_unsigned2str(n);
  if (!wf(opaque, str, strlen(str))) return false;
  return true;
}

static bool yaml_write_source(const SourceRef& ref, yaml_writer_func wf, void* opaque)
{
    const char* str = nullptr;

    switch (ref.type) {
      case SOURCE_TYPE_NONE:
        return wf(opaque, "NONE", 4);
      case SOURCE_TYPE_INPUT:
        if (!wf(opaque, "I", 1)) return false;
        str = yaml_unsigned2str(ref.index);
        break;
#if defined(LUA_INPUTS)
      case SOURCE_TYPE_LUA:
      {
        uint32_t script = ref.index / MAX_SCRIPT_OUTPUTS;
        if (!output_source_1_param("lua(", 4, script, wf, opaque)) return false;
        if (!wf(opaque, ",", 1)) return false;
        uint32_t output = ref.index % MAX_SCRIPT_OUTPUTS;
        str = yaml_unsigned2str(output);
        if (!wf(opaque, str, strlen(str))) return false;
        str = closing_parenthesis;
        break;
      }
#endif
      case SOURCE_TYPE_STICK:
        str = analogGetCanonicalName(ADC_INPUT_MAIN, ref.index);
        break;
      case SOURCE_TYPE_POT:
        str = analogGetCanonicalName(ADC_INPUT_FLEX, ref.index);
        break;
      case SOURCE_TYPE_HELI:
        if (!wf(opaque, "CYC", 3)) return false;
        str = yaml_unsigned2str(ref.index + 1);
        break;
      case SOURCE_TYPE_TRIM:
        if (!wf(opaque, "T", 1)) return false;
        str = yaml_unsigned2str(ref.index + 1);
        break;
      case SOURCE_TYPE_SWITCH:
        str = switchGetDefaultName(ref.index);
        break;
#if defined(FUNCTION_SWITCHES)
      case SOURCE_TYPE_CUSTOM_SWITCH_GROUP:
        str = fsSwitchGroupGetCanonicalName(ref.index);
        break;
#endif
      case SOURCE_TYPE_LOGICAL_SWITCH:
        if (!output_source_1_param("ls(", 3, ref.index + 1, wf, opaque)) return false;
        str = closing_parenthesis;
        break;
      case SOURCE_TYPE_TRAINER:
        if (!output_source_1_param("tr(", 3, ref.index, wf, opaque)) return false;
        str = closing_parenthesis;
        break;
      case SOURCE_TYPE_CHANNEL:
        if (!output_source_1_param("ch(", 3, ref.index, wf, opaque)) return false;
        str = closing_parenthesis;
        break;
      case SOURCE_TYPE_GVAR:
        if (!output_source_1_param("gv(", 3, ref.index, wf, opaque)) return false;
        str = closing_parenthesis;
        break;
      case SOURCE_TYPE_TIMER:
        if (!wf(opaque, "Tmr", 3)) return false;
        str = yaml_unsigned2str(ref.index + 1);
        break;
      case SOURCE_TYPE_TELEMETRY:
      {
        uint8_t sign = ref.index % 3;
        uint32_t sensor = ref.index / 3;
        if (!wf(opaque, "tele(", 5)) return false;
        if (sign == 1) { if (!wf(opaque, "-", 1)) return false; }
        else if (sign == 2) { if (!wf(opaque, "+", 1)) return false; }
        str = yaml_unsigned2str(sensor);
        if (!wf(opaque, str, strlen(str))) return false;
        str = closing_parenthesis;
        break;
      }
      case SOURCE_TYPE_MIN:
        return wf(opaque, "MIN", 3);
      case SOURCE_TYPE_MAX:
        return wf(opaque, "MAX", 3);
      case SOURCE_TYPE_TX_VOLTAGE:
        return wf(opaque, "TX_VOLTAGE", 10);
      case SOURCE_TYPE_TX_TIME:
        return wf(opaque, "TX_TIME", 7);
      case SOURCE_TYPE_TX_GPS:
        return wf(opaque, "TX_GPS", 6);
      case SOURCE_TYPE_LIGHT:
        return wf(opaque, "LIGHT", 5);
      default:
        return wf(opaque, "NONE", 4);
    }

    if (str) return wf(opaque, str, strlen(str));
    return true;
}


static void r_rssiDisabled(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  data -= offsetof(ModelData, rfAlarms);
  auto md = reinterpret_cast<ModelData*>(data);
  md->disableTelemetryWarning = yaml_str2int(val, val_len);
}

static void r_rssiWarning(void* user, uint8_t* data, uint32_t bitoffs,
                          const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  auto rf_alarm = reinterpret_cast<RFAlarmData*>(data);
  rf_alarm->warning = yaml_str2int(val, val_len) + 45;
}

static void r_rssiCritical(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  auto rf_alarm = reinterpret_cast<RFAlarmData*>(data);
  rf_alarm->critical = yaml_str2int(val, val_len) + 42;
}

static uint32_t r_vbat_min(const YamlNode* node, const char* val, uint8_t val_len)
{
    int32_t v = yaml_str2int(val, val_len);
    return (uint32_t)(v - 90);
}

static bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 90);
  return wf(opaque, s, strlen(s));
}

static uint32_t r_vbat_max(const YamlNode* node, const char* val, uint8_t val_len)
{
  int32_t v = yaml_str2int(val, val_len);
  return (uint32_t)(v - 120);
}

static bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 120);
  return wf(opaque, s, strlen(s));
}

#if defined(COLORLCD)
bool screen_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  auto screenData = g_model.getScreenData(tw->getElmts());

  return !screenData->LayoutId.empty();
}

void r_screen_id(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  auto screenData = g_model.getScreenData(tw->getElmts(1));

  // Convert renamed layout (TODO: remove in the future)
  if (strncmp(val, "Layout6x1", 9) == 0)
    screenData->LayoutId ="Layout1x6";
  else
    screenData->LayoutId = val;
}

bool w_screen_id(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  auto screenData = g_model.getScreenData(tw->getElmts(1));

  return wf(opaque, screenData->LayoutId.c_str(), screenData->LayoutId.size());
}

static ZonePersistentData* get_zoneData(void* user)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  int lvl = tw->getLevel();
  bool topBar = (lvl == 3) || (lvl == 4);
  int o = (lvl == 3) || (lvl == 5) ? 0 : 1;

  uint16_t zone = tw->getElmts(o);
  if (topBar) {
    // Top Bar
    return &g_model.getTopbarData()->zones[zone];
  } else {
    // Screen
    uint16_t screen = tw->getElmts(o + 3);
    return &g_model.getScreenLayoutData(screen)->zones[zone];
  }
}

bool widget_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto zoneData = get_zoneData(user);

  return zoneData->widgetName[0] != 0;
}

void r_widget_name(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  auto zoneData = get_zoneData(user);

  zoneData->widgetName = val;
}

bool w_widget_name(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  auto zoneData = get_zoneData(user);

  return wf(opaque, zoneData->widgetName.c_str(), zoneData->widgetName.size());
}

static WidgetPersistentData* get_widgetData(void* user, uint16_t& option)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  int lvl = tw->getLevel();
  bool topBar = (lvl == 7) || (lvl == 8);
  int o = ((lvl == 7) || (lvl == 9)) ? 1 : 2;

  option = tw->getElmts(o);
  uint16_t zone = tw->getElmts(o + 3);
  if (topBar) {
    // Top Bar
    return g_model.getTopbarData()->getWidgetData(zone);
  } else {
    // Screen
    uint16_t screen = tw->getElmts(o + 6);
    return g_model.getScreenLayoutData(screen)->getWidgetData(zone);
  }
}

bool widget_option_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  int lvl = tw->getLevel();
  bool topBar = (lvl == 6);
  int o = 0;

  WidgetPersistentData* widgetData;

  uint16_t option = tw->getElmts(o);
  uint16_t zone = tw->getElmts(o + 3);
  if (topBar) {
    // Top Bar
    widgetData = g_model.getTopbarData()->getWidgetData(zone);
  } else {
    // Screen
    uint16_t screen = tw->getElmts(o + 6);
    widgetData = g_model.getScreenLayoutData(screen)->getWidgetData(zone);
  }

  return widgetData->hasOption(option);
}

static uint32_t get_color(const char* val, uint8_t val_len)
{
  if (strncmp(val, "COLIDX", 6) == 0) {
    val += 6; val_len -= 6;
    return COLOR2FLAGS(yaml_str2uint(val, val_len));
  } else {
    if (val_len < sizeof("0xFFFFFF")-1
        || val[0] != '0'
        || val[1] != 'x')
      return 0;

    val += 2; val_len -= 2;

    auto rgb24 = yaml_hex2uint(val, val_len);
    return RGB2FLAGS((rgb24 & 0xFF0000) >> 16, (rgb24 & 0xFF00) >> 8, rgb24 & 0xFF);
  }
}

static bool set_color(uint32_t val, yaml_writer_func wf, void* opaque)
{
  if (val & RGB_FLAG) {
    val = COLOR_VAL(val);
    uint32_t color = (uint32_t)GET_RED(val) << 16 |
                     (uint32_t)GET_GREEN(val) << 8 | (uint32_t)GET_BLUE(val);

    if (!wf(opaque, "0x", 2)) return false;
    return wf(opaque, yaml_rgb2hex(color), 3 * 2);
  } else {
    if (!wf(opaque, "COLIDX", 6)) return false;
    const char* str = yaml_unsigned2str(COLOR_VAL(val));
    return wf(opaque, str, strlen(str));
  }
}

static uint8_t select_wov(void* user, uint8_t* data, uint32_t bitoffs)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  return widgetData->getType(option);
}

const struct YamlIdStr enum_WidgetOptionValueEnum[] = {
  {  WOV_Unsigned, "Unsigned"  },
  {  WOV_Signed, "Signed"  },
  {  WOV_Bool, "Bool"  },
  {  WOV_String, "String"  },
  {  WOV_Source, "Source"  },
  {  WOV_Color, "Color"  },
  {  0, NULL  }
};

void r_wov_type(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  widgetData->setType(option, (WidgetOptionValueEnum)yaml_parse_enum(enum_WidgetOptionValueEnum, val, val_len));
}

bool w_wov_type(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  const char* str = yaml_output_enum(widgetData->getType(option), enum_WidgetOptionValueEnum);
  return wf(opaque, str, strlen(str));
}

void r_wov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  SourceRef ref = yaml_parse_source(val, val_len);
  widgetData->setUnsignedValue(option, ref.toUint32());
}

bool w_wov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  SourceRef ref = SourceRef::fromUint32(widgetData->getUnsignedValue(option));
  return yaml_write_source(ref, wf, opaque);
}

void r_wov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  widgetData->setUnsignedValue(option, get_color(val, val_len));
}

bool w_wov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  return set_color(widgetData->getUnsignedValue(option), wf, opaque);
}

void r_wov_string(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  widgetData->setString(option, val);
}

bool w_wov_string(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  return wf(opaque, widgetData->getString(option).c_str(), widgetData->getString(option).size());
}

void r_wov_unsigned(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  widgetData->setUnsignedValue(option, yaml_str2uint(val, val_len));
}

bool w_wov_unsigned(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  const char* str = yaml_unsigned2str(widgetData->getUnsignedValue(option));
  return wf(opaque, str, strlen(str));
}

void r_wov_signed(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  widgetData->setSignedValue(option, yaml_str2int(val, val_len));
}

bool w_wov_signed(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto widgetData = get_widgetData(user, option);

  const char* str = yaml_signed2str(widgetData->getSignedValue(option));
  return wf(opaque, str, strlen(str));
}

static LayoutPersistentData* get_LayoutData(void* user, uint16_t& option)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  int lvl = tw->getLevel();
  int o = (lvl == 6) ? 1 : 2;

  option = tw->getElmts(o);
  // Screen
  uint16_t screen = tw->getElmts(o + 3);
  return g_model.getScreenLayoutData(screen);
}

bool layout_option_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  int o = 0;

  uint16_t option = tw->getElmts(o);
  uint16_t screen = tw->getElmts(o + 3);
  auto layoutData = g_model.getScreenLayoutData(screen);

  return layoutData->options[option].type != LOV_None || layoutData->options[option].value.unsignedValue != 0;
}

static uint8_t select_lov(void* user, uint8_t* data, uint32_t bitoffs)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  return layoutData->options[option].type;
}

const struct YamlIdStr enum_LayoutOptionValueEnum[] = {
  {  LOV_None, "None"  },
  {  LOV_Bool, "Bool"  },
  {  LOV_Color, "Color"  },
  {  0, NULL  }
};

void r_lov_type(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  layoutData->options[option].type = (LayoutOptionValueEnum)yaml_parse_enum(enum_LayoutOptionValueEnum, val, val_len);
}

bool w_lov_type(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  const char* str = yaml_output_enum(layoutData->options[option].type, enum_LayoutOptionValueEnum);
  return wf(opaque, str, strlen(str));
}

void r_lov_unsigned(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  layoutData->options[option].value.unsignedValue = yaml_str2uint(val, val_len);
}

bool w_lov_unsigned(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  const char* str = yaml_unsigned2str(layoutData->options[option].value.unsignedValue);
  return wf(opaque, str, strlen(str));
}

void r_lov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  layoutData->options[option].value.unsignedValue = get_color(val, val_len);
}

bool w_lov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  uint16_t option;
  auto layoutData = get_LayoutData(user, option);

  return set_color(layoutData->options[option].value.unsignedValue, wf, opaque);
}

extern const struct YamlIdStr enum_QMPage[];

void r_keyShortcut(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  event_t ev = g_eeGeneral.getKeyShortcutEvent(tw->getElmts(1));
  bool isApp = (strncmp(val, "APP,", 4) == 0);
  QMPage pg = isApp ? QM_APP : (QMPage)yaml_parse_enum(enum_QMPage, val, val_len);
  g_eeGeneral.setKeyShortcut(ev, pg);
  if (isApp)
    g_eeGeneral.setKeyToolName(ev, val+4);
}

bool w_keyShortcut(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  event_t ev = g_eeGeneral.getKeyShortcutEvent(tw->getElmts(1));
  QMPage pg = g_eeGeneral.getKeyShortcut(ev);

  const char* str = yaml_output_enum(pg, enum_QMPage);
  if (!wf(opaque, str, strlen(str))) return false;

  if (pg == QM_APP) {
    auto s = g_eeGeneral.getKeyToolName(ev);
    if (!wf(opaque, ",", 1)) return false;
    return wf(opaque, s.c_str(), s.size());
  }

  return true;
}

void r_qmFavorite(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  int idx = tw->getElmts(1);
  bool isApp = (strncmp(val, "APP,", 4) == 0);
  QMPage pg = isApp ? QM_APP : (QMPage)yaml_parse_enum(enum_QMPage, val, val_len);
  g_eeGeneral.qmFavorites[idx].shortcut = pg;
  if (isApp)
    g_eeGeneral.setFavoriteToolName(idx, val+4);
}

bool w_qmFavorite(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  int idx = tw->getElmts(1);
  QMPage pg = (QMPage)g_eeGeneral.qmFavorites[idx].shortcut;

  const char* str = yaml_output_enum(pg, enum_QMPage);
  if (!wf(opaque, str, strlen(str))) return false;

  if (pg == QM_APP) {
    auto s = g_eeGeneral.getFavoriteToolName(idx);
    if (!wf(opaque, ",", 1)) return false;
    return wf(opaque, s.c_str(), s.size());
  }

  return true;
}
#endif

static uint8_t select_mod_type(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= offsetof(ModuleData, ppm);

  ModuleData* mod_data = reinterpret_cast<ModuleData*>(data);
  switch (mod_data->type) {
    case MODULE_TYPE_NONE:
    case MODULE_TYPE_PPM:
    case MODULE_TYPE_DSM2:
      return 1;
    case MODULE_TYPE_MULTIMODULE:
      return 2;
    case MODULE_TYPE_XJT_PXX1:
    case MODULE_TYPE_R9M_PXX1:
    case MODULE_TYPE_R9M_LITE_PXX1:
      return 3;
    case MODULE_TYPE_SBUS:
      return 4;
    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_PXX2:
    case MODULE_TYPE_R9M_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
    case MODULE_TYPE_XJT_LITE_PXX2:
      return 5;
    case MODULE_TYPE_FLYSKY_AFHDS2A:
      return 6;
    case MODULE_TYPE_FLYSKY_AFHDS3:
      return 7;
      break;
    case MODULE_TYPE_GHOST:
      return 8;
    case MODULE_TYPE_CROSSFIRE:
      return 9;
    case MODULE_TYPE_LEMON_DSMP:
      return 10;
  }
  return 0;
}

static uint8_t select_script_input(void* user, uint8_t* data, uint32_t bitoffs)
{
  // always use 'value'
  return 0;
}

static uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->type == TELEM_TYPE_CALCULATED
      && sensor->persistent)
    return 1;

  return 0;
}

static uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= 2 /* size of id1 union */;
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->type == TELEM_TYPE_CALCULATED)
    return 2; // formula
  
  return 1; // instance
}

static uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= offsetof(TelemetrySensor, param);
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->unit < UNIT_FIRST_VIRTUAL) {
    if (sensor->type == TELEM_TYPE_CALCULATED) {
      switch(sensor->formula) {
      case TELEM_FORMULA_CELL: return 1; // cell
      case TELEM_FORMULA_DIST: return 4; // dist
      case TELEM_FORMULA_CONSUMPTION: return 3; // consumption
      case TELEM_FORMULA_TOTALIZE: return 3; // consumption
      default: return 2; // calc
      }
    } else {
      return 0; // custom
    }
  }
  
  return 5;
}

static uint32_t r_calib(void* user, const char* val, uint8_t val_len)
{
  (void)user;

  int idx = adcGetInputIdx(val, val_len);
  if (idx >= 0) return idx;

  idx = _legacy_input_idx(val, val_len);
  if (idx >= 0) return idx;

  // detect invalid values
  if (val_len == 0 || (val[0] < '0') || (val[0] > '9')) {
    return -1;
  }
  
  return (uint32_t)yaml_str2int(val, val_len);
}

static bool w_calib(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = adcGetInputName(idx);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void _read_analog_name(uint8_t type, void* user, uint8_t* data,
                              uint32_t bitoffs, const char* val,
                              uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);
  analogSetCustomLabel(type, idx, val, val_len);
}

static bool _write_analog_name(uint8_t type, void* user, uint8_t* data,
                               uint32_t bitoffs, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  const char* name = analogGetCustomLabel(type, idx);
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, name, strlen(name))) return false;
  return wf(opaque, "\"", 1);
}

static void r_stick_name(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  _read_analog_name(ADC_INPUT_MAIN, user, data, bitoffs, val, val_len);
}

static bool w_stick_name(void* user, uint8_t* data, uint32_t bitoffs,
                         yaml_writer_func wf, void* opaque)
{
  return _write_analog_name(ADC_INPUT_MAIN, user, data, bitoffs, wf, opaque);
}

static void r_stick_inv(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);
  setStickInversion(idx, val[0] != '0');
}

static bool w_stick_inv(void* user, uint8_t* data, uint32_t bitoffs,
                         yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);
  return wf(opaque, getStickInversion(idx) ? "1" : "0", 1);
}

static bool stick_name_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  if (getStickInversion(idx)) return true;
  return analogHasCustomLabel(ADC_INPUT_MAIN, idx);
}

static const struct YamlNode struct_stickConfig[] = {
    YAML_IDX,
    YAML_CUSTOM( "name", r_stick_name, w_stick_name),
    YAML_CUSTOM("inv", r_stick_inv, w_stick_inv),
    YAML_END
};

static uint32_t slider_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  SourceRef ref = _legacy_mix_src(val, val_len);
  if (ref.type == SOURCE_TYPE_POT) return ref.index;

  return -1;
}

static const struct YamlIdStr enum_SliderConfig[] = {
    {  FLEX_NONE, "none" },
    {  FLEX_SLIDER, "with_detent" },
    {  0, NULL }
};

static void sl_type_read(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  bitoffs += POT_CFG_BITS * idx;
  data += bitoffs >> 3UL;
  bitoffs &= 7;

  auto cfg = yaml_parse_enum(enum_SliderConfig, val, val_len);
  yaml_put_bits(data, cfg, bitoffs, POT_CFG_BITS);
}

static void sl_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  _read_analog_name(ADC_INPUT_FLEX, user, data, bitoffs, val, val_len);
}

static const struct YamlNode struct_sliderConfig[] = {
    YAML_IDX_CUST( "sl", slider_read, nullptr ),
    YAML_CUSTOM( "type", sl_type_read, nullptr ),
    YAML_CUSTOM( "name", sl_name_read, nullptr ),
    YAML_END
};

static uint32_t sw_idx_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  return switchLookupIdx(val, val_len);
}

static bool currentSwitchIsCFS = false;

bool sw_idx_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  currentSwitchIsCFS = switchIsCustomSwitch(idx);

  const char* str = switchGetDefaultName(idx);
  return str ? wf(opaque, str, strlen(str)) : true;
}

bool switch_is_cfs(void* user, uint8_t* data, uint32_t bitoffs)
{
  return currentSwitchIsCFS;
}

static bool flex_sw_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  return switchIsFlexValid_raw(idx);
}

static uint32_t flex_sw_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  auto idx = switchLookupIdx(val, val_len);
  return idx - switchGetMaxSwitches();
}

bool flex_sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  auto sw_offset = switchGetMaxSwitches();
  const char* str = switchGetDefaultName(idx + sw_offset);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void r_flex_sw_channel(void* user, uint8_t* data, uint32_t bitoffs,
			      const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  auto channel = analogLookupPhysicalIdx(ADC_INPUT_FLEX, val, val_len);
  switchConfigFlex_raw(idx, channel);
}

static bool w_flex_sw_channel(void* user, uint8_t* data, uint32_t bitoffs,
			      yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  auto channel = switchGetFlexConfig_raw(idx);
  const char* s = analogGetPhysicalName(ADC_INPUT_FLEX, channel);
  return s ? wf(opaque, s, strlen(s)) : true;
}

static const struct YamlNode struct_flexSwitch[] = {
    YAML_IDX_CUST( "sw", flex_sw_read, flex_sw_write),
    YAML_CUSTOM( "channel", r_flex_sw_channel, w_flex_sw_channel),
    YAML_END
};

static uint32_t pot_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  auto idx = analogLookupPhysicalIdx(ADC_INPUT_FLEX, val, val_len);
  if (idx >= 0) return idx;

  {
    SourceRef ref = _legacy_mix_src(val, val_len);
    if (ref.type == SOURCE_TYPE_POT) return ref.index;
  }

  return -1;
}

static bool pot_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = analogGetPhysicalName(ADC_INPUT_FLEX, idx);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void pot_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                          const char* val, uint8_t val_len)
{
  _read_analog_name(ADC_INPUT_FLEX, user, data, bitoffs, val, val_len);
}

static bool pot_name_write(void* user, uint8_t* data, uint32_t bitoffs,
                           yaml_writer_func wf, void* opaque)
{
  return _write_analog_name(ADC_INPUT_FLEX, user, data, bitoffs, wf, opaque);
}

static const struct YamlIdStr enum_PotConfig[] = {
    {  FLEX_NONE, "none" },
    {  FLEX_POT, "without_detent" },
    {  FLEX_POT_CENTER, "with_detent" },
    {  FLEX_SLIDER, "slider" },
    {  FLEX_MULTIPOS, "multipos_switch" },
    {  FLEX_AXIS_X, "axis_x" },
    {  FLEX_AXIS_Y, "axis_y" },
    {  FLEX_SWITCH, "switch" },
    {  0, NULL }
};

static const struct YamlNode struct_potConfig[] = {
    YAML_IDX_CUST("pot", pot_read, pot_write ),
    YAML_ENUM("type", POT_CFG_TYPE_BITS, enum_PotConfig, NULL),
    YAML_UNSIGNED("inv", POT_CFG_INV_BITS),
    YAML_CUSTOM("name", pot_name_read, pot_name_write),
    YAML_END
};

const struct YamlIdStr enum_SwitchSources[] = {
  {  SWSRC_NONE, "NONE"  },
  {  SWSRC_ON, "ON"  },
  {  SWSRC_ONE, "ONE"  },
  {  SWSRC_TELEMETRY_STREAMING, "TELEMETRY_STREAMING"  },
  {  SWSRC_RADIO_ACTIVITY, "RADIO_ACTIVITY"  },
  {  SWSRC_TRAINER_CONNECTED, "TRAINER_CONNECTED"  },
  {  SWSRC_OFF, "OFF"  },
  {  0, NULL  }
};

// Trim switch names
static const char* trimSwitchNames[] = {
  "TrimRudLeft", "TrimRudRight",
  "TrimEleDown", "TrimEleUp",
  "TrimThrDown", "TrimThrUp",
  "TrimAilLeft", "TrimAilRight",
  "TrimT5Down", "TrimT5Up",
  "TrimT6Down", "TrimT6Up",
  "TrimT7Down", "TrimT7Up",
  "TrimT8Down", "TrimT8Up",
};

// r_swtchSrc / w_swtchSrc / w_swtchSrc_unquoted removed —
// replaced by yaml_parse_switch / yaml_write_switch
// which produce SwitchRef directly without bridge functions.

#if 0 // Dead code
static uint32_t r_swtchSrc(const YamlNode* node, const char* val, uint8_t val_len)
{
    int32_t ival=0;
    bool neg =  false;
    if (val_len > 0 && val[0] == '!') {
        neg = true;
        val++;
        val_len--;
    }

    if (val_len > 3 && ((val[0] == 'S' && val[1] >= 'W')
	    || (val[0] == 'F' && val[1] >= 'L'))
        && val[2] >= '0' && val[2] <= '9'
        && val[3] >= '0' && val[3] <= '2') {

      ival = switchLookupIdx(val, val_len - 1) * 3;
      if (ival < 0) return SWSRC_NONE;
      ival += yaml_str2int(val + 3, val_len - 3);
      ival += SWSRC_FIRST_SWITCH;
      
    } else if (val_len > 2 && val[0] == 'S'
        && val[1] >= 'A' && val[1] <= 'Z'
        && val[2] >= '0' && val[2] <= '2') {

      ival = switchLookupIdx(val, val_len - 1) * 3;
      if (ival < 0) return SWSRC_NONE;
      ival += yaml_str2int(val + 2, val_len - 2);
      ival += SWSRC_FIRST_SWITCH;
      
    }
    else if (val_len > 3
        && val[0] == '6'
        && val[1] == 'P'
        && (val[2] >= '0' && val[2] <= '9')
        && (val[3] >= '0' && val[3] < (XPOTS_MULTIPOS_COUNT + '0'))) {

      ival = (val[2] - '0') * XPOTS_MULTIPOS_COUNT + (val[3] - '0')
        + SWSRC_FIRST_MULTIPOS_SWITCH;
    }
    else if (val_len > 3
             && val[0] == 'T' && val[1] == 'R'
             && val[2] >= '1' && val[2] <= '9') {

      ival = SWSRC_FIRST_TRIM + (yaml_str2int(val + 2, val_len - 3) - 1) * 2;
      if (val[val_len - 1] == '+') ival++;
    }
    else if (val_len > 4 && (strncmp(val, trimSwitchNames[0], 4) == 0)) {

      for (size_t i = 0; i < sizeof(trimSwitchNames)/sizeof(const char*); i += 1) {
        if (strncmp(val, trimSwitchNames[i], val_len) == 0) {
          ival = SWSRC_FIRST_TRIM + i;
          break;
        }
      }
    }
    else if (val_len >= 2
             && val[0] == 'L'
             && (val[1] >= '0' && val[1] <= '9')) {

      ival = SWSRC_FIRST_LOGICAL_SWITCH + yaml_str2int(val+1, val_len-1) - 1;
    }
    else if (val_len == 3
             && val[0] == 'F'
             && val[1] == 'M'
             && (val[2] >= '0' && val[2] <= '9')) {
        
        ival = SWSRC_FIRST_FLIGHT_MODE + (val[2] - '0');
    }
    else if (val_len >= 2
             && val[0] == 'T'
             && (val[1] >= '0' && val[1] <= '9')) {

        ival = SWSRC_FIRST_SENSOR + yaml_str2int(val+1, val_len-1) - 1;
    }
    else {
        ival = yaml_parse_enum(enum_SwitchSources, val, val_len);
    }

    return neg ? -ival : ival;
}

static bool w_swtchSrc_unquoted(const YamlNode* node, uint32_t val,
                                yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val, node->size);
    if (sval < 0) {
        wf(opaque, "!", 1);
        sval = abs(sval);
    }

    const char* str = NULL;

    str = yaml_output_enum(sval, enum_SwitchSources);
    if (str) return wf(opaque, str, strlen(str));

    if (sval <= SWSRC_LAST_SWITCH) {

      auto sw_info = switchInfo(sval);
      str = switchGetDefaultName(sw_info.quot);
      if (!str) return true;
      wf(opaque, str, strlen(str));
      str = yaml_unsigned2str(sw_info.rem);
      return wf(opaque, str, strlen(str));

    } else if (sval <= SWSRC_LAST_MULTIPOS_SWITCH) {

      wf(opaque, "6P", 2);

      // pot #: start with 6P1
      sval -= SWSRC_FIRST_MULTIPOS_SWITCH;
      str = yaml_unsigned2str(sval / XPOTS_MULTIPOS_COUNT);
      wf(opaque,str, strlen(str));

      // position
      str = yaml_unsigned2str(sval % XPOTS_MULTIPOS_COUNT);
      return wf(opaque,str, strlen(str));

    } else if (sval <= SWSRC_LAST_TRIM) {

      auto trim = trimSwitchNames[sval - SWSRC_FIRST_TRIM];
      return wf(opaque, trim, strlen(trim));
        
    } else if (sval <= SWSRC_LAST_LOGICAL_SWITCH) {

      wf(opaque, "L", 1);
      str = yaml_unsigned2str(sval - SWSRC_FIRST_LOGICAL_SWITCH + 1);
      return wf(opaque,str, strlen(str));
    }
    else if (sval <= SWSRC_LAST_FLIGHT_MODE) {

      wf(opaque, "FM", 2);
      str = yaml_unsigned2str(sval - SWSRC_FIRST_FLIGHT_MODE);
      return wf(opaque,str, strlen(str));
    }
    else if (sval <= SWSRC_LAST_SENSOR) {

      wf(opaque, "T", 1);
      str = yaml_unsigned2str(sval - SWSRC_FIRST_SENSOR + 1);
      return wf(opaque,str, strlen(str));
    }

    return true; // ignore error
}

bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  if (!wf(opaque,"\"",1)
      || !w_swtchSrc_unquoted(node, val, wf, opaque)
      || !wf(opaque,"\"",1))
    return false;

  return true;
}
#endif // Dead code

bool cfn_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  return !((CustomFunctionData*)data)->swtch.isNone();
}

static bool curve_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  return isCurveUsed(tw->getElmts());
}

static bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  gvar_t* gvar = (gvar_t*)(data + (bitoffs>>3UL));
  return *gvar != 0 && *gvar != GVAR_MAX + 1;
}

static bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  // FM0 defaults to all 0
  if (idx == 0) {
    return !yaml_is_zero(data, bitoffs, sizeof(FlightModeData) << 3UL);
  }

  bool is_active = !yaml_is_zero(data, bitoffs, sizeof(FlightModeData) << 3UL);

  for (uint8_t i = 0; i < getGVarCount(); i++) {
    is_active |= GVAR_VALUE(i, idx) != GVAR_MAX + 1; // FM0 -> default
  }

  return is_active;
}

static bool swash_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto swashR = reinterpret_cast<SwashRingData*>(data + (bitoffs >> 3UL));
  return swashR->type | swashR->value;
}

static void r_swtchWarn(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  data += (bitoffs >> 3UL);

  // Read from string like 'AdBuC-':
  //
  // -> reads:
  //    - Switch A: must be DOWN
  //    - Switch B: must be UP
  //    - Switch C: must be MIDDLE
  //
  // -> switches not in the list shall not be checked
  //
  swarnstate_t swtchWarn = 0;

  while (val_len--) {
    signed swtch = switchLookupIdx(*(val++));
    if (swtch < 0) break;

    swarnstate_t state = 0;
    switch (*(val++)) {
      case 'u':
        state = 1;
        break;
      case '-':
        state = 2;
        break;
      case 'd':
        state = 3;
        break;
      default:
        // no check
        break;
    }

    // 3 bits per switch
    swtchWarn |= (state << (3 * swtch));
  }
  memcpy(data, &swtchWarn, sizeof(swtchWarn));

}

static const struct YamlIdStr enum_SwitchWarnPos[] = {
  {  0, "none"  },
  {  1, "up"  },
  {  2, "mid" },
  {  3, "down"  },
  {  0, nullptr },
};

static const struct YamlNode struct_swtchWarn[] {
  YAML_IDX_CUST( "sw", sw_idx_read, sw_idx_write ),
  YAML_ENUM( "pos", 2, enum_SwitchWarnPos, NULL ),
  YAML_END,
};

extern const struct YamlIdStr enum_BeeperMode[];

static uint32_t r_beeperMode(const YamlNode* node, const char* val, uint8_t val_len)
{
    return yaml_parse_enum(enum_BeeperMode, val, val_len);
}

static bool w_beeperMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  int32_t sval = yaml_to_signed(val,node->size);
  const char* str = yaml_output_enum(sval, enum_BeeperMode);
  return wf(opaque, str, strlen(str));
}

static uint32_t r_5pos(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) - 2);
}

static bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  int32_t sval = yaml_to_signed(val,node->size);
  char* s = yaml_signed2str(sval + 2);
  return wf(opaque, s, strlen(s));
}

static uint32_t r_vol(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) - VOLUME_LEVEL_DEF);
}

static bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  int32_t sval = yaml_to_signed(val,node->size);
  char* s = yaml_signed2str(sval + VOLUME_LEVEL_DEF);
  return wf(opaque, s, strlen(s));
}

static uint32_t r_spPitch(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) / 15);
}

static bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  char* s = yaml_signed2str(val * 15);
  return wf(opaque, s, strlen(s));
}

static uint32_t r_vPitch(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) / 10);
}

static bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  int32_t sval = yaml_to_signed(val,node->size);
  char* s = yaml_signed2str(sval * 10);
  return wf(opaque, s, strlen(s));
}

static const struct YamlIdStr enum_TrainerMode[] = {
  {  TRAINER_MODE_OFF, "OFF"  },
  {  TRAINER_MODE_MASTER_TRAINER_JACK, "MASTER_TRAINER_JACK"  },
  {  TRAINER_MODE_SLAVE, "SLAVE"  },
  {  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE, "MASTER_SBUS_EXT"  },
  {  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, "MASTER_CPPM_EXT"  },
  {  TRAINER_MODE_MASTER_SERIAL, "MASTER_SERIAL"  },
  {  TRAINER_MODE_MASTER_SERIAL, "MASTER_BATT_COMP"  },
  {  TRAINER_MODE_MASTER_BLUETOOTH, "MASTER_BT"  },
  {  TRAINER_MODE_SLAVE_BLUETOOTH, "SLAVE_BT"  },
  {  TRAINER_MODE_MULTI, "MASTER_MULTI"  },
  {  TRAINER_MODE_CRSF, "MASTER_CRSF"  },
  {  0, NULL  }
};

static uint32_t r_trainerMode(const YamlNode* node, const char* val, uint8_t val_len)
{
  return yaml_parse_enum(enum_TrainerMode, val, val_len);
}

static bool w_trainerMode(const YamlNode* node, uint32_t val,
                          yaml_writer_func wf, void* opaque)
{
  const char* str = nullptr;
  str = yaml_output_enum(val, enum_TrainerMode);

  if (str) {
    return wf(opaque, str, strlen(str));
  }

  return true;
}

#if !defined(COLORLCD)

const char* const _tele_screen_type_lookup[] = {
  "NONE",
  "VALUES",
  "BARS",
  "SCRIPT",
};

static void r_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                               const char* val, uint8_t val_len)
{
  uint8_t type = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (!strncmp(val, _tele_screen_type_lookup[i], val_len)) {
      type = i;
      break;
    }
  }

  if (!type) return;
  
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);
  
  data -= sizeof(TelemetryScreenData) * idx + 1;
  *data = (*data & ~(0x03 << (2 * idx))) | (type << (2 * idx));
}

static bool w_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                               yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  data -= sizeof(TelemetryScreenData) * idx + 1;
  uint8_t type = ((*data) >> (2 * idx)) & 0x03;

  const char* str = _tele_screen_type_lookup[type];
  return wf(opaque, str, strlen(str));
}

static uint8_t select_tele_screen_data(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(2);

  auto md = reinterpret_cast<ModelData*>(data);
  uint8_t st = (md->screensType >> (2 * idx)) & 0x03;
  switch(st){
  case TELEMETRY_SCREEN_TYPE_NONE:
      break;
  case TELEMETRY_SCREEN_TYPE_VALUES:
      return 1; // lines
  case TELEMETRY_SCREEN_TYPE_BARS:
      return 0; // bars
  case TELEMETRY_SCREEN_TYPE_SCRIPT:
      return 2;
  }

  TRACE("select_tele_screen_data(idx=%d)", idx);
  return 0;
}
#endif

static uint32_t r_tele_sensor(const YamlNode* node, const char* val, uint8_t val_len)
{
  if (val_len == 0 || val[0] < '0' || val[0] > '9') return 0;
  return yaml_str2uint(val, val_len) + 1;
}

static bool w_tele_sensor(const YamlNode* node, uint32_t val,
                          yaml_writer_func wf, void* opaque)
{
  if (!val) {
    return wf(opaque, "none", 4);
  }
  
  const char* str = yaml_unsigned2str(val-1);  
  return wf(opaque, str, strlen(str));
}

static uint32_t r_flightModes(const YamlNode* node, const char* val, uint8_t val_len)
{
  uint32_t bits = 0;
  uint32_t mask = 1;

  for (uint32_t i = 0; i < val_len; i++) {
    if (val[i] == '1') bits |= mask;
    mask <<= 1;
  }

  return bits;
}

static bool w_flightModes(const YamlNode* node, uint32_t val,
                          yaml_writer_func wf, void* opaque)
{
  for (uint32_t i = 0; i < node->size; i++) {
    uint32_t bit = (val >> i) & 1;
    if (!wf(opaque, bit ? "1" : "0", 1)) return false;
  }
  return true;
}

static const char* const _func_reset_param_lookup[] = {
  "Tmr1","Tmr2","Tmr3","All","Tele","Trims"
};

static const char* const _func_failsafe_lookup[] = {
  "Int","Ext"
};

static const char* const _func_sound_lookup[] = {
  "Bp1","Bp2","Bp3","Wrn1","Wrn2",
  "Chee","Rata","Tick","Sirn","Ring",
  "SciF","Robt","Chrp","Tada","Crck","Alrm"
};

static const char* const _adjust_gvar_mode_lookup[] = {
  "Cst", "Src", "SrcRaw", "GVar", "IncDec"
};

static void r_customFn(void* user, uint8_t* data, uint32_t bitoffs,
                       const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  data -= offsetof(CustomFunctionData, all);

  auto cfn = reinterpret_cast<CustomFunctionData*>(data);
  uint8_t func = CFN_FUNC(cfn);

  if (func == FUNC_OVERRIDE_CHANNEL) {
    // CH index
    CFN_CH_INDEX(cfn) = yaml_str2uint_ref(val, val_len);
    // ","
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    // value
    CFN_PARAM(cfn) = yaml_str2int_ref(val, val_len);
  }
  
  // find "," and cut val_len
  uint8_t l_sep = find_sep(val, val_len);

  bool eat_comma = true;
  // read values...
  switch (func) {

  case FUNC_OVERRIDE_CHANNEL:
    break;

  case FUNC_TRAINER:
    if (l_sep == 6
        && val[0] == 's'
        && val[1] == 't'
        && val[2] == 'i'
        && val[3] == 'c'
        && val[4] == 'k'
        && val[5] == 's') {
      CFN_CH_INDEX(cfn) = 0;
    } else if (l_sep == 5
               && val[0] == 'c'
               && val[1] == 'h'
               && val[2] == 'a'
               && val[3] == 'n'
               && val[4] == 's') {
      CFN_CH_INDEX(cfn) = MAX_STICKS + 1;
    } else {
      auto stick = analogLookupCanonicalIdx(ADC_INPUT_MAIN, val, l_sep);
      if (stick >= 0) {
        CFN_CH_INDEX(cfn) = stick + 1;
      }
    }
    break;

  case FUNC_RESET:
    if (l_sep == 4
        && val[0] == 'T'
        && val[1] == 'm'
        && val[2] == 'r'
        && val[3] >= '1'
        && val[3] <= '3') {
      CFN_PARAM(cfn) = val[3] - '1';
    } else if (l_sep == 3
               && val[0] == 'A'
               && val[1] == 'l'
               && val[2] == 'l') {
      CFN_PARAM(cfn) = FUNC_RESET_FLIGHT;
    } else if (l_sep == 4
               && val[0] == 'T'
               && val[1] == 'e'
               && val[2] == 'l'
               && val[3] == 'e') {
      CFN_PARAM(cfn) = FUNC_RESET_TELEMETRY;
    } else if (l_sep == 5
             && val[0] == 'T'
             && val[1] == 'r'
             && val[2] == 'i'
             && val[3] == 'm'
             && val[4] == 's') {
      CFN_PARAM(cfn) = FUNC_RESET_TRIMS;
    } else {
      uint32_t sensor = yaml_str2uint(val, l_sep);
      CFN_PARAM(cfn) = sensor + FUNC_RESET_PARAM_FIRST_TELEM;
    }
    break;
      
  case FUNC_VOLUME:
  case FUNC_BACKLIGHT:
  case FUNC_PLAY_VALUE:
    // find "," and cut val_len
    {
      bool invert = (val[0] == '!');
      const char* v = invert ? val+1 : val;
      uint8_t vl = invert ? l_sep-1 : l_sep;
      SourceRef ref = yaml_parse_source(v, vl);
      if (invert) ref.flags |= SOURCE_FLAG_INVERTED;
      cfn->all.val.source = ref;
    }
    break;

  case FUNC_PLAY_SOUND:
    // find "," and cut val_len
    for (unsigned i=0; i < DIM(_func_sound_lookup); i++) {
      if (!strncmp(_func_sound_lookup[i],val,l_sep)) {
        CFN_PARAM(cfn) = i;
        break;
      }
    }
    break;

  case FUNC_PLAY_TRACK:
  case FUNC_BACKGND_MUSIC:
  case FUNC_PLAY_SCRIPT:
  case FUNC_RGB_LED:
    strncpy(cfn->play.name, val, std::min<uint8_t>(l_sep, LEN_FUNCTION_NAME));
    break;

  case FUNC_SET_TIMER:
    // Tmr1,Tmr2,Tmr3
    if (l_sep >= 4
        && val[0] == 'T'
        && val[1] == 'm'
        && val[2] == 'r'
        && val[3] >= '1'
        && val[3] <= ('1' + MAX_TIMERS)) {

      CFN_TIMER_INDEX(cfn) = val[3] - '1';

      val += 4; val_len -= 4;
      if (val_len == 0 || val[0] != ',') return;
      val++; val_len--;

      CFN_PARAM(cfn) = yaml_str2uint_ref(val, val_len);
      l_sep = 0;
    } else {
      return;
    }
    break;

  case FUNC_SET_FAILSAFE:
    // Int,Ext
    if (l_sep == 3) {
      if (val[0] == 'I'
          && val[1] == 'n'
          && val[2] == 't') {
        CFN_PARAM(cfn) = 0;
      } else if (val[0] == 'E'
                 && val[1] == 'x'
                 && val[2] == 't') {
        CFN_PARAM(cfn) = 1;
      }
    }
    break;

  case FUNC_SET_SCREEN:
  case FUNC_HAPTIC:
  case FUNC_LOGS: // 10th of seconds
    CFN_PARAM(cfn) = yaml_str2uint(val, l_sep);
    break;
#if defined(FUNCTION_SWITCHES)
  case FUNC_PUSH_CUST_SWITCH:
    CFN_CS_INDEX(cfn) = yaml_str2uint_ref(val, val_len); // SW index

    // ","
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    l_sep = find_sep(val, val_len);

    CFN_PARAM(cfn) = yaml_str2int(val, l_sep); // Duration, 10th of seconds
    break;
#endif
  case FUNC_ADJUST_GVAR: {

    CFN_GVAR_INDEX(cfn) = yaml_str2int_ref(val, l_sep);
    if (val_len == 0 || val[0] != ',') return;
    val++; val_len--;

    // find "," and cut val_len
    l_sep = find_sep(val, val_len);

    // parse CFN_GVAR_MODE
    for (unsigned i=0; i < DIM(_adjust_gvar_mode_lookup); i++) {
      if (!strncmp(_adjust_gvar_mode_lookup[i],val,l_sep)) {
        CFN_GVAR_MODE(cfn) = i;
        break;
      }
    }

    val += l_sep; val_len -= l_sep;
    if (val_len == 0 || val[0] != ',') return;
    val++; val_len--;
    // find "," and cut val_len
    l_sep = find_sep(val, val_len);

    // output param
    switch(CFN_GVAR_MODE(cfn)) {
    case FUNC_ADJUST_GVAR_CONSTANT:
    case FUNC_ADJUST_GVAR_INCDEC:
      CFN_PARAM(cfn) = yaml_str2int(val, l_sep);
      break;
    case FUNC_ADJUST_GVAR_SOURCE:
    case FUNC_ADJUST_GVAR_SOURCERAW:
    {
      bool invert = (val[0] == '!');
      if (invert) { val++; l_sep--; }
      SourceRef ref = yaml_parse_source(val, l_sep);
      if (invert) ref.flags |= SOURCE_FLAG_INVERTED;
      cfn->all.val.source = ref;
    } break;
    case FUNC_ADJUST_GVAR_GVAR: {
      SourceRef ref = yaml_parse_source(val, l_sep);
      if (ref.type == SOURCE_TYPE_GVAR) {
        CFN_PARAM(cfn) = ref.index;
      }
    } break;
    }

  } break;

  default:
    eat_comma = false;
    break;
  }

  // Set 'enabled' to handle old format YAML files
  // Will be updated if enabled flag is actually present
  CFN_ACTIVE(cfn) = 1;

  if (eat_comma) {
    val += l_sep;
    val_len -= l_sep;

    if (val_len == 0 || val[0] != ',')
      return;

    val++; val_len--;
  }

  // Handle old YAML files where only one of active/repeat was present
  bool read_enable_flag = true;
  if (HAS_REPEAT_PARAM(func)) {
    // Check for 2 values to be parsed
    uint8_t l_sep = find_sep(val, val_len);
    if (l_sep == val_len) {
      // only one more value - assume it is repeat
      read_enable_flag = false;
    }
  }

  // Enable param
  // "0/1"
  if (val_len > 0 && read_enable_flag) {
    CFN_ACTIVE(cfn) = (val[0] == '1') ? 1 : 0;
    uint8_t l_sep = find_sep(val, val_len);

    // Skip comma before optional repeat
    val += l_sep;
    val_len -= l_sep;
    if (val_len == 0 || val[0] != ',')
      return;
    val++; val_len--;
  }

  if (HAS_REPEAT_PARAM(func)) {
    if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
      if (val_len == 2 && val[0] == '1' && val[1] == 'x')
        CFN_PLAY_REPEAT(cfn) = 1;
      else
        CFN_PLAY_REPEAT(cfn) = 0;
    } else if (val_len == 2 && val[0] == '1' && val[1] == 'x') {
      CFN_PLAY_REPEAT(cfn) = 0;
    } else if (val_len == 3 && val[0] == '!' && val[1] == '1' && val[2] == 'x') {
      CFN_PLAY_REPEAT(cfn) = CFN_PLAY_REPEAT_NOSTART;
    } else {
      // repeat time in seconds
      CFN_PLAY_REPEAT(cfn) = yaml_str2uint(val,val_len) / CFN_PLAY_REPEAT_MUL;
    }
  }
}

static bool w_customFn(void* user, uint8_t* data, uint32_t bitoffs,
                       yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  data -= offsetof(CustomFunctionData, all);

  auto cfn = reinterpret_cast<CustomFunctionData*>(data);
  uint8_t func = CFN_FUNC(cfn);

  const char* str = nullptr;
  bool add_comma = true;
  if (!wf(opaque, "\"", 1)) return false;

  switch (func) {
  case FUNC_OVERRIDE_CHANNEL:
    str = yaml_unsigned2str(CFN_CH_INDEX(cfn)); // CH index
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque, ",", 1)) return false;
    str = yaml_signed2str(CFN_PARAM(cfn));    // value
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_TRAINER: {
    int16_t value = CFN_CH_INDEX(cfn);
    switch(value) {
    case 0:
      if (!wf(opaque, "sticks", 6)) return false;
      break;
    case MAX_STICKS + 1:
      if (!wf(opaque, "chans", 5)) return false;
      break;
    default:
      if (value > 0 && value <= MAX_STICKS) {
        str = analogGetCanonicalName(ADC_INPUT_MAIN, value - 1);
        if (str && !wf(opaque, str, strlen(str))) return false;
      }
    }
  } break;

  case FUNC_RESET:
    if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
      // Tmr1,Tmr2,Tmr3,All,Tele, Trims
      str = _func_reset_param_lookup[CFN_PARAM(cfn)];
    } else {
      // sensor index
      str = yaml_unsigned2str(CFN_PARAM(cfn) - FUNC_RESET_PARAM_FIRST_TELEM);
    }
    if (!wf(opaque, str, strlen(str))) return false;
    break;
      
  case FUNC_VOLUME:
  case FUNC_BACKLIGHT:
  case FUNC_PLAY_VALUE:
  {
    SourceRef ref = cfn->all.val.source;
    if (ref.isInverted()) {
      if (!wf(opaque, "!", 1)) return false;
      ref.flags &= ~SOURCE_FLAG_INVERTED;
    }
    if (!yaml_write_source(ref, wf, opaque)) return false;
  } break;

  case FUNC_PLAY_SOUND:
    // Bp1,Bp2,Bp3,Wrn1,Wrn2,Chee,Rata,Tick,Sirn,Ring,SciF,Robt,Chrp,Tada,Crck,Alrm
    str = _func_sound_lookup[CFN_PARAM(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_PLAY_TRACK:
  case FUNC_BACKGND_MUSIC:
  case FUNC_PLAY_SCRIPT:
  case FUNC_RGB_LED:
    if (!wf(opaque, cfn->play.name, strnlen(cfn->play.name, LEN_FUNCTION_NAME)))
      return false;
    break;

  case FUNC_SET_TIMER:
    // Tmr1,Tmr2,Tmr3
    str = _func_reset_param_lookup[CFN_TIMER_INDEX(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(CFN_PARAM(cfn));
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_SET_FAILSAFE:
    // Int,Ext
    str = _func_failsafe_lookup[CFN_PARAM(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    break;

#if defined(FUNCTION_SWITCHES)
  case FUNC_PUSH_CUST_SWITCH:
    str = yaml_unsigned2str(CFN_CS_INDEX(cfn)); // SW index
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_signed2str(CFN_PARAM(cfn)); // Duration
    if (!wf(opaque, str, strlen(str))) return false;
    break;
#endif

  case FUNC_SET_SCREEN:
  case FUNC_HAPTIC:
  case FUNC_LOGS: // 10th of seconds
    str = yaml_unsigned2str(CFN_PARAM(cfn));
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_ADJUST_GVAR:
    str = yaml_unsigned2str(CFN_GVAR_INDEX(cfn)); // GVAR index
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;

    // output CFN_GVAR_MODE
    str = _adjust_gvar_mode_lookup[CFN_GVAR_MODE(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;    

    // output param
    switch(CFN_GVAR_MODE(cfn)) {
    case FUNC_ADJUST_GVAR_CONSTANT:
    case FUNC_ADJUST_GVAR_INCDEC:
      str = yaml_signed2str(CFN_PARAM(cfn));
      if (!wf(opaque, str, strlen(str))) return false;
      break;
    case FUNC_ADJUST_GVAR_SOURCE:
    case FUNC_ADJUST_GVAR_SOURCERAW:
    {
      SourceRef ref = cfn->all.val.source;
      if (ref.isInverted()) {
        if (!wf(opaque, "!", 1)) return false;
        ref.flags &= ~SOURCE_FLAG_INVERTED;
      }
      if (!yaml_write_source(ref, wf, opaque)) return false;
    } break;
    case FUNC_ADJUST_GVAR_GVAR:
    {
      SourceRef ref = SourceRef_(SOURCE_TYPE_GVAR, (uint16_t)CFN_PARAM(cfn));
      if (!yaml_write_source(ref, wf, opaque)) return false;
    } break;
    }
    break;

  default:
    add_comma = false;
    break;
  }

  if (add_comma) {
    // ","
    if (!wf(opaque,",",1)) return false;
  }

  // Enable param
  // "0/1"
  if (!wf(opaque,CFN_ACTIVE(cfn) ? "1":"0",1)) return false;

  if (HAS_REPEAT_PARAM(func)) {
    // ","
    if (!wf(opaque,",",1)) return false;

    if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
      if (!wf(opaque,(CFN_PLAY_REPEAT(cfn) == 0) ? "On" : "1x",2)) return false;
    } else if (CFN_PLAY_REPEAT(cfn) == 0) {
      // "1x"
      if (!wf(opaque,"1x",2)) return false;
    } else if (CFN_PLAY_REPEAT(cfn) == (int8_t)CFN_PLAY_REPEAT_NOSTART) {
      // "!1x"
      if (!wf(opaque,"!1x",3)) return false;
    } else {
      // repeat time in seconds
      str = yaml_unsigned2str(CFN_PLAY_REPEAT(cfn) * CFN_PLAY_REPEAT_MUL);
      if (!wf(opaque, str, strlen(str))) return false;
    }
  }

  if (!wf(opaque, "\"", 1)) return false;
  return true;
}

#include "switches.h"

// Forward declarations (defined later, needed by r_logicSw / w_logicSw)
static SwitchRef yaml_parse_switch(const char* val, uint8_t val_len);
static bool yaml_write_switch(const SwitchRef& ref, yaml_writer_func wf, void* opaque);

static delayval_t timerValue2lsw(uint32_t t)
{
  if (t < 20) {
    return t - 129;
  } else if (t < 600) {
    return t / 5 - 113;
  } else {
    return t / 10 - 53;
  }
}

static void r_logicSw(void* user, uint8_t* data, uint32_t bitoffs,
                      const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  data -= sizeof(LogicalSwitchData::func);

  // find "," and cut val_len
  uint8_t l_sep = find_sep(val, val_len);

  auto ls = reinterpret_cast<LogicalSwitchData*>(data);
  switch(lswFamily(ls->func)) {
  
  case LS_FAMILY_BOOL:
  case LS_FAMILY_STICKY:
    ls->v1.swtch = yaml_parse_switch(val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2.swtch = yaml_parse_switch(val, val_len);
    break;

  case LS_FAMILY_EDGE:
    ls->v1.swtch = yaml_parse_switch(val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2.value = timerValue2lsw(yaml_str2uint_ref(val, val_len));
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    if (val_len == 1 && val[0] == '<') {
      ls->v3 = -1;
    } else if (val_len == 1 && val[0] == '-') {
        ls->v3 = 0;
    } else {
      int16_t t = (int16_t)timerValue2lsw(yaml_str2uint_ref(val, val_len));
      ls->v3 = t - ls->v2.value;
    }
    break;

  case LS_FAMILY_COMP:
  {
    bool inv1 = (val[0] == '!');
    SourceRef r1 = yaml_parse_source(inv1 ? val+1 : val, inv1 ? l_sep-1 : l_sep);
    if (inv1) r1.flags |= SOURCE_FLAG_INVERTED;
    ls->v1.source = r1;
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    bool inv2 = (val[0] == '!');
    SourceRef r2 = yaml_parse_source(inv2 ? val+1 : val, inv2 ? val_len-1 : val_len);
    if (inv2) r2.flags |= SOURCE_FLAG_INVERTED;
    ls->v2.source = r2;
  } break;

  case LS_FAMILY_TIMER:
    ls->v1.value = timerValue2lsw(yaml_str2uint(val, l_sep));
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2.value = timerValue2lsw(yaml_str2uint(val, val_len));
    break;

  default:
  {
    bool inv = (val[0] == '!');
    SourceRef r = yaml_parse_source(inv ? val+1 : val, inv ? l_sep-1 : l_sep);
    if (inv) r.flags |= SOURCE_FLAG_INVERTED;
    ls->v1.source = r;
  }
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    // TODO?: ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2
    ls->v2.value = yaml_str2int_ref(val, val_len);
    break;
  }

}

static const struct YamlNode _ls_node_v1 = YAML_PADDING(10);
static const struct YamlNode _ls_node_v2 = YAML_PADDING(16);

static bool w_logicSw(void* user, uint8_t* data, uint32_t bitoffs,
                      yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  data -= sizeof(LogicalSwitchData::func);
  if (!wf(opaque,"\"",1)) return false;

  const char* str = nullptr;
  auto ls = reinterpret_cast<LogicalSwitchData*>(data);
  switch(lswFamily(ls->func)) {
  
  case LS_FAMILY_BOOL:
  case LS_FAMILY_STICKY:
  {
    SwitchRef s1 = ls->v1.swtch;
    if (s1.isInverted()) { wf(opaque, "!", 1); s1.flags &= ~SWITCH_FLAG_INVERTED; }
    if (!yaml_write_switch(s1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    SwitchRef s2 = ls->v2.swtch;
    if (s2.isInverted()) { wf(opaque, "!", 1); s2.flags &= ~SWITCH_FLAG_INVERTED; }
    if (!yaml_write_switch(s2, wf, opaque)) return false;
  } break;

  case LS_FAMILY_EDGE:
  {
    SwitchRef s1 = ls->v1.swtch;
    if (s1.isInverted()) { wf(opaque, "!", 1); s1.flags &= ~SWITCH_FLAG_INVERTED; }
    if (!yaml_write_switch(s1, wf, opaque)) return false;
  }
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(lswTimerValue(ls->v2.value));
    if (!wf(opaque,str,strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    if (ls->v3 < 0) {
      if (!wf(opaque,"<",1)) return false;
    } else if(ls->v3 == 0) {
      if (!wf(opaque,"-",1)) return false;
    } else {
      str = yaml_unsigned2str(lswTimerValue(ls->v2.value + ls->v3));
      if (!wf(opaque, str, strlen(str))) return false;
    }
    break;

  case LS_FAMILY_COMP:
  {
    SourceRef r1 = ls->v1.source;
    if (r1.isInverted()) { wf(opaque, "!", 1); r1.flags &= ~SOURCE_FLAG_INVERTED; }
    if (!yaml_write_source(r1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    SourceRef r2 = ls->v2.source;
    if (r2.isInverted()) { wf(opaque, "!", 1); r2.flags &= ~SOURCE_FLAG_INVERTED; }
    if (!yaml_write_source(r2, wf, opaque)) return false;
  } break;

  case LS_FAMILY_TIMER:
    str = yaml_unsigned2str(lswTimerValue(ls->v1.value));
    if (!wf(opaque,str,strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(lswTimerValue(ls->v2.value));
    if (!wf(opaque,str,strlen(str))) return false;
    break;

  default:
  {
    SourceRef r1 = ls->v1.source;
    if (r1.isInverted()) { wf(opaque, "!", 1); r1.flags &= ~SOURCE_FLAG_INVERTED; }
    if (!yaml_write_source(r1, wf, opaque)) return false;
  }
    if (!wf(opaque,",",1)) return false;
    // TODO?: ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2
    str = yaml_signed2str(ls->v2.value);
    if (!wf(opaque,str,strlen(str))) return false;
    break;
  }

  if (!wf(opaque,"\"",1)) return false;
  return true;
}

// Generic SourceRef YAML read/write (packs/unpacks SourceRef as uint32_t)
static uint32_t r_sourceRef(const YamlNode* node, const char* val, uint8_t val_len)
{
  SourceRef ref = yaml_parse_source(val, val_len);
  uint32_t packed;
  memcpy(&packed, &ref, sizeof(packed));
  return packed;
}

static bool w_sourceRef(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                        void* opaque)
{
  SourceRef ref;
  memcpy(&ref, &val, sizeof(ref));
  return yaml_write_source(ref, wf, opaque);
}

// Inverted SourceRef (for MixData/ExpoData srcRaw which supports '!' prefix)
static uint32_t r_sourceRefEx(const YamlNode* node, const char* val, uint8_t val_len)
{
  bool invert = false;
  if (val_len > 0 && val[0] == '!') {
    invert = true;
    val += 1;
    val_len -= 1;
  }
  SourceRef ref = yaml_parse_source(val, val_len);
  if (invert) ref.flags |= SOURCE_FLAG_INVERTED;
  uint32_t packed;
  memcpy(&packed, &ref, sizeof(packed));
  return packed;
}

static bool w_sourceRefEx(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                          void* opaque)
{
  SourceRef ref;
  memcpy(&ref, &val, sizeof(ref));
  if (!wf(opaque, "\"", 1)) return false;
  if (ref.isInverted()) {
    if (!wf(opaque, "!", 1)) return false;
    ref.flags &= ~SOURCE_FLAG_INVERTED;
  }
  if (!yaml_write_source(ref, wf, opaque)) return false;
  return wf(opaque, "\"", 1);
}

// --- SwitchRef YAML parsing (text → SwitchRef) ---

// Backward-compat enum table for singleton switches
static const struct {
  const char* name;
  uint8_t type;
} switchEnumTable[] = {
  { "NONE",                 SWITCH_TYPE_NONE },
  { "ON",                   SWITCH_TYPE_ON },
  { "ONE",                  SWITCH_TYPE_ONE },
  { "TELEMETRY_STREAMING",  SWITCH_TYPE_TELEMETRY },
  { "RADIO_ACTIVITY",       SWITCH_TYPE_RADIO_ACTIVITY },
  { "TRAINER_CONNECTED",    SWITCH_TYPE_TRAINER },
  { "OFF",                  SWITCH_TYPE_NONE },  // OFF maps to NONE
};

static SwitchRef yaml_parse_switch(const char* val, uint8_t val_len)
{
    bool neg = false;
    if (val_len > 0 && val[0] == '!') {
      neg = true;
      val++;
      val_len--;
    }

    SwitchRef ref = {};

    // Physical switches: "SWA0", "SA0", "FL10", etc.
    if (val_len > 3 && ((val[0] == 'S' && val[1] >= 'W')
	    || (val[0] == 'F' && val[1] >= 'L'))
        && val[2] >= '0' && val[2] <= '9'
        && val[3] >= '0' && val[3] <= '2') {

      auto idx = switchLookupIdx(val, val_len - 1);
      if (idx < 0) return {};
      ref = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(idx * 3 + yaml_str2int(val + 3, val_len - 3)));
    }
    else if (val_len > 2 && val[0] == 'S'
        && val[1] >= 'A' && val[1] <= 'Z'
        && val[2] >= '0' && val[2] <= '2') {

      auto idx = switchLookupIdx(val, val_len - 1);
      if (idx < 0) return {};
      ref = SwitchRef_(SWITCH_TYPE_SWITCH, (uint16_t)(idx * 3 + yaml_str2int(val + 2, val_len - 2)));
    }
    // Multipos: "6P00"
    else if (val_len > 3 && val[0] == '6' && val[1] == 'P'
             && (val[2] >= '0' && val[2] <= '9')
             && (val[3] >= '0' && val[3] < (XPOTS_MULTIPOS_COUNT + '0'))) {
      ref = SwitchRef_(SWITCH_TYPE_MULTIPOS, (uint16_t)((val[2] - '0') * XPOTS_MULTIPOS_COUNT + (val[3] - '0')));
    }
    // Trim switches: "TR1-", "TR1+", or legacy names like "TrimRudLeft"
    else if (val_len > 3 && val[0] == 'T' && val[1] == 'R'
             && val[2] >= '1' && val[2] <= '9') {
      uint16_t idx = (yaml_str2int(val + 2, val_len - 3) - 1) * 2;
      if (val[val_len - 1] == '+') idx++;
      ref = SwitchRef_(SWITCH_TYPE_TRIM, idx);
    }
    else if (val_len > 4 && (strncmp(val, trimSwitchNames[0], 4) == 0)) {
      for (size_t i = 0; i < sizeof(trimSwitchNames)/sizeof(const char*); i += 1) {
        if (strncmp(val, trimSwitchNames[i], val_len) == 0) {
          ref = SwitchRef_(SWITCH_TYPE_TRIM, (uint16_t)i);
          break;
        }
      }
    }
    // Logical switches: "L1", "L12"
    else if (val_len >= 2 && val[0] == 'L' && (val[1] >= '0' && val[1] <= '9')) {
      ref = SwitchRef_(SWITCH_TYPE_LOGICAL, (uint16_t)(yaml_str2int(val+1, val_len-1) - 1));
    }
    // Flight modes: "FM0"
    else if (val_len == 3 && val[0] == 'F' && val[1] == 'M' && (val[2] >= '0' && val[2] <= '9')) {
      ref = SwitchRef_(SWITCH_TYPE_FLIGHT_MODE, (uint16_t)(val[2] - '0'));
    }
    // Telemetry sensors: "T1", "T12"
    else if (val_len >= 2 && val[0] == 'T' && (val[1] >= '0' && val[1] <= '9')) {
      ref = SwitchRef_(SWITCH_TYPE_SENSOR, (uint16_t)(yaml_str2int(val+1, val_len-1) - 1));
    }
    else {
      // Singleton enum names (ON, ONE, TELEMETRY_STREAMING, etc.)
      for (const auto& e : switchEnumTable) {
        if (val_len == strlen(e.name) && strncmp(val, e.name, val_len) == 0) {
          ref = {e.type, 0, 0};
          break;
        }
      }
    }

    if (neg) ref.flags |= SWITCH_FLAG_INVERTED;
    return ref;
}

// --- SwitchRef YAML writing (SwitchRef → text) ---

static bool yaml_write_switch(const SwitchRef& ref, yaml_writer_func wf, void* opaque)
{
    const char* str = nullptr;

    switch (ref.type) {
      case SWITCH_TYPE_NONE:
        return wf(opaque, "NONE", 4);
      case SWITCH_TYPE_SWITCH:
      {
        uint16_t sw = ref.index / 3;
        uint8_t pos = ref.index % 3;
        str = switchGetDefaultName(sw);
        if (!str) return true;
        if (!wf(opaque, str, strlen(str))) return false;
        str = yaml_unsigned2str(pos);
        break;
      }
      case SWITCH_TYPE_MULTIPOS:
      {
        if (!wf(opaque, "6P", 2)) return false;
        str = yaml_unsigned2str(ref.index / XPOTS_MULTIPOS_COUNT);
        if (!wf(opaque, str, strlen(str))) return false;
        str = yaml_unsigned2str(ref.index % XPOTS_MULTIPOS_COUNT);
        break;
      }
      case SWITCH_TYPE_TRIM:
        if (ref.index < sizeof(trimSwitchNames)/sizeof(const char*)) {
          str = trimSwitchNames[ref.index];
        }
        break;
      case SWITCH_TYPE_LOGICAL:
        if (!wf(opaque, "L", 1)) return false;
        str = yaml_unsigned2str(ref.index + 1);
        break;
      case SWITCH_TYPE_ON:
        return wf(opaque, "ON", 2);
      case SWITCH_TYPE_ONE:
        return wf(opaque, "ONE", 3);
      case SWITCH_TYPE_FLIGHT_MODE:
        if (!wf(opaque, "FM", 2)) return false;
        str = yaml_unsigned2str(ref.index);
        break;
      case SWITCH_TYPE_TELEMETRY:
        return wf(opaque, "TELEMETRY_STREAMING", 19);
      case SWITCH_TYPE_SENSOR:
        if (!wf(opaque, "T", 1)) return false;
        str = yaml_unsigned2str(ref.index + 1);
        break;
      case SWITCH_TYPE_RADIO_ACTIVITY:
        return wf(opaque, "RADIO_ACTIVITY", 14);
      case SWITCH_TYPE_TRAINER:
        return wf(opaque, "TRAINER_CONNECTED", 17);
      default:
        return wf(opaque, "NONE", 4);
    }

    if (str) return wf(opaque, str, strlen(str));
    return true;
}

// SwitchRef YAML custom handlers (packed uint32_t ↔ text)
static uint32_t r_switchRef(const YamlNode* node, const char* val, uint8_t val_len)
{
  SwitchRef ref = yaml_parse_switch(val, val_len);
  uint32_t packed;
  memcpy(&packed, &ref, sizeof(packed));
  return packed;
}

static bool w_switchRef(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                        void* opaque)
{
  SwitchRef ref;
  memcpy(&ref, &val, sizeof(ref));
  if (!wf(opaque, "\"", 1)) return false;
  if (ref.isInverted()) {
    if (!wf(opaque, "!", 1)) return false;
    ref.flags &= ~SWITCH_FLAG_INVERTED;
  }
  if (!yaml_write_switch(ref, wf, opaque)) return false;
  return wf(opaque, "\"", 1);
}

// ValueOrSource YAML read/write
static uint32_t r_valOrSrc(const YamlNode* node, const char* val, uint8_t val_len)
{
  ValueOrSource vos = {};

  if (((val[0] == '-') && (val[1] >= '0' && val[1] <= '9')) ||
      (val[0] >= '0' && val[0] <= '9')) {
    vos.value = (int16_t)yaml_str2int(val, val_len);
  } else if ((val[0] == '-') && (val[1] == 'G')) {
    // Legacy negative GVar: -GV1 etc. (1-based → 0-based index)
    uint16_t idx = (uint16_t)(val[3] - '0' - 1);
    vos.setSource(SourceRef_(SOURCE_TYPE_GVAR, idx, SOURCE_FLAG_INVERTED));
  } else if (val[0] == 'G') {
    // Legacy positive GVar: GV1 etc. (1-based → 0-based index)
    vos.isSource = 1;
    vos.srcType = SOURCE_TYPE_GVAR;
    vos.value = (val[2] - '0') - 1;
  } else {
    // Parse as source with possible '!' prefix
    bool invert = false;
    if (val[0] == '!') { invert = true; val++; val_len--; }
    SourceRef ref = yaml_parse_source(val, val_len);
    if (invert) ref.flags |= SOURCE_FLAG_INVERTED;
    vos.setSource(ref);
  }

  uint32_t packed;
  memcpy(&packed, &vos, sizeof(packed));
  return packed;
}

static bool w_valOrSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                       void* opaque)
{
  ValueOrSource vos;
  memcpy(&vos, &val, sizeof(vos));

  if (vos.isSource) {
    SourceRef ref = vos.toSourceRef();
    if (!wf(opaque, "\"", 1)) return false;
    if (ref.isInverted()) {
      if (!wf(opaque, "!", 1)) return false;
      ref.flags &= ~SOURCE_FLAG_INVERTED;
    }
    if (!yaml_write_source(ref, wf, opaque)) return false;
    return wf(opaque, "\"", 1);
  }

  char* s = yaml_signed2str(vos.value);
  return wf(opaque, s, strlen(s));
}

// Aliases for backward compat
static constexpr auto r_thrSrc = r_sourceRef;
static constexpr auto w_thrSrc = w_sourceRef;

extern const struct YamlIdStr enum_ModuleType[];

static const struct YamlIdStr enum_old_ModuleType[] = {
  { MODULE_TYPE_FLYSKY_AFHDS2A, "TYPE_FLYSKY" },
  { 0, NULL  }
};

static uint32_t r_moduleType(const YamlNode* node, const char* val, uint8_t val_len)
{
  uint32_t type = yaml_parse_enum(enum_ModuleType, val, val_len);
  if (!type && val_len > 0) {
    type = yaml_parse_enum(enum_old_ModuleType, val, val_len);
  }
  return type;
}

bool w_moduleType(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  const char* str = yaml_output_enum(val, enum_ModuleType);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static const struct YamlIdStr enum_XJT_Subtypes[] = {
  { MODULE_SUBTYPE_PXX1_ACCST_D16, "D16" },
  { MODULE_SUBTYPE_PXX1_ACCST_D8, "D8" },
  { MODULE_SUBTYPE_PXX1_ACCST_LR12, "LR12" },
  { 0, NULL  }
};

static const struct YamlIdStr enum_ISRM_Subtypes[] = {
  { MODULE_SUBTYPE_ISRM_PXX2_ACCESS, "ACCESS" },
  { MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16, "D16" },
  // unused !!!
  // { MODULE_SUBTYPE_ISRM_PXX2_ACCST_LR12, "LR12" },
  // { MODULE_SUBTYPE_ISRM_PXX2_ACCST_D8, "D8" },
  { 0, NULL  }
};

static const struct YamlIdStr enum_R9M_Subtypes[] = {
  { MODULE_SUBTYPE_R9M_FCC, "FCC" },
  { MODULE_SUBTYPE_R9M_EU, "EU" },
  { MODULE_SUBTYPE_R9M_EUPLUS, "EUPLUS" },
  { MODULE_SUBTYPE_R9M_AUPLUS, "AUPLUS" },
  { 0, NULL  }
};

enum ModuleSubtypeFlysky { FLYSKY_SUBTYPE_AFHDS3 = 0, FLYSKY_SUBTYPE_AFHDS2A };

static const struct YamlIdStr enum_FLYSKY_Subtypes[] = {
  { FLYSKY_SUBTYPE_AFHDS2A, "AFHDS2A" },
  { FLYSKY_SUBTYPE_AFHDS3, "AFHDS3" },
  { 0, NULL  }
};

static const struct YamlIdStr enum_PPM_Subtypes[] = {
  { 0, "NOTLM" },
  { 1, "MLINK" },
  { 0, NULL  }
};

static const struct YamlIdStr enum_DSM2_Subtypes[] = {
  { 0, "LP45" },
  { 1, "DSM2" },
  { 2, "DSMX" },
  { 0, NULL  }
};

static void r_modSubtype(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  data -= offsetof(ModuleData, channelsStart);

  auto md = reinterpret_cast<ModuleData*>(data);
  if (isModuleTypeXJT(md->type)) {
    md->subType = yaml_parse_enum(enum_XJT_Subtypes, val, val_len);
  } else if (isModuleTypeISRM(md->type)) {
    md->subType = yaml_parse_enum(enum_ISRM_Subtypes, val, val_len);
  } else if (isModuleTypeR9MNonAccess(md->type)) {
    md->subType = yaml_parse_enum(enum_R9M_Subtypes, val, val_len);
#if defined(AFHDS3)
  } else if (md->type == MODULE_TYPE_FLYSKY_AFHDS2A) {
    // Flysky sub-types have been converted into separate module types
    auto sub_type = yaml_parse_enum(enum_FLYSKY_Subtypes, val, val_len);
    if (sub_type == FLYSKY_SUBTYPE_AFHDS3)
      md->type = MODULE_TYPE_FLYSKY_AFHDS3;
#endif
  } else if (md->type == MODULE_TYPE_MULTIMODULE) {
#if defined(MULTIMODULE)
    // Read type/subType by the book (see MPM documentation)
    // read "[type],[subtype]"
    uint8_t l_sep = find_sep(val, val_len);

    int type = yaml_str2uint(val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    int subtype = yaml_str2uint(val, val_len);

    // convert to ETX format and write to vars
    if (type > 0) {
      type -= 1; // change to internal 0 based representation
      md->multi.rfProtocol = type;
      md->subType = subtype;
    }
#endif
  } else if (md->type == MODULE_TYPE_DSM2) {
    md->subType = yaml_parse_enum(enum_DSM2_Subtypes, val, val_len);
  } else if (md->type == MODULE_TYPE_PPM) {
    md->subType = yaml_parse_enum(enum_PPM_Subtypes, val, val_len);
  } else {
    md->subType = yaml_str2uint(val, val_len);
  }  
}

static bool w_modSubtype(void* user, uint8_t* data, uint32_t bitoffs,
                         yaml_writer_func wf, void* opaque)
{
  // rfProtocol + subType, depending on the module
  data += bitoffs >> 3UL;
  data -= offsetof(ModuleData, channelsStart);

  const char* str = nullptr;
  auto md = reinterpret_cast<ModuleData*>(data);
  int32_t val = md->subType;
  if (md->type == MODULE_TYPE_XJT_PXX1 || md->type == MODULE_TYPE_XJT_LITE_PXX2) {
    str = yaml_output_enum(val, enum_XJT_Subtypes);
  } else if (md->type == MODULE_TYPE_ISRM_PXX2) {
    str = yaml_output_enum(val, enum_ISRM_Subtypes);
  } else if (md->type == MODULE_TYPE_R9M_PXX1 || md->type == MODULE_TYPE_R9M_LITE_PXX1) {
    str = yaml_output_enum(val, enum_R9M_Subtypes);
  } else if (md->type == MODULE_TYPE_MULTIMODULE) {
#if defined(MULTIMODULE)
    // Use type/subType by the book (see MPM documentation)
    int type = md->multi.rfProtocol + 1;
    int subtype = val;

    // output "[type],[subtype]"
    str = yaml_unsigned2str(type);
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque, ",", 1)) return false;
    str = yaml_unsigned2str(subtype);
#endif
  } else if (md->type == MODULE_TYPE_DSM2) {
    str = yaml_output_enum(md->subType, enum_DSM2_Subtypes);
  } else if (md->type == MODULE_TYPE_PPM) {
    str = yaml_output_enum(md->subType, enum_PPM_Subtypes);
  } else if (md->type == MODULE_TYPE_FLYSKY_AFHDS2A) {
    str = yaml_output_enum(FLYSKY_SUBTYPE_AFHDS2A, enum_FLYSKY_Subtypes);
  } else if (md->type == MODULE_TYPE_FLYSKY_AFHDS3) {
    str = yaml_output_enum(FLYSKY_SUBTYPE_AFHDS3, enum_FLYSKY_Subtypes);
  } else {
    str = yaml_unsigned2str(val);
  }

  if (str && !wf(opaque, str, strlen(str)))
    return false;

  return true;
}

static uint32_t r_channelsCount(const YamlNode* node, const char* val, uint8_t val_len)
{
  return yaml_str2int(val,val_len) - 8;
}

bool w_channelsCount(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  // offset 8
  int32_t sval = yaml_to_signed(val, node->size) + 8;  
  const char* str = yaml_signed2str(sval);
  return wf(opaque,str,strlen(str));
}

static void r_jitterFilter(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  uint32_t i = yaml_str2uint(val, val_len);
  yaml_put_bits(data, i, bitoffs, 1);
}

static void r_carryTrim(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  int32_t i = yaml_str2int(val, val_len);
  yaml_put_bits(data, i, bitoffs, 6);
}

static void r_rotEncDirection(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  uint32_t i = yaml_str2uint(val, val_len);
  yaml_put_bits(data, i, bitoffs, 2);
}

static void r_telemetryBaudrate(void* user, uint8_t* data, uint32_t bitoffs,
                                const char* val, uint8_t val_len)
{
  uint32_t i = yaml_str2uint(val, val_len);
  yaml_put_bits(data, i, bitoffs, 3);
}

//struct_serialConfig
static const struct YamlIdStr enum_SerialPort[] = {
  {  SP_AUX1, "AUX1"  },
  {  SP_AUX2, "AUX2"  },
  {  SP_VCP, "VCP"  },
  {  0, NULL  }
};

const struct YamlIdStr _old_enum_UartModes[] = {
  {  UART_MODE_NONE, "MODE_NONE"  },
  {  UART_MODE_TELEMETRY_MIRROR, "MODE_TELEMETRY_MIRROR"  },
  {  UART_MODE_TELEMETRY, "MODE_TELEMETRY"  },
  {  UART_MODE_SBUS_TRAINER, "MODE_SBUS_TRAINER"  },
  {  UART_MODE_LUA, "MODE_LUA"  },
  {  0, NULL  }
};

static const struct YamlIdStr enum_UartModes[] = {
  {  UART_MODE_NONE, "NONE"  },
  {  UART_MODE_TELEMETRY_MIRROR, "TELEMETRY_MIRROR"  },
  {  UART_MODE_TELEMETRY, "TELEMETRY_IN"  },
  {  UART_MODE_SBUS_TRAINER, "SBUS_TRAINER"  },
{    UART_MODE_SBUS_TRAINER_INV, "SBUS_TRAINER_INV"  },
  {  UART_MODE_LUA, "LUA"  },
  {  UART_MODE_CLI, "CLI"  },
  {  UART_MODE_GPS, "GPS"  },
  {  UART_MODE_DEBUG, "DEBUG"  },
  {  UART_MODE_SPACEMOUSE, "SPACEMOUSE"  },
  {  UART_MODE_EXT_MODULE, "EXT_MODULE"  },
  {  0, NULL  }
};

static uint32_t port_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  uint32_t port = yaml_parse_enum(enum_SerialPort, val, val_len);
  if (port < MAX_SERIAL_PORTS) return port;

  return -1;
}

static bool port_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = yaml_output_enum(idx, enum_SerialPort);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static const struct YamlNode struct_serialConfig[] = {
    YAML_IDX_CUST( "port", port_read, port_write),
    YAML_ENUM( "mode", 4, enum_UartModes, NULL),
    YAML_PADDING( 3 ),
    YAML_UNSIGNED( "power", 1 ),
    YAML_END
};

static void r_serialMode(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);

  auto node = tw->getAttr();
  if (!node || node->tag_len() < 4) return;

  uint8_t port_nr;
  if (node->tag[3] == 'S')
    port_nr = SP_AUX1;
  else if (node->tag[3] == '2')
    port_nr = SP_AUX2;
  else
    return;

  auto m = yaml_parse_enum(_old_enum_UartModes, val, val_len);
  if (!m) return;
  
  auto serialPort = reinterpret_cast<uint32_t*>(data);
  *serialPort = (*serialPort & ~(0xF << port_nr * SERIAL_CONF_BITS_PER_PORT)) |
                (m << port_nr * SERIAL_CONF_BITS_PER_PORT);
}

#if defined(FUNCTION_SWITCHES)

static uint16_t getIdx(void *user)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  return tw->getElmts(1);
}

static void r_cfs_name(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  strAppend(g_model.customSwitches[idx].name, val, LEN_SWITCH_NAME);
}

static const struct YamlNode struct_cfsNameConfig[] = {
  YAML_IDX,
  YAML_CUSTOM( "val", r_cfs_name, nullptr),
  YAML_END
};

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
static void r_cfs_on_color_r(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].onColor.r = yaml_str2uint(val, val_len);
}

static void r_cfs_on_color_g(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].onColor.g = yaml_str2uint(val, val_len);
}

static void r_cfs_on_color_b(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].onColor.b = yaml_str2uint(val, val_len);
}

static const struct YamlNode struct_cfsOnColorConfig[] = {
  YAML_IDX,
  YAML_CUSTOM( "r", r_cfs_on_color_r, nullptr),
  YAML_CUSTOM( "g", r_cfs_on_color_g, nullptr),
  YAML_CUSTOM( "b", r_cfs_on_color_b, nullptr),
  YAML_END
};

static void r_cfs_off_color_r(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].offColor.r = yaml_str2uint(val, val_len);
}

static void r_cfs_off_color_g(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].offColor.g = yaml_str2uint(val, val_len);
}

static void r_cfs_off_color_b(void* user, uint8_t* data, uint32_t bitoffs,
                             const char* val, uint8_t val_len)
{
  uint16_t idx = getIdx(user);
  g_model.customSwitches[idx].offColor.b = yaml_str2uint(val, val_len);
}

static const struct YamlNode struct_cfsOffColorConfig[] = {
  YAML_IDX,
  YAML_CUSTOM( "r", r_cfs_off_color_r, nullptr),
  YAML_CUSTOM( "g", r_cfs_off_color_g, nullptr),
  YAML_CUSTOM( "b", r_cfs_off_color_b, nullptr),
  YAML_END
};
#endif

static uint32_t cfs_idx_read(void* user, const char* val, uint8_t val_len)
{
  return switchGetCustomSwitchIdx(switchLookupIdx(val, val_len));
}

static bool cfs_idx_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = switchGetDefaultName(switchGetSwitchFromCustomIdx(idx));
  return str ? wf(opaque, str, strlen(str)) : true;
}

static const struct YamlNode struct_cfsGroupOn[] {
  YAML_IDX,
  YAML_UNSIGNED( "v", 1 ),
  YAML_END,
};

#if NUM_FUNCTIONS_SWITCHES > 6
#define NUM_CFS_FOR_CONVERSION  6
#else
#define NUM_CFS_FOR_CONVERSION  NUM_FUNCTIONS_SWITCHES
#endif

static void r_functionSwitchConfig(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  uint32_t v = yaml_str2uint(val, val_len);
  for (int i = 0; i < NUM_CFS_FOR_CONVERSION; i += 1) {
    g_model.customSwitches[i].type = (SwitchConfig)bfGet<uint16_t>(v, 2 * i, 2);
  }
}

static void r_functionSwitchStartConfig(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  uint32_t v = yaml_str2uint(val, val_len);
  for (int i = 0; i < NUM_CFS_FOR_CONVERSION; i += 1) {
    uint8_t b = bfGet<uint16_t>(v, 2 * i, 2);
    if (b < 2) b ^= 1;  // Swap On & Off
    g_model.customSwitches[i].start = (fsStartPositionType)b;
  }
}

static void r_functionSwitchGroup(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  uint32_t v = yaml_str2uint(val, val_len);
  for (int i = 0; i < NUM_CFS_FOR_CONVERSION; i += 1) {
    g_model.customSwitches[i].group = bfGet<uint16_t>(v, 2 * i, 2);
  }
  for (int i = 0; i <= 3; i += 1) {
    g_model.cfsSetGroupAlwaysOn(i, bfGet<uint16_t>(v, 2 * 6 + i, 1));
  }
}

static void r_functionSwitchLogicalState(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  uint32_t v = yaml_str2uint(val, val_len);
  for (int i = 0; i < NUM_CFS_FOR_CONVERSION; i += 1) {
    g_model.customSwitches[i].state = bfGet<uint16_t>(v, 1 * i, 1);
  }
}

bool cfsGroupIsActive(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  return idx > 0 && idx <= NUM_FUNCTIONS_GROUPS;
}
#endif

bool switchIsActive(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  return idx < switchGetMaxAllSwitches();
}

bool isAlwaysActive(void* user, uint8_t* data, uint32_t bitoffs)
{
  return true;
}

//
// Extern-array drivers (one per distinct array)
//
using EADriver = YamlNode::ExternArrayDriver;

static const EADriver yaml_drv_mix =
    { yaml_get_mix_ptr, yaml_ensure_mix_capacity, nullptr };
static const EADriver yaml_drv_expo =
    { yaml_get_expo_ptr, yaml_ensure_expo_capacity, nullptr };
static const EADriver yaml_drv_curves =
    { yaml_get_curves_ptr, yaml_ensure_curves_capacity, curve_is_active };
static const EADriver yaml_drv_points =
    { yaml_get_points_ptr, yaml_ensure_points_capacity, nullptr };
static const EADriver yaml_drv_logical_sw =
    { yaml_get_logical_sw_ptr, yaml_ensure_logical_sw_capacity, nullptr };
static const EADriver yaml_drv_custom_fn =
    { yaml_get_custom_fn_ptr, yaml_ensure_custom_fn_capacity, cfn_is_active };
static const EADriver yaml_drv_fmd =
    { yaml_get_fmd_ptr, yaml_ensure_fmd_capacity, fmd_is_active };
static const EADriver yaml_drv_gvar_data =
    { yaml_get_gvar_data_ptr, yaml_ensure_gvar_data_capacity, nullptr };
static const EADriver yaml_drv_gvar_values =
    { yaml_get_gvar_values_ptr, yaml_ensure_gvar_values_capacity, gvar_is_active };
static const EADriver yaml_drv_input_names =
    { yaml_get_input_names_ptr, yaml_ensure_input_names_capacity, nullptr };

// Custom IDX for inputNames extern array:
// Read: parse input number → allocate arena slot, return slot index
// Write: reverse-lookup slot → input number, output input number
static uint32_t r_input_name_idx(void* user, const char* val, uint8_t val_len)
{
  uint32_t input = yaml_str2uint(val, val_len);
  if (input >= MAX_INPUTS) return 0;

  char* name = inputNameAlloc(input);
  if (!name) return 0;

  return g_model.inputNameIndex[input];
}

static bool w_input_name_idx(void* user, yaml_writer_func wf, void* opaque)
{
  auto* walker = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t slot = walker->getElmts();

  // Reverse lookup: find which input owns this slot
  for (uint8_t i = 0; i < MAX_INPUTS; i++) {
    if (g_model.inputNameIndex[i] == slot) {
      char* s = yaml_unsigned2str(i);
      return wf(opaque, s, strlen(s));
    }
  }
  return false;
}
static const EADriver yaml_drv_radio_cfn =
    { yaml_get_radio_cfn_ptr, yaml_ensure_radio_cfn_capacity, cfn_is_active };
