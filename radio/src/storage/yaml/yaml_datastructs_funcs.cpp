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
  static_assert(MAX_GVARS == 9,"");
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
  int gvar = (node->size > 8 ? GV1_LARGE : GV1_SMALL);
  
  if ((val_len == 4)
      && (val[0] == '-')
      && (val[1] == 'G')
      && (val[2] == 'V')
      && (val[3] >= '1')
      && (val[3] <= '9')) {

    TRACE("%.*s -> %i\n", val_len, val, gvar - (val[3] - '0'));
    return gvar - (val[3] - '0');  // -GVx => 128 - x
  }

  if ((val_len == 3)
      && (val[0] == 'G')
      && (val[1] == 'V')
      && (val[2] >= '1')
      && (val[2] <= '9')) {

    TRACE("%.*s -> %i\n", val_len, val, -gvar + (val[2] - '1'));
    return -gvar + (val[2] - '1');  //  GVx => -128 + (x-1)
  }

  return (uint32_t)yaml_str2int(val, val_len);
}

bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                     void* opaque)
{
  int32_t sval = yaml_to_signed(val, node->size <= 11 ? node->size : 11);
  int32_t gvar = (node->size > 8 ? GV1_LARGE : GV1_SMALL);

  if (sval >= gvar - 10 && sval <= gvar) {
    char n = gvar - sval + '0';
    return wf(opaque, "-GV", 3) && wf(opaque, &n, 1);
  } else if (sval <= -gvar + 10 && sval >= -gvar) {
    char n = val - gvar + '1';
    return wf(opaque, "GV", 2) && wf(opaque, &n, 1);
  }

  char* s = yaml_signed2str(sval);
  return wf(opaque, s, strlen(s));
}

static int _legacy_input_idx(const char* val, uint8_t val_len)
{
  for (uint8_t i = 0; i < DIM(_legacy_inputs); i++){
    if (!strncmp(_legacy_inputs[i].legacy, val, val_len))
      return i;
  }

  return -1;
}

static int _legacy_mix_src(const char* val, uint8_t val_len)
{
  auto idx = _legacy_input_idx(val, val_len);
  if (idx >= 0)
    return _legacy_inputs[idx].src_raw;

  return -1;
}

