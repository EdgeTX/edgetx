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

#include "opentx.h"
#include "mixer_scheduler.h"

#if !defined(EEPROM)
#include "storage/sdcard_common.h"
#include "storage/modelslist.h"
#endif

#if defined(CROSSFIRE)
  #include "telemetry/crossfire.h"
#endif

// TODO find why we need this (for REGISTER at least)
#if defined(PCBXLITE)
  #define EVT_BUTTON_PRESSED() EVT_KEY_FIRST(KEY_ENTER)
#else
  #define EVT_BUTTON_PRESSED() EVT_KEY_BREAK(KEY_ENTER)
#endif

uint8_t g_moduleIdx;

#if defined(PCBTARANIS)
uint8_t getSwitchWarningsCount()
{
  uint8_t count = 0;
  for (int i=0; i<NUM_SWITCHES - NUM_FUNCTIONS_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      ++count;
    }
  }
  return count;
}
#endif

enum MenuModelSetupItems {
  ITEM_MODEL_SETUP_NAME,
  ITEM_MODEL_SETUP_TIMER1,
  ITEM_MODEL_SETUP_TIMER1_NAME,
  ITEM_MODEL_SETUP_TIMER1_START,
  ITEM_MODEL_SETUP_TIMER1_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER1_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER1_COUNTDOWN_BEEP,
  ITEM_MODEL_SETUP_TIMER2,
  ITEM_MODEL_SETUP_TIMER2_NAME,
  ITEM_MODEL_SETUP_TIMER2_START,
  ITEM_MODEL_SETUP_TIMER2_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER2_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER2_COUNTDOWN_BEEP,
  ITEM_MODEL_SETUP_TIMER3,
  ITEM_MODEL_SETUP_TIMER3_NAME,
  ITEM_MODEL_SETUP_TIMER3_START,
  ITEM_MODEL_SETUP_TIMER3_PERSISTENT,
  ITEM_MODEL_SETUP_TIMER3_MINUTE_BEEP,
  ITEM_MODEL_SETUP_TIMER3_COUNTDOWN_BEEP,
#if defined(FUNCTION_SWITCHES)
  ITEM_MODEL_SETUP_LABEL,
  ITEM_MODEL_SETUP_SW1,
  ITEM_MODEL_SETUP_SW2,
  ITEM_MODEL_SETUP_SW3,
  ITEM_MODEL_SETUP_SW4,
  ITEM_MODEL_SETUP_SW5,
  ITEM_MODEL_SETUP_SW6,
  ITEM_MODEL_SETUP_FS_STARTUP,
#endif
  ITEM_MODEL_SETUP_EXTENDED_LIMITS,
  ITEM_MODEL_SETUP_EXTENDED_TRIMS,
  ITEM_MODEL_SETUP_DISPLAY_TRIMS,
  ITEM_MODEL_SETUP_TRIM_INC,
  ITEM_MODEL_SETUP_THROTTLE_REVERSED,
  ITEM_MODEL_SETUP_THROTTLE_TRACE,
  ITEM_MODEL_SETUP_THROTTLE_TRIM,
  ITEM_MODEL_SETUP_THROTTLE_TRIM_SWITCH,
  ITEM_MODEL_SETUP_PREFLIGHT_LABEL,
  ITEM_MODEL_SETUP_CHECKLIST_DISPLAY,
  ITEM_MODEL_SETUP_THROTTLE_WARNING,
  ITEM_MODEL_SETUP_CUSTOM_THROTTLE_WARNING,
  ITEM_MODEL_SETUP_CUSTOM_THROTTLE_WARNING_VALUE,
  ITEM_MODEL_SETUP_SWITCHES_WARNING1,
#if defined(PCBTARANIS)
  ITEM_MODEL_SETUP_SWITCHES_WARNING2,
  ITEM_MODEL_SETUP_POTS_WARNING,
#endif
  ITEM_MODEL_SETUP_BEEP_CENTER,
  ITEM_MODEL_SETUP_USE_GLOBAL_FUNCTIONS,
  ITEM_MODEL_SETUP_USE_JITTER_FILTER,
#if defined(PXX2)
  ITEM_MODEL_SETUP_REGISTRATION_ID,
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_LABEL,
#if defined(CROSSFIRE) || defined(GHOST)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_SERIALSTATUS,
#endif
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PROTOCOL,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_SUBTYPE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_STATUS,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_SYNCSTATUS,
#endif
  ITEM_MODEL_SETUP_INTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_RACING_MODE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_OPTIONS,
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_AUTOBIND,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_DISABLE_TELEM,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_DISABLE_MAPPING,
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  ITEM_MODEL_SETUP_INTERNAL_MODULE_ANTENNA,
#endif
  ITEM_MODEL_SETUP_INTERNAL_MODULE_POWER,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_REGISTER_RANGE,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_OPTIONS,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_2,
  ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_3,
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_TYPE,
#if defined(CROSSFIRE) || defined(GHOST)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_BAUDRATE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_SERIALSTATUS,
#endif
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PROTOCOL,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_SUBTYPE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_STATUS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_SYNCSTATUS,
#endif
#if defined(AFHDS3)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS_PROTOCOL,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_MODE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_STATUS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_POWER_STATUS,
#endif
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_CHANNELS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_MODEL_NUM,
#if defined(AFHDS3)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_RX_FREQ,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_ACTUAL_POWER,
#endif
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_POWER,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_OPTIONS,
#if defined(MULTIMODULE)
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_AUTOBIND,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_DISABLE_TELEM,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_DISABLE_MAPPING,
#endif

  ITEM_MODEL_SETUP_EXTERNAL_MODULE_FAILSAFE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_REGISTER_RANGE,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_OPTIONS,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_1,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_2,
  ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_3,

#endif

#if defined(PCBTARANIS)
  ITEM_MODEL_SETUP_TRAINER_LABEL,
  ITEM_MODEL_SETUP_TRAINER_MODE,
  #if defined(BLUETOOTH)
  ITEM_MODEL_SETUP_TRAINER_BLUETOOTH,
  #endif
  ITEM_MODEL_SETUP_TRAINER_CHANNELS,
  ITEM_MODEL_SETUP_TRAINER_PPM_PARAMS,
#endif
  ITEM_MODEL_SETUP_LINES_COUNT
};

#define MODEL_SETUP_2ND_COLUMN           (LCD_W-11*FW)
#define MODEL_SETUP_SET_FAILSAFE_OFS     7*FW-2

#define IF_PXX2_MODULE(module, xxx)      (isModulePXX2(module) ? (uint8_t)(xxx) : HIDDEN_ROW)
#define IF_NOT_PXX2_MODULE(module, xxx)  (isModulePXX2(module) ? HIDDEN_ROW : (uint8_t)(xxx))
#define IF_ACCESS_MODULE_RF(module, xxx) (isModuleRFAccess(module) ? (uint8_t)(xxx) : HIDDEN_ROW)
#define IF_NOT_ACCESS_MODULE_RF(module, xxx) (isModuleRFAccess(module) ? HIDDEN_ROW : (uint8_t)(xxx))
#if defined(CROSSFIRE) || defined(GHOST)
#define IF_MODULE_SYNCED(module, xxx)    ((isModuleCrossfire(module) || isModuleGhost(module)) ? (uint8_t)(xxx) : HIDDEN_ROW)
#if SPORT_MAX_BAUDRATE < 400000
#define IF_MODULE_BAUDRATE_ADJUST(module, xxx) ((isModuleCrossfire(module) || isModuleGhost(module)) ? (uint8_t)(xxx) : HIDDEN_ROW)
#else
#define IF_MODULE_BAUDRATE_ADJUST(module, xxx) (isModuleCrossfire(module) ? (uint8_t)(xxx) : HIDDEN_ROW)
#endif
#else
#define IF_MODULE_SYNCED(module, xxx)
#define IF_MODULE_BAUDRATE_ADJUST(module, xxx)
#endif


#if defined(PXX2)
#define REGISTRATION_ID_ROWS             uint8_t((isDefaultModelRegistrationID() || (warningText && popupFunc == runPopupRegister)) ? HIDDEN_ROW : READONLY_ROW),
#else
#define REGISTRATION_ID_ROWS
#endif

#if defined(HARDWARE_INTERNAL_MODULE) && defined(HARDWARE_EXTERNAL_MODULE)
  #define CURRENT_MODULE_EDITED(k)        (k >= ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL ? EXTERNAL_MODULE : INTERNAL_MODULE)
  #define CURRENT_RECEIVER_EDITED(k)      (k - (k >= ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL ? ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_1 : ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1))
#elif defined(HARDWARE_INTERNAL_MODULE)
  #define CURRENT_MODULE_EDITED(k)        (INTERNAL_MODULE)
  #define CURRENT_RECEIVER_EDITED(k)      (k - ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1)
#else
  #define CURRENT_MODULE_EDITED(k)        (EXTERNAL_MODULE)
  #define CURRENT_RECEIVER_EDITED(k)      (k - ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_1)
#endif

#define MAX_SWITCH_PER_LINE             (getSwitchWarningsCount() > 5 ? 4 : 5)
#if defined(PCBXLITE)
  #define SW_WARN_ROWS                    uint8_t(NAVIGATION_LINE_BY_LINE|getSwitchWarningsCount()), uint8_t(getSwitchWarningsCount() > 4 ? TITLE_ROW : HIDDEN_ROW) // X-Lite needs an additional column for full line selection (<])
#else
  #define SW_WARN_ROWS                    uint8_t(NAVIGATION_LINE_BY_LINE|(getSwitchWarningsCount()-1)), uint8_t(getSwitchWarningsCount() > MAX_SWITCH_PER_LINE ? TITLE_ROW : HIDDEN_ROW)
#endif

inline uint8_t MODULE_TYPE_ROWS(int moduleIdx)
{
  if (isModuleXJT(moduleIdx) || isModuleISRM(moduleIdx) || isModuleR9MNonAccess(moduleIdx) || isModuleDSM2(moduleIdx))
    return 1;
  else
    return 0;
}

inline uint8_t MODULE_SUBTYPE_ROWS(int moduleIdx)
{
#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    return MULTIMODULE_HAS_SUBTYPE(moduleIdx) ? 1 : HIDDEN_ROW;
  }
#endif

  return HIDDEN_ROW;
}

#define POT_WARN_ROWS                  ((g_model.potsWarnMode) ? (uint8_t)(NUM_POTS+NUM_SLIDERS) : (uint8_t)0)
#define TIMER_ROWS(x)                                                  \
  1, 0,                                                                \
      (uint8_t)((g_model.timers[x].start) ? 2 : 1),                   \
      0, 0,                                                            \
      g_model.timers[x].countdownBeep != COUNTDOWN_SILENT ? (uint8_t)1 \
                                                          : (uint8_t)0

#define EXTRA_MODULE_ROWS

#if defined(FUNCTION_SWITCHES)
  #define FUNCTION_SWITCHES_ROWS       READONLY_ROW, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|3, NAVIGATION_LINE_BY_LINE|(NUM_FUNCTIONS_SWITCHES-1),
#else
  #define FUNCTION_SWITCHES_ROWS
#endif

