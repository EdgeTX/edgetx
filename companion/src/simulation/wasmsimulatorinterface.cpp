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
#include <QTimer>

// WAMR native callback: called by WASM module to get analog values in
// ADC range (0..4096, center=2048).  The WASM ADC driver does a direct
// passthrough, so the host must provide values in ADC range.
// UI widgets emit -1024..+1024 (center=0); we convert here.
static uint32_t host_simuGetAnalog(wasm_exec_env_t exec_env, uint32_t idx)
{
  auto * inst = wasm_runtime_get_module_inst(exec_env);
  auto * iface = static_cast<WasmSimulatorInterface *>(
      wasm_runtime_get_custom_data(inst));
  if (iface) {
    int16_t raw = iface->getAnalogValue((uint8_t)idx);
    uint16_t adc = (uint16_t)((raw + 1024) * 2); // -1024..+1024 → 0..4096
    return (uint32_t)adc;
  }
  return 2048;
}

// WAMR native callback: called by WASM module to queue audio PCM data
static void host_simuQueueAudio(wasm_exec_env_t exec_env, uint8_t * buf,
                                uint32_t len)
{
  auto * inst = wasm_runtime_get_module_inst(exec_env);
  auto * iface = static_cast<WasmSimulatorInterface *>(
      wasm_runtime_get_custom_data(inst));
  if (iface && buf && len > 0)
    iface->queueAudio(buf, len);
}

// WAMR native callback: called by WASM module to send trace/debug output
static void host_simuTrace(wasm_exec_env_t exec_env, const char * text)
{
  auto * inst = wasm_runtime_get_module_inst(exec_env);
  auto * iface = static_cast<WasmSimulatorInterface *>(
      wasm_runtime_get_custom_data(inst));
  if (iface && text)
    iface->writeTrace(text);
}

static bool s_wamrInitialized = false;

static void host_simuLcdNotify(wasm_exec_env_t exec_env)
{
  auto * inst = wasm_runtime_get_module_inst(exec_env);
  auto * iface = static_cast<WasmSimulatorInterface *>(
      wasm_runtime_get_custom_data(inst));
  if (iface)
    iface->notifyLcdReady();
}

static NativeSymbol s_nativeSymbols[] = {
    {"simuGetAnalog", (void *)host_simuGetAnalog, "(i)i", nullptr},
    {"simuQueueAudio", (void *)host_simuQueueAudio, "(*~)", nullptr},
    {"simuTrace", (void *)host_simuTrace, "($)", nullptr},
    {"simuLcdNotify", (void *)host_simuLcdNotify, "()", nullptr},
};

