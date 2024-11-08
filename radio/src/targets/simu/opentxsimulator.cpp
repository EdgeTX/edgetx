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

#include "opentxsimulator.h"
#include "edgetx.h"
#include "simulcd.h"
#include "switches.h"
#include "serial.h"

#include "hal/adc_driver.h"
#include "hal/rotary_encoder.h"

#include <QDebug>
#include <QElapsedTimer>

#if !defined(MAX_LOGICAL_SWITCHES) && defined(NUM_CSW)
  #define MAX_LOGICAL_SWITCHES    NUM_CSW
#endif

#define GET_SWITCH_BOOL(sw__)    getSwitch((sw__), 0);

#define ETXS_DBG    qDebug() << "(" << simuTimerMicros() << "us)"

int16_t g_anas[MAX_ANALOG_INPUTS];
QVector<QIODevice *> OpenTxSimulator::tracebackDevices;

typedef struct {
  uint8_t index;
  QMutex mutex;
  QQueue<uint8_t> receiveBuffer;
  OpenTxSimulator * simulator;
} simulated_serial_port_t;

simulated_serial_port_t simulatedSerialPorts[MAX_AUX_SERIAL];
extern etx_serial_port_t * serialPorts[MAX_AUX_SERIAL];

#if defined(HARDWARE_TOUCH)
  tmr10ms_t downTime = 0;
  tmr10ms_t tapTime = 0;
  short tapCount = 0;
  #define TAP_TIME 25
#endif

uint16_t simu_get_analog(uint8_t idx)
{
  // 6POS simu mechanism use a different scale, so needs specific offset
  if (IS_POT_MULTIPOS(idx - adcGetInputOffset(ADC_INPUT_FLEX))) {
    // Use radio calibration data to determine conversion factor
    StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[idx];
    int range6POS = 2048; // Default if calibration is not valid
    if (calib->count != 0) {
      // calculate 6POS switch range from calibration data
      int c1 = calib->steps[calib->count - 1] * 32; // last calibration value
      int c2 = calib->steps[calib->count - 2] * 32; // 2nd last calibration value
      range6POS = c1 + (c1 - c2) / 2;
    }
    return (g_anas[idx] * range6POS / 2048);
  }
  return (g_anas[idx] * 2) + 2048;
}

void firmwareTraceCb(const char * text)
{
  foreach (QIODevice * dev, OpenTxSimulator::tracebackDevices) {
    if (dev)
      dev->write(text);
  }
}

// Serial port handling needs to know about OpenTxSimulator, so we we
// need to update what's in simpgmspace.cpp when we have a simulator
// to point at.

static void* simulator_host_drv_init(void* hw_def, const etx_serial_init* dev)
{
  if (hw_def == nullptr)
    return nullptr;

  simulated_serial_port_t *port = (simulated_serial_port_t *)hw_def;

  port->simulator->drv_auxSerialInit(port->index, dev);

  // Return the port definition as the context
  return (void *)port;
}

static void simulator_host_drv_deinit(void* ctx)
{
  if (ctx == nullptr)
    return;

  simulated_serial_port_t *port = (simulated_serial_port_t *)ctx;
  port->simulator->drv_auxSerialDeinit(port->index);
}

static void simulator_host_drv_send_byte(void* ctx, uint8_t b)
{
  if (ctx == nullptr)
    return;

  simulated_serial_port_t *port = (simulated_serial_port_t *)ctx;

  port->simulator->drv_auxSerialSendByte(port->index, b);
}

static void simulator_host_drv_send_buffer(void* ctx, const uint8_t* b, uint32_t l)
{
  if (ctx == nullptr)
    return;

  simulated_serial_port_t *port = (simulated_serial_port_t *)ctx;

  port->simulator->drv_auxSerialSendBuffer(port->index, b, l);
}

static int simulator_host_drv_get_byte(void* ctx, uint8_t* b)
{
  if (ctx == nullptr)
    return 0;

  simulated_serial_port_t *port = (simulated_serial_port_t *)ctx;

  return port->simulator->drv_auxSerialGetByte(port->index, b);
}

static void simulator_host_drv_set_baudrate(void* ctx, uint32_t baudrate)
{
  if (ctx == nullptr)
    return;

  simulated_serial_port_t *port = (simulated_serial_port_t *)ctx;
  port->simulator->drv_auxSerialSetBaudrate(port->index, baudrate);
}

