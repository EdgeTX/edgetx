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

#include "wasmsimulatorinterface.h"

#include <QDebug>
#include <QFile>
#include <QElapsedTimer>

// WAMR native callback: called by WASM module to get analog values
static uint32_t host_simuGetAnalog(wasm_exec_env_t exec_env, uint32_t idx)
{
  auto * inst = wasm_runtime_get_module_inst(exec_env);
  auto * iface = static_cast<WasmSimulatorInterface *>(
      wasm_runtime_get_custom_data(inst));
  if (iface) {
    int16_t raw = iface->getAnalogValue((uint8_t)idx);
    return (uint32_t)(uint16_t)(raw * 2 + 2048);
  }
  return 0;
}

// WAMR native callback: called by WASM module to queue audio
static void host_simuQueueAudio(wasm_exec_env_t exec_env, uint32_t buf_offset,
                                uint32_t len)
{
  // Stub for increment 1
  Q_UNUSED(exec_env);
  Q_UNUSED(buf_offset);
  Q_UNUSED(len);
}

static bool s_wamrInitialized = false;

static NativeSymbol s_nativeSymbols[] = {
    {"simuGetAnalog", (void *)host_simuGetAnalog, "(i)i", nullptr},
    {"simuQueueAudio", (void *)host_simuQueueAudio, "(*~)", nullptr},
};

WasmSimulatorInterface::WasmSimulatorInterface(const QString & wasmPath,
                                               const QString & boardName,
                                               Board::Type boardType)
    : SimulatorInterface(),
      m_wasmPath(wasmPath),
      m_boardName(boardName),
      m_boardType(boardType)
{
}

WasmSimulatorInterface::~WasmSimulatorInterface()
{
  if (isRunning())
    stop();

  unloadModule();

  delete[] m_lcdBuffer;
  delete m_timer10ms;
}

QString WasmSimulatorInterface::name()
{
  return m_boardName;
}

bool WasmSimulatorInterface::isRunning()
{
  if (!m_fnIsRunning || !m_execEnv)
    return false;

  QMutexLocker lckr(&m_mutex);
  uint32_t argv[1] = {0};
  if (wasm_runtime_call_wasm(m_execEnv, m_fnIsRunning, 0, argv))
    return argv[0] != 0;
  return false;
}

void WasmSimulatorInterface::readRadioData(QByteArray & dest)
{
  Q_UNUSED(dest);
}

uint8_t * WasmSimulatorInterface::getLcd()
{
  return m_lcdBuffer;
}

uint8_t WasmSimulatorInterface::getSensorInstance(uint16_t id,
                                                  uint8_t defaultValue)
{
  Q_UNUSED(id);
  return defaultValue;
}

uint16_t WasmSimulatorInterface::getSensorRatio(uint16_t id)
{
  Q_UNUSED(id);
  return 0;
}

const int WasmSimulatorInterface::getCapability(Capability cap)
{
  Q_UNUSED(cap);
  return 0;
}