#define TRAINER_CHANNELS_ROW           (IS_SLAVE_TRAINER() ? (IS_BLUETOOTH_TRAINER() ? (uint8_t)0 : (uint8_t)1) : HIDDEN_ROW)
#define TRAINER_PPM_PARAMS_ROW         (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)
#define TRAINER_BLUETOOTH_M_ROW        ((bluetooth.distantAddr[0] == '\0' || bluetooth.state == BLUETOOTH_STATE_CONNECTED) ? (uint8_t)0 : (uint8_t)1)
#define TRAINER_BLUETOOTH_S_ROW        (bluetooth.distantAddr[0] == '\0' ? HIDDEN_ROW : LABEL())
#define IF_BT_TRAINER_ON(x)            (g_eeGeneral.bluetoothMode == BLUETOOTH_TRAINER ? (uint8_t)(x) : HIDDEN_ROW)

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
#define EXTERNAL_ANTENNA_ROW             ((isModuleXJT(INTERNAL_MODULE) && g_eeGeneral.antennaMode == ANTENNA_MODE_PER_MODEL) ? (uint8_t)0 : HIDDEN_ROW),
void onModelAntennaSwitchConfirm(const char * result)
{
  if (result == STR_OK) {
    // Switch to external antenna confirmation
    g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode = ANTENNA_MODE_EXTERNAL;
    globalData.externalAntennaEnabled = true;
    storageDirty(EE_MODEL);
  }
  else {
    reusableBuffer.moduleSetup.antennaMode = g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode;
  }
}
#else
#define EXTERNAL_ANTENNA_ROW
#endif

#if defined(PCBX7ACCESS)
  #define TRAINER_BLUETOOTH_ROW          (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_BLUETOOTH_M_ROW : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? TRAINER_BLUETOOTH_S_ROW : HIDDEN_ROW))
  #define TRAINER_PPM_PARAMS_ROW         (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)
  #define TRAINER_ROWS                   LABEL(Trainer), 0, IF_BT_TRAINER_ON(TRAINER_BLUETOOTH_ROW), TRAINER_CHANNELS_ROW, TRAINER_PPM_PARAMS_ROW
#elif defined(PCBX7) || defined(PCBX9LITE)
  #if defined(BLUETOOTH)
    #define TRAINER_BLUETOOTH_ROW        (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_BLUETOOTH_M_ROW : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? TRAINER_BLUETOOTH_S_ROW : HIDDEN_ROW)),
  #else
    #define TRAINER_BLUETOOTH_ROW
  #endif
  #define TRAINER_PPM_PARAMS_ROW         (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)
  #define TRAINER_ROWS                   LABEL(Trainer), 0, TRAINER_BLUETOOTH_ROW TRAINER_CHANNELS_ROW, TRAINER_PPM_PARAMS_ROW
#elif defined(PCBXLITES)
  #define TRAINER_BLUETOOTH_ROW          (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_BLUETOOTH_M_ROW : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? TRAINER_BLUETOOTH_S_ROW : HIDDEN_ROW))
  #define TRAINER_PPM_PARAMS_ROW         (g_model.trainerData.mode == TRAINER_MODE_SLAVE ? (uint8_t)2 : HIDDEN_ROW)
  #define TRAINER_ROWS                   LABEL(Trainer), 0, IF_BT_TRAINER_ON(TRAINER_BLUETOOTH_ROW), TRAINER_CHANNELS_ROW, TRAINER_PPM_PARAMS_ROW
#elif defined(PCBXLITE)
  #define TRAINER_BLUETOOTH_ROW          (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH ? TRAINER_BLUETOOTH_M_ROW : (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH ? TRAINER_BLUETOOTH_S_ROW : HIDDEN_ROW))
  #define TRAINER_ROWS                   IF_BT_TRAINER_ON(LABEL(Trainer)), IF_BT_TRAINER_ON(0), IF_BT_TRAINER_ON(TRAINER_BLUETOOTH_ROW), IF_BT_TRAINER_ON(TRAINER_CHANNELS_ROW), HIDDEN_ROW /* xlite has only BT trainer, so never PPM */
#else
  #define TRAINER_ROWS
#endif

#if defined(BLUETOOTH)
void onBluetoothConnectMenu(const char * result)
{
  if (result != STR_EXIT) {
    uint8_t index = (result - reusableBuffer.moduleSetup.bt.devices[0]) / sizeof(reusableBuffer.moduleSetup.bt.devices[0]);
    strncpy(bluetooth.distantAddr, reusableBuffer.moduleSetup.bt.devices[index], LEN_BLUETOOTH_ADDR);
    bluetooth.state = BLUETOOTH_STATE_BIND_REQUESTED;
  }
  else {
    reusableBuffer.moduleSetup.bt.devicesCount = 0;
    bluetooth.state = BLUETOOTH_STATE_DISCOVER_END;
  }
}
#endif

void editTimerCountdown(int timerIdx, coord_t y, LcdFlags attr, event_t event)
{
  TimerData & timer = g_model.timers[timerIdx];
  lcdDrawTextAlignedLeft(y, STR_BEEPCOUNTDOWN);
  lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VBEEPCOUNTDOWN, timer.countdownBeep, (menuHorizontalPosition == 0 ? attr : 0));
  if (timer.countdownBeep != COUNTDOWN_SILENT) {
    lcdDrawNumber(MODEL_SETUP_2ND_COLUMN + 6 * FW, y, TIMER_COUNTDOWN_START(timerIdx), (menuHorizontalPosition == 1 ? attr : 0) | LEFT);
    lcdDrawChar(lcdLastRightPos, y, 's');
  }
  if (attr && s_editMode > 0) {
    switch (menuHorizontalPosition) {
      case 0:
        CHECK_INCDEC_MODELVAR(event, timer.countdownBeep, COUNTDOWN_SILENT, COUNTDOWN_COUNT - 1);
        break;
      case 1:
        timer.countdownStart = -checkIncDecModel(event, -timer.countdownStart, -1, +2);
        break;
    }
  }
}

#include "common/stdlcd/model_setup_pxx1.cpp"

#if defined(PXX2)
#include "common/stdlcd/model_setup_pxx2.cpp"
#endif
#if defined(AFHDS3)
#include "common/stdlcd/model_setup_afhds3.cpp"
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
  #define INTERNAL_MODULE_ROWS \
    LABEL(InternalModule), \
    MODULE_TYPE_ROWS(INTERNAL_MODULE),         /* ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE */ \
    IF_MODULE_SYNCED(INTERNAL_MODULE, 0),      /* Sync rate + errors */ \
    MULTIMODULE_TYPE_ROWS(INTERNAL_MODULE)     /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PROTOCOL */ \
    MULTIMODULE_SUBTYPE_ROWS(INTERNAL_MODULE)  /* ITEM_MODEL_SETUP_INTERNAL_MODULE_SUBTYPE */ \
    MULTIMODULE_STATUS_ROWS(INTERNAL_MODULE)   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_STATUS, ITEM_MODEL_SETUP_INTERNAL_MODULE_SYNCSTATUS */ \
    MODULE_CHANNELS_ROWS(INTERNAL_MODULE),     /* ITEM_MODEL_SETUP_INTERNAL_MODULE_CHANNELS */ \
    IF_ALLOW_RACING_MODE(INTERNAL_MODULE),     /* ITEM_MODEL_SETUP_INTERNAL_MODULE_RACING_MODE */ \
    IF_NOT_ACCESS_MODULE_RF(INTERNAL_MODULE, MODULE_BIND_ROWS(INTERNAL_MODULE)), /* *ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE */\
    IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0),   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM */ \
    MODULE_OPTION_ROW(INTERNAL_MODULE),        /* ITEM_MODEL_SETUP_INTERNAL_MODULE_OPTIONS */ \
    MULTIMODULE_MODULE_ROWS(INTERNAL_MODULE)   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_AUTOBIND */  \
    EXTERNAL_ANTENNA_ROW                       /* ITEM_MODEL_SETUP_INTERNAL_MODULE_ANTENNA */ \
    MODULE_POWER_ROW(INTERNAL_MODULE),         /* ITEM_MODEL_SETUP_INTERNAL_MODULE_POWER */ \
    IF_INTERNAL_MODULE_ON(FAILSAFE_ROWS(INTERNAL_MODULE)), /* ITEM_MODEL_SETUP_INTERNAL_MODULE_FAILSAFE */ \
    IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 1),   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_REGISTER_RANGE */ \
    IF_PXX2_MODULE(INTERNAL_MODULE, 0),        /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_OPTIONS */ \
    IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0),   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1 */ \
    IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0),   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_2 */ \
    IF_ACCESS_MODULE_RF(INTERNAL_MODULE, 0),   /* ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_3 */
#else
  #define INTERNAL_MODULE_ROWS
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  #define EXTERNAL_MODULE_ROWS \
    LABEL(ExternalModule), \
    MODULE_TYPE_ROWS(EXTERNAL_MODULE),  \
    IF_MODULE_BAUDRATE_ADJUST(EXTERNAL_MODULE, 0), /* Baudrate */ \
    IF_MODULE_SYNCED(EXTERNAL_MODULE, 0),          /* Sync rate + errors */ \
    MULTIMODULE_TYPE_ROWS(EXTERNAL_MODULE)         /* PROTOCOL */ \
    MULTIMODULE_SUBTYPE_ROWS(EXTERNAL_MODULE)      /* SUBTYPE */  \
    MULTIMODULE_STATUS_ROWS(EXTERNAL_MODULE)  \
    AFHDS3_PROTOCOL_ROW(EXTERNAL_MODULE)  \
    AFHDS3_MODE_ROWS(EXTERNAL_MODULE) \
    MODULE_CHANNELS_ROWS(EXTERNAL_MODULE),  \
    IF_NOT_ACCESS_MODULE_RF(EXTERNAL_MODULE, MODULE_BIND_ROWS(EXTERNAL_MODULE)),  /* line reused for PPM: PPM settings */  \
    IF_ACCESS_MODULE_RF(EXTERNAL_MODULE, 0),       /* RxNum */ \
    AFHDS3_MODULE_ROWS(EXTERNAL_MODULE) \
    MODULE_POWER_ROW(EXTERNAL_MODULE),  \
    IF_NOT_PXX2_MODULE(EXTERNAL_MODULE, MODULE_OPTION_ROW(EXTERNAL_MODULE)),  \
    MULTIMODULE_MODULE_ROWS(EXTERNAL_MODULE)  \
    FAILSAFE_ROWS(EXTERNAL_MODULE),               /* ITEM_MODEL_SETUP_EXTERNAL_MODULE_FAILSAFE */ \
    IF_ACCESS_MODULE_RF(EXTERNAL_MODULE, 1),      /* Range check and Register buttons */ \
    IF_PXX2_MODULE(EXTERNAL_MODULE, 0),           /* Module options */ \
    IF_ACCESS_MODULE_RF(EXTERNAL_MODULE, 0),      /* Receiver 1 */ \
    IF_ACCESS_MODULE_RF(EXTERNAL_MODULE, 0),      /* Receiver 2 */ \
    IF_ACCESS_MODULE_RF(EXTERNAL_MODULE, 0),      /* Receiver 3 */
#else
  #define EXTERNAL_MODULE_ROWS
#endif

#if defined(PCBTARANIS)
  #define WARN_ROWS \
    SW_WARN_ROWS, /* Switch warning */ \
    POT_WARN_ROWS, /* Pot warning */
#else
  #define WARN_ROWS \
    NUM_SWITCHES - 1, /* Switch warning */
#endif

static const char* _pots_warn_modes[] = { "OFF", "Man", "Auto" };
static const char* _fct_sw_start[] = { STR_CHAR_UP, STR_CHAR_DOWN, "=" };

