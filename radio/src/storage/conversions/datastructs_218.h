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

#ifndef OPENTX_DATASTRUCTS_218_H
#define OPENTX_DATASTRUCTS_218_H

#define MAX_TIMERS_218                     3
#define MAX_GVARS_218                      9

#if defined(PCBHORUS)
  #define NUM_SWITCHES_218                  8
#elif defined(PCBXLITE)
  #define NUM_SWITCHES_218                  4
#elif defined(PCBX7)
  #define NUM_SWITCHES_218                  6
#elif defined(PCBX9E)
  #define NUM_SWITCHES_218                  18 // yes, it's a lot!
#else
  #define NUM_SWITCHES_218                  8
#endif

#if defined(PCBHORUS)
  #define LEN_SWITCH_NAME_218              3
  #define LEN_ANA_NAME_218                 3
  #define LEN_MODEL_FILENAME_218           16
  #define LEN_BLUETOOTH_NAME_218           10
#else
  #define LEN_SWITCH_NAME_218              3
  #define LEN_ANA_NAME_218                 3
  #define LEN_BLUETOOTH_NAME_218           10
#endif

#define LEN_GVAR_NAME_218                  3

#if defined(PCBHORUS)
  #define LEN_MODEL_NAME_218               15
  #define LEN_TIMER_NAME_218               8
  #define LEN_FLIGHT_MODE_NAME_218         10
  #define LEN_BITMAP_NAME_218              10
  #define LEN_EXPOMIX_NAME_218             6
  #define LEN_CHANNEL_NAME_218             6
  #define LEN_INPUT_NAME_218               4
  #define LEN_CURVE_NAME_218               3
  #define LEN_FUNCTION_NAME_218            6
  #define MAX_CURVES_218                   32
  #define MAX_CURVE_POINTS_218             512
#elif LCD_W == 212
  #define LEN_MODEL_NAME_218               12
  #define LEN_TIMER_NAME_218               8
  #define LEN_FLIGHT_MODE_NAME_218         10
  #define LEN_BITMAP_NAME_218              10
  #define LEN_EXPOMIX_NAME_218             8
  #define LEN_CHANNEL_NAME_218             6
  #define LEN_INPUT_NAME_218               4
  #define LEN_CURVE_NAME_218               3
  #define LEN_FUNCTION_NAME_218            8
  #define MAX_CURVES_218                   32
  #define MAX_CURVE_POINTS_218             512
#else
  #define LEN_MODEL_NAME_218               10
  #define LEN_TIMER_NAME_218               3
  #define LEN_FLIGHT_MODE_NAME_218         6
  #define LEN_EXPOMIX_NAME_218             6
  #define LEN_CHANNEL_NAME_218             4
  #define LEN_INPUT_NAME_218               3
  #define LEN_CURVE_NAME_218               3
  #define LEN_FUNCTION_NAME_218            6
  #define MAX_CURVES_218                   16
  #define MAX_CURVE_POINTS_218             512
#endif

#if defined(PCBHORUS)
  #define MAX_MODELS_218                   60
  #define MAX_OUTPUT_CHANNELS_218          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES_218             9
  #define MAX_MIXERS_218                   64
  #define MAX_EXPOS_218                    64
  #define MAX_LOGICAL_SWITCHES_218         64
  #define MAX_SPECIAL_FUNCTIONS_218        64 // number of functions assigned to switches
  #define MAX_SCRIPTS_218                  9
  #define MAX_INPUTS_218                   32
  #define MAX_TRAINER_CHANNELS_218         16
  #define MAX_TELEMETRY_SENSORS_218        32
  #define MAX_CUSTOM_SCREENS_218           5
#elif defined(PCBTARANIS)
  #define MAX_MODELS_218                   60
  #define MAX_OUTPUT_CHANNELS_218          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES_218             9
  #define MAX_MIXERS_218                   64
  #define MAX_EXPOS_218                    64
  #define MAX_LOGICAL_SWITCHES_218         64
  #define MAX_SPECIAL_FUNCTIONS_218        64 // number of functions assigned to switches
  #define MAX_SCRIPTS_218                  7
  #define MAX_INPUTS_218                   32
  #define MAX_TRAINER_CHANNELS_218         16
  #define MAX_TELEMETRY_SENSORS_218        32
