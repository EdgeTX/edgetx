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

// No include guards here, this file may be included many times in different namespaces
// i.e. BACKUP RAM Backup/Restore functions

#include <inttypes.h>
#include "board.h"
#include "dataconstants.h"
#include "definitions.h"
#include "edgetx_types.h"
#include "globals.h"
#include "serial.h"
#include "usb_joystick.h"
#include "input_mapping.h"
#include "debug.h"

#if defined(PCBTARANIS)
  #define N_TARANIS_FIELD(x)
  #define TARANIS_FIELD(x) x;
#else
  #define N_TARANIS_FIELD(x) x;
  #define TARANIS_FIELD(x)
#endif

#if defined(PCBX9E)
  #define TARANIS_PCBX9E_FIELD(x)       x;
#else
  #define TARANIS_PCBX9E_FIELD(x)
#endif

#if defined(PCBHORUS)
  #define N_HORUS_FIELD(x)
  #define HORUS_FIELD(x) x;
#else
  #define N_HORUS_FIELD(x) x;
  #define HORUS_FIELD(x)
#endif

#if defined(BACKUP)
  #define NOBACKUP(...)
#else
  #define NOBACKUP(...)                __VA_ARGS__
#endif

#include "storage/yaml/yaml_defs.h"

PACK(union SourceNumVal {
  struct {
    int16_t value:10;
    uint16_t isSource:1;
  };
  uint16_t rawValue:11;
});

inline uint16_t makeSourceNumVal(int16_t val, bool isSource = false)
{
  SourceNumVal v;
  v.value = val;
  v.isSource = isSource;
  return v.rawValue;
}

/*
 * Mixer structure
 */

PACK(struct CurveRef {
  uint16_t type:5;
  int16_t  value:11 CUST(r_sourceNumVal,w_sourceNumVal);
});

PACK(struct MixData {
  uint16_t destCh:5;
  int16_t  srcRaw:10 CUST(r_mixSrcRawEx,w_mixSrcRawEx); // srcRaw=0 means not used
  uint16_t carryTrim:1;
  uint16_t mixWarn:2;       // mixer warning
  uint16_t mltpx:2 ENUM(MixerMultiplex);
  uint16_t delayPrec:1;
  uint16_t speedPrec:1;
  uint16_t flightModes:9 CUST(r_flightModes, w_flightModes);
  uint16_t spare:1 SKIP;
  uint32_t weight:11 CUST(r_sourceNumVal,w_sourceNumVal);
  uint32_t offset:11 CUST(r_sourceNumVal,w_sourceNumVal);
  int32_t  swtch:10 CUST(r_swtchSrc,w_swtchSrc);
  CurveRef curve;
  uint8_t  delayUp;
  uint8_t  delayDown;
  uint8_t  speedUp;
  uint8_t  speedDown;
  NOBACKUP(char name[LEN_EXPOMIX_NAME]);
});

/*
 * Expo/Input structure
 */

PACK(struct ExpoData {
  uint16_t mode:2;
  uint16_t scale:14;
  CUST_ATTR(carryTrim, r_carryTrim, nullptr); //pre 2.9
  int16_t  trimSource:6;
  int16_t  srcRaw:10 ENUM(MixSources) CUST(r_mixSrcRawEx,w_mixSrcRawEx);
  uint32_t weight:11 CUST(r_sourceNumVal,w_sourceNumVal);
  uint32_t offset:11 CUST(r_sourceNumVal,w_sourceNumVal);
  int32_t  swtch:10 CUST(r_swtchSrc,w_swtchSrc);
  CurveRef curve;
  uint16_t chn:5;
  uint16_t flightModes:9 CUST(r_flightModes, w_flightModes);
  uint16_t spare:2 SKIP;
  NOBACKUP(char name[LEN_EXPOMIX_NAME]);
});

/*
 * Limit structure
 */

PACK(struct LimitData {
  int32_t min:11 CUST(in_read_weight,in_write_weight);
  int32_t max:11 CUST(in_read_weight,in_write_weight);
  int32_t ppmCenter:10; // TODO can be reduced to 8 bits
  int16_t offset:11 CUST(in_read_weight,in_write_weight);
  uint16_t symetrical:1;
  uint16_t revert:1;
  uint16_t spare:3 SKIP;
  int8_t curve;
  NOBACKUP(char name[LEN_CHANNEL_NAME]);
});

/*
 * LogicalSwitch structure
 */

PACK(struct LogicalSwitchData {
  uint8_t  func ENUM(LogicalSwitchesFunctions);
  CUST_ATTR(def,r_logicSw,w_logicSw);
  int32_t  v1:10 SKIP;
  int32_t  v3:10 SKIP;
  int32_t  andsw:10 CUST(r_swtchSrc,w_swtchSrc);
  uint32_t lsPersist:1;
  uint32_t lsState:1;
  int16_t  v2 SKIP;
  uint8_t  delay;
  uint8_t  duration;
});

