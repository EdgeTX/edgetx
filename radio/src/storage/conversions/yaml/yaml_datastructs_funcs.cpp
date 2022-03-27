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

#include "dataconstants_220.h"
#include "datastructs_220.h"
#include "myeeprom_220.h"

#define VOLUME_LEVEL_DEF 12

bool w_board(void* user, uint8_t* data, uint32_t bitoffs,
             yaml_writer_func wf, void* opaque)
{
  return wf(opaque, FLAVOUR, sizeof(FLAVOUR)-1);
}

#define in_read_weight nullptr

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

extern const struct YamlIdStr enum_MixSources[];

#define r_mixSrcRaw nullptr

static constexpr char closing_parenthesis[] = ")";

bool output_source_1_param(const char* src_prefix, size_t src_len, uint32_t n,
                           yaml_writer_func wf, void* opaque)
{
  if (!wf(opaque, src_prefix, src_len)) return false;
  const char* str = yaml_unsigned2str(n);
  if (!wf(opaque, str, strlen(str))) return false;
  return true;
}

bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
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

        if (!output_source_1_param("lua(", 4, script, wf, opaque)) return false;
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
        if (!output_source_1_param("ls(", 3, val + 1, wf, opaque)) return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_TRAINER
             && val <= MIXSRC_LAST_TRAINER) {

        val -= MIXSRC_FIRST_TRAINER;
        if (!output_source_1_param("tr(", 3, val, wf, opaque)) return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_CH
             && val <= MIXSRC_LAST_CH) {

        val -= MIXSRC_FIRST_CH;
        if (!output_source_1_param("ch(", 3, val, wf, opaque)) return false;
        str = closing_parenthesis;
    }
    else if (val >= MIXSRC_FIRST_GVAR
             && val <= MIXSRC_LAST_GVAR) {

        val -= MIXSRC_FIRST_GVAR;
        if (!output_source_1_param("gv(", 3, val, wf, opaque)) return false;
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

#define r_vbat_min nullptr

bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 90);
  return wf(opaque, s, strlen(s));
}

#define r_vbat_max nullptr

bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 120);
  return wf(opaque, s, strlen(s));
}

#if defined(COLORLCD)
uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= offsetof(ZoneOptionValueTyped, value);
    ZoneOptionValueEnum* p_zovt = (ZoneOptionValueEnum*)data;
    if (*p_zovt > ZOV_Color) return 0;
    return *p_zovt;
}

#define r_zov_source nullptr

bool w_zov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);
  return w_mixSrcRaw(nullptr, p_val->unsignedValue, wf, opaque);
}

#include "colors.h"
#define r_zov_color nullptr

bool w_zov_color(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);

  uint32_t color = (uint32_t)GET_RED(p_val->unsignedValue) << 16 |
                   (uint32_t)GET_GREEN(p_val->unsignedValue) << 8 |
                   (uint32_t)GET_BLUE(p_val->unsignedValue);

  if (!wf(opaque, "0x", 2)) return false;
  return wf(opaque, yaml_rgb2hex(color), 3 * 2);
}
#endif

static uint8_t select_mod_type(void* user, uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= offsetof(ModuleData, ppm);

    ModuleData* mod_data = reinterpret_cast<ModuleData*>(data);
    switch(mod_data->type) {
    case MODULE_TYPE_NONE:
    case MODULE_TYPE_PPM:
    case MODULE_TYPE_DSM2:
    case MODULE_TYPE_CROSSFIRE:
    case MODULE_TYPE_GHOST:
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
        if (mod_data->subType == FLYSKY_SUBTYPE_AFHDS2A)
            return 6;
        if (mod_data->subType == FLYSKY_SUBTYPE_AFHDS3)
            return 7;
    }
    return 0;
}

uint8_t select_script_input(void* user, uint8_t* data, uint32_t bitoffs)
{
    // always use 'value'
    return 0;
}

uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->type == TELEM_TYPE_CALCULATED
      && sensor->persistent)
    return 1;

  return 0;
}

uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs)
{
  data += bitoffs >> 3UL;
  data -= 2 /* size of id1 union */;
  const TelemetrySensor* sensor = (const TelemetrySensor*)data;

  if (sensor->type == TELEM_TYPE_CALCULATED)
    return 2; // formula
  
  return 1; // instance
}

uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs)
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

#define r_calib nullptr

static bool w_calib(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str =
      yaml_output_enum(idx + MIXSRC_Rud, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

bool sw_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str =
      yaml_output_enum(idx + MIXSRC_FIRST_SWITCH, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

bool w_stick_name(void* user, uint8_t* data, uint32_t bitoffs,
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

bool stick_name_valid(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();
  RadioData* rd = reinterpret_cast<RadioData*>(data);
  return rd->anaNames[idx][0] != '\0';
}

static const struct YamlNode struct_sticksConfig[] = {
    YAML_IDX,
    YAML_CUSTOM( "name", nullptr, w_stick_name),
    YAML_END
};

bool sw_name_write(void* user, uint8_t* data, uint32_t bitoffs,
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

extern const struct YamlIdStr enum_SwitchConfig[];
const struct YamlIdStr enum_SwitchConfig[] = {
    {  SWITCH_NONE, "none"  },
    {  SWITCH_TOGGLE, "toggle"  },
    {  SWITCH_2POS, "2pos"  },
    {  SWITCH_3POS, "3pos"  },
    {  0, NULL  }
};

static const struct YamlNode struct_switchConfig[] = {
    YAML_IDX_CUST( "sw", nullptr, sw_write ),
    YAML_ENUM( "type", 2, enum_SwitchConfig),
    YAML_CUSTOM( "name", nullptr, sw_name_write),
    YAML_END
};

bool pot_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str = yaml_output_enum(idx + MIXSRC_FIRST_POT, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

bool pot_name_write(void* user, uint8_t* data, uint32_t bitoffs,
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
    YAML_IDX_CUST( "pot", nullptr, pot_write ),
    YAML_ENUM( "type", 2, enum_PotConfig),
    YAML_CUSTOM( "name", nullptr, pot_name_write),
    YAML_END
};

bool slider_write(void* user, yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts();

  const char* str =
      yaml_output_enum(idx + MIXSRC_FIRST_SLIDER, enum_MixSources);
  return str ? wf(opaque, str, strlen(str)) : true;
}

bool sl_name_write(void* user, uint8_t* data, uint32_t bitoffs,
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
    YAML_IDX_CUST( "sl", nullptr, slider_write ),
    YAML_ENUM( "type", 1, enum_SliderConfig),
    YAML_CUSTOM( "name", nullptr, sl_name_write),
    YAML_END
};

extern const struct YamlIdStr enum_SwitchSources[];

#define r_swtchSrc nullptr

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

static bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
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

bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  // TODO: no need to output 0 values for FM0
  gvar_t* gvar = (gvar_t*)(data + (bitoffs>>3UL));
  return *gvar != GVAR_MAX+1;
}

bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs)
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
  for (uint8_t i=0; i<MAX_GVARS; i++) {
    is_active |= fmd->gvars[i] != GVAR_MAX+1; // FM0 -> default
  }

  return is_active;
}

bool swash_is_active(void* user, uint8_t* data, uint32_t bitoffs)
{
  auto swashR = reinterpret_cast<SwashRingData*>(data + (bitoffs >> 3UL));
  return swashR->type | swashR->value;
}

#define r_swtchWarn nullptr

#if defined(COLORLCD)
bool w_swtchWarn(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    for (int i = 0; i < STORAGE_NUM_SWITCHES; i++) {
        //if (SWITCH_EXISTS(i)) {

            // decode check state
            // -> 3 bits per switch
            auto state = (val >> (3*i)) & 0x07;

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
        //}
    }

    return true;
}
#else
bool w_swtchWarn(void* user, uint8_t* data, uint32_t bitoffs,
                 yaml_writer_func wf, void* opaque)
{
  data += (bitoffs >> 3UL);

  // switchWarningState
  swarnstate_t states = *(swarnstate_t*)data;
  data += sizeof(swarnstate_t);

  // switchWarningEnable
  swarnenable_t enables = *(swarnenable_t*)data;

  for (int i = 0; i < STORAGE_NUM_SWITCHES; i++) {
    // decode check state
    // -> 2 bits per switch + enable
    swarnenable_t en = (enables >> i) & 0x01;
    if (en) continue;

    // state == 0 -> no check
    // state == 1 -> UP
    // state == 2 -> MIDDLE
    // state == 3 -> DOWN
    char swtchWarn[2] = {(char)('A' + i), 0};

    uint8_t state = (states >> (2 * i)) & 0x03;
    switch (state) {
      case 0:
        swtchWarn[1] = 'u';
        break;
      case 1:
        swtchWarn[1] = '-';
        break;
      case 2:
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

  return true;
}
#endif

extern const struct YamlIdStr enum_BeeperMode[];

#define r_beeperMode nullptr

bool w_beeperMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    const char* str = yaml_output_enum(sval, enum_BeeperMode);
    return wf(opaque, str, strlen(str));
}

#define r_5pos nullptr

bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    char* s = yaml_signed2str(sval + 2);
    return wf(opaque, s, strlen(s));
}

#define r_vol nullptr

bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    char* s = yaml_signed2str(sval + VOLUME_LEVEL_DEF);
    return wf(opaque, s, strlen(s));
}

#define r_spPitch nullptr

bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    char* s = yaml_signed2str(val * 15);
    return wf(opaque, s, strlen(s));
}

#define r_vPitch nullptr

bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    char* s = yaml_signed2str(sval * 10);
    return wf(opaque, s, strlen(s));
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

#define r_trainerMode nullptr

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
#define r_tele_screen_type nullptr

const char* _tele_screen_type_lookup[] = {
  "NONE",
  "VALUES",
  "BARS",
  "SCRIPT",
};

bool w_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs,
                        yaml_writer_func wf, void* opaque)
{
  auto tw = reinterpret_cast<YamlTreeWalker*>(user);
  uint16_t idx = tw->getElmts(1);

  data -= sizeof(TelemetryScreenData) * idx + 1;
  uint8_t type = ((*data) >> (2 * idx)) & 0x03;

  const char* str = _tele_screen_type_lookup[type];
  return wf(opaque, str, strlen(str));
}

