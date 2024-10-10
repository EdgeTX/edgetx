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
#else
  #define HW_SETTINGS_COLUMN1            30
  #define HW_SETTINGS_COLUMN2            (HW_SETTINGS_COLUMN1 + 5*FW)
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
    tab[i] = idx < max_sticks ? 0 : HIDDEN_ROW;
  }

  auto max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (int i = ITEM_RADIO_HARDWARE_POT; i <= ITEM_RADIO_HARDWARE_POT_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_POT;
    tab[i] = idx < max_pots ? (IS_POT_MULTIPOS(idx) ? 1 : 2) : HIDDEN_ROW;
  }

  auto max_switches = switchGetMaxSwitches();
  for (int i = ITEM_RADIO_HARDWARE_SWITCH; i <= ITEM_RADIO_HARDWARE_SWITCH_END; i++) {
    uint8_t idx = i - ITEM_RADIO_HARDWARE_SWITCH;
    tab[i] = switchIsFlex(idx) ? 2 : idx < max_switches ? 1 : HIDDEN_ROW;
  }

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

      case ITEM_RADIO_HARDWARE_BATTERY_CALIB:
        lcdDrawTextAlignedLeft(y, STR_BATT_CALIB);
        putsVolts(HW_SETTINGS_COLUMN2, y, getBatteryVoltage(), attr|PREC2|LEFT);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.txVoltageCalibration, -127, 127);
        }
        break;

      case ITEM_RADIO_HARDWARE_RTC_BATTERY:
        lcdDrawTextAlignedLeft(y, STR_RTC_BATT);
        putsVolts(HW_SETTINGS_COLUMN2, y, getRTCBatteryVoltage(), PREC2|LEFT);
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
          editStickHardwareSettings(HW_SETTINGS_COLUMN1, y,
                                    k - ITEM_RADIO_HARDWARE_STICK, event,
                                    attr, old_editMode);

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
            if (flags & (~RIGHT)) potinversion = checkIncDec(event, potinversion, 0, 1, (isModelMenuDisplayed()) ? EE_MODEL : EE_GENERAL);
            setPotInversion(idx, potinversion);
          } else if (getPotInversion(idx)) {
            setPotInversion(idx, 0);
            storageDirty(EE_GENERAL);
          }
        }
        else if (k <= ITEM_RADIO_HARDWARE_SWITCH_END) {
          // Switches
          int index = k - ITEM_RADIO_HARDWARE_SWITCH;
          int config = SWITCH_CONFIG(index);

          LcdFlags flags = menuHorizontalPosition < 0 ? attr : 0;
          lcdDrawText(INDENT_WIDTH, y, STR_CHAR_SWITCH, flags);
          lcdDrawText(lcdNextPos, y, switchGetName(index), flags);

          if (switchIsFlex(index)) {
            // flexSwitch source
            flags = menuHorizontalPosition == 0 ? attr : 0;
            auto source = switchGetFlexConfig(index);
            lcdDrawText(HW_SETTINGS_COLUMN1, y, (source < 0) ? STR_NONE : adcGetInputLabel(ADC_INPUT_FLEX, source), flags);
            if (flags & (~RIGHT)) source = checkIncDec(event, source, -1, adcGetMaxInputs(ADC_INPUT_FLEX) - 1, (isModelMenuDisplayed()) ? EE_MODEL : EE_GENERAL, isFlexSwitchSourceValid);
            switchConfigFlex(index, source);

            //Name
            flags = menuHorizontalPosition == 1 ? attr : 0;
            if (switchHasCustomName(index) ||
                (attr && s_editMode > 0 && menuHorizontalPosition == 1)) {
              editName(HW_SETTINGS_COLUMN2, y,
                       (char*)switchGetCustomName(index), LEN_SWITCH_NAME,
                       event, flags, 0, old_editMode);
            } else {
              lcdDrawMMM(HW_SETTINGS_COLUMN2, y, flags);
            }

            // Switch type
            flags = menuHorizontalPosition == 2 ? attr : 0;
            config = editChoice(HW_SETTINGS_COLUMN2 + 25, y, "", STR_SWTYPES, config,
                           SWITCH_NONE, switchGetMaxType(index), flags, event);

            if (attr && checkIncDec_Ret) {
              swconfig_t mask = SWITCH_CONFIG_MASK(index);
              g_eeGeneral.switchConfig =
                  (g_eeGeneral.switchConfig & ~mask) |
                  ((swconfig_t(config) & SW_CFG_MASK) << (SW_CFG_BITS * index));
            }
          }
          else {
            flags = menuHorizontalPosition == 0 ? attr : 0;
            if (switchHasCustomName(index) ||
                (attr && s_editMode > 0 && menuHorizontalPosition == 0)) {
              editName(HW_SETTINGS_COLUMN1, y,
                       (char*)switchGetCustomName(index), LEN_SWITCH_NAME,
                       event, flags, 0, old_editMode);
            } else {
              lcdDrawMMM(HW_SETTINGS_COLUMN1, y, flags);
            }

            flags = menuHorizontalPosition == 1 ? attr : 0;
            config =
                editChoice(HW_SETTINGS_COLUMN2, y, "", STR_SWTYPES, config,
                           SWITCH_NONE, switchGetMaxType(index), flags, event);

            if (attr && checkIncDec_Ret) {
              swconfig_t mask = SWITCH_CONFIG_MASK(index);
              g_eeGeneral.switchConfig =
                  (g_eeGeneral.switchConfig & ~mask) |
                  ((swconfig_t(config) & SW_CFG_MASK) << (SW_CFG_BITS * index));
            }
          }
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