/*
 * SpecialFunction structure
 */

PACK(struct CustomFunctionData {
  int16_t  swtch:10 CUST(r_swtchSrc,w_swtchSrc);
  uint16_t func:6 ENUM(Functions); // TODO: 6 bits for Functions?
  CUST_ATTR(def,r_customFn,w_customFn);
  PACK(union {
    NOBACKUP(PACK(struct {
      char name[LEN_FUNCTION_NAME];
    }) play);

    PACK(struct {
      int16_t val;
      uint8_t mode;
      uint8_t param;
      int32_t val2;
    }) all;

    NOBACKUP(PACK(struct {
      int32_t val1;
      int32_t val2;
    }) clear);
  }) NAME(fp) SKIP;
  uint8_t active : 1 SKIP;
  int8_t repeat:7 SKIP;

  bool isEmpty() const
  {
    return swtch == 0;
  }
});

/*
 * FlightMode structure
 */

PACK(struct trim_t {
  int16_t  value:11;
  uint16_t mode:5;
});

PACK(struct FlightModeData {
  trim_t trim[MAX_TRIMS];
  NOBACKUP(char name[LEN_FLIGHT_MODE_NAME]);
  // swtch of phase[0] is not used
  int16_t swtch:10 ENUM(SwitchSources) CUST(r_swtchSrc,w_swtchSrc);
  int16_t spare:6 SKIP;
  uint8_t fadeIn;
  uint8_t fadeOut;
  gvar_t gvars[MAX_GVARS] FUNC(gvar_is_active);
});

/*
 * Curve structure
 */

PACK(struct CurveHeader {
  uint8_t type:1;
  uint8_t smooth:1;
  int8_t  points:6;   // describes number of points - 5
  NOBACKUP(char name[LEN_CURVE_NAME]);
});

/*
 * GVar structure
 */

PACK(struct GVarData {
  NOBACKUP(char name[LEN_GVAR_NAME]);
  uint32_t min:12;
  uint32_t max:12;
  uint32_t popup:1;
  uint32_t prec:1;
  uint32_t unit:2;
  uint32_t spare:4 SKIP;
});

/*
 * Timer structure
 */

PACK(struct TimerData {
  uint32_t start:22;
  int32_t  swtch:10 CUST(r_swtchSrc,w_swtchSrc);
  int32_t  value:22;
  uint32_t mode:3 ENUM(TimerModes);
  uint32_t countdownBeep:2;
  uint32_t minuteBeep:1;
  uint32_t persistent:2;
  int32_t  countdownStart:2;
  uint8_t  showElapsed:1; 
  uint8_t  extraHaptic:1;
  uint8_t  spare:6 SKIP;
  NOBACKUP(char name[LEN_TIMER_NAME]);
});

/*
 * Swash Ring structure
 */

PACK(struct SwashRingData {
  uint8_t   type ENUM(SwashType);
  uint8_t   value;
  uint8_t   collectiveSource CUST(r_mixSrcRaw,w_mixSrcRaw);
  uint8_t   aileronSource CUST(r_mixSrcRaw,w_mixSrcRaw);
  uint8_t   elevatorSource CUST(r_mixSrcRaw,w_mixSrcRaw);
  int8_t    collectiveWeight;
  int8_t    aileronWeight;
  int8_t    elevatorWeight;
});

#if MAX_SCRIPTS > 0
union ScriptDataInput {
  int16_t value;
  source_t source CUST(r_mixSrcRaw,w_mixSrcRaw);
} FUNC(select_script_input);

PACK(struct ScriptData {
  char            file[LEN_SCRIPT_FILENAME];
  char            name[LEN_SCRIPT_NAME];
  ScriptDataInput inputs[MAX_SCRIPT_INPUTS];
});
#endif

/*
 * Frsky Telemetry structure (legacy read-only)
 */
PACK(struct RssiAlarmData {
  // int8_t disabled:1;
  CUST_ATTR(disabled,r_rssiDisabled,nullptr);
  // int8_t warning:6; + 45
  CUST_ATTR(warning,r_rssiWarning,nullptr);
  // int8_t critical:6; + 42
  CUST_ATTR(critical,r_rssiCritical,nullptr);
});

PACK(struct RFAlarmData {
  int8_t warning;
  int8_t critical;
});

typedef int16_t ls_telemetry_value_t;

#if !defined(COLORLCD)
PACK(struct FrSkyBarData {
  source_t source CUST(r_mixSrcRaw,w_mixSrcRaw);
  ls_telemetry_value_t barMin;           // minimum for bar display
  ls_telemetry_value_t barMax;           // ditto for max display (would usually = ratio)
});

// This is used to be able to use
// custom read/write functions in
// an array made of typdef'ed literal types
// (YAML generator)
#if defined(YAML_GENERATOR)
PACK(struct LineDataSource {
  source_t val CUST(r_mixSrcRaw,w_mixSrcRaw);
});
PACK(struct FrSkyLineData {
  LineDataSource sources[NUM_LINE_ITEMS];
});
#else
// This here is the real structure used at run-time
PACK(struct FrSkyLineData {
  source_t sources[NUM_LINE_ITEMS];
});
#endif

