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

#include "opentx.h"
#include "yaml_bits.h"
#include "yaml_tree_walker.h"

#include "pulses/multi.h"
#include "stamp.h"

// Use definitions from v220 conversions as long as nothing changes

namespace yaml_conv_220 {
  bool w_board(void* user, uint8_t* data, uint32_t bitoffs, yaml_writer_func wf, void* opaque);

  bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

  bool output_source_1_param(const char* src_prefix, size_t src_len, uint32_t n,
                             yaml_writer_func wf, void* opaque);

  bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

  uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_script_input(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs);

  extern const struct YamlIdStr enum_SwitchConfig[];

  bool cfn_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool swash_is_active(void* user, uint8_t* data, uint32_t bitoffs);

  bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

  extern const char* _tele_screen_type_lookup[];
  bool w_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                          yaml_writer_func wf, void* opaque);

  bool w_tele_sensor(const YamlNode* node, uint32_t val,
                     yaml_writer_func wf, void* opaque);

  bool w_flightModes(const YamlNode* node, uint32_t val,
                     yaml_writer_func wf, void* opaque);

  extern const char* _func_sound_lookup[];
  extern const uint8_t _func_sound_lookup_size;

  extern const char* _func_reset_param_lookup[];
  extern const char* _func_failsafe_lookup[];

  extern const char* _adjust_gvar_mode_lookup[];
  extern const uint8_t _adjust_gvar_mode_lookup_size;

  bool w_zov_source(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque);

  bool w_zov_color(void* user, uint8_t* data, uint32_t bitoffs,
                   yaml_writer_func wf, void* opaque);

  extern const struct YamlIdStr enum_XJT_Subtypes[];
  extern const struct YamlIdStr enum_ISRM_Subtypes[];
  extern const struct YamlIdStr enum_R9M_Subtypes[];
  extern const struct YamlIdStr enum_FLYSKY_Subtypes[];
  extern const struct YamlIdStr enum_DSM2_Subtypes[];
  
  bool w_modSubtype(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque);

  bool w_channelsCount(const YamlNode* node, uint32_t val, yaml_writer_func wf,
                       void* opaque);

  extern const struct YamlIdStr enum_UartModes[];
};

//
// WARNING:
// ========
//
//  If any of these static_assert() fails, you need to check that
//  the functions bellow are still applicable.
//
//  Please note that the sizes used here are those from the v220 format
//  (see storage/conversions/yaml/datastructs_220.h)
//
static inline void check_yaml_funcs()
{
  static_assert(offsetof(ModuleData, ppm) == 4,"");
  check_size<ModuleData, 29>();
  static_assert(MAX_GVARS == 9,"");
#if defined(PCBHORUS)
  static_assert(offsetof(FlightModeData, gvars) == 26,"");
  check_size<FlightModeData, 44>();
  check_size<CustomFunctionData, 9>();
#elif defined(PCBNV14)
  static_assert(offsetof(FlightModeData, gvars) == 22,"");
  check_size<FlightModeData, 40>();
  check_size<CustomFunctionData, 9>();
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
  static_assert(offsetof(FlightModeData, gvars) == 18,"");
  check_size<FlightModeData, 36>();
  check_size<CustomFunctionData, 11>();
  check_size<TelemetryScreenData, 24>();
#else
  static_assert(offsetof(FlightModeData, gvars) == 22,"");
  check_size<FlightModeData, 40>();
  check_size<CustomFunctionData, 11>();
  check_size<TelemetryScreenData, 24>();
#endif
}

static bool w_semver(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque)
{
  return wf(opaque, VERSION, sizeof(VERSION)-1);
}

static bool w_board(void* user, uint8_t* data, uint32_t bitoffs,
                    yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_board(user, data, bitoffs, wf, opaque);
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

static bool in_write_weight(const YamlNode* node, uint32_t val,
                            yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::in_write_weight(node, val, wf, opaque);
}

extern const struct YamlIdStr enum_MixSources[];

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
    }

    return yaml_parse_enum(enum_MixSources, val, val_len);
}

static constexpr char closing_parenthesis[] = ")";

static bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    const char* str = nullptr;

    if (val >= MIXSRC_FIRST_INPUT
        && val <= MIXSRC_LAST_INPUT) {

        if (!wf(opaque, "I", 1))
            return false;

        str = yaml_unsigned2str(val - MIXSRC_FIRST_INPUT);
    }
