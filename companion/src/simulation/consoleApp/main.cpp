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

#include <AppDebugMessageHandler>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QString>
#include <iostream>

#include "CliCommandHandler.h"
#include "SimulatorController.h"
#include "appdata.h"
#include "boardfactories.h"
#include "boards.h"
#include "constants.h"
#include "customdebug.h"
#include "simulation/simulatorinterface.h"
#include "simulator.h"
#include "storage.h"
#include "translations.h"
#include "version.h"

using namespace Simulator;
using namespace SimulatorCli;

static QFile simuDbgLog;

static int finish(int exitCode)
{
  SimulatorLoader::unregisterSimulators();
  unregisterStorageFactories();
  if (gBoardFactories) {
    gBoardFactories->unregisterBoardFactories();
  }

  qDebug() << "SIMULATOR EXIT" << exitCode;
  if (simuDbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&simuDbgLog);
    simuDbgLog.close();
  }
  return exitCode;
}

static void initApplicationSubsystems(QApplication& app)
{
  app.setApplicationName(APP_SIMULATOR);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName(COMPANY);
  app.setOrganizationDomain(COMPANY_DOMAIN);

  Q_INIT_RESOURCE(companion);

  if (AppDebugMessageHandler::instance()) {
    AppDebugMessageHandler::instance()->installAppMessageHandler();
  }

  CustomDebug::setFilterRules();
  g.init();

  if (AppDebugMessageHandler::instance() && g.appDebugLog() &&
      !g.appLogsDir().isEmpty() && QDir().mkpath(g.appLogsDir())) {
    const QString fn =
        g.appLogsDir() % "/SimulatorDebug_" %
        QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss") % ".log";
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

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  initApplicationSubsystems(app);

  QString radioName;
  QString sdPath;
  if (!parseCommandLine(app, radioName, sdPath)) {
    return finish(1);
  }

  SimulatorController simController(radioName, sdPath);
  CliCommandHandler cli(simController, app);

  if (!simController.start([&cli]() { cli.printBanner(); })) {
    return finish(1);
  }

  cli.startAsync();

  QObject::connect(&app, &QCoreApplication::aboutToQuit, &simController,
                   &SimulatorController::stop);

  const int result = app.exec();
  simController.waitForShutdown();

  return finish(result);
}