#if defined(PCBTARANIS)
PACK(struct TelemetryScriptData {
  char    file[LEN_SCRIPT_FILENAME];
  int16_t inputs[MAX_TELEM_SCRIPT_INPUTS];
});
#endif

#if defined(YAML_GENERATOR)
union TelemetryScreenData_u {
  FrSkyBarData  bars[4];
  FrSkyLineData lines[4];
#if defined(PCBTARANIS)
  TelemetryScriptData script;
#endif
};
PACK(struct TelemetryScreenData {
  CUST_ATTR(type,r_tele_screen_type,w_tele_screen_type);
  TelemetryScreenData_u u FUNC(select_tele_screen_data);
});
#else
union TelemetryScreenData {
  FrSkyBarData  bars[4];
  FrSkyLineData lines[4];
#if defined(PCBTARANIS)
  TelemetryScriptData script;
#endif
} FUNC(select_tele_screen_data);
#endif

#endif

PACK(struct VarioData {
  uint8_t source:7 CUST(r_tele_sensor,w_tele_sensor); // telemetry sensor idx + 1
  uint8_t centerSilent:1;
  int8_t  centerMax;
  int8_t  centerMin;
  int8_t  min;
  int8_t  max;
});

/*
 * Telemetry Sensor structure
 */

#define TELEMETRY_ENDPOINT_NONE    0xFF
#define TELEMETRY_ENDPOINT_SPORT   0x07

PACK(struct TelemetrySensor {
  union {
    uint16_t id;  // data identifier, for FrSky we can reuse existing ones.
                  // Source unit is derived from type.
    NOBACKUP(uint16_t persistentValue);
  } NAME(id1) FUNC(select_id1);
  union {
    NOBACKUP(PACK(struct {
      uint8_t physID:5;
      uint8_t rxIndex:3; // 1 bit for module index, 2 bits for receiver index
    }) frskyInstance);
    uint8_t instance;
    NOBACKUP(uint8_t formula ENUM(TelemetrySensorFormula));
  } NAME(id2) FUNC(select_id2);
  NOBACKUP(char label[TELEM_LABEL_LEN]); // user defined label
  uint8_t  subId;
  uint8_t  type:1 ENUM(TelemetrySensorType); // 0=custom / 1=calculated
                   // user can choose what unit to display each value in
  uint8_t  spare1:1 SKIP;
  uint8_t  unit:6;
  uint8_t  prec:2;
  uint8_t  autoOffset:1;
  uint8_t  filter:1;
  uint8_t  logs:1;
  uint8_t  persistent:1;
  uint8_t  onlyPositive:1;
  uint8_t  spare2:1 SKIP;
  union {
    NOBACKUP(PACK(struct {
      uint16_t ratio;
      int16_t  offset;
    }) custom);
    NOBACKUP(PACK(struct {
      uint8_t source;
      uint8_t index;
      uint16_t spare SKIP;
    }) cell);
    NOBACKUP(PACK(struct {
      int8_t sources[4];
    }) calc);
    NOBACKUP(PACK(struct {
      uint8_t source;
      uint8_t spare[3] SKIP;
    }) consumption);
    NOBACKUP(PACK(struct {
      uint8_t gps;
      uint8_t alt;
      uint16_t spare SKIP;
    }) dist);
    uint32_t param;
  } NAME(cfg) FUNC(select_sensor_cfg);
  NOBACKUP(
    void init(const char *label, uint8_t unit=UNIT_RAW, uint8_t prec=0);
    void init(uint16_t id);
    bool isAvailable() const;
    int32_t getValue(int32_t value, uint8_t unit, uint8_t prec) const;
    bool isConfigurable() const;
    bool isPrecConfigurable() const;
    int32_t getPrecMultiplier() const;
    int32_t getPrecDivisor() const;
    bool isSameInstance(TelemetryProtocol protocol, uint8_t instance);
  );
});

/*
 * Trainer module structure
 */

PACK(struct TrainerModuleData {
  uint8_t mode CUST(r_trainerMode,w_trainerMode);
  uint8_t channelsStart;
  int8_t  channelsCount; // 0=8 channels
  int8_t frameLength;
  int8_t  delay:6;
  uint8_t pulsePol:1;
  uint8_t spare2:1 SKIP;
});

/*
 * Module structure
 */

PACK(struct PpmModule {
  int8_t  delay:6;
  uint8_t pulsePol:1;
  uint8_t outputType:1;    // false = open drain, true = push pull
  int8_t  frameLength;
});

