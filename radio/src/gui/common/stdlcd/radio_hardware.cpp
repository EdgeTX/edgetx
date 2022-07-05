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

#include <opentx.h>
#include "hal/adc_driver.h"
#include "aux_serial_driver.h"

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

#if defined(PCBTARANIS)
enum {
  ITEM_RADIO_HARDWARE_LABEL_STICKS,
  ITEM_RADIO_HARDWARE_STICK1,
  ITEM_RADIO_HARDWARE_STICK2,
  ITEM_RADIO_HARDWARE_STICK3,
  ITEM_RADIO_HARDWARE_STICK4,
#if (NUM_POTS + NUM_SLIDERS) > 0
  ITEM_RADIO_HARDWARE_LABEL_POTS,
  ITEM_RADIO_HARDWARE_POT1,
#endif
#if defined(HARDWARE_POT2)
  ITEM_RADIO_HARDWARE_POT2,
#endif
#if defined(HARDWARE_POT3) || defined(PCBX9D)  // TODO #if defined(STORAGE_POT3)
  ITEM_RADIO_HARDWARE_POT3,
#endif
#if defined(HARDWARE_POT4)
  ITEM_RADIO_HARDWARE_POT4,
#endif
#if NUM_SLIDERS > 0
  ITEM_RADIO_HARDWARE_SLIDER1,
  ITEM_RADIO_HARDWARE_SLIDER2,
#endif
#if NUM_SLIDERS > 2
  ITEM_RADIO_HARDWARE_SLIDER3,
  ITEM_RADIO_HARDWARE_SLIDER4,
#endif
  ITEM_RADIO_HARDWARE_LABEL_SWITCHES,
  ITEM_RADIO_HARDWARE_SA,
  ITEM_RADIO_HARDWARE_SB,
  ITEM_RADIO_HARDWARE_SC,
  ITEM_RADIO_HARDWARE_SD,
#if !defined(RADIO_TPRO)
#if defined(HARDWARE_SWITCH_E)
  ITEM_RADIO_HARDWARE_SE,
#endif
#if defined(HARDWARE_SWITCH_F)
  ITEM_RADIO_HARDWARE_SF,
#endif
#if defined(HARDWARE_SWITCH_G)
  ITEM_RADIO_HARDWARE_SG,
#endif
#if defined(HARDWARE_SWITCH_H)
  ITEM_RADIO_HARDWARE_SH,
#endif
#if defined(HARDWARE_SWITCH_I)
  ITEM_RADIO_HARDWARE_SI,
#endif
#if defined(HARDWARE_SWITCH_J)
  ITEM_RADIO_HARDWARE_SJ,
#endif
#endif
#if defined(HARDWARE_SWITCH_K)
  ITEM_RADIO_HARDWARE_SK,
  ITEM_RADIO_HARDWARE_SL,
  ITEM_RADIO_HARDWARE_SM,
  ITEM_RADIO_HARDWARE_SN,
  ITEM_RADIO_HARDWARE_SO,
  ITEM_RADIO_HARDWARE_SP,
  ITEM_RADIO_HARDWARE_SQ,
  ITEM_RADIO_HARDWARE_SR,
#endif

  ITEM_RADIO_HARDWARE_BATTERY_CALIB,

  ITEM_RADIO_HARDWARE_RTC_BATTERY,
  ITEM_RADIO_HARDWARE_RTC_CHECK,

#if defined(TX_CAPACITY_MEASUREMENT)
  ITEM_RADIO_HARDWARE_CAPACITY_CALIB,
#endif

#if !defined(PCBX9D) && !defined(PCBX9DP) && !defined(PCBX9E)
  ITEM_RADIO_HARDWARE_LABEL_INTERNAL_MODULE,
  ITEM_RADIO_HARDWARE_INTERNAL_MODULE_TYPE,
  ITEM_RADIO_HARDWARE_INTERNAL_MODULE_BAUDRATE,
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE,
#endif
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

#if defined(AUX_SERIAL) || defined(USB_SERIAL)
  ITEM_RADIO_HARDWARE_SERIAL_PORT_LABEL,
  ITEM_RADIO_HARDWARE_AUX1_SERIAL_MODE,
  ITEM_RADIO_HARDWARE_AUX2_SERIAL_MODE,
  ITEM_RADIO_HARDWARE_VCP_SERIAL_MODE,
#endif

