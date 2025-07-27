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

#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "hal/module_port.h"
#include "hal/rgbleds.h"

#include "analogs.h"
#include "switches.h"

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
#endif

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

#if LCD_W >= 212
  #define HW_SETTINGS_COLUMN1            12*FW
  #define HW_SETTINGS_COLUMN2            (20*FW - 3)
  #define HW_SETTINGS_COLUMN3            HW_SETTINGS_COLUMN2
#else
  #define HW_SETTINGS_COLUMN1            30
  #define HW_SETTINGS_COLUMN2            (HW_SETTINGS_COLUMN1 + 5*FW)
  #define HW_SETTINGS_COLUMN3            HW_SETTINGS_COLUMN2 + FW
#endif

#if defined(BATTGRAPH)
  #define CASE_BATTGRAPH(x) x,
#else
  #define CASE_BATTGRAPH(x)
#endif

enum {
  ITEM_RADIO_HARDWARE_LABEL_STICKS = 0,
  ITEM_RADIO_HARDWARE_STICK,
  ITEM_RADIO_HARDWARE_STICK_END = ITEM_RADIO_HARDWARE_STICK + MAX_STICKS - 1,
  ITEM_RADIO_HARDWARE_LABEL_POTS,
  ITEM_RADIO_HARDWARE_POT,
  ITEM_RADIO_HARDWARE_POT_END = ITEM_RADIO_HARDWARE_POT + MAX_POTS - 1,
  ITEM_RADIO_HARDWARE_LABEL_SWITCHES,
  ITEM_RADIO_HARDWARE_SWITCH,
  ITEM_RADIO_HARDWARE_SWITCH_END = ITEM_RADIO_HARDWARE_SWITCH + MAX_SWITCHES - 1,
#if defined(FUNCTION_SWITCHES)
  ITEM_RADIO_HARDWARE_LABEL_CFS,
  ITEM_RADIO_HARDWARE_CFS,
  ITEM_RADIO_HARDWARE_CFS_END = ITEM_RADIO_HARDWARE_CFS + MAX_SWITCHES - 1,
#endif
  CASE_BATTGRAPH(ITEM_RADIO_HARDWARE_BATT_RANGE)
  ITEM_RADIO_HARDWARE_BATTERY_CALIB,
  ITEM_RADIO_HARDWARE_RTC_BATTERY,
  ITEM_RADIO_HARDWARE_RTC_CHECK,
#if defined(AUDIO_MUTE_GPIO)
  ITEM_RADIO_HARDWARE_AUDIO_MUTE,
#endif
  ITEM_RADIO_HARDWARE_LABEL_INTERNAL_MODULE,
  ITEM_RADIO_HARDWARE_INTERNAL_MODULE_TYPE,
  ITEM_RADIO_HARDWARE_INTERNAL_MODULE_BAUDRATE,
  ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE,
#if defined(BLUETOOTH)
  ITEM_RADIO_HARDWARE_BLUETOOTH_MODE,
  ITEM_RADIO_HARDWARE_BLUETOOTH_PAIRING_CODE,
  ITEM_RADIO_HARDWARE_BLUETOOTH_LOCAL_ADDR,
  ITEM_RADIO_HARDWARE_BLUETOOTH_DISTANT_ADDR,
  ITEM_RADIO_HARDWARE_BLUETOOTH_NAME,
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  ITEM_RADIO_HARDWARE_EXTERNAL_ANTENNA,
#endif
  ITEM_RADIO_HARDWARE_SERIAL_PORT_LABEL,
  ITEM_RADIO_HARDWARE_SERIAL_PORT,
  ITEM_RADIO_HARDWARE_SERIAL_PORT_END = ITEM_RADIO_HARDWARE_SERIAL_PORT + MAX_SERIAL_PORTS - 1,
  ITEM_RADIO_HARDWARE_JITTER_FILTER,
  ITEM_RADIO_HARDWARE_RAS,
  ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER,
#if (defined(BACKLIGHT_GPIO) || defined(OLED_SCREEN)) && (LCD_W == 128)
  ITEM_RADIO_HARDWARE_SCREEN_LABEL,
  ITEM_RADIO_HARDWARE_SCREEN_INVERT,
#endif
  ITEM_RADIO_HARDWARE_DEBUG,
#if defined(FUNCTION_SWITCHES)
  ITEM_RADIO_HARDWARE_DEBUG_FS,
#endif
  ITEM_RADIO_HARDWARE_MAX
};

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
static void onHardwareAntennaSwitchConfirm(const char * result)
{
  if (result == STR_OK) {
    // Switch to external antenna confirmation
    g_eeGeneral.antennaMode = reusableBuffer.radioHardware.antennaMode;
    storageDirty(EE_GENERAL);
  }
  else {
    reusableBuffer.radioHardware.antennaMode = g_eeGeneral.antennaMode;
  }
}
#endif

