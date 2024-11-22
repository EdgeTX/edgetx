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

#include <QCoreApplication>

#define CPN_MAX_TIMERS                 3
#define CPN_MAX_FLIGHT_MODES           9
#define CPN_MAX_MIXERS                 64
#define CPN_MAX_INPUTS                 32
#define CPN_MAX_EXPOS                  64
#define CPN_MAX_CURVES                 32
#define CPN_MAX_POINTS                 17
#define CPN_MAX_GVARS                  9
#define CPN_MAX_ENCODERS               2  // rotary encoders
#define CPN_MAX_CHNOUT                 32 // number of real output channels
#define CPN_MAX_LOGICAL_SWITCHES       64 // number of custom switches
#define CPN_MAX_SPECIAL_FUNCTIONS      64 // number of functions assigned to switches
#define CPN_MAX_MODULES                2
#define CPN_MAX_STICKS                 Board::STICK_AXIS_COUNT
#define CPN_MAX_TRIMS                  Board::TRIM_AXIS_COUNT
#define CPN_MAX_CYC                    3
#define CPN_MAX_SWITCHES_FLEX          4 // v2.10 cmake parameter FLEXSW
#define CPN_MAX_SWITCHES_FUNCTION      6
#define CPN_MAX_SWITCHES_STD           20
#define CPN_MAX_SWITCHES               (CPN_MAX_SWITCHES_STD + CPN_MAX_SWITCHES_FLEX + CPN_MAX_SWITCHES_FUNCTION)
#define CPN_MAX_SENSORS                60
#define CPN_MAX_SCRIPTS                9
#define CPN_MAX_SCRIPT_INPUTS          10
#define CPN_MAX_SPACEMOUSE             6
#define CPN_MAX_INPUTS                 32 // v2.10 replaces CPN_MAX_ANALOGS - the value is abitary as radio ADC refactor is still a WIP
#define CPN_MAX_FUNCTIONSWITCH_GROUP   3

// pre v2.10
#define CPN_MAX_POTS                   8
#define CPN_MAX_SLIDERS                4
#define CPN_MAX_MOUSE_ANALOGS          2
#define CPN_MAX_GYRO_ANALOGS           2
#define CPN_MAX_ANALOGS                (CPN_MAX_STICKS + CPN_MAX_POTS + CPN_MAX_SLIDERS + CPN_MAX_MOUSE_ANALOGS + CPN_MAX_GYRO_ANALOGS)
// =========

#define CPN_STR_APP_NAME               QCoreApplication::translate("Companion", "EdgeTX Companion")
#define CPN_STR_SIMU_NAME              QCoreApplication::translate("Companion", "EdgeTX Simulator")
#define CPN_STR_TTL_INFO               QCoreApplication::translate("Companion", "Information")        // shared Title Case words, eg. for a window title or section heading
#define CPN_STR_TTL_WARNING            QCoreApplication::translate("Companion", "Warning")
#define CPN_STR_TTL_ERROR              QCoreApplication::translate("Companion", "Error")
#define CPN_STR_TTL_ACCEPT             QCoreApplication::translate("Companion", "Accept")
#define CPN_STR_TTL_DECLINE            QCoreApplication::translate("Companion", "Decline")

#define CPN_STR_FILES                  QCoreApplication::translate("Companion", "files")
#define CPN_STR_RAD_MOD_SETTINGS       QCoreApplication::translate("Companion", "Radio and Models settings")
#define HEX_FILES_FILTER               "HEX " % CPN_STR_FILES % " (*.hex);;"
#define BIN_FILES_FILTER               "BIN " % CPN_STR_FILES % " (*.bin);;"
#define DFU_FILES_FILTER               "DFU " % CPN_STR_FILES % " (*.dfu);;"
#define EEPE_FILES_FILTER              "EEPE " % CPN_STR_FILES % " (*.eepe);;"
#define ETX_FILES_FILTER               "EdgeTX " % CPN_STR_FILES % " (*.etx);;"
#define YML_FILES_FILTER               "EdgeTX YAML " % CPN_STR_FILES % " (*.yml);;"
#define EEPROM_FILES_FILTER            CPN_STR_RAD_MOD_SETTINGS % " " % CPN_STR_FILES % " (*.etx *.yml);;" % ETX_FILES_FILTER % YML_FILES_FILTER
#define FLASH_FILES_FILTER             "FLASH " % CPN_STR_FILES % " (*.bin *.hex *.dfu);;" % BIN_FILES_FILTER % HEX_FILES_FILTER % DFU_FILES_FILTER
#define EXTERNAL_EEPROM_FILES_FILTER   "EEPROM " % CPN_STR_FILES % " (*.bin *.hex);;" % BIN_FILES_FILTER % HEX_FILES_FILTER
#define ER9X_EEPROM_FILE_TYPE          "ER9X_EEPROM_FILE"
#define EEPE_EEPROM_FILE_HEADER        "EEPE EEPROM FILE"
#define EEPE_MODEL_FILE_HEADER         "EEPE MODEL FILE"
#define SIMU_FILES_FILTER              ETX_FILES_FILTER

#define CPN_STR_APP_SETTINGS           QCoreApplication::translate("Companion", "Application Settings")
#define CPN_STR_APP_SETTINGS_FILES     CPN_STR_APP_SETTINGS % " " % CPN_STR_FILES
#define CPN_STR_APP_SETTINGS_FILTER    CPN_STR_APP_SETTINGS_FILES % " (*.ini)"

#define CPN_STR_UNKNOWN_ITEM           "???"
#define CPN_STR_NONE_ITEM              "----"

#define CPN_STR_SW_INDICATOR_UP        QCoreApplication::translate("RawSwitch", "\xE2\x86\x91")  // Switch up position indicator: Up arrow, or similar.
#define CPN_STR_SW_INDICATOR_DN        QCoreApplication::translate("RawSwitch", "\xE2\x86\x93")  // Switch down position indicator: Down arrow, or similar.
#define CPN_STR_SW_INDICATOR_NEUT      QCoreApplication::translate("RawSwitch", "-")             // Switch neutral (middle) position indicator.
#define CPN_STR_SW_INDICATOR_REV       QCoreApplication::translate("RawSwitch", "!")             // Switch reversed logic (NOT) indicator.

#define CPN_STR_SRC_INDICATOR_NEG      QCoreApplication::translate("RawSource", "-")             // Source negative value indicator.

#define EDGETX_HOME_PAGE_URL           "https://edgetx.org"

#define CPN_ADC_REFACTOR_VERSION       "2.10.0"
