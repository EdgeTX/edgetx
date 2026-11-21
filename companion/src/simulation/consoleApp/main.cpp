/*
- Copyright (C) EdgeTX
 *
- Based on code named
-   opentx - https://github.com/opentx/opentx
-   th9x - http://code.google.com/p/th9x
-   er9x - http://code.google.com/p/er9x
-   gruvin9x - http://code.google.com/p/gruvin9x
 *
- License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
- This program is free software; you can redistribute it and/or modify
- it under the terms of the GNU General Public License version 2 as
- published by the Free Software Foundation.
 *
- This program is distributed in the hope that it will be useful,
- but WITHOUT ANY WARRANTY; without even the implied warranty of
- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
- GNU General Public License for more details.
 */

#include <AppDebugMessageHandler>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <chrono>
#include <iostream>

#include "appdata.h"
#include "boardfactories.h"
#include "boards.h"
#include "clicommands.h"
#include "constants.h"
#include "customdebug.h"
#include "radioconfig.h"
#include "simulation/simulatorinterface.h"
#include "simulator.h"
#include "storage.h"
#include "translations.h"
#include "version.h"

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

qint32 currentChannels[16] = {0};
std::atomic<int> lastIntensity{0};
std::atomic<int64_t> lastChangeMs{0};

static int64_t nowMs()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
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

bool isHapticActive(int64_t tickDurationMs)
{
  int64_t changedAt = lastChangeMs.load();
  return changedAt > 0 && (nowMs() - changedAt) <= tickDurationMs;
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
                   [](int intensity) {
                     int prev = lastIntensity.exchange(intensity);
                     if (intensity > 0 && intensity != prev) {
                       lastChangeMs.store(nowMs());
                     } else if (intensity == 0) {
                       lastChangeMs.store(0);
                     }
                   });

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

        auto onInitFinished = [sim, analogMap]() {
          StdoutWatcher::instance().setCapturing(false);

          QTimer::singleShot(3000, sim, [sim, analogMap]() {
            sim->setAnalogValue(analogMap.roll, 0);
            sim->setAnalogValue(analogMap.pitch, 0);
            sim->setAnalogValue(analogMap.throttle, 0);
            sim->setAnalogValue(analogMap.yaw, 0);

            QTimer::singleShot(1000, sim, []() {
              std::cout
                  << "\n======================================================="
                     "============\n"
                  << ">>> EdgeTX Simulator CLI Ready! Available commands:\n"
                  << "---------------------------------------------------------"
                     "----------\n"
                  << "  THROTTLE <0..100>              ex: THROTTLE 0, "
                     "THROTTLE 50\n"
                  << "  ROLL|PITCH|YAW <-100..100>    ex: PITCH -20, ROLL 50, "
                     "YAW 0\n"
                  << "  SW <A..H> <UP|MID|DOWN>        ex: SW A UP, SW B "
                     "CENTER, SW G DOWN\n"
                  << "  POT <S1|S2|LS|RS> <-100..100> ex: POT S1 0, POT S2 "
                     "-100\n"
                  << "  6POS <1..6>                    ex: 6POS 1, 6POS 4\n"
                  << "  KEY <name>                     ex: KEY ENTER, KEY RTN\n"
                  << "    Keys: RTN, PAGE>, <PAGE, SYS, TELE, MDL, ENTER\n"
                  << "  SCROLL <steps>                 ex: SCROLL 1, SCROLL "
                     "-1\n"
                  << "  SCREENSHOT [file]              Save LCD screenshot to "
                     "PNG file\n"
                  << "  CHANNELS                       Print current 16 "
                     "channel outputs\n"
                  << "  HAPTIC                         Check vibration status "
                     "(ON / OFF)\n"
                  << "  RESTART                        Restart simulator "
                     "firmware\n"
                  << "  EXIT                           Shutdown simulator and "
                     "exit\n"
                  << "========================================================="
                     "==========\n\n"
                  << std::flush;
            });
          });
        };

        waitForLogOutput(sim, "YAML model reader", 15000,
                         [sim, isColorLcd, onInitFinished]() {
                           if (isColorLcd) {
                             waitForLogOutput(sim, "WidgetFactory::create()",
                                              10000, onInitFinished);
                           } else {
                             onInitFinished();
                           }
                         });
      },
      Qt::QueuedConnection);

  return sim;
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

  startConsoleInputThread(app, sim, radioName, sdPath);

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