#if defined(LUA_INPUTS)
    else if (val >= MIXSRC_FIRST_LUA
             && val <= MIXSRC_LAST_LUA) {
      
        val -= MIXSRC_FIRST_LUA;
        uint32_t script = val / MAX_SCRIPT_OUTPUTS;

        if (!yaml_conv_220::output_source_1_param("lua(", 4, script, wf,
                                                  opaque))
          return false;
        if (!wf(opaque, ",", 1)) return false;

        val = val % MAX_SCRIPT_OUTPUTS;
        str = yaml_unsigned2str(val);

        if (!wf(opaque, str, strlen(str))) return false;
        str = closing_parenthesis;
    }
#endif
    else if (val >= MIXSRC_FIRST_LOGICAL_SWITCH
             && val <= MIXSRC_LAST_LOGICAL_SWITCH) {

        val -= MIXSRC_FIRST_LOGICAL_SWITCH;
        if (!yaml_conv_220::output_source_1_param("ls(", 3, val + 1, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_TRAINER
             && val <= MIXSRC_LAST_TRAINER) {

        val -= MIXSRC_FIRST_TRAINER;
        if (!yaml_conv_220::output_source_1_param("tr(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_CH
             && val <= MIXSRC_LAST_CH) {

        val -= MIXSRC_FIRST_CH;
        if (!yaml_conv_220::output_source_1_param("ch(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_GVAR
             && val <= MIXSRC_LAST_GVAR) {

        val -= MIXSRC_FIRST_GVAR;
        if (!yaml_conv_220::output_source_1_param("gv(", 3, val, wf, opaque))
          return false;
        str = closing_parenthesis;
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

static uint32_t r_vbat_min(const YamlNode* node, const char* val, uint8_t val_len)
{
    int32_t v = yaml_str2int(val, val_len);
    return (uint32_t)(v - 90);
}

static bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_vbat_min(node, val, wf, opaque);
}

static uint32_t r_vbat_max(const YamlNode* node, const char* val, uint8_t val_len)
{
  int32_t v = yaml_str2int(val, val_len);
  return (uint32_t)(v - 120);
}

static bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_vbat_max(node, val, wf, opaque);
}

#if defined(COLORLCD)
static uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_zov(user, data, bitoffs);
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
  if (val_len < sizeof("0xFFFFFF")-1
      || val[0] != '0'
      || val[1] != 'x')
    return;

  val += 2; val_len -= 2;

  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);

  auto rgb24 = yaml_hex2uint(val, val_len);
  p_val->unsignedValue =
      RGB((rgb24 & 0xFF0000) >> 16, (rgb24 & 0xFF00) >> 8, rgb24 & 0xFF);
}

bool w_zov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_zov_color(user, data, bitoffs, wf, opaque);
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
    case MODULE_TYPE_FLYSKY:
      if (mod_data->subType == FLYSKY_SUBTYPE_AFHDS2A) return 6;
      if (mod_data->subType == FLYSKY_SUBTYPE_AFHDS3) return 7;
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
  return yaml_conv_220::select_script_input(user, data, bitoffs);
}

static uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_id1(user, data, bitoffs);
}

static uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_id2(user, data, bitoffs);
}

static uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_sensor_cfg(user, data, bitoffs);
}

static uint32_t r_calib(void* user, const char* val, uint8_t val_len)
{
  (void)user;

  uint32_t sw = yaml_parse_enum(enum_MixSources, val, val_len);
  if (sw >= MIXSRC_Rud) return sw - MIXSRC_Rud;

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

  const char* str =
      yaml_output_enum(idx + MIXSRC_Rud, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static uint32_t sw_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  uint32_t sw = yaml_parse_enum(enum_MixSources, val, val_len);
  if (sw >= MIXSRC_FIRST_SWITCH) return sw - MIXSRC_FIRST_SWITCH;

  return -1;
}

bool sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str =
      yaml_output_enum(idx + MIXSRC_FIRST_SWITCH, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void r_stick_name(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);
  if (idx >= NUM_STICKS) return;

  data -= offsetof(RadioData, switchConfig);
  RadioData* rd = reinterpret_cast<RadioData*>(data);
  strncpy(rd->anaNames[idx], val, std::min<uint8_t>(val_len, LEN_ANA_NAME));
}

static bool w_stick_name(void* user, uint8_t* data, uint32_t bitoffs,
                         yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  data -= offsetof(RadioData, switchConfig);
  RadioData* rd = reinterpret_cast<RadioData*>(data);
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, rd->anaNames[idx],
          strnlen(rd->anaNames[idx], LEN_ANA_NAME)))
    return false;
  return wf(opaque, "\"", 1);
}

