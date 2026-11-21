/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "SimulatorController.h"

#include <QTimer>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "StdoutWatcher.h"
#include "simulation/simulatorinterface.h"
#include "simulator.h"

namespace SimulatorCli
{

SimulatorController::SimulatorController(const QString& radioName,
                                         const QString& sdPath,
                                         QObject* parent) :
    QObject(parent),
    m_radioName(radioName),
    m_sdPath(sdPath),
    m_profile(getRadioProfile(radioName))
{
  for (auto& sw : m_switches) {
    sw.store(static_cast<int8_t>(SwitchPos::CENTER));
  }
}

SimulatorController::~SimulatorController()
{
  stop();
  waitForShutdown();
}

int64_t SimulatorController::nowMs()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}

bool SimulatorController::start(std::function<void()> onReady)
{
  m_sim = SimulatorLoader::loadSimulator(m_radioName);
  if (!m_sim) {
    std::cerr << "ERROR: Failed to load simulator library for: "
              << m_radioName.toStdString() << "\n";
    return false;
  }

  m_simuThread = new QThread();
  setupConnections();

  m_sim->moveToThread(m_simuThread);
  m_simuThread->start();
  StdoutWatcher::instance().start();

  invokeAsync([this, onReady]() {
    bootAndWaitForReady(/*restoreState=*/false, /*settleDelayMs=*/3000,
                        [this, onReady]() {
                          const auto& analogs = m_profile.analogs;
                          setAnalogInternal(analogs.roll, 0);
                          setAnalogInternal(analogs.pitch, 0);
                          setAnalogInternal(analogs.throttle, 0);
                          setAnalogInternal(analogs.yaw, 0);

                          if (onReady) {
                            QTimer::singleShot(1000, m_sim, onReady);
                          }
                        });
  });

  return true;
}

bool SimulatorController::restart(std::function<void()> onFinished)
{
  if (!m_sim || m_isRestarting.exchange(true)) {
    return false;
  }

  invokeAsync([this, onFinished]() {
    m_sim->stop();

    for (auto& ch : m_channels) {
      ch.store(0);
    }
    m_lastIntensity.store(0);
    m_lastChangeMs.store(0);

    bootAndWaitForReady(/*restoreState=*/true, /*settleDelayMs=*/300,
                        [this, onFinished]() {
                          m_isRestarting.store(false);
                          if (onFinished) {
                            onFinished();
                          }
                        });
  });

  return true;
}

void SimulatorController::stop()
{
  if (!m_sim || !m_simuThread) return;

  if (m_simuThread->isRunning()) {
    QMetaObject::invokeMethod(m_sim, &SimulatorInterface::stop,
                              Qt::QueuedConnection);
    m_simuThread->quit();
  }
}

void SimulatorController::waitForShutdown()
{
  if (!m_simuThread) return;

  if (m_simuThread->isRunning()) {
    m_simuThread->wait();
  }

  delete m_simuThread;
  m_simuThread = nullptr;
  m_sim = nullptr;
}

void SimulatorController::setupConnections()
{
  QObject::connect(m_sim, &SimulatorInterface::started, this, []() {
    std::cout << "Simulator started. Initializing radio...\n" << std::flush;
  });

  QObject::connect(m_sim, &SimulatorInterface::channelOutValueChange, this,
                   [this](quint8 index, qint32 value, qint32 limit) {
                     Q_UNUSED(limit);
                     if (index < m_channels.size()) {
                       m_channels[index].store(value);
                     }
                   });

  QObject::connect(m_sim, &SimulatorInterface::lcdChange, m_sim,
                   &SimulatorInterface::lcdFlushed);

  QObject::connect(m_sim, &SimulatorInterface::hapticChanged, this,
                   [this](int intensity) {
                     const int prev = m_lastIntensity.exchange(intensity);
                     if (intensity > 0 && intensity != prev) {
                       m_lastChangeMs.store(nowMs());
                     } else if (intensity == 0) {
                       m_lastChangeMs.store(0);
                     }
                   });

  QObject::connect(m_simuThread, &QThread::finished, m_sim,
                   &QObject::deleteLater);
}

void SimulatorController::bootAndWaitForReady(bool restoreState,
                                              int settleDelayMs,
                                              std::function<void()> onCompleted)
{
  StdoutWatcher::instance().setCapturing(true);

  m_sim->setSdPath(m_sdPath);
  m_sim->init();
  m_sim->start(nullptr, true);

  if (restoreState) {
    for (uint8_t axis = 0; axis < MAX_ANALOGS; ++axis) {
      m_sim->setAnalogValue(axis, m_analogs[axis].load());
    }
    for (uint8_t i = 0; i < m_profile.numSwitches; ++i) {
      m_sim->setSwitch(i, m_switches[i].load());
    }
  }

  auto finishBoot = [this, settleDelayMs, onCompleted]() {
    StdoutWatcher::instance().setCapturing(false);
    QTimer::singleShot(settleDelayMs, m_sim, [onCompleted]() {
      if (onCompleted) {
        onCompleted();
      }
    });
  };

  const bool isColor = m_profile.lcd.isColor();
  waitForLogOutput(
      m_sim, "YAML model reader", 15000, [this, isColor, finishBoot]() {
        if (isColor) {
          waitForLogOutput(m_sim, "WidgetFactory::create()", 10000, finishBoot);
        } else {
          finishBoot();
        }
      });
}

void SimulatorController::setAnalogInternal(uint8_t axis, int16_t rawValue)
{
  if (axis == INVALID_AXIS) return;

  if (axis < MAX_ANALOGS) {
    m_analogs[axis].store(rawValue);
  }

  invokeAsync(
      [sim = m_sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); });
}