extern const struct YamlIdStr enum_MixSources[];

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
static uint32_t r_mixSrcRaw(const YamlNode* node, const char* val, uint8_t val_len)
{
    // TODO: parse switch name as well
    if (val_len > 0 && val[0] == 'I') {
        return yaml_str2uint(val+1, val_len-1) + MIXSRC_FIRST_INPUT;
    } else if (val_len > 4 &&
               val[0] == 'l' &&
               val[1] == 'u' &&
               val[2] == 'a' &&
               val[3] == '(') {

      // parse int and ignore ','
      val += 4; val_len -= 4;
      uint8_t script = yaml_str2uint_ref(val, val_len);

      if (!val_len) return MIXSRC_NONE;
      val++; val_len--;
      
      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_LUA +
             script * MAX_SCRIPT_OUTPUTS;

    } else if (val_len > 3 &&
               val[0] == 'l' &&
               val[1] == 's' &&
               val[2] == '(') {

      val += 3; val_len -= 3;
      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_LOGICAL_SWITCH - 1;

    } else if (val_len > 3 &&
               val[0] == 't' &&
               val[1] == 'r' &&
               val[2] == '(') {

      val += 3; val_len -= 3;
      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_TRAINER;
      
    } else if (val_len > 3 &&
               val[0] == 'c' &&
               val[1] == 'h' &&
               val[2] == '(') {

      val += 3; val_len -= 3;
      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_CH;
      
    } else if (val_len > 3 &&
               val[0] == 'g' &&
               val[1] == 'v' &&
               val[2] == '(') {

      val += 3; val_len -= 3;
      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_GVAR;
#if defined(FUNCTION_SWITCHES)
    } else if (val_len > 2 &&
               val[0] == 'G' &&
               val[1] == 'R' &&
               val[2] >= '1' &&
               val[2] <= '3') {

      return MIXSRC_FIRST_CUSTOMSWITCH_GROUP + (val[2] - '1');
#endif
    } else if (val_len > 5 &&
               val[0] == 't' &&
               val[1] == 'e' &&
               val[2] == 'l' &&
               val[3] == 'e' &&
               val[4] == '(') {

      val += 5; val_len -= 5;

      // parse sign
      uint8_t sign = 0;
      if (*val == '-') {
        sign = 1;
        val++; val_len--;
      } else if (*val == '+') {
        sign = 2;
        val++; val_len--;
      }

      // parse int and ignore closing ')'
      return yaml_str2uint(val, val_len) * 3 + sign + MIXSRC_FIRST_TELEM;

    } else if (val_len > 3 &&
               val[0] == 'C' &&
               val[1] == 'Y' &&
               val[2] == 'C' &&
               val[3] >= '1' &&
               val[3] <= '3') {

      return MIXSRC_FIRST_HELI + (val[3] - '1');

    } else if (val_len > 3 &&
               val[0] == 'T' &&
               val[1] == 'm' &&
               val[2] == 'r' &&
               val[3] >= '1' &&
               val[3] <= ('0' + MAX_TIMERS)) {

      return MIXSRC_FIRST_TIMER + (val[3] - '1');

    } else if (val_len > 5 &&    // Old form, removed in 2.10
               val[0] == 'T' &&
               val[1] == 'I' &&
               val[2] == 'M' &&
               val[3] == 'E' &&
               val[4] == 'R' &&
               val[5] >= '1' &&
               val[5] <= ('0' + MAX_TIMERS)) {

      return MIXSRC_FIRST_TIMER + (val[5] - '1');

    } else if (val_len > 1 &&
               val[0] == 'T' &&
               val[1] >= '1' &&
               val[1] <= ('0' + MAX_TRIMS)) {

      return MIXSRC_FIRST_TRIM + (val[1] - '1');
    }

    auto idx = analogLookupCanonicalIdx(ADC_INPUT_MAIN, val, val_len);
    if (idx >= 0) return idx + MIXSRC_FIRST_STICK;

    idx = analogLookupCanonicalIdx(ADC_INPUT_FLEX, val, val_len);
    if (idx >= 0) return idx + MIXSRC_FIRST_POT;

    idx = switchLookupIdx(val, val_len);
    if (idx >= 0) return idx + MIXSRC_FIRST_SWITCH;
    
    idx = _legacy_mix_src(val, val_len);
    if (idx >= 0) return idx;
    
    return yaml_parse_enum(enum_MixSources, val, val_len);
}

static constexpr char closing_parenthesis[] = ")";

bool output_source_1_param(const char* src_prefix, size_t src_len, uint32_t n,
                           yaml_writer_func wf, void* opaque)
{
  if (!wf(opaque, src_prefix, src_len)) return false;
  const char* str = yaml_unsigned2str(n);
  if (!wf(opaque, str, strlen(str))) return false;
  return true;
}

static bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    const char* str = nullptr;

    if (val == MIXSRC_NONE) {

      return wf(opaque, "NONE", 4);

    } else if (val <= MIXSRC_LAST_INPUT) {

        if (!wf(opaque, "I", 1))
            return false;

        str = yaml_unsigned2str(val - MIXSRC_FIRST_INPUT);
    }
#if defined(LUA_INPUTS)
    else if (val <= MIXSRC_LAST_LUA) {
      
        val -= MIXSRC_FIRST_LUA;
        uint32_t script = val / MAX_SCRIPT_OUTPUTS;

        if (!output_source_1_param("lua(", 4, script, wf, opaque))
          return false;
        if (!wf(opaque, ",", 1)) return false;

        val = val % MAX_SCRIPT_OUTPUTS;
        str = yaml_unsigned2str(val);

        if (!wf(opaque, str, strlen(str))) return false;
        str = closing_parenthesis;
    }
