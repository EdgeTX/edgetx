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

#include "firmwares/boards.h"
#include "constants.h"

#include <algorithm>
#include <inttypes.h>

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDir>
#include <QLibrary>
#include <QMap>
#include <QSerialPort>

#define SIMULATOR_INTERFACE_HEARTBEAT_PERIOD    1000  // ms

enum SimulatorTelemetryProtocol {
  SIMU_TELEMETRY_PROTOCOL_FRSKY_SPORT = 0,
  SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB,
  SIMU_TELEMETRY_PROTOCOL_CROSSFIRE,
  SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB_OOB,
  SIMU_TELEMETRY_PROTOCOL_COUNT
};

enum SimulatorSerialEncoding {
  SERIAL_ENCODING_8N1 = 0,
  SERIAL_ENCODING_8E2,
  SERIAL_ENCODING_PXX1_PWM,
  SERIAL_ENCODING_COUNT
};

class SimulatorInterface : public QObject
{
  Q_OBJECT

  public:

    enum InputSourceType {
      INPUT_SRC_NONE = 0,
      INPUT_SRC_ANALOG,     // any analog source, index into g_anas[]
      INPUT_SRC_STICK,      // Board::StickAxes, g_anas[index]
      INPUT_SRC_KNOB,       // g_anas[index + StickAxes]
      INPUT_SRC_SLIDER,     // g_anas[index + StickAxes + num_pots]
      INPUT_SRC_TXVIN,      // g_anas[Analogs::TX_VOLTAGE]
      INPUT_SRC_SWITCH,     // Named 2/3-pos switches
      INPUT_SRC_TRIM_SW,    // Board::TrimSwitches
      INPUT_SRC_TRIM,       // Board::TrimAxes
      INPUT_SRC_KEY,        // UI key/pushbutton
      INPUT_SRC_ROTENC,     // Rotary encoder (TODO)
      INPUT_SRC_TRAINER,    // Virtual trainer input
      INPUT_SRC_ENUM_COUNT
    };

    enum OutputSourceType {
      OUTPUT_SRC_NONE = 0,
      OUTPUT_SRC_CHAN_OUT,
      OUTPUT_SRC_CHAN_MIX,
      OUTPUT_SRC_TRIM_VALUE,
      OUTPUT_SRC_TRIM_RANGE,
      OUTPUT_SRC_VIRTUAL_SW,
      OUTPUT_SRC_PHASE,
      OUTPUT_SRC_GVAR,
      OUTPUT_SRC_ENUM_COUNT
    };

    // only for data not available from Boards or Firmware, eg. compile-time options
    enum Capability {
      CAP_LUA,                // LUA
      CAP_ROTARY_ENC,         // ROTARY_ENCODERS
      CAP_ROTARY_ENC_NAV,     // ROTARY_ENCODER_NAVIGATION
      CAP_TELEM_FRSKY_SPORT,  // TELEMETRY_FRSKY_SPORT
      CAP_SERIAL_AUX1,        // Does AUX1 exist on this radio?
      CAP_SERIAL_AUX2,        // Does AUX2 exist on this radio?
      CAP_ENUM_COUNT
    };

    // This allows automatic en/decoding of flight mode + gvarIdx value to/from any int32
    struct gVarMode_t {
      int16_t value;
      uint8_t mode;
      uint8_t prec:2;
      uint8_t unit:2;

      gVarMode_t(int i = 0) {
        set(i);
      }
      void set(int i) {
        unit = (i >> 26) & 0x3;
        prec = (i >> 24) & 0x3;
        mode = (i >> 16) & 0xFF;
        value = (i & 0xFFFF);
      }
      operator int() {
        return (((unit & 0x3) << 26) | ((prec & 0x3) << 24) | ((mode & 0xFF) << 16) | (value & 0xFFFF));
      }
      gVarMode_t & operator =(const int i) {
        set(i);
        return *this;
      }
    };

    struct TxOutputs {
      TxOutputs() { clear(); }
      void clear() { memset(this, 0, sizeof(TxOutputs)); }