static const etx_serial_driver_t simulator_host_drv = {
  .init = simulator_host_drv_init,
  .deinit = simulator_host_drv_deinit,
  .sendByte = simulator_host_drv_send_byte,
  .sendBuffer = simulator_host_drv_send_buffer,
  .txCompleted = nullptr,
  .waitForTxCompleted = nullptr,
  .enableRx = nullptr,
  .getByte = simulator_host_drv_get_byte,
  .getLastByte = nullptr,
  .getBufferedBytes = nullptr,
  .copyRxBuffer = nullptr,
  .clearRxBuffer = nullptr,
  .getBaudrate = nullptr,
  .setBaudrate = simulator_host_drv_set_baudrate,
  .setPolarity = nullptr,
  .setHWOption = nullptr,
  .setReceiveCb = nullptr,
  .setIdleCb = nullptr,
  .setBaudrateCb = nullptr,
};


OpenTxSimulator::OpenTxSimulator() :
  SimulatorInterface(),
  m_timer10ms(nullptr),
  m_resetOutputsData(true),
  m_stopRequested(false)
{
  tracebackDevices.clear();
  traceCallback = firmwareTraceCb;

  // When we create the simulator, we change the UART driver 
  for (int i = 0; i < MAX_AUX_SERIAL; i++) {
    etx_serial_port_t * port = serialPorts[i];
    if (port != nullptr) {
      port->uart = &simulator_host_drv;
      port->hw_def = &(simulatedSerialPorts[i]);
      simulatedSerialPorts[i].index = i;
      simulatedSerialPorts[i].simulator = this;
    }
  }
}

OpenTxSimulator::~OpenTxSimulator()
{
  traceCallback = nullptr;
  tracebackDevices.clear();

  for (int i = 0; i < MAX_AUX_SERIAL; i++) {
    etx_serial_port_t * port = serialPorts[i];
    if (port != nullptr) {
      port->hw_def = nullptr;
    }
  }

  if (m_timer10ms)
    delete m_timer10ms;

  if (isRunning()) {
    stop();
    QElapsedTimer tmout;
    tmout.start();
    while (isRunning() && !tmout.hasExpired(1000))
      ;
  }
  //qDebug() << "Deleting OpenTxSimulator";
}

QString OpenTxSimulator::name()
{
  return QString(SIMULATOR_FLAVOUR);
}

bool OpenTxSimulator::isRunning()
{
  QMutexLocker lckr(&m_mtxSimuMain);
  return simuIsRunning();
}

void OpenTxSimulator::init()
{
  if (isRunning())
    return;

  ETXS_DBG;

  if (!m_timer10ms) {
    // make sure we create & control the timer from current thread
    m_timer10ms = new QTimer();
    m_timer10ms->setInterval(10);
    connect(m_timer10ms, &QTimer::timeout, this, &OpenTxSimulator::run);
    connect(this, SIGNAL(started()), m_timer10ms, SLOT(start()));
    connect(this, SIGNAL(stopped()), m_timer10ms, SLOT(stop()));
  }

  m_resetOutputsData = true;
  setStopRequested(false);

  QMutexLocker lckr(&m_mtxSimuMain);
  memset(g_anas, 0, sizeof(g_anas));

  simuInit();

  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) > 0) {
    auto idx = adcGetInputOffset(ADC_INPUT_RTC_BAT);
    setAnalogValue(idx, 800);
  }
}

void OpenTxSimulator::start(const char * filename, bool tests)
{
  if (isRunning()) return;
  ETXS_DBG << "file:" << filename << "tests:" << tests;

  QMutexLocker lckr(&m_mtxSimuMain);
  QMutexLocker slckr(&m_mtxSettings);
  startEepromThread(filename);
  startAudioThread(volumeGain);
  simuStart(tests, simuSdDirectory.toLatin1().constData(), simuSettingsDirectory.toLatin1().constData());

  emit started();
  QTimer::singleShot(0, this, SLOT(run()));  // old style for Qt < 5.4
}

void OpenTxSimulator::stop()
{
  if (!isRunning())
    return;
  ETXS_DBG;

  setStopRequested(true);

  QMutexLocker lckr(&m_mtxSimuMain);
  simuStop();
  stopAudioThread();
  stopEepromThread();

  emit stopped();
}

