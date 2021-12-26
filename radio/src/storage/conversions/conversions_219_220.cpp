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

#include <cstdlib>
#include <cstring>

#include "definitions.h"
#include "datastructs_219.h"
#include "datastructs_220.h"

#include "debug.h"
#include "strhelpers.h"

#if defined(EEPROM) || defined(EEPROM_RLC)
  #include <storage/eeprom_common.h>
#else
  #include <storage/sdcard_common.h>
#endif

#if STORAGE_CONVERSIONS < 220

//
// Summary of all changes between 219 & 220
//
// All Radios:
// ===========
//
//  PACK(struct TimerData {
// -  int32_t  mode:9;            // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
// -  uint32_t start:23;
// -  int32_t  value:24;
// +  uint32_t start:22;
// +  int32_t  swtch:10;
// +  int32_t  value:22;
// +  uint32_t mode:3; // timer mode (OFF, ON, Start, THs, TH%, THt)
//    uint32_t countdownBeep:2;
//    uint32_t minuteBeep:1;
//    uint32_t persistent:2;
//    int32_t  countdownStart:2;  // -2=30s -1=20s 0=10s 1=5s converted to -2=5s -1=10s 0=20s 1=30s
// -  uint32_t direction:1;
//    NOBACKUP(char name[LEN_TIMER_NAME]);
//  });
//
//  enum TimerModes {
// -  TMRMODE_NONE,// ->OFF
// -  TMRMODE_ABS, // ->ON
// +  TMRMODE_OFF,
// +  TMRMODE_ON,
// +  TMRMODE_START, // 2 -> +1
//    TMRMODE_THR,
//    TMRMODE_THR_REL,
// -  TMRMODE_THR_TRG, // ->START
// -  TMRMODE_COUNT
// +  TMRMODE_THR_START,
// +  TMRMODE_COUNT,
// +  TMRMODE_MAX = TMRMODE_COUNT - 1
//  };
//
// COLORLCD:
// =========
//
// -  CHKSIZE(CustomScreenData, 610);
// -  CHKSIZE(Topbar::PersistentData, 216);
// +  CHKSIZE(CustomScreenData, 850);
// +  CHKTYPE(TopBarPersistentData, 300);
//
// -  CHKSIZE(RadioData, 881);
// -  CHKSIZE(ModelData, 9736);
// +  CHKSIZE(RadioData, 901);
// +  CHKSIZE(ModelData, 11020);
//
//  -> RadioData change probably comes from THEME_DATA (added parameter types)
//  -> ModelData: +1284 (CustomScreenData x 5 = 1200; TopBar::PersistentData: +84)
//

typedef bin_storage_220::TimerData TimerData_v220;

static void convertToStr(char* str, size_t len)
{
  for (size_t c = 0; c < len; c++) {
    str[c] = zchar2char(str[c]);
  }
  // Trim string
  while(len > 0 && str[len-1]) {
    if (str[len - 1] != ' ' && str[len - 1] != '\0') break;
    str[--len] = '\0';
  }
}

