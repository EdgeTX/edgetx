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

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#include <AppDebugMessageHandler>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QImage>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>
#include <QElapsedTimer>
#include <functional>
#include <mutex>

#include "appdata.h"
#include "boards.h"
#include "boardfactories.h"
#include "constants.h"
#include "customdebug.h"
#include "simulation/simulatorinterface.h"
#include "simulator.h"
#include "storage.h"
#include "translations.h"
#include "version.h"

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

struct AnalogMap {
  uint8_t roll = 0;
  uint8_t pitch = 1;
  uint8_t throttle = 2;
  uint8_t yaw = 3;
  uint8_t s1 = 4;
  uint8_t s2 = 255;
  uint8_t ls = 255;
  uint8_t rs = 255;
  uint8_t pos6 = 255;
  uint8_t firstFuncSwitch = 255;
};

static AnalogMap getAnalogMapForRadio(const QString& radioName)
{
  const QString name = radioName.toLower();

  if (name.contains("tx16") || name.contains("t16") || name.contains("t18") ||
      name.contains("x10") || name.contains("x12s")) {
    // 4: S1, 5: 6POS, 6: S2, 7: LS, 8: RS
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/6, /*ls=*/7, /*rs=*/8, /*pos6=*/5, /*firstFuncSwitch=*/255};
  }

  if (name.contains("tx15") || name == "boxer") {
    // 4: S1, 5: S2, 6: 6POS
    return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/255, /*rs=*/255, /*pos6=*/255, /*firstFuncSwitch=*/6};
  }

  // TX12, TX12MK2, Zorro, Pocket: 4: S1, 5: S2
  return {0, 1, 2, 3, /*s1=*/4, /*s2=*/5, /*ls=*/255, /*rs=*/255, /*pos6=*/255, /*firstFuncSwitch=*/255};
}

namespace SwitchPos
{
    constexpr int8_t UP = -1;
    constexpr int8_t CENTER = 0;
    constexpr int8_t DOWN = 1;
}

static qint32 currentChannels[16] = {0};
static std::atomic<bool> isHapticActive{false};

constexpr int NUM_SWITCHES = 8;  // SA .. SH

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
                               SimulatorInterface* sim, const AnalogMap& analogMap)
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