void OpenTxSimulator::setSdPath(const QString & sdPath, const QString & settingsPath)
{
  QMutexLocker lckr(&m_mtxSettings);
  simuSdDirectory = sdPath;
  simuSettingsDirectory = settingsPath;
}

void OpenTxSimulator::setVolumeGain(const int value)
{
  QMutexLocker lckr(&m_mtxSettings);
  volumeGain = value;
}

void OpenTxSimulator::setRadioData(const QByteArray & data)
{
#if defined(EEPROM_SIZE)
  QMutexLocker lckr(&m_mtxRadioData);
  eeprom = (uint8_t *)malloc(qMin<int>(EEPROM_SIZE, data.size()));
  memcpy(eeprom, data.data(), qMin<int>(EEPROM_SIZE, data.size()));
#endif
}

void OpenTxSimulator::readRadioData(QByteArray & dest)
{
#if defined(EEPROM_SIZE)
  QMutexLocker lckr(&m_mtxRadioData);
  if (eeprom)
    memcpy(dest.data(), eeprom, qMin<int>(EEPROM_SIZE, dest.size()));
#endif
}

uint8_t * OpenTxSimulator::getLcd()
{
  return (uint8_t *)simuLcdBuf;
}

void OpenTxSimulator::setAnalogValue(uint8_t index, int16_t value)
{
  static int dim = DIM(g_anas);
  if (index < dim)
    g_anas[index] = value;
}

void OpenTxSimulator::setSwitch(uint8_t swtch, int8_t state)
{
  simuSetSwitch(swtch, state);
}

void OpenTxSimulator::setKey(uint8_t key, bool state)
{
  simuSetKey(key, state);
}

void OpenTxSimulator::setTrimSwitch(uint8_t trim, bool state)
{
  simuSetTrim(trim, state);
}

void OpenTxSimulator::setTrim(unsigned int idx, int value)
{
  unsigned i = inputMappingConvertMode(idx);
  uint8_t phase = getTrimFlightMode(getFlightMode(), i);
  setTrimValue(phase, i, value);
}

void OpenTxSimulator::setTrainerInput(unsigned int inputNumber, int16_t value)
{
  static unsigned dim = DIM(trainerInput);
  if (inputNumber < dim)
    trainerInput[inputNumber] = qMin(qMax((int16_t)-512, value), (int16_t)512);
}

void OpenTxSimulator::setInputValue(int type, uint8_t index, int16_t value)
{
  //qDebug() << type << index << value << this->thread();
  switch (type) {
    case INPUT_SRC_ANALOG :
    case INPUT_SRC_STICK :
    case INPUT_SRC_KNOB :
    case INPUT_SRC_SLIDER :
      setAnalogValue(index, value);
      break;
    case INPUT_SRC_TXVIN :
      if (adcGetMaxInputs(ADC_INPUT_VBAT) > 0) {
        auto idx = adcGetInputOffset(ADC_INPUT_VBAT);
        setAnalogValue(idx, voltageToAdc(value));
      }
      break;
    case INPUT_SRC_SWITCH :
      setSwitch(index, (int8_t)value);
      break;
    case INPUT_SRC_TRIM_SW :
      setTrimSwitch(index, (bool)value);
      break;
    case INPUT_SRC_TRIM :
      setTrim(index, value);
      break;
    case INPUT_SRC_KEY :
      setKey(index, (bool)value);
      break;
    case INPUT_SRC_TRAINER :
      setTrainerInput(index, value);
      break;
    case INPUT_SRC_ROTENC :  // TODO
    default:
      return;
  }
}

extern volatile rotenc_t rotencValue;
extern volatile uint32_t rotencDt;