void convertModelData_219_to_220(void* data)
{
  constexpr unsigned md_size = sizeof(bin_storage_220::ModelData);
  auto& model = *reinterpret_cast<bin_storage_220::ModelData*>(data);

  auto oldModelAllocated = reinterpret_cast<bin_storage_220::ModelData*>(malloc(md_size));
  bin_storage_220::ModelData& oldModel = *oldModelAllocated;

  memcpy(&oldModel, &model, md_size);
  bin_storage_220::ModelData& newModel = (bin_storage_220::ModelData&)model;
  convertToStr(model.header.name, LEN_MODEL_NAME_219);

  for (uint8_t i=0; i<MAX_TIMERS_219; i++) {
    // convert to new timers v220
    bin_storage_220::TimerData& timer = newModel.timers[i];
    convertToStr(timer.name, LEN_TIMER_NAME_219);

    TimerData_v219& timer_219 = *(TimerData_v219*)(&oldModel.timers[i]);

    // Convert mode

    if (timer_219.mode >= bin_storage_220::TMRMODE_START) {
      timer_219.mode += 1;
    }
    if (timer_219.mode < bin_storage_220::TMRMODE_COUNT
        && timer_219.mode >=0) {
      timer.mode = timer_219.mode;
    }
    else {
      timer.mode = bin_storage_220::TMRMODE_ON;
      if (timer_219.mode > 0)
        timer.swtch = timer_219.mode - (bin_storage_220::TMRMODE_COUNT - 1);
      else
        timer.swtch = timer_219.mode;
    }

    timer.start = timer_219.start;
    timer.value = timer_219.value;
    timer.countdownBeep = timer_219.countdownBeep;
    timer.minuteBeep = timer_219.minuteBeep;
    timer.persistent = timer_219.persistent;
    timer.countdownStart = (timer_219.countdownStart * -1) - 1;

  }

  // MixData::name
  for (uint8_t i=0; i<MAX_MIXERS_219; ++i) {
    convertToStr(model.mixData[i].name, LEN_EXPOMIX_NAME_219);
  }

  // LimitData::name
  for (uint8_t i=0; i<MAX_OUTPUT_CHANNELS_219; ++i) {
    convertToStr(model.limitData[i].name, LEN_CHANNEL_NAME_219);
  }

  // ExpoData::name
  for (uint8_t i=0; i<MAX_EXPOS_219; ++i) {
    convertToStr(model.expoData[i].name, LEN_EXPOMIX_NAME_219);
  }

  // CurveHeader.name
  for (uint8_t i=0; i<MAX_CURVES_219; ++i) {
    convertToStr(model.curves[i].name, LEN_CURVE_NAME_219);
  }

  // CustomFunctionData::play.name
  // for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS_219; ++i) {
  //   CustomFunctionData* cfn = &(model.customFn[i]);
  //   if (cfn->func == FUNC_PLAY_TRACK || cfn->func == FUNC_BACKGND_MUSIC) {
  //     convertToStr(cfn->play.name, LEN_FUNCTION_NAME_219);
  //   }
  // }

  // GVarData::name ?
  for (uint8_t i=0; i<MAX_GVARS_219; ++i) {
    convertToStr(model.gvars[i].name, LEN_GVAR_NAME_219);
  }

  // ScriptData::name ?
  // ScriptData::file ?
  for (uint8_t i=0; i<MAX_SCRIPTS_219; ++i) {
    convertToStr(model.scriptsData[i].file, LEN_SCRIPT_FILENAME);
    convertToStr(model.scriptsData[i].name, LEN_SCRIPT_NAME);
  }

  // Input names
  for (uint8_t i=0; i<MAX_INPUTS_219; ++i) {
    convertToStr(model.inputNames[i], LEN_INPUT_NAME_219);
  }

  // Telemetry names
  for (uint8_t i=0; i<MAX_TELEMETRY_SENSORS_219; ++i) {
    convertToStr(model.telemetrySensors[i].label, TELEM_LABEL_LEN_219);
  }

  // FlightModeData name
  for (uint8_t i=0; i<MAX_FLIGHT_MODES_219; ++i) {
    convertToStr(model.flightModeData[i].name, LEN_FLIGHT_MODE_NAME_219);
  }

  //
  // TODO: convert all names from zchar to "normal" chars
  //

  // g_model.header.bitmap ???? really ????
  // TelemetryScriptData::file ?
  // ModuleData::pxx2.receiverName

#if defined(COLORLCD)
  // Clear CustomScreenData + TopBarPersistentData
  // as they cannot be converted (missing option types)
  memset(newModel.screenData, 0,
         sizeof(newModel.screenData) +
         sizeof(newModel.topbarData));

  strcpy(newModel.screenData[0].LayoutId, "Layout2P1");
  auto persistentData = &newModel.screenData[0].layoutData;

  // top bar
  persistentData->options[0].type  = bin_storage_220::ZOV_Bool;
  persistentData->options[0].value.boolValue = true;

  // flight mode
  persistentData->options[1].type  = bin_storage_220::ZOV_Bool;
  persistentData->options[1].value.boolValue = true;

  // sliders
  persistentData->options[2].type  = bin_storage_220::ZOV_Bool;
  persistentData->options[2].value.boolValue = true;

  // trims
  persistentData->options[3].type  = bin_storage_220::ZOV_Bool;
  persistentData->options[3].value.boolValue = true;

  // mirrored
  persistentData->options[4].type  = bin_storage_220::ZOV_Bool;
  persistentData->options[4].value.boolValue = false;
#endif
  free(oldModelAllocated);
}

