/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
#include "datastructs_218.h"

enum Mix216Sources {
  MIXSRC216_NONE,

#if defined(PCBTARANIS)
  MIXSRC216_FIRST_INPUT,
  MIXSRC216_LAST_INPUT = MIXSRC216_FIRST_INPUT+MAX_INPUTS-1,

  MIXSRC216_FIRST_LUA,
  MIXSRC216_LAST_LUA = MIXSRC216_FIRST_LUA+(MAX_SCRIPTS*MAX_SCRIPT_OUTPUTS)-1,
#endif

  MIXSRC216_Rud,
  MIXSRC216_Ele,
  MIXSRC216_Thr,
  MIXSRC216_Ail,

  MIXSRC216_FIRST_POT,
#if defined(PCBTARANIS)
  MIXSRC216_POT1 = MIXSRC216_FIRST_POT,
  MIXSRC216_POT2,
  MIXSRC216_POT3,
  MIXSRC216_SLIDER1,
  MIXSRC216_SLIDER2,
  MIXSRC216_LAST_POT = MIXSRC216_SLIDER2,
#else
  MIXSRC216_P1 = MIXSRC216_FIRST_POT,
  MIXSRC216_P2,
  MIXSRC216_P3,
  MIXSRC216_LAST_POT = MIXSRC216_P3,
#endif

#if defined(PCBSKY9X)
  MIXSRC216_REa,
  MIXSRC216_LAST_ROTARY_ENCODER = MIXSRC216_REa,
#endif

  MIXSRC216_MAX,

  MIXSRC216_CYC1,
  MIXSRC216_CYC2,
  MIXSRC216_CYC3,

  MIXSRC216_TrimRud,
  MIXSRC216_TrimEle,
  MIXSRC216_TrimThr,
  MIXSRC216_TrimAil,

  MIXSRC216_FIRST_SWITCH,

#if defined(PCBTARANIS)
  MIXSRC216_SA = MIXSRC216_FIRST_SWITCH,
  MIXSRC216_SB,
  MIXSRC216_SC,
  MIXSRC216_SD,
  MIXSRC216_SE,
  MIXSRC216_SF,
  MIXSRC216_SG,
  MIXSRC216_SH,
#else
  MIXSRC216_3POS = MIXSRC216_FIRST_SWITCH,
  MIXSRC216_THR,
  MIXSRC216_RUD,
  MIXSRC216_ELE,
  MIXSRC216_AIL,
  MIXSRC216_GEA,
  MIXSRC216_TRN,
#endif
  MIXSRC216_FIRST_LOGICAL_SWITCH,
  MIXSRC216_SW1 = MIXSRC216_FIRST_LOGICAL_SWITCH,
  MIXSRC216_SW9 = MIXSRC216_SW1 + 8,
  MIXSRC216_SWA,
  MIXSRC216_SWB,
  MIXSRC216_SWC,
  MIXSRC216_LAST_LOGICAL_SWITCH = MIXSRC216_FIRST_LOGICAL_SWITCH+32-1,

  MIXSRC216_FIRST_TRAINER,
  MIXSRC216_LAST_TRAINER = MIXSRC216_FIRST_TRAINER+MAX_TRAINER_CHANNELS-1,

  MIXSRC216_FIRST_CH,
  MIXSRC216_CH1 = MIXSRC216_FIRST_CH,
  MIXSRC216_CH2,
  MIXSRC216_CH3,
  MIXSRC216_CH4,
  MIXSRC216_CH5,
  MIXSRC216_CH6,
  MIXSRC216_CH7,
  MIXSRC216_CH8,
  MIXSRC216_CH9,
  MIXSRC216_CH10,
  MIXSRC216_CH11,
  MIXSRC216_CH12,
  MIXSRC216_CH13,
  MIXSRC216_CH14,
  MIXSRC216_CH15,
  MIXSRC216_CH16,
  MIXSRC216_LAST_CH = MIXSRC216_CH1+MAX_OUTPUT_CHANNELS-1,

  MIXSRC216_GVAR1,
  MIXSRC216_LAST_GVAR = MIXSRC216_GVAR1+MAX_GVARS-1,

  MIXSRC216_FIRST_TELEM,
};

enum Telemetry216Source {
  TELEM216_NONE,
  TELEM216_TX_VOLTAGE,
  TELEM216_TX_TIME,
  TELEM216_RESERVE1,
  TELEM216_RESERVE2,
  TELEM216_RESERVE3,
  TELEM216_RESERVE4,
  TELEM216_RESERVE5,
  TELEM216_TIMER1,
  TELEM216_TIMER2,
  TELEM216_SWR,
  TELEM216_RSSI_TX,
  TELEM216_RSSI_RX,
  TELEM216_RESERVE0,
  TELEM216_A1,
  TELEM216_A2,
  TELEM216_A3,
  TELEM216_A4,
  TELEM216_ALT,
  TELEM216_RPM,
  TELEM216_FUEL,
  TELEM216_T1,
  TELEM216_T2,
  TELEM216_SPEED,
  TELEM216_DIST,
  TELEM216_GPSALT,
  TELEM216_CELL,
  TELEM216_CELLS_SUM,
  TELEM216_VFAS,
  TELEM216_CURRENT,
  TELEM216_CONSUMPTION,
  TELEM216_POWER,
  TELEM216_ACCx,
  TELEM216_ACCy,
  TELEM216_ACCz,
  TELEM216_HDG,
  TELEM216_VSPEED,
  TELEM216_ASPEED,
  TELEM216_DTE,
  TELEM216_RESERVE6,
  TELEM216_RESERVE7,
  TELEM216_RESERVE8,
  TELEM216_RESERVE9,
  TELEM216_RESERVE10,
  TELEM216_MIN_A1,
  TELEM216_MIN_A2,
  TELEM216_MIN_A3,
  TELEM216_MIN_A4,
  TELEM216_MIN_ALT,
  TELEM216_MAX_ALT,
  TELEM216_MAX_RPM,
  TELEM216_MAX_T1,
  TELEM216_MAX_T2,
  TELEM216_MAX_SPEED,
  TELEM216_MAX_DIST,
  TELEM216_MAX_ASPEED,
  TELEM216_MIN_CELL,
  TELEM216_MIN_CELLS_SUM,
  TELEM216_MIN_VFAS,
  TELEM216_MAX_CURRENT,
  TELEM216_MAX_POWER,
  TELEM216_RESERVE11,
  TELEM216_RESERVE12,
  TELEM216_RESERVE13,
  TELEM216_RESERVE14,
  TELEM216_RESERVE15,
  TELEM216_ACC,
  TELEM216_GPS_TIME,
};

PACK(typedef struct {
  uint8_t type:3;
  uint8_t smooth:1;
  uint8_t spare:4;
  int8_t  points;
}) CurveHeader_v216;