bool SimulatorController::setStick(const std::string& stickName, double percent)
{
  const auto& analogs = m_profile.analogs;
  uint8_t axis = INVALID_AXIS;
  int16_t rawValue = 0;

  if (stickName == "THROTTLE") {
    axis = analogs.throttle;
    // 0..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(-1024.0 + (percent * 20.48)));
  } else {
    // -100..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(percent * 10.24));

    if (stickName == "PITCH") {
      axis = analogs.pitch;
    } else if (stickName == "ROLL") {
      axis = analogs.roll;
    } else if (stickName == "YAW") {
      axis = analogs.yaw;
    } else {
      return false;
    }
  }

  setAnalogInternal(axis, rawValue);
  return true;
}

bool SimulatorController::setPot(const std::string& potName, double percent)
{
  const auto& analogs = m_profile.analogs;
  uint8_t axis = INVALID_AXIS;

  if (potName == "S1") {
    axis = analogs.s1;
  } else if (potName == "S2") {
    axis = analogs.s2;
  } else if (potName == "LS") {
    axis = analogs.ls;
  } else if (potName == "RS") {
    axis = analogs.rs;
  }

  if (axis == INVALID_AXIS) {
    return false;
  }

  percent = std::clamp(percent, -100.0, 100.0);
  const int16_t rawValue = static_cast<int16_t>(std::round(percent * 10.24));
  setAnalogInternal(axis, rawValue);
  return true;
}

bool SimulatorController::set6Pos(int pos)
{
  pos = std::clamp(pos, 1, 6);
  const auto& analogs = m_profile.analogs;

  if (analogs.hasPos6()) {
    static constexpr int16_t posValues[6] = {-1024, -614, -205, 205, 614, 1024};
    setAnalogInternal(analogs.pos6, posValues[pos - 1]);
    return true;
  }

  if (analogs.hasFuncSwitches()) {
    const uint8_t swIndex =
        static_cast<uint8_t>(analogs.firstFuncSwitch + (pos - 1));

    invokeAsync([sim = m_sim, swIndex]() {
      sim->setSwitch(swIndex, static_cast<int8_t>(SwitchPos::DOWN));
      QTimer::singleShot(100, sim, [sim, swIndex]() {
        sim->setSwitch(swIndex, static_cast<int8_t>(SwitchPos::UP));
      });
    });
    return true;
  }

  return false;
}

bool SimulatorController::setSwitch(char swLetter, SwitchPos pos)
{
  const uint8_t swIndex = static_cast<uint8_t>(swLetter - 'A');
  if (swIndex >= m_profile.numSwitches) {
    return false;
  }

  const int8_t rawState = static_cast<int8_t>(pos);
  m_switches[swIndex].store(rawState);

  invokeAsync([sim = m_sim, swIndex, rawState]() {
    sim->setSwitch(swIndex, rawState);
  });
  return true;
}

void SimulatorController::pressKey(int key, int durationMs)
{
  invokeAsync([sim = m_sim, key, durationMs]() {
    const uint8_t keyCode = static_cast<uint8_t>(key);
    sim->setKey(keyCode, true);
    QTimer::singleShot(durationMs, sim,
                       [sim, keyCode]() { sim->setKey(keyCode, false); });
  });
}

void SimulatorController::invokeDelayed(int delayMs,
                                        std::function<void()> fn) const
{
  if (!fn) return;

  invokeAsync([sim = m_sim, delayMs, fn = std::move(fn)]() {
    if (delayMs > 0) {
      QTimer::singleShot(delayMs, sim, fn);
    } else {
      fn();
    }
  });
}

std::array<int32_t, 16> SimulatorController::channels() const
{
  std::array<int32_t, 16> result{};
  for (size_t i = 0; i < result.size(); ++i) {
    result[i] = m_channels[i].load();
  }
  return result;
}

std::vector<SwitchPos> SimulatorController::switches() const
{
  std::vector<SwitchPos> result;
  result.reserve(m_profile.numSwitches);
  for (uint8_t i = 0; i < m_profile.numSwitches; ++i) {
    result.push_back(static_cast<SwitchPos>(m_switches[i].load()));
  }
  return result;
}

bool SimulatorController::isHapticActive(int64_t tickDurationMs) const
{
  const int64_t changedAt = m_lastChangeMs.load();
  return changedAt > 0 && (nowMs() - changedAt) <= tickDurationMs;
}

QImage SimulatorController::grabLcdImage() const
{
  if (!m_sim || !m_simuThread || !m_simuThread->isRunning()) {
    return {};
  }

  const LcdSpec& spec = m_profile.lcd;
  QImage image;

  auto capture = [sim = m_sim, spec, &image]() {
    uint8_t* lcd = sim->getLcd();
    if (!lcd || spec.width <= 0 || spec.height <= 0) return;

    if (spec.isColor()) {
      image = QImage(lcd, spec.width, spec.height, spec.width * 2,
                     QImage::Format_RGB16)
                  .copy();
    } else if (spec.depth == 1) {
      image = QImage(spec.width, spec.height, QImage::Format_RGB32);
      for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
          const bool pixelOn =
              (lcd[(y / 8) * spec.width + x] & (1 << (y % 8))) != 0;
          image.setPixel(x, y, pixelOn ? qRgb(0, 0, 0) : qRgb(47, 123, 227));
        }
      }
    }
  };

  if (QThread::currentThread() == m_sim->thread()) {
    capture();
  } else {
    QMetaObject::invokeMethod(m_sim, capture, Qt::BlockingQueuedConnection);
  }

  return image;
}

bool SimulatorController::saveScreenshot(const QString& filePath) const
{
  const QImage image = grabLcdImage();
  return !image.isNull() && image.save(filePath, "PNG");
}

}  // namespace SimulatorCli