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

#include "gtests.h"
#include "storage/conversions/conversions.h"
#include "location.h"

#if defined(EEPROM) || defined(EEPROM_RLC)
#include <storage/eeprom_common.h>
#endif

#if defined(SDCARD_YAML)
const char * loadRadioSettingsYaml();
#endif

#if defined(EEPROM_SIZE)
void loadEEPROMFile(const char * filename)
{
  FILE * f = fopen(filename, "rb");
  assert(fread(eeprom, 1, EEPROM_SIZE, f) == EEPROM_SIZE);
}
#endif

#if defined(PCBX9DP)
TEST(Conversions, ConversionX9DPFrom23)
{
#if defined(SDCARD_YAML)
  simuFatfsSetPaths(TESTS_BUILD_PATH "/", TESTS_BUILD_PATH "/");
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
#endif

  loadEEPROMFile(TESTS_PATH "/eeprom_23_x9d+.bin");
  eepromOpen();
#if defined(EEPROM)
  eeLoadGeneralSettingsData();
  convertRadioData_219_to_220(g_eeGeneral);
#else
  convertRadioData_219_to_220();
  convertRadioData_220_to_221();
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());
#endif
  eeConvertModel(0, 219);
  loadModel((uint8_t)0);

  EXPECT_EQ(-30, g_eeGeneral.vBatMin);
  EXPECT_EQ(8, g_eeGeneral.speakerVolume);
  EXPECT_EQ('e', g_eeGeneral.ttsLanguage[0]);
  EXPECT_EQ('n', g_eeGeneral.ttsLanguage[1]);

  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_eeGeneral.customFn[0].swtch);
  EXPECT_EQ(FUNC_LOGS, g_eeGeneral.customFn[0].func);
  EXPECT_EQ(20, g_eeGeneral.customFn[0].all.val);

  EXPECT_STRNEQ("Tes", g_eeGeneral.switchNames[0]); // ZSTREQ
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(0));

  EXPECT_STRNEQ("Test", g_model.header.name); // ZSTREQ
  EXPECT_EQ(TMRMODE_ON, g_model.timers[0].mode); // new!
  EXPECT_EQ(SWSRC_SA0, g_model.timers[0].swtch); // new!
  EXPECT_EQ(80, g_model.mixData[0].weight);
  EXPECT_EQ(-100, g_model.limitData[0].max); // 90.0
  EXPECT_EQ(80, g_model.expoData[0].weight);
  EXPECT_EQ(10, g_model.flightModeData[0].gvars[0]);
  EXPECT_STRNEQ("Tes", g_model.gvars[0].name); // ZSTREQ
  EXPECT_STRNEQ("Test", g_model.flightModeData[0].name); // ZSTREQ

