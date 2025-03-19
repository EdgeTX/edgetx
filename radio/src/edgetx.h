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

#include <stdlib.h>
#include "definitions.h"
#include "edgetx_types.h"
#include "edgetx_helpers.h"
#include "touch.h"
#include "switches.h"

#if defined(SIMU)
#include "targets/simu/simpgmspace.h"
#endif

#include "board.h"

#if !defined(SIMU)
#include "usbd_msc_conf.h"
#endif

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "lib_file.h"
#endif

#if defined(FAI)
  #define IS_FAI_ENABLED() true
  #define IF_FAI_CHOICE(x)
#elif defined(FAI_CHOICE)
  #define IS_FAI_ENABLED() g_eeGeneral.fai
  #define IF_FAI_CHOICE(x) x,
#else
  #define IS_FAI_ENABLED() false
  #define IF_FAI_CHOICE(x)
#endif

#define IS_FAI_FORBIDDEN(idx) (IS_FAI_ENABLED() && isFaiForbidden(idx))

#if defined(BLUETOOTH)
  #if defined(X9E)
    #define IS_BLUETOOTH_TRAINER()       (g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
    #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE)
  #else
    #define IS_BLUETOOTH_TRAINER()       (g_model.trainerData.mode == TRAINER_MODE_MASTER_BLUETOOTH || g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
    #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE || g_model.trainerData.mode == TRAINER_MODE_SLAVE_BLUETOOTH)
  #endif
#else
  #define IS_BLUETOOTH_TRAINER()       false
  #define IS_SLAVE_TRAINER()           (g_model.trainerData.mode == TRAINER_MODE_SLAVE)
#endif

#if defined(LUA) || defined(PXX2) || defined(MULTIMODULE)
  #define RADIO_TOOLS
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
enum RotaryEncoderMode {
  ROTARY_ENCODER_MODE_NORMAL,
  ROTARY_ENCODER_MODE_INVERT_BOTH,
  ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM,
  ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT,
  ROTARY_ENCODER_MODE_VERT_NORM_HORZ_INVERT,
  ROTARY_ENCODER_MODE_LAST = ROTARY_ENCODER_MODE_VERT_NORM_HORZ_INVERT
};
#endif

// RESX range is used for internal calculation; The menu says -100.0 to 100.0; internally it is -1024 to 1024 to allow some optimizations
#define RESX_SHIFT 10
#define RESX       1024
#define RESXu      1024u
#define RESXul     1024ul
#define RESXl      1024l

#if defined(JACK_DETECT_GPIO)
#define CASE_JACK_DETECT(x) x,
#else
#define CASE_JACK_DETECT(x)
#endif

#include "debug.h"

#include "myeeprom.h"
#include "curves.h"

void memswap(void * a, void * b, uint8_t size);

// TODO: move these config check macros somewhere else
#define POT_CONFIG(x) (getPotType(x))

#define IS_POT_MULTIPOS(x) (POT_CONFIG(x) == FLEX_MULTIPOS)

#define IS_POT_WITHOUT_DETENT(x) (POT_CONFIG(x) == FLEX_POT)

#define IS_SLIDER(x) (POT_CONFIG(x) == FLEX_SLIDER)

#define IS_POT_AVAILABLE(x)						\
  (POT_CONFIG(x) != FLEX_NONE && POT_CONFIG(x) < FLEX_SWITCH)

#define IS_POT_SLIDER_AVAILABLE(x) (IS_POT_AVAILABLE(x))

#define IS_MULTIPOS_CALIBRATED(cal)			\
  (cal->count > 0 && cal->count < XPOTS_MULTIPOS_COUNT)

#define IS_SWITCH_MULTIPOS(x) \
  (SWSRC_FIRST_MULTIPOS_SWITCH <= (x) && (x) <= SWSRC_LAST_MULTIPOS_SWITCH)

#define GET_LOWRES_POT_POSITION(i) (getValue(MIXSRC_FIRST_POT + (i)) >> 4)

#define SAVE_POT_POSITION(i) \
  g_model.potsWarnPosition[i] = GET_LOWRES_POT_POSITION(i)

