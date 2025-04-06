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

#include <QApplication>
#include <QDateTime>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#if defined(JOYSTICKS) || defined(SIMU_AUDIO)
  #include <SDL.h>
  #undef main
#endif

#include <AppDebugMessageHandler>

#include "appdata.h"
#include "constants.h"
#include "customdebug.h"
#include "eeprominterface.h"
#include "simulator.h"
#include "simulatormainwindow.h"
#include "simulatorstartupdialog.h"
#include "storage.h"
#include "translations.h"
#include "version.h"
#include "boardfactories.h"

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

void showMessage(const QString & message, enum QMessageBox::Icon icon = QMessageBox::NoIcon, bool useConsole = false)
{
  if (useConsole) {
    if (icon < QMessageBox::Warning)
      QTextStream(stdout) << message << Qt::endl;
    else
      QTextStream(stderr) << message << Qt::endl;

    return;
  }
  // use GUI
  QMessageBox msgBox;
  msgBox.setText("<html><body><pre>" + message.toHtmlEscaped() + "</pre></body></html>");
  msgBox.setIcon(icon);
  msgBox.setWindowTitle(QApplication::translate("SimulatorMain", "EdgeTx Simulator"));
  msgBox.exec();
}

const QString sharedHelpText()
{
  QString ret;
  QTextStream stream(&ret);
  // list all available profiles
  stream << Qt::endl << QApplication::translate("SimulatorMain", "Available profiles:") << Qt::endl;
  QMapIterator<int, QString> pi(g.getActiveProfiles());
  while (pi.hasNext()) {
    pi.next();
    stream << "\t" << QApplication::translate("SimulatorMain", "ID: ") << pi.key() << "; " << QApplication::translate("SimulatorMain", "Name: ") << pi.value() << Qt::endl;
  }
  // list all available radios
  stream << Qt::endl << QApplication::translate("SimulatorMain", "Available radios:") << Qt::endl;
  foreach(QString name, SimulatorLoader::getAvailableSimulators()) {
    stream << "\t" << name << Qt::endl;
  }
  return ret;
}

void showHelp(QCommandLineParser & parser, const QString & addMsg = QString(), int exitCode = 0) {
  QString msg = "\n";
  if (!addMsg.isEmpty())
    msg.append(addMsg).append("\n\n");
  msg.append(parser.helpText());
  msg.append(sharedHelpText());
  // display
  showMessage(msg, (exitCode ? QMessageBox::Warning : QMessageBox::Information));
}

enum CommandLineParseResult
{
  CommandLineNone,
  CommandLineFound,
  CommandLineExitOk,
  CommandLineExitErr
};

