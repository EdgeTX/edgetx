/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include "conversions.h"

void convertRadioData(int version)
{
  TRACE("convertRadioData(%d)", version);

#if STORAGE_CONVERSIONS < 217
  if (version == 216) {
    version = 217;
    convertRadioData_216_to_217(g_eeGeneral);
  }
#endif

#if STORAGE_CONVERSIONS < 218
  if (version == 217) {
    version = 218;
    convertRadioData_217_to_218(g_eeGeneral);
  }
#endif

#if STORAGE_CONVERSIONS < 219
  if (version == 218) {
    version = 219;
    convertRadioData_218_to_219(g_eeGeneral);
  }
#endif
}

void convertModelData(int version)
{
  TRACE("convertModelData(%d)", version);

#if STORAGE_CONVERSIONS < 217
  if (version == 216) {
    version = 217;
    convertModelData_216_to_217(g_model);
  }
#endif

#if STORAGE_CONVERSIONS < 218
  if (version == 217) {
    version = 218;
    convertModelData_217_to_218(g_model);
  }
#endif

#if STORAGE_CONVERSIONS < 219
  if (version == 218) {
    version = 219;
    convertModelData_218_to_219(g_model);
  }
#endif
}

#if defined(EEPROM)
void eeConvertModel(int id, int version)
{
  eeLoadModelData(id);
  convertModelData(version);
  uint8_t currModel = g_eeGeneral.currModel;
  g_eeGeneral.currModel = id;
  storageDirty(EE_MODEL);
  storageCheck(true);
  g_eeGeneral.currModel = currModel;
}

bool eeConvert()
{
  const char *msg = NULL;

  switch (g_eeGeneral.version) {
    case 216:
      msg = "EEprom Data v216";
      break;
    case 217:
      msg = "EEprom Data v217";
      break;
    case 218:
      msg = "EEprom Data v218";
      break;
    default:
      return false;
  }

  int conversionVersionStart = g_eeGeneral.version;

  // Information to the user and wait for key press
#if defined(PCBSKY9X)
  g_eeGeneral.optrexDisplay = 0;
#endif
  g_eeGeneral.backlightMode = e_backlight_mode_on;
  g_eeGeneral.backlightBright = 0;
  g_eeGeneral.contrast = 25;

  ALERT(STR_STORAGE_WARNING, msg, AU_BAD_RADIODATA);

  RAISE_ALERT(STR_STORAGE_WARNING, STR_EEPROM_CONVERTING, NULL, AU_NONE);

  // General Settings conversion
  eeLoadGeneralSettingsData();
  int version = conversionVersionStart;

#if STORAGE_CONVERSIONS < 217
  if (version == 216) {
    version = 217;
    convertRadioData_216_to_217(g_eeGeneral);
  }
#endif

#if STORAGE_CONVERSIONS < 218
  if (version == 217) {
    version = 218;
    convertRadioData_217_to_218(g_eeGeneral);
  }
#endif

#if STORAGE_CONVERSIONS < 219
  if (version == 218) {
    version = 219;
    convertRadioData_218_to_219(g_eeGeneral);
  }
#endif

  storageDirty(EE_GENERAL);
  storageCheck(true);

#if defined(COLORLCD)
#elif LCD_W >= 212
  lcdDrawRect(60, 6*FH+4, 132, 3);
#else
  lcdDrawRect(10, 6*FH+4, 102, 3);
#endif

  // Models conversion
  for (uint8_t id=0; id<MAX_MODELS; id++) {
#if defined(COLORLCD)
#elif LCD_W >= 212
    lcdDrawSolidHorizontalLine(61, 6*FH+5, 10+id*2, FORCE);
#else
    lcdDrawSolidHorizontalLine(11, 6*FH+5, 10+(id*3)/2, FORCE);
#endif
    lcdRefresh();
    if (eeModelExists(id)) {
      eeConvertModel(id, conversionVersionStart);
    }
  }

  return true;
}
#endif