#if defined(PCBTARANIS)
PACK(typedef struct {
  uint8_t  srcRaw;
  uint16_t scale;
  uint8_t  chn;
  int8_t   swtch;
  uint16_t flightModes;
  int8_t   weight;
  int8_t   carryTrim:6;
  uint8_t  mode:2;
  char     name[LEN_EXPOMIX_NAME];
  int8_t   offset;
  CurveRef curve;
  uint8_t  spare;
}) ExpoData_v216;
PACK(typedef struct {
  uint32_t srcRaw:10;
  uint32_t scale:14;
  uint32_t chn:8;
  int8_t   swtch;
  uint16_t flightModes;
  int8_t   weight;
  int8_t   carryTrim:6;
  uint8_t  mode:2;
  char     name[LEN_EXPOMIX_NAME];
  int8_t   offset;
  CurveRef curve;
}) ExpoData_v217;
#else
PACK(typedef struct {
  uint8_t  mode:2;         // 0=end, 1=pos, 2=neg, 3=both
  uint8_t  chn:4;
  uint8_t  curveMode:2;
  int8_t   swtch;
  uint16_t flightModes;
  int8_t   weight;
  char     name[LEN_EXPOMIX_NAME_218];
  int8_t   curveParam;
}) ExpoData_v216;
typedef ExpoData_v216 ExpoData_v217;
#endif

#if defined(PCBTARANIS)
PACK(typedef struct {
  int16_t min;
  int16_t max;
  int8_t  ppmCenter;
  int16_t offset:14;
  uint16_t symetrical:1;
  uint16_t revert:1;
  char name[LEN_CHANNEL_NAME];
  int8_t curve;
}) LimitData_v216;
#else
#define LimitData_v216 LimitData
#endif

#if defined(PCBTARANIS)
PACK(typedef struct {
  uint8_t  destCh;
  uint16_t flightModes;
  uint8_t  mltpx:2;         // multiplex method: 0 means +=, 1 means *=, 2 means :=
  uint8_t  carryTrim:1;
  uint8_t  spare1:5;
  int16_t  weight;
  int8_t   swtch;
  CurveRef curve;
  uint8_t  mixWarn:4;       // mixer warning
  uint8_t  spare2:4;
  uint8_t  delayUp;
  uint8_t  delayDown;
  uint8_t  speedUp;
  uint8_t  speedDown;
  uint8_t  srcRaw;
  int16_t  offset;
  char     name[LEN_EXPOMIX_NAME];
  uint8_t  spare3;
}) MixData_v216;
PACK(typedef struct {
  uint8_t  destCh;
  uint16_t flightModes:9;
  uint16_t mltpx:2;         // multiplex method: 0 means +=, 1 means *=, 2 means :=
  uint16_t carryTrim:1;
  uint16_t mixWarn:4;       // mixer warning
  int16_t  weight;
  uint32_t srcRaw:10;
  int32_t  offset:14;
  int32_t  swtch:8;
  CurveRef curve;
  uint8_t  delayUp;
  uint8_t  delayDown;
  uint8_t  speedUp;
  uint8_t  speedDown;
  char     name[LEN_EXPOMIX_NAME];
}) MixData_v217;
#else
PACK(typedef struct {
  uint8_t  destCh:5;
  uint8_t  mixWarn:3;         // mixer warning
  uint16_t flightModes;
  uint8_t  curveMode:1;
  uint8_t  noExpo:1;
  int8_t   carryTrim:3;
  uint8_t  mltpx:2;           // multiplex method: 0 means +=, 1 means *=, 2 means :=
  uint8_t  spare:1;
  int16_t  weight;
  int8_t   swtch;
  int8_t   curveParam;
  uint8_t  delayUp;
  uint8_t  delayDown;
  uint8_t  speedUp;
  uint8_t  speedDown;
  uint8_t  srcRaw;
  int16_t  offset;
  char     name[LEN_EXPOMIX_NAME_218];
}) MixData_v216;
typedef MixData MixData_v217;
#endif

PACK(typedef struct {
  int8_t   mode;            // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint16_t start;
  uint8_t  countdownBeep:2;
  uint8_t  minuteBeep:1;
  uint8_t  persistent:2;
  uint8_t  spare:3;
  uint16_t value;
}) TimerData_v216;

PACK(typedef struct {
  int32_t  mode:8;            // timer trigger source -> off, abs, stk, stk%, sw/!sw, !m_sw/!m_sw
  uint32_t start:24;
  int32_t  value:24;
  uint32_t countdownBeep:2;
  uint32_t minuteBeep:1;
  uint32_t persistent:2;
  uint32_t spare:3;
  char     name[LEN_TIMER_NAME_218];
}) TimerData_v217;

PACK(typedef struct {
  int16_t trim[NUM_STICKS];
  int8_t swtch;       // swtch of phase[0] is not used
  char name[LEN_FLIGHT_MODE_NAME_218];
  uint8_t fadeIn;
  uint8_t fadeOut;
  int16_t rotaryEncoders[1];
  gvar_t gvars[9];
}) FlightModeData_v216;

PACK(typedef struct { // Logical Switches data
  int8_t  v1;
  int16_t v2;
  int16_t v3;
  uint8_t func;
  uint8_t delay;
  uint8_t duration;
  int8_t  andsw;
}) LogicalSwitchData_v216;

PACK(typedef struct { // Logical Switches data
  uint16_t func:6;
  int16_t  v1:10;
  int16_t  v2;
  int16_t  v3;
  uint8_t  delay;
  uint8_t  duration;
  int8_t   andsw;
}) LogicalSwitchData_v217;

#if defined(PCBTARANIS)
PACK(typedef struct {
  int8_t  swtch;
  uint8_t func;
  PACK(union {
         PACK(struct {
                char name[8];
              }) play;

         PACK(struct {
                int16_t val;
                uint8_t mode;
                uint8_t param;
                int32_t spare2;
              }) all;

         PACK(struct {
                int32_t val1;
                int32_t val2;
              }) clear;
       });
  uint8_t active;
}) CustomFunctionData_v216;
#else
PACK(typedef struct {
  int8_t  swtch;
  uint8_t func;
  PACK(union {
    PACK(struct {
      char name[6];
    }) play;

    PACK(struct {
      int16_t val;
      uint8_t mode;
      uint8_t param;
      int16_t spare2;
    }) all;

    PACK(struct {
      int32_t val1;
      int16_t val2;
    }) clear;
  });
  uint8_t active;
}) CustomFunctionData_v216;
#endif

PACK(typedef struct {
  uint8_t    source;
  uint8_t    barMin;           // minimum for bar display
  uint8_t    barMax;           // ditto for max display (would usually = ratio)
}) FrSkyBarData_v216;

