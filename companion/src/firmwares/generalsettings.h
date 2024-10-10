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

#include "boards.h"
#include "constants.h"
#include "customfunctiondata.h"
#include "rawsource.h"
#include "datahelpers.h"
#include "customisation_data.h"

#include <QtCore>

class Firmware;
class ModelData;
class GeneralSettings;
class RadioDataConversionState;
class AbstractStaticItemModel;

// identiying names of static abstract item models
constexpr char AIM_GS_ANTENNAMODE[]        {"gs.antennamode"};
constexpr char AIM_GS_BLUETOOTHMODE[]      {"gs.bluetoothmode"};
constexpr char AIM_GS_SERIALMODE[]         {"gs.serialmode"};
constexpr char AIM_GS_INTMODULEBAUDRATE[]  {"gs.intmodulebaudrate"};
constexpr char AIM_GS_STICKDEADZONE[]      {"gs.stickdeadzone"};
constexpr char AIM_GS_UARTSAMPLEMODE[]     {"gs.uartsamplemode"};
constexpr char AIM_GS_HATSMODE[]           {"gs.hatsmode"};
constexpr char AIM_GS_STICKMODE[]          {"gs.stickmode"};
constexpr char AIM_GS_TEMPLATESETUP[]      {"gs.templatesetup"};

constexpr char AIM_TRAINERMIX_MODE[]       {"trainermix.mode"};
constexpr char AIM_TRAINERMIX_SRC[]        {"trainermix.src"};

static const QStringList moduleBaudratesList({"115K", "400K", "921K", "1.87M",
                                              "3.75M", "5.25M"});

enum UartModes {
  UART_MODE_NONE,
  UART_MODE_TELEMETRY_MIRROR,
  UART_MODE_TELEMETRY,
  UART_MODE_SBUS_TRAINER,
  UART_MODE_DEBUG
};

class TrainerMix {
  Q_DECLARE_TR_FUNCTIONS(TrainerMix)

  public:
    enum TrainerMixModes {
      TRN_MIX_MODE_OFF,
      TRN_MIX_MODE_ADD,
      TRN_MIX_MODE_SUBST,
      TRN_MIX_MODE_COUNT
    };

    TrainerMix() { clear(); }

    unsigned int src; // 0-7 = ch1-8
    RawSwitch swtch;
    int weight;
    unsigned int mode;   // off, add-mode, subst-mode

    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(TrainerMix)); }
    QString modeToString() const;
    QString srcToString() const;

    static FieldRange getWeightRange();
    static QString modeToString(int value);
    static QString srcToString(int value);
    static AbstractStaticItemModel * modeItemModel();
    static AbstractStaticItemModel * srcItemModel();
};

class TrainerData {
  public:
    TrainerData() { clear(); }

    int calib[4];
    TrainerMix mix[4];

    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(TrainerData)); }
};

constexpr int CURR_MODEL_FILENAME_LEN {16};
constexpr int OWNER_NAME_LEN          {10};
constexpr int BLUETOOTH_NAME_LEN      {10};
constexpr int TTS_LANGUAGE_LEN        {2};
constexpr int HARDWARE_NAME_LEN       {3};
constexpr int REGISTRATION_ID_LEN     {8};
constexpr int SELECTED_THEME_NAME_LEN {26};
constexpr int SWITCH_INPUTINDEX_NONE  {-1};

class GeneralSettings {
  Q_DECLARE_TR_FUNCTIONS(GeneralSettings)

  public:

    enum BeeperMode {
      BEEPER_QUIET = -2,
      BEEPER_ALARMS_ONLY = -1,
      BEEPER_NOKEYS = 0,
      BEEPER_ALL = 1
    };

    enum BluetoothMode {
      BLUETOOTH_MODE_OFF,
      BLUETOOTH_MODE_ENABLED,
      BLUETOOTH_MODE_TELEMETRY = BLUETOOTH_MODE_ENABLED,
      BLUETOOTH_MODE_TRAINER,
      BLUETOOTH_MODE_COUNT
    };

    enum AntennaMode {
      ANTENNA_MODE_INTERNAL = -2,
      ANTENNA_MODE_FIRST = ANTENNA_MODE_INTERNAL,
      ANTENNA_MODE_ASK,
      ANTENNA_MODE_PER_MODEL,
      ANTENNA_MODE_EXTERNAL,
      ANTENNA_MODE_INTERNAL_EXTERNAL = ANTENNA_MODE_EXTERNAL,
      ANTENNA_MODE_LAST = ANTENNA_MODE_INTERNAL_EXTERNAL
    };

    enum AuxSerialMode {
      AUX_SERIAL_OFF,
      AUX_SERIAL_TELE_MIRROR,
      AUX_SERIAL_TELE_IN,
      AUX_SERIAL_SBUS_TRAINER,
      AUX_SERIAL_LUA,
      AUX_SERIAL_CLI,
      AUX_SERIAL_GPS,
      AUX_SERIAL_DEBUG,
      AUX_SERIAL_SPACEMOUSE,
      AUX_SERIAL_EXT_MODULE,
      AUX_SERIAL_COUNT
    };

