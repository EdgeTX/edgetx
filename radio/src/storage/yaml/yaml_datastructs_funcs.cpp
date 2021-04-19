#include "opentx.h"
#include "yaml_bits.h"
#include "yaml_tree_walker.h"

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
        return GVAR_SMALL - (val[3] - '0'); // -GVx => 128 - x
    }

    if ((val_len == 3)
        && (val[0] == 'G')
        && (val[1] == 'V')
        && (val[2] >= '1')
        && (val[2] <= '9')) {

        TRACE("%.*s -> %i\n", val_len, val, -GVAR_SMALL + (val[2] - '1'));
        return -GVAR_SMALL + (val[2] - '1'); //  GVx => -128 + (x-1)
    }

    return (uint32_t)yaml_str2int(val, val_len);
}

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

static uint32_t r_mixSrcRaw(const YamlNode* node, const char* val, uint8_t val_len)
{
    if (val_len > 0 && val[0] == 'I') {
        return yaml_str2uint(val+1, val_len-1) + MIXSRC_FIRST_INPUT;
    }

    return yaml_parse_enum(enum_MixSources, val, val_len);
}

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

static uint8_t select_zov(uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= sizeof(ZoneOptionValueEnum);
    ZoneOptionValueEnum* p_zovt = (ZoneOptionValueEnum*)data;
    return *p_zovt;
}

static uint8_t select_mod_type(uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    data -= offsetof(ModuleData, ppm);

    switch(((ModuleData*)data)->type) {
    case MODULE_TYPE_NONE:
    case MODULE_TYPE_PPM:
    case MODULE_TYPE_DSM2:
    case MODULE_TYPE_CROSSFIRE:
        return 1;
    case MODULE_TYPE_MULTIMODULE:
        return 2;
    case MODULE_TYPE_XJT_PXX1:
    case MODULE_TYPE_R9M_PXX1:
    case MODULE_TYPE_R9M_LITE_PXX1:
    //case MODULE_TYPE_R9M_LITE_PRO_PXX1:
        return 3;
    case MODULE_TYPE_SBUS:
        return 4;
    case MODULE_TYPE_ISRM_PXX2:
    case MODULE_TYPE_R9M_PXX2:
    case MODULE_TYPE_R9M_LITE_PXX2:
    case MODULE_TYPE_R9M_LITE_PRO_PXX2:
    case MODULE_TYPE_XJT_LITE_PXX2:
        return 5;
    }
    return 0;
}

static uint8_t select_custom_fn(uint8_t* data, uint32_t bitoffs)
{
    // always use 'all'
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

static uint32_t sw_read(const char* val, uint8_t val_len)
{
    return yaml_parse_enum(enum_MixSources, val, val_len) - MIXSRC_FIRST_SWITCH;
}

static bool sw_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_SWITCH, enum_MixSources);
    return wf(opaque, str, strlen(str));
}

static const struct YamlIdStr enum_SwitchConfig[] = {
    {  SWITCH_NONE, "none"  },
    {  SWITCH_TOGGLE, "toggle"  },
    {  SWITCH_2POS, "2pos"  },
    {  SWITCH_3POS, "3pos"  },
    {  0, NULL  }
};

static const struct YamlNode struct_switchConfig[] = {
    YAML_IDX_CUST( "sw", sw_read, sw_write ),
    YAML_ENUM( "type", 2, enum_SwitchConfig),
    YAML_END
};

static uint32_t pot_read(const char* val, uint8_t val_len)
{
    return yaml_parse_enum(enum_MixSources, val, val_len) - MIXSRC_FIRST_POT;
}

static bool pot_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_POT, enum_MixSources);
    return wf(opaque, str, strlen(str));
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
    YAML_END
};

static uint32_t slider_read(const char* val, uint8_t val_len)
{
    return yaml_parse_enum(enum_MixSources, val, val_len) - MIXSRC_FIRST_SLIDER;
}

static bool slider_write(uint32_t idx, yaml_writer_func wf, void* opaque)
{
    const char* str = yaml_output_enum(idx + MIXSRC_FIRST_SLIDER, enum_MixSources);
    return wf(opaque, str, strlen(str));
}

static const struct YamlIdStr enum_SliderConfig[] = {
    {  SLIDER_NONE, "none" },
    {  SLIDER_WITH_DETENT, "with_detent" },
    {  0, NULL }
};

static const struct YamlNode struct_sliderConfig[] = {
    YAML_IDX_CUST( "sl", slider_read, slider_write ),
    YAML_ENUM( "type", 1, enum_SliderConfig),
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

    if (val_len > 3
        && val[0] == '6'
        && val[1] == 'P'
        && (val[2] >= '0' && val[2] <= '9')
        && (val[3] >= '0' && val[3] < (XPOTS_MULTIPOS_COUNT + '0'))) {

        ival = (val[2] - '0') * XPOTS_MULTIPOS_COUNT + (val[3] - '0')
            + SWSRC_FIRST_MULTIPOS_SWITCH;
    }
    else if (val_len >= 2
             && val[0] == 'L'
             && (val[1] >= '0' && val[1] <= '9')) {

        ival = SWSRC_FIRST_LOGICAL_SWITCH + yaml_str2int(val+1, val_len-1) - 1;
    }
    //TODO: Flight modes ('FM[0-9]')
    //TODO: Sensors ('Sensor[0-9]')
    else {
        ival = yaml_parse_enum(enum_SwitchSources, val, val_len);
    }

    return neg ? -ival : ival;
}

static bool w_swtchSrc(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque)
{
    int32_t sval = yaml_to_signed(val, node->size);
    if (sval < 0) {
        wf(opaque, "!", 1);
        sval = abs(sval);
    }

    const char* str = NULL;
    if (sval >= SWSRC_FIRST_MULTIPOS_SWITCH
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
    else if (sval >= SWSRC_FIRST_LOGICAL_SWITCH
             && sval <= SWSRC_LAST_LOGICAL_SWITCH) {

        wf(opaque, "L", 1);
        str = yaml_unsigned2str(sval - SWSRC_FIRST_LOGICAL_SWITCH + 1);
        return wf(opaque,str, strlen(str));
    }
    else if (sval >= SWSRC_FIRST_FLIGHT_MODE
             && sval <= SWSRC_LAST_FLIGHT_MODE) {

        //TODO
        return true;
    }
    else if (sval >= SWSRC_FIRST_SENSOR
             && sval <= SWSRC_LAST_SENSOR) {

        //TODO
        return true;
    }
    
    str = yaml_output_enum(sval, enum_SwitchSources);
    return wf(opaque, str, strlen(str));
}

static bool cfn_is_active(uint8_t* data, uint32_t bitoffs)
{
    data += bitoffs >> 3UL;
    return ((CustomFunctionData*)data)->swtch;
}

static bool gvar_is_active(uint8_t* data, uint32_t bitoffs)
{
    gvar_t* gvar = (gvar_t*)(data + (bitoffs>>3UL));
    return *gvar != GVAR_MAX+1;
}

static bool fmd_is_active(uint8_t* data, uint32_t bitoffs)
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