#if defined(INTERNAL_MODULE_PXX2)
  EXPECT_EQ(MODULE_TYPE_ISRM_PXX2, g_model.moduleData[INTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16, g_model.moduleData[INTERNAL_MODULE].subType);
#else
  EXPECT_EQ(MODULE_TYPE_NONE, g_model.moduleData[INTERNAL_MODULE].type);
#endif

  EXPECT_EQ(MODULE_TYPE_R9M_PXX1, g_model.moduleData[EXTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_R9M_FCC, g_model.moduleData[EXTERNAL_MODULE].subType);

  EXPECT_STRNEQ("Rud", g_model.inputNames[0]); // ZSTREQ
  EXPECT_STRNEQ("Tes", g_model.telemetrySensors[0].label); // ZSTREQ
  EXPECT_EQ(10, g_model.telemetrySensors[0].id);
  EXPECT_EQ(10, g_model.telemetrySensors[0].frskyInstance.physID);
  EXPECT_EQ(MIXSRC_FIRST_TELEM, g_model.logicalSw[0].v1);

  EXPECT_STRNEQ("abc.wav", g_model.customFn[1].play.name);

#if defined(SDCARD_YAML)
  simuFatfsSetPaths("","");
#endif
}

TEST(Conversions, ConversionX9DPFrom23_2)
{
#if defined(SDCARD_YAML)
  simuFatfsSetPaths(TESTS_BUILD_PATH "/", TESTS_BUILD_PATH "/");
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
#endif

  loadEEPROMFile(TESTS_PATH "/eeprom_23_x9d+2.bin");

  eepromOpen();
#if defined(EEPROM)
  eeLoadGeneralSettingsData();
  convertRadioData_219_to_220(g_eeGeneral);
#else
  convertRadioData_219_to_220();
  convertRadioData_220_to_221();
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());
#endif
  eeConvertModel(6, 219);
  loadModel((uint8_t)6);

  EXPECT_EQ(710, g_eeGeneral.calib[0].spanNeg);
  EXPECT_EQ(944, g_eeGeneral.calib[0].mid);
  EXPECT_EQ(770, g_eeGeneral.calib[0].spanPos);

  EXPECT_EQ(TMRMODE_ON, g_model.timers[0].mode); // new!
  EXPECT_EQ(-SWSRC_SA0, g_model.timers[0].swtch); // new!

#if defined(SDCARD_YAML)
  simuFatfsSetPaths("","");
#endif
}
#endif

#if defined(PCBXLITE) && !defined(PCBXLITES)
TEST(Conversions, ConversionXLiteFrom23)
{
#if defined(SDCARD_YAML)
  simuFatfsSetPaths(TESTS_BUILD_PATH "/", TESTS_BUILD_PATH "/");
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
#endif

  loadEEPROMFile(TESTS_PATH "/eeprom_23_xlite.bin");

  eepromOpen();
#if defined(EEPROM)
  eeLoadGeneralSettingsData();
  convertRadioData_219_to_220(g_eeGeneral);
#else
  convertRadioData_219_to_220();
  convertRadioData_220_to_221();
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());
#endif
  eeConvertModel(0, 219);
  loadModel((uint8_t)0);

  EXPECT_EQ(-30, g_eeGeneral.vBatMin);
  EXPECT_EQ(8, g_eeGeneral.speakerVolume);
  EXPECT_EQ('e', g_eeGeneral.ttsLanguage[0]);
  EXPECT_EQ('n', g_eeGeneral.ttsLanguage[1]);

  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_eeGeneral.customFn[0].swtch);
  EXPECT_EQ(FUNC_LOGS, g_eeGeneral.customFn[0].func);
  EXPECT_EQ(20, g_eeGeneral.customFn[0].all.val);

  EXPECT_STRNEQ("Tes", g_eeGeneral.switchNames[0]);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(0));

  EXPECT_STRNEQ("Test", g_model.header.name);
  EXPECT_EQ(MODULE_TYPE_R9M_PXX1, g_model.moduleData[EXTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_R9M_EU, g_model.moduleData[EXTERNAL_MODULE].subType);
  EXPECT_EQ(80, g_model.mixData[0].weight);
  EXPECT_EQ(80, g_model.expoData[0].weight);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.mixData[4].srcRaw);
  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_model.mixData[4].swtch);
  EXPECT_EQ(SWASH_TYPE_120X, g_model.swashR.type);
  EXPECT_STRNEQ("Thr", g_model.inputNames[0]);

  EXPECT_STRNEQ("Tes", g_model.telemetrySensors[0].label);
  EXPECT_EQ(10, g_model.telemetrySensors[0].id);
  EXPECT_EQ(8, g_model.telemetrySensors[0].frskyInstance.physID);
  EXPECT_EQ(-100, g_model.limitData[0].max); // 90.0

  EXPECT_EQ(LS_FUNC_VPOS, g_model.logicalSw[0].func);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.logicalSw[0].v1);
  EXPECT_EQ(0, g_model.logicalSw[0].v2);

  EXPECT_EQ(TELEMETRY_SCREEN_TYPE_VALUES, g_model.screensType & 0x03);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.screens[0].lines[0].sources[0]);
  EXPECT_EQ(MIXSRC_TIMER3, g_model.screens[0].lines[0].sources[1]);