bool WasmSimulatorInterface::loadModule()
{
  // Initialize WAMR runtime (once)
  if (!s_wamrInitialized) {
    if (!wasm_runtime_init()) {
      qWarning() << "Failed to initialize WAMR runtime";
      return false;
    }
    wasm_runtime_set_log_level(WASM_LOG_LEVEL_WARNING);
    wasm_runtime_register_natives("env", s_nativeSymbols,
                                  sizeof(s_nativeSymbols) /
                                      sizeof(NativeSymbol));
    s_wamrInitialized = true;
  }

  // Read .wasm file
  QFile file(m_wasmPath);
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open WASM file:" << m_wasmPath;
    return false;
  }
  m_wasmBinary = file.readAll();
  file.close();

  // Load module
  char errorBuf[128];
  m_module = wasm_runtime_load((uint8_t *)m_wasmBinary.data(),
                               m_wasmBinary.size(), errorBuf, sizeof(errorBuf));
  if (!m_module) {
    qWarning() << "Failed to load WASM module:" << errorBuf;
    return false;
  }

  // Set up WASI (filesystem access for SD card and settings)
  QByteArray sdPathUtf8 = m_sdPath.toUtf8();
  QByteArray settingsPathUtf8 = m_settingsPath.toUtf8();
  {
    const char * dirList[2] = {};
    int nDirs = 0;
    if (!m_sdPath.isEmpty()) {
      dirList[nDirs++] = sdPathUtf8.constData();
    }
    if (!m_settingsPath.isEmpty() && m_settingsPath != m_sdPath) {
      dirList[nDirs++] = settingsPathUtf8.constData();
    }
    if (nDirs > 0) {
      wasm_runtime_set_wasi_args(m_module, dirList, nDirs, nullptr, 0,
                                 nullptr, 0, nullptr, 0);
    }
  }

  // Instantiate
  uint32_t stackSize = 256 * 1024;
  uint32_t heapSize = 16 * 1024 * 1024;
  m_moduleInst = wasm_runtime_instantiate(m_module, stackSize, heapSize,
                                          errorBuf, sizeof(errorBuf));
  if (!m_moduleInst) {
    qWarning() << "Failed to instantiate WASM module:" << errorBuf;
    return false;
  }

  // Store pointer to this instance for native callbacks
  wasm_runtime_set_custom_data(m_moduleInst, this);

  // Create execution environment
  m_execEnv =
      wasm_runtime_create_exec_env(m_moduleInst, stackSize);
  if (!m_execEnv) {
    qWarning() << "Failed to create WASM execution environment";
    return false;
  }

  return resolveExports();
}

void WasmSimulatorInterface::unloadModule()
{
  if (m_execEnv) {
    wasm_runtime_destroy_exec_env(m_execEnv);
    m_execEnv = nullptr;
  }
  if (m_moduleInst) {
    wasm_runtime_deinstantiate(m_moduleInst);
    m_moduleInst = nullptr;
  }
  if (m_module) {
    wasm_runtime_unload(m_module);
    m_module = nullptr;
  }
  m_wasmBinary.clear();
}

bool WasmSimulatorInterface::resolveExports()
{
  m_fnInit = wasm_runtime_lookup_function(m_moduleInst, "simuInit");
  m_fnStart = wasm_runtime_lookup_function(m_moduleInst, "simuStart");
  m_fnStop = wasm_runtime_lookup_function(m_moduleInst, "simuStop");
  m_fnIsRunning = wasm_runtime_lookup_function(m_moduleInst, "simuIsRunning");
  m_fnSetKey = wasm_runtime_lookup_function(m_moduleInst, "simuSetKey");
  m_fnSetTrim = wasm_runtime_lookup_function(m_moduleInst, "simuSetTrim");
  m_fnSetSwitch = wasm_runtime_lookup_function(m_moduleInst, "simuSetSwitch");
  m_fnLcdChanged =
      wasm_runtime_lookup_function(m_moduleInst, "simuLcdChanged");
  m_fnLcdCopy = wasm_runtime_lookup_function(m_moduleInst, "simuLcdCopy");
  m_fnLcdGetWidth =
      wasm_runtime_lookup_function(m_moduleInst, "simuLcdGetWidth");
  m_fnLcdGetHeight =
      wasm_runtime_lookup_function(m_moduleInst, "simuLcdGetHeight");
  m_fnLcdGetDepth =
      wasm_runtime_lookup_function(m_moduleInst, "simuLcdGetDepth");
  m_fnTouchDown =
      wasm_runtime_lookup_function(m_moduleInst, "simuTouchDown");
  m_fnTouchUp =
      wasm_runtime_lookup_function(m_moduleInst, "simuTouchUp");
  m_fnFatfsSetPaths =
      wasm_runtime_lookup_function(m_moduleInst, "simuFatfsSetPaths");
  m_fnMalloc = wasm_runtime_lookup_function(m_moduleInst, "malloc");
  m_fnFree = wasm_runtime_lookup_function(m_moduleInst, "free");

  if (!m_fnInit || !m_fnStart || !m_fnStop || !m_fnIsRunning ||
      !m_fnLcdChanged || !m_fnLcdCopy || !m_fnLcdGetWidth ||
      !m_fnLcdGetHeight || !m_fnLcdGetDepth || !m_fnMalloc || !m_fnFree) {
    qWarning() << "Failed to resolve required WASM exports";
    return false;
  }

  return true;
}

