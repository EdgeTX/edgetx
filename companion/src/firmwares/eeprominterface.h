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

#ifndef _EEPROMINTERFACE_H_
#define _EEPROMINTERFACE_H_

#include "boards.h"
#include "macros.h"
#include "radiodata.h"
#include "../../radio/src/definitions.h"
#include "simulatorinterface.h"

#include <QtCore>
#include <QStringList>
#include <QList>
#include <QDebug>

#include <iostream>

const uint8_t modn12x3[4][4]= {
  {1, 2, 3, 4},
  {1, 3, 2, 4},
  {4, 2, 3, 1},
  {4, 3, 2, 1} };

enum Capability {
  Models,
  ModelName,
  FlightModes,
  FlightModesName,
  FlightModesHaveFades,
  Imperial,
  Mixes,
  Timers,
  TimersName,
  VoicesAsNumbers,
  VoicesMaxLength,
  MultiLangVoice,
  ModelImage,
  CustomFunctions,
  SafetyChannelCustomFunction,
  LogicalSwitches,
  CustomAndSwitches,
  HasNegAndSwitches,
  LogicalSwitchesExt,
  RotaryEncoders,
  Outputs,
  ChannelsName,
  ExtraInputs,
  TrimsRange,
  ExtendedTrimsRange,
  NumCurves,
  NumCurvePoints,
  OffsetWeight,
  Simulation,
  SoundMod,
  SoundPitch,
  MaxVolume,
  MaxContrast,
  MinContrast,
  Haptic,
  HasBeeper,
  ModelTrainerEnable,
  HasExpoNames,
  HasNoExpo,
  HasMixerNames,
  HasCvNames,
  HasPxxCountry,
  HasPPMStart,
  HasGeneralUnits,
  HasFAIMode,
  OptrexDisplay,
  PPMExtCtrl,
  PPMFrameLength,
  Telemetry,
  TelemetryUnits,
  TelemetryBars,
  Heli,
  Gvars,
  GvarsInCS,
  GvarsAreNamed,
  GvarsFlightModes,
  GvarsName,
  NoTelemetryProtocol,
  TelemetryCustomScreens,
  TelemetryCustomScreensBars,
  TelemetryCustomScreensLines,
  TelemetryCustomScreensFieldsPerLine,
  TelemetryMaxMultiplier,
  HasVario,
  HasVarioSink,
  HasFailsafe,
  HasSoundMixer,
  NumModules,
  NumFirstUsableModule,
  HasModuleR9MFlex,
  HasModuleR9MMini,
  PPMCenter,
  PPMUnitMicroseconds,
  SYMLimits,
  HastxCurrentCalibration,
  HasVolume,
  HasBrightness,
  PerModelTimers,
  SlowScale,
  SlowRange,
  PermTimers,
  HasSDLogs,
  CSFunc,
  LcdWidth,
  LcdHeight,
  LcdDepth,
  GetThrSwitch,
  HasDisplayText,
  HasTopLcd,
  GlobalFunctions,
  VirtualInputs,
  InputsLength,
  TrainerInputs,
  SportTelemetry,
  LuaScripts,
  LuaInputsPerScript,
  LuaOutputsPerScript,
  LimitsPer1000,
  EnhancedCurves,
  HasFasOffset,
  HasMahPersistent,
  SimulatorVariant,
  MavlinkTelemetry,
  HasInputDiff,
  HasMixerExpo,
  HasBatMeterRange,
  DangerousFunctions,
  HasModelCategories,
  HasSwitchableJack,
  HasSportConnector,
  PwrButtonPress,
  Sensors,
  HasAuxSerialMode,
  HasAux2SerialMode,
  HasBluetooth,
  HasAntennaChoice,
  HasADCJitterFilter,
  HasTelemetryBaudrate
};

class EEPROMInterface
{
  Q_DECLARE_TR_FUNCTIONS(EEPROMInterface)

  public:

    EEPROMInterface(Board::Type board):
      board(board)
    {
    }

    virtual ~EEPROMInterface() {}

    inline Board::Type getBoard() { return board; }

    virtual unsigned long load(RadioData &radioData, const uint8_t * eeprom, int size) = 0;

    virtual unsigned long loadBackup(RadioData & radioData, const uint8_t * eeprom, int esize, int index) = 0;

    virtual int save(uint8_t * eeprom, const RadioData & radioData, uint8_t version=0, uint32_t variant=0) = 0;

    virtual int getSize(const ModelData &) = 0;

    virtual int getSize(const GeneralSettings &) = 0;

    //static void showEepromErrors(QWidget *parent, const QString &title, const QString &mainMessage, unsigned long errorsFound);
    static QString getEepromWarnings(unsigned long errorsFound);

  protected:

    Board::Type board;

  private:

    EEPROMInterface();

};

/* EEPROM string conversion function (used only by er9xeeprom and ersky9xeeprom) */
inline void getEEPROMString(char *dst, const char *src, int size)
{
  memcpy(dst, src, size);
  dst[size] = '\0';
  for (int i=size-1; i>=0; i--) {
    if (dst[i] == ' ')
      dst[i] = '\0';
    else
      break;
  }
}