  ITEM_RADIO_HARDWARE_JITTER_FILTER,
  ITEM_RADIO_HARDWARE_RAS,
#if defined(SPORT_UPDATE_PWR_GPIO)
  ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER,
#endif
  ITEM_RADIO_HARDWARE_DEBUG,
#if defined(EEPROM)
  ITEM_RADIO_BACKUP_EEPROM,
  ITEM_RADIO_FACTORY_RESET,
#endif
  ITEM_RADIO_HARDWARE_MAX
};

#if (NUM_POTS + NUM_SLIDERS) == 0
  #define POTS_ROWS
#elif (NUM_POTS + NUM_SLIDERS) == 1
  #define POTS_ROWS                LABEL(Pots), NAVIGATION_LINE_BY_LINE|1,
#elif (NUM_POTS + NUM_SLIDERS) == 2
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#elif (NUM_POTS + NUM_SLIDERS) == 3
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#elif defined(PCBX9D) // TODO defined(STORAGE_POT3) && !defined(STORAGE_POT3)
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, HIDDEN_ROW, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#elif (NUM_POTS + NUM_SLIDERS) == 4
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#elif (NUM_POTS + NUM_SLIDERS) == 5
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#elif (NUM_POTS + NUM_SLIDERS) == 8
  #define POTS_ROWS               LABEL(Pots), NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1,
#endif

#if defined(PCBX9E)
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif defined(RADIO_TPRO)
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif defined(RADIO_T12)
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|HIDDEN_ROW, NAVIGATION_LINE_BY_LINE|HIDDEN_ROW
#elif NUM_SWITCHES == 9
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif NUM_SWITCHES == 8
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif NUM_SWITCHES == 7
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif NUM_SWITCHES == 6
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif NUM_SWITCHES == 5
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#elif NUM_SWITCHES == 4
  #define SWITCHES_ROWS           NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1, NAVIGATION_LINE_BY_LINE|1
#endif

#if !defined(BLUETOOTH)
  #define BLUETOOTH_ROWS
#elif defined(PCBX9E)
  #define BLUETOOTH_ROWS                 0, uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : READONLY_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : READONLY_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : 0), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : 0),
#else
  #define BLUETOOTH_ROWS                 uint8_t(IS_BLUETOOTH_CHIP_PRESENT() ? 0 : HIDDEN_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY ? READONLY_ROW : HIDDEN_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : READONLY_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : READONLY_ROW), uint8_t(g_eeGeneral.bluetoothMode == BLUETOOTH_OFF ? HIDDEN_ROW : 0),
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  #define EXTERNAL_ANTENNA_ROW           0,
  void onHardwareAntennaSwitchConfirm(const char * result)
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
#else
  #define EXTERNAL_ANTENNA_ROW
#endif

#if defined(PCBX9LITE)
  #define SWITCH_TYPE_MAX(sw)            (sw == MIXSRC_SD-MIXSRC_FIRST_SWITCH ? SWITCH_2POS : SWITCH_3POS)
#elif defined(PCBXLITES)
  #define SWITCH_TYPE_MAX(sw)            (sw >= MIXSRC_SE-MIXSRC_FIRST_SWITCH ? SWITCH_2POS : SWITCH_3POS)
#elif defined(PCBXLITE)
  #define SWITCH_TYPE_MAX(sw)            (SWITCH_3POS)
#elif defined(PCBX9E)
  #define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SF - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH - MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)
#elif defined(RADIO_ZORRO)
  #define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SB - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SC - MIXSRC_FIRST_SWITCH == sw) ? SWITCH_3POS : SWITCH_2POS)
#elif defined(RADIO_T8)
  #define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SA - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SD - MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)