void menuModelSetup(event_t event)
{
  int8_t old_editMode = s_editMode;

#if defined(PCBTARANIS)
  int8_t old_posHorz = menuHorizontalPosition;
#endif

  MENU_TAB({
    HEADER_LINE_COLUMNS
    0,
    TIMER_ROWS(0),
    TIMER_ROWS(1),
    TIMER_ROWS(2),
    FUNCTION_SWITCHES_ROWS
    0, // Extended limits
    1, // Extended trims
    0, // Show trims
    0, // Trims step
    0, // Throttle reverse
    0, // Throttle trace source
    0, // Throttle trim
    0, // Throttle trim switch

    LABEL(PreflightCheck),
      0, // Checklist
      0, // Throttle warning
      0, // Custom position for throttle warning enable
      0, // Custom position for throttle warning value
      WARN_ROWS

    NUM_STICKS + NUM_POTS + NUM_SLIDERS - 1, // Center beeps
    0, // Global functions

    0, // ADC Jitter filter

    REGISTRATION_ID_ROWS

    INTERNAL_MODULE_ROWS

    EXTERNAL_MODULE_ROWS

    EXTRA_MODULE_ROWS

    TRAINER_ROWS
  });

  MENU_CHECK(menuTabModel, MENU_MODEL_SETUP, HEADER_LINE + ITEM_MODEL_SETUP_LINES_COUNT);
  title(STR_MENU_MODEL_SETUP);

  if (event == EVT_ENTRY || event == EVT_ENTRY_UP) {
    memclear(&reusableBuffer.moduleSetup, sizeof(reusableBuffer.moduleSetup));
    reusableBuffer.moduleSetup.r9mPower = g_model.moduleData[EXTERNAL_MODULE].pxx.power;
    reusableBuffer.moduleSetup.previousType = g_model.moduleData[EXTERNAL_MODULE].type;
    reusableBuffer.moduleSetup.newType = g_model.moduleData[EXTERNAL_MODULE].type;
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    reusableBuffer.moduleSetup.antennaMode = g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode;
#endif
  }

#if (defined(DSM2) || defined(PXX))
  if (menuEvent) {
    moduleState[0].mode = 0;
#if NUM_MODULES > 1
    moduleState[1].mode = 0;
#endif
  }
#endif

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  for (uint8_t i=0; i<NUM_BODY_LINES; ++i) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
          return;
        }
      }
    }

    uint8_t moduleIdx = CURRENT_MODULE_EDITED(k);
    LcdFlags blink = ((s_editMode > 0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_MODEL_SETUP_NAME:
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, STR_MODELNAME,
                       g_model.header.name, sizeof(g_model.header.name), event,
                       attr, old_editMode);
        memcpy(modelHeaders[g_eeGeneral.currModel].name, g_model.header.name,
               sizeof(g_model.header.name));
        break;

      case ITEM_MODEL_SETUP_TIMER1:
      case ITEM_MODEL_SETUP_TIMER2:
      case ITEM_MODEL_SETUP_TIMER3:
      {
        unsigned int timerIdx = (k >= ITEM_MODEL_SETUP_TIMER3
                                     ? 2
                                     : (k >= ITEM_MODEL_SETUP_TIMER2 ? 1 : 0));

        TimerData *timer = &g_model.timers[timerIdx];
        drawStringWithIndex(0 * FW, y, STR_TIMER, timerIdx + 1);

        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VTMRMODES,
                           timer->mode, menuHorizontalPosition == 0 ? attr : 0);

        drawSwitch(MODEL_SETUP_2ND_COLUMN + 5 * FW, y, timer->swtch,
                   menuHorizontalPosition == 1 ? attr : 0);

        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, timer->mode, TMRMODE_MAX);
              break;
            case 1:
              CHECK_INCDEC_MODELSWITCH(event, timer->swtch, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, isSwitchAvailableInMixes);
              break;
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_TIMER1_START:
      case ITEM_MODEL_SETUP_TIMER2_START:
      case ITEM_MODEL_SETUP_TIMER3_START:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_START);

        TimerData *timer =
            &g_model.timers[k >= ITEM_MODEL_SETUP_TIMER3
                                ? 2
                                : (k >= ITEM_MODEL_SETUP_TIMER2 ? 1 : 0)];

        drawTimer(MODEL_SETUP_2ND_COLUMN, y, timer->start,
                  menuHorizontalPosition == 0 ? attr : 0,
                  menuHorizontalPosition == 1 ? attr : 0);

        if (timer->start) {
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN + 5 * FW, y, STR_TIMER_DIR,
                             timer->showElapsed,
                             menuHorizontalPosition == 2 ? attr : 0);
        }

        if (attr && s_editMode > 0) {
          div_t qr = div(timer->start, 60);
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, qr.quot, 539); // 8:59
              timer->start = qr.rem + qr.quot * 60;
              break;
            case 1:
              qr.rem -= checkIncDecModel(event, qr.rem + 2, 1, 62) - 2;
              timer->start -= qr.rem;
              if ((int16_t) timer->start < 0)
                timer->start = 0;
              if ((int16_t) timer->start > 5999)
                timer->start = 32399; // 8:59:59
              break;
            case 2:
              if (timer->start) {
                timer->showElapsed =
                    checkIncDecModel(event, timer->showElapsed, 0, 1);
              }
              break;
          }
        }
        break;
      }

      case ITEM_MODEL_SETUP_TIMER1_NAME:
      case ITEM_MODEL_SETUP_TIMER2_NAME:
      case ITEM_MODEL_SETUP_TIMER3_NAME:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_SETUP_TIMER3 ? 2 : (k>=ITEM_MODEL_SETUP_TIMER2 ? 1 : 0)];
        editSingleName(MODEL_SETUP_2ND_COLUMN, y, INDENT TR_NAME, timer->name,
                       sizeof(timer->name), event, attr, old_editMode);
        break;
      }

      case ITEM_MODEL_SETUP_TIMER1_MINUTE_BEEP:
      case ITEM_MODEL_SETUP_TIMER2_MINUTE_BEEP:
      case ITEM_MODEL_SETUP_TIMER3_MINUTE_BEEP:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_SETUP_TIMER3 ? 2 : (k>=ITEM_MODEL_SETUP_TIMER2 ? 1 : 0)];
        timer->minuteBeep = editCheckBox(timer->minuteBeep, MODEL_SETUP_2ND_COLUMN, y, INDENT TR_MINUTEBEEP, attr, event);
        break;
      }

      case ITEM_MODEL_SETUP_TIMER1_COUNTDOWN_BEEP:
      case ITEM_MODEL_SETUP_TIMER2_COUNTDOWN_BEEP:
      case ITEM_MODEL_SETUP_TIMER3_COUNTDOWN_BEEP:
      {
        editTimerCountdown(k>=ITEM_MODEL_SETUP_TIMER3 ? 2 : (k>=ITEM_MODEL_SETUP_TIMER2 ? 1 : 0), y, attr, event);
        break;
      }

      case ITEM_MODEL_SETUP_TIMER1_PERSISTENT:
      case ITEM_MODEL_SETUP_TIMER2_PERSISTENT:
      case ITEM_MODEL_SETUP_TIMER3_PERSISTENT:
      {
        TimerData * timer = &g_model.timers[k>=ITEM_MODEL_SETUP_TIMER3 ? 2 : (k>=ITEM_MODEL_SETUP_TIMER2 ? 1 : 0)];
        timer->persistent = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_PERSISTENT, STR_VPERSISTENT, timer->persistent, 0, 2, attr, event);
        break;
      }
#if defined(FUNCTION_SWITCHES)

      case ITEM_MODEL_SETUP_LABEL:
        lcdDrawTextAlignedLeft(y, "Function Switches");
        break;

      case ITEM_MODEL_SETUP_SW1:
      case ITEM_MODEL_SETUP_SW2:
      case ITEM_MODEL_SETUP_SW3:
      case ITEM_MODEL_SETUP_SW4:
      case ITEM_MODEL_SETUP_SW5:
      case ITEM_MODEL_SETUP_SW6:
      {
        int index = k - ITEM_MODEL_SETUP_SW1;
        int config = FSWITCH_CONFIG(index);
        lcdDrawTextAtIndex(INDENT_WIDTH, y, STR_VSRCRAW, MIXSRC_FIRST_SWITCH + NUM_REGULAR_SWITCHES - MIXSRC_Rud + index + 1, menuHorizontalPosition < 0 ? attr : 0);
        if (ZEXIST(g_model.switchNames[index]) || (attr && s_editMode > 0 && menuHorizontalPosition == 0))
          editName(35, y, g_model.switchNames[index], LEN_SWITCH_NAME, event, menuHorizontalPosition == 0 ? attr : 0, 0, old_editMode);
        else
          lcdDrawMMM(35, y, menuHorizontalPosition == 0 ? attr : 0);
        config = editChoice(30 + 5*FW, y, "", STR_SWTYPES, config, SWITCH_NONE, SWITCH_2POS, menuHorizontalPosition == 1 ? attr : 0, event);
        if (attr && checkIncDec_Ret) {
          swconfig_t mask = (swconfig_t)0x03 << (2*index);
          g_model.functionSwitchConfig = (g_model.functionSwitchConfig & ~mask) | ((swconfig_t(config) & 0x03) << (2*index));
        }

        config = FSWITCH_GROUP(index);
        config = editChoice(30 + 13 * FW, y, "", STR_FSGROUPS, config, 0, 3, menuHorizontalPosition == 2 ? attr : 0, event);
        if (attr && checkIncDec_Ret && menuHorizontalPosition == 2) {
          swconfig_t mask = (swconfig_t) 0x03 << (2 * index);
          g_model.functionSwitchGroup = (g_model.functionSwitchGroup & ~mask) | ((swconfig_t(config) & 0x03) << (2 * index));
        }

        if (FSWITCH_GROUP(index)) {
          uint8_t groupeAlwaysOn = IS_FSWITCH_GROUP_ON(config);
          groupeAlwaysOn = editCheckBox(groupeAlwaysOn, 30 + 15 * FW, y, "", menuHorizontalPosition == 3 ? attr : 0, event);
          if (attr && checkIncDec_Ret && menuHorizontalPosition == 3) {
            swconfig_t mask = (swconfig_t) 0x01 << (2 * NUM_FUNCTIONS_SWITCHES + config);
            g_model.functionSwitchGroup = (g_model.functionSwitchGroup & ~mask) | (groupeAlwaysOn << (2 * NUM_FUNCTIONS_SWITCHES + config));
          }
        }
        else if (attr && menuHorizontalPosition == 3) {  // Non visible checkbox
          REPEAT_LAST_CURSOR_MOVE();
        }
        break;
      }

      case ITEM_MODEL_SETUP_FS_STARTUP:
      {
        const char* s;
        lcdDrawText(0, y, INDENT "Start", menuHorizontalPosition < 0 ? attr : 0);
        for (uint8_t i = 0; i < NUM_FUNCTIONS_SWITCHES; i++) {
          uint8_t startPos = (g_model.functionSwitchStartConfig >> 2 * i) & 0x03;
          s = _fct_sw_start[(g_model.functionSwitchStartConfig >> 2 * i) & 0x03];
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN - (2 + FW) + i * 2 * FW, y, i + 1, 0);
          lcdDrawText(lcdNextPos, y, s, attr && (menuHorizontalPosition == i) ? (s_editMode ? INVERS + BLINK : INVERS) : 0);
          if (attr && menuHorizontalPosition == i) {
            CHECK_INCDEC_MODELVAR(event, startPos, 0, 2);
          }
          if (attr && checkIncDec_Ret) {
            swconfig_t mask = (swconfig_t)0x03 << (2*i);
            g_model.functionSwitchStartConfig = (g_model.functionSwitchStartConfig & ~mask) | ((swconfig_t(startPos) & 0x03) << (2*i));
            storageDirty(EE_MODEL);
          }
        }
        break;
      }
