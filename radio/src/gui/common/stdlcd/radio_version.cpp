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

#include "edgetx.h"
#include "options.h"
#include "hal/module_port.h"

#if defined(CROSSFIRE)
  #include "mixer_scheduler.h"
#endif

#include "fw_version.h"

#define MENU_BODY_TOP    (FH + 1)
#define MENU_BODY_BOTTOM (LCD_H)

#if defined(PXX2) || defined(CROSSFIRE)
constexpr uint8_t COLUMN2_X = 10 * FW;
#endif

void menuRadioFirmwareOptions(event_t event)
{
  title(STR_MENU_FIRM_OPTIONS);

  coord_t y = MENU_HEADER_HEIGHT + 1;
  lcdNextPos = INDENT_WIDTH;

  for (uint8_t i=0; options[i]; i++) {
    const char * option = options[i];

    if (i > 0) {
      lcdDrawText(lcdNextPos, y, ", ");
    }

    if (lcdNextPos + getTextWidth(option) + 5 > LCD_W) {
      lcdNextPos = INDENT_WIDTH;
      y += FH;
    }

    lcdDrawText(lcdNextPos, y, option);
  }

  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    popMenu();
  }
}

#if defined(PXX2)
void drawPXX2Version(coord_t x, coord_t y, PXX2Version version)
{
  if (version.major == 0xFF && version.minor == 0x0F && version.revision == 0x0F) {
    lcdDrawText(x, y, "---");
  }
  else {
    lcdDrawNumber(x, y, 1 + version.major, LEFT);
    lcdDrawChar(lcdNextPos, y, '.');
    lcdDrawNumber(lcdNextPos, y, version.minor, LEFT);
    lcdDrawChar(lcdNextPos, y, '.');
    lcdDrawNumber(lcdNextPos, y, version.revision, LEFT);
  }
}