PACK(typedef struct {
  uint8_t    sources[NUM_LINE_ITEMS];
}) FrSkyLineData_v216;

typedef union {
  FrSkyBarData_v216  bars[4];
  FrSkyLineData_v216 lines[4];
} FrSkyScreenData_v216;


PACK(struct FrSkyChannelData_v216 {
       uint8_t unused[7];
     });


PACK(typedef struct {
  FrSkyChannelData_v216 channels[4];
  uint8_t usrProto; // Protocol in FrSky user data, 0=None, 1=FrSky hub, 2=WS HowHigh, 3=Halcyon
  uint8_t voltsSource:7;
  uint8_t altitudeDisplayed:1;
  int8_t blades;    // How many blades for RPMs, 0=2 blades
  uint8_t currentSource;
  uint8_t screensType; // 2bits per screen (None/Gauges/Numbers/Script)
  FrSkyScreenData_v216 screens[3];
  uint8_t varioSource;
  int8_t  varioCenterMax;
  int8_t  varioCenterMin;
  int8_t  varioMin;
  int8_t  varioMax;
  uint8_t rssiAlarms[2];
  uint16_t mAhPersistent:1;
  uint16_t storedMah:15;
  int8_t   fasOffset;
}) FrSkyData_v216;

PACK(typedef struct {
  char    file[10];
  char    name[10];
  int8_t  inputs[10];
}) ScriptData_v216;

PACK(typedef struct { // Swash Ring data
  uint8_t   invertELE:1;
  uint8_t   invertAIL:1;
  uint8_t   invertCOL:1;
  uint8_t   type:5;
  uint8_t   collectiveSource;
  uint8_t   value;
}) SwashRingData_v216;

PACK(typedef struct {
  int8_t  rfProtocol;
  uint8_t channelsStart;
  int8_t  channelsCount; // 0=8 channels
  uint8_t failsafeMode;
  int16_t failsafeChannels[MAX_OUTPUT_CHANNELS_218];
  int8_t  ppmDelay;
  int8_t  ppmFrameLength;
  uint8_t ppmPulsePol;
}) ModuleData_v216;

PACK(typedef struct {
  uint8_t type:4;
  int8_t  rfProtocol:4;
  uint8_t channelsStart;
  int8_t  channelsCount; // 0=8 channels
  union {
    struct {
      uint8_t failsafeMode:4;  // only 3 bits used
      uint8_t subType:3;
      uint8_t invertedSerial:1; // telemetry serial inverted from standard
    };
    struct {
      uint8_t failsafeMode:4;  // only 3 bits used
      uint8_t freq:2;
      uint8_t region:2;
    } r9m;
  };
  int16_t failsafeChannels[MAX_OUTPUT_CHANNELS_218];
  union {
    struct {
      int8_t  delay:6;
      uint8_t pulsePol:1;
      uint8_t outputType:1;    // false = open drain, true = push pull
      int8_t  frameLength;
    } ppm;
  };
}) ModuleData_v217;

#if defined(PCBTARANIS)
#define MODELDATA_EXTRA_216 \
  uint8_t externalModule; \
  uint8_t trainerMode; \
  ModuleData_v216 moduleData[NUM_MODULES+1]; \
  char curveNames[MAX_CURVES][6]; \
  ScriptData_v216 scriptsData[MAX_SCRIPTS]; \
  char inputNames[MAX_INPUTS][LEN_INPUT_NAME]; \
  uint8_t nPotsToWarn; \
  int8_t potsWarnPosition[NUM_POTS+NUM_SLIDERS]; \
  uint8_t spare[2];
#elif defined(PCBSKY9X)
#define MODELDATA_EXTRA_216 \
  uint8_t externalModule; \
  ModuleData_v216 moduleData[NUM_MODULES+1]; \
  uint8_t nPotsToWarn; \
  int8_t potsWarnPosition[NUM_POTS+NUM_SLIDERS]; \
  uint8_t rxBattAlarms[2];
#endif

#if defined(PCBTARANIS)
#define MODELDATA_EXTRA_217 \
  uint8_t spare:3; \
  uint8_t trainerMode:3; \
  uint8_t potsWarnMode:2; \
  ModuleData_v217 moduleData[NUM_MODULES+1]; \
  char curveNames[MAX_CURVES][6]; \
  ScriptData scriptsData[MAX_SCRIPTS]; \
  char inputNames[MAX_INPUTS][LEN_INPUT_NAME]; \
  uint8_t potsWarnEnabled; \
  int8_t potsWarnPosition[NUM_POTS+NUM_SLIDERS];
#else
#define MODELDATA_EXTRA_217 \
  uint8_t spare:6; \
  uint8_t potsWarnMode:2; \
  ModuleData_v217 moduleData[NUM_MODULES+1]; \
  uint8_t potsWarnEnabled; \
  int8_t potsWarnPosition[NUM_POTS+NUM_SLIDERS]; \
  uint8_t rxBattAlarms[2];
#endif

#if defined(PCBTARANIS) && LCD_W >= 212
PACK(typedef struct {
  char name[LEN_MODEL_NAME];
  uint8_t modelId;
  char bitmap[LEN_BITMAP_NAME];
}) ModelHeader_v216;
#else
PACK(typedef struct {
  char      name[LEN_MODEL_NAME_218];
  uint8_t   modelId;
}) ModelHeader_v216;
#endif

PACK(typedef struct {
  ModelHeader_v216 header;
  TimerData_v216 timers[2];
  uint8_t   telemetryProtocol:3;
  uint8_t   thrTrim:1;            // Enable Throttle Trim
  int8_t    spare2:4;
  int8_t    trimInc:3;            // Trim Increments
  uint8_t   disableThrottleWarning:1;
  uint8_t displayChecklist:1;
  uint8_t   extendedLimits:1;
  uint8_t   extendedTrims:1;
  uint8_t   throttleReversed:1;
  BeepANACenter beepANACenter;        // 1<<0->A1.. 1<<6->A7
  MixData_v216 mixData[MAX_MIXERS];
  LimitData_v216 limitData[MAX_OUTPUT_CHANNELS_218];
  ExpoData_v216  expoData[MAX_EXPOS_218];

  CurveHeader_v216 curves[MAX_CURVES_218];
  int8_t    points[MAX_CURVE_POINTS_218];

  LogicalSwitchData_v216 logicalSw[32];
  CustomFunctionData_v216 customFn[MAX_SPECIAL_FUNCTIONS_218];
  SwashRingData_v216 swashR;
  FlightModeData_v216 flightModeData[MAX_FLIGHT_MODES_218];

  uint8_t   thrTraceSrc;

  uint16_t switchWarningState;
  uint8_t  switchWarningEnable;

  GVarData gvars[MAX_GVARS];

  FrSkyData_v216 frsky;

  MODELDATA_EXTRA_216

}) ModelData_v216;