#endif
      case ITEM_MODEL_SETUP_EXTENDED_LIMITS:
        g_model.extendedLimits = editCheckBox(g_model.extendedLimits, MODEL_SETUP_2ND_COLUMN, y, STR_ELIMITS, attr, event);
        break;

      case ITEM_MODEL_SETUP_EXTENDED_TRIMS:
        g_model.extendedTrims = editCheckBox(g_model.extendedTrims, MODEL_SETUP_2ND_COLUMN, y, STR_ETRIMS, menuHorizontalPosition<=0 ? attr : 0, event==EVT_KEY_BREAK(KEY_ENTER) ? event : 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW, y, STR_RESET_BTN, (menuHorizontalPosition>0  && !NO_HIGHLIGHT()) ? attr : 0);
        if (attr && menuHorizontalPosition>0) {
          s_editMode = 0;
          if (event==EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
              memclear(&g_model.flightModeData[i], TRIMS_ARRAY_SIZE);
            }
            storageDirty(EE_MODEL);
            AUDIO_WARNING1();
          }
        }
        break;

      case ITEM_MODEL_SETUP_DISPLAY_TRIMS:
        g_model.displayTrims = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_DISPLAY_TRIMS, STR_VDISPLAYTRIMS, g_model.displayTrims, 0, 2, attr, event);
        break;

      case ITEM_MODEL_SETUP_TRIM_INC:
        g_model.trimInc = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_TRIMINC, STR_VTRIMINC, g_model.trimInc, -2, 2, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_REVERSED:
        g_model.throttleReversed = editCheckBox(g_model.throttleReversed, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLEREVERSE, attr, event );
        break;

      case ITEM_MODEL_SETUP_THROTTLE_TRACE:
      {
        lcdDrawTextAlignedLeft(y, STR_TTRACE);
        if (attr)
          CHECK_INCDEC_MODELVAR_ZERO_CHECK(
              event, g_model.thrTraceSrc,
              NUM_POTS + NUM_SLIDERS + MAX_OUTPUT_CHANNELS,
              isThrottleSourceAvailable);

        uint8_t idx = throttleSource2Source(g_model.thrTraceSrc);
        drawSource(MODEL_SETUP_2ND_COLUMN, y, idx, attr);
        break;
      }

      case ITEM_MODEL_SETUP_THROTTLE_TRIM:
        g_model.thrTrim = editCheckBox(g_model.thrTrim, MODEL_SETUP_2ND_COLUMN, y, STR_TTRIM, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_TRIM_SWITCH:
        lcdDrawTextAlignedLeft(y, STR_TTRIM_SW);
        if (attr)
          CHECK_INCDEC_MODELVAR_ZERO(event, g_model.thrTrimSw, NUM_TRIMS - 1);
        drawSource(MODEL_SETUP_2ND_COLUMN, y, g_model.getThrottleStickTrimSource(), attr);
        break;

      case ITEM_MODEL_SETUP_PREFLIGHT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_PREFLIGHT);
        break;

      case ITEM_MODEL_SETUP_CHECKLIST_DISPLAY:
        g_model.displayChecklist = editCheckBox(g_model.displayChecklist, MODEL_SETUP_2ND_COLUMN, y, STR_CHECKLIST, attr, event);
        break;

      case ITEM_MODEL_SETUP_THROTTLE_WARNING:
        g_model.disableThrottleWarning = !editCheckBox(!g_model.disableThrottleWarning, MODEL_SETUP_2ND_COLUMN, y, STR_THROTTLE_WARNING, attr, event);
        break;

      case ITEM_MODEL_SETUP_CUSTOM_THROTTLE_WARNING:
        g_model.enableCustomThrottleWarning = editCheckBox(g_model.enableCustomThrottleWarning, MODEL_SETUP_2ND_COLUMN, y, STR_CUSTOM_THROTTLE_WARNING, attr, event);
        break;

      case ITEM_MODEL_SETUP_CUSTOM_THROTTLE_WARNING_VALUE:
        {
          lcdDrawText(INDENT_WIDTH * 4, y, STR_CUSTOM_THROTTLE_WARNING_VAL);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.customThrottleWarningPosition, attr | LEFT, 2);
          if (attr) {
            CHECK_INCDEC_MODELVAR(event, g_model.customThrottleWarningPosition, -100, 100);
          }
        }
        break;

      case ITEM_MODEL_SETUP_SWITCHES_WARNING2:
        if (i==0) {
          if (CURSOR_MOVED_LEFT(event))
            menuVerticalOffset--;
          else
            menuVerticalOffset++;
        }
        break;

      case ITEM_MODEL_SETUP_SWITCHES_WARNING1:
        {
          #define FIRSTSW_STR   (&STR_VSRCRAW[MIXSRC_FIRST_SWITCH-MIXSRC_FIRST_STICK+1])
          uint8_t switchWarningsCount = getSwitchWarningsCount();
          //uint8_t length = STR_VSRCRAW[0];
          horzpos_t l_posHorz = menuHorizontalPosition;

          if (i>=NUM_BODY_LINES-2 && getSwitchWarningsCount() > MAX_SWITCH_PER_LINE*(NUM_BODY_LINES-i)) {
            if (CURSOR_MOVED_LEFT(event))
              menuVerticalOffset--;
            else
              menuVerticalOffset++;
            break;
          }

          swarnstate_t states = g_model.switchWarningState;
          char c;

          lcdDrawTextAlignedLeft(y, STR_SWITCHWARNING);
#if defined(PCBXLITE)
          lcdDrawText(LCD_W, y, "<]", RIGHT);
          if (attr) {
            if (menuHorizontalPosition > switchWarningsCount)
              menuHorizontalPosition = switchWarningsCount;
          }
          if (attr && menuHorizontalPosition == switchWarningsCount) {
#else
          if (attr) {
#endif
            s_editMode = 0;
            if (!READ_ONLY()) {
              switch (event) {
                case EVT_KEY_BREAK(KEY_ENTER):
                  break;

                case EVT_KEY_LONG(KEY_ENTER):
                  if (menuHorizontalPosition < 0 ||
                      menuHorizontalPosition >= switchWarningsCount) {
                    START_NO_HIGHLIGHT();
                    getMovedSwitch();
                    // Mask switches enabled for warnings
                    swarnstate_t sw_mask = 0;
                    for(uint8_t i=0; i<NUM_SWITCHES; i++) {
                      if (SWITCH_WARNING_ALLOWED(i))
                        if (g_model.switchWarningState & (0x07 << (3 * i)))
                          sw_mask |= (0x07 << (3 * i));
                    }
                    g_model.switchWarningState = switches_states & sw_mask;
                    AUDIO_WARNING1();
                    storageDirty(EE_MODEL);
                  }
                  killEvents(event);
                  break;
              }
            }
          }

          int current = 0;
          for (int i = 0; i < NUM_SWITCHES - NUM_FUNCTIONS_SWITCHES; i++) {
            if (SWITCH_WARNING_ALLOWED(i)) {
              div_t qr = div(current, MAX_SWITCH_PER_LINE);
              if (!READ_ONLY() && event == EVT_KEY_BREAK(KEY_ENTER) && attr &&
                  l_posHorz == current && old_posHorz >= 0) {
                uint8_t curr_state = (states & 0x07);
                // remove old setting
                g_model.switchWarningState &= ~(0x07 << (3 * i));
                // add the new one (if switch UP and 2POS, jump directly to DOWN)
                curr_state += (curr_state != 1 || IS_CONFIG_3POS(i) ? 1 : 2);
                g_model.switchWarningState |= (curr_state & 0x03) << (3 * i);
                storageDirty(EE_MODEL);
#if defined(PCBXLITE)
                s_editMode = 0;
#endif
              }
              lcdDrawSizedText(
                  MODEL_SETUP_2ND_COLUMN + qr.rem * ((2 * FW) + 1),
                  y + FH * qr.quot, FIRSTSW_STR[i] + sizeof(STR_CHAR_SWITCH), 1,
                  attr && (menuHorizontalPosition == current) ? INVERS : 0);
              lcdDrawText(lcdNextPos, y + FH * qr.quot,
                          getSwitchWarnSymbol(states & 0x03));
              ++current;
            }
            states >>= 3;
          }
          if (attr && ((menuHorizontalPosition < 0) ||
                       menuHorizontalPosition >= switchWarningsCount)) {
            lcdDrawFilledRect(MODEL_SETUP_2ND_COLUMN - 1, y - 1,
                              8 * (2 * FW + 1), 1 + FH * ((current + 4) / 5));
          }
        break;
      }

      case ITEM_MODEL_SETUP_POTS_WARNING:
        lcdDrawTextAlignedLeft(y, STR_POTWARNING);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, _pots_warn_modes, g_model.potsWarnMode, (menuHorizontalPosition == 0) ? attr : 0);
        if (attr && (menuHorizontalPosition == 0)) {
          CHECK_INCDEC_MODELVAR(event, g_model.potsWarnMode, POTS_WARN_OFF, POTS_WARN_AUTO);
          storageDirty(EE_MODEL);
        }

        if (attr) {
          if (menuHorizontalPosition > 0) s_editMode = 0;
          if (!READ_ONLY() && menuHorizontalPosition > 0) {
            switch (event) {
              case EVT_KEY_LONG(KEY_ENTER):
                killEvents(event);
                if (g_model.potsWarnMode == POTS_WARN_MANUAL) {
                  SAVE_POT_POSITION(menuHorizontalPosition-1);
                  AUDIO_WARNING1();
                  storageDirty(EE_MODEL);
                }
                break;
              case EVT_KEY_BREAK(KEY_ENTER):
                g_model.potsWarnEnabled ^= (1 << (menuHorizontalPosition-1));
                storageDirty(EE_MODEL);
                break;
            }
          }
        }
        if (g_model.potsWarnMode) {
          coord_t x = MODEL_SETUP_2ND_COLUMN+28;
          for (int i=0; i<NUM_POTS+NUM_SLIDERS; ++i) {
            if (i<NUM_XPOTS && !IS_POT_SLIDER_AVAILABLE(POT1+i)) {
              if (attr && (menuHorizontalPosition==i+1)) REPEAT_LAST_CURSOR_MOVE();
            }
            else {
              LcdFlags flags = ((menuHorizontalPosition==i+1) && attr) ? BLINK : 0;
              if ((!attr || menuHorizontalPosition >= 0) && (g_model.potsWarnEnabled & (1 << i))) {
                flags |= INVERS;
              }

              // skip "---" (+1) and source symbol (+2)
              const char* source = STR_VSRCRAW[NUM_STICKS + 1 + i] + 2;
              lcdDrawSizedText(x, y, source, UINT8_MAX, flags);
              x = lcdNextPos+3;
            }
          }
        }
        break;

      case ITEM_MODEL_SETUP_BEEP_CENTER:
        lcdDrawTextAlignedLeft(y, STR_BEEPCTR);
        for (uint8_t i = 0; i < NUM_STICKS+NUM_POTS+NUM_SLIDERS; i++) {
          coord_t x = MODEL_SETUP_2ND_COLUMN + i*FW;
          lcdDrawTextAtIndex(x, y, STR_RETA123, i, ((menuHorizontalPosition==i) && attr) ? BLINK | INVERS : (((g_model.beepANACenter & ((BeepANACenter)1<<i)) || (attr && CURSOR_ON_LINE())) ? INVERS : 0 ) );
        }
        if (attr) {
          if (event == EVT_KEY_BREAK(KEY_ENTER)) {
            if (READ_ONLY_UNLOCKED()) {
              s_editMode = 0;
              g_model.beepANACenter ^= ((BeepANACenter)1<<menuHorizontalPosition);
              storageDirty(EE_MODEL);
            }
          }
        }
        break;

      case ITEM_MODEL_SETUP_USE_GLOBAL_FUNCTIONS:
        lcdDrawTextAlignedLeft(y, STR_USE_GLOBAL_FUNCS);
        drawCheckBox(MODEL_SETUP_2ND_COLUMN, y, !g_model.noGlobalFunctions, attr);
        if (attr) g_model.noGlobalFunctions = !checkIncDecModel(event, !g_model.noGlobalFunctions, 0, 1);
        break;

      case ITEM_MODEL_SETUP_USE_JITTER_FILTER:
        g_model.jitterFilter = editChoice(MODEL_SETUP_2ND_COLUMN, y, STR_JITTER_FILTER, STR_ADCFILTERVALUES, g_model.jitterFilter, 0, 2, attr, event);
        break;


