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

#define in_read_weight nullptr
#define r_mixSrcRaw nullptr

extern const struct YamlIdStr enum_MixSources[];
extern const struct YamlIdStr enum_SwitchSources[];

static constexpr char closing_parenthesis[] = ")";

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
#define r_vbat_max nullptr

#if defined(COLORLCD)

#define r_zov_source nullptr
bool w_zov_source(void* user, uint8_t* data, uint32_t bitoffs,
                  yaml_writer_func wf, void* opaque)
{
  data += bitoffs >> 3UL;
  auto p_val = reinterpret_cast<ZoneOptionValue*>(data);
  return w_mixSrcRaw(nullptr, p_val->unsignedValue, wf, opaque);
}

//#include "colors.h"
#define r_zov_color nullptr

#endif

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
  return wf(opaque, rd->anaNames[idx],
            strnlen(rd->anaNames[idx], LEN_ANA_NAME));
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
  return wf(opaque, str, strnlen(str, LEN_SWITCH_NAME));
}

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
  return wf(opaque, str, strnlen(str, LEN_ANA_NAME));
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

#define r_beeperMode nullptr
#define r_5pos nullptr
#define r_vol nullptr
#define r_spPitch nullptr
#define r_vPitch nullptr
#define r_trainerMode nullptr

#if !defined(COLORLCD)
#define r_tele_screen_type nullptr

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
#define r_flightModes nullptr
#define r_customFn nullptr

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
    str = ::_func_reset_param_lookup[CFN_TIMER_INDEX(cfn)];
    if (!wf(opaque, str, strlen(str))) return false;
    if (!wf(opaque,",",1)) return false;
    str = yaml_unsigned2str(CFN_PARAM(cfn));
    if (!wf(opaque, str, strlen(str))) return false;
    break;

  case FUNC_SET_FAILSAFE:
    // Int,Ext
    str = ::_func_failsafe_lookup[CFN_PARAM(cfn)];
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

bool w_thrSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
  auto src = throttleSource2Source(val);
  return w_mixSrcRaw(nullptr, src, wf, opaque);
}

#define r_modSubtype nullptr
#define r_channelsCount nullptr