static void handle6PosCommand(std::stringstream& ss, SimulatorInterface* sim,
                              const AnalogMap& analogMap)
{
  int pos = 1;
  if (!(ss >> pos)) return;
  pos = std::clamp(pos, 1, 6);

  if (analogMap.pos6 != 255) {
    // Analog 6POS (TX16S)
    static const int16_t posValues[6] = {-1024, -614, -205, 205, 614, 1024};
    const int16_t rawValue = posValues[pos - 1];
    const uint8_t axis = analogMap.pos6;

    QMetaObject::invokeMethod(
        sim, [sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); },
        Qt::QueuedConnection);
  } else if (analogMap.firstFuncSwitch != 255) {
    // Button 6POS / Function Switches SW1..SW6 (TX15, Boxer)
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

static void initApplicationSubsystems(QApplication& app)
{
  app.setApplicationName(APP_SIMULATOR);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName(COMPANY);
  app.setOrganizationDomain(COMPANY_DOMAIN);

  Q_INIT_RESOURCE(companion);

  if (AppDebugMessageHandler::instance())
    AppDebugMessageHandler::instance()->installAppMessageHandler();

  CustomDebug::setFilterRules();
  g.init();

  if (AppDebugMessageHandler::instance() && g.appDebugLog() &&
      !g.appLogsDir().isEmpty() && QDir().mkpath(g.appLogsDir())) {
    QString fn = g.appLogsDir() % "/SimulatorDebug_" %
                 QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss") %
                 ".log";
    simuDbgLog.setFileName(fn);
    if (simuDbgLog.open(QIODevice::WriteOnly | QIODevice::Text)) {
      AppDebugMessageHandler::instance()->addOutputDevice(&simuDbgLog);
    }
  }

  Translations::installTranslators();

  Q_INIT_RESOURCE(hwdefs);
  gBoardFactories = new BoardFactories();
  registerStorageFactories();
  SimulatorLoader::registerSimulators();
}

static bool parseCommandLine(const QApplication& app, QString& radioName,
                             QString& sdPath)
{
  QCommandLineParser parser;
  parser.setApplicationDescription("EdgeTX Console Simulator Runner");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption optRadio(QStringList() << "r" << "radio",
                              "Radio type (e.g. tx16s, boxer, zorro)", "name");
  QCommandLineOption optSd(QStringList() << "s" << "sd-path",
                           "Path to SD card directory", "path");

  parser.addOption(optRadio);
  parser.addOption(optSd);

  parser.addPositionalArgument(
      "radio", "Radio type (optional if specified via -r)", "[radio]");
  parser.addPositionalArgument(
      "sd-path", "Path to SD card directory (optional if specified via -s)",
      "[sd-path]");

  if (!parser.parse(app.arguments())) {
    std::cerr << parser.errorText().toStdString() << "\n";
    return false;
  }

  if (parser.isSet("help")) {
    std::cout << parser.helpText().toStdString() << "\nAvailable radios:\n";
    for (const QString& name : SimulatorLoader::getAvailableSimulators()) {
      std::cout << "  " << name.toStdString() << "\n";
    }
    return false;
  }

  if (parser.isSet(optRadio)) {
    radioName = parser.value(optRadio);
  } else if (!parser.positionalArguments().isEmpty()) {
    radioName = parser.positionalArguments().at(0);
  }

  if (parser.isSet(optSd)) {
    sdPath = parser.value(optSd);
  } else if (parser.positionalArguments().size() > 1) {
    sdPath = parser.positionalArguments().at(1);
  }

  if (radioName.isEmpty() || sdPath.isEmpty()) {
    std::cerr << "ERROR: Both radio name and sd-path are required!\n"
              << "Usage: simulatorConsole -r <radio> -s <sd-path>\n"
              << "   or: simulatorConsole <radio> <sd-path>\n"
              << "Run with --help to view available radios.\n";
    return false;
  }

  if (!QDir(sdPath).exists()) {
    std::cerr << "ERROR: SD path does not exist: " << sdPath.toStdString()
              << "\n";
    return false;
  }

  return true;
}

struct LcdSpec {
  int width;
  int height;
  int depth;
};

static LcdSpec getLcdSpecForRadio(const QString& radioName)
{
  const QString name = radioName.toLower();
  if (name.contains("tx15")) {
    return {480, 320, 16};
  }
  if (name.contains("nv14") || name.contains("el18") || name.contains("pl18")) {
    return {320, 480, 16};
  }
  if (name.contains("tx16") || name.contains("t16") || name.contains("t18") ||
      name.contains("t15") || name.contains("x10") || name.contains("x12s")) {
    return {480, 272, 16};
  }
  return {128, 64, 1};
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

class StdoutWatcher
{
 public:
  static StdoutWatcher& instance()
  {
    static StdoutWatcher inst;
    return inst;
  }

  void start()
  {
    if (m_started.exchange(true)) return;

    fflush(stdout);
    setvbuf(stdout, nullptr, _IONBF, 0);

    int pipeFds[2];
#ifdef _WIN32
    m_origStdoutFd = _dup(_fileno(stdout));
    if (m_origStdoutFd == -1 || _pipe(pipeFds, 4096, _O_BINARY) != 0) {
      return;
    }
    _dup2(pipeFds[1], _fileno(stdout));
    _close(pipeFds[1]);
#else
    m_origStdoutFd = dup(STDOUT_FILENO);
    if (m_origStdoutFd == -1 || pipe(pipeFds) != 0) {
      return;
    }
    dup2(pipeFds[1], STDOUT_FILENO);
    close(pipeFds[1]);
#endif

    const int readFd = pipeFds[0];
    std::thread reader([this, readFd]() {
      char buf[1024];
      while (true) {
#ifdef _WIN32
        int n = _read(readFd, buf, sizeof(buf));
#else
        ssize_t n = read(readFd, buf, sizeof(buf));
#endif
        if (n <= 0) break;

        if (m_origStdoutFd != -1) {
#ifdef _WIN32
          _write(m_origStdoutFd, buf, static_cast<unsigned int>(n));
#else
          (void)write(m_origStdoutFd, buf, static_cast<size_t>(n));
#endif
        }

        if (m_capturing.load()) {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_buffer.append(buf, static_cast<size_t>(n));
        }
      }
#ifdef _WIN32
      _close(readFd);
#else
      close(readFd);
#endif
    });
    reader.detach();
  }

  void setCapturing(bool enabled)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_capturing.store(enabled);
    if (!enabled) {
      m_buffer.clear();
    }
  }

  bool contains(const std::string& text) const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_buffer.find(text) != std::string::npos;
  }

 private:
  std::atomic<bool> m_started{false};
  std::atomic<bool> m_capturing{true};
  int m_origStdoutFd{-1};
  mutable std::mutex m_mutex;
  std::string m_buffer;
};