WasmSimulatorInterface::WasmSimulatorInterface(const QString & wasmPath,
                                               const QString & boardName,
                                               Board::Type boardType)
    : SimulatorInterface(),
      m_wasmPath(wasmPath),
      m_boardName(boardName),
      m_boardType(boardType),
      m_lastHaptic(0)
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
  if (!m_fnGetCapability || !m_execEnv)
    return 0;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[1] = {(uint32_t)cap};
  if (wasm_runtime_call_wasm(m_execEnv, m_fnGetCapability, 1, argv))
    return (int32_t)argv[0];
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

  // Set up WASI (filesystem access for SD card and settings).
  // NOTE: wasm_runtime_set_wasi_args() only stores pointers; the actual WASI
  // init happens inside wasm_runtime_instantiate(), so all buffers must remain
  // alive until after instantiation completes.
  QByteArray sdPathUtf8 = m_sdPath.toUtf8();
  QByteArray settingsPathUtf8 = m_settingsPath.toUtf8();
  const char * dirList[2] = {};
  int nDirs = 0;
  if (!m_sdPath.isEmpty()) {
    dirList[nDirs++] = sdPathUtf8.constData();
  }
  if (!m_settingsPath.isEmpty() && m_settingsPath != m_sdPath) {
    dirList[nDirs++] = settingsPathUtf8.constData();
  }
  if (nDirs > 0) {
    for (int i = 0; i < nDirs; ++i)
      qDebug() << "WASI preopen[" << i << "]:" << dirList[i];
    wasm_runtime_set_wasi_args(m_module, dirList, nDirs, nullptr, 0,
                               nullptr, 0, nullptr, 0);
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
  if (m_execEnv && m_fnFree) {
    if (m_wasmLcdBuf) {
      uint32_t freeArgv[1] = {m_wasmLcdBuf};
      wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
      m_wasmLcdBuf = 0;
    }
    if (m_wasmScratchBuf) {
      uint32_t freeArgv[1] = {m_wasmScratchBuf};
      wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
      m_wasmScratchBuf = 0;
    }
  }
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

  // Fully destroy the WAMR runtime so that its per-thread signal
  // environment is re-initialised on the next loadModule() call.
  // Without this, a new Qt thread (simuThread) will not have the
  // thread-local signal env set up and instantiation will fail with
  // "thread signal env not inited".
  if (s_wamrInitialized) {
    wasm_runtime_destroy();
    s_wamrInitialized = false;
  }
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
  m_fnRotaryEncoderEvent =
      wasm_runtime_lookup_function(m_moduleInst, "simuRotaryEncoderEvent");
  m_fnGetCapability =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetCapability");

  // Output value exports (bulk copy)
  m_fnGetNumChannels =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetNumChannels");
  m_fnCopyChannelOutputs =
      wasm_runtime_lookup_function(m_moduleInst, "simuCopyChannelOutputs");
  m_fnCopyMixOutputs =
      wasm_runtime_lookup_function(m_moduleInst, "simuCopyMixOutputs");
  m_fnGetNumLogicalSwitches =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetNumLogicalSwitches");
  m_fnCopyLogicalSwitches =
      wasm_runtime_lookup_function(m_moduleInst, "simuCopyLogicalSwitches");
  m_fnGetTrimValue =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetTrimValue");
  m_fnGetTrimRange =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetTrimRange");
  m_fnGetFlightMode =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetFlightMode");
  m_fnGetNumGVars =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetNumGVars");
  m_fnGetNumFlightModes =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetNumFlightModes");
  m_fnGetGVar =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetGVar");

  // Phase 4: telemetry, trim, lua, lcd, trainer
  m_fnSetTrimValue =
      wasm_runtime_lookup_function(m_moduleInst, "simuSetTrimValue");
  m_fnSendTelemetry =
      wasm_runtime_lookup_function(m_moduleInst, "simuSendTelemetry");
  m_fnLuaReloadPermanentScripts =
      wasm_runtime_lookup_function(m_moduleInst, "simuLuaReloadPermanentScripts");
  m_fnLcdFlushed =
      wasm_runtime_lookup_function(m_moduleInst, "simuLcdFlushed");
  m_fnGetMaxTrainerChannels =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetMaxTrainerChannels");
  m_fnCopyTrainerInput =
      wasm_runtime_lookup_function(m_moduleInst, "simuCopyTrainerInput");
  m_fnSetTrainerTimeout =
      wasm_runtime_lookup_function(m_moduleInst, "simuSetTrainerTimeout");

  // Backlight and function switch LEDs
  m_fnGetBacklightState =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetBacklightState");
  m_fnGetNumCustomSwitches =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetNumCustomSwitches");
  m_fnGetCustomSwitchColor =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetCustomSwitchColor");
  m_fnGetCustomSwitchIndex =
      wasm_runtime_lookup_function(m_moduleInst, "simuGetCustomSwitchIndex");

  m_fnGetHaptic = wasm_runtime_lookup_function(m_moduleInst, "simuGetHaptic");

  m_fnClearHaptic = wasm_runtime_lookup_function(m_moduleInst, "simuClearHaptic");

  m_fnMalloc = wasm_runtime_lookup_function(m_moduleInst, "malloc");
  m_fnFree = wasm_runtime_lookup_function(m_moduleInst, "free");

  if (!m_fnInit || !m_fnStart || !m_fnStop || !m_fnIsRunning ||
      !m_fnLcdCopy || !m_fnLcdGetWidth ||
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

    // Allocate persistent WASM-side buffer for simuLcdCopy
    if (m_fnMalloc) {
      uint32_t allocArgv[1] = {m_lcdBufferSize};
      if (wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) &&
          allocArgv[0]) {
        m_wasmLcdBuf = allocArgv[0];
      }
    }
  }

  // Allocate persistent scratch buffer in WASM memory for bulk copies.
  // Sized for max(channels * sizeof(int16_t), logical switches).
  m_wasmScratchSize = CPN_MAX_CHNOUT * sizeof(int16_t);
  if (CPN_MAX_LOGICAL_SWITCHES > m_wasmScratchSize)
    m_wasmScratchSize = CPN_MAX_LOGICAL_SWITCHES;
  if (m_fnMalloc) {
    uint32_t allocArgv[1] = {m_wasmScratchSize};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) &&
        allocArgv[0]) {
      m_wasmScratchBuf = allocArgv[0];
    }
  }

  m_lastOutputs.clear();
  m_resetOutputsData = true;

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

  initAudio();

  emit started();
  QTimer::singleShot(0, this, SLOT(run()));
}