#endif
    else if (val <= MIXSRC_LAST_STICK) {
        str = analogGetCanonicalName(ADC_INPUT_MAIN, val - MIXSRC_FIRST_STICK);
    }
    else if (val <= MIXSRC_LAST_POT) {
        str = analogGetCanonicalName(ADC_INPUT_FLEX, val - MIXSRC_FIRST_POT);
    }
    else if (val >= MIXSRC_FIRST_HELI
             && val <= MIXSRC_LAST_HELI) {
        if (!wf(opaque, "CYC", 3)) return false;
        str = yaml_unsigned2str(val - MIXSRC_FIRST_HELI + 1);
    }
    else if (val >= MIXSRC_FIRST_TRIM
             && val <= MIXSRC_LAST_TRIM) {
        if (!wf(opaque, "T", 1)) return false;
        str = yaml_unsigned2str(val - MIXSRC_FIRST_TRIM + 1);
    }
    else if (val >= MIXSRC_FIRST_SWITCH
             && val <= MIXSRC_LAST_SWITCH) {
        str = switchGetCanonicalName(val - MIXSRC_FIRST_SWITCH);
    }
#if defined(FUNCTION_SWITCHES)
    else if (val >= MIXSRC_FIRST_CUSTOMSWITCH_GROUP
             && val <= MIXSRC_LAST_CUSTOMSWITCH_GROUP) {
        str = fsSwitchGroupGetCanonicalName(val - MIXSRC_FIRST_CUSTOMSWITCH_GROUP);
    }