static bool stick_name_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  RadioData* rd = reinterpret_cast<RadioData*>(data);
  return rd->anaNames[idx][0] != '\0';
}

static const struct YamlNode struct_sticksConfig[] = {
    YAML_IDX,
    YAML_CUSTOM( "name", r_stick_name, w_stick_name),
    YAML_END
};

static void sw_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
  data -= ((idx + 1) * 2) / 8;
  data -= offsetof(RadioData, switchConfig);

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  strncpy(rd->switchNames[idx], val,
          std::min<uint8_t>(val_len, LEN_SWITCH_NAME));
}

static bool sw_name_write(void* user, uint8_t* data, uint32_t bitoffs,
                          yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
  data -= ((idx + 1) * 2) / 8;
  data -= offsetof(RadioData, switchConfig);

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  const char* str = rd->switchNames[idx];
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, str, strnlen(str, LEN_SWITCH_NAME)))
    return false;
  return wf(opaque, "\"", 1);
}

static const struct YamlNode struct_switchConfig[] = {
    YAML_IDX_CUST( "sw", sw_read, sw_write),
    YAML_ENUM( "type", 2, yaml_conv_220::enum_SwitchConfig),
    YAML_CUSTOM( "name", sw_name_read, sw_name_write),
    YAML_END
};

static uint32_t pot_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  uint32_t pot = yaml_parse_enum(enum_MixSources, val, val_len);
  if (pot >= MIXSRC_FIRST_POT) return pot - MIXSRC_FIRST_POT;

  return -1;
}

static bool pot_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = yaml_output_enum(idx + MIXSRC_FIRST_POT, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void pot_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                          const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
  data -= ((idx + 1) * 2) / 8;
  data -= offsetof(RadioData, potsConfig);

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  idx += NUM_STICKS;
  strncpy(rd->anaNames[idx], val, std::min<uint8_t>(val_len, LEN_ANA_NAME));
}

static bool pot_name_write(void* user, uint8_t* data, uint32_t bitoffs,
                           yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
  data -= ((idx + 1) * 2) / 8;
  data -= offsetof(RadioData, potsConfig);

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  idx += NUM_STICKS;
  const char* str = rd->anaNames[idx];
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, str, strnlen(str, LEN_ANA_NAME)))
    return false;
  return wf(opaque, "\"", 1);
}

static const struct YamlIdStr enum_PotConfig[] = {
    {  POT_NONE, "none" },
    {  POT_WITH_DETENT, "with_detent" },
    {  POT_MULTIPOS_SWITCH, "multipos_switch" },
    {  POT_WITHOUT_DETENT, "without_detent" },
    {  0, NULL }
};

static const struct YamlNode struct_potConfig[] = {
    YAML_IDX_CUST( "pot", pot_read, pot_write ),
    YAML_ENUM( "type", 2, enum_PotConfig),
    YAML_CUSTOM( "name", pot_name_read, pot_name_write),
    YAML_END
};

static uint32_t slider_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  uint32_t sl = yaml_parse_enum(enum_MixSources, val, val_len);
  if (sl >= MIXSRC_FIRST_SLIDER) return sl - MIXSRC_FIRST_SLIDER;

  return -1;
}