#elif defined(PCBSKY9X)
  #define MAX_MODELS_218                   60
  #define MAX_OUTPUT_CHANNELS_218          32 // number of real output channels CH1-CH32
  #define MAX_FLIGHT_MODES_218             9
  #define MAX_MIXERS_218                   64
  #define MAX_EXPOS_218                    32
  #define MAX_LOGICAL_SWITCHES_218         64
  #define MAX_SPECIAL_FUNCTIONS_218        64 // number of functions assigned to switches
  #define MAX_INPUTS_218                   32
  #define MAX_TRAINER_CHANNELS_218         16
  #define MAX_TELEMETRY_SENSORS_218        32
#endif

#if defined(COLORLCD)
typedef uint32_t swarnstate218_t;
#elif defined(PCBX9E)
typedef uint64_t swconfig218_t;
typedef uint64_t swarnstate218_t;
typedef uint32_t swarnenable218_t;
#elif defined(PCBTARANIS)
typedef uint16_t swconfig218_t;
typedef uint16_t swarnstate218_t;
typedef uint8_t swarnenable218_t;
#else
typedef uint8_t swarnstate218_t;
typedef uint8_t swarnenable218_t;
#endif

PACK(typedef struct {
  uint8_t type:4;
  int8_t  rfProtocol:4;
  uint8_t channelsStart;
  int8_t  channelsCount; // 0=8 channels
  uint8_t failsafeMode:4;  // only 3 bits used
  uint8_t subType:3;
  uint8_t invertedSerial:1; // telemetry serial inverted from standard
  int16_t failsafeChannels[MAX_OUTPUT_CHANNELS_218];
  union {
    struct {
      int8_t  delay:6;
      uint8_t pulsePol:1;
      uint8_t outputType:1;    // false = open drain, true = push pull
      int8_t  frameLength;
    } ppm;
    struct {
      uint8_t rfProtocolExtra:2;
      uint8_t spare1:3;
      uint8_t customProto:1;
      uint8_t autoBindMode:1;
      uint8_t lowPowerMode:1;
      int8_t optionValue;
    } multi;
    struct {
      uint8_t power:2;                  // 0=10 mW, 1=100 mW, 2=500 mW, 3=1W
      uint8_t spare1:2;
      uint8_t receiverTelemetryOff:1;     // false = receiver telem enabled
      uint8_t receiverHigherChannels:1;  // false = pwm out 1-8, true 9-16
      uint8_t external_antenna:1;       // false = internal antenna, true = external antenna
      uint8_t fast:1;
      uint8_t spare2;
    } pxx;
    struct {
      uint8_t spare1:6;
      uint8_t noninverted:1;
      uint8_t spare2:1;
      int8_t refreshRate;  // definition as framelength for ppm (* 5 + 225 = time in 1/10 ms)
    } sbus;
  };
}) ModuleData_v218;

PACK(typedef struct {
  int32_t  mode:9;            // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint32_t start:23;
  int32_t  value:24;
  uint32_t countdownBeep:2;
  uint32_t minuteBeep:1;
  uint32_t persistent:2;
  int32_t  countdownStart:2;
  uint32_t direction:1;
  char     name[LEN_TIMER_NAME_218];
}) TimerData_v218;

#if defined(PCBSKY9X)
  #define ROTENC_DATA       int16_t rotaryEncoders[1];
#else
  #define ROTENC_DATA
#endif

PACK(typedef struct {
  trim_t trim[NUM_TRIMS];
  char name[LEN_FLIGHT_MODE_NAME_218];
  int16_t swtch:9;       // swtch of phase[0] is not used
  int16_t spare:7;
  uint8_t fadeIn;
  uint8_t fadeOut;
  ROTENC_DATA
  gvar_t gvars[MAX_GVARS_218];
}) FlightModeData_v218;

