#include <QApplication>
#include <QDateTime>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QRegularExpression>

#if defined(USE_SDL)
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
#include "helpers.h"

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

enum MessageLevel { MsgInfo, MsgWarning, MsgCritical };

void showMessage(const QString& message, MessageLevel level = MsgInfo)
{
  if (level == MsgInfo) {
    QTextStream(stdout) << message << Qt::endl;
  } else {
    QTextStream(stderr) << message << Qt::endl;
  }
}

const QString sharedHelpText()
{
  QString ret;
  QTextStream stream(&ret);
  // list all available profiles
  stream << Qt::endl
         << QCoreApplication::translate("SimulatorMain", "Available profiles:")
         << Qt::endl;
  QMapIterator<int, QString> pi(g.getActiveProfiles());
  while (pi.hasNext()) {
    pi.next();
    stream << "\t" << QCoreApplication::translate("SimulatorMain", "ID: ")
           << pi.key() << "; "
           << QCoreApplication::translate("SimulatorMain", "Name: ")
           << pi.value() << Qt::endl;
  }
  // list all available radios
  stream << Qt::endl
         << QCoreApplication::translate("SimulatorMain", "Available radios:")
         << Qt::endl;
  foreach (QString name, SimulatorLoader::getAvailableSimulators()) {
    stream << "\t" << name << Qt::endl;
  }
  return ret;
}

void showHelp(QCommandLineParser& parser, const QString& addMsg = QString(),
              int exitCode = 0)
{
  QString msg = "\n";
  if (!addMsg.isEmpty()) msg.append(addMsg).append("\n\n");
  msg.append(parser.helpText());
  msg.append(sharedHelpText());
  showMessage(msg, (exitCode ? MsgWarning : MsgInfo));
}

enum CommandLineParseResult {
  CommandLineNone,
  CommandLineFound,
  CommandLineExitOk,
  CommandLineExitErr
};