#define ANALOG_CENTER_BEEP(x) \
  (g_model.beepANACenter & ((BeepANACenter)1 << (x)))

#define PPM_CENTER                     1500

#if defined(PPM_CENTER_ADJUSTABLE)
  #define PPM_CH_CENTER(ch)            (PPM_CENTER + limitAddress(ch)->ppmCenter)
#else
  #define PPM_CH_CENTER(ch)            (PPM_CENTER)
#endif

#include "fifo.h"
#include "io/frsky_sport.h"

#if defined(CLI)
#include "cli.h"
#endif

#include "timers.h"
#include "storage/storage.h"
#include "pulses/pulses.h"
#include "pulses/modules_helpers.h"

#define MASK_CFN_TYPE  uint64_t  // current max = 64 customizable switches
#define MASK_FUNC_TYPE uint32_t  // current max = 32 functions

struct CustomFunctionsContext {
  MASK_FUNC_TYPE activeFunctions;
  MASK_CFN_TYPE  activeSwitches;
  tmr10ms_t lastFunctionTime[MAX_SPECIAL_FUNCTIONS];

  inline bool isFunctionActive(uint8_t func)
  {
    return activeFunctions & ((MASK_FUNC_TYPE)1 << func);
  }

  void reset()
  {
    memclear(this, sizeof(*this));
  }
};

#include "strhelpers.h"
#if defined(COLORLCD)
#include "gui_common.h"
#include "menus.h"
#include "popups.h"
#else
#include "gui.h"
#endif

#if !defined(SIMU)
  #define assert(x)
  #if !defined(DEBUG)
    #define printf printf_not_allowed
  #endif
#endif

#define THRCHK_DEADBAND                16

#if !defined(COLORLCD)
inline bool SPLASH_NEEDED()
{
  return g_eeGeneral.splashMode != 3;
}
#endif

#define SPLASH_TIMEOUT (g_eeGeneral.splashMode == -4 ? 1500 : (g_eeGeneral.splashMode <= 0 ? (400-g_eeGeneral.splashMode * 200) : (400 - g_eeGeneral.splashMode * 100)))

extern void startSplash();
extern void waitSplash();
extern void cancelSplash();

extern uint8_t heartbeat;

#define LEN_STD_CHARS 40

#if defined(TRANSLATIONS_CZ)
#define ZCHAR_MAX (LEN_STD_CHARS)
#else
#define ZCHAR_MAX (LEN_STD_CHARS + LEN_SPECIAL_CHARS)
#endif

#include "keys.h"
#include "pwr.h"

bool trimDown(uint8_t idx);

#if defined(KEYS_GPIO_REG_BIND)
void bindButtonHandler(event_t event);
#endif

uint16_t evalChkSum();

void alert(const char * title, const char * msg, uint8_t sound);

#if !defined(GUI)

  #define RAISE_ALERT(...)
  #define ALERT(...)

#elif defined(COLORLCD)

#define TELEMETRY_CHECK_DELAY10ms 150

bool confirmationDialog(const char *title, const char *msg, bool checkPwr = true, const std::function<bool(void)>& closeCondition = nullptr);

void raiseAlert(const char *title, const char *msg, const char *info,
                uint8_t sound);

inline void RAISE_ALERT(const char *title, const char *msg, const char *info,
                        uint8_t sound)
{
  raiseAlert(title, msg, info, sound);
}
inline void ALERT(const char *title, const char *msg, uint8_t sound)
{
  raiseAlert(title, msg, STR_PRESS_ANY_KEY_TO_SKIP, sound);
}

#else // !COLORLCD && GUI

inline void RAISE_ALERT(const char *title, const char *msg, const char *info,
                        uint8_t sound)
{
  showAlertBox(title, msg, info, sound);
}

inline void ALERT(const char *title, const char *msg, uint8_t sound)
{
  alert(title, msg, sound);
}

#endif // !COLORLCD && GUI

enum PerOutMode {
  e_perout_mode_normal = 0,
  e_perout_mode_inactive_flight_mode = 1,
  e_perout_mode_notrainer = 2,
  e_perout_mode_notrims = 4,
  e_perout_mode_nosticks = 8,
  e_perout_mode_noinput = e_perout_mode_notrainer+e_perout_mode_notrims+e_perout_mode_nosticks
};

