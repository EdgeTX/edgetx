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
#include "opentx.h"
#include "simulcd.h"

#include <QDebug>
#include <QElapsedTimer>

#if !defined(MAX_LOGICAL_SWITCHES) && defined(NUM_CSW)
  #define MAX_LOGICAL_SWITCHES    NUM_CSW
#endif

#define GET_SWITCH_BOOL(sw__)    getSwitch((sw__), 0);

#define ETXS_DBG    qDebug() << "(" << simuTimerMicros() << "us)"

int16_t g_anas[Analogs::NUM_ANALOGS];
QVector<QIODevice *> OpenTxSimulator::tracebackDevices;

#if defined(HARDWARE_TOUCH)
  tmr10ms_t downTime = 0;
  tmr10ms_t tapTime = 0;
  short tapCount = 0;
  #define TAP_TIME 25
#endif

uint16_t anaIn(uint8_t chan)
{
  return g_anas[chan];
}

uint16_t getAnalogValue(uint8_t index)
{
  return anaIn(index);
}

void firmwareTraceCb(const char * text)
{
  foreach (QIODevice * dev, OpenTxSimulator::tracebackDevices) {
    if (dev)
      dev->write(text);
  }
}

OpenTxSimulator::OpenTxSimulator() :
  SimulatorInterface(),
  m_timer10ms(nullptr),
  m_resetOutputsData(true),
  m_stopRequested(false)
{
  tracebackDevices.clear();
  traceCallback = firmwareTraceCb;
}

OpenTxSimulator::~OpenTxSimulator()
{
  traceCallback = nullptr;
  tracebackDevices.clear();

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

#if defined(PCBTARANIS)
  g_anas[TX_RTC_VOLTAGE] = 800;  // 2,34V
#endif

  simuInit();
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
  memcpy(dest.data(), eeprom, std::min<int>(EEPROM_SIZE, dest.size()));
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
  unsigned i = idx;
  if (i < 4)  // swap axes
    i = modn12x3[4 * getStickMode() + idx];
  uint8_t phase = getTrimFlightMode(getFlightMode(), i);
  setTrimValue(phase, i, value);
}

void OpenTxSimulator::setTrainerInput(unsigned int inputNumber, int16_t value)
{
  static unsigned dim = DIM(ppmInput);
  //setTrainerTimeout(100);
  if (inputNumber < dim)
    ppmInput[inputNumber] = qMin(qMax((int16_t)-512, value), (int16_t)512);
}

void OpenTxSimulator::setInputValue(int type, uint8_t index, int16_t value)
{
  //qDebug() << type << index << value << this->thread();
  switch (type) {
    case INPUT_SRC_ANALOG :
    case INPUT_SRC_STICK :
      setAnalogValue(index, value);
      break;
    case INPUT_SRC_KNOB :
      setAnalogValue(index + NUM_STICKS, value);
      break;
    case INPUT_SRC_SLIDER :
      setAnalogValue(index + NUM_STICKS + NUM_POTS, value);
      break;
    case INPUT_SRC_TXVIN :
      setAnalogValue(Analogs::TX_VOLTAGE, voltageToAdc(value));
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

void OpenTxSimulator::rotaryEncoderEvent(int steps)
{
#if defined(ROTARY_ENCODER_NAVIGATION)
  (g_eeGeneral.rotEncMode >= ROTARY_ENCODER_MODE_INVERT_BOTH ? steps *= -1
                                                             : steps);
  ROTARY_ENCODER_NAVIGATION_VALUE += steps * ROTARY_ENCODER_GRANULARITY;
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
  ppmInputValidityTimer = ms;
}

void OpenTxSimulator::sendTelemetry(const QByteArray data)
{
  //ETXS_DBG << data;
  sportProcessTelemetryPacket((uint8_t *)data.constData());
}

uint8_t OpenTxSimulator::getSensorInstance(uint16_t id, uint8_t defaultValue)
{
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor * sensor = &g_model.telemetrySensors[i];
      if (sensor->id == id) {
        return sensor->frskyInstance.physID + 1;
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
  const uint8_t phase = getFlightMode();  // opentx.cpp
  const uint8_t mode = getStickMode();

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
    tmpVal = (qint32)GET_SWITCH_BOOL(SWSRC_SW1+i);
    if (lastOutputs.vsw[i] != (bool)tmpVal || m_resetOutputsData) {
      emit virtualSwValueChange(i, tmpVal);
      emit outputValueChange(OUTPUT_SRC_VIRTUAL_SW, i, tmpVal);
      lastOutputs.vsw[i] = tmpVal;
    }
  }

  for (i=0; i < Board::TRIM_AXIS_COUNT; i++) {
    if (i < 4)  // swap axes
      idx = modn12x3[4 * mode + i];
    else
      idx = i;

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
#else
      return Board::BOARD_TARANIS_X9D;
#endif
    }
};

extern "C" DLLEXPORT SimulatorFactory * registerSimu()
{
  return new OpenTxSimulatorFactory();
}