void OpenTxSimulator::rotaryEncoderEvent(int steps)
{
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  static uint32_t last_tick = 0;
  if (steps != 0) {
    if (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_BOTH)
      steps *= -1;
    rotencValue += steps * ROTARY_ENCODER_GRANULARITY;
    // TODO: set rotencDt
    uint32_t now = RTOS_GET_MS();
    uint32_t dt = now - last_tick;
    rotencDt += dt;
    last_tick = now;
  }
#else
  // TODO : this should probably be handled in the GUI
  int key;
#if defined(PCBXLITE)
  if (steps > 0)
    key = KEY_DOWN;
  else if (steps < 0)
    key = KEY_UP;
#elif defined(KEYS_GPIO_REG_PLUS) && defined(KEYS_GPIO_REG_MINUS)
  if (steps > 0)
    key = KEY_MINUS;
  else if (steps < 0)
    key = KEY_PLUS;
  else
#endif
    // Should not happen but Clang complains that key is unset otherwise
    return;

  setKey(key, 1);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
  QTimer::singleShot(10, [this, key]() { setKey(key, 0); });
#else
  QTimer *timer = new QTimer(this);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, [=]() {
    setKey(key, 0);
    timer->deleteLater();
  } );
  timer->start(10);
#endif
#endif  // defined(ROTARY_ENCODER_NAVIGATION)
}

void OpenTxSimulator::touchEvent(int type, int x, int y)
{
  #if defined(HARDWARE_TOUCH)
    tmr10ms_t now = get_tmr10ms();
    simTouchState.tapCount = 0;
  #endif

  switch (type) {
    case TouchDown:
      TRACE_WINDOWS("[Mouse Press] %d %d", x, y);

#if defined(HARDWARE_TOUCH)
      simTouchState.event = TE_DOWN;
      simTouchState.startX = simTouchState.x = x;
      simTouchState.startY = simTouchState.y = y;
      downTime = now;
#endif
      break;

    case TouchUp:
      TRACE_WINDOWS("[Mouse Release] %d %d", x, y);

#if defined(HARDWARE_TOUCH)
      if (simTouchState.event == TE_DOWN) {
        simTouchState.event = TE_UP;
        simTouchState.x = simTouchState.startX;
        simTouchState.y = simTouchState.startY;
        if (now - downTime <= TAP_TIME) {
          if (now - tapTime > TAP_TIME)
            tapCount = 1;
          else
            tapCount++;
          simTouchState.tapCount = tapCount;
          tapTime = now;
        }
      } else {
        simTouchState.event = TE_SLIDE_END;
      }
#endif
      break;

    case TouchSlide:
      TRACE_WINDOWS("[Mouse Move] %d %d", x, y);

#if defined(HARDWARE_TOUCH)
      simTouchState.deltaX += x - simTouchState.x;
      simTouchState.deltaY += y - simTouchState.y;
      if (simTouchState.event == TE_SLIDE ||
          abs(simTouchState.deltaX) >= SLIDE_RANGE ||
          abs(simTouchState.deltaY) >= SLIDE_RANGE) {
        simTouchState.event = TE_SLIDE;
        simTouchState.x = x;
        simTouchState.y = y;
      }
#endif
      break;
  }
#if defined(HARDWARE_TOUCH)
  simTouchOccured=true;
#endif
}

void OpenTxSimulator::lcdFlushed()
{
  ::lcdFlushed();
}

void OpenTxSimulator::setTrainerTimeout(uint16_t ms)
{
  trainerSetTimer(ms);
}

void OpenTxSimulator::sendTelemetry(const uint8_t module, const uint8_t protocol, const QByteArray data)
{
  //ETXS_DBG << data;
  switch (protocol) {
  case SIMU_TELEMETRY_PROTOCOL_FRSKY_SPORT:
    sportProcessTelemetryPacket(module,
                                (uint8_t *)data.constData(),
                                data.count());
    break;
  case SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB:
    frskyDProcessPacket(module,
                        (uint8_t *)data.constData(),
                        data.count());
    break;
  case SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB_OOB:
    // FrSky D telemetry is a stream which can span multiple
    // packets. The telemetry parser _could_ be in the middle of a
    // user packet when we want to inject telemetry, so we can't just
    // call frskyDProcessPacket() with a USRPKT (at least not safely!)
    // Instead we will bypass the telemetry stream parser and inject
    // out of band into processHubPacket(). This way we also don't
    // have to mess with byte stuffing and variable length packets.
    //
    // Note this doesn't take into account which module it's from, but
    // that's how it works in the radio too if you have two frsky D
    // modules running at once, so ¯\_(ツ)_/¯
    {
      uint8_t id = data[0];
      uint16_t value = ((uint8_t)(data[2]) << 8) + (uint8_t)(data[1]);

      processHubPacket(id, value);
    }
    break;
  case SIMU_TELEMETRY_PROTOCOL_CROSSFIRE:
    processCrossfireTelemetryFrame(module,
                                   (uint8_t *)data.constData(),
                                   data.count());
    break;
  default:
    // Do nothing
    break;
  }
}