#if defined(FUNCTION_SWITCHES)
#define RADIO_SETUP_2ND_COLUMN           (LCD_W-11*FW)
extern char* _fct_sw_start[];
static int swIndex;

extern bool checkCFSTypeAvailable(int val);

enum CFSFields {
  CFS_FIELD_TYPE,
  CFS_FIELD_NAME,
  CFS_FIELD_START,
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  CFS_FIELD_COLOR_LABEL,
  CFS_FIELD_ON_COLOR,
  CFS_FIELD_ON_LUA_OVERRIDE,
  CFS_FIELD_OFF_COLOR,
  CFS_FIELD_OFF_LUA_OVERRIDE,
#endif
  CFS_FIELD_COUNT
};

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
extern void menuCFSColor(coord_t y, RGBLedColor& color, const char* title, LcdFlags attr, event_t event);
#endif

static void menuRadioCFSOne(event_t event)
{
  std::string s(STR_CHAR_SWITCH);
  s += switchGetDefaultName(swIndex);

  int config = g_eeGeneral.switchType(swIndex);
  uint8_t group = g_eeGeneral.switchGroup(swIndex);
  int startPos = g_eeGeneral.switchStart(swIndex);

  SUBMENU(s.c_str(), CFS_FIELD_COUNT,
    {
      0,
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? 0 : HIDDEN_ROW),
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_TOGGLE && config != SWITCH_GLOBAL && group == 0) ? 0 : HIDDEN_ROW),
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? LABEL() : HIDDEN_ROW),
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? 3 : HIDDEN_ROW),
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? 0 : HIDDEN_ROW),
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? 3 : HIDDEN_ROW),
      (uint8_t)((config != SWITCH_NONE && config != SWITCH_GLOBAL) ? 0 : HIDDEN_ROW),
#endif
    });
  
  int8_t sub = menuVerticalPosition;
  int8_t editMode = s_editMode;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (int k = 0; k < NUM_BODY_LINES; k += 1) {
    int i = k + menuVerticalOffset;
    for (int j = 0; j <= i; j += 1) {
      if (j < (int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        i += 1;
      }
    }
    LcdFlags attr = (sub == i ? (editMode > 0 ? BLINK | INVERS : INVERS) : 0);

    switch(i) {
      case CFS_FIELD_TYPE:
        config = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_SWITCH_TYPE, STR_SWTYPES, config, SWITCH_NONE, SWITCH_3POS, attr, event, 0, checkCFSTypeAvailable);
        if (attr && checkIncDec_Ret) {
          g_eeGeneral.switchSetType(swIndex, (SwitchConfig)config);
          if (config == SWITCH_NONE) {
#if defined(FUNCTION_SWITCHES_RGB_LEDS)
            if (g_model.getSwitchType(swIndex) == SWITCH_NONE)
              fsLedRGB(switchGetCustomSwitchIdx(swIndex), 0);
#endif
          } else if (config == SWITCH_TOGGLE) {
            g_eeGeneral.switchSetStart(swIndex, FS_START_PREVIOUS);  // Toggle switches do not have startup position
          }
        }
        break;

      case CFS_FIELD_NAME:
        editSingleName(RADIO_SETUP_2ND_COLUMN, y, STR_NAME, g_eeGeneral.switchName(swIndex),
                       LEN_SWITCH_NAME, event, (attr != 0),
                       editMode);
        break;

      case CFS_FIELD_START:
        lcdDrawText(0, y, STR_SWITCH_STARTUP);
        lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, _fct_sw_start[startPos], attr ? (s_editMode ? INVERS + BLINK : INVERS) : 0);
        if (attr) {
          startPos = checkIncDec(event, startPos, FS_START_OFF, FS_START_PREVIOUS, EE_MODEL);
          g_eeGeneral.switchSetStart(swIndex, (fsStartPositionType)startPos);
        }
        break;

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
      case CFS_FIELD_COLOR_LABEL:
        lcdDrawText(0, y, STR_BLCOLOR);
        lcdDrawText(LCD_W - 6 * FW, y, "R", RIGHT | SMLSIZE);
        lcdDrawText(LCD_W - 3 * FW, y, "G", RIGHT | SMLSIZE);
        lcdDrawText(LCD_W, y, "B", RIGHT | SMLSIZE);
        break;

      case CFS_FIELD_ON_COLOR:
        menuCFSColor(y, g_eeGeneral.switchOnColor(swIndex), STR_OFFON[1], attr, event);
        break;

      case CFS_FIELD_ON_LUA_OVERRIDE:
        g_eeGeneral.cfsSetOnColorLuaOverride(swIndex, editCheckBox(g_eeGeneral.cfsOnColorLuaOverride(swIndex), LCD_W - 2 * FW, y, STR_LUA_OVERRIDE, attr, event, INDENT_WIDTH));
        break;

      case CFS_FIELD_OFF_COLOR:
        menuCFSColor(y, g_eeGeneral.switchOffColor(swIndex), STR_OFFON[0], attr, event);
        break;

      case CFS_FIELD_OFF_LUA_OVERRIDE:
        g_eeGeneral.cfsSetOffColorLuaOverride(swIndex, editCheckBox(g_eeGeneral.cfsOffColorLuaOverride(swIndex), LCD_W - 2 * FW, y, STR_LUA_OVERRIDE, attr, event, INDENT_WIDTH));
        break;
#endif
    }

    y += FH;
  }
}
#endif