static bool slider_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str =
      yaml_output_enum(idx + MIXSRC_FIRST_SLIDER, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

static void sl_name_read(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
#if defined(PCBTARANIS)
  // Please note:
  //   slidersConfig is defined as a bit-field member,
  //   so let's take the next field and subtract 1
  //
  data -= (idx + 4 /* bitsize previous field (auxSerialMode) */ + 1) / 8;
  data -= offsetof(RadioData, potsConfig) - 1;
#else
  data -= (idx + 1) / 8;
  data -= offsetof(RadioData, slidersConfig);
#endif

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  idx += NUM_STICKS + STORAGE_NUM_POTS;
  strncpy(rd->anaNames[idx], val, std::min<uint8_t>(val_len, LEN_ANA_NAME));
}

static bool sl_name_write(void* user, uint8_t* data, uint32_t bitoffs,
                          yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  // data / bitoffs already incremented
#if defined(PCBTARANIS)
  // Please note:
  //   slidersConfig is defined as a bit-field member,
  //   so let's take the next field and subtract 1
  //
  data -= (idx + 4 /* bitsize previous field (auxSerialMode) */ + 1) / 8;
  data -= offsetof(RadioData, potsConfig) - 1;
#else
  data -= (idx + 1) / 8;
  data -= offsetof(RadioData, slidersConfig);
#endif

  RadioData* rd = reinterpret_cast<RadioData*>(data);
  idx += NUM_STICKS + STORAGE_NUM_POTS;
  const char* str = rd->anaNames[idx];
  if (!wf(opaque, "\"", 1)) return false;
  if (!wf(opaque, str, strnlen(str, LEN_ANA_NAME)))
    return false;
  return wf(opaque, "\"", 1);
}

static const struct YamlIdStr enum_SliderConfig[] = {
    {  SLIDER_NONE, "none" },
    {  SLIDER_WITH_DETENT, "with_detent" },
    {  0, NULL }
};

static const struct YamlNode struct_sliderConfig[] = {
    YAML_IDX_CUST( "sl", slider_read, slider_write ),
    YAML_ENUM( "type", 1, enum_SliderConfig),
    YAML_CUSTOM( "name", sl_name_read, sl_name_write),
    YAML_END
};

extern const struct YamlIdStr enum_SwitchSources[];

static uint32_t r_swtchSrc(const YamlNode* node, const char* val, uint8_t val_len)
{
    int32_t ival=0;
    bool neg =  false;
    if (val_len > 0 && val[0] == '!') {
        neg = true;
        val++;
        val_len--;
    }

    if (val_len >= 2
             && val[0] == 'L'
             && (val[1] >= '0' && val[1] <= '9')) {

        ival = SWSRC_FIRST_LOGICAL_SWITCH + yaml_str2int(val+1, val_len-1) - 1;
    }
#if NUM_XPOTS > 0
    else if (val_len > 3
        && val[0] == '6'
        && val[1] == 'P'
        && (val[2] >= '0' && val[2] <= '9')
        && (val[3] >= '0' && val[3] < (XPOTS_MULTIPOS_COUNT + '0'))) {

        ival = (val[2] - '0') * XPOTS_MULTIPOS_COUNT + (val[3] - '0')
            + SWSRC_FIRST_MULTIPOS_SWITCH;
    }
#endif
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
    if (sval >= SWSRC_FIRST_LOGICAL_SWITCH
             && sval <= SWSRC_LAST_LOGICAL_SWITCH) {

        wf(opaque, "L", 1);
        str = yaml_unsigned2str(sval - SWSRC_FIRST_LOGICAL_SWITCH + 1);
        return wf(opaque,str, strlen(str));
    }
#if NUM_XPOTS > 0
    else if (sval >= SWSRC_FIRST_MULTIPOS_SWITCH
             && sval <= SWSRC_LAST_MULTIPOS_SWITCH) {

        wf(opaque, "6P", 2);

        // pot #: start with 6P1
        sval -= SWSRC_FIRST_MULTIPOS_SWITCH;
        str = yaml_unsigned2str(sval / XPOTS_MULTIPOS_COUNT);
        wf(opaque,str, strlen(str));

        // position
        str = yaml_unsigned2str(sval % XPOTS_MULTIPOS_COUNT);
        return wf(opaque,str, strlen(str));
    }
#endif
    else if (sval >= SWSRC_FIRST_FLIGHT_MODE
             && sval <= SWSRC_LAST_FLIGHT_MODE) {

        wf(opaque, "FM", 2);
        str = yaml_unsigned2str(sval - SWSRC_FIRST_FLIGHT_MODE);
        return wf(opaque,str, strlen(str));
    }
    else if (sval >= SWSRC_FIRST_SENSOR
             && sval <= SWSRC_LAST_SENSOR) {

        wf(opaque, "T", 1);
        str = yaml_unsigned2str(sval - SWSRC_FIRST_SENSOR + 1);
        return wf(opaque,str, strlen(str));
    }
    
    str = yaml_output_enum(sval, enum_SwitchSources);
    return wf(opaque, str, strlen(str));
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
  return yaml_conv_220::cfn_is_active(user, data, bitoffs);
}

static bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::gvar_is_active(user, data, bitoffs);
}

static bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::fmd_is_active(user, data, bitoffs);
}

static bool swash_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::swash_is_active(user, data, bitoffs);
}