void WasmSimulatorInterface::stop()
{
  if (!isRunning())
    return;

  m_stopRequested = true;
  deinitAudio();

  {
    QMutexLocker lckr(&m_mutex);
    wasm_runtime_call_wasm(m_execEnv, m_fnStop, 0, nullptr);
  }

  // Fully destroy the WASM module so the next init()/start() gets a clean
  // instance.  WASI threads created by the firmware's RTOS layer cannot be
  // reliably restarted within the same module instance.
  unloadModule();

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
  m_volumeGain = qBound(0, value * SDL_MIX_MAXVOLUME / 100, SDL_MIX_MAXVOLUME);
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
  if (!m_fnSetTrimValue || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[2] = {(uint32_t)idx, (uint32_t)(int32_t)value};
  wasm_runtime_call_wasm(m_execEnv, m_fnSetTrimValue, 2, argv);
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
  if (inputNumber < MAX_TRAINER_CH) {
    m_trainerValues[inputNumber] = value;
    m_trainerDirty = true;
  }
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
  if (!m_fnRotaryEncoderEvent || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[1] = {(uint32_t)(int32_t)steps};
  wasm_runtime_call_wasm(m_execEnv, m_fnRotaryEncoderEvent, 1, argv);
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
  if (!m_fnLcdFlushed || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  wasm_runtime_call_wasm(m_execEnv, m_fnLcdFlushed, 0, nullptr);
}

void WasmSimulatorInterface::notifyLcdReady()
{
  // Called from WAMR thread — coalesce multiple notifications into one
  // Qt event by only posting when the flag transitions from false to true.
  if (!m_lcdNotified.exchange(true)) {
    QMetaObject::invokeMethod(this, "onLcdNotify", Qt::QueuedConnection);
  }
}

void WasmSimulatorInterface::onLcdNotify()
{
  if (!m_lcdNotified.exchange(false))
    return;
  refreshLcd();
}

void WasmSimulatorInterface::refreshLcd()
{
  if (!m_fnLcdCopy || !m_execEnv || !m_wasmLcdBuf || !m_lcdBuffer)
    return;

  bool backlightOn = true;
  {
    QMutexLocker lckr(&m_mutex);

    uint32_t copyArgv[2] = {m_wasmLcdBuf, m_lcdBufferSize};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnLcdCopy, 2, copyArgv)) {
      uint32_t bytesWritten = copyArgv[0];
      void * nativePtr =
          wasm_runtime_addr_app_to_native(m_moduleInst, m_wasmLcdBuf);
      if (nativePtr && bytesWritten > 0) {
        memcpy(m_lcdBuffer, nativePtr, qMin(bytesWritten, m_lcdBufferSize));
      }
    }

    if (m_fnGetBacklightState) {
      uint32_t blArgv[1] = {0};
      if (wasm_runtime_call_wasm(m_execEnv, m_fnGetBacklightState, 0, blArgv))
        backlightOn = blArgv[0] != 0;
    }
  }

  // Emit outside mutex — onLcdChange() calls lcdFlushed() which re-acquires it
  emit lcdChange(backlightOn);
}

void WasmSimulatorInterface::setTrainerTimeout(uint16_t ms)
{
  if (!m_fnSetTrainerTimeout || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  uint32_t argv[1] = {(uint32_t)ms};
  wasm_runtime_call_wasm(m_execEnv, m_fnSetTrainerTimeout, 1, argv);
}

void WasmSimulatorInterface::sendInternalModuleTelemetry(const quint8 protocol,
                                                         const QByteArray data)
{
  if (!m_fnSendTelemetry || !m_execEnv || !m_fnMalloc || !m_fnFree ||
      data.isEmpty())
    return;

  QMutexLocker lckr(&m_mutex);
  uint32_t len = data.size();
  uint32_t allocArgv[1] = {len};
  if (!wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) ||
      !allocArgv[0])
    return;

  uint32_t wasmBuf = allocArgv[0];
  void * nativePtr = wasm_runtime_addr_app_to_native(m_moduleInst, wasmBuf);
  if (nativePtr)
    memcpy(nativePtr, data.constData(), len);

  uint32_t argv[4] = {0 /* INTERNAL_MODULE */, (uint32_t)protocol, wasmBuf, len};
  wasm_runtime_call_wasm(m_execEnv, m_fnSendTelemetry, 4, argv);

  uint32_t freeArgv[1] = {wasmBuf};
  wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
}

void WasmSimulatorInterface::sendExternalModuleTelemetry(const quint8 protocol,
                                                         const QByteArray data)
{
  if (!m_fnSendTelemetry || !m_execEnv || !m_fnMalloc || !m_fnFree ||
      data.isEmpty())
    return;

  QMutexLocker lckr(&m_mutex);
  uint32_t len = data.size();
  uint32_t allocArgv[1] = {len};
  if (!wasm_runtime_call_wasm(m_execEnv, m_fnMalloc, 1, allocArgv) ||
      !allocArgv[0])
    return;

  uint32_t wasmBuf = allocArgv[0];
  void * nativePtr = wasm_runtime_addr_app_to_native(m_moduleInst, wasmBuf);
  if (nativePtr)
    memcpy(nativePtr, data.constData(), len);

  uint32_t argv[4] = {1 /* EXTERNAL_MODULE */, (uint32_t)protocol, wasmBuf, len};
  wasm_runtime_call_wasm(m_execEnv, m_fnSendTelemetry, 4, argv);

  uint32_t freeArgv[1] = {wasmBuf};
  wasm_runtime_call_wasm(m_execEnv, m_fnFree, 1, freeArgv);
}

void WasmSimulatorInterface::setLuaStateReloadPermanentScripts()
{
  if (!m_fnLuaReloadPermanentScripts || !m_execEnv)
    return;
  QMutexLocker lckr(&m_mutex);
  wasm_runtime_call_wasm(m_execEnv, m_fnLuaReloadPermanentScripts, 0, nullptr);
}

void WasmSimulatorInterface::addTracebackDevice(QIODevice * device)
{
  QMutexLocker lckr(&m_mtxTbDevices);
  if (device && !m_tracebackDevices.contains(device))
    m_tracebackDevices.append(device);
}

void WasmSimulatorInterface::removeTracebackDevice(QIODevice * device)
{
  QMutexLocker lckr(&m_mtxTbDevices);
  m_tracebackDevices.removeAll(device);
}

void WasmSimulatorInterface::writeTrace(const char * text)
{
  QMutexLocker lckr(&m_mtxTbDevices);
  for (QIODevice * dev : m_tracebackDevices) {
    if (dev)
      dev->write(text);
  }
}

void WasmSimulatorInterface::initAudio()
{
  if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
      qWarning() << "Failed to init SDL audio:" << SDL_GetError();
      return;
    }
  }

  SDL_AudioSpec wanted = {};
  wanted.freq = 32000;  // AUDIO_SAMPLE_RATE
  wanted.format = AUDIO_S16SYS;
  wanted.channels = 1;
  wanted.samples = 1024;

  SDL_AudioSpec have = {};
  m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &wanted, &have, 0);
  if (m_audioDevice > 0) {
    qDebug() << "SDL audio opened: freq=" << have.freq
             << "format=" << have.format << "channels=" << have.channels
             << "samples=" << have.samples;
    SDL_PauseAudioDevice(m_audioDevice, 0);
  } else {
    qWarning() << "Failed to open SDL audio:" << SDL_GetError();
  }
}