extern uint8_t mixerCurrentFlightMode;
extern uint8_t lastFlightMode;
extern uint8_t flightModeTransitionLast;

extern uint32_t availableMemory();


void evalFlightModeMixes(uint8_t mode, uint8_t tick10ms);
void evalMixes(uint8_t tick10ms);
void doMixerCalculations();
void doMixerPeriodicUpdates();

void checkTrims();
extern uint8_t currentBacklightBright;
void perMain();

getvalue_t getValue(mixsrc_t i, bool* valid = nullptr);

int8_t getMovedSource(uint8_t min);

#if defined(FLIGHT_MODES)
  extern uint8_t getFlightMode();
#else
  #define getFlightMode() 0
#endif

#define getTrimFlightMode(phase, idx) (phase)

#if defined(GVARS)
  extern int8_t trimGvar[MAX_TRIMS];
  #define TRIM_REUSED(idx) trimGvar[idx] >= 0
#else
  #define TRIM_REUSED(idx) 0
#endif

trim_t getRawTrimValue(uint8_t phase, uint8_t idx);
int getTrimValue(uint8_t phase, uint8_t idx);

bool setTrimValue(uint8_t phase, uint8_t idx, int trim);

#include "gvars.h"

void flightReset(uint8_t check=true);

#define DURATION_MS_PREC2(x) ((x)/20)

#if defined(THRTRACE)
  #if defined(COLORLCD)
    #define MAXTRACE (LCD_W-2*10)
  #else
    #define MAXTRACE (LCD_W - 8)
  #endif
  extern uint8_t  s_traceBuf[MAXTRACE];
  extern uint16_t s_traceWr;
  extern uint8_t  s_cnt_10s;
  extern uint16_t s_cnt_samples_thr_10s;
  extern uint16_t s_sum_samples_thr_10s;
  #define RESET_THR_TRACE() s_traceWr = s_cnt_10s = s_cnt_samples_thr_10s = s_sum_samples_thr_10s = s_timeCum16ThrP = s_timeCumThr = 0
#else
  #define RESET_THR_TRACE() s_timeCum16ThrP = s_timeCumThr = 0
#endif

void checkLowEEPROM();
void checkThrottleStick();
void checkSwitches();
void checkAlarm();
void checkAll(bool isBootCheck = false);

void getADC();

#include "sbus.h"

void resetBacklightTimeout();
void checkBacklight();

uint16_t isqrt32(uint32_t n);

void setDefaultOwnerId();
void generalDefault();

uint32_t hash(const void * ptr, uint32_t size);

#define calc100to256_16Bits(x) calc100to256(x)
#define calc100toRESX_16Bits(x) calc100toRESX(x)

#define calc100to256_16Bits(x) calc100to256(x)
#define calc100toRESX_16Bits(x) calc100toRESX(x)

inline int calc100to256(int x)
{
  return divRoundClosest(x*256, 100);
}

inline int calc100toRESX(int x)
{
  return divRoundClosest(x*RESX, 100);
}

inline int calc1000toRESX(int x)
{
  return divRoundClosest(x*RESX, 1000);
}

inline int calcRESXto1000(int x)
{
  return divRoundClosest(x*1000, RESX);
}

inline int calcRESXto100(int x)
{
  return divRoundClosest(x*100, RESX);
}

#define g_blinkTmr10ms    (*(uint8_t*)&g_tmr10ms)

#include "trainer.h"

int expo(int x, int k);

extern void getMixSrcRange(const int source, int16_t & valMin, int16_t & valMax, LcdFlags * flags = nullptr);

void applyExpos(int16_t * anas, uint8_t mode, int16_t ovwrIdx=0, int16_t ovwrValue=0);
int16_t applyLimits(uint8_t channel, int32_t value);

void evalInputs(uint8_t mode);
uint16_t anaIn(uint8_t chan);

#define FLASH_DURATION 20 /*200ms*/

