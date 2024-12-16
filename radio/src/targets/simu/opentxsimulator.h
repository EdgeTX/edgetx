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

#include "simulatorinterface.h"
#include "dataconstants.h"

#include <QMutex>
#include <QObject>
#include <QTimer>

#if defined __GNUC__
  #define DLLEXPORT
#else
  #define DLLEXPORT __declspec(dllexport)
#endif

class DLLEXPORT OpenTxSimulator : public SimulatorInterface
{
  Q_OBJECT

  public:

    enum TouchEvent {
      TouchUp=0,
      TouchDown,
      TouchSlide
    };

    OpenTxSimulator();
    virtual ~OpenTxSimulator();

    virtual QString name();
    virtual bool isRunning();
    virtual void readRadioData(QByteArray & dest);
    virtual uint8_t * getLcd();
    virtual uint8_t getSensorInstance(uint16_t id, uint8_t defaultValue = 0);
    virtual uint16_t getSensorRatio(uint16_t id);
    virtual const int getCapability(Capability cap);

    static QVector<QIODevice *> tracebackDevices;

    void drv_auxSerialInit(quint8 port_num, const etx_serial_init* dev);
    void drv_auxSerialDeinit(quint8 port_num);
    void drv_auxSerialSetBaudrate(quint8 port_num, uint32_t baudrate);
    void drv_auxSerialSendByte(quint8 port_num, uint8_t byte);
    void drv_auxSerialSendBuffer(quint8 port_num, const uint8_t* data, uint32_t size);
    int drv_auxSerialGetByte(quint8 port_num, uint8_t* data);

  public slots:

    virtual void init();
    virtual void start(const char * filename = nullptr, bool tests = true);
    virtual void stop();
    virtual void setSdPath(const QString & sdPath = "", const QString & settingsPath = "");
    virtual void setVolumeGain(const int value);
    virtual void setRadioData(const QByteArray & data);
    virtual void setAnalogValue(uint8_t index, int16_t value);
    virtual void setKey(uint8_t key, bool state);
    virtual void setSwitch(uint8_t swtch, int8_t state);
    virtual void setTrim(unsigned int idx, int value);
    virtual void setTrimSwitch(uint8_t trim, bool state);
    virtual void setTrainerInput(unsigned int inputNumber, int16_t value);
    virtual void setInputValue(int type, uint8_t index, int16_t value);
    virtual void rotaryEncoderEvent(int steps);
    virtual void touchEvent(int type, int x, int y);
    virtual void lcdFlushed();
    virtual void setTrainerTimeout(uint16_t ms);
    virtual void sendTelemetry(const uint8_t module, const uint8_t protocol, const QByteArray data);
    virtual void sendInternalModuleTelemetry(const uint8_t protocol, const QByteArray data);
    virtual void sendExternalModuleTelemetry(const uint8_t protocol, const QByteArray data);
    virtual void setLuaStateReloadPermanentScripts();
    virtual void addTracebackDevice(QIODevice * device);
    virtual void removeTracebackDevice(QIODevice * device);
    virtual void receiveAuxSerialData(const quint8 port_num, const QByteArray & data);

  protected slots:
    void run();

  protected:

    bool isStopRequested();
    void setStopRequested(bool stop);
    bool checkLcdChanged();
    void checkOutputsChanged();
    uint8_t getStickMode();
    const char * getPhaseName(unsigned int phase);
    const QString getCurrentPhaseName();
    const char * getError();
    const int voltageToAdc(const int volts);

    QString simuSdDirectory;
    QString simuSettingsDirectory;
    QTimer * m_timer10ms;
    QMutex m_mtxStopReq;
    QMutex m_mtxSimuMain;
    QMutex m_mtxRadioData;
    QMutex m_mtxSettings;
    QMutex m_mtxTbDevices;
    int volumeGain;
    bool m_resetOutputsData;
    bool m_stopRequested;

};