// (used only by er9xeeprom and ersky9xeeprom)
inline int applyStickMode(int stick, unsigned int mode)
{
  if (mode == 0 || mode > 4) {
    std::cerr << "Incorrect stick mode" << mode;
    return stick;
  }

  if (stick >= 1 && stick <= 4)
    return modn12x3[mode-1][stick-1];
  else
    return stick;
}

float ValToTim(int value);
int TimToVal(float value);

void registerEEpromInterfaces();
void unregisterEEpromInterfaces();
void registerOpenTxFirmwares();
void unregisterOpenTxFirmwares();

enum EepromLoadErrors {
  ALL_OK,
  UNKNOWN_ERROR,
  UNSUPPORTED_NEWER_VERSION,
  WRONG_SIZE,
  WRONG_FILE_SYSTEM,
  NOT_OPENTX,
  NOT_ERSKY9X,
  UNKNOWN_BOARD,
  WRONG_BOARD,
  BACKUP_NOT_SUPPORTED,

  HAS_WARNINGS,
  OLD_VERSION,
  WARNING_WRONG_FIRMWARE,

  NUM_ERRORS
};

class Firmware
{
  Q_DECLARE_TR_FUNCTIONS(Firmware)

  public:
    struct Option {
      const char * name = nullptr;
      QString tooltip;
      unsigned variant = 0;

      explicit Option(const char * name, const QString & description, unsigned variant = 0) :
        name(name), tooltip(description), variant(variant) { }
    };
    typedef QList<Option> OptionsGroup;
    typedef QList<OptionsGroup> OptionsList;


    explicit Firmware(const QString & id, const QString & name, Board::Type board) :
      Firmware(nullptr, id, name, board)
    { }

    explicit Firmware(Firmware * base, const QString & id, const QString & name, Board::Type board) :
      id(id),
      name(name),
      board(board),
      variantBase(0),
      base(base),
      eepromInterface(nullptr)
    { }

    virtual ~Firmware() { }

    inline const Firmware * getFirmwareBase() const
    {
      return base ? base : this;
    }

    virtual Firmware * getFirmwareVariant(const QString & id) { return NULL; }

    unsigned int getVariantNumber();

    virtual void addLanguage(const char * lang);

    //virtual void addTTSLanguage(const char * lang);

    virtual void addOption(const char * option, const QString & tooltip = QString(), unsigned variant = 0);

    virtual void addOption(const Option & option);

    virtual void addOptionsGroup(const OptionsGroup & options);

    virtual QString getStampUrl() = 0;

    virtual QString getReleaseNotesUrl() = 0;

    virtual QString getFirmwareUrl() = 0;

    Board::Type getBoard() const
    {
      return board;
    }

    void setEEpromInterface(EEPROMInterface * eeprom)
    {
      eepromInterface = eeprom;
    }

    EEPROMInterface * getEEpromInterface()
    {
      return eepromInterface;
    }

    QString getName() const
    {
      return name;
    }

    QString getId() const
    {
      return id;
    }

    QList<const char *> languageList() const
    {
      return languages;
    }

    OptionsList optionGroups() const
    {
      return opts;
    }

    virtual int getCapability(Capability) = 0;

    virtual QString getAnalogInputName(unsigned int index) = 0;

    virtual QTime getMaxTimerStart() = 0;

    virtual bool isAvailable(PulsesProtocol proto, int port=0) = 0;

    const int getFlashSize();

    static Firmware * getFirmwareForId(const QString & id);

    static QVector<Firmware *> getRegisteredFirmwares()
    {
      return registeredFirmwares;
    }

    static void addRegisteredFirmware(Firmware * fw)
    {
      registeredFirmwares.append(fw);
    }

    static Firmware * getDefaultVariant()
    {
      return defaultVariant;
    }
    static void setDefaultVariant(Firmware * value)
    {
      defaultVariant = value;
    }

    static Firmware * getCurrentVariant()
    {
      return currentVariant;
    }

    static void setCurrentVariant(Firmware * value)
    {
      currentVariant = value;
    }

  protected:
    QString id;
    QString name;
    Board::Type board;
    unsigned int variantBase;
    Firmware * base;
    EEPROMInterface * eepromInterface;
    QList<const char *> languages;
    //QList<const char *> ttslanguages;
    OptionsList opts;

    static QVector<Firmware *> registeredFirmwares;
    static Firmware * defaultVariant;
    static Firmware * currentVariant;
};

inline Firmware * getCurrentFirmware()
{
  return Firmware::getCurrentVariant();
}

inline EEPROMInterface * getCurrentEEpromInterface()
{
  return Firmware::getCurrentVariant()->getEEpromInterface();
}

inline Board::Type getCurrentBoard()
{
  return Firmware::getCurrentVariant()->getBoard();
}

inline int divRoundClosest(const int n, const int d)
{
  return ((n < 0) ^ (d < 0)) ? ((n - d/2)/d) : ((n + d/2)/d);
}

inline int calcRESXto100(int x)
{
  return divRoundClosest(x*100, 1024);
}

extern QList<EEPROMInterface *> eepromInterfaces;


#endif // _EEPROMINTERFACE_H_