PACK(typedef struct {
  char    name[6];
  uint8_t popup:1;
  uint8_t spare:7;
}) GVarData_v217;

PACK(typedef struct {
  ModelHeader_v218 header;
  TimerData_v217 timers[MAX_TIMERS_218];
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
  MixData_v217 mixData[MAX_MIXERS_218];
  LimitData limitData[MAX_OUTPUT_CHANNELS_218];
  ExpoData_v217  expoData[MAX_EXPOS_218];

  CurveHeader_v216 curves[MAX_CURVES_218];
  int8_t    points[MAX_CURVE_POINTS_218];

  LogicalSwitchData_v217 logicalSw[MAX_LOGICAL_SWITCHES_218];
  CustomFunctionData_v216 customFn[MAX_SPECIAL_FUNCTIONS_218];
  SwashRingData swashR;
  FlightModeData_v216 flightModeData[MAX_FLIGHT_MODES_218];

  uint8_t thrTraceSrc;

  swarnstate_t  switchWarningState;
  swarnenable_t switchWarningEnable;

  GVarData_v217 gvars[MAX_GVARS_218];

  FrSkyTelemetryData_v217 frsky;
  RssiAlarmData rssiAlarms;

  MODELDATA_EXTRA_217

  TelemetrySensor_218 telemetrySensors[MAX_TELEMETRY_SENSORS_218];

  TARANIS_PCBX9E_FIELD(uint8_t toplcdTimer)
}) ModelData_v217;

int ConvertTelemetrySource_216_to_217(int source)
{
  // TELEM_TIMER3 added
//  if (source >= TELEM_TIMER3)
  //  source += 1;

  return source;
}

#if defined(PCBTARANIS)
int convertSwitch_216_to_217(int swtch)
{
  if (swtch < 0)
    return -convertSwitch_216_to_217(-swtch);

  if (swtch > SWSRC_SF0)
    swtch += 1;

  if (swtch > SWSRC_SH0)
    swtch += 1;

  return swtch;
}
#else
int convertSwitch_216_to_217(int swtch)
{
  return swtch;
}
#endif

int convertSwitch_217_to_218(int swtch)
{
  // 32 additional logical switches

  if (swtch < 0)
    return -convertSwitch_217_to_218(-swtch);

  if (swtch >= SWSRC_FIRST_LOGICAL_SWITCH+32)
    return swtch+32;

  return swtch;
}

int convertSource_216_to_217(int source)
{
#if defined(PCBX9E)
  // SI to SR switches added
  if (source >= MIXSRC_SI)
    source += 10;
#endif
  // Telemetry conversions
  if (source >= MIXSRC_FIRST_TELEM)
    source = 0;

  return source;
}

int convertSource_217_to_218(int source)
{
#if defined(PCBTARANIS)
  if (source >= MIXSRC_FIRST_LOGICAL_SWITCH + 32)
    source += 32;
#endif

  return source;
}

int convertGVar_216_to_217(int value)
{
  if (value < -4096 + 9)
    value += 4096 - 1024;
  else if (value > 4095 - 9)
    value -= 4095 - 1023;
  return value;
}

PACK(typedef struct {
  uint8_t   version;
  uint16_t  variant;
  CalibData calib[NUM_STICKS+NUM_POTS+NUM_SLIDERS];
  uint16_t  chkSum;
  int8_t    currModel;
  uint8_t   contrast;
  uint8_t   vBatWarn;
  int8_t    txVoltageCalibration;
  int8_t    backlightMode;
  TrainerData trainer;
  uint8_t   view;            // index of view in main screen
  int8_t    buzzerMode:2;    // -2=quiet, -1=only alarms, 0=no keys, 1=all
  uint8_t   fai:1;
  int8_t    beepMode:2;      // -2=quiet, -1=only alarms, 0=no keys, 1=all
  uint8_t   alarmsFlash:1;
  uint8_t   disableMemoryWarning:1;
  uint8_t   disableAlarmWarning:1;
  uint8_t   stickMode:2;
  int8_t    timezone:5;
  uint8_t   adjustRTC:1;
  uint8_t   inactivityTimer;
  uint8_t   telemetryBaudrate:3;
  int8_t    splashMode:3;
  int8_t    hapticMode:2;    // -2=quiet, -1=only alarms, 0=no keys, 1=all
  int8_t    switchesDelay;
  uint8_t   lightAutoOff;
  uint8_t   templateSetup;   // RETA order for receiver channels
  int8_t    PPM_Multiplier;
  int8_t    hapticLength;
  uint8_t   reNavigation;
  N_TARANIS_FIELD(uint8_t stickReverse)
  int8_t    beepLength:3;
  int8_t    hapticStrength:3;
  uint8_t   gpsFormat:1;
  uint8_t   unexpectedShutdown:1;
  uint8_t   speakerPitch;
  int8_t    speakerVolume;
  int8_t    vBatMin;
  int8_t    vBatMax;

  uint8_t  backlightBright;
  int8_t   txCurrentCalibration;
  int8_t   spare;
  uint8_t  mAhWarn;
  uint16_t mAhUsed;
  uint32_t globalTimer;
  int8_t   temperatureCalib;
  uint8_t  bluetoothBaudrate;
  uint8_t  optrexDisplay;
  uint8_t  sticksGain;
  uint8_t  rotarySteps;
  uint8_t  countryCode;
  uint8_t  imperial;
  char     ttsLanguage[2];
  int8_t   beepVolume;
  int8_t   wavVolume;
  int8_t   varioVolume;
  int8_t   varioPitch;
  int8_t   varioRange;
  int8_t   varioRepeat;
  int8_t   backgroundVolume;

  TARANIS_FIELD(uint8_t auxSerialMode:6)
  TARANIS_FIELD(uint8_t slidersConfig:2)
  TARANIS_FIELD(uint8_t potsConfig)
  TARANIS_FIELD(uint8_t backlightColor)
  TARANIS_FIELD(swarnstate_t switchUnlockStates)
  TARANIS_FIELD(CustomFunctionData_v216 customFn[MAX_SPECIAL_FUNCTIONS_218])
  TARANIS_FIELD(swconfig_t switchConfig)
  TARANIS_FIELD(char switchNames[NUM_SWITCHES][LEN_SWITCH_NAME_218])
  TARANIS_FIELD(char anaNames[NUM_STICKS+NUM_POTS+NUM_SLIDERS][LEN_ANA_NAME_218])
  N_TARANIS_FIELD(CustomFunctionData_v216 customFn[MAX_SPECIAL_FUNCTIONS_218])

  TARANIS_PCBX9E_FIELD(uint8_t bluetoothEnable)
  TARANIS_PCBX9E_FIELD(char bluetoothName[LEN_BLUETOOTH_NAME_218])
}) RadioData_v216;