void OpenTxSimulator::sendInternalModuleTelemetry(const uint8_t protocol, const QByteArray data)
{
  sendTelemetry(INTERNAL_MODULE, protocol, data);
}

void OpenTxSimulator::sendExternalModuleTelemetry(const uint8_t protocol, const QByteArray data)
{
  sendTelemetry(EXTERNAL_MODULE, protocol, data);
}

uint8_t OpenTxSimulator::getSensorInstance(uint16_t id, uint8_t defaultValue)
{
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor * sensor = &g_model.telemetrySensors[i];
      if (sensor->id == id) {
        return sensor->frskyInstance.physID;
      }
    }
  }
  return defaultValue;
}

uint16_t OpenTxSimulator::getSensorRatio(uint16_t id)
{
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor * sensor = &g_model.telemetrySensors[i];
      if (sensor->id == id) {
        return sensor->custom.ratio;
      }
    }
  }
  return 0;
}

const int OpenTxSimulator::getCapability(Capability cap)
{
  int ret = 0;
  switch(cap) {
    case CAP_LUA :
      #ifdef LUA
        ret = 1;
      #endif
      break;

    case CAP_ROTARY_ENC :
      break;

    case CAP_ROTARY_ENC_NAV :
      #ifdef ROTARY_ENCODER_NAVIGATION
        ret = 1;
      #endif
      break;

    case CAP_TELEM_FRSKY_SPORT :
        ret = 1;
      break;

    case CAP_SERIAL_AUX1:
      ret = (auxSerialGetPort(SP_AUX1) != nullptr);
      break;

    case CAP_SERIAL_AUX2:
      ret = (auxSerialGetPort(SP_AUX2) != nullptr);
      break;
  }
  return ret;
}

void OpenTxSimulator::setLuaStateReloadPermanentScripts()
{
#if defined(LUA)
  luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
#endif
}

void OpenTxSimulator::addTracebackDevice(QIODevice * device)
{
  QMutexLocker lckr(&m_mtxTbDevices);
  if (device && !tracebackDevices.contains(device))
    tracebackDevices.append(device);
}

void OpenTxSimulator::removeTracebackDevice(QIODevice * device)
{
  if (device) {
    QMutexLocker lckr(&m_mtxTbDevices);
    // no QVector::removeAll() in Qt < 5.4
    int i = 0;
    foreach (QIODevice * d, tracebackDevices) {
      if (d == device)
        tracebackDevices.remove(i);
      ++i;
    }
  }
}

void OpenTxSimulator::receiveAuxSerialData(quint8 port_nr, const QByteArray & data)
{
  if (port_nr >= MAX_AUX_SERIAL)
    return;

  QMutexLocker locker(&(simulatedSerialPorts[port_nr].mutex));

  for (uint8_t byte : data)
    simulatedSerialPorts[port_nr].receiveBuffer.enqueue(byte);
}

void OpenTxSimulator::drv_auxSerialSetBaudrate(quint8 port_nr, quint32 baudrate)
{
  emit auxSerialSetBaudrate(port_nr, baudrate);
}

void OpenTxSimulator::drv_auxSerialInit(quint8 port_nr, const etx_serial_init* dev)
{
  switch(dev->encoding) {
  case ETX_Encoding_8N1:
    emit auxSerialSetEncoding(port_nr, SERIAL_ENCODING_8N1);
    break;
  case ETX_Encoding_8E2:
    emit auxSerialSetEncoding(port_nr, SERIAL_ENCODING_8E2);
    break;
  default:
    // Do nothing, host hardware can't do SERIAL_ENCODING_PXX1_PWM
    break;
  }

  if (dev->baudrate != 0)
    emit auxSerialSetBaudrate(port_nr, dev->baudrate);

  emit auxSerialStart(port_nr);
}

void OpenTxSimulator::drv_auxSerialDeinit(quint8 port_nr)
{
  emit auxSerialStop(port_nr);
}

