#include <QDateTime>
#include <QElapsedTimer>
#include <QImage>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

#include "clicommands.h"
#include "radioconfig.h"

namespace SwitchPos
{
constexpr int8_t UP = -1;
constexpr int8_t CENTER = 0;
constexpr int8_t DOWN = 1;
}  // namespace SwitchPos

constexpr int NUM_SWITCHES = 8;  // SA .. SH
constexpr int NUM_ANALOGS = 16;
static std::atomic<int16_t> currentAnalogs[NUM_ANALOGS]{};
static std::atomic<bool> isRestarting{false};

static std::atomic<int8_t> currentSwitches[NUM_SWITCHES]{
    SwitchPos::CENTER, SwitchPos::CENTER, SwitchPos::CENTER, SwitchPos::CENTER,
    SwitchPos::CENTER, SwitchPos::CENTER, SwitchPos::CENTER, SwitchPos::CENTER};

static void printSwitches()
{
  std::string header;
  std::string states;

  for (int i = 0; i < NUM_SWITCHES; ++i) {
    header += static_cast<char>('A' + i);

    int8_t st = currentSwitches[i].load();
    if (st == SwitchPos::UP) {
      states += '^';
    } else if (st == SwitchPos::DOWN) {
      states += 'v';
    } else {
      states += '-';
    }
  }

  std::cout << header << "\n" << states << std::endl;
}

static std::string toUpper(std::string str)
{
  for (char& c : str) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return str;
}

static void handleStickCommand(const std::string& cmd, std::stringstream& ss,
                               SimulatorInterface* sim,
                               const AnalogMap& analogMap)
{
  double percent = 0.0;
  if (!(ss >> percent)) return;

  uint8_t axis = 0;
  int16_t rawValue = 0;

  if (cmd == "THROTTLE") {
    axis = analogMap.throttle;
    // 0..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(-1024.0 + (percent * 20.48)));
  } else {
    // -100..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(percent * 10.24));

    if (cmd == "PITCH") {
      axis = analogMap.pitch;
    } else if (cmd == "ROLL") {
      axis = analogMap.roll;
    } else if (cmd == "YAW") {
      axis = analogMap.yaw;
    }
  }

  QMetaObject::invokeMethod(
      sim, [sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); },
      Qt::QueuedConnection);
}

static void handlePotCommand(std::stringstream& ss, SimulatorInterface* sim,
                             const AnalogMap& analogMap)
{
  std::string potName;
  double percent = 0.0;
  if (!(ss >> potName >> percent)) return;

  potName = toUpper(potName);
  uint8_t axis = 255;

  if (potName == "S1") {
    axis = analogMap.s1;
  } else if (potName == "S2") {
    axis = analogMap.s2;
  } else if (potName == "LS") {
    axis = analogMap.ls;
  } else if (potName == "RS") {
    axis = analogMap.rs;
  }

  if (axis == 255) {
    std::cerr << "Pot " << potName << " is not available on this radio\n";
    return;
  }

  percent = std::clamp(percent, -100.0, 100.0);
  int16_t rawValue = static_cast<int16_t>(std::round(percent * 10.24));

  QMetaObject::invokeMethod(
      sim, [sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); },
      Qt::QueuedConnection);
}

static void PressKey(SimulatorInterface* sim, int key, int durationMs = 100)
{
  if (!sim) return;

  QMetaObject::invokeMethod(
      sim,
      [sim, key, durationMs]() {
        const uint8_t keyCode = static_cast<uint8_t>(key);
        sim->setKey(keyCode, true);
        QTimer::singleShot(durationMs, sim,
                           [sim, keyCode]() { sim->setKey(keyCode, false); });
      },
      Qt::QueuedConnection);
}

static void handle6PosCommand(std::stringstream& ss, SimulatorInterface* sim,
                              const AnalogMap& analogMap)
{
  int pos = 1;
  if (!(ss >> pos)) return;
  pos = std::clamp(pos, 1, 6);

  if (analogMap.pos6 != 255) {
    static const int16_t posValues[6] = {-1024, -614, -205, 205, 614, 1024};
    const int16_t rawValue = posValues[pos - 1];
    const uint8_t axis = analogMap.pos6;

    QMetaObject::invokeMethod(
        sim, [sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); },
        Qt::QueuedConnection);
  } else if (analogMap.firstFuncSwitch != 255) {
    const uint8_t swIndex =
        static_cast<uint8_t>(analogMap.firstFuncSwitch + (pos - 1));

    QMetaObject::invokeMethod(
        sim,
        [sim, swIndex]() {
          sim->setSwitch(swIndex, SwitchPos::DOWN);
          QTimer::singleShot(100, sim, [sim, swIndex]() {
            sim->setSwitch(swIndex, SwitchPos::UP);
          });
        },
        Qt::QueuedConnection);
  } else {
    std::cerr << "6POS is not available on this radio\n";
  }
}