static uint8_t _dispSerialPort(uint8_t port_nr)
{
  auto port = serialGetPort(port_nr);
  if (!port || !port->name) return HIDDEN_ROW;
  return 0;
}

static bool _isAux1ModeAvailable(int m) { return isSerialModeAvailable(SP_AUX1, m); }
static bool _isAux2ModeAvailable(int m) { return isSerialModeAvailable(SP_AUX2, m); }
static bool _isVCPModeAvailable(int m) { return isSerialModeAvailable(SP_VCP, m); }

static const IsValueAvailable _isSerialModeAvailable[MAX_SERIAL_PORTS] = {
  _isAux1ModeAvailable, _isAux2ModeAvailable, _isVCPModeAvailable
};

static void _init_menu_tab_array(uint8_t* tab, size_t len)
{
  memset((void*)tab, READONLY_ROW, sizeof(MENU_TAB_ARRAY_TYPE) * len);
  tab += HEADER_LINE; // skip header line

  tab[ITEM_RADIO_HARDWARE_LABEL_STICKS] = 0; // calib button
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (int i = ITEM_RADIO_HARDWARE_STICK; i <= ITEM_RADIO_HARDWARE_STICK_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_STICK;
    tab[i] = idx < max_sticks ? 1 : HIDDEN_ROW;
  }

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (int i = ITEM_RADIO_HARDWARE_POT; i <= ITEM_RADIO_HARDWARE_POT_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_POT;
    tab[i] = idx < max_pots ? (IS_POT_MULTIPOS(idx) ? 1 : 2) : HIDDEN_ROW;
  }

  int max_switches = switchGetMaxAllSwitches();
  for (int i = ITEM_RADIO_HARDWARE_SWITCH; i <= ITEM_RADIO_HARDWARE_SWITCH_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_SWITCH;
    if (idx < max_switches && !switchIsCustomSwitch(idx)) {
      tab[i] = switchIsFlex(idx) ? 2 : 1;
    } else {
      tab[i] = HIDDEN_ROW;
    }
  }
#if defined (FUNCTION_SWITCHES)
  for (int i = ITEM_RADIO_HARDWARE_CFS; i <= ITEM_RADIO_HARDWARE_CFS_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_CFS;
    if (idx < max_switches && switchIsCustomSwitch(idx)) {
      tab[i] = 0;
    } else {
      tab[i] = HIDDEN_ROW;
    }
  }
#endif

#if defined(BATTGRAPH)
  tab[ITEM_RADIO_HARDWARE_BATT_RANGE] = 1;
#endif
  tab[ITEM_RADIO_HARDWARE_BATTERY_CALIB] = 0;
  tab[ITEM_RADIO_HARDWARE_RTC_BATTERY] = READONLY_ROW;
  tab[ITEM_RADIO_HARDWARE_RTC_CHECK] = 0;
#if defined(AUDIO_MUTE_GPIO)
  tab[ITEM_RADIO_HARDWARE_AUDIO_MUTE] = 0;
#endif
#if defined(HARDWARE_INTERNAL_MODULE)
  tab[ITEM_RADIO_HARDWARE_INTERNAL_MODULE_TYPE] = 0;
  tab[ITEM_RADIO_HARDWARE_INTERNAL_MODULE_BAUDRATE] =
    isInternalModuleCrossfire() ? (uint8_t)0 : HIDDEN_ROW;
#else
  memset((void*)&tab[ITEM_RADIO_HARDWARE_LABEL_INTERNAL_MODULE], HIDDEN_ROW, 3);
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  tab[ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE] = 0;
#else
  tab[ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE] = HIDDEN_ROW;
#endif

#if defined(BLUETOOTH)
  bool has_bt = IS_BLUETOOTH_CHIP_PRESENT();
  tab[ITEM_RADIO_HARDWARE_BLUETOOTH_MODE] = has_bt ? 0 : HIDDEN_ROW;

  bool bt_telem = g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY;
  tab[ITEM_RADIO_HARDWARE_BLUETOOTH_PAIRING_CODE] = bt_telem ? READONLY_ROW : HIDDEN_ROW;

  bool bt_off = g_eeGeneral.bluetoothMode == BLUETOOTH_OFF;
  tab[ITEM_RADIO_HARDWARE_BLUETOOTH_LOCAL_ADDR] = bt_off ? HIDDEN_ROW : READONLY_ROW;
  tab[ITEM_RADIO_HARDWARE_BLUETOOTH_DISTANT_ADDR] = bt_off ? HIDDEN_ROW : READONLY_ROW;
  tab[ITEM_RADIO_HARDWARE_BLUETOOTH_NAME] = bt_off ? HIDDEN_ROW : 0;
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  tab[ITEM_RADIO_HARDWARE_EXTERNAL_ANTENNA] =
    g_eeGeneral.internalModule == MODULE_TYPE_XJT_PXX1 ? 0 : HIDDEN_ROW;
#endif

  bool has_serial = false;
  for (int i = 0; i < MAX_SERIAL_PORTS; i++) {
    uint8_t idx = ITEM_RADIO_HARDWARE_SERIAL_PORT + i;
    auto r = _dispSerialPort(i);
    tab[idx] = r;
    has_serial = has_serial || (!r);
  }
  tab[ITEM_RADIO_HARDWARE_SERIAL_PORT_LABEL] = has_serial ? READONLY_ROW : HIDDEN_ROW;
  tab[ITEM_RADIO_HARDWARE_JITTER_FILTER] = 0;
  tab[ITEM_RADIO_HARDWARE_RAS] = READONLY_ROW;

  auto mod_desc = modulePortGetModuleDescription(SPORT_MODULE);
  if (mod_desc && mod_desc->set_pwr) {
    tab[ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER] = 0;
  } else {
    tab[ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER] = HIDDEN_ROW;
  }

#if (defined(BACKLIGHT_GPIO) || defined(OLED_SCREEN)) && (LCD_W == 128)
  tab[ITEM_RADIO_HARDWARE_SCREEN_LABEL] = READONLY_ROW;
  tab[ITEM_RADIO_HARDWARE_SCREEN_INVERT] = 0;
#endif

  tab[ITEM_RADIO_HARDWARE_DEBUG] = 1;
#if defined(FUNCTION_SWITCHES)
  tab[ITEM_RADIO_HARDWARE_DEBUG_FS] = 0;
#endif
}