#if defined(SDCARD_YAML)
  simuFatfsSetPaths("","");
#endif
}
#endif

#if defined(PCBX7)
TEST(Conversions, ConversionX7From23)
{
#if defined(SDCARD_YAML)
  simuFatfsSetPaths(TESTS_BUILD_PATH "/", TESTS_BUILD_PATH "/");
  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);
#endif

  loadEEPROMFile(TESTS_PATH "/eeprom_23_x7.bin");

  eepromOpen();
#if defined(EEPROM)
  eeLoadGeneralSettingsData();
  convertRadioData_218_to_219(g_eeGeneral);
#else
  convertRadioData_219_to_220();
  convertRadioData_220_to_221();
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());
#endif
  eeConvertModel(0, 219);
  loadModel((uint8_t)0);

  EXPECT_EQ(-30, g_eeGeneral.vBatMin);
  EXPECT_EQ(8, g_eeGeneral.speakerVolume);
  EXPECT_EQ('e', g_eeGeneral.ttsLanguage[0]);
  EXPECT_EQ('n', g_eeGeneral.ttsLanguage[1]);

  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_eeGeneral.customFn[0].swtch);
  EXPECT_EQ(FUNC_LOGS, g_eeGeneral.customFn[0].func);
  EXPECT_EQ(20, g_eeGeneral.customFn[0].all.val);

  EXPECT_STRNEQ("Tes", g_eeGeneral.switchNames[0]);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(0));

  EXPECT_STRNEQ("Test", g_model.header.name);
  EXPECT_EQ(MODULE_TYPE_R9M_PXX1, g_model.moduleData[EXTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_R9M_EU, g_model.moduleData[EXTERNAL_MODULE].subType);
  EXPECT_EQ(80, g_model.mixData[0].weight);
  EXPECT_EQ(80, g_model.expoData[0].weight);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.mixData[4].srcRaw);
  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_model.mixData[4].swtch);
  EXPECT_EQ(SWASH_TYPE_120X, g_model.swashR.type);
  EXPECT_STRNEQ("Thr", g_model.inputNames[0]);

  EXPECT_STRNEQ("Tes", g_model.telemetrySensors[0].label);
  EXPECT_EQ(10, g_model.telemetrySensors[0].id);
  EXPECT_EQ(9, g_model.telemetrySensors[0].frskyInstance.physID);
  EXPECT_EQ(-100, g_model.limitData[0].max); // 90.0

  EXPECT_EQ(10, g_model.flightModeData[0].gvars[0]);
  EXPECT_STRNEQ("FMtest", g_model.flightModeData[1].name);
  EXPECT_EQ(45, g_model.flightModeData[1].swtch);
  EXPECT_STRNEQ("Tes", g_model.gvars[0].name);

  EXPECT_EQ(LS_FUNC_VPOS, g_model.logicalSw[0].func);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.logicalSw[0].v1);
  EXPECT_EQ(0, g_model.logicalSw[0].v2);

  EXPECT_EQ(TELEMETRY_SCREEN_TYPE_VALUES, g_model.screensType & 0x03);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.screens[0].lines[0].sources[0]);
  EXPECT_EQ(MIXSRC_TIMER3, g_model.screens[0].lines[0].sources[1]);

#if defined(SDCARD_YAML)
  simuFatfsSetPaths("","");
#endif
}
#endif