#if !defined(EEPROM_RLC)
const char* convertModelData_219_to_220(const char* path)
{
  constexpr unsigned md_size = sizeof(bin_storage_220::ModelData);

  auto md_220 = reinterpret_cast<bin_storage_220::ModelData*>(malloc(md_size));
  memset(md_220, 0, md_size);
  
  uint8_t version;
  const char* error = loadFileBin(path, (uint8_t *)md_220, md_size, &version);
  if (!error) {
    convertModelData_219_to_220((void*)md_220);
    error = writeFileBin(path, (uint8_t *)md_220, md_size, 220);
  }

  free(md_220);
  return error;
}
#else
const char* convertModelData_219_to_220(uint8_t id)
{
  constexpr unsigned md_size = sizeof(bin_storage_220::ModelData);

  auto md_220 = reinterpret_cast<uint8_t*>(malloc(md_size));
  memset(md_220, 0, md_size);
  
  uint16_t read = eeLoadModelData(id, md_220, md_size);
  if (read == md_size) {
    convertModelData_219_to_220((void*)md_220);
    eeWriteModelData(id, (uint8_t *)md_220, md_size, true);
  } else {
    // ERROR: size mismatch !!!
  }

  free(md_220);
  return nullptr;
}
#endif

void convertRadioData_219_to_220(void* data)
{
  TRACE("Radio conversion from v219 to v220");

  auto& settings = *reinterpret_cast<bin_storage_220::RadioData*>(data);
  settings.version = 220;
  settings.variant = EEPROM_VARIANT;

#if defined(COLORLCD)
  // Clear CustomScreenData + TopBarPersistentData
  // as they cannot be converted (missing option types)
  strcpy(settings.themeName, "EdgeTX");
  memset(&settings.themeData, 0, sizeof(settings.themeData));
#endif

  for (int i = 0; i < NUM_SWITCHES_219; ++i) {
    char* sw_name = &(settings.switchNames[i][0]);
    convertToStr(sw_name, LEN_SWITCH_NAME_219);
  }

  for (int i = 0; i < NUM_STICKS_219 + NUM_POTS_219 + NUM_SLIDERS_219; ++i) {
    char* sw_name = &(settings.anaNames[i][0]);
    convertToStr(sw_name, LEN_ANA_NAME_219);
  }

#if defined(PCBHORUS) || defined(PCBNV14) || defined(PCBPL18)
  convertToStr(settings.bluetoothName, LEN_BLUETOOTH_NAME_219);
#endif
}

#if !defined(EEPROM_RLC)
const char* convertRadioData_219_to_220(const char* path)
{
  constexpr unsigned rd_size = sizeof(bin_storage_220::RadioData);

  auto rd_220 = reinterpret_cast<bin_storage_220::RadioData*>(malloc(rd_size));
  memset(rd_220, 0, rd_size);
  
  uint8_t version;
  const char* error = loadFileBin(path, (uint8_t *)rd_220, rd_size, &version);
  if (!error) {
    convertRadioData_219_to_220((void*)rd_220);
    error = writeFileBin(path, (uint8_t *)rd_220, rd_size, 220);
  }

  free(rd_220);
  return error;
}
#else
const char* convertRadioData_219_to_220()
{
  constexpr unsigned rd_size = sizeof(bin_storage_220::RadioData);
  auto rd_220 = reinterpret_cast<uint8_t*>(malloc(rd_size));
  
  uint16_t read = eeLoadGeneralSettingsData(rd_220, rd_size);
  if (read >= rd_size - 1) { // 220 started with 1 byte less than 219
    convertRadioData_219_to_220((void*)rd_220);
    eeWriteGeneralSettingData((uint8_t *)rd_220, rd_size, true);
  } else {
    // ERROR: size mismatch !!!
  }

  free(rd_220);
  return nullptr;
}
#endif

#endif