void OpenTxSimulator::drv_auxSerialSendByte(quint8 port_nr, uint8_t b)
{
  QByteArray data = QByteArray((const char *)&b, 1);

  emit auxSerialSendData(port_nr, data);
}

void OpenTxSimulator::drv_auxSerialSendBuffer(quint8 port_nr, const uint8_t* b, uint32_t l)
{
  QByteArray data = QByteArray((const char *)b, l);

  emit auxSerialSendData(port_nr, data);
}

int OpenTxSimulator::drv_auxSerialGetByte(quint8 port_nr, uint8_t *b)
{
  // Obtain the port's mutex before messing with the buffer
  QMutexLocker locker(&(simulatedSerialPorts[port_nr].mutex));

  if (simulatedSerialPorts[port_nr].receiveBuffer.isEmpty())
    return 0;

  uint8_t byte = simulatedSerialPorts[port_nr].receiveBuffer.dequeue();

  *b = byte;
  return 1;
}

/*** Protected functions ***/

void OpenTxSimulator::run()
{
  static uint32_t loops = 0;
  static QElapsedTimer ts;

  if (!loops)
    ts.start();

  if (isStopRequested()) {
    return;
  }
  if (!isRunning()) {
    QString err(getError());
    emit runtimeError(err);
    emit stopped();
    return;
  }

  ++loops;

  per10ms();

  checkLcdChanged();

  if (!(loops % 5)) {
    checkOutputsChanged();
  }

  if (!(loops % (SIMULATOR_INTERFACE_HEARTBEAT_PERIOD / 10))) {
    emit heartbeat(loops, simuTimerMicros() / 1000);
  }
}

bool OpenTxSimulator::isStopRequested()
{
  QMutexLocker lckr(&m_mtxStopReq);
  return m_stopRequested;
}

void OpenTxSimulator::setStopRequested(bool stop)
{
  QMutexLocker lckr(&m_mtxStopReq);
  m_stopRequested = stop;
}

bool OpenTxSimulator::checkLcdChanged()
{
  if (simuLcdRefresh) {
    simuLcdRefresh = false;
    emit lcdChange(isBacklightEnabled());
    return true;
  }
  return false;
}

void OpenTxSimulator::checkOutputsChanged()
{
  static TxOutputs lastOutputs;
  static size_t chansDim = DIM(channelOutputs);
  const static int16_t limit = 512 * 2;
  qint32 tmpVal;
  uint8_t i, idx;
  const uint8_t phase = getFlightMode();  // edgetx.cpp

  for (i=0; i < chansDim; i++) {
    if (lastOutputs.chans[i] != channelOutputs[i] || m_resetOutputsData) {
      emit channelOutValueChange(i, channelOutputs[i], (g_model.extendedLimits ? limit * LIMIT_EXT_PERCENT / 100 : limit));
      emit outputValueChange(OUTPUT_SRC_CHAN_OUT, i, channelOutputs[i]);
      lastOutputs.chans[i] = channelOutputs[i];
    }
    if (lastOutputs.ex_chans[i] != ex_chans[i] || m_resetOutputsData) {
      emit channelMixValueChange(i, ex_chans[i], limit * 2);
      emit outputValueChange(OUTPUT_SRC_CHAN_MIX, i, ex_chans[i]);
      lastOutputs.ex_chans[i] = ex_chans[i];
    }
  }

  for (i=0; i < MAX_LOGICAL_SWITCHES; i++) {
    tmpVal = (qint32)GET_SWITCH_BOOL(SWSRC_FIRST_LOGICAL_SWITCH+i);
    if (lastOutputs.vsw[i] != (bool)tmpVal || m_resetOutputsData) {
      emit virtualSwValueChange(i, tmpVal);
      emit outputValueChange(OUTPUT_SRC_VIRTUAL_SW, i, tmpVal);
      lastOutputs.vsw[i] = tmpVal;
    }
  }

  for (i=0; i < Board::TRIM_AXIS_COUNT; i++) {
    idx = inputMappingConvertMode(i);
    tmpVal = getTrimValue(getTrimFlightMode(phase, idx), idx);
    if (lastOutputs.trims[i] != tmpVal || m_resetOutputsData) {
      emit trimValueChange(i, tmpVal);
      emit outputValueChange(OUTPUT_SRC_TRIM_VALUE, i, tmpVal);
      lastOutputs.trims[i] = tmpVal;
    }
  }

  tmpVal = g_model.extendedTrims ? TRIM_EXTENDED_MAX : TRIM_MAX;
  if (lastOutputs.trimRange != tmpVal || m_resetOutputsData) {
    emit trimRangeChange(Board::TRIM_AXIS_COUNT, -tmpVal, tmpVal);
    emit outputValueChange(OUTPUT_SRC_TRIM_RANGE, Board::TRIM_AXIS_COUNT, tmpVal);
    lastOutputs.trimRange = tmpVal;
  }

  if (lastOutputs.phase != phase || m_resetOutputsData) {
    emit phaseChanged(phase, getCurrentPhaseName());
    emit outputValueChange(OUTPUT_SRC_PHASE, 0, qint16(phase));
    lastOutputs.phase = phase;
  }

#if defined(GVAR_VALUE) && defined(GVARS)
  gVarMode_t gvar;
  for (uint8_t gv=0; gv < MAX_GVARS; gv++) {
    gvar.prec = g_model.gvars[gv].prec;
    gvar.unit = g_model.gvars[gv].unit;
    for (uint8_t fm=0; fm < MAX_FLIGHT_MODES; fm++) {
      gvar.mode = fm;
      gvar.value = (int16_t)GVAR_VALUE(gv, getGVarFlightMode(fm, gv));
      tmpVal = gvar;
      if (lastOutputs.gvars[fm][gv] != tmpVal || m_resetOutputsData) {
        lastOutputs.gvars[fm][gv] = tmpVal;
        emit gVarValueChange(gv, tmpVal);
        emit outputValueChange(OUTPUT_SRC_GVAR, gv, tmpVal);
      }
    }
  }
#endif

  m_resetOutputsData = false;
}

