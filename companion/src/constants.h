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
#define CPN_MAX_GVARS                  15 // 0, 9, 15
#define CPN_MAX_CHNOUT                 32 // number of real output channels
#define CPN_MAX_LOGICAL_SWITCHES       64 // number of custom switches
#define CPN_MAX_GLOBAL_FUNCTIONS       64 // number of functions assigned to switches
#define CPN_MAX_SPECIAL_FUNCTIONS      64 // number of functions assigned to switches
#define CPN_MAX_MODELS                 60 // B&W
#define CPN_MAX_MODULES                2
#define CPN_MAX_STICKS                 Board::STICK_AXIS_COUNT
#define CPN_MAX_TRIMS                  Board::TRIM_AXIS_COUNT
#define CPN_MAX_CYC                    3
#define CPN_MAX_SWITCHES_FLEX          4 // v2.10 cmake parameter FLEXSW
#define CPN_MAX_SWITCHES_FUNCTION      8
#define CPN_MAX_SWITCHES_STD           20
#define CPN_MAX_SWITCHES               (CPN_MAX_SWITCHES_STD + CPN_MAX_SWITCHES_FLEX + CPN_MAX_SWITCHES_FUNCTION)
#define CPN_MAX_SENSORS                99
#define CPN_MAX_SCRIPTS                9  // Max for Color LCD
#define CPN_LEN_SCRIPT_FILENAME        6
#define CPN_LEN_SCRIPT_NAME            6
#define CPN_MAX_SCRIPT_INPUTS          6
#define CPN_MAX_SCRIPT_OUTPUTS         6
#define CPN_MAX_SPACEMOUSE             6
#define CPN_MAX_USER_DATA              50

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
#define BIN_FILES_FILTER               "BIN " % CPN_STR_FILES % " (*.bin);;"
#define UF2_FILES_FILTER               "UF2 " % CPN_STR_FILES % " (*.uf2);;"
#define ETX_FILES_FILTER               "EdgeTX " % CPN_STR_FILES % " (*.etx);;"
#define YML_FILES_FILTER               "EdgeTX YAML " % CPN_STR_FILES % " (*.yml);;"
#define EEPROM_FILES_FILTER            CPN_STR_RAD_MOD_SETTINGS % " " % CPN_STR_FILES % " (*.etx *.yml);;" % ETX_FILES_FILTER % YML_FILES_FILTER
#define FIRMWARE_FILES_FILTER          "Firmware " % CPN_STR_FILES % " (*.bin *.uf2);;" % BIN_FILES_FILTER % UF2_FILES_FILTER
#define EXTERNAL_EEPROM_FILES_FILTER   "EEPROM " % CPN_STR_FILES % " (*.bin);;" % BIN_FILES_FILTER
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

#define EDGETX_HOME_PAGE_URL           QStringLiteral("https://edgetx.org")
#define EDGETX_DONATE_URL              QStringLiteral("https://opencollective.com/edgetx/donate")
#define EDGETX_COMMIT_URL              QStringLiteral("https://github.com/EdgeTX/edgetx/commit/")
#define EDGETX_ISSUES_URL              QStringLiteral("https://github.com/EdgeTX/edgetx/issues/new/choose")

#define CPN_ADC_REFACTOR_VERSION       "2.10.0"

// moved from various data structs as part of firmware and boards refactor
// to be replaced by firmware definitions when fixed arrays refactored to std::string and std::vector

// curvedata
#define CURVEDATA_NAME_LEN      3

// customisation_data color radios
constexpr int MAX_CUSTOM_SCREENS      {10};
constexpr int MAX_LAYOUT_OPTIONS      {10};
constexpr int MAX_LAYOUT_ZONES        {10};
constexpr int MAX_THEME_OPTIONS       {5};
constexpr int MAX_TOPBAR_OPTIONS      {1};
constexpr int MAX_TOPBAR_ZONES        {7};    // 4 for portrait LCD, 6 for standard LCD, 7 for wide screen LCD
constexpr int MAX_WIDGET_OPTIONS      {50};

// flightmodedata
#define FLIGHTMODE_NAME_LEN     10

// generalsettings
constexpr int BLUETOOTH_NAME_LEN      {10};
constexpr int CURR_MODEL_FILENAME_LEN {16};
constexpr int HARDWARE_NAME_LEN       {3};
constexpr int MAX_KEYSHORTCUTS        {6};
constexpr int MAX_QMFAVOURITES        {12};
constexpr int OWNER_NAME_LEN          {10};
constexpr int REGISTRATION_ID_LEN     {8};
constexpr int SELECTED_THEME_NAME_LEN {26};
constexpr int TTS_LANGUAGE_LEN        {2};

// gvardata
#define GVAR_NAME_LEN           3

// input_data
#define EXPODATA_NAME_LEN       10

// mixdata
#define MIXDATA_NAME_LEN        10

// modeldata
#define MODEL_NAME_LEN 15
#define MODEL_FILENAME_LEN 16  // must match radio LEN_MODEL_FILENAME (dataconstants.h)
#define MODEL_SEMVER_LEN 8
#define MODEL_LABELS_LEN 99    // CSV of labels; was char labels[100]
#define INPUT_NAME_LEN 4
#define CPN_MAX_BITMAP_LEN 14

#define CPN_USBJ_MAX_JOYSTICK_CHANNELS 26

// output_data
#define LIMITDATA_NAME_LEN      6

// sensordata
constexpr int SENSOR_LABEL_LEN { 4 };

// timerdata
constexpr int TIMER_NAME_LEN {8};

// new b&w radios
constexpr int CPN_TELE_CUST_SCRNS { 4 };
constexpr int CPN_TELE_CUST_SCRNS_BARS { 4 };
constexpr int CPN_TELE_CUST_SCRNS_PERLINE { 3 };
constexpr int CPN_TELE_CUST_SCRNS_LINES { 4 };
