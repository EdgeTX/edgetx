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

#include <string.h>

#include "board.h"
#include "lcd.h"

#include "translations.h"

#include "fw_version.h"
#include "translations/bl_translations.h"
#include "bootloader/boot.h"
#include "bootloader/firmware_files.h"

void bootloaderInitScreen()
{
  lcdInit();
  lcdSetContrast(true);

  backlightInit();
#if defined(LCD_BRIGHTNESS_DEFAULT)
  backlightEnable(LCD_BRIGHTNESS_DEFAULT);
#else
  backlightFullOn();
#endif
}

static void bootloaderDrawMsg(const char *str, uint8_t line, bool inverted)
{
  lcdDrawSizedText(LCD_W / 2, (line + 2) * FH, str, DISPLAY_CHAR_WIDTH, (inverted ? INVERS : 0) | CENTERED);
}

void bootloaderDrawFilename(const char *str, uint8_t line, bool selected)
{
  lcdDrawSizedText(INDENT_WIDTH, (line + 2) * FH, str, DISPLAY_CHAR_WIDTH, selected ? INVERS : 0);
}

bool checkFirmwareFlavor(const char * flavour)
{
  if (strncmp(flavour,FLAVOUR, sizeof(FLAVOUR) - 1) != 0)
    return false;

  char * tmp = (char *) flavour;
  while (*tmp != 0) tmp++;
  if ((tmp - flavour) != (sizeof(FLAVOUR) - 1))
    return false;

  return true;
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char *str)
{
  lcdClear();
  lcdDrawText(LCD_W / 2, 0, BOOTLOADER_TITLE, CENTERED);
  lcdInvertLine(0);

  if (st == ST_START) {
    lcdDrawText(3*FW, 2*FH, TR_BL_WRITE_FW, opt == 0 ? INVERS : 0);
    lcdDrawText(3*FW, 3*FH, TR_BL_EXIT, opt == 1 ? INVERS : 0);

    lcdDrawText(LCD_W / 2, 5 * FH + FH / 2, TR_BL_OR_PLUGIN_USB_CABLE, CENTERED);

    const char * vers = getFirmwareVersion();
#if LCD_W < 212
    // Remove "edgetx-" from string
    if (strncmp(vers, "edgetx-", 7) == 0)
      vers += 7;
#endif
    lcdDrawCenteredText(7 * FH, vers);
    lcdInvertLine(7);
  }
  else if (st == ST_USB) {
    lcdDrawCenteredText(4 * FH, TR_BL_USB_CONNECTED);
  }
  else if (st == ST_DIR_CHECK) {
    if (opt == FR_NO_PATH) {
      bootloaderDrawMsg(TR_BL_DIR_MISSING, 1, false);
      bootloaderDrawMsg(getFirmwarePath(), 2, false);
    }
    else {
      bootloaderDrawMsg(TR_BL_DIR_EMPTY, 1, false);
    }
  }
  else if (st == ST_FLASH_CHECK) {
    if (opt == FC_ERROR) {
      bootloaderDrawMsg(TR_BL_INVALID_FIRMWARE, 2, false);
    }
    else if (opt == FC_OK) {
      bool flavorCheck = false;
      VersionTag tag;
      getFileFirmwareVersion(&tag);
#if LCD_W < 212
      // Remove "edgetx-" from string
      flavorCheck = checkFirmwareFlavor(tag.flavour);
#else
      flavorCheck = checkFirmwareFlavor(tag.flavour);
#endif
      bootloaderDrawMsg(tag.version, 0, false);
      if (flavorCheck)
        bootloaderDrawMsg(TR_BL_HOLD_ENTER_TO_START, 2, false);
      else
        bootloaderDrawMsg(TR_BL_INVALID_FIRMWARE, 2, false);
    }
  }
  else if (st == ST_FLASHING) {
    lcdDrawCenteredText(4 * FH, TR_BL_WRITING_FW);

    lcdDrawRect(3, 6 * FH + 4, (LCD_W - 8), 7);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 6, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 7, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 8, (LCD_W - 12) * opt / 100, FORCE);
  }
  else if (st == ST_FLASH_DONE) {
    lcdDrawCenteredText(4 * FH, TR_BL_WRITING_COMPL);
  }
}

uint32_t bootloaderGetMenuItemCount(int baseCount)
{
    return baseCount;
}

bool bootloaderRadioMenu(uint32_t menuItem, event_t event)
{
    return true;
}

void blExit(void)
{
#if defined(RADIO_COMMANDO8)
      lcdClear();
      lcdDrawText(2, 22, TR_BL_POWER_KEY);
      lcdDrawText(2, 33, TR_BL_FLASH_EXIT);
      lcdRefresh();
      lcdRefreshWait();
      while(1);
#else
      lcdClear();
      lcdRefresh();
      lcdRefreshWait();
#endif
}