#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, STR_INTERNALRF);
        break;
#endif


#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_LABEL:
        lcdDrawTextAlignedLeft(y, STR_EXTERNALRF);
        break;
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_TYPE:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_TYPE:
#endif
        lcdDrawTextAlignedLeft(y, INDENT TR_MODE);
        lcdDrawTextAtIndex(
            MODEL_SETUP_2ND_COLUMN, y,
            moduleIdx == EXTERNAL_MODULE ? STR_EXTERNAL_MODULE_PROTOCOLS
                                         : STR_INTERNAL_MODULE_PROTOCOLS,
            moduleIdx == EXTERNAL_MODULE ? reusableBuffer.moduleSetup.newType
                                         : g_model.moduleData[moduleIdx].type,
            menuHorizontalPosition == 0 ? attr : 0);
        if (isModuleXJT(moduleIdx))
          lcdDrawTextAtIndex(lcdNextPos + 3, y, STR_XJT_ACCST_RF_PROTOCOLS,
                             g_model.moduleData[moduleIdx].subType,
                             menuHorizontalPosition == 1 ? attr : 0);
        else if (isModuleISRM(moduleIdx))
          lcdDrawTextAtIndex(lcdNextPos + 3, y, STR_ISRM_RF_PROTOCOLS,
                             g_model.moduleData[INTERNAL_MODULE].subType,
                             menuHorizontalPosition == 1 ? attr : 0);
        else if (isModuleDSM2(moduleIdx))
          lcdDrawTextAtIndex(lcdNextPos + 3, y, STR_DSM_PROTOCOLS,
                             g_model.moduleData[moduleIdx].subType,
                             menuHorizontalPosition == 1 ? attr : 0);
        else if (isModuleR9MNonAccess(moduleIdx))
          lcdDrawTextAtIndex(lcdNextPos + 3, y, STR_R9M_REGION,
                             g_model.moduleData[moduleIdx].subType,
                             (menuHorizontalPosition == 1 ? attr : 0));
        else if (moduleIdx == INTERNAL_MODULE &&
                 isModuleCrossfire(INTERNAL_MODULE))
          lcdDrawTextAtIndex(
              lcdNextPos + 3, y, STR_CRSF_BAUDRATE,
              CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate), 0);

        if (attr && menuHorizontalPosition == 0 &&
            moduleIdx == EXTERNAL_MODULE) {
          if (s_editMode > 0) {
            g_model.moduleData[moduleIdx].type = MODULE_TYPE_NONE;
          } else if (reusableBuffer.moduleSetup.newType !=
                     reusableBuffer.moduleSetup.previousType) {
            g_model.moduleData[moduleIdx].type =
                reusableBuffer.moduleSetup.newType;
            reusableBuffer.moduleSetup.previousType =
                reusableBuffer.moduleSetup.newType;
            setModuleType(moduleIdx, g_model.moduleData[moduleIdx].type);
            storageDirty(EE_MODEL);
          } else if (g_model.moduleData[moduleIdx].type == MODULE_TYPE_NONE) {
            g_model.moduleData[moduleIdx].type =
                reusableBuffer.moduleSetup.newType;
          }
        }
        if (attr) {
          if (s_editMode > 0) {
            switch (menuHorizontalPosition) {
              case 0:
#if defined(HARDWARE_INTERNAL_MODULE)
                if (moduleIdx == INTERNAL_MODULE) {
                  uint8_t moduleType =
                      checkIncDec(event, g_model.moduleData[moduleIdx].type,
                                  MODULE_TYPE_NONE, MODULE_TYPE_MAX, EE_MODEL,
                                  isInternalModuleAvailable);
                  if (checkIncDec_Ret) {
                    setModuleType(moduleIdx, moduleType);
                  }
                } else
#endif
                  reusableBuffer.moduleSetup.newType =
                      checkIncDec(event, reusableBuffer.moduleSetup.newType,
                                  MODULE_TYPE_NONE, MODULE_TYPE_MAX, 0,
                                  isExternalModuleAvailable);
                break;

              case 1:
                if (isModuleXJT(moduleIdx)) {
                  g_model.moduleData[moduleIdx].subType =
                      checkIncDec(event, g_model.moduleData[moduleIdx].subType,
                                  0, MODULE_SUBTYPE_PXX1_LAST, EE_MODEL,
                                  isRfProtocolAvailable);
                  if (checkIncDec_Ret) {
                    g_model.moduleData[moduleIdx].channelsStart = 0;
                    g_model.moduleData[moduleIdx].channelsCount =
                        defaultModuleChannels_M8(moduleIdx);
                  }
                } else if (isModuleISRM(moduleIdx)) {
                  g_model.moduleData[moduleIdx].subType =
                      checkIncDec(event, g_model.moduleData[moduleIdx].subType,
                                  0, MODULE_SUBTYPE_ISRM_PXX2_ACCST_D16,
                                  EE_MODEL, isRfProtocolAvailable);
                } else if (isModuleDSM2(moduleIdx)) {
                  CHECK_INCDEC_MODELVAR(event,
                                        g_model.moduleData[moduleIdx].subType,
                                        DSM2_PROTO_LP45, DSM2_PROTO_DSMX);
                } else if (isModuleR9MNonAccess(moduleIdx)) {
                  g_model.moduleData[moduleIdx].subType =
                      checkIncDec(event, g_model.moduleData[moduleIdx].subType,
                                  MODULE_SUBTYPE_R9M_FCC,
                                  MODULE_SUBTYPE_R9M_LAST, EE_MODEL);
                  if (checkIncDec_Ret) {
                    g_model.moduleData[moduleIdx].pxx.power = 0;
                    g_model.moduleData[moduleIdx].channelsStart = 0;
                    g_model.moduleData[moduleIdx].channelsCount =
                        defaultModuleChannels_M8(moduleIdx);
                  }
                } else {
                  CHECK_INCDEC_MODELVAR(event,
                                        g_model.moduleData[moduleIdx].subType,
                                        0, MODULE_SUBTYPE_PXX1_LAST);
                }

                if (checkIncDec_Ret) {
                  g_model.moduleData[moduleIdx].channelsStart = 0;
                  g_model.moduleData[moduleIdx].channelsCount =
                      defaultModuleChannels_M8(moduleIdx);
                }
            }
          }
#if POPUP_LEVEL > 1
          else if (old_editMode > 0) {
            if (isModuleR9MNonAccess(moduleIdx)) {
              if (g_model.moduleData[moduleIdx].subType >
                  MODULE_SUBTYPE_R9M_EU) {
                POPUP_WARNING(STR_MODULE_PROTOCOL_FLEX_WARN_LINE1);
                SET_WARNING_INFO(STR_MODULE_PROTOCOL_WARN_LINE2,
                                 sizeof(TR_MODULE_PROTOCOL_WARN_LINE2) - 1, 0);
              }
#if POPUP_LEVEL >= 3
              else if (g_model.moduleData[moduleIdx].subType ==
                       MODULE_SUBTYPE_R9M_EU) {
                POPUP_WARNING(STR_MODULE_PROTOCOL_EU_WARN_LINE1);
                SET_WARNING_INFO(STR_MODULE_PROTOCOL_WARN_LINE2,
                                 sizeof(TR_MODULE_PROTOCOL_WARN_LINE2) - 1, 0);
              } else {
                POPUP_WARNING(STR_MODULE_PROTOCOL_FCC_WARN_LINE1);
                SET_WARNING_INFO(STR_MODULE_PROTOCOL_WARN_LINE2,
                                 sizeof(TR_MODULE_PROTOCOL_WARN_LINE2) - 1, 0);
              }
#endif
            }
          }
#endif
        }
        break;

#if defined(HARDWARE_EXTERNAL_MODULE) && (defined(CROSSFIRE) || defined(GHOST))
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_BAUDRATE: {
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawText(INDENT_WIDTH, y, STR_BAUDRATE);
        if (isModuleCrossfire(EXTERNAL_MODULE)) {
          lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_CRSF_BAUDRATE, CROSSFIRE_STORE_TO_INDEX(moduleData.crsf.telemetryBaudrate),attr | LEFT);
          if (attr) {
            moduleData.crsf.telemetryBaudrate =CROSSFIRE_INDEX_TO_STORE(checkIncDecModel(event,CROSSFIRE_STORE_TO_INDEX(moduleData.crsf.telemetryBaudrate),0, DIM(CROSSFIRE_BAUDRATES) - 1));
            if (checkIncDec_Ret) {
              restartModule(moduleIdx);
            }
          }
        }
#if SPORT_MAX_BAUDRATE < 400000
        else {
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, CROSSFIRE_BAUDRATES[CROSSFIRE_STORE_TO_INDEX(g_model.moduleData[EXTERNAL_MODULE].ghost.telemetryBaudrate)],attr | LEFT);
          if (attr) {
            moduleData.ghost.telemetryBaudrate = CROSSFIRE_INDEX_TO_STORE(checkIncDecModel(event,CROSSFIRE_STORE_TO_INDEX(moduleData.ghost.telemetryBaudrate),0, 1));
            if (checkIncDec_Ret) {
              restartModule(moduleIdx);
            }
          }
        }
#endif
        break;
      }
#endif
#if (defined(CROSSFIRE) || defined(GHOST))
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_SERIALSTATUS:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_SERIALSTATUS:
#endif
        lcdDrawText(INDENT_WIDTH, y, STR_STATUS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, 1000000 / getMixerSchedulerPeriod(), LEFT | attr);
        lcdDrawText(lcdNextPos, y, "Hz ", attr);
        lcdDrawNumber(lcdNextPos, y, telemetryErrors, attr);
        lcdDrawText(lcdNextPos + 1, y, "Err", attr);
        if (attr) {
          s_editMode = 0;
          if (event == EVT_KEY_LONG(KEY_ENTER)) {
            START_NO_HIGHLIGHT();
            telemetryErrors = 0;
            AUDIO_WARNING1();
            killEvents(event);
          }
        }
        break;
#endif

#if defined(MULTIMODULE)
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PROTOCOL:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PROTOCOL:
#endif
      {
        lcdDrawTextAlignedLeft(y, TR_TYPE);
        int multi_rfProto = g_model.moduleData[moduleIdx].multi.rfProtocol;
        lcdDrawMultiProtocolString(MODEL_SETUP_2ND_COLUMN, y, moduleIdx, multi_rfProto, attr);
        if (attr) {
          int multiRfProto = g_model.moduleData[moduleIdx].multi.rfProtocol;
          MultiModuleStatus &status = getMultiModuleStatus(moduleIdx);
          if (status.isValid()) {
            int8_t direction = checkIncDec(event, 0, -1, 1);
            if (direction == -1) {
              if (multiRfProto == MODULE_SUBTYPE_MULTI_FRSKY)
                multiRfProto = MODULE_SUBTYPE_MULTI_FRSKYX_RX;
              else
                multiRfProto = convertMultiToOtx(status.protocolPrev);
            }
            if (direction == 1) {
              if (multiRfProto == MODULE_SUBTYPE_MULTI_FRSKY)
                multiRfProto = MODULE_SUBTYPE_MULTI_FRSKYX2;
              else
                multiRfProto = convertMultiToOtx(status.protocolNext);
            }
          }
          else {
            CHECK_INCDEC_MODELVAR_CHECK(event, multiRfProto, MODULE_SUBTYPE_MULTI_FIRST, MULTI_MAX_PROTOCOLS, isMultiProtocolSelectable);
          }
          if (checkIncDec_Ret) {
            g_model.moduleData[moduleIdx].multi.rfProtocol = multiRfProto;
            g_model.moduleData[moduleIdx].subType = 0;
            resetMultiProtocolsOptions(moduleIdx);
            storageDirty(EE_MODEL);
          }
        }
      }
      break;

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_SUBTYPE:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_SUBTYPE:
#endif
      {
        lcdDrawTextAlignedLeft(y, STR_SUBTYPE);
        lcdDrawMultiSubProtocolString(MODEL_SETUP_2ND_COLUMN, y, moduleIdx, g_model.moduleData[moduleIdx].subType, attr);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:{
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].subType, 0, getMaxMultiSubtype(moduleIdx));
              if (checkIncDec_Ret) {
                resetMultiProtocolsOptions(moduleIdx);
              }
              break;
            }
          }
        }
      }
      break;