uint8_t select_tele_screen_data(void* user, uint8_t* data, uint32_t bitoffs)
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

#define r_tele_sensor nullptr

bool w_tele_sensor(const YamlNode* node, uint32_t val,
                   yaml_writer_func wf, void* opaque)
{
  if (!val) {
    return wf(opaque, "none", 4);
  }
  
  const char* str = yaml_unsigned2str(val-1);  
  return wf(opaque, str, strlen(str));
}

#define r_flightModes nullptr

bool w_flightModes(const YamlNode* node, uint32_t val,
                   yaml_writer_func wf, void* opaque)
{
  for (uint32_t i = 0; i < node->size; i++) {
    uint32_t bit = (val >> i) & 1;
    if (!wf(opaque, bit ? "1" : "0", 1)) return false;
  }
  return true;
}

#define r_customFn nullptr

extern const char* _func_reset_param_lookup[];
const char* _func_reset_param_lookup[] = {
  "Tmr1","Tmr2","Tmr3","All","Tele"
};

extern const char* _func_failsafe_lookup[];
const char* _func_failsafe_lookup[] = {
  "Int","Ext"
};

// used in read routine as well
extern const char* _func_sound_lookup[];
const char* _func_sound_lookup[] = {
  "Bp1","Bp2","Bp3","Wrn1","Wrn2",
  "Chee","Rata","Tick","Sirn","Ring",
  "SciF","Robt","Chrp","Tada","Crck","Alrm"
};
extern const uint8_t _func_sound_lookup_size = 16;

// force external linkage
extern const char* _adjust_gvar_mode_lookup[];
const char* _adjust_gvar_mode_lookup[] = {
  "Cst", "Src", "GVar", "IncDec"
};
extern const uint8_t _adjust_gvar_mode_lookup_size = 4;
  