PACK(typedef struct {
  int16_t  weight:11;       // GV1=-1024, -GV1=1023
  uint16_t destCh:5;
  uint16_t srcRaw:10;       // srcRaw=0 means not used
  uint16_t carryTrim:1;
  uint16_t mixWarn:2;       // mixer warning
  uint16_t mltpx:2;         // multiplex method: 0 means +=, 1 means *=, 2 means :=
  uint16_t spare:1;
  int32_t  offset:14;
  int32_t  swtch:9;
  uint32_t flightModes:9;
  CurveRef curve;
  uint8_t  delayUp;
  uint8_t  delayDown;
  uint8_t  speedUp;
  uint8_t  speedDown;
  char     name[LEN_EXPOMIX_NAME_218];
}) MixData_v218;

PACK(typedef struct {
  uint16_t mode:2;
  uint16_t scale:14;
  uint16_t srcRaw:10;
  int16_t  carryTrim:6;
  uint32_t chn:5;
  int32_t  swtch:9;
  uint32_t flightModes:9;
  int32_t  weight:8;
  int32_t  spare:1;
  char     name[LEN_EXPOMIX_NAME_218];
  int8_t   offset;
  CurveRef curve;
}) ExpoData_v218;

PACK(typedef struct {
  uint8_t type:1;
  uint8_t smooth:1;
  int8_t  points:6;   // describes number of points - 5
  char name[LEN_CURVE_NAME_218];
}) CurveHeader_v218;

PACK(typedef struct {
  int16_t        calib[4];
  TrainerMix mix[4];
}) TrainerData_v218;

PACK(typedef struct { // Logical Switches data
  uint8_t  func;
  int32_t  v1:10;
  int32_t  v3:10;
  int32_t  andsw:9;
  uint32_t andswtype:1;
  uint32_t spare:2;
  int16_t  v2;
  uint8_t  delay;
  uint8_t  duration;
}) LogicalSwitchData_v218;

#if defined(PCBTARANIS)
#define CFN_SPARE_TYPE               int32_t
#else
#define CFN_SPARE_TYPE               int16_t
#endif

PACK(typedef struct {
  int16_t  swtch:9;
  uint16_t func:7;
  PACK(union {
    PACK(struct {
      char name[LEN_FUNCTION_NAME_218];
    }) play;

    PACK(struct {
      int16_t val;
      uint8_t mode;
      uint8_t param;
      CFN_SPARE_TYPE spare2;
    }) all;

    PACK(struct {
      int32_t val1;
      CFN_SPARE_TYPE val2;
    }) clear;
  });
  uint8_t active;
}) CustomFunctionData_v218;

PACK(struct GVarData_v218 {
  char    name[LEN_GVAR_NAME_218];
  uint32_t min:12;
  uint32_t max:12;
  uint32_t popup:1;
  uint32_t prec:1;
  uint32_t unit:2;
  uint32_t spare:4;
});

#if defined(COLORLCD)
PACK(struct FrSkyTelemetryData_v217 {
  uint8_t varioSource:7;
  uint8_t varioCenterSilent:1;
  int8_t  varioCenterMax;
  int8_t  varioCenterMin;
  int8_t  varioMin;
  int8_t  varioMax;
});
#else
union FrSkyScreenData {
  FrSkyBarData  bars[4];
  FrSkyLineData lines[4];
#if defined(PCBTARANIS)
  TelemetryScriptData script;
#endif
};
PACK(struct FrSkyTelemetryData_v217 {
  uint8_t voltsSource;
  uint8_t altitudeSource;
  uint8_t screensType; // 2bits per screen (None/Gauges/Numbers/Script)
  FrSkyScreenData screens[MAX_TELEMETRY_SCREENS];
  uint8_t varioSource:7;
  uint8_t varioCenterSilent:1;
  int8_t  varioCenterMax;
  int8_t  varioCenterMin;
  int8_t  varioMin;
  int8_t  varioMax;
});
#endif