#if defined(PCBX10) && !defined(RADIO_FAMILY_T16)
TEST(Conversions, ConversionX10From23)
{
  simuFatfsSetPaths(TESTS_BUILD_PATH "/model_23_x10/", TESTS_BUILD_PATH "/model_23_x10/");

  convertRadioData_219_to_220("/RADIO/radio.bin");
  convertRadioData_220_to_221("/RADIO/radio.bin");
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());

  char modelname[] = "model1.bin";
  convertBinModelData(modelname, 219);
  loadModel(modelname);

  EXPECT_EQ(220, g_eeGeneral.version); // TODO: 221?

  EXPECT_EQ(100, g_eeGeneral.calib[9].spanNeg);
  EXPECT_EQ(500, g_eeGeneral.calib[9].mid);
  EXPECT_EQ(900, g_eeGeneral.calib[9].spanPos);

  EXPECT_EQ(200, g_eeGeneral.calib[10].spanNeg);
  EXPECT_EQ(400, g_eeGeneral.calib[10].mid);
  EXPECT_EQ(600, g_eeGeneral.calib[10].spanPos);

  EXPECT_EQ(-23, g_eeGeneral.vBatMin);
  EXPECT_EQ(8, g_eeGeneral.speakerVolume);
  EXPECT_STRNEQ("en", g_eeGeneral.ttsLanguage);
  EXPECT_STRNEQ("model1.yml", g_eeGeneral.currModelFilename);

  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_eeGeneral.customFn[0].swtch);
  EXPECT_EQ(FUNC_LOGS, g_eeGeneral.customFn[0].func);
  EXPECT_EQ(20, g_eeGeneral.customFn[0].all.val);

  EXPECT_EQ(SWSRC_ON, g_eeGeneral.customFn[1].swtch);
  EXPECT_EQ(FUNC_VOLUME, g_eeGeneral.customFn[1].func);
  EXPECT_EQ(MIXSRC_RS, g_eeGeneral.customFn[1].all.val);

  EXPECT_STRNEQ("Tes", g_eeGeneral.switchNames[0]);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(0));

  EXPECT_STRNEQ("BT_X10", g_eeGeneral.bluetoothName);
  EXPECT_STREQ("EdgeTX", g_eeGeneral.themeName);

  EXPECT_STRNEQ("Test", g_model.header.name);
  EXPECT_EQ(0, g_model.noGlobalFunctions);
  EXPECT_EQ(0, g_model.beepANACenter);
  EXPECT_EQ(80, g_model.mixData[0].weight);
  EXPECT_EQ(MIXSRC_MAX, g_model.mixData[2].srcRaw); // MAX
  EXPECT_EQ(MIXSRC_LS, g_model.mixData[3].srcRaw); // LS
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.mixData[5].srcRaw);
  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_model.mixData[5].swtch);
  EXPECT_EQ(-100, g_model.limitData[0].max); // 90.0
  EXPECT_EQ(80, g_model.expoData[0].weight);
  EXPECT_EQ(LS_FUNC_VPOS, g_model.logicalSw[0].func);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.logicalSw[0].v1);
  EXPECT_EQ(0, g_model.logicalSw[0].v2);
  EXPECT_EQ(MIXSRC_FIRST_TELEM+19*3, g_model.logicalSw[1].v1); // TELE:20
  EXPECT_EQ(20, g_model.logicalSw[1].v2);
  EXPECT_EQ(SWSRC_FIRST_LOGICAL_SWITCH, g_model.logicalSw[1].andsw);
  EXPECT_EQ(SWASH_TYPE_120X, g_model.swashR.type);
  EXPECT_STRNEQ("Tes", g_model.flightModeData[0].name);
  EXPECT_EQ(10, g_model.flightModeData[0].gvars[0]);
  EXPECT_STRNEQ("Tes", g_model.gvars[0].name);
  EXPECT_EQ(MODULE_TYPE_R9M_PXX1, g_model.moduleData[EXTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_R9M_EU, g_model.moduleData[EXTERNAL_MODULE].subType);
  EXPECT_STRNEQ("Rud", g_model.inputNames[0]);
  EXPECT_STRNEQ("Tes", g_model.telemetrySensors[0].label);
  EXPECT_EQ(10, g_model.telemetrySensors[0].id);
  EXPECT_EQ(9, g_model.telemetrySensors[0].frskyInstance.physID);
  EXPECT_EQ((NUM_POTS + NUM_SLIDERS + 3), g_model.thrTraceSrc); // CH3

  simuFatfsSetPaths("","");
}
#endif