void convertRadioData_216_to_217(RadioData &settings)
{
  RadioData_v216 * settings_v216 = (RadioData_v216 *)&settings;
  settings_v216->version = 217;
#if defined(PCBTARANIS)
  settings_v216->potsConfig = 0x05; // S1 and S2 = pots with detent
  settings_v216->switchConfig = 0x00007bff; // 6x3POS, 1x2POS, 1xTOGGLE
#endif
}

void ConvertSpecialFunctions_217_to_218(CustomFunctionData_v218 * cf218, CustomFunctionData_v216 * cf216)
{
  for (int i=0; i<MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData_v218 & cf = cf218[i];
    memcpy(&cf, &cf216[i], sizeof(CustomFunctionData_v218));
    cf.swtch = convertSwitch_217_to_218(cf216[i].swtch);
    cf.func = cf216[i].func;
    if (cf.func == FUNC_PLAY_VALUE || cf.func == FUNC_VOLUME || (IS_ADJUST_GV_FUNC(cf.func) && cf.all.mode == FUNC_ADJUST_GVAR_SOURCE)) {
      cf.all.val = convertSource_217_to_218(cf.all.val);
    }
  }
}

void convertRadioData_217_to_218(RadioData &settings)
{
  RadioData_v216 settings_v217 = (RadioData_v216 &)settings;

  settings.version = 218;
#if !defined(PCBTARANIS)
  settings.stickReverse = settings_v217.stickReverse;
#endif
  settings.beepLength = settings_v217.beepLength;
  settings.hapticStrength = settings_v217.hapticStrength;
  settings.gpsFormat = settings_v217.gpsFormat;
  settings.unexpectedShutdown = settings_v217.unexpectedShutdown;
  settings.speakerPitch = settings_v217.speakerPitch;
  settings.speakerVolume = settings_v217.speakerVolume;
  settings.vBatMin = settings_v217.vBatMin;
  settings.vBatMax = settings_v217.vBatMax;
  settings.backlightBright = settings_v217.backlightBright;
  settings.globalTimer = settings_v217.globalTimer;
  settings.bluetoothBaudrate = settings_v217.bluetoothBaudrate;
  settings.countryCode = settings_v217.countryCode;
  settings.imperial = settings_v217.imperial;
  settings.ttsLanguage[0] = settings_v217.ttsLanguage[0];
  settings.ttsLanguage[1] = settings_v217.ttsLanguage[1];
  settings.beepVolume = settings_v217.beepVolume;
  settings.wavVolume = settings_v217.wavVolume;
  settings.varioVolume = settings_v217.varioVolume;
  settings.backgroundVolume = settings_v217.backgroundVolume;
  settings.varioPitch = settings_v217.varioPitch;
  settings.varioRange = settings_v217.varioRange;
  settings.varioRepeat = settings_v217.varioRepeat;
  ConvertSpecialFunctions_217_to_218((CustomFunctionData_v218 *)settings.customFn, settings_v217.customFn);

#if defined(PCBTARANIS)
  settings.auxSerialMode = settings_v217.auxSerialMode;
  settings.slidersConfig = settings_v217.slidersConfig;
  settings.potsConfig = settings_v217.potsConfig;
  settings.backlightColor = settings_v217.backlightColor;
  settings.switchUnlockStates = settings_v217.switchUnlockStates;
  settings.switchConfig = settings_v217.switchConfig;
  memcpy(settings.switchNames, settings_v217.switchNames, sizeof(settings.switchNames));
  memcpy(settings.anaNames, settings_v217.anaNames, sizeof(settings.anaNames));
#endif

#if defined(PCBX9E)
  memcpy(settings.bluetoothName, settings_v217.bluetoothName, sizeof(settings.bluetoothName));
#endif

#if defined(PCBSKY9X)
  settings.txCurrentCalibration = settings_v217.txCurrentCalibration;
  settings.mAhWarn = settings_v217.mAhWarn;
  settings.mAhUsed = settings_v217.mAhUsed;
  settings.temperatureCalib = settings_v217.temperatureCalib;
  settings.optrexDisplay = settings_v217.optrexDisplay;
  settings.sticksGain = settings_v217.sticksGain;
  settings.rotarySteps = settings_v217.rotarySteps;
#endif
}