PACK(struct ModuleData {
  uint8_t type ENUM(ModuleType) CUST(r_moduleType, w_moduleType);
  CUST_ATTR(subType,r_modSubtype,w_modSubtype);
  uint8_t channelsStart;
  int8_t  channelsCount CUST(r_channelsCount,w_channelsCount); // 0=8 channels
  uint8_t failsafeMode:4 ENUM(FailsafeModes);  // only 3 bits used
  uint8_t subType:4 SKIP;

  union {
    uint8_t raw[PXX2_MAX_RECEIVERS_PER_MODULE * PXX2_LEN_RX_NAME + 1];
    NOBACKUP(PpmModule ppm);
    NOBACKUP(struct {
      uint8_t rfProtocol SKIP;
      uint8_t disableTelemetry:1;
      uint8_t disableMapping:1;
      uint8_t autoBindMode:1;
      uint8_t lowPowerMode:1;
      uint8_t receiverTelemetryOff:1;
      uint8_t receiverHigherChannels:1;
      uint8_t spare:2 SKIP;
      int8_t optionValue;
    } multi);
    NOBACKUP(struct {
      uint8_t power:2;                  // 0=10 mW, 1=100 mW, 2=500 mW, 3=1W
      uint8_t spare1:2 SKIP;
      uint8_t receiverTelemetryOff:1;     // false = receiver telem enabled
      uint8_t receiverHigherChannels:1;  // false = pwm out 1-8, true 9-16
      int8_t antennaMode:2;
      uint8_t spare2 SKIP;
    } pxx);
    NOBACKUP(struct {
      uint8_t spare1:6 SKIP;
      uint8_t noninverted:1;
      uint8_t spare2:1 SKIP;
      int8_t refreshRate;  // definition as framelength for ppm (* 5 + 225 = time in 1/10 ms)
    } sbus);
    NOBACKUP(struct {
      uint8_t receivers:7; // 4 bits spare
      uint8_t racingMode:1;
      char receiverName[PXX2_MAX_RECEIVERS_PER_MODULE][PXX2_LEN_RX_NAME];
    } pxx2);
    NOBACKUP(struct {
      uint8_t rx_id[4];
      uint8_t mode:3;
      uint8_t rfPower:1;
      uint8_t reserved:4;
      uint8_t rx_freq[2];

      void setDefault() {
        rx_id[0] = rx_id[1] = rx_id[2] = rx_id[3] = 0;
        mode = 3;
        rfPower = 0;
        rx_freq[0] = 50;
        rx_freq[1] = 0;
      }
    } flysky);
    NOBACKUP(struct {
      uint8_t emi:2;
      uint8_t telemetry:1;
      uint8_t phyMode:3;
      uint8_t reserved:2;
      uint8_t rfPower;
    } afhds3);
    NOBACKUP(struct {
      uint8_t raw12bits:1;
      uint8_t telemetryBaudrate:3;
      uint8_t spare1:4 SKIP;
    } ghost);
    NOBACKUP(PACK(struct {
      uint8_t telemetryBaudrate:3;
      uint8_t crsfArmingMode:1;
      uint8_t spare2:4 SKIP;
      int16_t crsfArmingTrigger:10 CUST(r_swtchSrc,w_swtchSrc);
      int16_t spare3:6;
    }) crsf);
    NOBACKUP(struct {
      uint8_t flags;
    } dsmp);
  } NAME(mod) FUNC(select_mod_type);

  NOBACKUP(inline uint8_t getChannelsCount() const
  {
    return channelsCount + 8;
  })
});

/*
 * Model structure
 */

#if LEN_BITMAP_NAME > 0
#define MODEL_HEADER_BITMAP_FIELD      NOBACKUP(char bitmap[LEN_BITMAP_NAME]);
#else
#define MODEL_HEADER_BITMAP_FIELD
#endif


PACK(struct ModelHeader {
  char      name[LEN_MODEL_NAME]; // must be first for eeLoadModelName
  uint8_t   modelId[NUM_MODULES];
  MODEL_HEADER_BITMAP_FIELD
#if defined(STORAGE_MODELSLIST)
  char      labels[LABELS_LENGTH];
#endif
});

// 2 bits per switch, max 32 switches
static_assert(sizeof(swconfig_t) >= (MAX_SWITCHES * 2 + 7) / 8,
              "MAX_SWITCHES must fit swconfig_t");

static_assert(sizeof(swarnstate_t) >= (MAX_SWITCHES * 2 + 7) / 8,
              "MAX_SWITCHES must fit swarnstate_t");

// pot config: 4 bits per pot
static_assert(sizeof(potconfig_t) * 8 >= ((MAX_POTS - 1) / 4) + 1,
              "MAX_POTS must fit potconfig_t");

// pot warning enabled: 1 bit per pot
static_assert(sizeof(potwarnen_t) * 8 >= MAX_POTS,
              "MAX_POTS must fit potwarnen_t");