#if defined(PCBHORUS)
  #define MODELDATA_EXTRA_218   uint8_t spare:3; uint8_t trainerMode:3; uint8_t potsWarnMode:2; ModuleData_v218 moduleData[NUM_MODULES+1]; ScriptData scriptsData[MAX_SCRIPTS_218]; char inputNames[MAX_INPUTS_218][LEN_INPUT_NAME_218]; uint8_t potsWarnEnabled; int8_t potsWarnPosition[3 + 4];
#elif defined(PCBTARANIS)
  #define MODELDATA_EXTRA_218   uint8_t spare:3; uint8_t trainerMode:3; uint8_t potsWarnMode:2; ModuleData_v218 moduleData[NUM_MODULES+1]; ScriptData scriptsData[MAX_SCRIPTS_218]; char inputNames[MAX_INPUTS_218][LEN_INPUT_NAME_218]; uint8_t potsWarnEnabled; int8_t potsWarnPosition[STORAGE_NUM_POTS + STORAGE_NUM_SLIDERS];
#elif defined(PCBSKY9X)
  #define MODELDATA_EXTRA_218   uint8_t spare:6;                        uint8_t potsWarnMode:2; ModuleData_v218 moduleData[NUM_MODULES+1];                                          char inputNames[MAX_INPUTS_218][LEN_INPUT_NAME_218]; uint8_t potsWarnEnabled; int8_t potsWarnPosition[STORAGE_NUM_POTS + STORAGE_NUM_SLIDERS]; uint8_t rxBattAlarms[2];
#endif

PACK(struct TelemetrySensor_218 {
  union {
    uint16_t id;                   // data identifier, for FrSky we can reuse existing ones. Source unit is derived from type.
    uint16_t persistentValue;
  };
  union {
    PACK(struct {
      uint8_t physID:5;
      uint8_t rxIndex:3; // 1 bit for module index, 2 bits for receiver index
    }) frskyInstance;
    uint8_t instance;
    uint8_t formula;
  };
  char     label[TELEM_LABEL_LEN]; // user defined label
  uint8_t  type:1;                 // 0=custom / 1=calculated
  uint8_t  unit:5;                 // user can choose what unit to display each value in
  uint8_t  prec:2;
  uint8_t  autoOffset:1;
  uint8_t  filter:1;
  uint8_t  logs:1;
  uint8_t  persistent:1;
  uint8_t  onlyPositive:1;
  uint8_t  subId:3;
  union {
    PACK(struct {
      uint16_t ratio;
      int16_t  offset;
    }) custom;
    PACK(struct {
      uint8_t source;
      uint8_t index;
      uint16_t spare;
    }) cell;
    PACK(struct {
      int8_t sources[4];
    }) calc;
    PACK(struct {
      uint8_t source;
      uint8_t spare[3];
    }) consumption;
    PACK(struct {
      uint8_t gps;
      uint8_t alt;
      uint16_t spare;
    }) dist;
    uint32_t param;
  };
});

#if defined(PCBHORUS)
  PACK(struct CustomScreenData_v218 {
    char LayoutId[10];
    LayoutPersistentData layoutData;
  });
  #define VIEW_DATA \
    CustomScreenData screenData[MAX_CUSTOM_SCREENS]; \
    TopBarPersistentData topbarData; \
    uint8_t view;
#elif defined(PCBTARANIS)
  #define VIEW_DATA   uint8_t view;
#else
  #define VIEW_DATA
#endif

#if LEN_BITMAP_NAME > 0
#define MODEL_HEADER_BITMAP_FIELD_218      char bitmap[10];
#else
#define MODEL_HEADER_BITMAP_FIELD_218
#endif

PACK(struct ModelHeader_v218 {
  char      name[LEN_MODEL_NAME]; // must be first for eeLoadModelName
  uint8_t   modelId[NUM_MODULES];
  MODEL_HEADER_BITMAP_FIELD_218
});

#if defined(COLORLCD)
#define SWITCH_WARNING_DATA_218
#else
#define SWITCH_WARNING_DATA_218 swarnenable218_t switchWarningEnable;
#endif