CommandLineParseResult cliOptions(SimulatorOptions * simOptions, int * profileId)
{
  QCommandLineParser cliOptions;
  bool cliOptsFound = false;
  int pId = *profileId;

  const QCommandLineOption optHelp = cliOptions.addHelpOption();
  const QCommandLineOption optVer = cliOptions.addVersionOption();

  const QCommandLineOption optProfi(QStringList() << "profile" << "p",
                                    QApplication::translate("SimulatorMain", "Radio profile ID or Name to use for simulator."),
                                    QApplication::translate("SimulatorMain", "profile"));

  const QCommandLineOption optRadio(QStringList() << "radio" << "r",
                                    QApplication::translate("SimulatorMain", "Radio type to simulate (usually defined in profile)."),
                                    QApplication::translate("SimulatorMain", "radio"));

  const QCommandLineOption optSdDir(QStringList() << "sd-path" << "s",
                                    QApplication::translate("SimulatorMain", "Directory containing the SD card image to use. The default is configured in the chosen Radio Profile."),
                                    QApplication::translate("SimulatorMain", "path"));

  const QCommandLineOption optStart(QStringList() << "start-with" << "w",
                                    QApplication::translate("SimulatorMain", "Data source type to use. One of:") + " (file|folder|sd)",
                                    QApplication::translate("SimulatorMain", "type"));

  cliOptions.addPositionalArgument(QApplication::translate("SimulatorMain", "data-source"),
                                   QApplication::translate("SimulatorMain", "Radio data (.bin/.eeprom/.etx) image file to use OR data folder path (for Horus-style radios).\n"
                                         "NOTE: any existing EEPROM data incompatible with the selected radio type may be overwritten!"),
                                   QApplication::translate("SimulatorMain", "[data-source]"));

  cliOptions.addOption(optProfi);
  cliOptions.addOption(optRadio);
  cliOptions.addOption(optSdDir);
  cliOptions.addOption(optStart);

  QStringList args = QCoreApplication::arguments();
#ifdef Q_OS_WIN
  // For backwards compat. with QxtCommandOptions, convert Windows-style CLI switches (/opt) since QCommandLineParser doesn't support them
  for (int i=0; i < args.size(); ++i) {
    args[i].replace(QRegExp("^/([^\\s]{2,10})$"), "--\\1");  // long opts
    args[i].replace(QRegExp("^/([^\\s]){1}$"), "-\\1");      // short opts
  }
#endif

  if (!cliOptions.parse(args)) {
    showHelp(cliOptions, cliOptions.errorText());
    return CommandLineExitErr;
  }

  if (cliOptions.isSet(optHelp)) {
    showHelp(cliOptions);
    return CommandLineExitOk;
  }

  if (cliOptions.isSet(optVer)) {
    showMessage(APP_SIMULATOR % " v" VERSION " " __DATE__, QMessageBox::Information);
    return CommandLineExitOk;
  }

  if (cliOptions.isSet(optProfi)) {
    bool chk;
    pId = cliOptions.value(optProfi).toInt(&chk);
    if (!chk)
      pId = g.getActiveProfiles().key(cliOptions.value(optProfi), -1);

    if (!g.getActiveProfiles().contains(pId)) {
      showHelp(cliOptions, QApplication::translate("SimulatorMain", "Error: Profile ID %1 was not found.").arg(pId));
      return CommandLineExitErr;
    }

    *simOptions = g.profile[pId].simulatorOptions();
    cliOptsFound = true;
  }

  if (cliOptions.isSet(optRadio)) {
    simOptions->firmwareId = cliOptions.value(optRadio);
    cliOptsFound = true;
  }

  if (cliOptsFound || (simOptions->dataFile.isEmpty() && !simOptions->simulatorId.isEmpty())) {
    // this constructs a new default radio data file name in the user-configured eeprom directory
    simOptions->dataFile = SimulatorStartupDialog::radioEepromFileName(simOptions->simulatorId, g.eepromDir());
  }

  if (cliOptions.isSet(optSdDir)) {
    simOptions->sdPath = cliOptions.value(optSdDir);
    cliOptsFound = true;
  }

  if (cliOptions.positionalArguments().size()) {
    QString datasrc = cliOptions.positionalArguments().at(0);
    if (datasrc.contains(QRegExp(".*\\.[\\w]{2,6}$"))) {
      simOptions->dataFile = datasrc;
      simOptions->startupDataType = SimulatorOptions::START_WITH_FILE;
    }
    else {
      simOptions->dataFolder = datasrc;
      simOptions->startupDataType = SimulatorOptions::START_WITH_FOLDER;
    }
    cliOptsFound = true;
  }

  if (cliOptions.isSet(optStart)) {
    QString stTyp = cliOptions.value(optStart);
    if (stTyp == "file") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_FILE;
    }
    else  if (stTyp == "folder") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_FOLDER;
    }
    else  if (stTyp == "sd") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_SDPATH;
    }
    else {
      showHelp(cliOptions, QApplication::translate("SimulatorMain", "Unrecognized startup data source type: %1").arg(stTyp));
      return CommandLineExitErr;
    }

    cliOptsFound = true;
  }

  *profileId = pId;
  if (cliOptsFound)
    return CommandLineFound;
  else
    return CommandLineNone;
}