#endif

#if defined(AFHDS3) && defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS_PROTOCOL:
        lcdDrawText(INDENT_WIDTH, y, STR_PROTOCOL);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_AFHDS3_PROTOCOLS, g_model.moduleData[EXTERNAL_MODULE].subType, attr);
        if (attr && s_editMode > 0 && menuHorizontalPosition == 0) {
          CHECK_INCDEC_MODELVAR(event, g_model.moduleData[moduleIdx].subType, AFHDS_SUBTYPE_FIRST, AFHDS_SUBTYPE_LAST);
        }
        break;
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_MODE:
        lcdDrawText(INDENT_WIDTH, y, TR_TELEMETRY_TYPE);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y,
            g_model.moduleData[EXTERNAL_MODULE].afhds3.telemetry ? STR_AFHDS3_ONE_TO_ONE_TELEMETRY : TR_AFHDS3_ONE_TO_MANY);
        break;
#endif

#if defined(PCBTARANIS)
      case ITEM_MODEL_SETUP_TRAINER_LABEL:
        lcdDrawTextAlignedLeft(y, STR_TRAINER);
        break;

      case ITEM_MODEL_SETUP_TRAINER_MODE:
        lcdDrawTextAlignedLeft(y, INDENT TR_MODE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VTRAINERMODES, g_model.trainerData.mode, attr);
        if (attr) {
          g_model.trainerData.mode = checkIncDec(event, g_model.trainerData.mode, TRAINER_MODE_MIN(), TRAINER_MODE_MAX(), EE_MODEL, isTrainerModeAvailable);
#if defined(BLUETOOTH)
          if (checkIncDec_Ret) {
            bluetooth.state = BLUETOOTH_STATE_OFF;
            bluetooth.distantAddr[0] = '\0';
          }
#endif
        }
        break;
#endif

#if defined(PCBTARANIS) && defined(BLUETOOTH)
      case ITEM_MODEL_SETUP_TRAINER_BLUETOOTH:
        if (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH) {
          if (attr) {
            s_editMode = 0;
          }
          if (bluetooth.distantAddr[0]) {
            lcdDrawText(INDENT_WIDTH, y+1, bluetooth.distantAddr, TINSIZE);
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, BUTTON(TR_CLEAR), attr);
            if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
              bluetooth.state = BLUETOOTH_STATE_CLEAR_REQUESTED;
              memclear(bluetooth.distantAddr, sizeof(bluetooth.distantAddr));
            }
          }
          else {
            lcdDrawText(INDENT_WIDTH, y, "---");
            if (bluetooth.state < BLUETOOTH_STATE_IDLE)
              lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_BUTTON_INIT, attr);
            else
              lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, BUTTON(TR_DISCOVER), attr);
            if (attr && event == EVT_KEY_BREAK(KEY_ENTER)) {
              if (bluetooth.state < BLUETOOTH_STATE_IDLE) {
                bluetooth.state = BLUETOOTH_STATE_OFF;
              }
              else {
                reusableBuffer.moduleSetup.bt.devicesCount = 0;
                bluetooth.state = BLUETOOTH_STATE_DISCOVER_REQUESTED;
              }
            }

            if (bluetooth.state == BLUETOOTH_STATE_DISCOVER_START && reusableBuffer.moduleSetup.bt.devicesCount > 0) {
              popupMenuItemsCount = min<uint8_t>(reusableBuffer.moduleSetup.bt.devicesCount, MAX_BLUETOOTH_DISTANT_ADDR);
              for (uint8_t i=0; i<popupMenuItemsCount; i++) {
                popupMenuItems[i] = reusableBuffer.moduleSetup.bt.devices[i];
              }
              POPUP_MENU_TITLE(STR_BT_SELECT_DEVICE);
              POPUP_MENU_START(onBluetoothConnectMenu);
            }
          }
        }
        else {
          if (bluetooth.distantAddr[0])
            lcdDrawText(INDENT_WIDTH, y+1, bluetooth.distantAddr, TINSIZE);
          else
            lcdDrawText(INDENT_WIDTH, y, "---");
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, bluetooth.state == BLUETOOTH_STATE_CONNECTED ? STR_CONNECTED : STR_NOT_CONNECTED);
        }
        break;
#endif

#if defined(PCBTARANIS)
      case ITEM_MODEL_SETUP_TRAINER_CHANNELS:
        lcdDrawTextAlignedLeft(y, STR_CHANNELRANGE);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_CH, menuHorizontalPosition==0 ? attr : 0);
        lcdDrawNumber(lcdLastRightPos, y, g_model.trainerData.channelsStart+1, LEFT | (menuHorizontalPosition==0 ? attr : 0));
        lcdDrawChar(lcdLastRightPos, y, '-');
        lcdDrawNumber(lcdLastRightPos + FW+1, y, g_model.trainerData.channelsStart + 8 + g_model.trainerData.channelsCount, LEFT | (menuHorizontalPosition==1 ? attr : 0));
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.trainerData.channelsStart, 32-8-g_model.trainerData.channelsCount);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.channelsCount, -4, min<int8_t>(MAX_TRAINER_CHANNELS_M8, 32-8-g_model.trainerData.channelsStart));
              break;
          }
        }
        break;
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_CHANNELS:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_CHANNELS:
#endif
      {
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawTextAlignedLeft(y, STR_CHANNELRANGE);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_CH, menuHorizontalPosition==0 ? attr : 0);
        lcdDrawNumber(lcdLastRightPos, y, moduleData.channelsStart+1, LEFT | (menuHorizontalPosition==0 ? attr : 0));
        lcdDrawChar(lcdLastRightPos, y, '-');
        lcdDrawNumber(lcdLastRightPos + FW+1, y, moduleData.channelsStart+sentModuleChannels(moduleIdx), LEFT | (menuHorizontalPosition==1 ? attr : 0));
        const char * delay = getModuleDelay(moduleIdx);
        if (delay)
          lcdDrawText(lcdLastRightPos+4, y, delay, SMLSIZE);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.channelsStart, 32-8-moduleData.channelsCount);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR_CHECK(event, moduleData.channelsCount, -4, min<int8_t>(maxModuleChannels_M8(moduleIdx), 32-8-moduleData.channelsStart), moduleData.type == MODULE_TYPE_ISRM_PXX2 ? isPxx2IsrmChannelsCountAllowed : nullptr);
              if (checkIncDec_Ret && moduleData.type == MODULE_TYPE_PPM) {
                setDefaultPpmFrameLength(moduleIdx);
              }
              break;
          }
        }
        break;
      }

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_RACING_MODE:
      {
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        lcdDrawText(INDENT_WIDTH, y, STR_RACING_MODE);
        moduleData.pxx2.racingMode = editCheckBox(moduleData.pxx2.racingMode, MODEL_SETUP_2ND_COLUMN, y, "", attr, event);
        break;
      }
#endif

#if defined(PCBX7) || defined(PCBX9LITE) || defined(PCBXLITE)
      case ITEM_MODEL_SETUP_TRAINER_PPM_PARAMS:
        lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.trainerData.frameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (g_model.trainerData.delay*50)+300, RIGHT | ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0));
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, g_model.trainerData.pulsePol ? '+' : '-', (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.frameLength, -20, 35);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.trainerData.delay, -4, 10);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.trainerData.pulsePol, 1);
              break;
          }
        }
        break;
#endif