PACK(struct ModelData_v218 {
  ModelHeader_v218 header;
  TimerData_v218 timers[MAX_TIMERS_218];
  uint8_t   telemetryProtocol:3;
  uint8_t   thrTrim:1;            // Enable Throttle Trim
  uint8_t   noGlobalFunctions:1;
  uint8_t   displayTrims:2;
  uint8_t   ignoreSensorIds:1;
  int8_t    trimInc:3;            // Trim Increments
  uint8_t   disableThrottleWarning:1;
  uint8_t   displayChecklist:1;
  uint8_t   extendedLimits:1;
  uint8_t   extendedTrims:1;
  uint8_t   throttleReversed:1;
  BeepANACenter beepANACenter;
  MixData_v218 mixData[MAX_MIXERS_218];
  LimitData limitData[MAX_OUTPUT_CHANNELS_218];
  ExpoData_v218  expoData[MAX_EXPOS_218];

  CurveHeader_v218 curves[MAX_CURVES_218];
  int8_t    points[MAX_CURVE_POINTS_218];

  LogicalSwitchData_v218 logicalSw[MAX_LOGICAL_SWITCHES_218];
  CustomFunctionData_v218 customFn[MAX_SPECIAL_FUNCTIONS_218];
  SwashRingData swashR;
  FlightModeData_v218 flightModeData[MAX_FLIGHT_MODES_218];

  uint8_t thrTraceSrc;

  swarnstate218_t  switchWarningState;

  SWITCH_WARNING_DATA_218

  GVarData_v218 gvars[MAX_GVARS_218];

  FrSkyTelemetryData_v217 frsky;
  RssiAlarmData rssiAlarms;

  MODELDATA_EXTRA_218

  TelemetrySensor_218 telemetrySensors[MAX_TELEMETRY_SENSORS_218];

  TARANIS_PCBX9E_FIELD(uint8_t toplcdTimer)

  // TODO conversion for custom screens?
  VIEW_DATA
});

#if defined(PCBHORUS)
#define EXTRA_GENERAL_FIELDS_218 \
    uint8_t  auxSerialMode:4; \
    uint8_t  slidersConfig:4; \
    uint32_t switchConfig; \
    uint8_t  potsConfig; /* two bits per pot */ \
    char switchNames[NUM_SWITCHES_218][LEN_SWITCH_NAME_218]; \
    char anaNames[NUM_STICKS+3+4][LEN_ANA_NAME_218]; \
    char currModelFilename[LEN_MODEL_FILENAME_218+1]; \
    uint8_t spare:1; \
    uint8_t blOffBright:7; \
    char bluetoothName[LEN_BLUETOOTH_NAME_218];
#elif defined(PCBTARANIS)
  #if defined(STORAGE_BLUETOOTH) && !defined(PCBX9D) && !defined(PCBX9DP)
    #define BLUETOOTH_FIELDS_218 \
      uint8_t spare; \
      char bluetoothName[LEN_BLUETOOTH_NAME_218];
  #else
    #define BLUETOOTH_FIELDS_218
  #endif
  #define EXTRA_GENERAL_FIELDS_218 \
    uint8_t  auxSerialMode:4; \
    uint8_t  slidersConfig:4; \
    uint8_t  potsConfig; /* two bits per pot */\
    uint8_t  backlightColor; \
    swarnstate218_t switchUnlockStates; \
    swconfig218_t switchConfig; \
    char switchNames[NUM_SWITCHES_218][LEN_SWITCH_NAME_218]; \
    char anaNames[NUM_STICKS+STORAGE_NUM_POTS+STORAGE_NUM_SLIDERS][LEN_ANA_NAME_218]; \
    BLUETOOTH_FIELDS_218
#elif defined(PCBSKY9X)
  #define EXTRA_GENERAL_FIELDS_218 \
    int8_t   txCurrentCalibration; \
    int8_t   temperatureWarn; \
    uint8_t  mAhWarn; \
    uint16_t mAhUsed; \
    int8_t   temperatureCalib; \
    uint8_t  optrexDisplay; \
    uint8_t  sticksGain; \
    uint8_t  rotarySteps; \
    char switchNames[NUM_SWITCHES_218][LEN_SWITCH_NAME_218]; \
    char anaNames[NUM_STICKS+STORAGE_NUM_POTS+STORAGE_NUM_SLIDERS][LEN_ANA_NAME_218];
  #else
    #define EXTRA_GENERAL_FIELDS_218  EXTRA_GENERAL_FIELDS_GENERAL_218