#if defined(COLORLCD) && defined(BACKUP)
#define CUSTOM_SCREENS_DATA
#elif defined(COLORLCD)
#include "layout.h"
#include "topbar.h"
#define LAYOUT_ID_LEN 12
PACK(struct CustomScreenData {
  char LayoutId[LAYOUT_ID_LEN];
  LayoutPersistentData layoutData;
});
#define CUSTOM_SCREENS_DATA \
  NOBACKUP(CustomScreenData screenData[MAX_CUSTOM_SCREENS]); \
  NOBACKUP(TopBarPersistentData topbarData); \
  NOBACKUP(uint8_t topbarWidgetWidth[MAX_TOPBAR_ZONES]); \
  NOBACKUP(uint8_t view);
#else
#define CUSTOM_SCREENS_DATA \
  uint8_t screensType SKIP; /* 2bits per screen (None/Gauges/Numbers/Script) */ \
  TelemetryScreenData screens[MAX_TELEMETRY_SCREENS]; \
  uint8_t view;
#endif

#if defined(PCBX9D) || defined(PCBX9DP) || defined(PCBX9E)
  // telemetry sensor idx + 1
  #define TOPBAR_DATA \
    NOBACKUP(uint8_t voltsSource CUST(r_tele_sensor,w_tele_sensor)); \
    NOBACKUP(uint8_t altitudeSource CUST(r_tele_sensor,w_tele_sensor));
#else
  #define TOPBAR_DATA
#endif

#if defined(PCBHORUS) || defined(PCBTARANIS) || defined(PCBNV14) || defined(PCBPL18)
  #define SCRIPT_DATA \
    NOBACKUP(ScriptData scriptsData[MAX_SCRIPTS]);
#else
  #define SCRIPT_DATA
#endif

struct RGBLedColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    uint32_t getColor() {
      return ((r << 16) + (g << 8) + b);
    }

    void setColor(uint32_t color) {
      r = color >> 16;
      g = color >> 8;
      b = color;
    }
};

#if defined(FUNCTION_SWITCHES_RGB_LEDS)
  #define FUNCTION_SWITCHS_RGB_LEDS_FIELDS \
    RGBLedColor functionSwitchLedONColor[NUM_FUNCTIONS_SWITCHES]; \
    RGBLedColor functionSwitchLedOFFColor[NUM_FUNCTIONS_SWITCHES];
#else
  #define FUNCTION_SWITCHS_RGB_LEDS_FIELDS
#endif
#if defined(FUNCTION_SWITCHES)
  #define FUNCTION_SWITCHS_FIELDS \
    uint16_t functionSwitchConfig;  \
    uint16_t functionSwitchGroup; \
    uint16_t functionSwitchStartConfig; \
    uint8_t functionSwitchLogicalState; \
    char switchNames[NUM_FUNCTIONS_SWITCHES][LEN_SWITCH_NAME]; \
    FUNCTION_SWITCHS_RGB_LEDS_FIELDS
#else
  #define FUNCTION_SWITCHS_FIELDS
#endif

PACK(struct PartialModel {
  ModelHeader header;
  TimerData timers[MAX_TIMERS];
});

/*
 * USB Joystick channel structure
 */

PACK(struct USBJoystickChData {
  uint8_t mode:3 ENUM(USBJoystickCh);
  uint8_t inversion:1;
  uint8_t param:4;
  uint8_t btn_num:5;
  uint8_t switch_npos:3;

#if defined(USBJ_EX)
  NOBACKUP(
    uint8_t btnCount() {
      // Use one less joystick button for 2POS and 3POS switches for Companion mode
      if ((param == USBJOYS_BTN_MODE_COMPANION) && (switch_npos > 0) && (switch_npos < 3))
        return switch_npos;
      return switch_npos + 1;
    }
    uint8_t lastBtnNumNoCLip() {
      uint8_t last = btn_num + switch_npos;
      // Use one less joystick button for 2POS and 3POS switches for Companion mode
      if ((param == USBJOYS_BTN_MODE_COMPANION) && (switch_npos > 0) && (switch_npos < 3)) last -= 1;
      return last;
    }
    uint8_t lastBtnNum() {
      uint8_t last = lastBtnNumNoCLip();
      if (last >= USBJ_BUTTON_SIZE) {
        last = USBJ_BUTTON_SIZE - 1;
      }
      return last;
    }
  );
#endif
});

