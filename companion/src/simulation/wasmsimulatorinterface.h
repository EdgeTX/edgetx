/*
 * Copyright (C) EdgeTX
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

#include <QMutex>
#include <QTimer>

#include "wasm_export.h"

class WasmSimulatorInterface : public SimulatorInterface
{
  Q_OBJECT

  public:
    WasmSimulatorInterface(const QString & wasmPath, const QString & boardName,
                           Board::Type boardType);
    virtual ~WasmSimulatorInterface();

    QString name() override;
    bool isRunning() override;
    void readRadioData(QByteArray & dest) override;
    uint8_t * getLcd() override;
    uint8_t getSensorInstance(uint16_t id, uint8_t defaultValue = 0) override;
    uint16_t getSensorRatio(uint16_t id) override;
    const int getCapability(Capability cap) override;

  public slots:
    void init() override;
    void start(const char * filename = nullptr, bool tests = true) override;
    void stop() override;
    void setSdPath(const QString & sdPath = "",
                   const QString & settingsPath = "") override;
    void setVolumeGain(const int value) override;
    void setRadioData(const QByteArray & data) override;
    void setAnalogValue(uint8_t index, int16_t value) override;
    void setKey(uint8_t key, bool state) override;
    void setSwitch(uint8_t swtch, int8_t state) override;
    void setTrim(unsigned int idx, int value) override;
    void setTrimSwitch(uint8_t trim, bool state) override;
    void setTrainerInput(unsigned int inputNumber, int16_t value) override;
    void setInputValue(int type, uint8_t index, int16_t value) override;
    void rotaryEncoderEvent(int steps) override;
    void touchEvent(int type, int x, int y) override;
    void lcdFlushed() override;
    void setTrainerTimeout(uint16_t ms) override;
    void sendInternalModuleTelemetry(const quint8 protocol,
                                     const QByteArray data) override;
    void sendExternalModuleTelemetry(const quint8 protocol,
                                     const QByteArray data) override;
    void setLuaStateReloadPermanentScripts() override;
    void addTracebackDevice(QIODevice * device) override;
    void removeTracebackDevice(QIODevice * device) override;
    void receiveAuxSerialData(const quint8 port_num,
                              const QByteArray & data) override;

    // Called by WASM import simuGetAnalog
    int16_t getAnalogValue(uint8_t index);

  protected slots:
    void run();

  protected:
    bool loadModule();
    void unloadModule();
    bool resolveExports();

    QString m_wasmPath;
    QString m_boardName;
    Board::Type m_boardType;
    QString m_sdPath;
    QString m_settingsPath;

    QTimer * m_timer10ms = nullptr;
    QMutex m_mutex;
    bool m_stopRequested = false;

    // Host-side analog values (polled by WASM via simuGetAnalog)
    static constexpr int MAX_ANALOGS = 32;
    int16_t m_analogValues[MAX_ANALOGS] = {};

    // Host-side LCD buffer
    uint8_t * m_lcdBuffer = nullptr;
    uint32_t m_lcdBufferSize = 0;
    uint32_t m_lcdWidth = 0;
    uint32_t m_lcdHeight = 0;
    uint32_t m_lcdDepth = 0;

    // WAMR handles
    wasm_module_t m_module = nullptr;
    wasm_module_inst_t m_moduleInst = nullptr;
    wasm_exec_env_t m_execEnv = nullptr;
    QByteArray m_wasmBinary;

    // Cached WASM function references
    wasm_function_inst_t m_fnInit = nullptr;
    wasm_function_inst_t m_fnStart = nullptr;
    wasm_function_inst_t m_fnStop = nullptr;
    wasm_function_inst_t m_fnIsRunning = nullptr;
    wasm_function_inst_t m_fnSetKey = nullptr;
    wasm_function_inst_t m_fnSetTrim = nullptr;
    wasm_function_inst_t m_fnSetSwitch = nullptr;
    wasm_function_inst_t m_fnLcdChanged = nullptr;
    wasm_function_inst_t m_fnLcdCopy = nullptr;
    wasm_function_inst_t m_fnLcdGetWidth = nullptr;
    wasm_function_inst_t m_fnLcdGetHeight = nullptr;
    wasm_function_inst_t m_fnLcdGetDepth = nullptr;
    wasm_function_inst_t m_fnTouchDown = nullptr;
    wasm_function_inst_t m_fnTouchUp = nullptr;
    wasm_function_inst_t m_fnFatfsSetPaths = nullptr;
    wasm_function_inst_t m_fnMalloc = nullptr;
    wasm_function_inst_t m_fnFree = nullptr;
};

class WasmSimulatorFactory : public SimulatorFactory
{
  public:
    WasmSimulatorFactory(const QString & wasmPath, const QString & boardName,
                         Board::Type boardType);

    QString name() override;
    Board::Type type() override;
    SimulatorInterface * create() override;

  private:
    QString m_wasmPath;
    QString m_boardName;
    Board::Type m_boardType;
};