    enum AuxSerialModeContext
    {
      AUX1Context = 0x01,
      AUX2Context = 0x02,
      VCPContext  = 0x04,
      AllAuxSerialModeContexts = AUX1Context | AUX2Context | VCPContext,
    };

    enum TelemetryBaudrate {
    };

    enum SerialPort {
      SP_AUX1,
      SP_AUX2,
      SP_VCP,
      SP_COUNT,
    };

    enum UartSampleMode {
      UART_SAMPLE_MODE_NORMAL,
      UART_SAMPLE_MODE_ONEBIT,
      UART_SAMPLE_MODE_COUNT
    };

    enum HatsMode {
      HATSMODE_TRIMS_ONLY,
      HATSMODE_KEYS_ONLY,
      HATSMODE_SWITCHABLE,
      HATSMODE_GLOBAL,
      HATSMODE_COUNT
    };

    enum PPMUnit {
      PPM_PERCENT_PREC0,
      PPM_PERCENT_PREC1,
      PPM_US
    };

    enum StickMode {
      STICK_MODE_1,
      STICK_MODE_2,
      STICK_MODE_3,
      STICK_MODE_4,
      STICK_MODE_COUNT,
    };

    enum RadioTypeContext {
      RadioTypeContextAir     = 1 << 1,
      RadioTypeContextSurface = 1 << 2,
    };

    GeneralSettings() { clear(); }
    void clear();
    void init();
    void convert(RadioDataConversionState & cstate);

    void setDefaultControlTypes(Board::Type board);
    int getDefaultStick(unsigned int channel) const;
    RawSource getDefaultSource(unsigned int channel) const;
    int getDefaultChannel(unsigned int stick) const;
    bool fix6POSCalibration();

    bool manuallyEdited;

    char semver[8 + 1];
    unsigned int version;
    unsigned int variant;
    unsigned int currModelIndex;
    char currModelFilename[CURR_MODEL_FILENAME_LEN + 1];
    unsigned int contrast;
    unsigned int vBatWarn;
    int txVoltageCalibration;
    int txCurrentCalibration;
    int vBatMin;
    int vBatMax;
    int backlightMode;
    unsigned int internalModule;  // Introducted in EdgeTX 2.6 yaml only
    TrainerData trainer;
    unsigned int view;    // main screen view // TODO enum
    bool disableThrottleWarning;
    bool fai;
    bool disableMemoryWarning;
    BeeperMode beeperMode;
    bool disableAlarmWarning;
    bool disableRssiPoweroffAlarm;
    bool disableTrainerPoweroffAlarm;
    unsigned int usbMode;
    unsigned int hatsMode;
    unsigned int stickDeadZone;
    unsigned int jackMode;
    bool sportPower;
    BeeperMode hapticMode;
    unsigned int stickMode; // TODO enum
    int timezone;
    int timezoneMinutes;
    bool adjustRTC;
    bool optrexDisplay;
    unsigned int inactivityTimer;
    unsigned int internalModuleBaudrate;
    bool minuteBeep;
    bool preBeep;
    bool alarmsFlash;
    int splashMode;
    bool dontPlayHello;
    unsigned int backlightDelay;
    unsigned int templateSetup;  //RETA order according to chout_ar array
    int PPM_Multiplier;
    int hapticLength;
    unsigned int reNavigation;
    unsigned int stickReverse;
    unsigned int speakerPitch;
    int hapticStrength;
    unsigned int speakerMode;
    char ownerName[OWNER_NAME_LEN + 1];
    int beeperLength;
    unsigned int gpsFormat;
    int speakerVolume;
    unsigned int backlightBright;
    unsigned int backlightOffBright;
    int switchesDelay;
    unsigned int mAhWarn;
    unsigned int mAhUsed;
    unsigned int globalTimer;
    char bluetoothName[BLUETOOTH_NAME_LEN + 1];
    unsigned int bluetoothBaudrate;
    unsigned int bluetoothMode;
    unsigned int sticksGain;
    unsigned int rotarySteps;
    unsigned int countryCode;
    bool noJitterFilter;
    bool rtcCheckDisable;
    bool muteIfNoSound;
    bool keysBacklight;
    unsigned int rotEncMode;
    unsigned int imperial;
    unsigned int ppmunit;
    char ttsLanguage[TTS_LANGUAGE_LEN + 1];
    int beepVolume;
    int wavVolume;
    int varioVolume;
    int varioPitch;
    int varioRange;
    int varioRepeat;
    int backgroundVolume;
    unsigned int mavbaud;
    unsigned int switchUnlockStates;
    unsigned int serialPort[SP_COUNT];
    bool serialPower[SP_COUNT];
    int antennaMode;
    unsigned int backlightColor;
    bool modelQuickSelect;
    CustomFunctionData customFn[CPN_MAX_SPECIAL_FUNCTIONS];

