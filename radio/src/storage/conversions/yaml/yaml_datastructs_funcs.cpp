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

#define GVAR_SMALL 128
#define VOLUME_LEVEL_DEF 12

#define in_read_weight nullptr

static bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    
    if (sval > GVAR_SMALL-11 && sval < GVAR_SMALL-1) {
        char n = GVAR_SMALL - sval + '0';
        return wf(opaque, "-GV", 3) && wf(opaque, &n, 1);
    }
    else if (sval < -GVAR_SMALL+11 && sval > -GVAR_SMALL+1) {
        char n = val - GVAR_SMALL + '1';
        return wf(opaque, "GV", 2) && wf(opaque, &n, 1);
    }

    char* s = yaml_signed2str(sval);
    return wf(opaque, s, strlen(s));
}

extern const struct YamlIdStr enum_MixSources[];

#define r_mixSrcRaw nullptr

static bool w_mixSrcRaw(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    const char* str = nullptr;

    if (val >= MIXSRC_FIRST_INPUT
        && val <= MIXSRC_LAST_INPUT) {

        if (!wf(opaque, "I", 1))
            return false;

        str = yaml_unsigned2str(val - MIXSRC_FIRST_INPUT);
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

static bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 90);
    return wf(opaque, s, strlen(s));
}

#define r_vbat_max nullptr

static bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    char* s = yaml_signed2str(yaml_to_signed(val,node->size) + 120);
    return wf(opaque, s, strlen(s));
}

#if defined(COLORLCD)
static uint8_t select_zov(uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= sizeof(ZoneOptionValueEnum);
    ZoneOptionValueEnum* p_zovt = (ZoneOptionValueEnum*)data;
    if (*p_zovt > ZOV_String)
        return 0;
    return *p_zovt;
}
#endif

static uint8_t select_mod_type(uint8_t* data, uint32_t bitoffs)
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

static uint8_t select_custom_fn(uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= offsetof(CustomFunctionData, all);

    auto cf = reinterpret_cast<CustomFunctionData*>(data);
    switch(cf->func) {
    case FUNC_PLAY_TRACK:
    case FUNC_BACKGND_MUSIC:
    case FUNC_PLAY_SCRIPT:
        // play: 0
        return 0;
    }
    
    // all:  1
    return 1;
}

static uint8_t select_script_input(uint8_t* data, uint32_t bitoffs)
{
    // always use 'value'
    return 0;
}

static uint8_t select_id1(uint8_t* data, uint32_t bitoffs)
{
    // always use 'id'
    return 0;
}

static uint8_t select_id2(uint8_t* data, uint32_t bitoffs)
{
    // always use 'instance'
    return 0;
}

static uint8_t select_sensor_cfg(uint8_t* data, uint32_t bitoffs)
{
    // always use 'param'
    return 5;
}

#if 0
static uint32_t sw_read(const char* val, uint8_t val_len)
{
    uint32_t sw = yaml_parse_enum(enum_MixSources, val, val_len);
    if (sw >= MIXSRC_FIRST_SWITCH)
        return sw - MIXSRC_FIRST_SWITCH;

    return -1;
}
#endif

static bool sw_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_SWITCH, enum_MixSources);
    return str ? wf(opaque, str, strlen(str)) : true;
}

#if 0
static void r_stick_name(const YamlNode* node, uint8_t* data,
                         uint32_t bitoffs, uint16_t idx,
                         const char* val, uint8_t val_len)
{
    data -= offsetof(RadioData, switchConfig);
    RadioData* rd = reinterpret_cast<RadioData*>(data);
    strncpy(rd->anaNames[idx], val, std::min<uint8_t>(val_len, LEN_ANA_NAME));
}
#endif

static bool w_stick_name(const YamlNode* node, uint8_t* data,
                         uint32_t bitoffs, uint16_t idx,
                         yaml_writer_func wf, void* opaque)
{
    data -= offsetof(RadioData, switchConfig);
    RadioData* rd = reinterpret_cast<RadioData*>(data);
    return wf(opaque, rd->anaNames[idx], strnlen(rd->anaNames[idx], LEN_ANA_NAME));
}

static bool stick_name_valid(uint8_t* data, uint32_t bitoffs, uint16_t idx)
{
    RadioData* rd = reinterpret_cast<RadioData*>(data);
    return rd->anaNames[idx][0] != '\0';
}

static const struct YamlNode struct_sticksConfig[] = {
    YAML_IDX,
    YAML_CUSTOM( "name", nullptr, w_stick_name),
    YAML_END
};

#if 0
static void sw_name_read(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                         uint16_t idx, const char* val, uint8_t val_len)
{
    // data / bitoffs already incremented
    data -= ((idx + 1) * 2) / 8;
    data -= offsetof(RadioData, switchConfig);

    RadioData* rd = reinterpret_cast<RadioData*>(data);
    strncpy(rd->switchNames[idx], val, std::min<uint8_t>(val_len, LEN_SWITCH_NAME));
}
#endif