#if defined(PXX2)
      case ITEM_MODEL_SETUP_REGISTRATION_ID:
        lcdDrawTextAlignedLeft(y, STR_REG_ID);
        if (isDefaultModelRegistrationID())
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_PXX2_DEFAULT);
        else
          lcdDrawSizedText(MODEL_SETUP_2ND_COLUMN, y, g_model.modelRegistrationID, PXX2_LEN_REGISTRATION_ID);
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM:
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_MODEL_NUM:
#endif
      {
        lcdDrawText(INDENT_WIDTH, y, STR_RECEIVER_NUM);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], attr | LEADING0 | LEFT, 2);
        if (attr) {
          CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], getMaxRxNum(moduleIdx));
          if (checkIncDec_Ret) {
            modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx];
          }
        }
      }
      break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_REGISTER_RANGE:
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_REGISTER_RANGE:
#endif
      {
        lcdDrawTextAlignedLeft(y, INDENT TR_MODULE);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, BUTTON(TR_REGISTER), (menuHorizontalPosition == 0 ? attr : 0));
        lcdDrawText(lcdLastRightPos + 3, y, STR_MODULE_RANGE, (menuHorizontalPosition == 1 ? attr : 0));
        if (attr) {
          if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL && s_editMode > 0) {
            if (menuHorizontalPosition == 0 && event == EVT_BUTTON_PRESSED()) {
              startRegisterDialog(moduleIdx);
            }
            else if (menuHorizontalPosition == 1) {
              moduleState[moduleIdx].mode = MODULE_MODE_RANGECHECK;
            }
          }
          if (s_editMode == 0 && !warningText) {
            moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
          }
          if (moduleState[moduleIdx].mode == MODULE_MODE_NORMAL) {
            // REGISTER finished
            s_editMode = 0;
          }
        }
      }
      break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_OPTIONS:
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_OPTIONS:
#endif
        lcdDrawText(INDENT_WIDTH, y, STR_OPTIONS);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_SET, attr);
        if (event == EVT_KEY_BREAK(KEY_ENTER) && attr) {
          g_moduleIdx = CURRENT_MODULE_EDITED(k);
          pushMenu(menuModelModuleOptions);
        }
        break;

      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_1:
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_2:
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_RECEIVER_3:
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_1:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_2:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_RECEIVER_3:
#endif
        modelSetupModulePxx2ReceiverLine(CURRENT_MODULE_EDITED(k), CURRENT_RECEIVER_EDITED(k), y, event, attr);
        break;
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE:
#endif
      {
        ModuleData & moduleData = g_model.moduleData[moduleIdx];
        if (isModulePPM(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.ppm.frameLength*PPM_STEP_SIZE + PPM_DEF_PERIOD, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (moduleData.ppm.delay*50)+300, RIGHT | ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0));
          lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, moduleData.ppm.pulsePol ? '+' : '-', (CURSOR_ON_LINE() || menuHorizontalPosition==2) ? attr : 0);
          if (attr && s_editMode > 0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.frameLength, -20, 35);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR(event, moduleData.ppm.delay, -4, 10);
                break;
              case 2:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.ppm.pulsePol, 1);
                break;
            }
          }
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_REFRESHRATE);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)moduleData.sbus.refreshRate * SBUS_STEPSIZE + SBUS_DEF_PERIOD, (menuHorizontalPosition<=0 ? attr : 0) | PREC1|LEFT);
          lcdDrawText(lcdLastRightPos, y, STR_MS);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN+5*FW+2, y, moduleData.sbus.noninverted ? STR_NOT_INVERTED : STR_NORMAL, (CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0);
          if (attr && s_editMode > 0) {
            switch (menuHorizontalPosition) {
              case 0:
                CHECK_INCDEC_MODELVAR(event, moduleData.sbus.refreshRate, (SBUS_MIN_PERIOD-SBUS_DEF_PERIOD)/SBUS_STEPSIZE, (SBUS_MAX_PERIOD-SBUS_DEF_PERIOD)/SBUS_STEPSIZE);
                break;
              case 1:
                CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.sbus.noninverted, 1);
                break;
            }
          }
        }
        else {
          lcdDrawText(INDENT_WIDTH, y, STR_RECEIVER);
          horzpos_t l_posHorz = menuHorizontalPosition;
          coord_t bindButtonPos = MODEL_SETUP_2ND_COLUMN;
          if (isModuleRxNumAvailable(moduleIdx)) {
            lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.header.modelId[moduleIdx], (l_posHorz==0 ? attr : 0) | LEADING0|LEFT, 2);
            bindButtonPos = lcdNextPos + 2;
            if (attr && l_posHorz == 0) {
              if (s_editMode > 0) {
                CHECK_INCDEC_MODELVAR_ZERO(event, g_model.header.modelId[moduleIdx], getMaxRxNum(moduleIdx));
                if (checkIncDec_Ret) {
                  if (isModuleCrossfire(moduleIdx))
                    moduleState[moduleIdx].counter = CRSF_FRAME_MODELID;
                  modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx];
                }
                else if (event == EVT_KEY_LONG(KEY_ENTER)) {
                  killEvents(event);
                  uint8_t newVal = 0;
#if defined(STORAGE_MODELSLIST)
                  newVal = modelslist.findNextUnusedModelId(moduleIdx);
#else
                  newVal = findNextUnusedModelId(g_eeGeneral.currModel, moduleIdx);
#endif
                  if (newVal != g_model.header.modelId[moduleIdx]) {
                    modelHeaders[g_eeGeneral.currModel].modelId[moduleIdx] = g_model.header.modelId[moduleIdx] = newVal;
                    storageDirty(EE_MODEL);
                  }
                }
              }
            }
          }
          else if (attr) {
            l_posHorz += 1;
          }
          if (isModuleBindRangeAvailable(moduleIdx)) {
            lcdDrawText(bindButtonPos, y, STR_MODULE_BIND, l_posHorz == 1 ? attr : 0);
            if (isModuleRangeAvailable(moduleIdx)) {
              lcdDrawText(lcdNextPos + 2, y, STR_MODULE_RANGE, l_posHorz == 2 ? attr : 0);
            }
            uint8_t newFlag = 0;
            uint8_t oldFlag = moduleState[moduleIdx].mode;
#if defined(MULTIMODULE)
            if (isModuleMultimodule(moduleIdx) &&
                getMultiBindStatus(moduleIdx) == MULTI_BIND_FINISHED) {
              setMultiBindStatus(moduleIdx, MULTI_BIND_NONE);
              s_editMode = 0;
            }
#endif
#if defined(PCBTARANIS)
            if (attr && l_posHorz > 0) {
              if (s_editMode > 0) {
                if (l_posHorz == 1) {
                  if (isModuleR9MNonAccess(moduleIdx) || isModuleD16(moduleIdx) || isModuleAFHDS3(moduleIdx)) {
#if defined(PCBXLITE)
                    if (EVT_KEY_MASK(event) == KEY_ENTER) {
#elif defined(NAVIGATION_9X)
                    if (event ==  EVT_KEY_FIRST(KEY_ENTER)) {
#else
                    if (event == EVT_KEY_BREAK(KEY_ENTER)) {
#endif
                      killEvents(event);
#if defined(AFHDS3)
                      if (isModuleAFHDS3(moduleIdx)) {
                        startBindMenuAfhds3(moduleIdx);
                        continue;
                      }
#endif
                      startBindMenu(moduleIdx);
                      continue;
                    }
                    if (oldFlag == MODULE_MODE_BIND) {
                      newFlag = MODULE_MODE_BIND;
                    }
                    else {
                      if (!popupMenuItemsCount) {
                        s_editMode = 0;  // this is when popup is exited before a choice is made
                      }
                    }
                  }
                  else {
                    newFlag = MODULE_MODE_BIND;
                  }

                  if (!event && (oldFlag != newFlag) &&
                      (oldFlag == MODULE_MODE_NORMAL)) {
                    // Bind mode switched off from somewhere else
                    s_editMode = 0;
                  }
                }
                else if (l_posHorz == 2) {
                  newFlag = MODULE_MODE_RANGECHECK;
                }
              }
            }
#else
            if (attr && l_posHorz > 0 && s_editMode > 0) {
              if (l_posHorz == 1)
                newFlag = MODULE_MODE_BIND;
              else if (l_posHorz == 2)
                newFlag = MODULE_MODE_RANGECHECK;
            }
#endif
            moduleState[moduleIdx].mode = newFlag;

#if defined(MULTIMODULE)
            if (isModuleMultimodule(moduleIdx) &&
                (newFlag == MODULE_MODE_BIND)) {
              setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
            }
#endif
            if (isModuleDSMP(moduleIdx) &&
                (oldFlag != newFlag) &&
                (oldFlag == MODULE_MODE_BIND)) {
              // Restart DSMP module when exiting bind mode
              restartModule(moduleIdx);
            }
          }
        }
        break;
      }


#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_FAILSAFE:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_FAILSAFE:
#endif
      {
        ModuleData &moduleData = g_model.moduleData[moduleIdx];
        lcdDrawTextAlignedLeft(y, STR_FAILSAFE);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VFAILSAFE, moduleData.failsafeMode, menuHorizontalPosition == 0 ? attr : 0);
        if (moduleData.failsafeMode == FAILSAFE_CUSTOM)
          lcdDrawText(MODEL_SETUP_2ND_COLUMN + MODEL_SETUP_SET_FAILSAFE_OFS, y, STR_SET, menuHorizontalPosition == 1 ? attr : 0);
        if (attr) {
          if (moduleData.failsafeMode != FAILSAFE_CUSTOM)
            menuHorizontalPosition = 0;
          if (menuHorizontalPosition == 0) {
            if (s_editMode > 0) {
              CHECK_INCDEC_MODELVAR_ZERO(event, moduleData.failsafeMode, isModuleR9M(moduleIdx) ? FAILSAFE_NOPULSES : FAILSAFE_LAST);
              if (checkIncDec_Ret)
                SEND_FAILSAFE_NOW(moduleIdx);
            }
          }
          else if (menuHorizontalPosition == 1) {
            s_editMode = 0;
            if (moduleData.failsafeMode == FAILSAFE_CUSTOM) {
              if (event == EVT_KEY_LONG(KEY_ENTER)) {
                killEvents(event);
                setCustomFailsafe(moduleIdx);
                AUDIO_WARNING1();
                SEND_FAILSAFE_NOW(moduleIdx);
              }
              else if (event == EVT_KEY_BREAK(KEY_ENTER) && attr) {
                g_moduleIdx = moduleIdx;
                pushMenu(menuModelFailsafe);
              }
            }
          }
          else {
            lcdDrawSolidFilledRect(MODEL_SETUP_2ND_COLUMN, y, LCD_W - MODEL_SETUP_2ND_COLUMN, 8);
          }
        }
      }
      break;

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_ANTENNA:
        reusableBuffer.moduleSetup.antennaMode = editChoice(MODEL_SETUP_2ND_COLUMN, y, INDENT TR_ANTENNA, STR_ANTENNA_MODES,
                                                            reusableBuffer.moduleSetup.antennaMode == ANTENNA_MODE_PER_MODEL ? ANTENNA_MODE_INTERNAL : reusableBuffer.moduleSetup.antennaMode,
                                                            ANTENNA_MODE_INTERNAL, ANTENNA_MODE_EXTERNAL, attr, event,
                                                            [](int value) { return value != ANTENNA_MODE_PER_MODEL; });
        if (event && !s_editMode && reusableBuffer.moduleSetup.antennaMode != g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode) {
          if (reusableBuffer.moduleSetup.antennaMode == ANTENNA_MODE_EXTERNAL && !isExternalAntennaEnabled()) {
            POPUP_CONFIRMATION(STR_ANTENNACONFIRM1, onModelAntennaSwitchConfirm);
            SET_WARNING_INFO(STR_ANTENNACONFIRM2, sizeof(TR_ANTENNACONFIRM2), 0);
          }
          else {
            g_model.moduleData[INTERNAL_MODULE].pxx.antennaMode = reusableBuffer.moduleSetup.antennaMode;
            checkExternalAntenna();
          }
        }
        break;
#endif
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_OPTIONS:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_OPTIONS:
#endif
      {
#if defined(MULTIMODULE)
        if (MULTIMODULE_PROTOCOL_KNOWN(moduleIdx)) {
          const char * title = getMultiOptionTitle(moduleIdx);

          if (!title) break;
          lcdDrawText(INDENT_WIDTH, y, title);
          if (title == STR_MULTI_RFTUNE) {
            lcdDrawText(MODEL_SETUP_2ND_COLUMN + 23, y, "RSSI(", LEFT);
            lcdDrawNumber(lcdLastRightPos, y, TELEMETRY_RSSI(), LEFT);
            lcdDrawText(lcdLastRightPos, y, ")", LEFT);
          }

          int optionValue = g_model.moduleData[moduleIdx].multi.optionValue;
          const uint8_t multi_proto = g_model.moduleData[moduleIdx].multi.rfProtocol;
          int8_t min, max;
          getMultiOptionValues(multi_proto, min, max);

          if (title == STR_MULTI_RFPOWER) {
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_POWER, optionValue, LEFT | attr);
            min = 0;
            max = 15;
          }
          else if (title == STR_MULTI_TELEMETRY) {
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_TELEMETRY_MODE, optionValue, LEFT | attr);
          }
          else if (multi_proto == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
            lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, 50 + 5 * optionValue, LEFT | attr);
          }
          else if (title == STR_MULTI_WBUS) {
            lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_MULTI_WBUS_MODE, optionValue, LEFT | attr);
            min = 0;
            max = 1;
          }
          else if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {
            optionValue = optionValue & 0x01;
            editCheckBox(optionValue, MODEL_SETUP_2ND_COLUMN, y, "", LEFT | attr, event);
          }
          else {
            if (min == 0 && max == 1)
              editCheckBox(optionValue, MODEL_SETUP_2ND_COLUMN, y, "", LEFT | attr, event);
            else
              lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, optionValue, LEFT | attr);
          }
          if (attr) {
            CHECK_INCDEC_MODELVAR(event, optionValue, min, max);
            if (checkIncDec_Ret) {
              if (multi_proto == MODULE_SUBTYPE_MULTI_DSM2) {
                g_model.moduleData[moduleIdx].multi.optionValue = (g_model.moduleData[moduleIdx].multi.optionValue & 0xFE) + optionValue;
              }
              else {
                g_model.moduleData[moduleIdx].multi.optionValue = optionValue;
              }
            }
          }
        }
#endif
        if (isModuleR9MNonAccess(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_MODULE_TELEMETRY);
          if (isSportLineUsedByInternalModule())
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_DISABLE_INTERNAL);
          else
            lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, STR_MODULE_TELEM_ON);
        }
        else if (isModuleSBUS(moduleIdx)) {
          lcdDrawTextAlignedLeft(y, STR_WARN_BATTVOLTAGE);
          putsVolts(lcdLastRightPos, y, getBatteryVoltage(), attr | PREC2 | LEFT);
        }
        else if (isModuleGhost(moduleIdx)) {
          auto & module = g_model.moduleData[moduleIdx];
          module.ghost.raw12bits = editCheckBox(module.ghost.raw12bits , MODEL_SETUP_2ND_COLUMN, y, INDENT "Raw 12 bits", attr, event);
        }
        break;
      }