void menuRadioHardware(event_t event)
{
  uint8_t old_editMode = s_editMode;

  uint8_t MENU_TAB_ARRAY_NAME[HEADER_LINE + ITEM_RADIO_HARDWARE_MAX];
  _init_menu_tab_array(MENU_TAB_ARRAY_NAME, HEADER_LINE + ITEM_RADIO_HARDWARE_MAX);

  MENU_CHECK(menuTabGeneral, MENU_RADIO_HARDWARE, HEADER_LINE + ITEM_RADIO_HARDWARE_MAX);
  title(STR_HARDWARE);

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

#if defined(BLUETOOTH)
  if (g_eeGeneral.bluetoothMode != BLUETOOTH_OFF && !IS_BLUETOOTH_CHIP_PRESENT()) {
    g_eeGeneral.bluetoothMode = BLUETOOTH_OFF;
    storageDirty(EE_GENERAL);
  }
#endif

  if (menuEvent) {
    disableVBatBridge();
  }
  else if (event == EVT_ENTRY) {
    enableVBatBridge();
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    reusableBuffer.radioHardware.antennaMode = g_eeGeneral.antennaMode;
#endif
  }

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j = 0; j <= k; j++) {
      if (mstate_tab[j + HEADER_LINE] == HIDDEN_ROW) {
        k++;
      }
    }
    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch(k) {
      case ITEM_RADIO_HARDWARE_LABEL_STICKS:
        lcdDrawTextAlignedLeft(y, STR_STICKS);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, BUTTON(TR_CALIBRATION), attr);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          pushMenu(menuRadioCalibration);
        }
        break;

      case ITEM_RADIO_HARDWARE_LABEL_POTS:
        lcdDrawTextAlignedLeft(y, STR_POTS);
        break;

      case ITEM_RADIO_HARDWARE_LABEL_SWITCHES:
        lcdDrawTextAlignedLeft(y, STR_SWITCHES);
        break;

#if defined(FUNCTION_SWITCHES)
      case ITEM_RADIO_HARDWARE_LABEL_CFS:
        lcdDrawTextAlignedLeft(y, STR_FUNCTION_SWITCHES);
        break;
#endif