#if defined(PCBX12S)
TEST(Conversions, ConversionX12SFrom23)
{
  simuFatfsSetPaths(TESTS_BUILD_PATH "/model_23_x12s/", TESTS_BUILD_PATH "/model_23_x12s/");

  convertRadioData_219_to_220("/RADIO/radio.bin");
  convertRadioData_220_to_221("/RADIO/radio.bin");
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());

  char modelname[] = "model1.bin";
  convertBinModelData(modelname, 219);
  loadModel(modelname);

  EXPECT_EQ(220, g_eeGeneral.version); // TODO: 221?
  EXPECT_EQ(-30, g_eeGeneral.vBatMin);
  EXPECT_EQ(8, g_eeGeneral.speakerVolume);
  EXPECT_STRNEQ("en", g_eeGeneral.ttsLanguage);
  EXPECT_STRNEQ("model1.yml", g_eeGeneral.currModelFilename);

  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_eeGeneral.customFn[0].swtch);
  EXPECT_EQ(FUNC_LOGS, g_eeGeneral.customFn[0].func);
  EXPECT_EQ(20, g_eeGeneral.customFn[0].all.val);

  EXPECT_EQ(SWSRC_ON, g_eeGeneral.customFn[1].swtch);
  EXPECT_EQ(FUNC_VOLUME, g_eeGeneral.customFn[1].func);
  EXPECT_EQ(MIXSRC_RS, g_eeGeneral.customFn[1].all.val);

  EXPECT_STRNEQ("Tes", g_eeGeneral.switchNames[0]);
  EXPECT_EQ(SWITCH_3POS, SWITCH_CONFIG(0));

  EXPECT_STRNEQ("BT", g_eeGeneral.bluetoothName);
  EXPECT_STREQ("EdgeTX", g_eeGeneral.themeName);

  EXPECT_STRNEQ("Test", g_model.header.name);
  EXPECT_EQ(0, g_model.noGlobalFunctions);
  EXPECT_EQ(0, g_model.beepANACenter);
  EXPECT_EQ(80, g_model.mixData[0].weight);
  EXPECT_EQ(MIXSRC_MAX, g_model.mixData[2].srcRaw); // MAX
  EXPECT_EQ(MIXSRC_LS, g_model.mixData[3].srcRaw); // LS
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.mixData[5].srcRaw);
  EXPECT_EQ(SWSRC_TELEMETRY_STREAMING, g_model.mixData[5].swtch);
  EXPECT_EQ(-100, g_model.limitData[0].max); // 90.0
  EXPECT_EQ(80, g_model.expoData[0].weight);
  EXPECT_EQ(LS_FUNC_VPOS, g_model.logicalSw[0].func);
  EXPECT_EQ(MIXSRC_FIRST_TRAINER, g_model.logicalSw[0].v1);
  EXPECT_EQ(0, g_model.logicalSw[0].v2);
  EXPECT_EQ(MIXSRC_FIRST_TELEM+19*3, g_model.logicalSw[1].v1); // TELE:20
  EXPECT_EQ(20, g_model.logicalSw[1].v2);
  EXPECT_EQ(SWSRC_FIRST_LOGICAL_SWITCH, g_model.logicalSw[1].andsw);
  EXPECT_EQ(SWASH_TYPE_120X, g_model.swashR.type);
  EXPECT_STRNEQ("Test", g_model.flightModeData[0].name);
  EXPECT_EQ(10, g_model.flightModeData[0].gvars[0]);
  EXPECT_STRNEQ("Tes", g_model.gvars[0].name);
  EXPECT_EQ(MODULE_TYPE_R9M_PXX1, g_model.moduleData[EXTERNAL_MODULE].type);
  EXPECT_EQ(MODULE_SUBTYPE_R9M_EU, g_model.moduleData[EXTERNAL_MODULE].subType);
  EXPECT_STRNEQ("Rud", g_model.inputNames[0]);
  EXPECT_STRNEQ("Tes", g_model.telemetrySensors[0].label);
  EXPECT_EQ(10, g_model.telemetrySensors[0].id);
  EXPECT_EQ(9, g_model.telemetrySensors[0].frskyInstance.physID);
  EXPECT_EQ((NUM_POTS + NUM_SLIDERS + 3), g_model.thrTraceSrc); // CH3

  simuFatfsSetPaths("","");
}
#endif

