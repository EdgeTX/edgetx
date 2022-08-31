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
#include "conversions.h"

#if defined(SDCARD_RAW) || defined(SDCARD_YAML)
#include "storage/modelslist.h"
#include "storage/sdcard_common.h"
#include "storage/sdcard_raw.h"
#include "storage/sdcard_yaml.h"
#endif

#if defined(STORAGE_MODELSLIST)
static void drawProgressScreen(const char* filename, int progress, int total)
{
#if defined(COLORLCD)
  lcdInitDirectDrawing();

  OpenTxTheme* l_theme = static_cast<OpenTxTheme*>(theme);
  l_theme->drawBackground(lcd);

  lcd->drawText(LCD_W / 2, LCD_H / 2 - 30, STR_CONVERTING,
                FONT(XL) | CENTERED | COLOR_THEME_WARNING);
  lcd->drawText(LCD_W / 2, LCD_H / 2, filename,
                FONT(STD) | CENTERED | COLOR_THEME_SECONDARY1);

  l_theme->drawProgressBar(lcd,
                           LCD_W / 4,
                           LCD_H / 2 + 40,
                           LCD_W / 2,
                           20,
                           progress, total);
  lcdRefresh();

  // invalidate screen to enable quick return
  // to normal display routine
  lv_obj_invalidate(lv_scr_act());

  WDG_RESET();
#else
  // TODO: BW progress screen
#endif
}

void convertBinRadioData(const char * path, int version)
{
  TRACE("convertRadioData(%s,%d)", path, version);

#if defined(COLORLCD)
  // the theme has not been loaded before
  static_cast<OpenTxTheme*>(theme)->load();

  // Init backlight OFF value before entering alert screens
  g_eeGeneral.blOffBright = 20;
#endif

  RAISE_ALERT(STR_STORAGE_WARNING, STR_SDCARD_CONVERSION_REQUIRE, NULL,
              AU_NONE);

  // Load models list before converting
  modelslist.load(ModelsList::Format::txt);

  unsigned converted = 0;
  auto to_convert = modelslist.getModelsCount() + 1;

  drawProgressScreen(RADIO_FILENAME, converted, to_convert);
  TRACE("converting '%s' (%d/%d)", RADIO_FILENAME, converted, to_convert);

#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    convertRadioData_219_to_220(path);
    version = 220;
  }
#endif
#if STORAGE_CONVERSIONS < 220
  if (version == 220) {
    convertRadioData_220_to_221(path);
    version = 221;
  }
#endif
  converted++;

#if defined(SIMU)
  RTOS_WAIT_MS(200);
#endif

  const char* error = nullptr;
  for (auto category_ptr : modelslist.getCategories()) {

    auto model_it = category_ptr->begin();

    while(model_it != category_ptr->end()) {

      uint8_t model_version = 0;
      auto* model_ptr = *model_it;
      char* filename = model_ptr->modelFilename;

      TRACE("converting '%s' (%d/%d)", filename, converted, to_convert);
      drawProgressScreen(filename, converted, to_convert);

      // read only the version number (size=0)
      error = readModelBin(filename, nullptr, 0, &model_version);
      if (!error) {
        // TODO: error handling
        error = convertBinModelData(filename, model_version);
        ++model_it;

        if (error) {
          TRACE("ERROR converting '%s': %s", filename, error);
          category_ptr->removeModel(model_ptr);
        } else {
          PartialModel partial;
          memclear(&partial, sizeof(PartialModel));
          
          readModelYaml(filename, reinterpret_cast<uint8_t*>(&partial), sizeof(partial));
          model_ptr->setModelName(partial.header.name);
        }
      } else {
        TRACE("ERROR reading '%s': %s", filename, error);

        // remove that file from the models list
        ++model_it;
        category_ptr->removeModel(model_ptr);
      }

      converted++;

#if defined(SIMU)
      RTOS_WAIT_MS(200);
#endif
    }
  }

#if defined(SDCARD_YAML) || defined(STORAGE_MODELSLIST)
  modelslist.save();
  // trigger models list reload
  modelslist.clear();