static void waitForLogOutput(SimulatorInterface* sim,
                             const std::string& expectedText, int timeoutMs,
                             std::function<void()> onCompleted)
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

static SimulatorInterface* createAndStartSimulator(const QString& radioName,
                                                   const QString& sdPath,
                                                   QThread* simuThread,
                                                   QApplication& app)
{
  SimulatorInterface* sim = SimulatorLoader::loadSimulator(radioName);
  if (!sim) {
    std::cerr << "ERROR: Failed to load simulator library for: "
              << radioName.toStdString() << "\n";
    return nullptr;
  }

  QObject::connect(sim, &SimulatorInterface::started, &app, []() {
    std::cout << "Simulator started. Initializing radio...\n" << std::flush;
  });

  QObject::connect(sim, &SimulatorInterface::channelOutValueChange, &app,
                   [](quint8 index, qint32 value, qint32 limit) {
                     Q_UNUSED(limit);
                     if (index < 16) {
                       currentChannels[index] = value;
                     }
                   });

  QObject::connect(sim, &SimulatorInterface::lcdChange, sim,
                   &SimulatorInterface::lcdFlushed);
  QObject::connect(sim, &SimulatorInterface::hapticChanged, &app,
                   [](int intensity) { isHapticActive.store(intensity > 0); });

  sim->moveToThread(simuThread);
  simuThread->start();
  StdoutWatcher::instance().start();

  const AnalogMap analogMap = getAnalogMapForRadio(radioName);
  const bool isColorLcd = (getLcdSpecForRadio(radioName).depth == 16);

  QMetaObject::invokeMethod(
      sim,
      [sim, sdPath, analogMap, isColorLcd]() {
        StdoutWatcher::instance().setCapturing(true);

        sim->setSdPath(sdPath);
        sim->init();
        sim->start(nullptr, true);
        sim->setAnalogValue(analogMap.throttle, -1024);

        auto onInitFinished = [sim, analogMap]() {
          StdoutWatcher::instance().setCapturing(false);

          QTimer::singleShot(3000, sim, [sim, analogMap]() {
            sim->setAnalogValue(analogMap.roll, 0);
            sim->setAnalogValue(analogMap.pitch, 0);
            sim->setAnalogValue(analogMap.throttle, -1024);
            sim->setAnalogValue(analogMap.yaw, 0);

            if (analogMap.s1 != 255) sim->setAnalogValue(analogMap.s1, -1024);
            if (analogMap.s2 != 255) sim->setAnalogValue(analogMap.s2, -1024);
            if (analogMap.ls != 255) sim->setAnalogValue(analogMap.ls, -1024);
            if (analogMap.rs != 255) sim->setAnalogValue(analogMap.rs, -1024);
            if (analogMap.pos6 != 255)
              sim->setAnalogValue(analogMap.pos6, -1024);

            for (uint8_t i = 0; i < NUM_SWITCHES; ++i) {
              currentSwitches[i].store(SwitchPos::UP);
              sim->setSwitch(i, SwitchPos::UP);
            }

            QTimer::singleShot(1000, sim, []() {
              std::cout
                  << "\n===================================================================\n"
                  << ">>> EdgeTX Simulator CLI Ready! Available commands:\n"
                  << "-------------------------------------------------------------------\n"
                  << "  THROTTLE <0..100>             ex: THROTTLE 0, THROTTLE 50\n"
                  << "  ROLL|PITCH|YAW <-100..100>    ex: PITCH -20, ROLL 50, YAW 0\n"
                  << "  SW <A..H> <UP|MID|DOWN>       ex: SW A UP, SW B CENTER, SW G DOWN\n"
                  << "  POT <S1|S2|LS|RS> <-100..100> ex: POT S1 0, POT S2 -100\n"
                  << "  6POS <1..6>                   ex: 6POS 1, 6POS 4\n"
                  << "  KEY <id>                      ex: KEY 1, KEY 2\n"
                  << "  SCREENSHOT [file]             Save LCD screenshot to PNG file\n"
                  << "  CHANNELS                      Print current 16 channel outputs\n"
                  << "  HAPTIC                        Check vibration status (ON / OFF)\n"
                  << "  EXIT                          Shutdown simulator and exit\n"
                  << "===================================================================\n\n"
                  << std::flush;
            });
          });
        };

        waitForLogOutput(
            sim, "YAML model reader", 15000,
            [sim, isColorLcd, onInitFinished]() {
              if (isColorLcd) {
                waitForLogOutput(sim, "WidgetFactory::create()", 10000,
                                 onInitFinished);
              } else {
                onInitFinished();
              }
            });
      },
      Qt::QueuedConnection);

  return sim;
}