#if defined(BATTGRAPH)
      case ITEM_RADIO_HARDWARE_BATT_RANGE:
        lcdDrawTextAlignedLeft(y, STR_BATTERY_RANGE);
        putsVolts(HW_SETTINGS_COLUMN3, y, 90+g_eeGeneral.vBatMin, (menuHorizontalPosition==0 ? attr : 0)|NO_UNIT);
        lcdDrawChar(lcdNextPos, y, '-');
        putsVolts(lcdNextPos, y, 120+g_eeGeneral.vBatMax, (menuHorizontalPosition>0 ? attr : 0)|NO_UNIT);
        if (attr && s_editMode>0) {
          if (menuHorizontalPosition==0)
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMin, -60, g_eeGeneral.vBatMax+29); // min=3.0V
          else
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMax, g_eeGeneral.vBatMin-29, +40); // max=16.0V
        }
        break;
#endif

      case ITEM_RADIO_HARDWARE_BATTERY_CALIB:
        lcdDrawTextAlignedLeft(y, STR_BATT_CALIB);
        putsVolts(HW_SETTINGS_COLUMN3, y, getBatteryVoltage(), attr|PREC2|LEFT);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.txVoltageCalibration, -127, 127);
        }
        break;

      case ITEM_RADIO_HARDWARE_RTC_BATTERY:
        lcdDrawTextAlignedLeft(y, STR_RTC_BATT);
        putsVolts(HW_SETTINGS_COLUMN3, y, getRTCBatteryVoltage(), PREC2|LEFT);
        break;

      case ITEM_RADIO_HARDWARE_RTC_CHECK:
        g_eeGeneral.disableRtcWarning = 1 - editCheckBox(1 - g_eeGeneral.disableRtcWarning, HW_SETTINGS_COLUMN2, y, STR_RTC_CHECK, attr, event);
        break;

      case ITEM_RADIO_HARDWARE_LABEL_INTERNAL_MODULE:
        lcdDrawTextAlignedLeft(y, STR_INTERNALRF);
        break;

      case ITEM_RADIO_HARDWARE_INTERNAL_MODULE_TYPE: {
        g_eeGeneral.internalModule =
            editChoice(HW_SETTINGS_COLUMN2, y, STR_TYPE,
                       STR_MODULE_PROTOCOLS,
                       g_eeGeneral.internalModule, MODULE_TYPE_NONE,
                       MODULE_TYPE_MAX, attr, event, INDENT_WIDTH, isInternalModuleSupported);
        if (g_model.moduleData[INTERNAL_MODULE].type !=
            g_eeGeneral.internalModule) {
          memclear(&g_model.moduleData[INTERNAL_MODULE], sizeof(ModuleData));
          storageDirty(EE_MODEL);
          storageDirty(EE_GENERAL);
        #if defined(CROSSFIRE) && defined(USB_SERIAL)
          // If USB-VCP was off, set it to CLI to enable passthrough flashing
          if (isInternalModuleCrossfire() && serialGetMode(SP_VCP) ==  UART_MODE_NONE)
            serialSetMode(SP_VCP, UART_MODE_CLI);
        #endif
        }
      } break;

#if defined(AUDIO_MUTE_GPIO)
      case ITEM_RADIO_HARDWARE_AUDIO_MUTE:
        g_eeGeneral.audioMuteEnable = editCheckBox(g_eeGeneral.audioMuteEnable, HW_SETTINGS_COLUMN2, y, STR_AUDIO_MUTE, attr, event);
        break;
#endif

      case ITEM_RADIO_HARDWARE_INTERNAL_MODULE_BAUDRATE:
        lcdDrawTextIndented(y, STR_BAUDRATE);
        lcdDrawTextAtIndex(HW_SETTINGS_COLUMN2, y, STR_CRSF_BAUDRATE, CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate),attr | LEFT);
        if (attr) {
          g_eeGeneral.internalModuleBaudrate = CROSSFIRE_INDEX_TO_STORE(checkIncDecModel(event, CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate), 0, CROSSFIRE_MAX_INTERNAL_BAUDRATE));
          if (checkIncDec_Ret) {
              restartModule(INTERNAL_MODULE);
          }
        }
        break;

      case ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE:
        g_eeGeneral.uartSampleMode = editChoice(HW_SETTINGS_COLUMN2, y, STR_SAMPLE_MODE, STR_SAMPLE_MODES, g_eeGeneral.uartSampleMode, 0, UART_SAMPLE_MODE_MAX, attr, event);
        if (attr && checkIncDec_Ret) {
          restartModule(EXTERNAL_MODULE);
        }
        break;