FlightModeData * flightModeAddress(uint8_t idx);
ExpoData * expoAddress(uint8_t idx);
LimitData * limitAddress(uint8_t idx);
LogicalSwitchData * lswAddress(uint8_t idx);
USBJoystickChData * usbJChAddress(uint8_t idx);

void applyDefaultTemplate();
void instantTrim();
void evalTrims();
void copyTrimsToOffset(uint8_t ch);
void copySticksToOffset(uint8_t ch);
void copyMinMaxToOutputs(uint8_t ch);
void moveTrimsToOffsets();

inline bool isExpoActive(uint8_t expo)
{
  return mixState[expo].activeExpo;
}

inline bool isMixActive(uint8_t mix)
{
  return mixState[mix].activeMix;
}

enum FunctionsActive {
  FUNCTION_TRAINER_STICK1,
  FUNCTION_TRAINER_CHANNELS = FUNCTION_TRAINER_STICK1 + MAX_STICKS,
  FUNCTION_INSTANT_TRIM,
  FUNCTION_VARIO,
  FUNCTION_LOGS,
  FUNCTION_BACKGND_MUSIC,
  FUNCTION_BACKGND_MUSIC_PAUSE,
  FUNCTION_BACKLIGHT,
  FUNCTION_RACING_MODE,
  FUNCTION_DISABLE_TOUCH,
  FUNCTION_DISABLE_AUDIO_AMP,
};

#define VARIO_FREQUENCY_ZERO   700/*Hz*/
#define VARIO_FREQUENCY_RANGE  1000/*Hz*/
#define VARIO_REPEAT_ZERO      500/*ms*/
#define VARIO_REPEAT_MAX       80/*ms*/

extern CustomFunctionsContext modelFunctionsContext;
extern CustomFunctionsContext globalFunctionsContext;
inline bool isFunctionActive(uint8_t func)
{
  return globalFunctionsContext.isFunctionActive(func) || modelFunctionsContext.isFunctionActive(func);
}
void evalFunctions(CustomFunctionData * functions, CustomFunctionsContext & functionsContext);
inline void customFunctionsReset()
{
  globalFunctionsContext.reset();
  modelFunctionsContext.reset();
}

const char* funcGetLabel(uint8_t func);
uint8_t getFuncSortIdx(uint8_t func);
extern Functions cfn_sorted[];

#include "telemetry/telemetry.h"
#include "crc.h"

#define PLAY_REPEAT(x)            (x)                 /* Range 0 to 15 */
#define PLAY_NOW                  0x10
#define PLAY_BACKGROUND           0x20

enum AUDIO_SOUNDS {
  AUDIO_HELLO,
  AU_BYE,
  AU_THROTTLE_ALERT,
  AU_SWITCH_ALERT,
  AU_BAD_RADIODATA,
  AU_TX_BATTERY_LOW,
  AU_INACTIVITY,
  AU_RSSI_ORANGE,
  AU_RSSI_RED,
  AU_RAS_RED,
  AU_TELEMETRY_CONNECTED,
  AU_TELEMETRY_LOST,
  AU_TELEMETRY_BACK,
  AU_TRAINER_CONNECTED,
  AU_TRAINER_LOST,
  AU_TRAINER_BACK,
  AU_SENSOR_LOST,
  AU_SERVO_KO,
  AU_RX_OVERLOAD,
  AU_MODEL_STILL_POWERED,
  AU_ERROR,
  AU_WARNING1,
  AU_WARNING2,
  AU_WARNING3,
  AU_TRIM_MIDDLE,
  AU_TRIM_MIN,
  AU_TRIM_MAX,
  AU_STICK1_MIDDLE,
  AU_STICK2_MIDDLE,
  AU_STICK3_MIDDLE,
  AU_STICK4_MIDDLE,
#if defined(PCBFRSKY)
  AU_POT1_MIDDLE,
  AU_POT2_MIDDLE,
#if defined(PCBX9E)
  AU_POT3_MIDDLE,
  AU_POT4_MIDDLE,
#endif
#if defined(PCBX10)
  AU_POT4_MIDDLE,
  AU_POT5_MIDDLE,
  AU_POT6_MIDDLE,
  AU_POT7_MIDDLE,
#endif
  AU_SLIDER1_MIDDLE,
  AU_SLIDER2_MIDDLE,
#if defined(PCBX9E)
  AU_SLIDER3_MIDDLE,
  AU_SLIDER4_MIDDLE,
#endif
#else
  AU_POT1_MIDDLE,
  AU_POT2_MIDDLE,
  AU_POT3_MIDDLE,
#endif
  AU_MIX_WARNING_1,
  AU_MIX_WARNING_2,
  AU_MIX_WARNING_3,
  AU_TIMER1_ELAPSED,
  AU_TIMER2_ELAPSED,
  AU_TIMER3_ELAPSED,

