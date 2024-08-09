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

#define RECEIVER_OPTIONS_2ND_COLUMN 80

extern uint8_t g_moduleIdx;

enum {
  RECEIVER_SETTINGS_OK = 0,
  RECEIVER_SETTINGS_DIRTY = 1,
  RECEIVER_SETTINGS_WRITING = 2,
};

void onRxOptionsUpdateConfirm(const char * result)
{
  if (result == STR_OK) {
    reusableBuffer.hardwareAndSettings.receiverSettings.state = PXX2_SETTINGS_WRITE;
    reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_WRITING;
    reusableBuffer.hardwareAndSettings.receiverSettings.timeout = 0;
    moduleState[g_moduleIdx].mode = MODULE_MODE_RECEIVER_SETTINGS;
  }
  else {
    popMenu();
  }
}

enum {
  ITEM_RECEIVER_SETTINGS_PWM_RATE,
  ITEM_RECEIVER_SETTINGS_TELEMETRY,
  ITEM_RECEIVER_SETTINGS_TELEMETRY_25MW,
  ITEM_RECEIVER_SETTINGS_SPORT_MODE,
  ITEM_RECEIVER_SETTINGS_SBUS24,
  ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED1,
  ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED2,
  ITEM_RECEIVER_SETTINGS_PINMAP_FIRST
};

#define IS_RECEIVER_CAPABILITY_ENABLED(capability)    (reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilities & (1 << capability))
#define IF_RECEIVER_CAPABILITY(capability, count)     uint8_t(IS_RECEIVER_CAPABILITY_ENABLED(capability) ? count : HIDDEN_ROW)

bool isSPortModeAvailable(int mode)
{
  uint8_t receiverId = reusableBuffer.hardwareAndSettings.receiverSettings.receiverId;
  if (mode == 2 && !IS_RECEIVER_CAPABILITY_ENABLED(RECEIVER_CAPABILITY_FPORT2)) {
    return false;
  }
  return true;
}

