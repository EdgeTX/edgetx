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

#ifndef _boot_h_
#define _boot_h_

#include "stamp.h"

#if LCD_W < 212
  #if defined(VERSION_TAG)
    #define BOOTLOADER_TITLE               " Bootloader - " VERSION_TAG
  #else
    #define BOOTLOADER_TITLE               " Bootloader " VERSION "-" VERSION_SUFFIX
  #endif
#else
  #if defined(VERSION_TAG)
    #define BOOTLOADER_TITLE               " EdgeTX Bootloader - " VERSION_TAG
  #else
    #define BOOTLOADER_TITLE               " EdgeTX Bootloader - " VERSION "-" VERSION_SUFFIX
  #endif
#endif

#define DISPLAY_CHAR_WIDTH             (LCD_COLS+4)

#if LCD_W >= 480
  #define STR_INVALID_FIRMWARE         "Not a valid firmware file"
#elif LCD_W >= 212
  #define STR_OR_PLUGIN_USB_CABLE      "Or plug in a USB cable for mass storage"
  #define STR_HOLD_ENTER_TO_START      "\012Hold [ENT] to start writing"
  #define STR_INVALID_FIRMWARE         "\011Not a valid firmware file!        "
  #define STR_INVALID_EEPROM           "\011Not a valid EEPROM file!          "
#else
  #define STR_OR_PLUGIN_USB_CABLE      "Or plug in a USB cable"
  #define STR_HOLD_ENTER_TO_START      "\006Hold [ENT] to start"
  #define STR_INVALID_FIRMWARE         "\004Not a valid firmware!        "
  #define STR_INVALID_EEPROM           "\004Not a valid EEPROM!          "
#endif

#define STR_USB_CONNECTED              CENTER "\011USB Connected"


// Bootloader states
enum BootloaderState {
  ST_START,
  ST_FLASH_MENU,
  ST_DIR_CHECK,
  ST_OPEN_DIR,
  ST_FILE_LIST,
  ST_FLASH_CHECK,
  ST_FLASHING,
  ST_FLASH_DONE,
  ST_RESTORE_MENU,
  ST_USB,
  ST_REBOOT,
};

enum FlashCheckRes {
    FC_UNCHECKED=0,
    FC_OK,
    FC_ERROR
};

// Declarations of functions that need to be implemented
// for each target with a bootloader

// On bootloader start after lcdInit()
void bootloaderInitScreen();

// Depending on the state, up to two optional parameters are passed.
// See boot.cpp/main for more details
void bootloaderDrawScreen(BootloaderState st, int opt, const char* str = NULL);

// Once for each file in a filename list on screen
void bootloaderDrawFilename(const char* str, uint8_t line, bool selected);

#endif