CommandLineParseResult cliOptions(SimulatorOptions* simOptions, int* profileId)
{
  QCommandLineParser cliOptions;
  bool cliOptsFound = false;
  int pId = *profileId;

  const QCommandLineOption optHelp = cliOptions.addHelpOption();
  const QCommandLineOption optVer = cliOptions.addVersionOption();

  const QCommandLineOption optProfi(
      QStringList() << "profile" << "p",
      QCoreApplication::translate(
          "SimulatorMain", "Radio profile ID or Name to use for simulator."),
      QCoreApplication::translate("SimulatorMain", "profile"));

  const QCommandLineOption optRadio(
      QStringList() << "radio" << "r",
      QCoreApplication::translate(
          "SimulatorMain",
          "Radio type to simulate (usually defined in profile)."),
      QCoreApplication::translate("SimulatorMain", "radio"));

  const QCommandLineOption optSdDir(
      QStringList() << "sd-path" << "s",
      QCoreApplication::translate(
          "SimulatorMain",
          "Directory containing the SD card image to use. The default is "
          "configured in the chosen Radio Profile."),
      QCoreApplication::translate("SimulatorMain", "path"));

  const QCommandLineOption optStart(
      QStringList() << "start-with" << "w",
      QCoreApplication::translate("SimulatorMain",
                                  "Data source type to use. One of:") +
          " (file|folder|sd)",
      QCoreApplication::translate("SimulatorMain", "type"));

  const QCommandLineOption optFlags(
      QStringList() << "flags" << "f",
      QCoreApplication::translate("SimulatorMain",
                                  "Flags passed from Companion"),
      QCoreApplication::translate("SimulatorMain", "flags"));

  cliOptions.addPositionalArgument(
      QCoreApplication::translate("SimulatorMain", "data-source"),
      QCoreApplication::translate(
          "SimulatorMain",
          "Radio data (.bin/.eeprom/.etx) image file to use OR data folder "
          "path (for Horus-style radios).\n"
          "NOTE: any existing EEPROM data incompatible with the selected radio "
          "type may be overwritten!"),
      QCoreApplication::translate("SimulatorMain", "[data-source]"));

  cliOptions.addOption(optProfi);
  cliOptions.addOption(optRadio);
  cliOptions.addOption(optSdDir);
  cliOptions.addOption(optStart);
  cliOptions.addOption(optFlags);

  QStringList args = QCoreApplication::arguments();
#ifdef Q_OS_WIN
  for (int i = 0; i < args.size(); ++i) {
    args[i].replace(QRegularExpression("^/([^\\s]{2,10})$"), "--\\1");
    args[i].replace(QRegularExpression("^/([^\\s]){1}$"), "-\\1");
  }
#endif

  if (!cliOptions.parse(args)) {
    showHelp(cliOptions, cliOptions.errorText(), 1);
    return CommandLineExitErr;
  }

  if (cliOptions.isSet(optHelp)) {
    showHelp(cliOptions);
    return CommandLineExitOk;
  }

  if (cliOptions.isSet(optVer)) {
    showMessage(APP_SIMULATOR % " v" VERSION " " __DATE__, MsgInfo);
    return CommandLineExitOk;
  }

  if (cliOptions.isSet(optProfi)) {
    bool chk;
    pId = cliOptions.value(optProfi).toInt(&chk);
    if (!chk) pId = g.getActiveProfiles().key(cliOptions.value(optProfi), -1);

    if (!g.getActiveProfiles().contains(pId)) {
      showHelp(cliOptions,
               QCoreApplication::translate(
                   "SimulatorMain", "Error: Profile ID %1 was not found.")
                   .arg(pId),
               1);
      return CommandLineExitErr;
    }

    *simOptions = g.profile[pId].simulatorOptions();
    simOptions->sdPath = g.profile[pId].sdPath();
    cliOptsFound = true;
  }

  if (cliOptions.isSet(optRadio)) {
    simOptions->firmwareId = cliOptions.value(optRadio);
    cliOptsFound = true;
  }

  if (cliOptsFound ||
      (simOptions->dataFile.isEmpty() && !simOptions->simulatorId.isEmpty())) {
    QString defName = g.eepromDir() % "/" % simOptions->simulatorId % ".bin";
    simOptions->dataFile = defName;
  }

  if (cliOptions.isSet(optSdDir)) {
    simOptions->sdPath = cliOptions.value(optSdDir);
    cliOptsFound = true;
  }

  if (cliOptions.positionalArguments().size()) {
    QString datasrc = cliOptions.positionalArguments().at(0);
    if (datasrc.contains(QRegularExpression(".*\\.[\\w]{2,6}$"))) {
      simOptions->dataFile = datasrc;
      simOptions->startupDataType = SimulatorOptions::START_WITH_FILE;
    } else {
      simOptions->dataFolder = datasrc;
      simOptions->startupDataType = SimulatorOptions::START_WITH_FOLDER;
    }
    cliOptsFound = true;
  }

  if (cliOptions.isSet(optStart)) {
    QString stTyp = cliOptions.value(optStart);
    if (stTyp == "file") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_FILE;
    } else if (stTyp == "folder") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_FOLDER;
    } else if (stTyp == "sd") {
      simOptions->startupDataType = SimulatorOptions::START_WITH_SDPATH;
    } else {
      showHelp(cliOptions,
               QCoreApplication::translate(
                   "SimulatorMain", "Unrecognized startup data source type: %1")
                   .arg(stTyp),
               1);
      return CommandLineExitErr;
    }

    cliOptsFound = true;
  }

  int flags = 0;

  if (cliOptions.isSet(optFlags)) {
    bool chk;
    flags = cliOptions.value(optFlags).toInt(&chk);
    if (!chk) flags = 0;
  }

  simOptions->flags = flags;

  *profileId = pId;
  if (cliOptsFound)
    return CommandLineFound;
  else
    return CommandLineNone;
}