#endif
    else if (val >= MIXSRC_FIRST_LOGICAL_SWITCH
             && val <= MIXSRC_LAST_LOGICAL_SWITCH) {

        val -= MIXSRC_FIRST_LOGICAL_SWITCH;
        if (!output_source_1_param("ls(", 3, val + 1, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_TRAINER
             && val <= MIXSRC_LAST_TRAINER) {

        val -= MIXSRC_FIRST_TRAINER;
        if (!output_source_1_param("tr(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_CH
             && val <= MIXSRC_LAST_CH) {

        val -= MIXSRC_FIRST_CH;
        if (!output_source_1_param("ch(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_GVAR
             && val <= MIXSRC_LAST_GVAR) {

        val -= MIXSRC_FIRST_GVAR;
        if (!output_source_1_param("gv(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_TIMER
             && val <= MIXSRC_LAST_TIMER) {
        if (!wf(opaque, "Tmr", 3)) return false;
        str = yaml_unsigned2str(val - MIXSRC_FIRST_TIMER + 1);
    }
    else if (val >= MIXSRC_FIRST_TELEM
             && val <= MIXSRC_LAST_TELEM) {

        val -= MIXSRC_FIRST_TELEM;
        uint8_t sign = val % 3;
        val = val / 3;
        if (!wf(opaque, "tele(", 5)) return false;
        if (sign == 1) {
          if (!wf(opaque, "-", 1)) return false;
        } else if (sign == 2) {
          if (!wf(opaque, "+", 1)) return false;
        }
        str = yaml_unsigned2str(val);
        if (!wf(opaque, str, strlen(str))) return false;
        str = closing_parenthesis;
    }
    else {
        str = yaml_output_enum(val, enum_MixSources);
    }

    if (str) {
        return wf(opaque, str, strlen(str));
    }

    return true;
}

static uint32_t r_mixSrcRawEx(const YamlNode* node, const char* val, uint8_t val_len)
{
  bool invert = false;
  if (val[0] == '!') {
    invert = true;
    val += 1;
    val_len -= 1;
  }
  int32_t rv = r_mixSrcRaw(node, val, val_len);
  if (invert)
    rv = -rv;
  return (uint32_t)rv;
}

static bool w_mixSrcRawExNoQuote(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  // Check for negative 10 bit value. TODO: handle this better!
  val &= 0x3FF;
  if (val >= 512) {
    if (!wf(opaque, "!", 1)) return false;
    val = 1024 - val;
  }
  return w_mixSrcRaw(node, val, wf, opaque);
}

static bool w_mixSrcRawEx(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  if (!wf(opaque, "\"", 1)) return false;
  if (!w_mixSrcRawExNoQuote(node, val, wf, opaque)) return false;
  return wf(opaque, "\"", 1);
}

static uint32_t r_sourceNumVal(const YamlNode* node, const char* val, uint8_t val_len)
{
  SourceNumVal v;

  if (((val[0] == '-') && (val[1] >= '0' && val[1] <= '9')) || (val[0] >= '0' && val[0] <= '9')) {
    v.isSource = 0;
    v.value = (uint32_t)yaml_str2int(val, val_len);
  } else if ((val[0] == '-') && (val[1] == 'G')) {
    v.isSource = 1;
    v.value = -((val[3] - '0') + MIXSRC_FIRST_GVAR - 1);
  } else if (val[0] == 'G') {
    v.isSource = 1;
    v.value = (val[2] - '0') + MIXSRC_FIRST_GVAR - 1;
  } else {
    v.isSource = 1;
    v.value = r_mixSrcRawEx(node, val, val_len);
  }

  return v.rawValue;
}

bool w_sourceNumVal(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                     void* opaque)
{
  SourceNumVal v;
  v.rawValue = val;

  if (v.isSource)
    return w_mixSrcRawEx(node, v.value, wf, opaque);

  char* s = yaml_signed2str(v.value);
  return wf(opaque, s, strlen(s));
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
static uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= offsetof(ZoneOptionValueTyped, value);
  ZoneOptionValueEnum* p_zovt = (ZoneOptionValueEnum*)data;
  if (*p_zovt > ZOV_Color) return 0;
  return *p_zovt;
}

void r_zov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);
  p_val->unsignedValue = r_mixSrcRaw(nullptr, val, val_len);
}

bool w_zov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);
  return w_mixSrcRaw(nullptr, p_val->unsignedValue, wf, opaque);
}

void r_zov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;

  ZoneOptionValue zov;
  if (strncmp(val, "COLIDX", 6) == 0) {
    val += 6; val_len -= 6;
    zov.unsignedValue = COLOR2FLAGS(yaml_str2uint(val, val_len));
  } else {
    if (val_len < sizeof("0xFFFFFF")-1
        || val[0] != '0'
        || val[1] != 'x')
      return;

    val += 2; val_len -= 2;

    auto rgb24 = yaml_hex2uint(val, val_len);
    zov.unsignedValue = RGB2FLAGS((rgb24 & 0xFF0000) >> 16,
                                  (rgb24 & 0xFF00) >> 8, rgb24 & 0xFF);
  }

  memcpy(data, &zov, sizeof(ZoneOptionValue));
}

bool w_zov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;

  ZoneOptionValue zov;
  memcpy(&zov, data, sizeof(ZoneOptionValue));

  uint32_t val = zov.unsignedValue;
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

static bool stick_name_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  return analogHasCustomLabel(ADC_INPUT_MAIN, idx);
}

static const struct YamlNode struct_stickConfig[] = {
    YAML_IDX,
    YAML_CUSTOM( "name", r_stick_name, w_stick_name),
    YAML_END
};

static uint32_t slider_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  auto idx = _legacy_mix_src(val, val_len);
  if (idx >= 0) return idx - MIXSRC_FIRST_POT;

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

static uint32_t sw_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  return switchLookupIdx(val, val_len);
}

bool sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = switchGetCanonicalName(idx);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void sw_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);


  switchSetCustomName(idx, val, val_len);
}

