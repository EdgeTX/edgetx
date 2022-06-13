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
#include "fw_version.h"
#include "translations.h"
#include "../../common/arm/stm32/bootloader/boot.h"
#include "../../common/arm/stm32/bootloader/bin_files.h"

extern MemoryType memoryType;

void bootloaderInitScreen()
{
  lcdSetContrast(true);
}

static void bootloaderDrawMsg(unsigned int x, const char *str, uint8_t line, bool inverted)
{
  lcdDrawSizedText(x, (line + 2) * FH, str, DISPLAY_CHAR_WIDTH, inverted ? INVERS : 0);
}

void bootloaderDrawFilename(const char *str, uint8_t line, bool selected)
{
  bootloaderDrawMsg(INDENT_WIDTH, str, line, selected);
}

void bootloaderDrawScreen(BootloaderState st, int opt, const char *str)
{
  lcdClear();
  lcdDrawText(LCD_W / 2, 0, BOOTLOADER_TITLE, CENTERED);
  lcdInvertLine(0);

  if (st == ST_START) {
    lcdDrawText(3*FW, 2*FH, "Write Firmware", opt == 0 ? INVERS : 0);
#if defined(EEPROM)
    lcdDrawText(3*FW, 3*FH, "Restore EEPROM", opt == 1 ? INVERS : 0);
    lcdDrawText(3*FW, 4*FH, "Exit", opt == 2 ? INVERS : 0);
#else
    lcdDrawText(3*FW, 3*FH, "Exit", opt == 1 ? INVERS : 0);
#endif    

    lcdDrawText(LCD_W / 2, 5 * FH + FH / 2, STR_OR_PLUGIN_USB_CABLE, CENTERED);

    // Remove "opentx-" from string
    const char * vers = getFirmwareVersion();
#if LCD_W < 212
    if (strncmp(vers, "opentx-", 7) == 0)
      vers += 7;
#endif
    lcdDrawText(LCD_W / 2, 7 * FH, vers, CENTERED);
    lcdInvertLine(7);
  }
  else if (st == ST_USB) {
    lcdDrawTextAlignedLeft(4 * FH, STR_USB_CONNECTED);
  }
  else if (st == ST_DIR_CHECK) {
    if (opt == FR_NO_PATH) {
      bootloaderDrawMsg(INDENT_WIDTH, "Directory is missing!", 1, false);
      bootloaderDrawMsg(INDENT_WIDTH, getBinaryPath(memoryType), 2, false);
    }
    else {
      bootloaderDrawMsg(INDENT_WIDTH, "Directory is empty!", 1, false);
    }
  }
  else if (st == ST_FLASH_CHECK) {
    if (opt == FC_ERROR) {
      if (memoryType == MEM_FLASH)
        bootloaderDrawMsg(0, STR_INVALID_FIRMWARE, 2, false);
      else
        bootloaderDrawMsg(0, STR_INVALID_EEPROM, 2, false);
    }
    else if (opt == FC_OK) {
      if (memoryType == MEM_FLASH) {
        const char * vers = getFirmwareVersion((const char *)Block_buffer);
#if LCD_W < 212
        // Remove "opentx-" from string
        if (strncmp(vers, "opentx-", 7) == 0)
          vers += 7;
#endif
        bootloaderDrawMsg(INDENT_WIDTH, vers, 0, false);
      }
      bootloaderDrawMsg(0, STR_HOLD_ENTER_TO_START, 2, false);
    }
  }
  else if (st == ST_FLASHING) {
    lcdDrawTextAlignedLeft(4 * FH, CENTER "\015Writing...");

    lcdDrawRect(3, 6 * FH + 4, (LCD_W - 8), 7);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 6, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 7, (LCD_W - 12) * opt / 100, FORCE);
    lcdDrawSolidHorizontalLine(5, 6 * FH + 8, (LCD_W - 12) * opt / 100, FORCE);
  }
  else if (st == ST_FLASH_DONE) {
    lcdDrawTextAlignedLeft(4 * FH, CENTER "\007Writing complete");
  }
}
