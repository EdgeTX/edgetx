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

// Use definitions from v220 conversions as long as nothing changes

namespace yaml_conv_220 {
  bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_mod_type(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_custom_fn(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_script_input(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs);
  uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs);
  bool sw_write(void* user, yaml_writer_func wf, void* opaque);
  bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool cfn_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs);
  bool w_beeperMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
  bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

  extern const struct YamlIdStr enum_TrainerMode[];
  bool w_trainerMode(const YamlNode* node, uint32_t val,
                     yaml_writer_func wf, void* opaque);

  extern const char* _tele_screen_type_lookup[];
  bool w_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                          yaml_writer_func wf, void* opaque);
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
static_assert(sizeof(ModuleData) == 29,"");
#if defined(PCBHORUS) || defined(PCBNV14)
static_assert(sizeof(FlightModeData) == 44, "");
static_assert(sizeof(CustomFunctionData) == 9,"");
#elif defined(PCBX7) || defined(PCBXLITE) || defined(PCBX9LITE)
static_assert(sizeof(FlightModeData) == 36, "");
static_assert(sizeof(CustomFunctionData) == 11,"");
static_assert(sizeof(TelemetryScreenData) == 24, "");
#else
static_assert(sizeof(FlightModeData) == 40, "");
static_assert(sizeof(CustomFunctionData) == 11,"");
static_assert(sizeof(TelemetryScreenData) == 24, "");
#endif

#define GVAR_SMALL 128

static uint32_t in_read_weight(const YamlNode* node, const char* val, uint8_t val_len)
{
  if ((val_len == 4)
      && (val[0] == '-')
      && (val[1] == 'G')
      && (val[2] == 'V')
      && (val[3] >= '1')
      && (val[3] <= '9')) {

    TRACE("%.*s -> %i\n", val_len, val, GVAR_SMALL - (val[3] - '0'));
    return GVAR_SMALL - (val[3] - '0');  // -GVx => 128 - x
  }

  if ((val_len == 3)
      && (val[0] == 'G')
      && (val[1] == 'V')
      && (val[2] >= '1')
      && (val[2] <= '9')) {

    TRACE("%.*s -> %i\n", val_len, val, -GVAR_SMALL + (val[2] - '1'));
    return -GVAR_SMALL + (val[2] - '1');  //  GVx => -128 + (x-1)
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
      uint8_t script = yaml_str2uint(val, val_len);

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
      return yaml_str2uint(val, val_len) + MIXSRC_FIRST_LOGICAL_SWITCH;

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
      return yaml_str2uint_ref(val, val_len) + MIXSRC_FIRST_CH;
      
    } else if (val_len > 3 &&
               val[0] == 'g' &&
               val[1] == 'v' &&
               val[2] == '(') {

      val += 3; val_len -= 3;
      // parse int and ignore closing ')'
      return yaml_str2uint_ref(val, val_len) + MIXSRC_FIRST_GVAR;

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
      return yaml_str2uint_ref(val, val_len) * 3 + sign + MIXSRC_FIRST_TELEM;
    }

    return yaml_parse_enum(enum_MixSources, val, val_len);
}

static bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_mixSrcRaw(node, val, wf, opaque);
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
#endif

static uint8_t select_mod_type(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_mod_type(user, data, bitoffs);
}

static uint8_t select_custom_fn(void* user, uint8_t* data, uint32_t bitoffs)
{
  return yaml_conv_220::select_custom_fn(user, data, bitoffs);
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

static uint32_t sw_read(void* user, const char* val, uint8_t val_len)
{
  (void)user;
  uint32_t sw = yaml_parse_enum(enum_MixSources, val, val_len);
  if (sw >= MIXSRC_FIRST_SWITCH) return sw - MIXSRC_FIRST_SWITCH;

  return -1;
}

bool sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::sw_write(user, wf, opaque);
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
  return wf(opaque, rd->anaNames[idx],
            strnlen(rd->anaNames[idx], LEN_ANA_NAME));
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
  return wf(opaque, str, strnlen(str, LEN_SWITCH_NAME));
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
  return wf(opaque, str, strnlen(str, LEN_ANA_NAME));
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
  return wf(opaque, str, strnlen(str, LEN_ANA_NAME));
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

static bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  return yaml_conv_220::w_swtchSrc(node, val, wf, opaque);
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

static uint32_t r_swtchWarn(const YamlNode* node, const char* val, uint8_t val_len)
{
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
    while(val_len--) {

        signed swtch = *(val++) - 'A';
        if (swtch >= NUM_SWITCHES)
            break;

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
        swtchWarn |= (state << (3*swtch));
    }
    
    return swtchWarn;
}

static bool w_swtchWarn(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    for (int i = 0; i < NUM_SWITCHES; i++) {
        if (SWITCH_EXISTS(i)) {

            // decode check state
            // -> 3 bits per switch
            auto state = (val >> (3*i)) & 0x07;

            // state == 0 -> no check
            // state == 1 -> UP
            // state == 2 -> MIDDLE
            // state == 3 -> DOWN
            char swtchWarn[2] = {(char)('A' + i), 0};

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
  return yaml_conv_220::w_beeperMode(node, val, wf, opaque);
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

const struct YamlIdStr enum_TrainerMode[] = {
#if defined(PCBNV14)
  {  TRAINER_MODE_OFF, "OFF"  },
#endif
  {  TRAINER_MODE_MASTER_TRAINER_JACK, "MASTER_TRAINER_JACK"  },
  {  TRAINER_MODE_SLAVE, "SLAVE"  },
#if defined(PCBTARANIS) || defined(PCBNV14)
  {  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE, "MASTER_SBUS_EXT"  },
  {  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE, "MASTER_CPPM_EXT"  },
#endif
#if defined(PCBTARANIS) || defined(AUX_SERIAL) || defined(AUX2_SERIAL)
  {  TRAINER_MODE_MASTER_BATTERY_COMPARTMENT, "MASTER_BATT_COMP"  },
#endif
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
  return yaml_conv_220::w_trainerMode(node, val, wf, opaque);
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