static bool sw_name_write(void* user, uint8_t* data, uint32_t bitoffs,
                          yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);


  const char* str = switchGetCustomName(idx);
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, str, strnlen(str, LEN_SWITCH_NAME)))
    return false;
  return wf(opaque, "\"", 1);
}

static const struct YamlIdStr enum_SwitchConfig[] = {
    {  SWITCH_NONE, "none"  },
    {  SWITCH_TOGGLE, "toggle"  },
    {  SWITCH_2POS, "2pos"  },
    {  SWITCH_3POS, "3pos"  },
    {  0, NULL  }
};

static const struct YamlNode struct_switchConfig[] = {
    YAML_IDX_CUST( "sw", sw_read, sw_write),
    YAML_ENUM( "type", 2, enum_SwitchConfig),
    YAML_CUSTOM( "name", sw_name_read, sw_name_write),
    YAML_END
};

static bool flex_sw_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  return switchIsFlexValid_raw(idx);
}

uint8_t boardGetMaxSwitches();

static uint32_t flex_sw_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  auto idx = switchLookupIdx(val, val_len);
  return idx - boardGetMaxSwitches();
}

bool flex_sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  auto sw_offset = boardGetMaxSwitches();
  const char* str = switchGetCanonicalName(idx + sw_offset);
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

  idx = _legacy_mix_src(val, val_len);
  if (idx >= MIXSRC_FIRST_POT && idx <= MIXSRC_LAST_POT)
    return idx - MIXSRC_FIRST_POT;

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
    YAML_ENUM("type", POT_CFG_TYPE_BITS, enum_PotConfig),
    YAML_UNSIGNED("inv", POT_CFG_INV_BITS),
    YAML_CUSTOM("name", pot_name_read, pot_name_write),
    YAML_END
};

extern const struct YamlIdStr enum_SwitchSources[];

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