bool w_customFn(void* user, uint8_t* data, uint32_t bitoffs,
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
    if (!w_mixSrcRaw(nullptr, CFN_PARAM(cfn), wf, opaque)) return false;
    break;

  case FUNC_PLAY_SOUND:
    // Bp1,Bp2,Bp3,Wrn1,Wrn2,Chee,Rata,Tick,Sirn,Ring,SciF,Robt,Chrp,Tada,Crck,Alrm
    str = _func_sound_lookup[CFN_PARAM(cfn)];
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

#define r_logicSw nullptr

static const struct YamlNode _ls_node_v1 = YAML_PADDING(10);
static const struct YamlNode _ls_node_v2 = YAML_PADDING(16);

bool w_logicSw(void* user, uint8_t* data, uint32_t bitoffs,
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

#define r_thrSrc nullptr

int16_t throttleSource2Source_v220(int16_t thrSrc)
{
  if (thrSrc == 0) return (int16_t)MIXSRC_Thr;
  if (--thrSrc < NUM_POTS + NUM_SLIDERS)
    return (int16_t)(thrSrc + MIXSRC_FIRST_POT);
  return (int16_t)(thrSrc - (NUM_POTS + NUM_SLIDERS) + MIXSRC_FIRST_CH);
}

static bool w_thrSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  auto src = throttleSource2Source_v220(val);
  return w_mixSrcRaw(nullptr, src, wf, opaque);
}

// Force external linkage
extern const struct YamlIdStr enum_XJT_Subtypes[];
extern const struct YamlIdStr enum_ISRM_Subtypes[];
extern const struct YamlIdStr enum_R9M_Subtypes[];
extern const struct YamlIdStr enum_FLYSKY_Subtypes[];
extern const struct YamlIdStr enum_DSM2_Subtypes[];

const struct YamlIdStr enum_XJT_Subtypes[] = {
  { MODULE_SUBTYPE_PXX1_ACCST_D16, "D16" },
  { MODULE_SUBTYPE_PXX1_ACCST_D8, "D8" },
  { MODULE_SUBTYPE_PXX1_ACCST_LR12, "LR12" },
  { 0, NULL  }
};

const struct YamlIdStr enum_ISRM_Subtypes[] = {
  { MODULE_SUBTYPE_ISRM_PXX2_ACCESS, "ACCESS" },
  { MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16, "D16" },
  // unused !!!
  // { MODULE_SUBTYPE_ISRM_PXX2_ACCST_LR12, "LR12" },
  // { MODULE_SUBTYPE_ISRM_PXX2_ACCST_D8, "D8" },
  { 0, NULL  }
};

const struct YamlIdStr enum_R9M_Subtypes[] = {
  { MODULE_SUBTYPE_R9M_FCC, "FCC" },
  { MODULE_SUBTYPE_R9M_EU, "EU" },
  { MODULE_SUBTYPE_R9M_EUPLUS, "EUPLUS" },
  { MODULE_SUBTYPE_R9M_AUPLUS, "AUPLUS" },
  { 0, NULL  }
};

const struct YamlIdStr enum_FLYSKY_Subtypes[] = {
  { FLYSKY_SUBTYPE_AFHDS3, "AFHDS3" },
  { FLYSKY_SUBTYPE_AFHDS2A, "AFHDS2A" },
  { 0, NULL  }
};

const struct YamlIdStr enum_DSM2_Subtypes[] = {
  { 0, "LP45" },
  { 1, "DSM2" },
  { 2, "DSMX" },
  { 0, NULL  }
};

#define r_modSubtype nullptr

bool w_modSubtype(void* user, uint8_t* data, uint32_t bitoffs,
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
  } else if (md->type == MODULE_TYPE_FLYSKY) {
    str = yaml_output_enum(val, enum_FLYSKY_Subtypes);
  } else if (md->type == MODULE_TYPE_MULTIMODULE) {
#if defined(MULTIMODULE)
    // Use type/subType by the book (see MPM documentation)
    // TODO: remove that crappy translation and use the MPM
    //       data as-is (no FrSky special casing)
    int type = md->getMultiProtocol() + 1;
    int subtype = val;
    convertEtxProtocolToMulti(&type, &subtype);

    // output "[type],[subtype]"
    str = yaml_unsigned2str(type);
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque, ",", 1)) return false;
    str = yaml_unsigned2str(subtype);
#endif
  } else if (md->type == MODULE_TYPE_DSM2) {
    str = yaml_output_enum(md->rfProtocol, enum_DSM2_Subtypes);
  } else {
    str = yaml_unsigned2str(val);
  }

  if (str && !wf(opaque, str, strlen(str)))
    return false;

  return true;
}

#define r_channelsCount nullptr

bool w_channelsCount(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  // offset 8
  int32_t sval = yaml_to_signed(val, node->size) + 8;  
  const char* str = yaml_signed2str(sval);
  return wf(opaque,str,strlen(str));
}

// force storage class
extern const struct YamlIdStr enum_UartModes[];