int main(int argc, char* argv[])
{
  qWarning() << "DEBUG >> MAIN!";

  //// Force offscreen platform so QtGui/QtWidgets works without X11/Wayland
  //// display
  //if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
  //  qputenv("QT_QPA_PLATFORM", "offscreen");
  //}

  QApplication app(argc, argv);
  app.setApplicationName(APP_SIMULATOR);
  app.setApplicationVersion(VERSION);
  app.setOrganizationName(COMPANY);
  app.setOrganizationDomain(COMPANY_DOMAIN);

  qWarning() << "DEBUG >> Q_INIT_RESOURCE!";

  Q_INIT_RESOURCE(companion);

  if (AppDebugMessageHandler::instance())
    AppDebugMessageHandler::instance()->installAppMessageHandler();

  CustomDebug::setFilterRules();
  qWarning() << "DEBUG >> setFilterRules!";

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

#if defined(USE_SDL)
#if defined(_WIN32) || defined(_WIN64)
  putenv("SDL_AUDIODRIVER=directsound");
#endif
  if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0) {
    showMessage(QCoreApplication::translate(
                    "SimulatorMain", "WARNING: couldn't initialize SDL:\n%1")
                    .arg(SDL_GetError()),
                MsgWarning);
  }
#endif

  qWarning() << "DEBUG >> Q_INIT_RESOURCE!";

  Q_INIT_RESOURCE(hwdefs);
  gBoardFactories = new BoardFactories();
  registerStorageFactories();
  registerOpenTxFirmwares();
  SimulatorLoader::registerSimulators();

  if (!SimulatorLoader::getAvailableSimulators().size()) {
    showMessage(
        QCoreApplication::translate("SimulatorMain", "ERROR: No simulator libraries available."),
        MsgCritical);
    return finish(3);
  }

  int profileId = (g.simuLastProfId() > -1 ? g.simuLastProfId() : g.id());
  SimulatorOptions simOptions = g.profile[profileId].simulatorOptions();

  qWarning() << "DEBUG >> CommandLineParseResult!";

  CommandLineParseResult cliResult = cliOptions(&simOptions, &profileId);

  if (cliResult == CommandLineExitOk) return finish(0);
  if (cliResult == CommandLineExitErr) return finish(1);

  if (!g.profile[profileId].fwType().isEmpty())
    simOptions.firmwareId = g.profile[profileId].fwType();

  if (simOptions.dataFolder.isEmpty()) simOptions.dataFolder = g.eepromDir();
  if (!g.profile[profileId].sdPath().isEmpty())
    simOptions.sdPath = g.profile[profileId].sdPath();

  simOptions.simulatorId = SimulatorLoader::findSimulatorByName(
      Firmware::getFirmwareForId(simOptions.firmwareId)->getSimulatorId());

  // Fail immediately in console mode if parameters are missing (no UI dialog)
  if (cliResult == CommandLineNone || profileId == -1 ||
      simOptions.simulatorId.isEmpty() ||
      (simOptions.dataFile.isEmpty() && simOptions.dataFolder.isEmpty())) {
    QString err = QCoreApplication::translate(
        "SimulatorMain",
        "ERROR: Missing required startup parameters in console mode.\nRun with "
        "--help to view options.");
    showMessage(err, MsgCritical);
    return finish(1);
  }

  qDebug() << "Starting with options: profileId=" << profileId << simOptions;

  QString resultMsg;
  if (!g.getActiveProfiles().contains(profileId) ||
      !SimulatorLoader::getAvailableSimulators().contains(
          simOptions.simulatorId)) {
    QTextStream stream(&resultMsg);
    stream << QCoreApplication::translate(
                  "SimulatorMain",
                  "ERROR: Radio profile or simulator firmware not "
                  "found.\nProfile ID: [%1]; Radio ID: [%2]")
                  .arg(profileId)
                  .arg(simOptions.simulatorId);
    stream << sharedHelpText();
    showMessage(resultMsg, MsgCritical);
    return finish(1);
  }

  g.sessionId(profileId);
  g.simuLastProfId(profileId);

  qWarning() << "DEBUG >> Firmware!";

  Firmware* simfw =
      Firmware::getFirmwareForId(simOptions.firmwareId + "-simulator");
  delete simfw;

  Firmware::setCurrentVariant(
      Firmware::getFirmwareForId(simOptions.firmwareId));

  QTemporaryDir tempDir(QDir::tempPath() % "/etx-cpn-XXXXXX");

  qWarning() << "DEBUG >> QTemporaryDir!";

  if (!tempDir.isValid()) {
    qDebug() << "Unable to create application temporary directory";
    gAppTempPath.clear();
    return finish(1);
  } else {
    gAppTempPath = tempDir.path();

    if (!QDir(gAppTempPath).mkdir("IMAGES")) {
      qDebug() << "Unable to create images cache directory:"
               << Helpers::getImagesCacheDir();
      return finish(1);
    }
  }

  qDebug() << "Created images cache directory:" << Helpers::getImagesCacheDir();

  //int result = 0;
  //SimulatorMainWindow* mainWindow = new SimulatorMainWindow(
  //    nullptr, simOptions.simulatorId,
  //    (simOptions.flags ? simOptions.flags : SIMULATOR_FLAGS_STANDALONE));

  //qDebug() << "DEBUG >> IF-1";

  //if ((result = mainWindow->getExitStatus(&resultMsg))) {
  //  qDebug() << "DEBUG >> IF-2";

  //  if (resultMsg.isEmpty())
  //    resultMsg = QCoreApplication::translate(
  //        "SimulatorMain", "Unknown error during Simulator startup.");
  //  showMessage(resultMsg, MsgCritical);
  //} else if (mainWindow->setOptions(simOptions, true)) {
  //  // Run event loop headlessly without calling mainWindow->show()
  //  qDebug() << "DEBUG >> ELSE-IF-3";

  //  result = app.exec();
  //  if (!result) {
  //    qDebug() << "DEBUG >> IF-4";

  //    if ((result = mainWindow->getExitStatus(&resultMsg)) &&
  //        !resultMsg.isEmpty())
  //      qWarning() << "Exit message from SimulatorMainWindow:" << resultMsg;
  //  }
  //} else {
  //  qDebug() << "DEBUG >> ELSE-IF-5";

  //  result = 3;
  //}

  // mainWindow.m_simulator
  qWarning() << "DEBUG >> TRY LOAD SIMULATOR!";
  QString m_simulatorId = SimulatorLoader::findSimulatorByName(getCurrentFirmware()->getSimulatorId());
  SimulatorInterface* sim = SimulatorLoader::loadSimulator(m_simulatorId);

  qWarning() << "DEBUG >> moveToThread!";
  // TODO: move simuThread to GLOBAL variables. Maybe
  QThread* simuThread = new QThread();
  sim->moveToThread(simuThread);
  simuThread->start();
  qWarning() << "DEBUG >> simuThread.start!";

  //QEventLoop loop;
  //QObject::connect(sim, &SimulatorInterface::started, &loop, &QEventLoop::quit);
  //sim->start();
  //qWarning() << "DEBUG >> sim->start!";
  //loop.exec();
  //qWarning() << "DEBUG >> loop.exec!";

  qWarning() << "DEBUG >> sim->setSwitch!";
  sim->setSwitch(6, 1);
  qWarning() << "DEBUG >> sim->setAnalogValue!";
  sim->setAnalogValue(0, -512);
  //sim->setStick(0, -512);
  //sim->run();
  QThread::msleep(50);
  qWarning() << "DEBUG >> QThread::msleep(50)";
  
  //sim->start();
  // PRINT CHANNELS DATA

  qWarning() << "DEBUG >> END!";

  //delete mainWindow;
  return finish(0);
}

int finish(int exitCode)
{
  SimulatorLoader::unregisterSimulators();
  unregisterOpenTxFirmwares();
  unregisterStorageFactories();
  gBoardFactories->unregisterBoardFactories();

#if defined(USE_SDL)
  SDL_Quit();
#endif

  qDebug() << "SIMULATOR EXIT" << exitCode;
  if (simuDbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&simuDbgLog);
    simuDbgLog.close();
  }
  return exitCode;
}