#endif
}

void patchFilenameToYaml(char* str)
{
  constexpr unsigned bin_len = sizeof(MODELS_EXT) - 1;
  constexpr unsigned yml_len = sizeof(YAML_EXT) - 1;

  // patch file extension
  const char* ext = strrchr(str, '.');
  if (ext && (strlen(ext) == bin_len) &&
      !strncmp(ext, STR_MODELS_EXT, bin_len)) {
    memcpy((void*)ext, (void*)STR_YAML_EXT, yml_len + 1);
  }
}

const char* convertBinModelData(char* filename, int version)
{
  TRACE("convertModelData(%s)", filename);

  char path[FF_MAX_LFN + 1];
  memcpy(path, MODELS_PATH, sizeof(MODELS_PATH)-1);
  path[sizeof(MODELS_PATH)-1] = '/';
  strcpy(&path[sizeof(MODELS_PATH)], filename);
  
#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    const char* error = convertModelData_219_to_220(path);
    if (error) return error;
    version = 220;
  }
#endif
#if STORAGE_CONVERSIONS < 221
  if (version == 220) {
    const char* error = convertModelData_220_to_221(path);
    if (error) return error;
    version = 221;
  }
#endif

  patchFilenameToYaml(filename);
  return nullptr;
}
#endif

#if defined(EEPROM) || defined(EEPROM_RLC)
#include "storage/eeprom_common.h"

void eeConvertModel(int id, int version)
{
  TRACE("eeConvertModel(%d,%d)", id, version);
  
#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    convertModelData_219_to_220(id);
    version = 220;
  }
#endif
#if STORAGE_CONVERSIONS < 221
  if (version == 220) {
    convertModelData_220_to_221(id);
    version = 221;
  }
#endif
  // TODO: error handling
}

bool eeConvert(uint8_t start_version)
{
  const char *msg = NULL;

  switch (start_version) {
    case 219:
      msg = "EEprom Data v219";
      break;
    case 220:
      msg = "EEprom Data v220";
      break;
    default:
      return false;
  }

  // Information to the user and wait for key press
  g_eeGeneral.backlightMode = e_backlight_mode_on;
  g_eeGeneral.backlightBright = 0;
  g_eeGeneral.contrast = 25;

  ALERT(STR_STORAGE_WARNING, msg, AU_BAD_RADIODATA);

  RAISE_ALERT(STR_STORAGE_WARNING, STR_EEPROM_CONVERTING, NULL, AU_NONE);

  // General Settings conversion
  int version = start_version;
  (void)version;

  sdCheckAndCreateDirectory(RADIO_PATH);
  sdCheckAndCreateDirectory(MODELS_PATH);

#if STORAGE_CONVERSIONS < 220
  if (version == 219) {
    version = 220;
    convertRadioData_219_to_220();
  }
#endif
#if STORAGE_CONVERSIONS < 221
  if (version == 220) {
    version = 221;
    convertRadioData_220_to_221();
  }
#endif

#if defined(STORAGE_MODELSLIST)
  modelslist.clear();
#endif
  
#if LCD_W >= 212
  lcdDrawRect(60, 6*FH+4, 132, 3);
#else
  lcdDrawRect(10, 6*FH+4, 102, 3);
#endif

  // Models conversion
  for (uint8_t id=0; id<MAX_MODELS; id++) {
#if LCD_W >= 212
    lcdDrawSolidHorizontalLine(61, 6*FH+5, 10+id*2, FORCE);
#else
    lcdDrawSolidHorizontalLine(11, 6*FH+5, 10+(id*3)/2, FORCE);
#endif
    lcdRefresh();

#if defined(SIMU)
    RTOS_WAIT_MS(100);
#endif

#if defined(SDCARD_RAW) || defined(SDCARD_YAML)
    if (eeModelExistsRlc(id)) {
#else
    if (eeModelExists(id)) {
#endif
      eeConvertModel(id, start_version);
    }
  }

#if defined(STORAGE_MODELSLIST)
  modelslist.save();
#endif

  return true;
}
#endif