  AU_SPECIAL_SOUND_FIRST,
  AU_SPECIAL_SOUND_BEEP1 = AU_SPECIAL_SOUND_FIRST,
  AU_SPECIAL_SOUND_BEEP2,
  AU_SPECIAL_SOUND_BEEP3,
  AU_SPECIAL_SOUND_WARN1,
  AU_SPECIAL_SOUND_WARN2,
  AU_SPECIAL_SOUND_CHEEP,
  AU_SPECIAL_SOUND_RATATA,
  AU_SPECIAL_SOUND_TICK,
  AU_SPECIAL_SOUND_SIREN,
  AU_SPECIAL_SOUND_RING,
  AU_SPECIAL_SOUND_SCIFI,
  AU_SPECIAL_SOUND_ROBOT,
  AU_SPECIAL_SOUND_CHIRP,
  AU_SPECIAL_SOUND_TADA,
  AU_SPECIAL_SOUND_CRICKET,
  AU_SPECIAL_SOUND_ALARMC,
  AU_SPECIAL_SOUND_LAST,

  AU_NONE = 0xff
};

#if defined(AUDIO)
#include "audio.h"
#endif

#include "buzzer.h"
#include "translations.h"

#if defined(HAPTIC)
#include "haptic.h"
#endif

#include "sdcard.h"

#if defined(RTCLOCK)
#include "rtc.h"
#endif


void checkBattery();
void edgeTxClose(uint8_t shutdown=true);
void edgeTxInit();
void edgeTxResume();

constexpr uint8_t OPENTX_START_NO_SPLASH = 0x01;
constexpr uint8_t OPENTX_START_NO_CALIBRATION = 0x02;
constexpr uint8_t OPENTX_START_NO_CHECKS = 0x04;

#if defined(STATUS_LEDS)
  #define LED_ERROR_BEGIN()            ledRed()
  // Green "ready to use" if available, unless overridden by user or mfg preference
#if !defined(POWER_LED_BLUE) && (defined(LED_GREEN_GPIO) || defined(LED_STRIP_GPIO))
  #define LED_ERROR_END() ledGreen()
  #define LED_BIND() ledBlue()
#else
// Either green is not an option, or blue is preferred "ready to use" color
  #define LED_ERROR_END()              ledBlue()
#endif
#else
  #define LED_ERROR_BEGIN()
  #define LED_ERROR_END()
#endif

#if LCD_W <= 212
constexpr uint8_t SD_SCREEN_FILE_LENGTH = 32;
#else
constexpr uint8_t SD_SCREEN_FILE_LENGTH = 64;
#endif

#if defined(BLUETOOTH)
#include "bluetooth.h"
#endif

constexpr uint8_t TEXT_FILENAME_MAXLEN = 40;

#if defined(GHOST)
  #include "telemetry/ghost_menu.h"
#endif

// Re-useable byte array to save having multiple buffers
union ReusableBuffer
{
  struct {
#if !defined(COLORLCD)
    char menu_bss[POPUP_MENU_MAX_LINES][MENU_LINE_LENGTH];
    char mainname[45]; // because reused for SD backup / restore, max backup filename 44 chars: "/MODELS/MODEL0134353-2014-06-19-04-51-27.bin"
#elif !defined(COLORLCD)
    char mainname[LEN_MODEL_NAME];
#endif
  } modelsel;