void menuModelReceiverOptions(event_t event)
{
  const int lim = (g_model.extendedLimits ? (512 * LIMIT_EXT_PERCENT / 100) : 512) * 2;
  uint8_t wbar = LCD_W / 2 - 20;
  auto outputsCount = min<uint8_t>(16, reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount);

  if (event == EVT_ENTRY) {
    // reusableBuffer.hardwareSettings should have been cleared before calling this menu
#if defined(SIMU)
    auto& hwSettings = getPXX2HardwareAndSettingsBuffer();
    memclear(&hwSettings, sizeof(hwSettings));
    hwSettings.moduleSettings.state = PXX2_SETTINGS_OK;
    hwSettings.receiverSettings.state = PXX2_SETTINGS_OK;
    moduleState[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    auto& rxInfo = hwSettings.modules[0].receivers[0].information;
    rxInfo.capabilities = 0x62;
    hwSettings.receiverSettings.outputsCount = 6;
    for (int i=0; i<6; i++) {
      hwSettings.receiverSettings.outputsMapping[i] = i;
    }
#endif
  }

  uint8_t receiverId = reusableBuffer.hardwareAndSettings.receiverSettings.receiverId;
  uint8_t receiverModelId = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.modelID;
  uint8_t receiverVariant = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.variant;

  SUBMENU_NOTITLE(ITEM_RECEIVER_SETTINGS_PINMAP_FIRST + outputsCount, {
    0, // ITEM_RECEIVER_SETTINGS_PWM_RATE
    isModuleR9MAccess(g_moduleIdx) && receiverVariant == PXX2_VARIANT_EU && reusableBuffer.hardwareAndSettings.moduleSettings.txPower > 14 /*25mW*/ ? READONLY_ROW : (uint8_t)0, // ITEM_RECEIVER_SETTINGS_TELEMETRY
    IF_RECEIVER_CAPABILITY(RECEIVER_CAPABILITY_TELEMETRY_25MW, 0), // ITEM_RECEIVER_SETTINGS_TELEMETRY_25MW
    uint8_t((IS_RECEIVER_CAPABILITY_ENABLED(RECEIVER_CAPABILITY_FPORT) || IS_RECEIVER_CAPABILITY_ENABLED(RECEIVER_CAPABILITY_FPORT2)) ? 0 : HIDDEN_ROW), // ITEM_RECEIVER_SETTINGS_SPORT_MODE
    uint8_t(IS_RECEIVER_CAPABILITY_ENABLED(RECEIVER_CAPABILITY_SBUS24) ? 0 : HIDDEN_ROW), // ITEM_RECEIVER_SETTINGS_SBUS24
    uint8_t(reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilityNotSupported ? READONLY_ROW : HIDDEN_ROW),  // ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED1
    uint8_t(reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].receivers[receiverId].information.capabilityNotSupported ? READONLY_ROW : HIDDEN_ROW),  // ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED2
    0 // ITEM_RECEIVER_SETTINGS_PINMAP_FIRST ...
  });

  if (menuEvent) {
    killEvents(KEY_EXIT); // TODO: Check if needed
    moduleState[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.hardwareAndSettings.receiverSettings.dirty) {
      abortPopMenu();
      POPUP_CONFIRMATION(STR_UPDATE_RX_OPTIONS, onRxOptionsUpdateConfirm);
    }
    else {
      return;
    }
  }

  if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_HARDWARE_INFO && moduleState[g_moduleIdx].mode == MODULE_MODE_NORMAL) {
    if (!receiverModelId)
      moduleState[g_moduleIdx].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[g_moduleIdx], receiverId, receiverId);
    else if (isModuleR9MAccess(g_moduleIdx) && receiverVariant == PXX2_VARIANT_EU && !reusableBuffer.hardwareAndSettings.moduleSettings.txPower)
      moduleState[g_moduleIdx].readModuleSettings(&reusableBuffer.hardwareAndSettings.moduleSettings);
    else
      moduleState[g_moduleIdx].readReceiverSettings(&reusableBuffer.hardwareAndSettings.receiverSettings);
  }

  if (event == EVT_KEY_LONG(KEY_ENTER) && reusableBuffer.hardwareAndSettings.receiverSettings.dirty) {
    killEvents(event);
    reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_OK;
    moduleState[g_moduleIdx].writeReceiverSettings(&reusableBuffer.hardwareAndSettings.receiverSettings);
  }

  if (reusableBuffer.hardwareAndSettings.receiverSettings.dirty == RECEIVER_SETTINGS_WRITING && reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
    popMenu();
    return;
  }

  if (receiverModelId && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
    menuVerticalPosition = 0;
    while (menuVerticalPosition < ITEM_RECEIVER_SETTINGS_PINMAP_FIRST && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
      ++menuVerticalPosition;
    }
  }

  int8_t sub = menuVerticalPosition;
  lcdDrawTextAlignedLeft(0, STR_RECEIVER_OPTIONS);
  drawReceiverName(lcdNextPos + FW, 0, g_moduleIdx, reusableBuffer.hardwareAndSettings.receiverSettings.receiverId);
  lcdInvertLine(0);

  if (reusableBuffer.hardwareAndSettings.receiverSettings.state == PXX2_SETTINGS_OK) {
    for (uint8_t k = 0; k < LCD_LINES - 1; k++) {
      coord_t y = MENU_HEADER_HEIGHT + 1 + k * FH;
      uint8_t i = k + menuVerticalOffset;
      for (int j = 0; j <= i; ++j) {
        if (j < (int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
          ++i;
        }
      }
      LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

      switch (i) {
        case ITEM_RECEIVER_SETTINGS_PWM_RATE:
          reusableBuffer.hardwareAndSettings.receiverSettings.pwmRate = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.pwmRate, RECEIVER_OPTIONS_2ND_COLUMN, y, isModuleR9MAccess(g_moduleIdx) ? "6.67ms PWM": "7ms PWM", attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_TELEMETRY:
          reusableBuffer.hardwareAndSettings.receiverSettings.telemetryDisabled = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.telemetryDisabled, RECEIVER_OPTIONS_2ND_COLUMN, y, STR_TELEMETRY_DISABLED, attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_TELEMETRY_25MW:
          reusableBuffer.hardwareAndSettings.receiverSettings.telemetry25mw = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.telemetry25mw, RECEIVER_OPTIONS_2ND_COLUMN, y, "25mw Tele", attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_SPORT_MODE:
        {
          lcdDrawText(0, y, STR_PROTOCOL);
          uint8_t portType = reusableBuffer.hardwareAndSettings.receiverSettings.fport | (reusableBuffer.hardwareAndSettings.receiverSettings.fport2 << 1);
          lcdDrawTextAtIndex(RECEIVER_OPTIONS_2ND_COLUMN, y, STR_SPORT_MODES, portType, attr);
          portType = checkIncDec(event, portType, 0, 2, EE_MODEL, isSPortModeAvailable);
          if (checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.fport = portType & 0x01;
            reusableBuffer.hardwareAndSettings.receiverSettings.fport2 = (portType & 0x02) >> 1;
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
          }
          break;
        }

        case ITEM_RECEIVER_SETTINGS_SBUS24:
          reusableBuffer.hardwareAndSettings.receiverSettings.sbus24 = editCheckBox(reusableBuffer.hardwareAndSettings.receiverSettings.sbus24, RECEIVER_OPTIONS_2ND_COLUMN, y, STR_SBUS24, attr, event);
          if (attr && checkIncDec_Ret) {
            reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
          }
          break;

        case ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED1:
          lcdDrawText(LCD_W/2, y+1, STR_MORE_OPTIONS_AVAILABLE, SMLSIZE|CENTERED);
          break;

        case ITEM_RECEIVER_SETTINGS_CAPABILITY_NOT_SUPPORTED2:
          lcdDrawText(LCD_W/2, y+1, STR_EDGETX_UPGRADE_REQUIRED, SMLSIZE|CENTERED);
          break;

        default:
        // Pin
        {
          uint8_t pin = i - ITEM_RECEIVER_SETTINGS_PINMAP_FIRST;
          if (pin < reusableBuffer.hardwareAndSettings.receiverSettings.outputsCount) {
            uint8_t mapping = reusableBuffer.hardwareAndSettings.receiverSettings.outputsMapping[pin];
            lcdDrawText(0, y, STR_PIN);
            lcdDrawNumber(lcdLastRightPos + 1, y, pin + 1);

            uint8_t channelMax = sentModuleChannels(g_moduleIdx) - 1;
            uint8_t selectionMax = channelMax;

            if (mapping <= channelMax) {
              uint8_t channel = g_model.moduleData[g_moduleIdx].channelsStart + mapping;
              putsChn(7 * FW, y, channel + 1, attr);
            }

            if (isPXX2ReceiverOptionAvailable(receiverModelId, RECEIVER_OPTION_D_TELE_PORT)) {
              if (mapping == CH_MAP_SPORT) {
                lcdDrawText(7 * FW, y, STR_SPORT, attr);
                mapping = channelMax + 1;
              } else if (mapping == CH_MAP_SBUS_OUT) {
                lcdDrawText(7 * FW, y, STR_SBUSOUT, attr);
                mapping = channelMax + 2;
              } else if (mapping == CH_MAP_FBUS) {
                lcdDrawText(7 * FW, y, STR_FBUS, attr);
                mapping = channelMax + 3;
              }
              if (pin == 0) {
                selectionMax = channelMax + 4;
                if (mapping == CH_MAP_SBUS_IN) {
                  lcdDrawText(7 * FW, y, STR_SBUSIN, attr);
                  mapping = selectionMax;
                }
              }
              else {
                selectionMax = channelMax + 3;
              }
            } else if (IS_RECEIVER_CAPABILITY_ENABLED(RECEIVER_CAPABILITY_ENABLE_PWM_CH5_CH6)) {
              if (CH_ENABLE_SPORT == pin) {
                if (++selectionMax == mapping) {
                  lcdDrawText(7 * FW, y, STR_SPORT, attr);
                }
              } else if (CH_ENABLE_SBUS == pin) {
                if (++selectionMax == mapping) {
                  lcdDrawText(7 * FW, y, STR_SBUSOUT, attr);
                }
              }
            }

            // Channel
            if (attr) {
              mapping = checkIncDec(event, mapping, 0, selectionMax);
              if (checkIncDec_Ret) {
                if (isPXX2ReceiverOptionAvailable(receiverModelId, RECEIVER_OPTION_D_TELE_PORT)) {
                  if (mapping == channelMax + 1) {
                    mapping = CH_MAP_SPORT;
                  } else if (mapping == channelMax + 2) {
                    mapping = CH_MAP_SBUS_OUT;
                  } else if (mapping == channelMax + 3) {
                    mapping = CH_MAP_FBUS;
                  } else if (mapping == channelMax + 4) {
                    mapping = CH_MAP_SBUS_IN;
		  }
                }
                reusableBuffer.hardwareAndSettings.receiverSettings.outputsMapping[pin] = mapping;
                reusableBuffer.hardwareAndSettings.receiverSettings.dirty = RECEIVER_SETTINGS_DIRTY;
              }
            }

            // Bargraph
            if (mapping <= channelMax) {
              uint8_t channel = g_model.moduleData[g_moduleIdx].channelsStart + mapping;
              int32_t channelValue = channelOutputs[channel];
  #if !(defined(PCBX7) || defined(PCBX9LITE) || defined(PCBX9LITES)) // X7/X9 LCD doesn't like too many horizontal lines
              lcdDrawRect(RECEIVER_OPTIONS_2ND_COLUMN, y + 2, wbar + 1, 4);
  #endif
              auto lenChannel = limit<uint8_t>(1, (abs(channelValue) * wbar / 2 + lim / 2) / lim, wbar / 2);
              auto xChannel = (channelValue > 0) ? RECEIVER_OPTIONS_2ND_COLUMN + wbar / 2 : RECEIVER_OPTIONS_2ND_COLUMN + wbar / 2 + 1 - lenChannel;
              lcdDrawHorizontalLine(xChannel, y + 3, lenChannel, SOLID, 0);
              lcdDrawHorizontalLine(xChannel, y + 4, lenChannel, SOLID, 0);
            }
          }
          break;
        }
      }
    }
  }
  else {
    lcdDrawCenteredText(LCD_H/2, STR_WAITING_FOR_RX);
  }
}