    char registrationId[REGISTRATION_ID_LEN + 1];
    int imuMax;
    int imuOffset;
    int uartSampleMode;

    int pwrOnSpeed;
    int pwrOffSpeed;
    bool disablePwrOnOffHaptic;

    char selectedTheme[SELECTED_THEME_NAME_LEN + 1];

    // Radio level tabs control (global settings)
    bool radioThemesDisabled;
    bool radioGFDisabled;
    bool radioTrainerDisabled;
    // Model level tabs control (global setting)
    bool modelHeliDisabled;
    bool modelFMDisabled;
    bool modelCurvesDisabled;
    bool modelGVDisabled;
    bool modelLSDisabled;
    bool modelSFDisabled;
    bool modelCustomScriptsDisabled;
    bool modelTelemetryDisabled;
    unsigned int pwrOffIfInactive;

    int modelSelectLayout;
    int labelSingleSelect;
    int labelMultiMode;
    int favMultiMode;

    bool invertLCD;

    // v 2.10 ADC refactor
    // earlier version data is read into legacy structs to maintain older version compatibility
    // post reading the legacy structs are manipulated into the new structs
    // An opportunity was taken group related structs
    // Companion gui only references the new structs

    // pre v2.10 legacy
    // TODO remove when importing and conversion no longer neceesary
    int calibMid[CPN_MAX_ANALOGS];
    int calibSpanNeg[CPN_MAX_ANALOGS];
    int calibSpanPos[CPN_MAX_ANALOGS];
    char swtchName[CPN_MAX_SWITCHES][HARDWARE_NAME_LEN + 1];
    unsigned int swtchConfig[CPN_MAX_SWITCHES];
    char stickName[CPN_MAX_STICKS][HARDWARE_NAME_LEN + 1];
    char potName[CPN_MAX_POTS][HARDWARE_NAME_LEN + 1];
    unsigned int potConfig[CPN_MAX_POTS];
    char sliderName[CPN_MAX_SLIDERS][HARDWARE_NAME_LEN + 1];
    unsigned int sliderConfig[CPN_MAX_SLIDERS];

    // ===================================================================================
    // IMPORTANT: starting v2.10 this function MUST be called after importing non-yaml formats
    //            yaml conversion is performed on the fly
    bool convertLegacyConfiguration(Board::Type board);
    // ===================================================================================

    // default values are retrieved from the radio json file

    struct InputCalib {
      int mid;
      int spanNeg;
      int spanPos;
    };

    struct InputConfig {
      Board::AnalogInputType type;
      char name[HARDWARE_NAME_LEN + 1];
      Board::FlexType flexType;
      bool inverted;
      InputCalib calib;
    };

    InputConfig inputConfig[CPN_MAX_INPUTS];

    struct SwitchConfig {
      char name[HARDWARE_NAME_LEN + 1];
      Board::SwitchType type;
      bool inverted;
      int inputIdx;  //  used if switch tag = FLn, value -1 = none selected
    };

    SwitchConfig switchConfig[CPN_MAX_SWITCHES];

    bool switchPositionAllowed(int index) const;
    bool switchSourceAllowed(int index) const;

    bool isInputAvailable(int index) const;
    bool isInputMultiPosPot(int index) const;
    bool isInputPot(int index) const;
    bool isInputSlider(int index) const;
    bool isInputStick(int index) const;
    bool isInputFlexSwitchAvailable(int index) const;
    bool isSwitchAvailable(int index) const;
    bool isSwitchFlex(int index) const;
    bool unassignedInputFlexSwitches() const;

    QString antennaModeToString() const;
    QString bluetoothModeToString() const;
    QString serialPortModeToString(int port_nr) const;
    QString internalModuleBaudrateToString() const;
    QString uartSampleModeToString() const;
    QString hatsModeToString() const;
    QString stickModeToString() const;
    QString templateSetupToString() const;

    static QString antennaModeToString(int value);
    static QString bluetoothModeToString(int value);
    static QString serialModeToString(int value);
    static QString moduleBaudrateToString(int value);
    static FieldRange getPPM_MultiplierRange();
    static FieldRange getTxCurrentCalibration();
    static QString uartSampleModeToString(int value);
    static QString hatsModeToString(int value);
    static QString stickModeToString(int value);
    static QString templateSetupToString(int value, bool isBoardAir);

    static AbstractStaticItemModel * antennaModeItemModel(bool model_setup = false);
    static AbstractStaticItemModel * bluetoothModeItemModel();
    static AbstractStaticItemModel * serialModeItemModel();
    static AbstractStaticItemModel * internalModuleBaudrateItemModel();
    static AbstractStaticItemModel * stickDeadZoneItemModel();
    static AbstractStaticItemModel * uartSampleModeItemModel();
    static AbstractStaticItemModel * hatsModeItemModel(bool radio_setup = true);
    static AbstractStaticItemModel * stickModeItemModel();
    static AbstractStaticItemModel * templateSetupItemModel();

    void validateFlexSwitches();
};