int main(int argc, char *argv[])
{
  /* From doc: This attribute must be set before Q(Gui)Application is constructed. */
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  app.setApplicationName(APP_SIMULATOR);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName(COMPANY);
  app.setOrganizationDomain(COMPANY_DOMAIN);

  Q_INIT_RESOURCE(companion);

  if (AppDebugMessageHandler::instance())
    AppDebugMessageHandler::instance()->installAppMessageHandler();

  CustomDebug::setFilterRules();

  g.init();  // init settings before installing translations

  if (AppDebugMessageHandler::instance() && g.appDebugLog() && !g.appLogsDir().isEmpty() && QDir().mkpath(g.appLogsDir())) {
    QString fn = g.appLogsDir() % "/SimulatorDebug_" % QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss") % ".log";
    simuDbgLog.setFileName(fn);
    if (simuDbgLog.open(QIODevice::WriteOnly | QIODevice::Text)) {
      AppDebugMessageHandler::instance()->addOutputDevice(&simuDbgLog);
    }
  }

  Translations::installTranslators();

#if defined(JOYSTICKS) || defined(SIMU_AUDIO)
  uint32_t sdlFlags = 0;
  #ifdef JOYSTICKS
    sdlFlags |= SDL_INIT_JOYSTICK;
  #endif
  #ifdef SIMU_AUDIO
    sdlFlags |= SDL_INIT_AUDIO;
  #endif
  #if defined(_WIN32) || defined(_WIN64)
  putenv("SDL_AUDIODRIVER=directsound");
  #endif
  if (SDL_Init(sdlFlags) < 0) {
    showMessage(QApplication::translate("SimulatorMain", "WARNING: couldn't initialize SDL:\n%1").arg(SDL_GetError()), QMessageBox::Warning);
  }
#endif

  Q_INIT_RESOURCE(hwdefs);
  gBoardFactories = new BoardFactories();
  registerStorageFactories();
  registerOpenTxFirmwares();
  SimulatorLoader::registerSimulators();

  if (!SimulatorLoader::getAvailableSimulators().size()) {
    showMessage(QApplication::translate("SimulatorMain", "ERROR: No simulator libraries available."), QMessageBox::Critical);
    return finish(3);
  }

  int profileId = (g.simuLastProfId() > -1 ? g.simuLastProfId() : g.id());
  SimulatorOptions simOptions = g.profile[profileId].simulatorOptions();

  // TODO : defaults should be set in Profile::init()
  if (simOptions.firmwareId.isEmpty()) {
    simOptions.firmwareId = g.profile[profileId].fwType();
  }

  // DO NOT use saved simulatorId as could be changed in later releases
  simOptions.simulatorId = SimulatorLoader::findSimulatorByName(Firmware::getFirmwareForId(simOptions.firmwareId)->getSimulatorId());

  if (simOptions.dataFolder.isEmpty())
    simOptions.dataFolder = g.eepromDir();
  if (simOptions.sdPath.isEmpty())
    simOptions.sdPath = g.profile[profileId].sdPath();

  // Handle startup options

  // check for command-line options
  CommandLineParseResult cliResult = cliOptions(&simOptions, &profileId);

  if (cliResult == CommandLineExitOk)
    return finish(0);
  if (cliResult == CommandLineExitErr)
    return finish(1);

  // Present GUI startup options dialog if necessary
  if (cliResult == CommandLineNone || profileId == -1 || simOptions.simulatorId.isEmpty() || (simOptions.dataFile.isEmpty() && simOptions.dataFolder.isEmpty())) {
    SimulatorStartupDialog * dlg = new SimulatorStartupDialog(&simOptions, &profileId);
    int ret = dlg->exec();
    delete dlg;
    if (ret != QDialog::Accepted) {
      return finish(0);
    }
  }
  qDebug() << "Starting with options: profileId=" << profileId << simOptions;

  // Validate startup options

  QString resultMsg;
  if (profileId < 0 || simOptions.simulatorId.isEmpty() || (simOptions.dataFile.isEmpty() && simOptions.dataFolder.isEmpty() && simOptions.sdPath.isEmpty())) {
    resultMsg = QApplication::translate("SimulatorMain", "ERROR: Couldn't start simulator, missing radio/profile/data file/folder.\n  Profile ID: [%1]; Radio ID: [%2];\nData File: [%3]");
    showMessage(resultMsg.arg(profileId).arg(simOptions.simulatorId, simOptions.dataFile), QMessageBox::Critical);
    return finish(1);
  }
  if (!g.getActiveProfiles().contains(profileId) || !SimulatorLoader::getAvailableSimulators().contains(simOptions.simulatorId)) {
    QTextStream stream(&resultMsg);
    stream << QApplication::translate("SimulatorMain", "ERROR: Radio profile or simulator firmware not found.\nProfile ID: [%1]; Radio ID: [%2]").arg(profileId).arg(simOptions.simulatorId);
    stream << sharedHelpText();
    showMessage(resultMsg, QMessageBox::Critical);
    return finish(1);
  }

  // All checks passed, save profile ID and start simulator

  g.sessionId(profileId);
  g.simuLastProfId(profileId);

  // TODO: fix this in Firmware and Boards refactor
  // Append a dummy variant to firmware name to force the Board Type to be registered
  Firmware * simfw = Firmware::getFirmwareForId(simOptions.firmwareId + "-simulator");
  delete simfw;

  // Set global firmware environment
  Firmware::setCurrentVariant(Firmware::getFirmwareForId(simOptions.firmwareId));
  //qDebug() << "current firmware:" << getCurrentFirmware()->getId();

  int result = 0;
  SimulatorMainWindow * mainWindow = new SimulatorMainWindow(NULL, simOptions.simulatorId, SIMULATOR_FLAGS_STANDALONE);
  if ((result = mainWindow->getExitStatus(&resultMsg))) {
    if (resultMsg.isEmpty())
      resultMsg = QApplication::translate("SimulatorMain", "Unknown error during Simulator startup.");
    showMessage(resultMsg, QMessageBox::Critical);
  }
  else if (mainWindow->setOptions(simOptions, true)) {
    mainWindow->show();
    result = app.exec();
    if (!result) {
      if ((result = mainWindow->getExitStatus(&resultMsg)) && !resultMsg.isEmpty())
        qWarning() << "Exit message from SimulatorMainWindow:" << resultMsg;
    }
  }
  else {
    result = 3;
  }

  delete mainWindow;
  return finish(result);
}

int finish(int exitCode)
{
  SimulatorLoader::unregisterSimulators();
  unregisterOpenTxFirmwares();
  unregisterStorageFactories();
  gBoardFactories->unregisterBoardFactories();

#if defined(JOYSTICKS) || defined(SIMU_AUDIO)
  SDL_Quit();
#endif

  qDebug() << "SIMULATOR EXIT" << exitCode;
  if (simuDbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&simuDbgLog);
    simuDbgLog.close();
  }
  return exitCode;
}