  struct {
    char msg[64];
    uint8_t r9mPower;
    int8_t antennaMode;
    uint8_t previousType;
    uint8_t newType;
#if defined(PXX2)
    BindInformation bindInformation;
    PXX2ModuleSetup pxx2;
#endif
#if defined(BLUETOOTH)
    struct {
      char devices[MAX_BLUETOOTH_DISTANT_ADDR][LEN_BLUETOOTH_ADDR+1];
      uint8_t devicesCount;
    } bt;
#endif
  } moduleSetup;

  struct {
    uint8_t state;
    union {
      struct {
        int16_t midVal;
        int16_t loVal;
        int16_t hiVal;
      } input;
      struct {
        uint8_t stepsCount;
        int16_t steps[XPOTS_MULTIPOS_COUNT];
        uint8_t lastCount;
        int16_t lastPosition;
      } xpot;
    } inputs[MAX_ANALOG_INPUTS];
  } calib;

  struct {
#if defined(NUM_BODY_LINES)
    char lines[NUM_BODY_LINES][SD_SCREEN_FILE_LENGTH+1+1]; // the last char is used to store the flags (directory) of the line
#endif
    uint32_t available;
    uint16_t offset;
    uint16_t count;
    char originalName[SD_SCREEN_FILE_LENGTH+1];
#if defined(PXX2)
    OtaUpdateInformation otaUpdateInformation;
    char otaReceiverVersion[sizeof(TR_CURRENT_VERSION) + 12];
#endif
  } sdManager;

  struct
  {
    char id[27];
  } version;

#if defined(PXX2)
  PXX2HardwareAndSettings hardwareAndSettings; // radio_version
#endif

#if defined(NUM_BODY_LINES)
  #define TOOL_NAME_MAX_LEN (LCD_W / FW)
  #define TOOL_PATH_MAX_LEN (LEN_FILE_PATH_MAX + 10)
  struct scriptInfo{
      uint8_t index;
      char label[TOOL_NAME_MAX_LEN + 1];
      uint8_t module;
      void (* tool)(event_t);
      char path[TOOL_PATH_MAX_LEN + 1];
  };
#endif

  struct {
#if defined(NUM_BODY_LINES)
    scriptInfo script[NUM_BODY_LINES];
    uint8_t oldOffset;
#endif
#if defined(PXX2)
    ModuleInformation modules[NUM_MODULES];
#endif
    char msg[64];
#if !defined(COLORLCD)
    uint8_t linesCount;
#endif
  } radioTools;

  struct {
    int8_t antennaMode;
  } radioHardware;

  struct {
    uint8_t stickMode;
#if defined(ROTARY_ENCODER_NAVIGATION)
    uint8_t rotaryEncoderMode;
#endif
  } generalSettings;

  struct {
    uint8_t bars[LCD_W];
    uint8_t max[LCD_W];
    uint32_t freq;
    uint32_t span;
    uint32_t step;
    uint32_t track;
    uint8_t spanDefault;
    uint8_t spanMax;
    uint16_t freqDefault;
    uint16_t freqMax;
    uint16_t freqMin;
    uint8_t dirty;
    uint8_t moduleOFF;
  } spectrumAnalyser;

#if defined(GHOST)
  struct {
    GhostMenuData line[GHST_MENU_LINES + 1];
    uint8_t menuStatus;
    uint8_t menuAction;
    uint8_t buttonAction;
  } ghostMenu;
#endif

  struct {
    uint32_t freq;
    int16_t power;
    int16_t peak;
    uint8_t attn;
    uint8_t dirty;
  } powerMeter;

  struct {
    int8_t preset;
  } curveEdit;

#if !defined(COLORLCD)
  struct {
    char filename[TEXT_FILENAME_MAXLEN];
    char lines[NUM_BODY_LINES][LCD_COLS + 1];
    int linesCount;
    bool checklistComplete;
    bool pushMenu;
  } viewText;
#endif

  struct {
    uint8_t maxNameLen;
  } modelFailsafe;

  struct {
#if defined(PXX2)
    ModuleInformation internalModule;
#endif
  } viewMain;
};

extern ReusableBuffer reusableBuffer;

