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

#pragma once

#include <stdint.h>
#include "stamp.h"
#include "keys.h"

#include "hal/flash_driver.h"

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
#if defined(SPI_FLASH)
  ST_CLEAR_FLASH_CHECK,
  ST_CLEAR_FLASH,
#endif
  ST_RADIO_MENU,
  ST_REBOOT,
};

enum FlashCheckRes {
    FC_UNCHECKED=0,
    FC_OK,
    FC_ERROR
};

typedef struct {
  const char* description;
} flash_media_t;

void bootloaderRegisterFlash(const char* desc, etx_flash_driver_t* drv);

// Declarations of functions that need to be implemented
// for each target with a bootloader

// On bootloader start after lcdInit()
void bootloaderInitScreen();

// Depending on the state, up to two optional parameters are passed.
// See boot.cpp/main for more details
void bootloaderDrawScreen(BootloaderState st, int opt, const char* str = nullptr);

// Once for each file in a filename list on screen
void bootloaderDrawFilename(const char* str, uint8_t line, bool selected);

// get menu item count for main menu, by adding 1 to the baseCount, when a radio specific menu is used
uint32_t bootloaderGetMenuItemCount(int baseCount);

// when a radio specifc menu is used, all events are forwarded to that menu code
// returns true on submenu exit
bool bootloaderRadioMenu(uint32_t menuItem, event_t event);

void blExit();