#elif defined(RADIO_TX12)
  #define SWITCH_TYPE_MAX(sw)            (((MIXSRC_SA - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SD - MIXSRC_FIRST_SWITCH == sw) || \
                                           (MIXSRC_SI - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SJ - MIXSRC_FIRST_SWITCH == sw)) ? SWITCH_2POS : SWITCH_3POS)
#elif defined(RADIO_TX12MK2)
  #define SWITCH_TYPE_MAX(sw)            (((MIXSRC_SA - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SD - MIXSRC_FIRST_SWITCH == sw)) ? SWITCH_2POS : SWITCH_3POS)
#elif defined(RADIO_T12)
  #define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SG - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH - MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)
#else
  #define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SF - MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH - MIXSRC_FIRST_SWITCH <= sw) ? SWITCH_2POS : SWITCH_3POS)
#endif

  #define RTC_ROW                        READONLY_ROW, 0,

#if defined(TX_CAPACITY_MEASUREMENT)
  #define TX_CAPACITY_MEASUREMENT_ROWS   0,
#else
  #define TX_CAPACITY_MEASUREMENT_ROWS
#endif

#if !defined(PCBX9D) && !defined(PCBX9DP) && !defined(PCBX9E)
  #define INTERNAL_MODULE_ROWS \
    LABEL(InternalModule), 0,  \
        (isInternalModuleCrossfire() ? (uint8_t)0 : HIDDEN_ROW),
#else
  #define INTERNAL_MODULE_ROWS
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
#define SERIAL_SAMPLE_MODE_ROWS          0,
#else
#define SERIAL_SAMPLE_MODE_ROWS
#endif

#if defined(AUX_SERIAL) || defined(USB_SERIAL)
static uint8_t _dispSerialPort(uint8_t port_nr)
{
  auto port = serialGetPort(port_nr);
  if (!port || !port->name) return HIDDEN_ROW;
  return 0;
}
#define SERIAL_PORT_ROWS                                                  \
  LABEL(SerialPorts), _dispSerialPort(SP_AUX1), _dispSerialPort(SP_AUX2), \
      _dispSerialPort(SP_VCP),
#else
  #define SERIAL_PORT_ROWS
#endif

#if LCD_W >= 212
  #define HW_SETTINGS_COLUMN1            12*FW
  #define HW_SETTINGS_COLUMN2            (20*FW - 3)
#else
  #define HW_SETTINGS_COLUMN1            30
  #define HW_SETTINGS_COLUMN2            (HW_SETTINGS_COLUMN1 + 5*FW)
#endif

#if defined(SPORT_UPDATE_PWR_GPIO)
  #define SPORT_POWER_ROWS 0,
#else
  #define SPORT_POWER_ROWS
#endif

#if defined(EEPROM)
#define EEPROM_ROWS 0 /* EEPROM backup */, 0 /* Factory reset */,
void onFactoryResetConfirm(const char* result)
{
  if (result == STR_OK) {
    showMessageBox(STR_STORAGE_FORMAT);
    storageEraseAll(false);
    NVIC_SystemReset();
  }
}
#else
#define EEPROM_ROWS
#endif

static bool _isAux1ModeAvailable(int m) { return isSerialModeAvailable(SP_AUX1, m); }
static bool _isAux2ModeAvailable(int m) { return isSerialModeAvailable(SP_AUX2, m); }
static bool _isVCPModeAvailable(int m) { return isSerialModeAvailable(SP_VCP, m); }

static const IsValueAvailable _isSerialModeAvailable[MAX_SERIAL_PORTS] = {
  _isAux1ModeAvailable, _isAux2ModeAvailable, _isVCPModeAvailable
};