void WasmSimulatorInterface::init()
{
  if (isRunning())
    return;

  if (!m_moduleInst && !loadModule()) {
    qWarning() << "Failed to load WASM module for" << m_boardName;
    return;
  }

  if (!m_timer10ms) {
    m_timer10ms = new QTimer();
    m_timer10ms->setInterval(10);
    connect(m_timer10ms, &QTimer::timeout, this,
            &WasmSimulatorInterface::run);
    connect(this, SIGNAL(started()), m_timer10ms, SLOT(start()));
    connect(this, SIGNAL(stopped()), m_timer10ms, SLOT(stop()));
  }

  m_stopRequested = false;
  memset(m_analogValues, 0, sizeof(m_analogValues));

  // Call simuInit
  QMutexLocker lckr(&m_mutex);
  wasm_runtime_call_wasm(m_execEnv, m_fnInit, 0, nullptr);

  // Query LCD dimensions
  uint32_t argv[1] = {0};
  if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdGetWidth, 0, argv))
    m_lcdWidth = argv[0];
  argv[0] = 0;
  if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdGetHeight, 0, argv))
    m_lcdHeight = argv[0];
  argv[0] = 0;
  if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdGetDepth, 0, argv))
    m_lcdDepth = argv[0];

  // Allocate host-side LCD buffer
  if (m_lcdWidth > 0 && m_lcdHeight > 0) {
    uint32_t bitsPerPixel = m_lcdDepth;
    if (bitsPerPixel == 1)
      m_lcdBufferSize = m_lcdWidth * ((m_lcdHeight + 7) / 8);
    else if (bitsPerPixel == 4)
      m_lcdBufferSize = m_lcdWidth * m_lcdHeight * 4 / 8;
    else
      m_lcdBufferSize = m_lcdWidth * m_lcdHeight * (bitsPerPixel / 8);

    delete[] m_lcdBuffer;
    m_lcdBuffer = new uint8_t[m_lcdBufferSize];
    memset(m_lcdBuffer, 0, m_lcdBufferSize);
  }

  qDebug() << "WASM simulator" << m_boardName << "initialized: LCD"
           << m_lcdWidth << "x" << m_lcdHeight << "depth" << m_lcdDepth;
}

void WasmSimulatorInterface::start(const char * filename, bool tests)
{
  Q_UNUSED(filename);
  if (isRunning())
    return;

  QMutexLocker lckr(&m_mutex);

  // Set up FATFS paths before starting (prevents STORAGE WARNING)
  if (m_fnFatfsSetPaths && m_fnMalloc && m_fnFree) {
    QByteArray sdPathUtf8 = m_sdPath.toUtf8();
    QByteArray settingsPathUtf8 = m_settingsPath.toUtf8();
    uint32_t sdLen = sdPathUtf8.size() + 1;
    uint32_t settingsLen = settingsPathUtf8.size() + 1;

    // Allocate WASM memory for both strings
    uint32_t allocArgv[1] = {sdLen};
    uint32_t wasmSdPath = 0, wasmSettingsPath = 0;

    if (wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) &&
        allocArgv[0]) {
      wasmSdPath = allocArgv[0];
      void * nativeSd =
          wasm_runtime_addr_app_to_native(m_moduleInst, wasmSdPath);
      if (nativeSd)
        memcpy(nativeSd, sdPathUtf8.constData(), sdLen);
    }

    allocArgv[0] = settingsLen;
    if (wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) &&
        allocArgv[0]) {
      wasmSettingsPath = allocArgv[0];
      void * nativeSettings =
          wasm_runtime_addr_app_to_native(m_moduleInst, wasmSettingsPath);
      if (nativeSettings)
        memcpy(nativeSettings, settingsPathUtf8.constData(), settingsLen);
    }

    if (wasmSdPath && wasmSettingsPath) {
      uint32_t pathArgv[2] = {wasmSdPath, wasmSettingsPath};
      if (!wasm_runtime_call_wasm(m_execEnv, m_fnFatfsSetPaths, 2, pathArgv)) {
        qWarning() << "WASM simuFatfsSetPaths failed:"
                   << wasm_runtime_get_exception(m_moduleInst);
      }
    }

    // Free WASM string buffers
    if (wasmSdPath) {
      uint32_t freeArgv[1] = {wasmSdPath};
      wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
    }
    if (wasmSettingsPath) {
      uint32_t freeArgv[1] = {wasmSettingsPath};
      wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
    }
  }

  uint32_t argv[1] = {(uint32_t)tests};
  if (!wasm_runtime_call_wasm(m_execEnv, m_fnStart, 1, argv)) {
    qWarning() << "WASM simuStart failed:"
               << wasm_runtime_get_exception(m_moduleInst);
    return;
  }

  emit started();
  QTimer::singleShot(0, this, SLOT(run()));
}

