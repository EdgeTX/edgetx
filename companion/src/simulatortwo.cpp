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

using namespace Simulator;

QFile simuDbgLog;
int finish(int exitCode);

// Масив для збереження останніх відомих значень каналів (1-16)
static qint32 currentChannels[16] = {0};

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
  return cliOptsFound ? CommandLineFound : CommandLineNone;
}

int main(int argc, char* argv[])
{
  qWarning() << "DEBUG >> MAIN!";

  QApplication app(argc, argv);
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

  if (!SimulatorLoader::getAvailableSimulators().size()) {
    showMessage(
        QCoreApplication::translate("SimulatorMain",
                                    "ERROR: No simulator libraries available."),
        MsgCritical);
    return finish(3);
  }

  int profileId = (g.simuLastProfId() > -1 ? g.simuLastProfId() : g.id());
  SimulatorOptions simOptions = g.profile[profileId].simulatorOptions();

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

  Firmware* simfw =
      Firmware::getFirmwareForId(simOptions.firmwareId + "-simulator");
  delete simfw;

  Firmware::setCurrentVariant(
      Firmware::getFirmwareForId(simOptions.firmwareId));

  QTemporaryDir tempDir(QDir::tempPath() % "/etx-cpn-XXXXXX");
  if (!tempDir.isValid()) {
    gAppTempPath.clear();
    return finish(1);
  } else {
    gAppTempPath = tempDir.path();
    if (!QDir(gAppTempPath).mkdir("IMAGES")) {
      return finish(1);
    }
  }

  SimulatorInterface* sim = SimulatorLoader::loadSimulator("tx16s");
  if (!sim) {
    qCritical() << "Cannot load simulator!";
    return finish(1);
  }

  QThread* simuThread = new QThread();

  QObject::connect(sim, &SimulatorInterface::started, &app, []() {
    qInfo() << "Simulator started. Ready for console input.";
  });

  // Оновлюємо стан каналів у пам'яті
  QObject::connect(sim, &SimulatorInterface::channelOutValueChange, &app,
                   [](quint8 index, qint32 value, qint32 limit) {
                     Q_UNUSED(limit);
                     if (index < 16) {
                       currentChannels[index] = value;
                     }
                   });

  // Автоматичне скидання дисплея (необхідне для роботи рушія LVGL)
  QObject::connect(sim, &SimulatorInterface::lcdChange, sim,
                   &SimulatorInterface::lcdFlushed);

  sim->moveToThread(simuThread);
  simuThread->start();

  QMetaObject::invokeMethod(
      sim,
      [sim]() {
        sim->setSdPath("C:/SD");
        sim->init();
        sim->start(nullptr, true);

        // Початковий газ у мінімум
        sim->setAnalogValue(1, -1024);
        sim->setAnalogValue(2, -1024);

        // Скидання попереджень старту через 5 сек
        QTimer::singleShot(5000, sim, [sim]() {
          sim->setKey(2, true);
          QTimer::singleShot(100, sim, [sim]() {
            sim->setKey(2, false);
            QTimer::singleShot(3000, sim, [sim]() {
              // Виставляємо стики в центр
              sim->setAnalogValue(0, 0);
              sim->setAnalogValue(1, 0);
              sim->setAnalogValue(2, 0);
              sim->setAnalogValue(3, 0);
              qInfo() << ">>> Ready for commands: ANALOG, SWITCH, CHANNELS, "
                         "EXIT <<<";
            });
          });
        });
      },
      Qt::QueuedConnection);

  // Окремий потік для зчитування команд з консолі
  std::thread cliThread([&app, sim]() {
    std::string line;
    while (std::getline(std::cin, line)) {
      std::stringstream ss(line);
      std::string cmd;
      if (!(ss >> cmd)) continue;

      if (cmd == "EXIT") {
        QMetaObject::invokeMethod(&app, &QCoreApplication::quit,
                                  Qt::QueuedConnection);
        break;
      } else if (cmd == "CHANNELS") {
        QString channelsStr;
        for (int i = 0; i < 16; ++i) {
          channelsStr += QString::number(currentChannels[i]) + " ";
        }

        std::cout << channelsStr.trimmed().toStdString() << "\n" << std::flush;
      } else if (cmd == "ANALOG") {
        int x, y;
        ss >> x >> y;
        QMetaObject::invokeMethod(
            sim,
            [sim, x, y]() {
              sim->setAnalogValue(static_cast<uint8_t>(x),
                                  static_cast<int16_t>(y));
            },
            Qt::QueuedConnection);
      } else if (cmd == "SWITCH") {
        int x, y;
        ss >> x >> y;
        QMetaObject::invokeMethod(
            sim,
            [sim, x, y]() {
              sim->setSwitch(static_cast<uint8_t>(x), static_cast<int8_t>(y));
            },
            Qt::QueuedConnection);
      }
    }
  });
  cliThread.detach();

  // Завершення роботи
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