#if defined(RADIO_TX16S)
TEST(Conversions, ConversionTX16SFrom25)
{
  simuFatfsSetPaths(TESTS_BUILD_PATH "/model_25_tx16s/", TESTS_BUILD_PATH "/model_25_tx16s/");

  convertRadioData_219_to_220("/RADIO/radio.bin");
  convertRadioData_220_to_221("/RADIO/radio.bin");
  EXPECT_EQ(nullptr, loadRadioSettingsYaml());

  char modelname[] = "model1.bin";
  convertBinModelData(modelname, 220);
  loadModel(modelname);

  EXPECT_EQ(220, g_eeGeneral.version); // TODO: 221?
  EXPECT_EQ(-23, g_eeGeneral.vBatMin);
  EXPECT_EQ(0, g_eeGeneral.speakerVolume);
  EXPECT_STRNEQ("en", g_eeGeneral.ttsLanguage);
  EXPECT_STRNEQ("model1.yml", g_eeGeneral.currModelFilename);

  EXPECT_STRNEQ("Model", g_model.header.name);

  EXPECT_EQ(MODULE_SUBTYPE_MULTI_FRSKY, g_model.moduleData[0].getMultiProtocol());
  EXPECT_EQ(MM_RF_FRSKY_SUBTYPE_D8_CLONED, g_model.moduleData[0].subType);
  EXPECT_EQ(-12, g_model.moduleData[0].multi.optionValue);

  EXPECT_EQ(MODULE_TYPE_XJT_PXX1, g_model.moduleData[1].type);
  EXPECT_EQ(MODULE_SUBTYPE_PXX1_ACCST_LR12, g_model.moduleData[1].subType);
  EXPECT_EQ(4, g_model.moduleData[1].channelsCount); // 4 + 8 = 12

  const auto& top_widget = g_model.topbarData.zones[3];
  EXPECT_STRNEQ("Value", top_widget.widgetName);

  const auto& top_option = top_widget.widgetData.options[0];
  EXPECT_EQ(ZOV_Source, top_option.type);
  EXPECT_EQ(MIXSRC_TX_VOLTAGE, top_option.value.unsignedValue);
  
  const auto& color_option = top_widget.widgetData.options[1];
  EXPECT_EQ(ZOV_Color, color_option.type);
  EXPECT_EQ(0xFFFF, color_option.value.unsignedValue);

  char modelname2[] = "model2.bin";
  convertBinModelData(modelname2, 220);
  loadModel(modelname2);

  EXPECT_EQ(LS_FUNC_EDGE, g_model.logicalSw[0].func);
  EXPECT_EQ(SWSRC_SF2, g_model.logicalSw[0].v1);
  EXPECT_EQ(-129, g_model.logicalSw[0].v2);
  EXPECT_EQ(-1, g_model.logicalSw[0].v3);
  EXPECT_EQ(SWSRC_SF2, g_model.logicalSw[0].andsw);

  EXPECT_EQ(LS_FUNC_VNEG, g_model.logicalSw[1].func);
  EXPECT_EQ(MIXSRC_FIRST_INPUT, g_model.logicalSw[1].v1);
  EXPECT_EQ(-98, g_model.logicalSw[1].v2);
  EXPECT_EQ(SWSRC_NONE, g_model.logicalSw[1].andsw);

  simuFatfsSetPaths("","");
}
#endif

