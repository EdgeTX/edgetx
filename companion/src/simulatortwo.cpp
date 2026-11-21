#include <AppDebugMessageHandler>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include <memory>
#include <sstream>
#include <thread>

#include "eeprominterface.h"
#include "appdata.h"
#include "boardfactories.h"
#include "constants.h"
#include "customdebug.h"
#include "helpers.h"
#include "simulation/simulatorinterface.h"
#include "simulator.h"
#include "storage.h"
#include "translations.h"
#include "version.h"

#include <algorithm>
#include <cmath>

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

namespace StickAxis
{
constexpr uint8_t ROLL = 0;      // Aileron (Roll)
constexpr uint8_t PITCH = 1;     // Elevator (Pitch)
constexpr uint8_t THROTTLE = 2;  // Throttle
constexpr uint8_t YAW = 3;       // Rudder (Yaw)
}

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

constexpr uint8_t KEY_RTN = 2;
static qint32 currentChannels[16] = {0};
static std::atomic<bool> isHapticActive {false};

namespace SwitchPos
{
    constexpr int8_t UP = -1;
    constexpr int8_t CENTER = 0;
    constexpr int8_t DOWN = 1;
}

static void handleSwitchCommand(std::stringstream& ss, SimulatorInterface* sim)
{
  std::string sw, pos;
  if (!(ss >> sw >> pos)) return;
  
  char swLetter = static_cast<char>(std::toupper(sw.back()));
  uint8_t swIndex = static_cast<uint8_t>(swLetter - 'A');

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

enum MessageLevel { MsgInfo, MsgWarning, MsgCritical };

void showMessage(const QString& message, MessageLevel level = MsgInfo)
{
  if (level == MsgInfo) {
    QTextStream(stdout) << message << Qt::endl;
  } else {
    QTextStream(stderr) << message << Qt::endl;
  }
}
enum class InitResult { Success, ExitClean, Error };

void initApplicationSubsystems(QApplication& app)
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
  registerOpenTxFirmwares();
  SimulatorLoader::registerSimulators();
}

InitResult setupSessionEnvironment(SimulatorOptions& simOptions, int& profileId)
{
  if (!SimulatorLoader::getAvailableSimulators().size()) {
    showMessage(QCoreApplication::translate("SimulatorMain", "ERROR: No simulator libraries available."), MsgCritical);
    return InitResult::Error;
  }

  profileId = (g.simuLastProfId() > -1 ? g.simuLastProfId() : g.id());
  simOptions = g.profile[profileId].simulatorOptions();

  Firmware* simfw = Firmware::getFirmwareForId(simOptions.firmwareId + "-simulator");
  delete simfw;

  Firmware::setCurrentVariant(Firmware::getFirmwareForId(simOptions.firmwareId));
  static std::unique_ptr<QTemporaryDir> tempDir = std::make_unique<QTemporaryDir>(QDir::tempPath() % "/etx-cpn-XXXXXX");

  return InitResult::Success;
}

SimulatorInterface* createAndStartSimulator(const SimulatorOptions& simOptions, QThread* simuThread, QApplication& app)
{
  SimulatorInterface* sim = SimulatorLoader::loadSimulator("tx16s");
  if (!sim) {
    qCritical() << "Cannot load simulator!";
    return nullptr;
  }

  QObject::connect(sim, &SimulatorInterface::started, &app, []() {
    std::cout << "Simulator started. Ready for console input." << std::flush;
  });

  QObject::connect(sim, &SimulatorInterface::channelOutValueChange, &app,
                   [](quint8 index, qint32 value, qint32 limit) {
                     Q_UNUSED(limit);
                     if (index < 16) {
                       currentChannels[index] = value;
                     }
                   });

  QObject::connect(sim, &SimulatorInterface::lcdChange, sim, &SimulatorInterface::lcdFlushed);
  QObject::connect(sim, &SimulatorInterface::hapticChanged, &app, [](int intensity) { isHapticActive.store(intensity > 1); });

  sim->moveToThread(simuThread);
  simuThread->start();

  QMetaObject::invokeMethod(
      sim,
      [sim]() {
        sim->setSdPath("C:/SD");
        sim->init();
        sim->start(nullptr, true);

        QTimer::singleShot(5000, sim, [sim]() {
          sim->setKey(KEY_RTN, true);
          QTimer::singleShot(100, sim, [sim]() {
            sim->setKey(KEY_RTN, false);
            QTimer::singleShot(3000, sim, [sim]() {
                std::cout << "\n===================================================================\n"
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

void startConsoleInputThread(QApplication& app, SimulatorInterface* sim)
{
  std::thread cliThread([&app, sim]() {
    std::string line;
    while (std::getline(std::cin, line)) {
      std::stringstream ss(line);
      std::string rawCmd;
      if (!(ss >> rawCmd)) continue;

      std::string cmd = toUpper(rawCmd);

      if (cmd == "EXIT") {
        QMetaObject::invokeMethod(&app, &QCoreApplication::quit, Qt::QueuedConnection);
        break;
      } else if (cmd == "CHANNELS") {
        QString channelsStr;
        for (int i = 0; i < 16; ++i) {
          channelsStr += QString::number(i+1) + ":" + QString::number(currentChannels[i]) + " ";
          //channelsStr += QString::number(currentChannels[i]) + " ";
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

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  initApplicationSubsystems(app);

  int profileId = -1;
  SimulatorOptions simOptions;
  InitResult initRes = setupSessionEnvironment(simOptions, profileId);

  if (initRes == InitResult::ExitClean) return finish(0);
  if (initRes == InitResult::Error) return finish(1);

  QThread* simuThread = new QThread();
  SimulatorInterface* sim =
      createAndStartSimulator(simOptions, simuThread, app);
  if (!sim) {
    delete simuThread;
    return finish(1);
  }

  startConsoleInputThread(app, sim);

  QObject::connect(&app, &QCoreApplication::aboutToQuit, sim, [sim]() { sim->stop(); });
  QObject::connect(&app, &QCoreApplication::aboutToQuit, simuThread, &QThread::quit);
  QObject::connect(simuThread, &QThread::finished, sim, &QObject::deleteLater);

  const int result = app.exec();
  simuThread->wait();
  delete simuThread;

  return finish(result);
}

int finish(int exitCode)
{
  SimulatorLoader::unregisterSimulators();
  unregisterOpenTxFirmwares();
  unregisterStorageFactories();
  gBoardFactories->unregisterBoardFactories();

  qDebug() << "SIMULATOR EXIT" << exitCode;
  if (simuDbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&simuDbgLog);
    simuDbgLog.close();
  }
  return exitCode;
}