uint8_t OpenTxSimulator::getStickMode()
{
  return limit<uint8_t>(0, g_eeGeneral.stickMode, 3);
}

const char * OpenTxSimulator::getPhaseName(unsigned int phase)
{
  static char buff[sizeof(g_model.flightModeData[0].name)+1];
  zchar2str(buff, g_model.flightModeData[phase].name, sizeof(g_model.flightModeData[0].name));
  return buff;
}

const QString OpenTxSimulator::getCurrentPhaseName()
{
  unsigned phase = getFlightMode();
  QString name(getPhaseName(phase));
  if (name.isEmpty())
    name = QString::number(phase);
  return name;
}

const char * OpenTxSimulator::getError()
{
  return main_thread_error;
}

const int OpenTxSimulator::voltageToAdc(const int volts)
{
  int ret = 0;
#if defined(PCBHORUS) || defined(PCBX7)
  ret = (float)volts * 16.2f;
#elif defined(PCBTARANIS)
  ret = (float)volts * 13.3f;
#else
  ret = (float)volts * 14.15f;
#endif
  return ret;
}


/*
 * OpenTxSimulatorFactory
 */

class OpenTxSimulatorFactory: public SimulatorFactory
{
  public:
    OpenTxSimulatorFactory()
    {
    }

    virtual SimulatorInterface * create()
    {
      return new OpenTxSimulator();
    }

    virtual QString name()
    {
      return QString(SIMULATOR_FLAVOUR);
    }

    virtual Board::Type type()
    {
#if defined(PCBX12S)
      return Board::BOARD_HORUS_X12S;
#elif defined(PCBX10)
      return Board::BOARD_X10;
#elif defined(PCBX7ACCESS)
      return Board::BOARD_TARANIS_X7_ACCESS;
#elif defined(PCBX7)
      return Board::BOARD_TARANIS_X7;
#elif defined(PCBX9LITES)
      return Board::BOARD_TARANIS_X9LITES;
#elif defined(PCBX9LITE)
      return Board::BOARD_TARANIS_X9LITE;
#elif defined(PCBNV14)
      return Board::BOARD_FLYSKY_NV14;
#elif defined(PCBPL18)
      return Board::BOARD_FLYSKY_PL18;
#else
      return Board::BOARD_TARANIS_X9D;
#endif
    }
};

extern "C" DLLEXPORT SimulatorFactory * registerSimu()
{
  return new OpenTxSimulatorFactory();
}