static void r_swtchWarn(void* user, uint8_t* data, uint32_t bitoffs,
                        const char* val, uint8_t val_len)
{
  data += (bitoffs >> 3UL);
  swarnstate_t& swtchWarn = *(swarnstate_t*)data;

  // Read from string like 'AdBuC-':
  //
  // -> reads:
  //    - Switch A: must be DOWN
  //    - Switch B: must be UP
  //    - Switch C: must be MIDDLE
  //
  // -> switches not in the list shall not be checked
  //
  swtchWarn = 0;
  while (val_len--) {
    signed swtch = getRawSwitchIdx(*(val++));
    if (swtch < 0) break;

    unsigned state = 0;
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
}

static bool w_swtchWarn(void* user, uint8_t* data, uint32_t bitoffs,
                        yaml_writer_func wf, void* opaque)
{
  data += (bitoffs >> 3UL);
  swarnstate_t states = *(swarnstate_t*)data;

  for (int i = 0; i < NUM_SWITCHES; i++) {
    // TODO: SWITCH_EXISTS() uses the g_eeGeneral stucture, which might not be
    // avail
    if (SWITCH_EXISTS(i)) {
      // decode check state
      // -> 3 bits per switch
      auto state = (states >> (3 * i)) & 0x07;

      // state == 0 -> no check
      // state == 1 -> UP
      // state == 2 -> MIDDLE
      // state == 3 -> DOWN
      char swtchWarn[2] = {getRawSwitchFromIdx(i), 0};

      switch (state) {
        case 0:
          break;
        case 1:
          swtchWarn[1] = 'u';
          break;
        case 2:
          swtchWarn[1] = '-';
          break;
        case 3:
          swtchWarn[1] = 'd';
          break;
        default:
          // this should never happen
          swtchWarn[1] = 'x';
          break;
      }

      if (swtchWarn[1] != 0) {
        if (!wf(opaque, swtchWarn, 2)) {
          return false;
        }
      }
    }
  }

  return true;
}

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
  return yaml_conv_220::w_5pos(node, val, wf, opaque);
}

static uint32_t r_vol(const YamlNode* node, const char* val, uint8_t val_len)
{
    return (uint32_t)(yaml_str2int(val, val_len) - VOLUME_LEVEL_DEF);
}

static bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_vol(node, val, wf, opaque);
}

static uint32_t r_spPitch(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) / 15);
}

static bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_spPitch(node, val, wf, opaque);
}

static uint32_t r_vPitch(const YamlNode* node, const char* val, uint8_t val_len)
{
  return (uint32_t)(yaml_str2int(val, val_len) / 10);
}

static bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_vPitch(node, val, wf, opaque);
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