PACK(struct ModelData {
  CUST_ATTR(semver,nullptr,w_semver);
  ModelHeader header;
  TimerData timers[MAX_TIMERS];
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
  uint8_t   enableCustomThrottleWarning:1;
  uint8_t   disableTelemetryWarning:1;
  uint8_t   showInstanceIds:1;
  uint8_t   checklistInteractive:1;
#if defined(USE_HATS_AS_KEYS)
  NOBACKUP(uint8_t hatsMode:2 ENUM(HatsMode));
  uint8_t   spare3:2 SKIP;  // padding to 8-bit aligment
#else
  uint8_t   spare3:4 SKIP;  // padding to 8-bit aligment
#endif
  int8_t    customThrottleWarningPosition;
  BeepANACenter beepANACenter;
  MixData   mixData[MAX_MIXERS] NO_IDX;
  LimitData limitData[MAX_OUTPUT_CHANNELS];
  ExpoData  expoData[MAX_EXPOS] NO_IDX;

  CurveHeader curves[MAX_CURVES];
  int8_t    points[MAX_CURVE_POINTS];

  LogicalSwitchData logicalSw[MAX_LOGICAL_SWITCHES];
  CustomFunctionData customFn[MAX_SPECIAL_FUNCTIONS] FUNC(cfn_is_active);
  SwashRingData swashR FUNC(swash_is_active);
  FlightModeData flightModeData[MAX_FLIGHT_MODES] FUNC(fmd_is_active);

  NOBACKUP(uint8_t thrTraceSrc CUST(r_thrSrc,w_thrSrc));
  CUST_ATTR(switchWarningState, r_swtchWarn, nullptr);
  NOBACKUP(swarnstate_t switchWarning ARRAY(3, struct_swtchWarn, nullptr));

  GVarData gvars[MAX_GVARS];

  NOBACKUP(VarioData varioData);
  NOBACKUP(uint8_t rssiSource CUST(r_tele_sensor,w_tele_sensor));

  TOPBAR_DATA

#if defined(YAML_GENERATOR)
  RssiAlarmData rssiAlarms;
#endif
  NOBACKUP(RFAlarmData rfAlarms);

  uint8_t thrTrimSw:3;
  uint8_t potsWarnMode:2 ENUM(PotsWarnMode);
  NOBACKUP(uint8_t jitterFilter:2 ENUM(ModelOverridableEnable));
  uint8_t spare1:1 SKIP;

  ModuleData moduleData[NUM_MODULES];
  int16_t failsafeChannels[MAX_OUTPUT_CHANNELS];
  TrainerModuleData trainerData;

  SCRIPT_DATA

  NOBACKUP(char inputNames[MAX_INPUTS][LEN_INPUT_NAME]);
  NOBACKUP(potwarnen_t potsWarnEnabled);
  NOBACKUP(int8_t potsWarnPosition[MAX_POTS]);

  NOBACKUP(TelemetrySensor telemetrySensors[MAX_TELEMETRY_SENSORS];)

  TARANIS_PCBX9E_FIELD(uint8_t toplcdTimer)

  CUSTOM_SCREENS_DATA

  char modelRegistrationID[PXX2_LEN_REGISTRATION_ID];

  FUNCTION_SWITCHS_FIELDS

  uint8_t getThrottleStickTrimSource() const
  {
    // Makes Throttle the default (=0)
    auto thr = inputMappingGetThrottle();
    if (thrTrimSw == 0) {
      return MIXSRC_FIRST_TRIM + thr;
    } else if (thrTrimSw == thr) {
      return MIXSRC_FIRST_TRIM;
    } else {
      return MIXSRC_FIRST_TRIM + thrTrimSw;
    }
  }

  void setThrottleStickTrimSource(int16_t src)
  {
    auto thr = inputMappingGetThrottle();
    if (src == MIXSRC_FIRST_TRIM + thr) {
      thrTrimSw = 0;
    } else if (src == MIXSRC_FIRST_TRIM) {
      thrTrimSw = thr;
    } else {
      thrTrimSw = src - MIXSRC_FIRST_TRIM;
    }
  }

  NOBACKUP(uint8_t usbJoystickExtMode:1);
  NOBACKUP(uint8_t usbJoystickIfMode:3 ENUM(USBJoystickIfMode));
  NOBACKUP(uint8_t usbJoystickCircularCut:4);
  NOBACKUP(USBJoystickChData usbJoystickCh[USBJ_MAX_JOYSTICK_CHANNELS]);
  
  // Radio level tabs control (model settings)
#if defined(COLORLCD)
  uint8_t radioThemesDisabled:2 ENUM(ModelOverridableEnable);
#endif
  uint8_t radioGFDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t radioTrainerDisabled:2 ENUM(ModelOverridableEnable);
  // Model level tabs control (model setting)
  uint8_t modelHeliDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelFMDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelCurvesDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelGVDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelLSDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelSFDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelCustomScriptsDisabled:2 ENUM(ModelOverridableEnable);
  uint8_t modelTelemetryDisabled:2 ENUM(ModelOverridableEnable);
});

/*
 * Radio structure
 */

#if XPOTS_MULTIPOS_COUNT > 0
PACK(struct StepsCalibData {
  uint8_t count;
  uint8_t steps[XPOTS_MULTIPOS_COUNT-1];
});
#endif

PACK(struct CalibData {
  CUST_IDX(calib, r_calib, w_calib);
  int16_t mid;
  int16_t spanNeg;
  int16_t spanPos;
});

PACK(struct TrainerMix {
  uint8_t srcChn:6; // 0-7 = ch1-8
  uint8_t mode:2 ENUM(TrainerMultiplex);   // off,add-mode,subst-mode
  int8_t  studWeight;
});

PACK(struct TrainerData {
  int16_t        calib[4];
  NOBACKUP(TrainerMix mix[4]);
});