#if defined(BLUETOOTH)
      case ITEM_RADIO_HARDWARE_BLUETOOTH_MODE:
        lcdDrawTextAlignedLeft(y, STR_BLUETOOTH);
        lcdDrawTextAtIndex(HW_SETTINGS_COLUMN2, y, STR_BLUETOOTH_MODES, g_eeGeneral.bluetoothMode, attr);
        if (attr) {
          g_eeGeneral.bluetoothMode = checkIncDecGen(event, g_eeGeneral.bluetoothMode, BLUETOOTH_OFF, BLUETOOTH_MAX);
        }
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_PAIRING_CODE:
        lcdDrawTextIndented(y, STR_BLUETOOTH_PIN_CODE);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, "000000");
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_LOCAL_ADDR:
        lcdDrawTextIndented(y, STR_BLUETOOTH_LOCAL_ADDR);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, bluetooth.localAddr[0] == '\0' ? "---" : bluetooth.localAddr);
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_DISTANT_ADDR:
        lcdDrawTextIndented(y, STR_BLUETOOTH_DIST_ADDR);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, bluetooth.distantAddr[0] == '\0' ? "---" : bluetooth.distantAddr);
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_NAME:
        lcdDrawTextIndented(y, STR_NAME);
        editName(HW_SETTINGS_COLUMN2, y, g_eeGeneral.bluetoothName,
                 LEN_BLUETOOTH_NAME, event, (attr != 0), attr, old_editMode);
        break;
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
      case ITEM_RADIO_HARDWARE_EXTERNAL_ANTENNA:
        reusableBuffer.radioHardware.antennaMode = editChoice(HW_SETTINGS_COLUMN2, y, STR_ANTENNA, STR_ANTENNA_MODES, reusableBuffer.radioHardware.antennaMode, ANTENNA_MODE_INTERNAL, ANTENNA_MODE_EXTERNAL, attr, event);
        if (!s_editMode && reusableBuffer.radioHardware.antennaMode != g_eeGeneral.antennaMode) {
          if (!isExternalAntennaEnabled() && (reusableBuffer.radioHardware.antennaMode == ANTENNA_MODE_EXTERNAL || (reusableBuffer.radioHardware.antennaMode == ANTENNA_MODE_PER_MODEL && g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode == ANTENNA_MODE_EXTERNAL))) {
            POPUP_CONFIRMATION(STR_ANTENNACONFIRM1, onHardwareAntennaSwitchConfirm);
            SET_WARNING_INFO(STR_ANTENNACONFIRM2, sizeof(TR_ANTENNACONFIRM2), 0);
          }
          else {
            g_eeGeneral.antennaMode = reusableBuffer.radioHardware.antennaMode;
            checkExternalAntenna();
          }
        }
        break;
#endif

      case ITEM_RADIO_HARDWARE_SERIAL_PORT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_AUX_SERIAL_MODE);
        break;

      case ITEM_RADIO_HARDWARE_JITTER_FILTER:
        g_eeGeneral.noJitterFilter =
            1 - editCheckBox(1 - g_eeGeneral.noJitterFilter,
                             HW_SETTINGS_COLUMN2, y, STR_JITTER_FILTER, attr,
                             event);
        break;

      case ITEM_RADIO_HARDWARE_RAS:
#if defined(HARDWARE_INTERNAL_RAS)
        lcdDrawTextAlignedLeft(y, "RAS");
        if (telemetryData.swrInternal.isFresh())
          lcdDrawNumber(HW_SETTINGS_COLUMN2, y, telemetryData.swrInternal.value());
        else
          lcdDrawText(HW_SETTINGS_COLUMN2, y, "---");
        lcdDrawText(lcdNextPos, y, "/");
#else
        lcdDrawTextAlignedLeft(y, "Ext. RAS");
        lcdNextPos = HW_SETTINGS_COLUMN2;
#endif
        if (telemetryData.swrExternal.isFresh())
          lcdDrawNumber(lcdNextPos, y, telemetryData.swrExternal.value());
        else
          lcdDrawText(lcdNextPos, y, "---");
        break;

      case ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER:
        g_eeGeneral.sportUpdatePower = editChoice(HW_SETTINGS_COLUMN2, y, STR_SPORT_UPDATE_POWER_MODE, STR_SPORT_UPDATE_POWER_MODES, g_eeGeneral.sportUpdatePower, 0, 1, attr, event);
        if (attr && checkIncDec_Ret) {
          modulePortSetPower(SPORT_MODULE, g_eeGeneral.sportUpdatePower);
        }
        break;