void convertModelData_216_to_217(ModelData &model)
{
  // Timer3 added
  // 32bits Timers
  // MixData reduction
  // PPM center range
  // Telemetry custom screens

  static_assert(sizeof(ModelData_v216) <= sizeof(ModelData), "ModelData size has been reduced");

  ModelData_v216 oldModel;
  memcpy(&oldModel, &model, sizeof(oldModel));
  ModelData_v217 & newModel = (ModelData_v217 &)model;
  memset(&newModel, 0, sizeof(ModelData_v217));

  char name[LEN_MODEL_NAME+1];
  zchar2str(name, oldModel.header.name, LEN_MODEL_NAME);
  TRACE("Model %s conversion from v216 to v217", name);

  newModel.header.modelId[0] = oldModel.header.modelId;
  memcpy(newModel.header.name, oldModel.header.name, LEN_MODEL_NAME);
#if defined(PCBTARANIS) && LCD_W >= 212
  memcpy(newModel.header.bitmap, oldModel.header.bitmap, LEN_BITMAP_NAME);
#endif

  for (uint8_t i=0; i<2; i++) {
    TimerData_v217 & timer = newModel.timers[i];
    if (oldModel.timers[i].mode >= 5)
      timer.mode = 5 + convertSwitch_216_to_217(oldModel.timers[i].mode - 5 + 1) - 1;
    else
      timer.mode = convertSwitch_216_to_217(oldModel.timers[i].mode);
    timer.start = oldModel.timers[i].start;
    timer.countdownBeep = oldModel.timers[i].countdownBeep;
    timer.minuteBeep = oldModel.timers[i].minuteBeep;
    timer.persistent = oldModel.timers[i].persistent;
    timer.value = oldModel.timers[i].value;
  }
  newModel.telemetryProtocol = oldModel.telemetryProtocol;
  newModel.thrTrim = oldModel.thrTrim;
  newModel.trimInc = oldModel.trimInc;
  newModel.disableThrottleWarning = oldModel.disableThrottleWarning;
  newModel.displayChecklist = oldModel.displayChecklist;
  newModel.extendedLimits = oldModel.extendedLimits;
  newModel.extendedTrims = oldModel.extendedTrims;
  newModel.throttleReversed = oldModel.throttleReversed;
  newModel.beepANACenter = oldModel.beepANACenter;
  for (int i=0; i<MAX_MIXERS; i++) {
    newModel.mixData[i].destCh = oldModel.mixData[i].destCh;
    newModel.mixData[i].flightModes = oldModel.mixData[i].flightModes;
    newModel.mixData[i].mltpx = oldModel.mixData[i].mltpx;
    newModel.mixData[i].carryTrim = oldModel.mixData[i].carryTrim;
    newModel.mixData[i].mixWarn = oldModel.mixData[i].mixWarn;
    newModel.mixData[i].weight = convertGVar_216_to_217(oldModel.mixData[i].weight);
    newModel.mixData[i].swtch = convertSwitch_216_to_217(oldModel.mixData[i].swtch);
#if defined(PCBTARANIS)
    newModel.mixData[i].curve = oldModel.mixData[i].curve;
#else
    // TODO newModel.mixData[i].curveMode = oldModel.mixData[i].curveMode;
    // TODO newModel.mixData[i].noExpo = oldModel.mixData[i].noExpo;
    // TODO newModel.mixData[i].curveParam = oldModel.mixData[i].curveParam;
#endif
    newModel.mixData[i].delayUp = oldModel.mixData[i].delayUp;
    newModel.mixData[i].delayDown = oldModel.mixData[i].delayDown;
    newModel.mixData[i].speedUp = oldModel.mixData[i].speedUp;
    newModel.mixData[i].speedDown = oldModel.mixData[i].speedDown;
    newModel.mixData[i].srcRaw = convertSource_216_to_217(oldModel.mixData[i].srcRaw);
    newModel.mixData[i].offset = convertGVar_216_to_217(oldModel.mixData[i].offset);
    memcpy(newModel.mixData[i].name, oldModel.mixData[i].name, sizeof(newModel.mixData[i].name));
  }
  for (int i=0; i<MAX_OUTPUT_CHANNELS; i++) {
#if defined(PCBTARANIS)
    newModel.limitData[i].min = convertGVar_216_to_217(oldModel.limitData[i].min);
    newModel.limitData[i].max = convertGVar_216_to_217(oldModel.limitData[i].max);
    newModel.limitData[i].offset = convertGVar_216_to_217(oldModel.limitData[i].offset);
    newModel.limitData[i].ppmCenter = oldModel.limitData[i].ppmCenter;
    newModel.limitData[i].symetrical = oldModel.limitData[i].symetrical;
    newModel.limitData[i].revert = oldModel.limitData[i].revert;
    newModel.limitData[i].curve = oldModel.limitData[i].curve;
    memcpy(newModel.limitData[i].name, oldModel.limitData[i].name, sizeof(newModel.limitData[i].name));
#else
    newModel.limitData[i] = oldModel.limitData[i];
#endif
  }
  for (int i=0; i<MAX_EXPOS; i++) {
#if defined(PCBTARANIS)
    newModel.expoData[i].srcRaw = convertSource_216_to_217(oldModel.expoData[i].srcRaw);
    newModel.expoData[i].scale = oldModel.expoData[i].scale;
    newModel.expoData[i].carryTrim = oldModel.expoData[i].carryTrim;
    newModel.expoData[i].curve = oldModel.expoData[i].curve;
    newModel.expoData[i].offset = oldModel.expoData[i].offset;
#else
    newModel.expoData[i].curveMode = oldModel.expoData[i].curveMode;
    newModel.expoData[i].curveParam = oldModel.expoData[i].curveParam;
#endif
    newModel.expoData[i].chn = oldModel.expoData[i].chn;
    newModel.expoData[i].swtch = convertSwitch_216_to_217(oldModel.expoData[i].swtch);
    newModel.expoData[i].flightModes = oldModel.expoData[i].flightModes;
    newModel.expoData[i].weight = oldModel.expoData[i].weight;
    newModel.expoData[i].mode = oldModel.expoData[i].mode;
    memcpy(newModel.expoData[i].name, oldModel.expoData[i].name, sizeof(newModel.expoData[i].name));
  }
  memcpy(newModel.curves, oldModel.curves, sizeof(newModel.curves));
  memcpy(newModel.points, oldModel.points, sizeof(newModel.points));
  for (int i=0; i<32; i++) {
    LogicalSwitchData_v217 & sw = newModel.logicalSw[i];
    sw.func = oldModel.logicalSw[i].func;
    sw.v1 = oldModel.logicalSw[i].v1;
    sw.v2 = oldModel.logicalSw[i].v2;
    sw.v3 = oldModel.logicalSw[i].v3;
    sw.delay = oldModel.logicalSw[i].delay;
    sw.duration = oldModel.logicalSw[i].duration;
    uint8_t cstate = lswFamily(sw.func);
    if (cstate == LS_FAMILY_OFS || cstate == LS_FAMILY_COMP || cstate == LS_FAMILY_DIFF) {
      sw.v1 = convertSource_216_to_217((uint8_t)sw.v1);
      if (cstate == LS_FAMILY_COMP) {
        sw.v2 = convertSource_216_to_217((uint8_t)sw.v2);
      }
    }
    else if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
      sw.v1 = convertSwitch_216_to_217(sw.v1);
      sw.v2 = convertSwitch_216_to_217(sw.v2);
    }
    else if (cstate == LS_FAMILY_EDGE) {
      sw.v1 = convertSwitch_216_to_217(sw.v1);
    }
    sw.andsw = convertSwitch_216_to_217(sw.andsw);
  }
  for (int i=0; i<MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData_v216 & fn = newModel.customFn[i];
    fn = oldModel.customFn[i];
    fn.swtch = convertSwitch_216_to_217(fn.swtch);
    if (fn.func == FUNC_PLAY_VALUE || fn.func == FUNC_VOLUME || (IS_ADJUST_GV_FUNC(fn.func) && fn.all.mode == FUNC_ADJUST_GVAR_SOURCE)) {
      fn.all.val = convertSource_216_to_217(fn.all.val);
    }
  }

  newModel.swashR.collectiveSource = convertSource_216_to_217(newModel.swashR.collectiveSource);
  // TODO other fields

  for (int i=0; i<MAX_FLIGHT_MODES; i++) {
    newModel.flightModeData[i] = oldModel.flightModeData[i];
    newModel.flightModeData[i].swtch = convertSwitch_216_to_217(oldModel.flightModeData[i].swtch);
  }

  newModel.thrTraceSrc = oldModel.thrTraceSrc;
  newModel.switchWarningState = oldModel.switchWarningState;
  newModel.switchWarningEnable = oldModel.switchWarningEnable;
  memcpy(newModel.gvars, oldModel.gvars, sizeof(newModel.gvars));

  memcpy(&newModel.rssiAlarms, &oldModel.frsky.rssiAlarms, sizeof(newModel.rssiAlarms));

  for (int i=0; i<NUM_MODULES+1; i++) {
    newModel.moduleData[i].type = 0;
    newModel.moduleData[i].rfProtocol = oldModel.moduleData[i].rfProtocol;
    newModel.moduleData[i].channelsStart = oldModel.moduleData[i].channelsStart;
    newModel.moduleData[i].channelsCount = oldModel.moduleData[i].channelsCount;
    newModel.moduleData[i].failsafeMode = oldModel.moduleData[i].failsafeMode + 1;
    for (int j=0; j<MAX_OUTPUT_CHANNELS; j++) {
      newModel.moduleData[i].failsafeChannels[j] = oldModel.moduleData[i].failsafeChannels[j];
    }
    newModel.moduleData[i].ppm.delay = oldModel.moduleData[i].ppmDelay;
    newModel.moduleData[i].ppm.frameLength = oldModel.moduleData[i].ppmFrameLength;
    newModel.moduleData[i].ppm.pulsePol = oldModel.moduleData[i].ppmPulsePol;
  }