#endif

#if defined(COLORLCD)
  #include "gui/colorlcd/theme.h"
  #define THEME_NAME_LEN 8
  #define THEME_DATA \
    char themeName[THEME_NAME_LEN]; \
    OpenTxTheme::PersistentData themeData;
#else
  #define THEME_DATA
#endif

#if defined(PCBHORUS)
  #define NUM_CALIBRATION_INPUTS  13
#else
  #define NUM_CALIBRATION_INPUTS  (NUM_STICKS+STORAGE_NUM_POTS+STORAGE_NUM_SLIDERS)
#endif

PACK(struct RadioData_v218 {
  uint8_t version;
  uint16_t variant;
  CalibData calib[NUM_CALIBRATION_INPUTS];
  uint16_t chkSum;
  N_HORUS_FIELD(int8_t currModel);
  N_HORUS_FIELD(uint8_t contrast);
  uint8_t vBatWarn;
  int8_t txVoltageCalibration;
  int8_t backlightMode;
  TrainerData_v218 trainer;
  uint8_t view;            // index of view in main screen
  int8_t buzzerMode:2;
  uint8_t fai:1;
  int8_t beepMode:2;      // -2=quiet, -1=only alarms, 0=no keys, 1=all
  uint8_t alarmsFlash:1;
  uint8_t disableMemoryWarning:1;
  uint8_t disableAlarmWarning:1;
  uint8_t stickMode:2;
  int8_t timezone:5;
  uint8_t adjustRTC:1;
  uint8_t inactivityTimer;
  uint8_t telemetryBaudrate:3;
  int8_t splashMode:3;
  int8_t hapticMode:2;
  int8_t switchesDelay;
  uint8_t lightAutoOff;
  uint8_t templateSetup;   // RETA order for receiver channels
  int8_t PPM_Multiplier;
  int8_t hapticLength;
  N_HORUS_FIELD(N_TARANIS_FIELD(uint8_t reNavigation));
  N_HORUS_FIELD(N_TARANIS_FIELD(uint8_t stickReverse));
  int8_t beepLength:3;
  int8_t hapticStrength:3;
  uint8_t gpsFormat:1;
  uint8_t unexpectedShutdown:1;
  uint8_t speakerPitch;
  int8_t speakerVolume;
  int8_t vBatMin;
  int8_t vBatMax;

  uint8_t  backlightBright;
  uint32_t globalTimer;
  uint8_t  bluetoothBaudrate:4;
  uint8_t  bluetoothMode:4;
  uint8_t  countryCode;
  uint8_t  imperial:1;
  uint8_t  noJitterFilter:1; /* 0 - Jitter filter active */
  uint8_t  disableRssiPoweroffAlarm:1;
  uint8_t  USBMode:2;
  uint8_t  spareExtraArm:3;
  char     ttsLanguage[2];
  int8_t   beepVolume:4;
  int8_t   wavVolume:4;
  int8_t   varioVolume:4;
  int8_t   backgroundVolume:4;
  int8_t   varioPitch;
  int8_t   varioRange;
  int8_t   varioRepeat;
  CustomFunctionData_v218 customFn[MAX_SPECIAL_FUNCTIONS_218];

  EXTRA_GENERAL_FIELDS_218

  THEME_DATA
});

#include "chksize.h"

#define CHKSIZE(x, y) check_size<struct x, y>()

static inline void check_struct_218()
{
#if defined(PCBHORUS)
  // FIXME !!!
  CHKSIZE(ModelData_v218, 10664);
#elif defined(PCBX9E)
  CHKSIZE(ModelData_v218, 6520);
#elif defined(PCBX9D)
  CHKSIZE(RadioData_v218, 872);
#endif
}


#undef CHKSIZE

#endif //OPENTX_DATASTRUCTS_218_H
