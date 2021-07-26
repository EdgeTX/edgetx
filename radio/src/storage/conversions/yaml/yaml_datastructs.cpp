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

#include <storage/yaml/yaml_node.h>
#include <storage/yaml/yaml_bits.h>
#include <storage/yaml/yaml_tree_walker.h>

#include <algorithm>

#include <debug.h>
#include <string.h>

#include "../../../strhelpers.h"
#include "../../../switches.h"
#include "../../../timers.h"
#include "../../../pulses/multi.h"

bool w_board(void* user, uint8_t* data, uint32_t bitoffs, yaml_writer_func wf, void* opaque);
bool in_write_weight(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool output_source_1_param(const char* src_prefix, size_t src_len, uint32_t n, yaml_writer_func wf, void* opaque);
bool w_vbat_min(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_vbat_max(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

#if defined(COLORLCD)
uint8_t select_zov(void* user, uint8_t* data, uint32_t bitoffs);
bool w_zov_color(void* user, uint8_t* data, uint32_t bitoffs, yaml_writer_func wf, void* opaque);
#else
bool w_tele_screen_type(void* user, uint8_t* data, uint32_t bitoffs, yaml_writer_func wf, void* opaque);
#endif

uint8_t select_mod_type(void* user, uint8_t* data, uint32_t bitoffs);
uint8_t select_script_input(void* user, uint8_t* data, uint32_t bitoffs);
uint8_t select_id1(void* user, uint8_t* data, uint32_t bitoffs);
uint8_t select_id2(void* user, uint8_t* data, uint32_t bitoffs);
uint8_t select_sensor_cfg(void* user, uint8_t* data, uint32_t bitoffs);

extern const struct YamlIdStr enum_SwitchConfig[];

bool cfn_is_active(void* user, uint8_t* data, uint32_t bitoffs);
bool gvar_is_active(void* user, uint8_t* data, uint32_t bitoffs);
bool fmd_is_active(void* user, uint8_t* data, uint32_t bitoffs);
bool swash_is_active(void* user, uint8_t* data, uint32_t bitoffs);

extern const struct YamlIdStr enum_BeeperMode[];

bool w_beeperMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_5pos(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_vol(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_spPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_vPitch(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_trainerMode(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_tele_sensor(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);
bool w_flightModes(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

extern const char* _func_reset_param_lookup[];
extern const char* _func_failsafe_lookup[];

extern const char* _func_sound_lookup[];
extern const uint8_t _func_sound_lookup_size;

extern const char* _adjust_gvar_mode_lookup[];
extern const uint8_t _adjust_gvar_mode_lookup_size;

bool w_modSubtype(void* user, uint8_t* data, uint32_t bitoffs, yaml_writer_func wf, void* opaque);
bool w_channelsCount(const YamlNode* node, uint32_t val, yaml_writer_func wf, void* opaque);

namespace yaml_conv_220 {

#include "yaml_datastructs_funcs.cpp"

#if defined(PCBX12S)
 #include "yaml_datastructs_x12s.cpp"
#elif defined(PCBX10)
 #include "yaml_datastructs_x10.cpp"
#elif defined(PCBNV14)
 #include "yaml_datastructs_nv14.cpp"
#elif defined(PCBX7)
 #if defined(RADIO_T12)
  #include "yaml_datastructs_t12.cpp"
 #elif defined(RADIO_TLITE)
  #include "yaml_datastructs_tlite.cpp"
 #elif defined(RADIO_TX12)
  #include "yaml_datastructs_tx12.cpp"
 #elif defined(RADIO_T8)
  #include "yaml_datastructs_t8.cpp"
 #else
  #include "yaml_datastructs_x7.cpp"
 #endif
#elif defined(PCBX9LITE)
 #include "yaml_datastructs_x9lite.cpp"
#elif defined(PCBX9LITES)
 #include "yaml_datastructs_x9lites.cpp"
#elif defined(PCBXLITE)
 #include "yaml_datastructs_xlite.cpp"
#elif defined(PCBXLITES)
 #include "yaml_datastructs_xlites.cpp"
#elif defined(PCBX9E)
 #include "yaml_datastructs_x9e.cpp"
#elif defined(PCBX9D) || defined(PCBX9DP)
 #include "yaml_datastructs_x9d.cpp"
#else
#error "Board not supported by YAML storage"
#endif

static_assert(MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN,
              "MAX_STR > MAX_RADIODATA_MODELDATA_PARTIALMODEL_STR_LEN");

};