// Stick tolerance varies between transmitters, Higher is better
#define STICK_TOLERANCE 64

ls_telemetry_value_t maxTelemValue(source_t channel);

getvalue_t convert16bitsTelemValue(source_t channel, ls_telemetry_value_t value);
getvalue_t convertLswTelemValue(LogicalSwitchData * cs);

inline getvalue_t convertTelemValue(source_t channel, ls_telemetry_value_t value)
{
  return convert16bitsTelemValue(channel, value);
}

extern uint8_t g_vbat100mV;

inline uint8_t GET_TXBATT_BARS(uint8_t barsMax)
{
  return limit<int8_t>(0, divRoundClosest(barsMax * (g_vbat100mV - g_eeGeneral.vBatMin - 90), 30 + g_eeGeneral.vBatMax - g_eeGeneral.vBatMin), barsMax);
}

inline bool IS_TXBATT_WARNING()
{
  return g_vbat100mV <= g_eeGeneral.vBatWarn;
}

enum TelemetryViews {
  TELEMETRY_CUSTOM_SCREEN_1,
  TELEMETRY_CUSTOM_SCREEN_2,
  TELEMETRY_CUSTOM_SCREEN_3,
  TELEMETRY_CUSTOM_SCREEN_4,
  TELEMETRY_VIEW_MAX = TELEMETRY_CUSTOM_SCREEN_4
};

extern uint8_t selectedTelemView;

constexpr uint32_t EARTH_RADIUS = 6371009;

void varioWakeup();

#if defined(AUDIO) && defined(BUZZER)
  #define IS_SOUND_OFF() (g_eeGeneral.buzzerMode==e_mode_quiet && g_eeGeneral.beepMode==e_mode_quiet)
#else
  #define IS_SOUND_OFF() (g_eeGeneral.beepMode == e_mode_quiet)
#endif

#define IS_IMPERIAL_ENABLE() (g_eeGeneral.imperial)

#if defined(PCBTARANIS)
  extern const unsigned char logo_taranis[];
#endif

#include "lua/lua_api.h"

enum ClipboardType {
  CLIPBOARD_TYPE_NONE,
  CLIPBOARD_TYPE_CUSTOM_SWITCH,
  CLIPBOARD_TYPE_CUSTOM_FUNCTION,
  CLIPBOARD_TYPE_SD_FILE,
};

#if defined(SIMU)
  #define CLIPBOARD_PATH_LEN 1024
#else
  #define CLIPBOARD_PATH_LEN 32
#endif

struct Clipboard {
  ClipboardType type;
  union {
    LogicalSwitchData csw;
    CustomFunctionData cfn;
    struct {
      char directory[CLIPBOARD_PATH_LEN];
      char filename[CLIPBOARD_PATH_LEN];
    } sd;
  } data;
};

extern Clipboard clipboard;

#if defined(INTERNAL_GPS)
  #include "gps.h"
#endif

#if defined(SPACEMOUSE)
  #include "spacemouse.h"
#endif

#if defined(JACK_DETECT_GPIO)
enum JackMode {
  JACK_UNSELECTED_MODE,
  JACK_HEADPHONE_MODE,
  JACK_TRAINER_MODE,
  JACK_MAX_MODE = JACK_TRAINER_MODE
};
#endif

#if defined(IMU)
#include "gyro.h"
#endif

#if defined(DEBUG_LATENCY)
extern uint8_t latencyToggleSwitch;
#endif

#include "module.h"

extern CircularBuffer<uint8_t, 8> luaSetStickySwitchBuffer;

// Radio menu tab state
#if defined(COLORLCD)
extern bool radioThemesEnabled();
#endif
extern bool radioGFEnabled();
extern bool radioTrainerEnabled();
extern bool modelHeliEnabled();
extern bool modelFMEnabled();
extern bool modelCurvesEnabled();
extern bool modelGVEnabled();
extern bool modelLSEnabled();
extern bool modelSFEnabled();
extern bool modelCustomScriptsEnabled();
extern bool modelTelemetryEnabled();

int pwrDelayFromYaml(int delay);
int pwrDelayToYaml(int delay);