void WasmSimulatorInterface::deinitAudio()
{
  if (m_audioDevice > 0) {
    SDL_CloseAudioDevice(m_audioDevice);
    m_audioDevice = 0;
  }
}

void WasmSimulatorInterface::queueAudio(const uint8_t * buf, uint32_t len)
{
  if (m_audioDevice == 0 || len == 0)
    return;

  // Apply companion volume gain (m_volumeGain is 0..SDL_MIX_MAXVOLUME)
  QByteArray scaled(len, 0);
  SDL_MixAudioFormat((uint8_t *)scaled.data(), buf, AUDIO_S16SYS, len,
                     m_volumeGain);
  SDL_QueueAudio(m_audioDevice, scaled.constData(), len);
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

  ++loops;

  // Emit heartbeat early so it is not delayed by WASM calls below
  if (!(loops % (SIMULATOR_INTERFACE_HEARTBEAT_PERIOD / 10))) {
    emit heartbeat(loops, 0);
  }

  if (!isRunning()) {
    emit stopped();
    return;
  }

  // LCD updates are handled by onLcdNotify() via simuLcdNotify callback

  // Flush trainer input buffer (bulk copy)
  if (m_trainerDirty && m_fnCopyTrainerInput && m_wasmScratchBuf) {
    m_trainerDirty = false;
    QMutexLocker lckr(&m_mutex);
    uint8_t count = MAX_TRAINER_CH;
    if (m_fnGetMaxTrainerChannels) {
      uint32_t r[1] = {0};
      if (wasm_runtime_call_wasm(m_execEnv, m_fnGetMaxTrainerChannels, 0, r))
        count = qMin(count, (uint8_t)r[0]);
    }
    uint32_t bytes = count * sizeof(int16_t);
    if (bytes <= m_wasmScratchSize) {
      void * nativePtr = wasm_runtime_addr_app_to_native(m_moduleInst,
                                                          m_wasmScratchBuf);
      if (nativePtr) {
        memcpy(nativePtr, m_trainerValues, bytes);
        uint32_t argv[2] = {m_wasmScratchBuf, (uint32_t)count};
        wasm_runtime_call_wasm(m_execEnv, m_fnCopyTrainerInput, 2, argv);
      }
    }
  }

  // Check output values every 50ms (5 loops)
  if (!(loops % 5)) {
    QMutexLocker lckr(&m_mutex);
    checkOutputsChanged();

    // 1. Read the global variable from the WASM module
    // "simuHapticValue" must match exactly what you wrote in simulib.cpp
    uint32_t currentHaptic = 0;

    // Poll haptic state from WASM every 50ms.
    // When haptic fires, emit hapticChanged() to trigger
    // visual (window jitter) and audible (beep) feedback in the simulatormainwindow.cpp
    // as a substitute for physical vibration hardware.
    if (m_fnGetHaptic) {
      uint32_t argv[1] = {0};
      if (wasm_runtime_call_wasm(m_execEnv, m_fnGetHaptic, 0, argv)) {
        uint32_t currentHapticValue = argv[0];
        if (currentHapticValue != m_lastHaptic) {
          m_lastHaptic = currentHapticValue;
          if (currentHapticValue > 0) {
            emit hapticChanged((int)currentHapticValue);
          }
        }
      }
    }
  }
}