#if (defined(BACKLIGHT_GPIO) || defined(OLED_SCREEN)) && (LCD_W == 128)
      case ITEM_RADIO_HARDWARE_SCREEN_LABEL:
        lcdDrawTextAlignedLeft(y, STR_SCREEN);
        break;
      case ITEM_RADIO_HARDWARE_SCREEN_INVERT:
        {
          lcdDrawText(INDENT_WIDTH, y, STR_MENU_INVERT);
          bool inv = g_eeGeneral.invertLCD;
          g_eeGeneral.invertLCD = editCheckBox(g_eeGeneral.invertLCD, HW_SETTINGS_COLUMN2, y, nullptr, attr, event);
          if (inv != g_eeGeneral.invertLCD)
            lcdSetInvert(g_eeGeneral.invertLCD);
        }
        break;
#endif

      case ITEM_RADIO_HARDWARE_DEBUG:
        lcdDrawTextAlignedLeft(y, STR_DEBUG);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, STR_ANALOGS_BTN, menuHorizontalPosition == 0 ? attr : 0);
        lcdDrawText(lcdLastRightPos + 2, y, STR_KEYS_BTN, menuHorizontalPosition == 1 ? attr : 0);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          if (menuHorizontalPosition == 0)
            pushMenu(menuRadioDiagAnalogs);
          else
            pushMenu(menuRadioDiagKeys);
        }
        break;
#if defined(FUNCTION_SWITCHES)
      case ITEM_RADIO_HARDWARE_DEBUG_FS:
        lcdDrawText(HW_SETTINGS_COLUMN2, y, STR_FS_BTN, attr);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
            pushMenu(menuRadioDiagFS);
        }
        break;