#if defined(INTERNAL_MODULE_MULTI)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_POWER:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_POWER:
#endif
      {
        auto & module = g_model.moduleData[moduleIdx];
        // Lite FCC / Lite FLEX / Lite Pro Flex
        if (isModuleTypeR9MNonAccess(module.type)) {
          lcdDrawText(INDENT_WIDTH, y, STR_RF_POWER);
          if (isModuleR9M_FCC_VARIANT(moduleIdx)) {
            // FCC and FLEX modes ...
            if (isModuleTypeR9MLiteNonPro(module.type)) { // R9M lite FCC has only one power value, so displayed for info only
              lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_LITE_FCC_POWER_VALUES, 0, LEFT);
              if (attr) {
                REPEAT_LAST_CURSOR_MOVE();
              }
            }
            else {
              module.pxx.power = min<uint8_t>(module.pxx.power, R9M_FCC_POWER_MAX); // Sanitize
              lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_FCC_POWER_VALUES, module.pxx.power, LEFT | attr);
              if (attr) {
                CHECK_INCDEC_MODELVAR_ZERO(event, module.pxx.power, R9M_FCC_POWER_MAX);
                if (s_editMode == 0 && reusableBuffer.moduleSetup.r9mPower != module.pxx.power && module.channelsCount > maxModuleChannels_M8(moduleIdx)) {
                  module.channelsStart = 0;
                  module.channelsCount = maxModuleChannels_M8(moduleIdx);
                }
              }
            }
          }
          else {
            // EU-LBT mode ...
            if (isModuleTypeR9MLiteNonPro(module.type)) {
              // R9M Lite in EU-LBT mode ...
              module.pxx.power = min<uint8_t>(module.pxx.power, R9M_LITE_LBT_POWER_MAX); // Sanitize
              lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_LITE_LBT_POWER_VALUES, module.pxx.power, LEFT | attr);
              if (attr) {
                CHECK_INCDEC_MODELVAR_ZERO(event, module.pxx.power, R9M_LITE_LBT_POWER_MAX);
                if (s_editMode == 0 && reusableBuffer.moduleSetup.r9mPower != module.pxx.power) {
                  module.channelsStart = 0;
                  if (module.channelsCount > maxModuleChannels_M8(moduleIdx))
                    module.channelsCount = maxModuleChannels_M8(moduleIdx);
                  if (reusableBuffer.moduleSetup.r9mPower + module.pxx.power < 5) { // switching between mode 2 and 3 does not require rebind
                    POPUP_WARNING(STR_REBIND);
                  }
                  reusableBuffer.moduleSetup.r9mPower = module.pxx.power;
                }
              }
            }
            else {
              // R9M (full size) or R9M Lite Pro in EU-LBT mode ...
              module.pxx.power = min((uint8_t) module.pxx.power, (uint8_t) R9M_LBT_POWER_MAX); // Sanitize
              lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_R9M_LBT_POWER_VALUES, module.pxx.power, LEFT | attr);
              if (attr) {
                CHECK_INCDEC_MODELVAR_ZERO(event, module.pxx.power, R9M_LBT_POWER_MAX);
                if (s_editMode == 0 && reusableBuffer.moduleSetup.r9mPower != module.pxx.power) {
                  module.channelsStart = 0;
                  if (module.channelsCount > maxModuleChannels_M8(moduleIdx))
                    module.channelsCount = maxModuleChannels_M8(moduleIdx);
                  if (reusableBuffer.moduleSetup.r9mPower + module.pxx.power < 5) { //switching between mode 2 and 3 does not require rebind
                    POPUP_WARNING(STR_REBIND);
                  }
                  reusableBuffer.moduleSetup.r9mPower = module.pxx.power;
                }
              }
            }
          }
        }
#if defined(MULTIMODULE)
        else if (isModuleMultimodule(moduleIdx)) {
          module.multi.lowPowerMode = editCheckBox(module.multi.lowPowerMode, MODEL_SETUP_2ND_COLUMN, y, IS_RX_MULTI(moduleIdx) ? STR_MULTI_LNA_DISABLE : STR_MULTI_LOWPOWER, attr, event);
        }
#endif
#if defined(AFHDS3)
      else if (isModuleAFHDS3(EXTERNAL_MODULE)) {
        lcdDrawText(INDENT_WIDTH, y, STR_RF_POWER);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_AFHDS3_POWERS, g_model.moduleData[EXTERNAL_MODULE].afhds3.runPower, LEFT | attr);
        if (attr)
          CHECK_INCDEC_MODELVAR(event, g_model.moduleData[EXTERNAL_MODULE].afhds3.runPower, afhds3::RUN_POWER::RUN_POWER_FIRST, afhds3::RUN_POWER::RUN_POWER_LAST);
      }
#endif

      }
      break;

#if defined(MULTIMODULE)
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_AUTOBIND:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AUTOBIND:
#endif
        if (g_model.moduleData[moduleIdx].multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2) {
          int8_t value = (g_model.moduleData[moduleIdx].multi.optionValue & 0x02) >> 1;
          lcdDrawText(INDENT_WIDTH, y, STR_MULTI_SERVOFREQ);
          lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, value ? 11 : 22, attr);
          lcdDrawText(lcdNextPos, y, "ms", attr);
          if (attr) {
            CHECK_INCDEC_MODELVAR(event, value, 0, 1);
            if (checkIncDec_Ret) {
              g_model.moduleData[moduleIdx].multi.optionValue = (g_model.moduleData[moduleIdx].multi.optionValue & 0xFD) + (value << 1);
            }
          }
        }
        else {
          g_model.moduleData[moduleIdx].multi.autoBindMode = editCheckBox(g_model.moduleData[moduleIdx].multi.autoBindMode, MODEL_SETUP_2ND_COLUMN, y,
                                                                          STR_MULTI_AUTOBIND, attr, event);
        }
        break;

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_DISABLE_TELEM:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_DISABLE_TELEM:
#endif
        g_model.moduleData[moduleIdx].multi.disableTelemetry = editCheckBox(g_model.moduleData[moduleIdx].multi.disableTelemetry, MODEL_SETUP_2ND_COLUMN, y, INDENT TR_DISABLE_TELEM, attr, event);
        break;

#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_DISABLE_MAPPING:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_DISABLE_MAPPING:
#endif
        g_model.moduleData[moduleIdx].multi.disableMapping = editCheckBox(g_model.moduleData[moduleIdx].multi.disableMapping, MODEL_SETUP_2ND_COLUMN, y, INDENT TR_DISABLE_CH_MAP, attr, event);
        break;
#endif

#if defined(AFHDS3) && defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_RX_FREQ:
        lcdDrawText(INDENT_WIDTH, y, STR_AFHDS3_RX_FREQ);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, g_model.moduleData[EXTERNAL_MODULE].afhds3.rxFreq(), attr | LEFT);
        if (attr) {
          uint16_t rxFreq = g_model.moduleData[EXTERNAL_MODULE].afhds3.rxFreq();
          CHECK_INCDEC_MODELVAR(event, rxFreq, MIN_FREQ, MAX_FREQ);
          g_model.moduleData[EXTERNAL_MODULE].afhds3.setRxFreq(rxFreq);
        }
        break;

      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_ACTUAL_POWER:
        lcdDrawText(INDENT_WIDTH, y, STR_AFHDS3_ACTUAL_POWER);
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_AFHDS3_POWERS, actualAfhdsRunPower(EXTERNAL_MODULE), LEFT);
        break;
#endif

#if defined(MULTIMODULE)
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_STATUS:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_STATUS:
#endif
      {
        lcdDrawTextAlignedLeft(y, STR_MODULE_STATUS);
        getModuleStatusString(moduleIdx, reusableBuffer.moduleSetup.msg);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, reusableBuffer.moduleSetup.msg);
        break;
      }
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_SYNCSTATUS:
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_SYNCSTATUS:
#endif
        {
          lcdDrawTextAlignedLeft(y, STR_MODULE_SYNC);
          getModuleSyncStatusString(moduleIdx, reusableBuffer.moduleSetup.msg);
          lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, reusableBuffer.moduleSetup.msg);
          break;
        }
#endif

#if defined(AFHDS3) && defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_STATUS: {
        lcdDrawTextAlignedLeft(y, STR_MODULE_STATUS);

        char statusText[64];
        getModuleStatusString(moduleIdx, statusText);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
        break;
      }
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_AFHDS3_POWER_STATUS: {
        lcdDrawText(INDENT_WIDTH, y, STR_AFHDS3_POWER_SOURCE);
        char statusText[64];
        getModuleSyncStatusString(moduleIdx, statusText);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN, y, statusText);
        break;
      }
#endif

#if 0
      case ITEM_MODEL_SETUP_PPM2_PROTOCOL:
        lcdDrawTextAlignedLeft(y, "Port2");
        lcdDrawTextAtIndex(MODEL_SETUP_2ND_COLUMN, y, STR_VPROTOS, 0, 0);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+4*FW+3, y, STR_CH, menuHorizontalPosition<=0 ? attr : 0);
        lcdDrawNumber(lcdLastRightPos, y, g_model.moduleData[1].channelsStart+1, LEFT | (menuHorizontalPosition<=0 ? attr : 0));
        lcdDrawChar(lcdLastRightPos, y, '-');
        lcdDrawNumber(lcdLastRightPos + FW+1, y, g_model.moduleData[1].channelsStart+8+g_model.moduleData[1].channelsCount, LEFT | (menuHorizontalPosition!=0 ? attr : 0));
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].channelsStart, 32-8-g_model.moduleData[1].channelsCount);
              setDefaultPpmFrameLength(1);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].channelsCount, -4, min<int8_t>(8, 32-8-g_model.moduleData[1].channelsStart));
              setDefaultPpmFrameLength(1);
              break;
          }
        }
        break;

      case ITEM_MODEL_SETUP_PPM2_PARAMS:
        lcdDrawTextAlignedLeft(y, STR_PPMFRAME);
        lcdDrawText(MODEL_SETUP_2ND_COLUMN+3*FW, y, STR_MS);
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN, y, (int16_t)g_model.moduleData[1].ppmFrameLength*5 + 225, (menuHorizontalPosition<=0 ? attr : 0) | PREC1 | LEFT);
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, 'u');
        lcdDrawNumber(MODEL_SETUP_2ND_COLUMN+8*FW+2, y, (g_model.moduleData[1].ppmDelay*50)+300, RIGHT | ((menuHorizontalPosition < 0 || menuHorizontalPosition==1) ? attr : 0));
        lcdDrawChar(MODEL_SETUP_2ND_COLUMN+10*FW, y, g_model.moduleData[1].ppmPulsePol ? '+' : '-', (menuHorizontalPosition < 0 || menuHorizontalPosition==2) ? attr : 0);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmFrameLength, -20, 35);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.moduleData[1].ppmDelay, -4, 10);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.moduleData[1].ppmPulsePol, 1);
              break;
          }
        }
        break;
#endif
    }
  }

#if defined(PXX)
  if (isModuleInRangeCheckMode()) {
    showMessageBox("RSSI: ");
    lcdDrawNumber(WARNING_LINE_X, 5*FH, TELEMETRY_RSSI(), BOLD);
  }
#endif

  // some field just finished being edited
  if (old_editMode > 0 && s_editMode == 0) {
    switch(menuVerticalPosition) {
#if defined(HARDWARE_INTERNAL_MODULE)
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE:
      case ITEM_MODEL_SETUP_INTERNAL_MODULE_PXX2_MODEL_NUM:
        if (menuHorizontalPosition == 0)
          checkModelIdUnique(g_eeGeneral.currModel, INTERNAL_MODULE);
        break;
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_NOT_ACCESS_RXNUM_BIND_RANGE:
      case ITEM_MODEL_SETUP_EXTERNAL_MODULE_PXX2_MODEL_NUM:
        if (menuHorizontalPosition == 0)
          checkModelIdUnique(g_eeGeneral.currModel, EXTERNAL_MODULE);
        break;
#endif
    }
  }
}
