#include <AppDebugMessageHandler>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>

#include "appdata.h"
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

namespace StickAxis
{
    constexpr uint8_t ROLL = 0;      // Aileron
    constexpr uint8_t PITCH = 1;     // Elevator
    constexpr uint8_t THROTTLE = 2;  // Throttle
    constexpr uint8_t YAW = 3;       // Rudder
}  // namespace StickAxis

namespace SwitchPos
{
    constexpr int8_t UP = -1;
    constexpr int8_t CENTER = 0;
    constexpr int8_t DOWN = 1;
}  // namespace SwitchPos

constexpr uint8_t KEY_RTN = 2;  // Return/Exit key for TX16S

static qint32 currentChannels[16] = {0};
static std::atomic<bool> isHapticActive{false};

// ============================================================================
static std::string toUpper(std::string str)
{
  for (char& c : str) {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }
  return str;
}

static void handleStickCommand(const std::string& cmd, std::stringstream& ss,
                               SimulatorInterface* sim)
{
  double percent = 0.0;
  if (!(ss >> percent)) return;

  uint8_t axis = 0;
  int16_t rawValue = 0;

  if (cmd == "THROTTLE") {
    axis = StickAxis::THROTTLE;
    // 0..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(-1024.0 + (percent * 20.48)));
  } else {
    // -100..100% -> -1024..+1024
    rawValue = static_cast<int16_t>(std::round(percent * 10.24));

    if (cmd == "PITCH") {
      axis = StickAxis::PITCH;
    } else if (cmd == "ROLL") {
      axis = StickAxis::ROLL;
    } else if (cmd == "YAW") {
      axis = StickAxis::YAW;
    }
  }

  QMetaObject::invokeMethod(
      sim, [sim, axis, rawValue]() { sim->setAnalogValue(axis, rawValue); },
      Qt::QueuedConnection);
}

static void handleSwitchCommand(std::stringstream& ss, SimulatorInterface* sim)
{
  std::string sw, pos;
  if (!(ss >> sw >> pos)) return;

  char swLetter = static_cast<char>(std::toupper(sw.back()));
  uint8_t swIndex = static_cast<uint8_t>(swLetter - 'A');

  pos = toUpper(pos);
  int8_t state = SwitchPos::CENTER;
  if (pos == "UP") {
    state = SwitchPos::UP;
  } else if (pos == "DOWN") {
    state = SwitchPos::DOWN;
  } else if (pos == "CENTER" || pos == "MID") {
    state = SwitchPos::CENTER;
  }

  QMetaObject::invokeMethod(
      sim, [sim, swIndex, state]() { sim->setSwitch(swIndex, state); },
      Qt::QueuedConnection);
}

// ============================================================================
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
              << "Usage: simulatortwo -r <radio> -s <sd-path>\n"
              << "   or: simulatortwo <radio> <sd-path>\n"
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

// ============================================================================
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

  QObject::connect(sim, &SimulatorInterface::lcdChange, sim, &SimulatorInterface::lcdFlushed);
  QObject::connect(sim, &SimulatorInterface::hapticChanged, &app, [](int intensity) { isHapticActive.store(intensity > 0); });

  sim->moveToThread(simuThread);
  simuThread->start();

  QMetaObject::invokeMethod(
      sim,
      [sim, sdPath]() {
        sim->setSdPath(sdPath);
        sim->init();
        sim->start(nullptr, true);

        sim->setAnalogValue(1, -1024);
        sim->setAnalogValue(2, -1024);

        QTimer::singleShot(5000, sim, [sim]() {
          sim->setKey(KEY_RTN, true);
          QTimer::singleShot(100, sim, [sim]() {
            sim->setKey(KEY_RTN, false);
            QTimer::singleShot(
                3000, sim, [sim]() {
                  sim->setAnalogValue(0, 0);
                  sim->setAnalogValue(1, 0);
                  sim->setAnalogValue(2, 0);
                  sim->setAnalogValue(3, 0);

                  std::cout
                      << "\n===================================================================\n"
                      << ">>> EdgeTX Simulator CLI Ready! Available commands:\n"
                      << "-------------------------------------------------------------------\n"
                      << "  THROTTLE <0..100>          ex: THROTTLE 0, THROTTLE 50\n"
                      << "  ROLL|PITCH|YAW <-100..100> ex: PITCH -20, ROLL 50, YAW 0\n"
                      << "  SW <A..H> <UP|MID|DOWN>    ex: SW A UP, SW B CENTER, SW SG DOWN\n"
                      << "  CHANNELS                   Print current 16 channel outputs\n"
                      << "  HAPTIC                     Check vibration status (ON / OFF)\n"
                      << "  EXIT                       Shutdown simulator and exit\n"
                      << "===================================================================\n\n"
                      << std::flush;
                });
          });
        });
      },
      Qt::QueuedConnection);

  return sim;
}

static void startConsoleInputThread(QApplication& app, SimulatorInterface* sim)
{
  std::thread cliThread([&app, sim]() {
    std::string line;
    while (std::getline(std::cin, line)) {
      std::stringstream ss(line);
      std::string rawCmd;
      if (!(ss >> rawCmd)) continue;

      std::string cmd = toUpper(rawCmd);

      if (cmd == "EXIT") {
        QMetaObject::invokeMethod(&app, &QCoreApplication::quit,
                                  Qt::QueuedConnection);
        break;
      } else if (cmd == "CHANNELS") {
        QString channelsStr;
        for (int i = 0; i < 16; ++i) {
          channelsStr += QString::number(i + 1) + ":" +
                         QString::number(currentChannels[i]) + " ";
        }
        std::cout << channelsStr.trimmed().toStdString() << "\n" << std::flush;
      } else if (cmd == "HAPTIC") {
        std::cout << (isHapticActive.load() ? "ON" : "OFF") << std::endl;
      } else if (cmd == "THROTTLE" || cmd == "YAW" || cmd == "PITCH" ||
                 cmd == "ROLL") {
        handleStickCommand(cmd, ss, sim);
      } else if (cmd == "SW" || cmd == "SWITCH") {
        handleSwitchCommand(ss, sim);
      }
    }
  });
  cliThread.detach();
}

// ============================================================================
// MAIN
// ============================================================================
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

  startConsoleInputThread(app, sim);

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