void menuRadioHardware(event_t event)
{
  uint8_t old_editMode = s_editMode;
  
  MENU(STR_HARDWARE, menuTabGeneral, MENU_RADIO_HARDWARE, HEADER_LINE + ITEM_RADIO_HARDWARE_MAX, {
    HEADER_LINE_COLUMNS
    0 /* calibration button */,
      0 /* stick 1 */,
      0 /* stick 2 */,
      0 /* stick 3 */,
      0 /* stick 4 */,
    POTS_ROWS
    LABEL(Switches),
      SWITCHES_ROWS,
    0 /* battery calib */,
    RTC_ROW
    TX_CAPACITY_MEASUREMENT_ROWS
    INTERNAL_MODULE_ROWS
    SERIAL_SAMPLE_MODE_ROWS
    BLUETOOTH_ROWS
    EXTERNAL_ANTENNA_ROW
    SERIAL_PORT_ROWS
    0 /* ADC filter */,
    READONLY_ROW /* RAS */,
    SPORT_POWER_ROWS
    1 /* debugs */,
    EEPROM_ROWS
  });

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
    uint8_t k = i+menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        k++;
      }
    }
    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch(k) {
      case ITEM_RADIO_HARDWARE_LABEL_STICKS:
        lcdDrawTextAlignedLeft(y, STR_STICKS);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, BUTTON(TR_CALIBRATION), attr);
        if (attr && event == EVT_KEY_FIRST(KEY_ENTER)) {
          pushMenu(menuRadioCalibration);
        }
        break;

      case ITEM_RADIO_HARDWARE_STICK1:
      case ITEM_RADIO_HARDWARE_STICK2:
      case ITEM_RADIO_HARDWARE_STICK3:
      case ITEM_RADIO_HARDWARE_STICK4:
        editStickHardwareSettings(HW_SETTINGS_COLUMN1, y,
                                  k - ITEM_RADIO_HARDWARE_STICK1, event,
                                  attr, old_editMode);
        break;

#if (NUM_POTS + NUM_SLIDERS) > 0
      case ITEM_RADIO_HARDWARE_LABEL_POTS:
        lcdDrawTextAlignedLeft(y, STR_POTS);
        break;

      case ITEM_RADIO_HARDWARE_POT1:
#if defined(HARDWARE_POT2)
      case ITEM_RADIO_HARDWARE_POT2:
#endif
#if defined(HARDWARE_POT3)
      case ITEM_RADIO_HARDWARE_POT3:
#endif
#if defined(HARDWARE_POT4)
      case ITEM_RADIO_HARDWARE_POT4:
#endif
      {
        int idx = k - ITEM_RADIO_HARDWARE_POT1;
        uint8_t shift = (2*idx);
        uint8_t mask = (0x03 << shift);
        lcdDrawTextAtIndex(INDENT_WIDTH, y, STR_VSRCRAW, NUM_STICKS+idx+1, menuHorizontalPosition < 0 ? attr : 0);
        if (g_eeGeneral.anaNames[NUM_STICKS+idx][0] || (attr && s_editMode > 0 && menuHorizontalPosition == 0))
          editName(HW_SETTINGS_COLUMN1, y,
                   g_eeGeneral.anaNames[NUM_STICKS + idx], LEN_ANA_NAME, event,
                   attr && menuHorizontalPosition == 0, 0, old_editMode);
        else
          lcdDrawMMM(HW_SETTINGS_COLUMN1, y, menuHorizontalPosition==0 ? attr : 0);
        uint8_t potType = (g_eeGeneral.potsConfig & mask) >> shift;
        potType = editChoice(HW_SETTINGS_COLUMN2, y, "", STR_POTTYPES, potType, POT_NONE, POT_WITHOUT_DETENT, menuHorizontalPosition == 1 ? attr : 0, event);
        g_eeGeneral.potsConfig &= ~mask;
        g_eeGeneral.potsConfig |= (potType << shift);
        break;
      }

#if NUM_SLIDERS > 0
      case ITEM_RADIO_HARDWARE_SLIDER1:
      case ITEM_RADIO_HARDWARE_SLIDER2:
#if NUM_SLIDERS > 2
      case ITEM_RADIO_HARDWARE_SLIDER3:
      case ITEM_RADIO_HARDWARE_SLIDER4:
#endif
      {
        int idx = k - ITEM_RADIO_HARDWARE_SLIDER1;
        lcdDrawTextAtIndex(INDENT_WIDTH, y, STR_VSRCRAW, NUM_STICKS+NUM_POTS+idx+1, menuHorizontalPosition < 0 ? attr : 0);
        if (ZEXIST(g_eeGeneral.anaNames[NUM_STICKS+NUM_POTS+idx]) || (attr && menuHorizontalPosition == 0 && s_editMode > 0))
          editName(HW_SETTINGS_COLUMN1, y,
                   g_eeGeneral.anaNames[NUM_STICKS + NUM_POTS + idx],
                   LEN_ANA_NAME, event, attr && menuHorizontalPosition == 0,
                   0, old_editMode);
        else
          lcdDrawMMM(HW_SETTINGS_COLUMN1, y, menuHorizontalPosition == 0 ? attr : 0);
        uint8_t mask = (0x01 << idx);
        uint8_t potType = (g_eeGeneral.slidersConfig & mask) >> idx;
        potType = editChoice(HW_SETTINGS_COLUMN2, y, "", STR_SLIDERTYPES, potType, SLIDER_NONE, SLIDER_WITH_DETENT, menuHorizontalPosition == 1 ? attr : 0, event);
        g_eeGeneral.slidersConfig &= ~mask;
        g_eeGeneral.slidersConfig |= (potType << idx);
        break;
      }
#endif
#endif

      case ITEM_RADIO_HARDWARE_LABEL_SWITCHES:
        lcdDrawTextAlignedLeft(y, STR_SWITCHES);
        break;

      case ITEM_RADIO_HARDWARE_SA:
      case ITEM_RADIO_HARDWARE_SB:
      case ITEM_RADIO_HARDWARE_SC:
      case ITEM_RADIO_HARDWARE_SD:
#if !defined(RADIO_TPRO)
#if defined(HARDWARE_SWITCH_E)
      case ITEM_RADIO_HARDWARE_SE:
#endif
#if defined(HARDWARE_SWITCH_F)
      case ITEM_RADIO_HARDWARE_SF:
#endif
#if defined(HARDWARE_SWITCH_G)
      case ITEM_RADIO_HARDWARE_SG:
#endif
#if defined(HARDWARE_SWITCH_H)
      case ITEM_RADIO_HARDWARE_SH:
#endif
#if defined(HARDWARE_SWITCH_I)
      case ITEM_RADIO_HARDWARE_SI:
#endif
#if defined(HARDWARE_SWITCH_J)
      case ITEM_RADIO_HARDWARE_SJ:
#endif
#endif
#if defined(HARDWARE_SWITCH_K)
      case ITEM_RADIO_HARDWARE_SK:
      case ITEM_RADIO_HARDWARE_SL:
      case ITEM_RADIO_HARDWARE_SM:
      case ITEM_RADIO_HARDWARE_SN:
      case ITEM_RADIO_HARDWARE_SO:
      case ITEM_RADIO_HARDWARE_SP:
      case ITEM_RADIO_HARDWARE_SQ:
      case ITEM_RADIO_HARDWARE_SR:
#endif
      {
        int index = k - ITEM_RADIO_HARDWARE_SA;
        int config = SWITCH_CONFIG(index);
        lcdDrawTextAtIndex(INDENT_WIDTH, y, STR_VSRCRAW, MIXSRC_FIRST_SWITCH - MIXSRC_Rud + index + 1, menuHorizontalPosition < 0 ? attr : 0);
        if (ZEXIST(g_eeGeneral.switchNames[index]) || (attr && s_editMode > 0 && menuHorizontalPosition == 0))
          editName(HW_SETTINGS_COLUMN1, y, g_eeGeneral.switchNames[index],
                   LEN_SWITCH_NAME, event,
                   menuHorizontalPosition == 0 ? attr : 0, 0, old_editMode);
        else
          lcdDrawMMM(HW_SETTINGS_COLUMN1, y, menuHorizontalPosition == 0 ? attr : 0);
        config = editChoice(HW_SETTINGS_COLUMN2, y, "", STR_SWTYPES, config, SWITCH_NONE, SWITCH_TYPE_MAX(index), menuHorizontalPosition == 1 ? attr : 0, event);
        if (attr && checkIncDec_Ret) {
          swconfig_t mask = (swconfig_t)0x03 << (2*index);
          g_eeGeneral.switchConfig = (g_eeGeneral.switchConfig & ~mask) | ((swconfig_t(config) & 0x03) << (2*index));
        }
        break;
      }

      case ITEM_RADIO_HARDWARE_BATTERY_CALIB:
#if defined(PCBTARANIS)
        lcdDrawTextAlignedLeft(y, STR_BATT_CALIB);
        putsVolts(HW_SETTINGS_COLUMN2, y, getBatteryVoltage(), attr|PREC2|LEFT);
#else
        lcdDrawTextAlignedLeft(MENU_HEADER_HEIGHT + 1 + (NUM_STICKS+NUM_POTS+NUM_SLIDERS+1)/2 * FH, STR_BATT_CALIB);
        putsVolts(HW_SETTINGS_COLUMN2, y, g_vbat100mV, attr|LEFT);
#endif
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

#if defined(TX_CAPACITY_MEASUREMENT)
      case ITEM_RADIO_HARDWARE_CAPACITY_CALIB:
        lcdDrawTextAlignedLeft(y, STR_CURRENT_CALIB);
        drawValueWithUnit(HW_SETTINGS_COLUMN2, y, getCurrent(), UNIT_MILLIAMPS, attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.txCurrentCalibration, -49, 49);
        }
        break;
#endif

#if !defined(PCBX9D) && !defined(PCBX9DP) && !defined(PCBX9E)
      case ITEM_RADIO_HARDWARE_LABEL_INTERNAL_MODULE:
        lcdDrawTextAlignedLeft(y, STR_INTERNALRF);
        break;

      case ITEM_RADIO_HARDWARE_INTERNAL_MODULE_TYPE: {
        g_eeGeneral.internalModule =
            editChoice(HW_SETTINGS_COLUMN2, y, STR_TYPE,
                       STR_INTERNAL_MODULE_PROTOCOLS,
                       g_eeGeneral.internalModule, MODULE_TYPE_NONE,
                       MODULE_TYPE_MAX, attr, event, isInternalModuleSupported);
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
#endif

#if !defined(PCBX9D) && !defined(PCBX9DP) && !defined(PCBX9E)
      case ITEM_RADIO_HARDWARE_INTERNAL_MODULE_BAUDRATE:
        lcdDrawText(INDENT_WIDTH, y, STR_BAUDRATE);
        lcdDrawTextAtIndex(HW_SETTINGS_COLUMN2, y, STR_CRSF_BAUDRATE, CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate),attr | LEFT);
        if (attr) {
          g_eeGeneral.internalModuleBaudrate = CROSSFIRE_INDEX_TO_STORE(checkIncDecModel(event, CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate), 0, CROSSFIRE_MAX_INTERNAL_BAUDRATE));
          if (checkIncDec_Ret) {
              restartModule(INTERNAL_MODULE);
          }
        }
        break;
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_RADIO_HARDWARE_SERIAL_SAMPLE_MODE:
        g_eeGeneral.uartSampleMode = editChoice(HW_SETTINGS_COLUMN2, y, STR_SAMPLE_MODE, STR_SAMPLE_MODES, g_eeGeneral.uartSampleMode, 0, UART_SAMPLE_MODE_MAX, attr, event);
        if (attr && checkIncDec_Ret) {
          restartModule(EXTERNAL_MODULE);
        }
        break;
