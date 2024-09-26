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
#include "macros.h"
#include "radiodata.h"
#include "../../radio/src/definitions.h"
#include "simulatorinterface.h"
#include "datahelpers.h"

#include <QtCore>
#include <QStringList>
#include <QList>
#include <QDebug>

#include <iostream>
#include <string>

const uint8_t modn12x3[8][4]= {
  {1, 2, 3, 4}, // air mode 1
  {1, 3, 2, 4}, // air mode 2
  {4, 2, 3, 1}, // air mode 3
  {4, 3, 2, 1}, // air mode 4
  {2, 3, 4, 1}, // surface mode 1
  {2, 3, 4, 1}, // surface mode 2
  {2, 3, 4, 1}, // surface mode 3
  {2, 3, 4, 1}  // surface mode 4
};

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
  HasModelImage,
  ModelImageNameLen,
  ModelImageFilters,
  ModelImageKeepExtn,
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
  MavlinkTelemetry,
  HasInputDiff,
  HasMixerExpo,
  HasBatMeterRange,
  DangerousFunctions,
  HasModelLabels,
  HasSwitchableJack,
  HasSportConnector,
  PwrButtonPress,
  Sensors,
  HasAuxSerialMode,
  HasAux2SerialMode,
  HasVCPSerialMode,
  HasBluetooth,
  HasADCJitterFilter,
  HasTelemetryBaudrate,
  TopBarZones,
  HasModelsList,
  HasFlySkyGimbals,
  RotaryEncoderNavigation,
  HasSoftwareSerialPower,
  HasIntModuleMulti,
  HasIntModuleCRSF,
  HasIntModuleELRS,
  HasIntModuleFlySky,
  BacklightLevelMin,
};

float ValToTim(int value);
int TimToVal(float value);

void registerOpenTxFirmwares();
void unregisterOpenTxFirmwares();

constexpr char FIRMWARE_ID_PREFIX[] = { "edgetx-" };

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


    explicit Firmware(const QString & id, const QString & name, Board::Type board, const QString & downloadId = QString(),
                      const QString & simulatorId = QString(), const QString & hwdefnId = QString()) :
      Firmware(nullptr, id, name, board, downloadId, simulatorId, hwdefnId)
    { }

    explicit Firmware(Firmware * base, const QString & id, const QString & name, Board::Type board,
                      const QString & downloadId = QString(), const QString & simulatorId = QString(),
                      const QString & hwdefnId = QString());

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

    virtual QString getCapabilityStr(Capability) = 0;

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

    static void sortRegisteredFirmwares()
    {
      std::sort(registeredFirmwares.begin(), registeredFirmwares.end(),
                [](const Firmware *a, const Firmware *b) {
                  return QString::compare(a->getName(), b->getName(),
                                          Qt::CaseInsensitive) < 0;
                });
    }

    QString getFlavour();

    static Firmware * getFirmwareForFlavour(const QString & flavour)
    {
      return getFirmwareForId(FIRMWARE_ID_PREFIX + flavour);
    }

    const QString getDownloadId() { return getFirmwareBase()->downloadId.isEmpty() ? getFlavour() : getFirmwareBase()->downloadId; }
    const QString getSimulatorId() { return getFirmwareBase()->simulatorId.isEmpty() ? getId() : getFirmwareBase()->simulatorId; }
    const QString getHwDefnId() { return getFirmwareBase()->hwdefnId.isEmpty() ? getFlavour() : getFirmwareBase()->hwdefnId; }

  protected:
    QString id;
    QString name;
    Board::Type board;
    unsigned int variantBase;
    Firmware * base;
    QString downloadId;
    QString simulatorId;
    QString hwdefnId;

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

inline Board::Type getCurrentBoard()
{
  return Firmware::getCurrentVariant()->getBoard();
}

inline int divRoundClosest(const int n, const int d)
{
  return ((n < 0) ^ (d < 0)) ? ((n - d / 2) / d) : ((n + d / 2) / d);
}

inline int calcRESXto100(int x)
{
  return divRoundClosest(x * 100, 1024);
}