void WasmSimulatorInterface::stop()
{
  if (!isRunning())
    return;

  m_stopRequested = true;

  QMutexLocker lckr(&m_mutex);
  wasm_runtime_call_wasm(m_execEnv, m_fnStop, 0, nullptr);

  emit stopped();
}

void WasmSimulatorInterface::setSdPath(const QString & sdPath,
                                       const QString & settingsPath)
{
  m_sdPath = sdPath;
  m_settingsPath = settingsPath;
}

void WasmSimulatorInterface::setVolumeGain(const int value)
{
  Q_UNUSED(value);
}

void WasmSimulatorInterface::setRadioData(const QByteArray & data)
{
  Q_UNUSED(data);
}

void WasmSimulatorInterface::setAnalogValue(uint8_t index, int16_t value)
{
  if (index < MAX_ANALOGS)
    m_analogValues[index] = value;
}

int16_t WasmSimulatorInterface::getAnalogValue(uint8_t index)
{
  if (index < MAX_ANALOGS)
    return m_analogValues[index];
  return 0;
}

void WasmSimulatorInterface::setKey(uint8_t key, bool state)
{
  if (!m_fnSetKey || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[2] = {key, (uint32_t)state};
  wasm_runtime_call_wasm(m_execEnv, m_fnSetKey, 2, argv);
}

void WasmSimulatorInterface::setSwitch(uint8_t swtch, int8_t state)
{
  if (!m_fnSetSwitch || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[2] = {swtch, (uint32_t)(int32_t)state};
  wasm_runtime_call_wasm(m_execEnv, m_fnSetSwitch, 2, argv);
}

void WasmSimulatorInterface::setTrim(unsigned int idx, int value)
{
  Q_UNUSED(idx);
  Q_UNUSED(value);
  // TODO: needs WASM export for setTrim (value-based, not button press)
}

void WasmSimulatorInterface::setTrimSwitch(uint8_t trim, bool state)
{
  if (!m_fnSetTrim || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[2] = {trim, (uint32_t)state};
  wasm_runtime_call_wasm(m_execEnv, m_fnSetTrim, 2, argv);
}

void WasmSimulatorInterface::setTrainerInput(unsigned int inputNumber,
                                             int16_t value)
{
  Q_UNUSED(inputNumber);
  Q_UNUSED(value);
}

void WasmSimulatorInterface::setInputValue(int type, uint8_t index,
                                           int16_t value)
{
  switch (type) {
    case INPUT_SRC_ANALOG:
    case INPUT_SRC_STICK:
    case INPUT_SRC_KNOB:
    case INPUT_SRC_SLIDER:
      setAnalogValue(index, value);
      break;
    case INPUT_SRC_TXVIN:
      setAnalogValue(index, value);
      emit txBatteryVoltageChanged((unsigned int)value);
      break;
    case INPUT_SRC_SWITCH:
      setSwitch(index, (int8_t)value);
      break;
    case INPUT_SRC_TRIM_SW:
      setTrimSwitch(index, (bool)value);
      break;
    case INPUT_SRC_TRIM:
      setTrim(index, value);
      break;
    case INPUT_SRC_KEY:
      setKey(index, (bool)value);
      break;
    case INPUT_SRC_TRAINER:
      setTrainerInput(index, value);
      break;
    default:
      break;
  }
}

void WasmSimulatorInterface::rotaryEncoderEvent(int steps)
{
  Q_UNUSED(steps);
  // TODO: needs WASM export
}

void WasmSimulatorInterface::touchEvent(int type, int x, int y)
{
  if (!m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  if ((type == 1 || type == 2) && m_fnTouchDown) {  // TouchDown or TouchSlide
    uint32_t argv[2] = {(uint32_t)(int16_t)x, (uint32_t)(int16_t)y};
    wasm_runtime_call_wasm(m_execEnv, m_fnTouchDown, 2, argv);
  } else if (type == 0 && m_fnTouchUp) {  // TouchUp
    wasm_runtime_call_wasm(m_execEnv, m_fnTouchUp, 0, nullptr);
  }
}

void WasmSimulatorInterface::lcdFlushed()
{
  // TODO: needs WASM export
}

void WasmSimulatorInterface::setTrainerTimeout(uint16_t ms)
{
  Q_UNUSED(ms);
}

void WasmSimulatorInterface::sendInternalModuleTelemetry(const quint8 protocol,
                                                         const QByteArray data)
{
  Q_UNUSED(protocol);
  Q_UNUSED(data);
}

void WasmSimulatorInterface::sendExternalModuleTelemetry(const quint8 protocol,
                                                         const QByteArray data)
{
  Q_UNUSED(protocol);
  Q_UNUSED(data);
}

void WasmSimulatorInterface::setLuaStateReloadPermanentScripts()
{
}

void WasmSimulatorInterface::addTracebackDevice(QIODevice * device)
{
  Q_UNUSED(device);
}

void WasmSimulatorInterface::removeTracebackDevice(QIODevice * device)
{
  Q_UNUSED(device);
}

void WasmSimulatorInterface::receiveAuxSerialData(const quint8 port_num,
                                                  const QByteArray & data)
{
  Q_UNUSED(port_num);
  Q_UNUSED(data);
}

void WasmSimulatorInterface::run()
{
  static uint32_t loops = 0;

  if (m_stopRequested)
    return;

  if (!isRunning()) {
    emit stopped();
    return;
  }

  ++loops;

  // Check LCD
  if (m_fnLcdChanged && m_execEnv) {
    QMutexLocker lckr(&m_mutex);
    uint32_t argv[1] = {0};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdChanged, 0, argv) &&
        argv[0]) {
      // LCD changed - copy buffer from WASM memory
      if (m_lcdBuffer && m_lcdBufferSize > 0 && m_fnMalloc && m_fnFree) {
        // Allocate buffer in WASM memory
        uint32_t allocArgv[1] = {m_lcdBufferSize};
        if (wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) &&
            allocArgv[0]) {
          uint32_t wasmBuf = allocArgv[0];

          // Call simuLcdCopy(buf, maxLen)
          uint32_t copyArgv[2] = {wasmBuf, m_lcdBufferSize};
          if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdCopy, 2, copyArgv)) {
            uint32_t bytesWritten = copyArgv[0];
            // Copy from WASM linear memory to host buffer
            void * nativePtr =
                wasm_runtime_addr_app_to_native(m_moduleInst, wasmBuf);
            if (nativePtr && bytesWritten > 0) {
              memcpy(m_lcdBuffer, nativePtr,
                     qMin(bytesWritten, m_lcdBufferSize));
            }
          }

          // Free WASM buffer
          uint32_t freeArgv[1] = {wasmBuf};
          wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
        }
      }
      emit lcdChange(true);
    }
  }

  // Heartbeat
  if (!(loops % (SIMULATOR_INTERFACE_HEARTBEAT_PERIOD / 10))) {
    emit heartbeat(loops, 0);
  }
}

/*
 * WasmSimulatorFactory
 */

WasmSimulatorFactory::WasmSimulatorFactory(const QString & wasmPath,
                                           const QString & boardName,
                                           Board::Type boardType)
    : m_wasmPath(wasmPath), m_boardName(boardName), m_boardType(boardType)
{
}

QString WasmSimulatorFactory::name()
{
  return m_boardName;
}

Board::Type WasmSimulatorFactory::type()
{
  return m_boardType;
}

SimulatorInterface * WasmSimulatorFactory::create()
{
  return new WasmSimulatorInterface(m_wasmPath, m_boardName, m_boardType);
}