#endif
#if defined(BLUETOOTH)
      case ITEM_RADIO_HARDWARE_BLUETOOTH_MODE:
        lcdDrawTextAlignedLeft(y, STR_BLUETOOTH);
        lcdDrawTextAtIndex(HW_SETTINGS_COLUMN2, y, STR_BLUETOOTH_MODES, g_eeGeneral.bluetoothMode, attr);
        if (attr) {
          g_eeGeneral.bluetoothMode = checkIncDecGen(event, g_eeGeneral.bluetoothMode, BLUETOOTH_OFF, BLUETOOTH_MAX);
        }
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_PAIRING_CODE:
        lcdDrawText(INDENT_WIDTH, y, STR_BLUETOOTH_PIN_CODE);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, "000000");
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_LOCAL_ADDR:
        lcdDrawText(INDENT_WIDTH, y, STR_BLUETOOTH_LOCAL_ADDR);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, bluetooth.localAddr[0] == '\0' ? "---" : bluetooth.localAddr);
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_DISTANT_ADDR:
        lcdDrawText(INDENT_WIDTH, y, STR_BLUETOOTH_DIST_ADDR);
        lcdDrawText(HW_SETTINGS_COLUMN2, y, bluetooth.distantAddr[0] == '\0' ? "---" : bluetooth.distantAddr);
        break;

      case ITEM_RADIO_HARDWARE_BLUETOOTH_NAME:
        lcdDrawText(INDENT_WIDTH, y, STR_NAME);
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

#if defined(AUX_SERIAL) || defined(USB_SERIAL)
      case ITEM_RADIO_HARDWARE_SERIAL_PORT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_AUX_SERIAL_MODE);
        break;
      case ITEM_RADIO_HARDWARE_AUX1_SERIAL_MODE:
      case ITEM_RADIO_HARDWARE_AUX2_SERIAL_MODE:
      case ITEM_RADIO_HARDWARE_VCP_SERIAL_MODE:
      {
        auto port_nr = k - ITEM_RADIO_HARDWARE_AUX1_SERIAL_MODE;
        auto port = serialGetPort(port_nr);
        if (port && port->name) {
          lcdDrawText(INDENT_WIDTH, y, port->name);

          auto mode = serialGetMode(port_nr);
          mode = editChoice(HW_SETTINGS_COLUMN2, y, nullptr,
                            STR_AUX_SERIAL_MODES, mode, 0, UART_MODE_MAX, attr,
                            event, _isSerialModeAvailable[port_nr]);

          if (attr && checkIncDec_Ret) {
            serialSetMode(port_nr, mode);
            serialInit(port_nr, mode);
          }
        }
      } break;
#endif
      case ITEM_RADIO_HARDWARE_JITTER_FILTER:
        g_eeGeneral.noJitterFilter = 1 - editCheckBox(1 - g_eeGeneral.noJitterFilter, HW_SETTINGS_COLUMN2, y, STR_JITTER_FILTER, attr, event);
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

#if defined(SPORT_UPDATE_PWR_GPIO)
      case ITEM_RADIO_HARDWARE_SPORT_UPDATE_POWER:
        g_eeGeneral.sportUpdatePower = editChoice(HW_SETTINGS_COLUMN2, y, STR_SPORT_UPDATE_POWER_MODE, STR_SPORT_UPDATE_POWER_MODES, g_eeGeneral.sportUpdatePower, 0, 1, attr, event);
        if (attr && checkIncDec_Ret) {
          SPORT_UPDATE_POWER_INIT();
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

#if defined(EEPROM)
      case ITEM_RADIO_BACKUP_EEPROM:
        if (LCD_W < 212)
          lcdDrawText(LCD_W / 2, y, BUTTON(STR_EEBACKUP), attr | CENTERED);
        else
          lcdDrawText(HW_SETTINGS_COLUMN2, y, BUTTON(STR_EEBACKUP), attr);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = EDIT_SELECT_FIELD;
          eepromBackup();
        }
        break;

      case ITEM_RADIO_FACTORY_RESET:
        if (LCD_W < 212)
          lcdDrawText(LCD_W / 2, y, BUTTON(STR_FACTORYRESET), attr | CENTERED);
        else
          lcdDrawText(HW_SETTINGS_COLUMN2, y, BUTTON(STR_FACTORYRESET), attr);
        if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = EDIT_SELECT_FIELD;
          POPUP_CONFIRMATION(STR_CONFIRMRESET, onFactoryResetConfirm);
        }
        break;
#endif
    }
  }
}
#endif