#if defined(COLORLCD)
  #define EXTRA_GENERAL_FIELDS \
    NOBACKUP(char currModelFilename[LEN_MODEL_FILENAME+1]); \
    NOBACKUP(uint8_t modelQuickSelect:1); \
    NOBACKUP(uint8_t blOffBright:7); \
    NOBACKUP(char bluetoothName[LEN_BLUETOOTH_NAME]);
#else
  #define EXTRA_GENERAL_FIELDS \
    uint8_t  backlightColor; \
    char bluetoothName[LEN_BLUETOOTH_NAME];
#endif

#if defined(BUZZER)
  #define BUZZER_FIELD int8_t buzzerMode:2    // -2=quiet, -1=only alarms, 0=no keys, 1=all (only used on AVR radios without audio hardware)
#else
  #define BUZZER_FIELD int8_t spare2:2 SKIP
#endif

PACK(struct RadioData {

  // Real attributes
  NOBACKUP(uint8_t manuallyEdited:1);
  int8_t timezoneMinutes:3;    // -3 to +3 ==> (-45 to 45 minutes in 15 minute increments)
  NOBACKUP(uint8_t ppmunit:2);  // PPMUnit enum
#if defined(USE_HATS_AS_KEYS)
  NOBACKUP(uint8_t hatsMode:2 ENUM(HatsMode));
#else
  NOBACKUP(uint8_t hatsModeSpare:2 SKIP);
#endif
  CUST_ATTR(semver,nullptr,w_semver);
  CUST_ATTR(board,nullptr,w_board);
  CalibData calib[MAX_CALIB_ANALOG_INPUTS] NO_IDX;
  NOBACKUP(uint16_t chkSum SKIP);
  N_HORUS_FIELD(int8_t currModel);
  N_HORUS_FIELD(uint8_t contrast);
  NOBACKUP(uint8_t vBatWarn);
  NOBACKUP(int8_t txVoltageCalibration);
  uint8_t backlightMode:3 ENUM(BacklightMode);
  int8_t antennaMode:2 ENUM(AntennaModes);
  uint8_t disableRtcWarning:1;
  uint8_t keysBacklight:1;
  NOBACKUP(uint8_t dontPlayHello:1);
  uint8_t internalModule ENUM(ModuleType);
  NOBACKUP(TrainerData trainer);
  NOBACKUP(uint8_t view);            // index of view in main screen
  NOBACKUP(BUZZER_FIELD); /* 2bits */
  NOBACKUP(uint8_t fai:1);
  NOBACKUP(int8_t beepMode:2 ENUM(BeeperMode) CUST(r_beeperMode,w_beeperMode));
  NOBACKUP(uint8_t alarmsFlash:1);
  NOBACKUP(uint8_t disableMemoryWarning:1);
  NOBACKUP(uint8_t disableAlarmWarning:1);
  uint8_t stickMode:2;
  int8_t timezone:5;
  uint8_t adjustRTC:1;
  NOBACKUP(uint8_t inactivityTimer);
  CUST_ATTR(telemetryBaudrate, r_telemetryBaudrate, nullptr);
  uint8_t internalModuleBaudrate:3;
  int8_t splashMode:3; /* 3bits */
  int8_t hapticMode:2 CUST(r_beeperMode,w_beeperMode);
  int8_t switchesDelay;
  NOBACKUP(uint8_t lightAutoOff);
  NOBACKUP(uint8_t templateSetup);   // RETA order for receiver channels
  NOBACKUP(int8_t PPM_Multiplier);
  NOBACKUP(int8_t hapticLength CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t beepLength:3 CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t hapticStrength:3 CUST(r_5pos,w_5pos));
  NOBACKUP(uint8_t gpsFormat:1);
  NOBACKUP(uint8_t  audioMuteEnable:1);
  NOBACKUP(uint8_t speakerPitch CUST(r_spPitch,w_spPitch));
  NOBACKUP(int8_t speakerVolume CUST(r_vol,w_vol));
  NOBACKUP(int8_t vBatMin CUST(r_vbat_min,w_vbat_min));
  NOBACKUP(int8_t vBatMax CUST(r_vbat_max,w_vbat_max));

  NOBACKUP(uint8_t  backlightBright);
  NOBACKUP(uint32_t globalTimer);
  NOBACKUP(uint8_t  bluetoothBaudrate:4);
  NOBACKUP(uint8_t  bluetoothMode:4 ENUM(BluetoothModes));

  NOBACKUP(uint8_t  countryCode:2);
  NOBACKUP(int8_t   pwrOnSpeed:3);
  NOBACKUP(int8_t   pwrOffSpeed:3);

  CUST_ATTR(jitterFilter, r_jitterFilter, nullptr);
  NOBACKUP(uint8_t  noJitterFilter:1); /* 0 - Jitter filter active */
  NOBACKUP(uint8_t  imperial:1);
  NOBACKUP(uint8_t  disableRssiPoweroffAlarm:1);
  NOBACKUP(uint8_t  USBMode:2);
  NOBACKUP(uint8_t  jackMode:2);
  NOBACKUP(uint8_t  sportUpdatePower:1 SKIP);

  NOBACKUP(char     ttsLanguage[2]);
  NOBACKUP(int8_t   beepVolume:4 CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t   wavVolume:4 CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t   varioVolume:4 CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t   backgroundVolume:4 CUST(r_5pos,w_5pos));
  NOBACKUP(int8_t   varioPitch CUST(r_vPitch,w_vPitch));
  NOBACKUP(int8_t   varioRange CUST(r_vPitch,w_vPitch));
  NOBACKUP(int8_t   varioRepeat);
  CustomFunctionData customFn[MAX_SPECIAL_FUNCTIONS] FUNC(cfn_is_active);

  CUST_ATTR(auxSerialMode, r_serialMode, nullptr);
  CUST_ATTR(aux2SerialMode, r_serialMode, nullptr);
  NOBACKUP(uint32_t serialPort ARRAY(SERIAL_CONF_BITS_PER_PORT,struct_serialConfig,nullptr));

  CUST_ARRAY(sticksConfig, struct_stickConfig, MAX_STICKS, stick_name_valid);
  CUST_ARRAY(slidersConfig, struct_sliderConfig, MAX_POTS, nullptr);
  potconfig_t potsConfig ARRAY(4,struct_potConfig,nullptr);
  swconfig_t switchConfig ARRAY(2,struct_switchConfig,nullptr);
  CUST_ARRAY(flexSwitches, struct_flexSwitch, MAX_FLEX_SWITCHES, flex_sw_valid);

  EXTRA_GENERAL_FIELDS

  char ownerRegistrationID[PXX2_LEN_REGISTRATION_ID];

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  CUST_ATTR(rotEncDirection, r_rotEncDirection, nullptr);
  NOBACKUP(uint8_t  rotEncMode:3);
#else
  NOBACKUP(uint8_t  rotEncModeSpare:3 SKIP);
#endif

  NOBACKUP(int8_t   uartSampleMode:2); // See UartSampleModes

#if defined(STICK_DEAD_ZONE)
  NOBACKUP(uint8_t  stickDeadZone:3);
#else
  NOBACKUP(uint8_t  stickDeadZoneSpare:3 SKIP);
#endif

#if defined(IMU)
  NOBACKUP(int8_t imuMax);
  NOBACKUP(int8_t imuOffset);
#endif

#if defined(COLORLCD)
  NOBACKUP(char selectedTheme[SELECTED_THEME_NAME_LEN]);
#endif

#if defined(COLORLCD)
  NOBACKUP(uint8_t labelSingleSelect:1);  // 0 = multi-select, 1 = single select labels
  NOBACKUP(uint8_t labelMultiMode:1);     // 0 = match all labels (AND), 1 = match any labels (OR)
  NOBACKUP(uint8_t favMultiMode:1);       // 0 = match all (AND), 1 = match any (OR)
  // Radio level tabs control (global settings)
  NOBACKUP(uint8_t modelSelectLayout:2);
  NOBACKUP(uint8_t radioThemesDisabled:1);
#endif
  NOBACKUP(uint8_t radioGFDisabled:1);
  NOBACKUP(uint8_t radioTrainerDisabled:1);
  // Model level tabs control (global setting)
  NOBACKUP(uint8_t modelHeliDisabled:1);
  NOBACKUP(uint8_t modelFMDisabled:1);
  NOBACKUP(uint8_t modelCurvesDisabled:1);
  NOBACKUP(uint8_t modelGVDisabled:1);
  NOBACKUP(uint8_t modelLSDisabled:1);
  NOBACKUP(uint8_t modelSFDisabled:1);
  NOBACKUP(uint8_t modelCustomScriptsDisabled:1);
  NOBACKUP(uint8_t modelTelemetryDisabled:1);
  NOBACKUP(uint8_t disableTrainerPoweroffAlarm:1);
  NOBACKUP(uint8_t disablePwrOnOffHaptic:1);

#if defined(COLORLCD)
  NOBACKUP(uint8_t spare:6 SKIP);
#elif LCD_W == 128
  uint8_t invertLCD:1;          // Invert B&W LCD display
  NOBACKUP(uint8_t spare:3 SKIP);
#else
  NOBACKUP(uint8_t spare:4 SKIP);
#endif

  NOBACKUP(uint8_t pwrOffIfInactive);

  NOBACKUP(uint8_t getBrightness() const
  {
#if defined(OLED_SCREEN)
    return contrast;
#else
    return backlightBright;
#endif
  });
});

#undef SWITCHES_WARNING_DATA
#undef MODEL_GVARS_DATA
#undef TELEMETRY_DATA
#undef SCRIPTS_DATA
#undef CUSTOM_SCREENS_DATA
#undef EXTRA_GENERAL_FIELDS
#undef NOBACKUP