static bool sw_name_write(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                          uint16_t idx, yaml_writer_func wf, void* opaque)
{
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
    YAML_IDX_CUST( "sw", nullptr, sw_write ),
    YAML_ENUM( "type", 2, enum_SwitchConfig),
    YAML_CUSTOM( "name", nullptr, sw_name_write),
    YAML_END
};

#if 0
static uint32_t pot_read(const char* val, uint8_t val_len)
{
    uint32_t pot = yaml_parse_enum(enum_MixSources, val, val_len);
    if (pot >= MIXSRC_FIRST_POT)
        return pot - MIXSRC_FIRST_POT;

    return -1;
}
#endif

static bool pot_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_POT, enum_MixSources);
    return str ? wf(opaque, str, strlen(str)) : true;
}

#if 0
static void pot_name_read(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                          uint16_t idx, const char* val, uint8_t val_len)
{
    // data / bitoffs already incremented
    data -= ((idx + 1) * 2) / 8;
    data -= offsetof(RadioData, potsConfig);

    RadioData* rd = reinterpret_cast<RadioData*>(data);
    idx += NUM_STICKS;
    strncpy(rd->anaNames[idx], val, std::min<uint8_t>(val_len, LEN_ANA_NAME));
}
#endif

static bool pot_name_write(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                           uint16_t idx, yaml_writer_func wf, void* opaque)
{
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

#if 0
static uint32_t slider_read(const char* val, uint8_t val_len)
{
    uint32_t sl = yaml_parse_enum(enum_MixSources, val, val_len);
    if (sl >= MIXSRC_FIRST_SLIDER)
        return sl - MIXSRC_FIRST_SLIDER;

    return -1;
}
#endif

static bool slider_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_SLIDER, enum_MixSources);
    return str ? wf(opaque, str, strlen(str)) : true;
}

#if 0
static void sl_name_read(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                         uint16_t idx, const char* val, uint8_t val_len)
{
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
#endif

static bool sl_name_write(const YamlNode* node, uint8_t* data, uint32_t bitoffs,
                          uint16_t idx, yaml_writer_func wf, void* opaque)
{
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

extern const struct YamlIdStr enum_SwitchSources[];

#define r_swtchSrc nullptr

static bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
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
        str = yaml_unsigned2str(sval - SWSRC_FIRST_FLIGHT_MODE + 1);
        return wf(opaque,str, strlen(str));
    }
    else if (sval >= SWSRC_FIRST_SENSOR
             && sval <= SWSRC_LAST_SENSOR) {

        wf(opaque, "T", 2);
        str = yaml_unsigned2str(sval - SWSRC_FIRST_SENSOR + 1);
        return wf(opaque,str, strlen(str));
    }
    
    str = yaml_output_enum(sval, enum_SwitchSources);
    return wf(opaque, str, strlen(str));
}

static bool cfn_is_active(uint8_t* data, uint32_t bitoffs, uint16_t idx)
{
    data += bitoffs >> 3UL;
    return ((CustomFunctionData*)data)->swtch;
}

static bool gvar_is_active(uint8_t* data, uint32_t bitoffs, uint16_t idx)
{
    gvar_t* gvar = (gvar_t*)(data + (bitoffs>>3UL));
    return *gvar != GVAR_MAX+1;
}

static bool fmd_is_active(uint8_t* data, uint32_t bitoffs, uint16_t idx)
{
    uint32_t data_ofs = bitoffs >> 3UL;
    if (data_ofs == offsetof(ModelData, flightModeData)) {
        return !yaml_is_zero(data, bitoffs, sizeof(FlightModeData)*8);
    }

    bool is_active = !yaml_is_zero(data, bitoffs,
                                   (sizeof(FlightModeData)
                                    - sizeof(FlightModeData::gvars))*8);

    FlightModeData* fmd = (FlightModeData*)(data + data_ofs);
    for (uint8_t i=0; i<MAX_GVARS; i++) {
        is_active |= fmd->gvars[i] != GVAR_MAX+1;
    }

    return is_active;
}

#define r_swtchWarn nullptr

static bool w_swtchWarn(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
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
        //}
    }

    return true;
}

extern const struct YamlIdStr enum_BeeperMode[];

#define r_beeperMode nullptr

static bool w_beeperMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    const char* str = yaml_output_enum(sval, enum_BeeperMode);
    return wf(opaque, str, strlen(str));
}

#define r_5pos nullptr

static bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    char* s = yaml_signed2str(sval + 2);
    return wf(opaque, s, strlen(s));
}

#define r_vol nullptr

static bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val,node->size);
    char* s = yaml_signed2str(sval + VOLUME_LEVEL_DEF);
    return wf(opaque, s, strlen(s));
}

#define r_spPitch nullptr

static bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    char* s = yaml_signed2str(val * 15);
    return wf(opaque, s, strlen(s));
}

#define r_vPitch nullptr

static bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
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