static void handleSwitchCommand(std::stringstream& ss, SimulatorInterface* sim)
{
  std::string sw, pos;
  if (!(ss >> sw >> pos)) {
    printSwitches();
    return;
  }

  char swLetter = static_cast<char>(std::toupper(sw.back()));
  uint8_t swIndex = static_cast<uint8_t>(swLetter - 'A');
  if (swIndex >= NUM_SWITCHES) return;

  pos = toUpper(pos);
  int8_t state = SwitchPos::CENTER;
  if (pos == "UP") {
    state = SwitchPos::UP;
  } else if (pos == "DOWN") {
    state = SwitchPos::DOWN;
  } else if (pos == "CENTER" || pos == "MID") {
    state = SwitchPos::CENTER;
  }

  currentSwitches[swIndex].store(state);

  QMetaObject::invokeMethod(
      sim, [sim, swIndex, state]() { sim->setSwitch(swIndex, state); },
      Qt::QueuedConnection);
}

static bool saveScreenshot(SimulatorInterface* sim, const QString& radioName,
                           const QString& filePath)
{
  const LcdSpec spec = getLcdSpecForRadio(radioName);
  QImage image;
  bool ok = false;

  auto capture = [sim, spec, &image, &ok]() {
    uint8_t* lcd = sim->getLcd();
    if (!lcd || spec.width <= 0 || spec.height <= 0) return;

    if (spec.depth == 16) {
      image = QImage(lcd, spec.width, spec.height, spec.width * 2,
                     QImage::Format_RGB16)
                  .copy();
      ok = true;
    } else if (spec.depth == 1) {
      image = QImage(spec.width, spec.height, QImage::Format_RGB32);
      for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
          const bool pixelOn =
              (lcd[(y / 8) * spec.width + x] & (1 << (y % 8))) != 0;
          image.setPixel(x, y, pixelOn ? qRgb(0, 0, 0) : qRgb(47, 123, 227));
        }
      }
      ok = true;
    }
  };

  if (QThread::currentThread() == sim->thread()) {
    capture();
  } else {
    QMetaObject::invokeMethod(sim, capture, Qt::BlockingQueuedConnection);
  }

  return ok && image.save(filePath, "PNG");
}

static void handleSchreenshotCommand(std::stringstream& ss,
                                     SimulatorInterface* sim,
                                     const QString& radioName)
{
  std::string filePath;
  if (!(ss >> filePath)) {
    filePath = "screenshot.png";
  }
  QString qFilePath = QString::fromStdString(filePath);
  if (saveScreenshot(sim, radioName, qFilePath)) {
    std::cout << "Screenshot saved to: " << qFilePath.toStdString() << "\n"
              << std::flush;
  } else {
    std::cout << "Failed to save screenshot to: " << qFilePath.toStdString()
              << "\n"
              << std::flush;
  }
}

void waitForLogOutput(SimulatorInterface* sim, const std::string& expectedText,
                      int timeoutMs, std::function<void()> onCompleted)
{
  if (StdoutWatcher::instance().contains(expectedText)) {
    onCompleted();
    return;
  }

  auto* timer = new QTimer(sim);
  QElapsedTimer elapsed;
  elapsed.start();

  QObject::connect(timer, &QTimer::timeout, sim,
                   [timer, expectedText, timeoutMs, elapsed, onCompleted]() {
                     if (StdoutWatcher::instance().contains(expectedText)) {
                       timer->stop();
                       timer->deleteLater();
                       onCompleted();
                     } else if (elapsed.hasExpired(timeoutMs)) {
                       std::cerr
                           << "[WARN] Timeout waiting for: " << expectedText
                           << "\n"
                           << std::flush;
                       timer->stop();
                       timer->deleteLater();
                       onCompleted();
                     }
                   });

  timer->start(50);
}

static std::mutex coutMutex;

static void printAck(const std::string& cmd)
{
  const QString ts =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  std::lock_guard<std::mutex> lock(coutMutex);
  std::cout << "[ACK] " << ts.toStdString() << " " << cmd << "\n" << std::flush;
}

static void scheduleAck(SimulatorInterface* sim, const std::string& cmd,
                        int delayMs = 0)
{
  QMetaObject::invokeMethod(
      sim,
      [cmd, delayMs]() {
        if (delayMs > 0) {
          QTimer::singleShot(delayMs, [cmd]() { printAck(cmd); });
        } else {
          printAck(cmd);
        }
      },
      Qt::QueuedConnection);
}