// Helper: call a WASM function with 0 args, return int32 result
static int32_t wasmCall0(wasm_exec_env_t env, wasm_function_inst_t fn)
{
  uint32_t argv[1] = {0};
  if (wasm_runtime_call_wasm(env, fn, 0, argv))
    return (int32_t)argv[0];
  return 0;
}

// Helper: call a WASM function with 1 uint32 arg, return int32 result
static int32_t wasmCall1(wasm_exec_env_t env, wasm_function_inst_t fn,
                         uint32_t arg0)
{
  uint32_t argv[1] = {arg0};
  if (wasm_runtime_call_wasm(env, fn, 1, argv))
    return (int32_t)argv[0];
  return 0;
}

// Helper: call a WASM function with 2 uint32 args, return int32 result
static int32_t wasmCall2(wasm_exec_env_t env, wasm_function_inst_t fn,
                         uint32_t arg0, uint32_t arg1)
{
  uint32_t argv[2] = {arg0, arg1};
  if (wasm_runtime_call_wasm(env, fn, 2, argv))
    return (int32_t)argv[0];
  return 0;
}

void WasmSimulatorInterface::checkOutputsChanged()
{
  if (!m_execEnv || !m_wasmScratchBuf)
    return;

  const int16_t limit = 512 * 2;
  int32_t tmpVal;
  void * nativePtr = wasm_runtime_addr_app_to_native(m_moduleInst,
                                                      m_wasmScratchBuf);
  if (!nativePtr)
    return;

  // Channel outputs (bulk copy)
  if (m_fnCopyChannelOutputs) {
    uint32_t argv[2] = {m_wasmScratchBuf, CPN_MAX_CHNOUT};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnCopyChannelOutputs, 2, argv)) {
      uint8_t numCh = (uint8_t)argv[0];
      const int16_t * chans = (const int16_t *)nativePtr;
      for (uint8_t i = 0; i < numCh; i++) {
        if (m_lastOutputs.chans[i] != chans[i] || m_resetOutputsData) {
          emit channelOutValueChange(i, chans[i], limit);
          emit outputValueChange(OUTPUT_SRC_CHAN_OUT, i, chans[i]);
          m_lastOutputs.chans[i] = chans[i];
        }
      }
    }
  }

  // Mix outputs (bulk copy, reuse same buffer)
  if (m_fnCopyMixOutputs) {
    uint32_t argv[2] = {m_wasmScratchBuf, CPN_MAX_CHNOUT};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnCopyMixOutputs, 2, argv)) {
      uint8_t numCh = (uint8_t)argv[0];
      const int16_t * mix = (const int16_t *)nativePtr;
      for (uint8_t i = 0; i < numCh; i++) {
        if (m_lastOutputs.ex_chans[i] != mix[i] || m_resetOutputsData) {
          emit channelMixValueChange(i, mix[i], limit * 2);
          emit outputValueChange(OUTPUT_SRC_CHAN_MIX, i, mix[i]);
          m_lastOutputs.ex_chans[i] = mix[i];
        }
      }
    }
  }

  // Logical switches (bulk copy)
  if (m_fnCopyLogicalSwitches) {
    uint32_t argv[2] = {m_wasmScratchBuf, CPN_MAX_LOGICAL_SWITCHES};
    if (wasm_runtime_call_wasm(m_execEnv, m_fnCopyLogicalSwitches, 2, argv)) {
      uint8_t numLsw = (uint8_t)argv[0];
      const uint8_t * lsw = (const uint8_t *)nativePtr;
      for (uint8_t i = 0; i < numLsw; i++) {
        bool val = lsw[i] != 0;
        if (m_lastOutputs.vsw[i] != val || m_resetOutputsData) {
          emit virtualSwValueChange(i, val ? 1 : 0);
          emit outputValueChange(OUTPUT_SRC_VIRTUAL_SW, i, val ? 1 : 0);
          m_lastOutputs.vsw[i] = val;
        }
      }
    }
  }

  // Trims
  if (m_fnGetTrimValue) {
    for (uint8_t i = 0; i < Board::TRIM_AXIS_COUNT; i++) {
      tmpVal = wasmCall1(m_execEnv, m_fnGetTrimValue, i);
      if (m_lastOutputs.trims[i] != tmpVal || m_resetOutputsData) {
        emit trimValueChange(i, tmpVal);
        emit outputValueChange(OUTPUT_SRC_TRIM_VALUE, i, tmpVal);
        m_lastOutputs.trims[i] = tmpVal;
      }
    }
  }

  // Trim range
  if (m_fnGetTrimRange) {
    tmpVal = (int16_t)wasmCall0(m_execEnv, m_fnGetTrimRange);
    if (m_lastOutputs.trimRange != tmpVal || m_resetOutputsData) {
      emit trimRangeChange(Board::TRIM_AXIS_COUNT, -tmpVal, tmpVal);
      emit outputValueChange(OUTPUT_SRC_TRIM_RANGE, Board::TRIM_AXIS_COUNT, tmpVal);
      m_lastOutputs.trimRange = tmpVal;
    }
  }

  // Flight mode
  if (m_fnGetFlightMode) {
    int8_t phase = (int8_t)wasmCall0(m_execEnv, m_fnGetFlightMode);
    if (m_lastOutputs.phase != phase || m_resetOutputsData) {
      emit phaseChanged(phase, QString::number(phase));
      emit outputValueChange(OUTPUT_SRC_PHASE, 0, (qint16)phase);
      m_lastOutputs.phase = phase;
    }
  }

  // GVars
  if (m_fnGetNumGVars && m_fnGetNumFlightModes && m_fnGetGVar) {
    uint8_t numGv = (uint8_t)wasmCall0(m_execEnv, m_fnGetNumGVars);
    uint8_t numFm = (uint8_t)wasmCall0(m_execEnv, m_fnGetNumFlightModes);
    if (numGv > CPN_MAX_GVARS) numGv = CPN_MAX_GVARS;
    if (numFm > CPN_MAX_FLIGHT_MODES) numFm = CPN_MAX_FLIGHT_MODES;
    for (uint8_t gv = 0; gv < numGv; gv++) {
      for (uint8_t fm = 0; fm < numFm; fm++) {
        tmpVal = wasmCall2(m_execEnv, m_fnGetGVar, gv, fm);
        if (m_lastOutputs.gvars[fm][gv] != tmpVal || m_resetOutputsData) {
          m_lastOutputs.gvars[fm][gv] = tmpVal;
          emit gVarValueChange(gv, tmpVal);
          emit outputValueChange(OUTPUT_SRC_GVAR, gv, tmpVal);
        }
      }
    }
  }

  // Function switch LED colors
  if (m_fnGetNumCustomSwitches && m_fnGetCustomSwitchColor && m_fnGetCustomSwitchIndex) {
    uint8_t numFs = (uint8_t)wasmCall0(m_execEnv, m_fnGetNumCustomSwitches);
    if (numFs > MAX_FS_LEDS) numFs = MAX_FS_LEDS;
    for (uint8_t i = 0; i < numFs; i++) {
      uint32_t color = (uint32_t)wasmCall1(m_execEnv, m_fnGetCustomSwitchColor, i);
      if (m_lastFSLedColors[i] != color || m_resetOutputsData) {
        m_lastFSLedColors[i] = color;
        // Map custom switch index to global switch index for the UI widget
        uint8_t swIdx = (uint8_t)wasmCall1(m_execEnv, m_fnGetCustomSwitchIndex, i);
        emit fsColorChange(swIdx, (qint32)color);
      }
    }
  }

  m_resetOutputsData = false;
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