static uint32_t r_swtchSrc(const YamlNode* node, const char* val, uint8_t val_len)
{
    int32_t ival=0;
    bool neg =  false;
    if (val_len > 0 && val[0] == '!') {
        neg = true;
        val++;
        val_len--;
    }

    if (val_len > 3
	&& ((val[0] == 'S' && val[1] >= 'W')
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
      str = switchGetCanonicalName(sw_info.quot);
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

bool cfn_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  return ((CustomFunctionData*)data)->swtch;
}

static bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  // TODO: no need to output 0 values for FM0
  gvar_t* gvar = (gvar_t*)(data + (bitoffs>>3UL));
  return *gvar != GVAR_MAX+1;
}

static bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  // FM0 defaults to all 0
  if (idx == 0) {
    return !yaml_is_zero(data, bitoffs, sizeof(FlightModeData) << 3UL);
  }

  // assumes gvars array is last
  bool is_active = !yaml_is_zero(data, bitoffs, offsetof(FlightModeData, gvars) << 3UL);

  data += bitoffs >> 3UL;
  FlightModeData* fmd = (FlightModeData*)(data);
  for (uint8_t i = 0; i < MAX_GVARS; i++) {
    is_active |= fmd->gvars[i] != GVAR_MAX + 1; // FM0 -> default
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
  YAML_IDX_CUST( "sw", sw_read, sw_write ),
  YAML_ENUM( "pos", 2, enum_SwitchWarnPos ),
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
    CFN_PARAM(cfn) = r_mixSrcRawEx(nullptr, val, l_sep);
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
      CFN_PARAM(cfn) = r_mixSrcRawEx(nullptr, val, l_sep);
      break;
    case FUNC_ADJUST_GVAR_GVAR: {
      uint32_t gvar = r_mixSrcRawEx(nullptr, val, l_sep);
      if (gvar >= MIXSRC_FIRST_GVAR) {
        CFN_PARAM(cfn) = gvar - MIXSRC_FIRST_GVAR;
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
    if (!w_mixSrcRawExNoQuote(nullptr, CFN_PARAM(cfn), wf, opaque)) return false;
    break;

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
      if (!w_mixSrcRawExNoQuote(nullptr, CFN_PARAM(cfn), wf, opaque)) return false;
      break;
    case FUNC_ADJUST_GVAR_GVAR:
      if (!w_mixSrcRawExNoQuote(nullptr, CFN_PARAM(cfn) + MIXSRC_FIRST_GVAR, wf, opaque)) return false;
      break;
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
    ls->v1 = r_swtchSrc(nullptr, val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = r_swtchSrc(nullptr, val, val_len);
    break;

  case LS_FAMILY_EDGE:
    ls->v1 = r_swtchSrc(nullptr, val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = timerValue2lsw(yaml_str2uint_ref(val, val_len));
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    if (val_len == 1 && val[0] == '<') {
      ls->v3 = -1;
    } else if (val_len == 1 && val[0] == '-') {
        ls->v3 = 0;
    } else {
      int16_t t = (int16_t)timerValue2lsw(yaml_str2uint_ref(val, val_len));
      ls->v3 = t - ls->v2;
    }
    break;
    
  case LS_FAMILY_COMP:
    ls->v1 = r_mixSrcRawEx(nullptr, val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = r_mixSrcRawEx(nullptr, val, val_len);
    break;
    
  case LS_FAMILY_TIMER:
    ls->v1 = timerValue2lsw(yaml_str2uint(val, l_sep));
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = timerValue2lsw(yaml_str2uint(val, val_len));
    break;
    
  default:
    ls->v1 = r_mixSrcRawEx(nullptr, val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    // TODO?: ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2
    ls->v2 = yaml_str2int_ref(val, val_len);
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
    if (!w_swtchSrc_unquoted(&_ls_node_v1, ls->v1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    if (!w_swtchSrc_unquoted(&_ls_node_v2, ls->v2, wf, opaque)) return false;
    break;

  case LS_FAMILY_EDGE:
    if (!w_swtchSrc_unquoted(&_ls_node_v1, ls->v1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(lswTimerValue(ls->v2));
    if (!wf(opaque,str,strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    if (ls->v3 < 0) {
      if (!wf(opaque,"<",1)) return false;
    } else if(ls->v3 == 0) {
      if (!wf(opaque,"-",1)) return false;
    } else {
      str = yaml_unsigned2str(lswTimerValue(ls->v2 + ls->v3));
      if (!wf(opaque, str, strlen(str))) return false;
    }
    break;
    
  case LS_FAMILY_COMP:
    if (!w_mixSrcRawExNoQuote(nullptr, ls->v1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    if (!w_mixSrcRawExNoQuote(nullptr, ls->v2, wf, opaque)) return false;
    break;
    
  case LS_FAMILY_TIMER:
    str = yaml_unsigned2str(lswTimerValue(ls->v1));
    if (!wf(opaque,str,strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(lswTimerValue(ls->v2));
    if (!wf(opaque,str,strlen(str))) return false;
    break;
    
  default:
    if (!w_mixSrcRawExNoQuote(nullptr, ls->v1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    // TODO?: ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2
    str = yaml_signed2str(ls->v2);
    if (!wf(opaque,str,strlen(str))) return false;
    break;
  }

  if (!wf(opaque,"\"",1)) return false;
  return true;
}

static uint32_t r_thrSrc(const YamlNode* node, const char* val, uint8_t val_len)
{
  auto src = r_mixSrcRaw(nullptr, val, val_len);
  auto thrSrc = source2ThrottleSource(src);

  if (thrSrc < 0)
    return 0;

  return thrSrc;
}

static bool w_thrSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                     void* opaque)
{
  auto src = throttleSource2Source(val);
  return w_mixSrcRaw(nullptr, src, wf, opaque);
}

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

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
static void r_rotEncDirection(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  uint32_t i = yaml_str2uint(val, val_len);
  yaml_put_bits(data, i, bitoffs, 2);
}
#endif

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
    YAML_ENUM( "mode", 4, enum_UartModes),
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