      int16_t chans[CPN_MAX_CHNOUT];       // final channel outputs
      int16_t ex_chans[CPN_MAX_CHNOUT];    // raw mix outputs
      qint32 gvars[CPN_MAX_FLIGHT_MODES][CPN_MAX_GVARS];
      int trims[CPN_MAX_TRIMS];            // Board::TrimAxes enum
      bool vsw[CPN_MAX_LOGICAL_SWITCHES];  // virtual/logic switches
      int8_t phase;
      qint16 trimRange;                  // TRIM_MAX or TRIM_EXTENDED_MAX
      // bool beep;
    };

    virtual ~SimulatorInterface() {}

    virtual QString name() = 0;
    virtual bool isRunning() = 0;
    virtual void readRadioData(QByteArray & dest) = 0;
    virtual uint8_t * getLcd() = 0;
    virtual uint8_t getSensorInstance(uint16_t id, uint8_t defaultValue = 0) = 0;
    virtual uint16_t getSensorRatio(uint16_t id) = 0;
    virtual const int getCapability(Capability cap) = 0;

  public slots:

    virtual void init() = 0;
    virtual void start(const char * filename = NULL, bool tests = true) = 0;
    virtual void stop() = 0;
    virtual void setSdPath(const QString & sdPath = "", const QString & settingsPath = "") = 0;
    virtual void setVolumeGain(const int value) = 0;
    virtual void setRadioData(const QByteArray & data) = 0;
    virtual void setAnalogValue(uint8_t index, int16_t value) = 0;
    virtual void setKey(uint8_t key, bool state) = 0;
    virtual void setSwitch(uint8_t swtch, int8_t state) = 0;
    virtual void setTrim(unsigned int idx, int value) = 0;
    virtual void setTrimSwitch(uint8_t trim, bool state) = 0;
    virtual void setTrainerInput(unsigned int inputNumber, int16_t value) = 0;
    virtual void setInputValue(int type, uint8_t index, int16_t value) = 0;
    virtual void rotaryEncoderEvent(int steps) = 0;
    virtual void touchEvent(int type, int x, int y) = 0;
    virtual void lcdFlushed() = 0;
    virtual void setTrainerTimeout(uint16_t ms) = 0;
    virtual void sendInternalModuleTelemetry(const quint8 protocol, const QByteArray data) = 0;
    virtual void sendExternalModuleTelemetry(const quint8 protocol, const QByteArray data) = 0;
    virtual void setLuaStateReloadPermanentScripts() = 0;
    virtual void addTracebackDevice(QIODevice * device) = 0;
    virtual void removeTracebackDevice(QIODevice * device) = 0;
    virtual void receiveAuxSerialData(const quint8 port_num, const QByteArray & data) = 0;

  signals:

    void started();
    void stopped();
    void heartbeat(qint32 loops, qint64 timestamp);
    void runtimeError(const QString & error);
    void lcdChange(bool backlightEnable);
    void phaseChanged(qint8 phase, const QString & name);
    void channelOutValueChange(quint8 index, qint32 value, qint32 limit);
    void channelMixValueChange(quint8 index, qint32 value, qint32 limit);
    void virtualSwValueChange(quint8 index, qint32 value);
    void trimValueChange(quint8 index, qint32 value);
    void trimRangeChange(quint8 index, qint32 min, qint16 max);
    void gVarValueChange(quint8 index, qint32 value);
    void outputValueChange(int type, quint8 index, qint32 value);
    void auxSerialSendData(const quint8 port_num, const QByteArray & data);
    void auxSerialSetEncoding(const quint8 port_num, const quint8 encoding);
    void auxSerialSetBaudrate(const quint8 port_num, const quint32 baudrate);
    void auxSerialStart(const quint8 port_num);
    void auxSerialStop(const quint8 port_num);
};

class SimulatorFactory {

  public:
    virtual ~SimulatorFactory() { }
    virtual QString name() = 0;
    virtual Board::Type type() = 0;
    virtual SimulatorInterface *create() = 0;
};

class SimulatorLoader
{
  public:
    static void registerSimulators();
    static void unregisterSimulators();
    static QStringList getAvailableSimulators();
    static QString findSimulatorByName(const QString & name);
    static SimulatorInterface * loadSimulator(const QString & name);
    static bool unloadSimulator(const QString & name);

  protected:
    typedef SimulatorFactory * (*RegisterSimulator)();

    static int registerSimulators(const QDir & dir);
    static QMap<QString, QLibrary *> registeredSimulators;
};