#endif

      default:
        if (k <= ITEM_RADIO_HARDWARE_STICK_END) {
          // Sticks
          int idx = k - ITEM_RADIO_HARDWARE_STICK;

          LcdFlags flags = menuHorizontalPosition == 0 ? attr : 0;
          editStickHardwareSettings(HW_SETTINGS_COLUMN1, y,
                                    idx, event, flags, old_editMode);
          // ADC inversion
          flags = menuHorizontalPosition == 1 ? attr : 0;
          bool stickinversion = getStickInversion(idx);
          lcdDrawChar(LCD_W - 8, y, stickinversion ? 127 : 126, flags);
          if (flags & (~RIGHT)) stickinversion = checkIncDec(event, stickinversion, 0, 1, EE_GENERAL);
          setStickInversion(idx, stickinversion);

        } else if (k <= ITEM_RADIO_HARDWARE_POT_END) {
          // Pots & sliders
          int idx = k - ITEM_RADIO_HARDWARE_POT;

          // draw hw name
          LcdFlags flags = menuHorizontalPosition < 0 ? attr : 0;
          lcdDrawText(INDENT_WIDTH, y, STR_CHAR_POT, flags);
          lcdDrawText(lcdNextPos, y, adcGetInputLabel(ADC_INPUT_FLEX, idx), flags);

          // draw custom name
          if (analogHasCustomLabel(ADC_INPUT_FLEX, idx) ||
              (attr && s_editMode > 0 && menuHorizontalPosition == 0)) {
            editName(HW_SETTINGS_COLUMN1, y,
                     (char*)analogGetCustomLabel(ADC_INPUT_FLEX, idx), LEN_ANA_NAME, event,
                     attr && menuHorizontalPosition == 0, 0, old_editMode);
          } else {
            lcdDrawMMM(HW_SETTINGS_COLUMN1, y, menuHorizontalPosition==0 ? attr : 0);
          }

          // pot config
          uint8_t potType = getPotType(idx);
          potType = editChoice(HW_SETTINGS_COLUMN2, y, "", STR_POTTYPES, potType,
                               FLEX_NONE, FLEX_SWITCH,
                               menuHorizontalPosition == 1 ? attr : 0, event);
          if (checkIncDec_Ret) switchFixFlexConfig();
          setPotType(idx, potType);

          if (!IS_POT_MULTIPOS(idx)) {
            // ADC inversion
            flags = menuHorizontalPosition == 2 ? attr : 0;
            bool potinversion = getPotInversion(idx);
            lcdDrawChar(LCD_W - 8, y, potinversion ? 127 : 126, flags);
            if (flags & (~RIGHT)) potinversion = checkIncDec(event, potinversion, 0, 1, EE_GENERAL);
            setPotInversion(idx, potinversion);
          } else if (getPotInversion(idx)) {
            setPotInversion(idx, 0);
            storageDirty(EE_GENERAL);
          }
        } else if (k <= ITEM_RADIO_HARDWARE_SWITCH_END) {
          // Switches
          int index = k - ITEM_RADIO_HARDWARE_SWITCH;
          int config = g_eeGeneral.switchType(index);

          LcdFlags flags = menuHorizontalPosition < 0 ? attr : 0;
          lcdDrawText(INDENT_WIDTH, y, STR_CHAR_SWITCH, flags);
          lcdDrawText(lcdNextPos, y, switchGetDefaultName(index), flags);

          if (switchIsFlex(index)) {
            // flexSwitch source
            flags = menuHorizontalPosition == 0 ? attr : 0;
            auto source = switchGetFlexConfig(index);
            lcdDrawText(HW_SETTINGS_COLUMN1, y, (source < 0) ? STR_NONE : adcGetInputLabel(ADC_INPUT_FLEX, source), flags);
            if (flags & (~RIGHT)) source = checkIncDec(event, source, -1, adcGetMaxInputs(ADC_INPUT_FLEX) - 1, EE_GENERAL, isFlexSwitchSourceValid);
            switchConfigFlex(index, source);

            //Name
            flags = menuHorizontalPosition == 1 ? attr : 0;
            if (g_eeGeneral.switchHasCustomName(index) ||
                (attr && s_editMode > 0 && menuHorizontalPosition == 1)) {
              editName(HW_SETTINGS_COLUMN2, y,
                       g_eeGeneral.getSwitchCustomName(index), LEN_SWITCH_NAME,
                       event, flags, 0, old_editMode);
            } else {
              lcdDrawMMM(HW_SETTINGS_COLUMN2, y, flags);
            }

            // Switch type
            flags = menuHorizontalPosition == 2 ? attr : 0;
            config = editChoice(HW_SETTINGS_COLUMN2 + 25, y, "", STR_SWTYPES, config,
                           SWITCH_NONE, switchGetMaxType(index), flags, event);

            if (attr && checkIncDec_Ret) {
              g_eeGeneral.switchSetType(index, (SwitchConfig)config);
            }
          } else {
            flags = menuHorizontalPosition == 0 ? attr : 0;
            if (g_eeGeneral.switchHasCustomName(index) ||
                (attr && s_editMode > 0 && menuHorizontalPosition == 0)) {
              editName(HW_SETTINGS_COLUMN1, y,
                       g_eeGeneral.getSwitchCustomName(index), LEN_SWITCH_NAME,
                       event, flags, 0, old_editMode);
            } else {
              lcdDrawMMM(HW_SETTINGS_COLUMN1, y, flags);
            }

            flags = menuHorizontalPosition == 1 ? attr : 0;
            config =
                editChoice(HW_SETTINGS_COLUMN2, y, "", STR_SWTYPES, config,
                           SWITCH_NONE, switchGetMaxType(index), flags, event);

            if (attr && checkIncDec_Ret) {
              g_eeGeneral.switchSetType(index, (SwitchConfig)config);
            }
          }
#if defined(FUNCTION_SWITCHES)
        } else if (k <= ITEM_RADIO_HARDWARE_CFS_END) {
          // Customisable Switches
          int index = k - ITEM_RADIO_HARDWARE_CFS;
          int config = g_eeGeneral.switchType(index);

          lcdDrawText(INDENT_WIDTH, y, STR_CHAR_SWITCH, attr);
          lcdDrawText(lcdNextPos, y, switchGetDefaultName(index), attr);

          lcdDrawText(HW_SETTINGS_COLUMN2, y, STR_SWTYPES[config]);

          if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
            swIndex = index;
            pushMenu(menuRadioCFSOne);
          }
  
          if (config != SWITCH_NONE) {
            if (g_eeGeneral.switchName(index)[0]) {
              char s[LEN_SWITCH_NAME + 1];
              strAppend(s, g_eeGeneral.switchName(index), LEN_SWITCH_NAME);
              lcdDrawText(HW_SETTINGS_COLUMN1, y, s);
            } else {
              lcdDrawMMM(HW_SETTINGS_COLUMN1, y, 0);
            }

            uint8_t group = g_eeGeneral.switchGroup(index);
            // lcdDrawText(30 + 13 * FW, y, STR_FSGROUPS[group]);

            if (config != SWITCH_TOGGLE && group == 0) {
              int startPos = g_eeGeneral.switchStart(index);
              lcdDrawText(30 + 15 * FW, y, _fct_sw_start[startPos]);
            }
          }
#endif
        } else if (k <= ITEM_RADIO_HARDWARE_SERIAL_PORT_END) {
          auto port_nr = k - ITEM_RADIO_HARDWARE_SERIAL_PORT;
          auto port = serialGetPort(port_nr);
          if (port && port->name) {
            lcdDrawTextIndented(y, port->name);

            auto mode = serialGetMode(port_nr);
            mode = editChoice(HW_SETTINGS_COLUMN2, y, nullptr,
                              STR_AUX_SERIAL_MODES, mode, 0, UART_MODE_MAX, attr,
                              event, 0, _isSerialModeAvailable[port_nr]);

            if (attr && checkIncDec_Ret) {
              serialSetMode(port_nr, mode);
              serialInit(port_nr, mode);
            }
          }
        }
        break;
    }
  }
}