static void r_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                               const char* val, uint8_t val_len)
{
  uint8_t type = 0;
  for (uint8_t i = 0; i < 4; i++) {
    if (!strncmp(val, yaml_conv_220::_tele_screen_type_lookup[i], val_len)) {
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
  return yaml_conv_220::w_tele_screen_type(user, data, bitoffs, wf, opaque);
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
  return yaml_conv_220::w_tele_sensor(node, val, wf, opaque);
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
  return yaml_conv_220::w_flightModes(node, val, wf, opaque);
}

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
  const char* sep = (const char *)memchr(val, ',', val_len);
  uint8_t l_sep = sep ? sep - val : val_len;

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
      CFN_CH_INDEX(cfn) = NUM_STICKS + 1;
    } else {
      uint32_t stick = yaml_parse_enum(enum_MixSources, val, l_sep);
      if (stick >= MIXSRC_FIRST_STICK && stick <= MIXSRC_LAST_STICK) {
        CFN_CH_INDEX(cfn) = stick - MIXSRC_FIRST_STICK + 1;
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
    } else {
      uint32_t sensor = yaml_str2uint(val, l_sep);
      CFN_PARAM(cfn) = sensor + FUNC_RESET_PARAM_FIRST_TELEM;
    }
    break;
      
  case FUNC_VOLUME:
  case FUNC_PLAY_VALUE:
    // find "," and cut val_len
    CFN_PARAM(cfn) = r_mixSrcRaw(nullptr, val, l_sep);
    break;

  case FUNC_PLAY_SOUND:
    // find "," and cut val_len
    for (int i=0; i < yaml_conv_220::_func_sound_lookup_size; i++) {
      if (!strncmp(yaml_conv_220::_func_sound_lookup[i],val,l_sep)) {
        CFN_PARAM(cfn) = i;
        break;
      }
    }
    break;

  case FUNC_PLAY_TRACK:
  case FUNC_BACKGND_MUSIC:
  case FUNC_PLAY_SCRIPT:
    strncpy(cfn->play.name, val, std::min<uint8_t>(l_sep, LEN_FUNCTION_NAME));
    break;

  case FUNC_SET_TIMER:
    // Tmr1,Tmr2,Tmr3
    if (l_sep >= 4
        && val[0] == 'T'
        && val[1] == 'm'
        && val[2] == 'r'
        && val[3] >= '1'
        && val[3] <= '3') {

      CFN_TIMER_INDEX(cfn) = val[3] - '1';

      val += 4; val_len -= 4;
      if (val_len == 0 || val[0] != ',') return;
      val++; val_len--;

      CFN_PARAM(cfn) = yaml_str2uint_ref(val, val_len);
      if (val_len == 0 || val[0] != ',') return;
      val++; val_len--;

      eat_comma = false;
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

  case FUNC_HAPTIC:
  case FUNC_LOGS: // 10th of seconds
    CFN_PARAM(cfn) = yaml_str2uint(val, l_sep);
    break;

  case FUNC_ADJUST_GVAR: {

    CFN_GVAR_INDEX(cfn) = yaml_str2int_ref(val, l_sep);
    if (val_len == 0 || val[0] != ',') return;
    val++; val_len--;

    // find "," and cut val_len
    sep = (const char *)memchr(val, ',', val_len);
    l_sep = sep ? sep - val : val_len;

    // parse CFN_GVAR_MODE
    for (int i=0; i < yaml_conv_220::_adjust_gvar_mode_lookup_size; i++) {
      if (!strncmp(yaml_conv_220::_adjust_gvar_mode_lookup[i],val,l_sep)) {
        CFN_GVAR_MODE(cfn) = i;
        break;
      }
    }

    val += l_sep; val_len -= l_sep;
    if (val_len == 0 || val[0] != ',') return;
    val++; val_len--;
    // find "," and cut val_len
    sep = (const char *)memchr(val, ',', val_len);
    l_sep = sep ? sep - val : val_len;

    // output param
    switch(CFN_GVAR_MODE(cfn)) {
    case FUNC_ADJUST_GVAR_CONSTANT:
    case FUNC_ADJUST_GVAR_INCDEC:
      CFN_PARAM(cfn) = yaml_str2int(val, l_sep);
      break;
    case FUNC_ADJUST_GVAR_SOURCE:
      CFN_PARAM(cfn) = r_mixSrcRaw(nullptr, val, l_sep);
      break;
    case FUNC_ADJUST_GVAR_GVAR: {
      uint32_t gvar = r_mixSrcRaw(nullptr, val, l_sep);
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

  if (eat_comma) {
    val += l_sep;
    val_len -= l_sep;

    if (val_len == 0 || val[0] != ',')
      return;

    val++; val_len--;
  }

  if (HAS_ENABLE_PARAM(func)) {
    // "0/1"
    if (val_len > 0) {
      if (val[0] == '0') {
        CFN_ACTIVE(cfn) = 0;
      } else if (val[0] == '1') {
        CFN_ACTIVE(cfn) = 1;
      }
    }
  } else if (HAS_REPEAT_PARAM(func)) {
    if (val_len == 2
        && val[0] == '1'
        && val[1] == 'x') {
      CFN_PLAY_REPEAT(cfn) = 0;
    } else if (val_len == 3
        && val[0] == '!'
        && val[1] == '1'
        && val[2] == 'x') {
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
    case NUM_STICKS + 1:
      if (!wf(opaque, "chans", 5)) return false;
      break;
    default:
      if (value > 0 && value < NUM_STICKS + 1) {
        str = yaml_output_enum(value - 1 + MIXSRC_FIRST_STICK, enum_MixSources);
        if (str && !wf(opaque, str, strlen(str))) return false;
      }
    }
  } break;

  case FUNC_RESET:
    if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
      // Tmr1,Tmr2,Tmr3,All
      str = yaml_conv_220::_func_reset_param_lookup[CFN_PARAM(cfn)];
    } else {
      // sensor index
      str = yaml_unsigned2str(CFN_PARAM(cfn) - FUNC_RESET_PARAM_FIRST_TELEM);
    }
    if (!wf(opaque, str, strlen(str))) return false;
    break;
      
  case FUNC_VOLUME:
  case FUNC_PLAY_VALUE:
    if (!w_mixSrcRaw(nullptr, CFN_PARAM(cfn), wf, opaque)) return false;
    break;

  case FUNC_PLAY_SOUND:
    // Bp1,Bp2,Bp3,Wrn1,Wrn2,Chee,Rata,Tick,Sirn,Ring,SciF,Robt,Chrp,Tada,Crck,Alrm
    str = yaml_conv_220::_func_sound_lookup[CFN_PARAM(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_PLAY_TRACK:
  case FUNC_BACKGND_MUSIC:
  case FUNC_PLAY_SCRIPT:
    if (!wf(opaque, cfn->play.name, strnlen(cfn->play.name, LEN_FUNCTION_NAME)))
      return false;
    break;

  case FUNC_SET_TIMER:
    // Tmr1,Tmr2,Tmr3
    str = yaml_conv_220::_func_reset_param_lookup[CFN_TIMER_INDEX(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(CFN_PARAM(cfn));
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_SET_FAILSAFE:
    // Int,Ext
    str = yaml_conv_220::_func_failsafe_lookup[CFN_PARAM(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    break;

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
    str = yaml_conv_220::_adjust_gvar_mode_lookup[CFN_GVAR_MODE(cfn)];
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
      if (!w_mixSrcRaw(nullptr, CFN_PARAM(cfn), wf, opaque)) return false;
      break;
    case FUNC_ADJUST_GVAR_GVAR:
      if (!w_mixSrcRaw(nullptr, CFN_PARAM(cfn) + MIXSRC_FIRST_GVAR, wf, opaque)) return false;
      break;
    }
    break;

  default:
    add_comma = false;
    break;
  }

  if (HAS_ENABLE_PARAM(func)) {
    if (add_comma) {
      // ","
      if (!wf(opaque,",",1)) return false;
    }
    // "0/1"
    if (!wf(opaque,CFN_ACTIVE(cfn) ? "1":"0",1)) return false;
  } else if (HAS_REPEAT_PARAM(func)) {
    if (add_comma) {
      // ","
      if (!wf(opaque,",",1)) return false;
    }
    if (CFN_PLAY_REPEAT(cfn) == 0) {
      // "1x"
      if (!wf(opaque,"1x",2)) return false;
    } else if (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART) {
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
  } else if (600) {
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
  const char* sep = (const char *)memchr(val, ',', val_len);
  uint8_t l_sep = sep ? sep - val : val_len;

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
    ls->v1 = r_mixSrcRaw(nullptr, val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = r_mixSrcRaw(nullptr, val, val_len);
    break;
    
  case LS_FAMILY_TIMER:
    ls->v1 = timerValue2lsw(yaml_str2uint(val, l_sep));
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    ls->v2 = timerValue2lsw(yaml_str2uint(val, val_len));
    break;
    
  default:
    ls->v1 = r_mixSrcRaw(nullptr, val, l_sep);
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
    if (!w_mixSrcRaw(nullptr, ls->v1, wf, opaque)) return false;
    if (!wf(opaque,",",1)) return false;
    if (!w_mixSrcRaw(nullptr, ls->v2, wf, opaque)) return false;
    break;
    
  case LS_FAMILY_TIMER:
    str = yaml_unsigned2str(lswTimerValue(ls->v1));
    if (!wf(opaque,str,strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(lswTimerValue(ls->v2));
    if (!wf(opaque,str,strlen(str))) return false;
    break;
    
  default:
    if (!w_mixSrcRaw(nullptr, ls->v1, wf, opaque)) return false;
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

static void r_modSubtype(void* user, uint8_t* data, uint32_t bitoffs,
                         const char* val, uint8_t val_len)
{
  data += bitoffs >> 3UL;
  data -= offsetof(ModuleData, channelsStart);

  auto md = reinterpret_cast<ModuleData*>(data);
  if (isModuleTypeXJT(md->type)) {
    md->subType = yaml_parse_enum(yaml_conv_220::enum_XJT_Subtypes, val, val_len);
  } else if (isModuleTypeISRM(md->type)) {
    md->subType = yaml_parse_enum(yaml_conv_220::enum_ISRM_Subtypes, val, val_len);
  } else if (isModuleTypeR9MNonAccess(md->type)) {
    md->subType = yaml_parse_enum(yaml_conv_220::enum_R9M_Subtypes, val, val_len);
  } else if (md->type == MODULE_TYPE_FLYSKY) {
    md->subType = yaml_parse_enum(yaml_conv_220::enum_FLYSKY_Subtypes, val, val_len);
  } else if (md->type == MODULE_TYPE_MULTIMODULE) {
#if defined(MULTIMODULE)
    // Read type/subType by the book (see MPM documentation)
    // TODO: remove that crappy translation and use the MPM
    //       data as-is (no FrSky special casing)

    // read "[type],[subtype]"
    const char* sep = (const char *)memchr(val, ',', val_len);
    uint8_t l_sep = sep ? sep - val : val_len;

    int type = yaml_str2uint(val, l_sep);
    val += l_sep; val_len -= l_sep;
    if (!val_len || val[0] != ',') return;
    val++; val_len--;
    int subtype = yaml_str2uint(val, val_len);

    // convert to ETX format and write to vars
    convertMultiProtocolToEtx(&type, &subtype);
    if (type > 0) {
      md->multi.rfProtocol = type - 1;
      md->subType = subtype;
    }
#endif
  } else if (md->type == MODULE_TYPE_DSM2) {
    md->subType = yaml_parse_enum(yaml_conv_220::enum_DSM2_Subtypes, val, val_len);
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
    str = yaml_output_enum(val, yaml_conv_220::enum_XJT_Subtypes);
  } else if (md->type == MODULE_TYPE_ISRM_PXX2) {
    str = yaml_output_enum(val, yaml_conv_220::enum_ISRM_Subtypes);
  } else if (md->type == MODULE_TYPE_R9M_PXX1 || md->type == MODULE_TYPE_R9M_LITE_PXX1) {
    str = yaml_output_enum(val, yaml_conv_220::enum_R9M_Subtypes);
  } else if (md->type == MODULE_TYPE_FLYSKY) {
    str = yaml_output_enum(val, yaml_conv_220::enum_FLYSKY_Subtypes);
  } else if (md->type == MODULE_TYPE_MULTIMODULE) {
#if defined(MULTIMODULE)
    // Use type/subType by the book (see MPM documentation)
    // TODO: remove that crappy translation and use the MPM
    //       data as-is (no FrSky special casing)
    int type = md->multi.rfProtocol + 1;
    int subtype = val;
    convertEtxProtocolToMulti(&type, &subtype);

    // output "[type],[subtype]"
    str = yaml_unsigned2str(type);
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque, ",", 1)) return false;
    str = yaml_unsigned2str(subtype);
#endif
  } else if (md->type == MODULE_TYPE_DSM2) {
    str = yaml_output_enum(md->subType, yaml_conv_220::enum_DSM2_Subtypes);
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
  return yaml_conv_220::w_channelsCount(node, val, wf, opaque);
}

static void r_jitterFilter(void* user, uint8_t* data, uint32_t bitoffs,
                           const char* val, uint8_t val_len)
{
  uint32_t i = yaml_str2uint(val, val_len);
  yaml_put_bits(data, i, bitoffs, 1);
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

#if STORAGE_CONVERSIONS >= 221
const struct YamlIdStr _old_enum_UartModes[] = {
  {  UART_MODE_NONE, "MODE_NONE"  },
  {  UART_MODE_TELEMETRY_MIRROR, "MODE_TELEMETRY_MIRROR"  },
  {  UART_MODE_TELEMETRY, "MODE_TELEMETRY"  },
  {  UART_MODE_SBUS_TRAINER, "MODE_SBUS_TRAINER"  },
  {  UART_MODE_LUA, "MODE_LUA"  },
  {  0, NULL  }
};
#endif

static const struct YamlIdStr enum_UartModes[] = {
  {  UART_MODE_NONE, "NONE"  },
  {  UART_MODE_TELEMETRY_MIRROR, "TELEMETRY_MIRROR"  },
  {  UART_MODE_TELEMETRY, "TELEMETRY_IN"  },
  {  UART_MODE_SBUS_TRAINER, "SBUS_TRAINER"  },
  {  UART_MODE_LUA, "LUA"  },
  {  UART_MODE_CLI, "CLI"  },
  {  UART_MODE_GPS, "GPS"  },
  {  UART_MODE_DEBUG, "DEBUG"  },
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
  if (!node || node->tag_len < 4) return;

  uint8_t port_nr;
  if (node->tag[3] == 'S')
    port_nr = SP_AUX1;
  else if (node->tag[3] == '2')
    port_nr = SP_AUX2;
  else
    return;

#if STORAGE_CONVERSIONS < 221
  auto m = yaml_parse_enum(yaml_conv_220::enum_UartModes, val, val_len);
#else
  auto m = yaml_parse_enum(_old_enum_UartModes, val, val_len);
#endif
  if (!m) return;
  
  auto serialPort = reinterpret_cast<uint32_t*>(data);
  *serialPort = (*serialPort & ~(0xF << port_nr * SERIAL_CONF_BITS_PER_PORT)) |
                (m << port_nr * SERIAL_CONF_BITS_PER_PORT);
}