void drawPXX2FullVersion(coord_t x, coord_t y, PXX2Version hwVersion, PXX2Version swVersion)
{
  drawPXX2Version(x, y, hwVersion);
  lcdDrawText(lcdNextPos, y, "/");
  drawPXX2Version(lcdNextPos, y, swVersion);
}
#endif
#if defined(PXX2) || defined(CROSSFIRE)
void menuRadioModulesVersion(event_t event)
{
  if (menuEvent) {
    for (uint8_t i = 0; i < MAX_MODULES; i++) {
      moduleState[i].mode = MODULE_MODE_NORMAL;
    }
    return;
  }

  title(STR_MENU_MODULES_RX_VERSION);
#if defined(PXX2)
if (event == EVT_ENTRY) {
    memclear(&reusableBuffer.hardwareAndSettings.modules, sizeof(reusableBuffer.hardwareAndSettings.modules));
  }

  if (event == EVT_ENTRY || get_tmr10ms() >= reusableBuffer.hardwareAndSettings.updateTime) {

#if defined(HARDWARE_INTERNAL_MODULE)
    if (isModulePXX2(INTERNAL_MODULE) && modulePortPowered(INTERNAL_MODULE)) {
      moduleState[INTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE], PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
    if (isModulePXX2(EXTERNAL_MODULE) && modulePortPowered(EXTERNAL_MODULE)) {
      moduleState[EXTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE], PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }
#endif

    reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms() + 1000 /* 10s*/;
  }
#endif

  coord_t y = (FH + 1) - menuVerticalOffset * FH;

  for (uint8_t module=0; module<NUM_MODULES; module++) {
    // Label
    if (y >= MENU_BODY_TOP && y < MENU_BODY_BOTTOM) {
#if defined(HARDWARE_INTERNAL_MODULE)
      if (module == INTERNAL_MODULE)
        lcdDrawTextAlignedLeft(y, STR_INTERNAL_MODULE);
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      if (module == EXTERNAL_MODULE)
        lcdDrawTextAlignedLeft(y, STR_EXTERNAL_MODULE);
#endif
    }
    y += FH;

    // Module model
    if (y >= MENU_BODY_TOP && y < MENU_BODY_BOTTOM) {
      lcdDrawTextIndented(y, STR_MODULE);
      bool module_off = true;
#if defined(HARDWARE_INTERNAL_MODULE)
      if (module == INTERNAL_MODULE && modulePortPowered(INTERNAL_MODULE))
        module_off = false;
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      if (module == EXTERNAL_MODULE && modulePortPowered(EXTERNAL_MODULE))
        module_off = false;
#endif
      if (module_off) {
        lcdDrawText(COLUMN2_X, y, STR_OFF);
        y += FH;
        continue;
      }
#if defined(MULTIMODULE)
      if (isModuleMultimodule(module)) {
        char statusText[64] = "";
        getMultiModuleStatus(module).getStatusString(statusText);
        lcdDrawText(COLUMN2_X, y, statusText);
        y += FH;
        continue;
      }
#endif
#if defined(CROSSFIRE)
      if (isModuleCrossfire(module)) {
        char statusText[64] = "";
        sprintf(statusText, "%d Hz"/* %" PRIu32" Err"*/,
                1000000 / getMixerSchedulerPeriod()/*, UINT32_C(telemetryErrors)*/);
        lcdDrawText(COLUMN2_X, y, statusText);
        y += FH;
        lcdDrawText(INDENT_WIDTH, y, crossfireModuleStatus[module].name);
        lcdDrawChar(lcdNextPos + 5, y, 'V');
        lcdDrawNumber(lcdNextPos, y, crossfireModuleStatus[module].major);
        lcdDrawChar(lcdNextPos, y, '.');
        lcdDrawNumber(lcdNextPos, y, crossfireModuleStatus[module].minor);
        lcdDrawChar(lcdNextPos, y, '.');
        lcdDrawNumber(lcdNextPos, y, crossfireModuleStatus[module].revision);
        y += FH;
        continue;
      }
#endif
      if (!isModulePXX2(module)) {
        lcdDrawText(COLUMN2_X, y, STR_NO_INFORMATION);
        y += FH;
        continue;
      }
#if defined(PXX2)
      uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[module]
                            .information.modelID;
      lcdDrawText(COLUMN2_X, y, getPXX2ModuleName(modelId));
#endif
    }
    y += FH;
#if defined(PXX2)
    // Module version
    if (y >= MENU_BODY_TOP && y < MENU_BODY_BOTTOM) {
      if (reusableBuffer.hardwareAndSettings.modules[module].information.modelID) {
        drawPXX2FullVersion(COLUMN2_X, y, reusableBuffer.hardwareAndSettings.modules[module].information.hwVersion, reusableBuffer.hardwareAndSettings.modules[module].information.swVersion);
        static const char * variants[] = {"FCC", "EU", "FLEX"};
        uint8_t variant = reusableBuffer.hardwareAndSettings.modules[module].information.variant - 1;
        if (variant < DIM(variants)) {
          lcdDrawText(lcdNextPos + 1, y, variants[variant]);
        }
      }
    }
    y += FH;

    for (uint8_t receiver=0; receiver<PXX2_MAX_RECEIVERS_PER_MODULE; receiver++) {
      if (reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.modelID && reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].timestamp < get_tmr10ms() + 2000) {
        // Receiver model
        if (y >= MENU_BODY_TOP && y < MENU_BODY_BOTTOM) {
          lcdDrawTextIndented(y, STR_RECEIVER);
          lcdDrawNumber(lcdLastRightPos + 2, y, receiver + 1);
          uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.modelID;
          lcdDrawText(COLUMN2_X, y, getPXX2ReceiverName(modelId));
        }
        y += FH;

        // Receiver version
        if (y >= MENU_BODY_TOP && y < MENU_BODY_BOTTOM) {
          drawPXX2FullVersion(COLUMN2_X, y, reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.hwVersion, reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.swVersion);
        }
        y += FH;
      }
    }
#endif
  }

  uint8_t lines = (y - (FH + 1)) / FH + menuVerticalOffset;
  if (lines > NUM_BODY_LINES) {
    drawVerticalScrollbar(LCD_W-1, FH, LCD_H-FH, menuVerticalOffset, lines, NUM_BODY_LINES);
  }

  if (IS_PREVIOUS_EVENT(event)) {
    if (lines > NUM_BODY_LINES) {
      if (menuVerticalOffset-- == 0)
        menuVerticalOffset = lines - 1;
    }
  } else if (IS_NEXT_EVENT(event)) {
    if (lines > NUM_BODY_LINES) {
      if (++menuVerticalOffset + NUM_BODY_LINES > lines)
        menuVerticalOffset = 0;
    }
  } else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    if (menuVerticalOffset != 0)
      menuVerticalOffset = 0;
    else
      popMenu();
  }
}
#endif

enum MenuRadioVersionItems
{
  ITEM_RADIO_VERSION_FIRST = HEADER_LINE - 1,
#if defined(PCBTARANIS)
  ITEM_RADIO_FIRMWARE_OPTIONS,
#endif
#if defined(PXX2) || defined(CROSSFIRE)
  ITEM_RADIO_MODULES_VERSION,
#endif
  ITEM_RADIO_VERSION_COUNT
};

void menuRadioVersion(event_t event)
{
  SIMPLE_MENU(STR_MENUVERSION, menuTabGeneral, MENU_RADIO_VERSION, ITEM_RADIO_VERSION_COUNT);

  coord_t y = MENU_HEADER_HEIGHT + 2;
  lcdDrawText(FW, y, vers_stamp, SMLSIZE);
  y += 5 * (FH - 1) + 2;

#if defined(PCBTARANIS)
  lcdDrawText(INDENT_WIDTH, y, BUTTON(TR_FIRMWARE_OPTIONS), menuVerticalPosition == ITEM_RADIO_FIRMWARE_OPTIONS ? INVERS : 0);
  y += FH;
  if (menuVerticalPosition == ITEM_RADIO_FIRMWARE_OPTIONS && event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = EDIT_SELECT_FIELD;
    pushMenu(menuRadioFirmwareOptions);
  }
#endif

#if defined(PXX2) || defined(CROSSFIRE)
  lcdDrawText(INDENT_WIDTH, y, BUTTON(TR_MODULES_RX_VERSION), menuVerticalPosition == ITEM_RADIO_MODULES_VERSION ? INVERS : 0);
  y += FH;
  if (menuVerticalPosition == ITEM_RADIO_MODULES_VERSION && event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = EDIT_SELECT_FIELD;
    pushMenu(menuRadioModulesVersion);
  }
#endif
}