static std::mutex coutMutex;

static void printAck(const std::string& cmd)
{
  const QString ts =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  std::lock_guard<std::mutex> lock(coutMutex);
  std::cout
      << "[ACK] " << ts.toStdString() << " " << cmd << "\n"
      << std::flush;
}

static void scheduleAck(SimulatorInterface* sim, const std::string& cmd, int delayMs = 0)
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

static void startConsoleInputThread(QApplication& app, SimulatorInterface* sim,
                                    const QString& radioName)
{
  const AnalogMap analogMap = getAnalogMapForRadio(radioName);

  std::thread cliThread([&app, sim, radioName, analogMap]() {
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
        std::cout << (isHapticActive.load() ? "ON" : "OFF") << "\n"
                  << std::flush;
      } else if (cmd == "THROTTLE" || cmd == "YAW" || cmd == "PITCH" ||
                 cmd == "ROLL") {
        handleStickCommand(cmd, ss, sim, analogMap);
      } else if (cmd == "POT" || cmd == "KNOB") {
        handlePotCommand(ss, sim, analogMap);
      } else if (cmd == "6POS" || cmd == "POS6") {
        handle6PosCommand(ss, sim, analogMap);
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

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  initApplicationSubsystems(app);

  QString radioName;
  QString sdPath;

  if (!parseCommandLine(app, radioName, sdPath)) {
    return finish(1);
  }

  QThread* simuThread = new QThread();
  SimulatorInterface* sim =
      createAndStartSimulator(radioName, sdPath, simuThread, app);
  if (!sim) {
    delete simuThread;
    return finish(1);
  }

  startConsoleInputThread(app, sim, radioName);

  QObject::connect(&app, &QCoreApplication::aboutToQuit, sim,
                   [sim]() { sim->stop(); });
  QObject::connect(&app, &QCoreApplication::aboutToQuit, simuThread,
                   &QThread::quit);
  QObject::connect(simuThread, &QThread::finished, sim, &QObject::deleteLater);

  const int result = app.exec();
  simuThread->wait();
  delete simuThread;

  return finish(result);
}

int finish(int exitCode)
{
  SimulatorLoader::unregisterSimulators();
  unregisterStorageFactories();
  gBoardFactories->unregisterBoardFactories();

  qDebug() << "SIMULATOR EXIT" << exitCode;
  if (simuDbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&simuDbgLog);
    simuDbgLog.close();
  }
  return exitCode;
}