static void handleRestartCommand(SimulatorInterface* sim, const QString& sdPath,
                                 const QString& radioName,
                                 const std::string& rawLine)
{
  if (isRestarting.exchange(true)) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cerr << "[WARN] Restart already in progress\n" << std::flush;
    return;
  }

  const bool isColorLcd = (getLcdSpecForRadio(radioName).depth == 16);

  QMetaObject::invokeMethod(
      sim,
      [sim, sdPath, isColorLcd, rawLine]() {
        sim->stop();

        for (int i = 0; i < 16; ++i) {
          currentChannels[i] = 0;
        }
        lastIntensity.store(0);
        lastChangeMs.store(0);

        StdoutWatcher::instance().setCapturing(true);

        sim->setSdPath(sdPath);
        sim->init();
        sim->start(nullptr, true);

        for (uint8_t axis = 0; axis < NUM_ANALOGS; ++axis) {
          sim->setAnalogValue(axis, currentAnalogs[axis].load());
        }
        for (uint8_t i = 0; i < NUM_SWITCHES; ++i) {
          sim->setSwitch(i, currentSwitches[i].load());
        }

        auto onRestartFinished = [sim, rawLine]() {
          StdoutWatcher::instance().setCapturing(false);
          QTimer::singleShot(300, sim, [rawLine]() {
            isRestarting.store(false);
            printAck(rawLine);
          });
        };

        waitForLogOutput(sim, "YAML model reader", 15000,
                         [sim, isColorLcd, onRestartFinished]() {
                           if (isColorLcd) {
                             waitForLogOutput(sim, "WidgetFactory::create()",
                                              10000, onRestartFinished);
                           } else {
                             onRestartFinished();
                           }
                         });
      },
      Qt::QueuedConnection);
}

void startConsoleInputThread(QApplication& app, SimulatorInterface* sim,
                             const QString& radioName, const QString& sdPath)
{
  const AnalogMap analogMap = getAnalogMapForRadio(radioName);

  std::thread cliThread([&app, sim, radioName, sdPath, analogMap]() {
    std::string line;
    while (std::getline(std::cin, line)) {
      std::stringstream ss(line);
      std::string rawCmd;
      if (!(ss >> rawCmd)) continue;

      std::string cmd = toUpper(rawCmd);
      int ackDelayMs = 100;
      bool validCommand = true;

      if (cmd == "EXIT") {
        printAck(cmd);
        QMetaObject::invokeMethod(&app, &QCoreApplication::quit,
                                  Qt::QueuedConnection);
        break;
      } else if (cmd == "RESTART") {
        handleRestartCommand(sim, sdPath, radioName, line);
        continue;
      } else if (cmd == "CHANNELS") {
        QString channelsStr = "CHANNELS ";
        for (int i = 0; i < 16; ++i) {
          channelsStr += QString::number(i + 1) + ":" +
                         QString::number(currentChannels[i]) + " ";
        }
        {
          std::lock_guard<std::mutex> lock(coutMutex);
          std::cout << channelsStr.trimmed().toStdString() << "\n"
                    << std::flush;
        }
      } else if (cmd == "HAPTIC") {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "HAPTIC " << (isHapticActive() ? "ON" : "OFF") << "\n"
                  << std::flush;
      } else if (cmd == "THROTTLE" || cmd == "YAW" || cmd == "PITCH" ||
                 cmd == "ROLL") {
        handleStickCommand(cmd, ss, sim, analogMap);
      } else if (cmd == "POT" || cmd == "KNOB") {
        handlePotCommand(ss, sim, analogMap);
      } else if (cmd == "6POS" || cmd == "POS6") {
        handle6PosCommand(ss, sim, analogMap);
        ackDelayMs = 200;
      } else if (cmd == "SW" || cmd == "SWITCH") {
        handleSwitchCommand(ss, sim);
      } else if (cmd == "SCREENSHOT" || cmd == "SHOT") {
        handleSchreenshotCommand(ss, sim, radioName);
      } else if (cmd == "KEY") {
        int keyId = 0;
        if (ss >> keyId) {
          PressKey(sim, keyId);
        } else {
          validCommand = false;
        }
      } else {
        validCommand = false;
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "[ERR] Unknown command: " << cmd << "\n" << std::flush;
      }

      if (validCommand) {
        scheduleAck(sim, line, ackDelayMs);
      }
    }
  });
  cliThread.detach();
}