#if defined(PCBTARANIS)
  newModel.moduleData[INTERNAL_MODULE].type = MODULE_TYPE_XJT_PXX1;
#endif
  newModel.moduleData[EXTERNAL_MODULE].type = oldModel.externalModule;

#if defined(PCBTARANIS)
  newModel.trainerMode = oldModel.trainerMode;
  // TODO memcpy(newModel.scriptsData, oldModel.scriptsData, sizeof(newModel.scriptsData));
  memcpy(newModel.curveNames, oldModel.curveNames, sizeof(newModel.curveNames));
  memcpy(newModel.inputNames, oldModel.inputNames, sizeof(newModel.inputNames));
#endif
  newModel.potsWarnMode = oldModel.nPotsToWarn >> 6;
  newModel.potsWarnEnabled = oldModel.nPotsToWarn & 0x1f;
  memcpy(newModel.potsWarnPosition, oldModel.potsWarnPosition, sizeof(newModel.potsWarnPosition));
}

void convertModelData_217_to_218(ModelData &model)
{
  static_assert(sizeof(ModelData_v217) <= sizeof(ModelData), "ModelData size has been reduced");

  ModelData_v217 oldModel;
  memcpy(&oldModel, &model, sizeof(oldModel));
  ModelData_v218 & newModel = (ModelData_v218 &) model;
  memset(&newModel, 0, sizeof(ModelData_v218));

  char name[LEN_MODEL_NAME+1];
  zchar2str(name, oldModel.header.name, LEN_MODEL_NAME);
  TRACE("Model %s conversion from v217 to v218", name);

  newModel.header = oldModel.header;
  for (uint8_t i=0; i<MAX_TIMERS; i++) {
    if (oldModel.timers[i].mode >= 5)
      newModel.timers[i].mode = 5 + convertSwitch_217_to_218(oldModel.timers[i].mode - 5 + 1) - 1;
    else
      newModel.timers[i].mode = convertSwitch_217_to_218(oldModel.timers[i].mode);
    if (oldModel.timers[i].mode)
      TRACE("timer mode %d => %d", oldModel.timers[i].mode, newModel.timers[i].mode);
    newModel.timers[i].start = oldModel.timers[i].start;
    newModel.timers[i].value = oldModel.timers[i].value;
    newModel.timers[i].countdownBeep = oldModel.timers[i].countdownBeep;
    newModel.timers[i].minuteBeep = oldModel.timers[i].minuteBeep;
    newModel.timers[i].persistent = oldModel.timers[i].persistent;
    memcpy(newModel.timers[i].name, oldModel.timers[i].name, sizeof(newModel.timers[i].name));
  }
  newModel.telemetryProtocol = oldModel.telemetryProtocol;
  newModel.thrTrim = oldModel.thrTrim;
  newModel.noGlobalFunctions = oldModel.noGlobalFunctions;
  newModel.displayTrims = oldModel.displayTrims;
  newModel.ignoreSensorIds = oldModel.ignoreSensorIds;
  newModel.trimInc = oldModel.trimInc;
  newModel.disableThrottleWarning = oldModel.disableThrottleWarning;
  newModel.displayChecklist = oldModel.displayChecklist;
  newModel.extendedLimits = oldModel.extendedLimits;
  newModel.extendedTrims = oldModel.extendedTrims;
  newModel.throttleReversed = oldModel.throttleReversed;
  newModel.beepANACenter = oldModel.beepANACenter;
  for (int i=0; i<MAX_MIXERS; i++) {
    newModel.mixData[i].destCh = oldModel.mixData[i].destCh;
    newModel.mixData[i].flightModes = oldModel.mixData[i].flightModes;
    newModel.mixData[i].mltpx = oldModel.mixData[i].mltpx;
    newModel.mixData[i].carryTrim = oldModel.mixData[i].carryTrim;
    newModel.mixData[i].mixWarn = oldModel.mixData[i].mixWarn;
    newModel.mixData[i].weight = oldModel.mixData[i].weight;
    newModel.mixData[i].swtch = convertSwitch_217_to_218(oldModel.mixData[i].swtch);
#if defined(PCBTARANIS)
    newModel.mixData[i].curve = oldModel.mixData[i].curve;
#else
    // newModel.mixData[i].curveMode = oldModel.mixData[i].curveMode;
    // newModel.mixData[i].noExpo = oldModel.mixData[i].noExpo;
    // newModel.mixData[i].curveParam = oldModel.mixData[i].curveParam;
#endif
    newModel.mixData[i].delayUp = oldModel.mixData[i].delayUp;
    newModel.mixData[i].delayDown = oldModel.mixData[i].delayDown;
    newModel.mixData[i].speedUp = oldModel.mixData[i].speedUp;
    newModel.mixData[i].speedDown = oldModel.mixData[i].speedDown;
    newModel.mixData[i].srcRaw = convertSource_217_to_218(oldModel.mixData[i].srcRaw);
    newModel.mixData[i].offset = oldModel.mixData[i].offset;
    memcpy(newModel.mixData[i].name, oldModel.mixData[i].name, sizeof(newModel.mixData[i].name));
  }
  for (int i=0; i<MAX_OUTPUT_CHANNELS; i++) {
    newModel.limitData[i] = oldModel.limitData[i];
#if defined(PCBTARANIS)
    if (newModel.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_XJT_PXX1 || newModel.moduleData[EXTERNAL_MODULE].type == MODULE_TYPE_XJT_PXX1) {
      newModel.limitData[i].ppmCenter = (oldModel.limitData[i].ppmCenter * 612) / 1024;
    }
#endif
  }
  for (int i=0; i<MAX_EXPOS; i++) {
#if defined(PCBTARANIS)
    newModel.expoData[i].srcRaw = convertSource_217_to_218(oldModel.expoData[i].srcRaw);
    newModel.expoData[i].scale = oldModel.expoData[i].scale;
    newModel.expoData[i].carryTrim = oldModel.expoData[i].carryTrim;
    newModel.expoData[i].curve = oldModel.expoData[i].curve;
    newModel.expoData[i].offset = oldModel.expoData[i].offset;
#else
    // TODO newModel.expoData[i].curveMode = oldModel.expoData[i].curveMode;
    // TODO newModel.expoData[i].curveParam = oldModel.expoData[i].curveParam;
#endif
    newModel.expoData[i].chn = oldModel.expoData[i].chn;
    newModel.expoData[i].swtch = convertSwitch_217_to_218(oldModel.expoData[i].swtch);
    newModel.expoData[i].flightModes = oldModel.expoData[i].flightModes;
    newModel.expoData[i].weight = oldModel.expoData[i].weight;
    newModel.expoData[i].mode = oldModel.expoData[i].mode;
    memcpy(newModel.expoData[i].name, oldModel.expoData[i].name, sizeof(newModel.expoData[i].name));
  }
  for (int i=0; i<MAX_CURVES; i++) {
#if defined(PCBTARANIS)
    newModel.curves[i].type = oldModel.curves[i].type;
    newModel.curves[i].smooth = oldModel.curves[i].smooth;
    newModel.curves[i].points = oldModel.curves[i].points;
    memcpy(newModel.curves[i].name, oldModel.curveNames[i], sizeof(newModel.curves[i].name));
#else
    // TODO newModel.curves[i] = oldModel.curves[i];
#endif
  }
  memcpy(newModel.points, oldModel.points, sizeof(newModel.points));
  for (int i=0; i<32; i++) {
    LogicalSwitchData_v218 & sw = newModel.logicalSw[i];
    sw.func = oldModel.logicalSw[i].func;
    sw.v1 = oldModel.logicalSw[i].v1;
    sw.v2 = oldModel.logicalSw[i].v2;
    sw.v3 = oldModel.logicalSw[i].v3;
    newModel.logicalSw[i].andsw = convertSwitch_217_to_218(oldModel.logicalSw[i].andsw);
    sw.delay = oldModel.logicalSw[i].delay;
    sw.duration = oldModel.logicalSw[i].duration;
    uint8_t cstate = lswFamily(sw.func);
    if (cstate == LS_FAMILY_OFS || cstate == LS_FAMILY_COMP || cstate == LS_FAMILY_DIFF) {
      sw.v1 = convertSource_217_to_218((uint8_t)sw.v1);
      if (cstate == LS_FAMILY_COMP) {
        sw.v2 = convertSource_217_to_218((uint8_t)sw.v2);
      }
    }
    else if (cstate == LS_FAMILY_BOOL || cstate == LS_FAMILY_STICKY) {
      sw.v1 = convertSwitch_217_to_218(sw.v1);
      sw.v2 = convertSwitch_217_to_218(sw.v2);
    }
    else if (cstate == LS_FAMILY_EDGE) {
      sw.v1 = convertSwitch_217_to_218(sw.v1);
    }
  }
  ConvertSpecialFunctions_217_to_218(newModel.customFn, oldModel.customFn);
  newModel.swashR = oldModel.swashR;
  for (int i=0; i<MAX_FLIGHT_MODES; i++) {
    memcpy(newModel.flightModeData[i].trim, oldModel.flightModeData[i].trim, sizeof(newModel.flightModeData[i].trim));
    memcpy(newModel.flightModeData[i].name, oldModel.flightModeData[i].name, sizeof(newModel.flightModeData[i].name));
    newModel.flightModeData[i].swtch = convertSwitch_217_to_218(oldModel.flightModeData[i].swtch);
    newModel.flightModeData[i].fadeIn = oldModel.flightModeData[i].fadeIn;
    newModel.flightModeData[i].fadeOut = oldModel.flightModeData[i].fadeOut;
#if defined(PCBSKY9X)
    memcpy(newModel.flightModeData[i].rotaryEncoders, oldModel.flightModeData[i].rotaryEncoders, sizeof(newModel.flightModeData[i].rotaryEncoders));
#endif
    memcpy(newModel.flightModeData[i].gvars, oldModel.flightModeData[i].gvars, sizeof(newModel.flightModeData[i].gvars));
  }
  newModel.thrTraceSrc = oldModel.thrTraceSrc;
  newModel.switchWarningState = oldModel.switchWarningState;
  newModel.switchWarningEnable = oldModel.switchWarningEnable;
  for (int i=0; i<MAX_GVARS; i++) {
    memcpy(newModel.gvars[i].name, oldModel.gvars[i].name, sizeof(newModel.gvars[i].name));
    newModel.gvars[i].popup = oldModel.gvars[i].popup;
  }
  newModel.frsky = oldModel.frsky;
  for (int i=0; i<MAX_TELEMETRY_SCREENS; i++) {
    if (((oldModel.frsky.screensType >> (2*i)) & 0x03) == TELEMETRY_SCREEN_TYPE_VALUES) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < NUM_LINE_ITEMS; k++) {
          newModel.frsky.screens[i].lines[j].sources[k] = convertSource_217_to_218(oldModel.frsky.screens[i].lines[j].sources[k]);
        }
      }
    }
    else if (((oldModel.frsky.screensType >> (2*i)) & 0x03) == TELEMETRY_SCREEN_TYPE_BARS) {
      for (int j = 0; j < 4; j++) {
        newModel.frsky.screens[i].bars[j].source = convertSource_217_to_218(oldModel.frsky.screens[i].bars[j].source);
      }
    }
  }
  for (int i=0; i<NUM_MODULES+1; i++) {
    memcpy(&newModel.moduleData[i], &oldModel.moduleData[i], sizeof(ModuleData_v218));
  }
#if defined(PCBTARANIS)
  newModel.trainerMode = oldModel.trainerMode;
  memcpy(newModel.scriptsData, oldModel.scriptsData, sizeof(newModel.scriptsData));
  memcpy(newModel.inputNames, oldModel.inputNames, sizeof(newModel.inputNames));
#endif
  newModel.potsWarnMode = oldModel.potsWarnMode;
  newModel.potsWarnEnabled = oldModel.potsWarnEnabled;
  memcpy(newModel.potsWarnPosition, oldModel.potsWarnPosition, sizeof(newModel.potsWarnPosition));
  for (uint8_t i=0; i<MAX_TELEMETRY_SENSORS_218; i++) {
    newModel.telemetrySensors[i] = oldModel.telemetrySensors[i];
    if (newModel.telemetrySensors[i].unit > UNIT_WATTS)
      newModel.telemetrySensors[i].unit += 1;
  }
#if defined(PCBX9E)
  newModel.toplcdTimer = oldModel.toplcdTimer;
#endif
}
