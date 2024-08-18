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
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSplashScreen>
#if defined(JOYSTICKS) || defined(SIMU_AUDIO)
  #include <SDL.h>
  #undef main
#endif

#include <AppDebugMessageHandler>

#include "customdebug.h"
#include "mainwindow.h"
#include "version.h"
#include "appdata.h"
#include "simulatorinterface.h"
#include "storage.h"
#include "translations.h"
#include "helpers.h"
#include "boardfactories.h"

#ifdef __APPLE__
#include <QProxyStyle>

class MyProxyStyle : public QProxyStyle
 {
   public:
    void polish ( QWidget * w ) {
      QMenu* mn = dynamic_cast<QMenu*>(w);
      QPushButton* pb = dynamic_cast<QPushButton*>(w);
      if(!(mn || pb) && !w->testAttribute(Qt::WA_MacNormalSize))
          w->setAttribute(Qt::WA_MacSmallSize);
    }
 };
#endif

void importError()
{
  QMessageBox::critical(nullptr, CPN_STR_APP_NAME, QCoreApplication::translate("Companion", "The saved settings could not be imported, please try again or continue with current settings."), QMessageBox::Ok, 0);
}

void checkSettingsImport(bool force = false)
{
  QString previousVersion;
  bool found;
  if (!(found = g.findPreviousVersionSettings(&previousVersion))) {
    found = QDir(CPN_SETTINGS_BACKUP_DIR).entryList(QStringList({"*.ini"}), QDir::Files).size() > 0;
  }
  if (!found && !force)
    return;

  QString msg;
  if (previousVersion.isEmpty()) {
    const QString impFileBtn = QCoreApplication::translate("Companion", "Import from File");
    const QString impNoneBtn = QCoreApplication::translate("Companion", "Do not import");

    if (found)
      msg = QCoreApplication::translate("Companion", "We have found possible Companion settings backup file(s).\nDo you want to import settings from a file?");
    else
      msg = QCoreApplication::translate("Companion", "Import settings from a file, or start with current values.");

    const int ret = QMessageBox::question(nullptr, CPN_STR_APP_NAME, msg, impNoneBtn, impFileBtn, 0, 0);

    if (!ret)
      return;
  }
  else {
    if (!g.importSettings(previousVersion)) {
      // very unlikely, but just in case of unexpected error, restart the import
      importError();
      checkSettingsImport();
    }
    return;
  }

  // Import from file
  bool err = false;
  QString impFile = CPN_SETTINGS_BACKUP_DIR;
  impFile = QFileDialog::getOpenFileName(nullptr, QCoreApplication::translate("Companion", "Select %1:").arg(CPN_STR_APP_SETTINGS_FILES), impFile, CPN_STR_APP_SETTINGS_FILTER);

  if (!impFile.isEmpty() && QFileInfo(impFile).isReadable()) {
    QSettings fromSettings(impFile, QSettings::IniFormat);
    if (g.importSettings(&fromSettings)) {
      return;
    }
    else {
      importError();
      err = true;
    }
  }
  // Restart the import prompt if user cancelled the file select dialog but we do have previous settings, or if we had a file import error.
  if (err || !previousVersion.isEmpty())
    checkSettingsImport();
}

void printHelpText()
{
  printf(qPrintable(QString(APP_COMPANION % " v%s\n\n")), VERSION);
  const char tmpl[] = "  %s \t %s\n";
  printf(tmpl, "--export",   QCoreApplication::translate("Companion", "Save application settings to file...").toUtf8().constData());
  printf(tmpl, "--import",   QCoreApplication::translate("Companion", "Load application settings from file or previous version...").toUtf8().constData());
  printf(tmpl, "--defaults", QCoreApplication::translate("Companion", "Reset ALL application settings to default and remove radio profiles...").toUtf8().constData());
  printf(tmpl, "--quit  ",   QCoreApplication::translate("Companion", "Exit before settings initialization and application startup.").toUtf8().constData());
  printf(tmpl, "--version",  QCoreApplication::translate("Companion", "Print version number and exit.").toUtf8().constData());
  printf(tmpl, "--help|-h",  QCoreApplication::translate("Companion", "Print this help text.").toUtf8().constData());
  fflush(stdout);
}

int main(int argc, char *argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  /* From doc: This attribute must be set before Q(Gui)Application is constructed. */
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);
  app.setApplicationName(APP_COMPANION);
  app.setOrganizationName(COMPANY);
  app.setOrganizationDomain(COMPANY_DOMAIN);
  app.setAttribute(Qt::AA_DontShowIconsInMenus, false);

  const QStringList args(QApplication::arguments());

  if (args.contains("--help") || args.contains("-h") || args.contains("/?")) {
    printHelpText();
    exit(0);
  }

  if (args.contains("--version")) {
    printf("%s\n", VERSION);
    fflush(stdout);
    exit(0);
  }

  Q_INIT_RESOURCE(companion);

  if (AppDebugMessageHandler::instance())
    AppDebugMessageHandler::instance()->installAppMessageHandler();

  CustomDebug::setFilterRules();

  // handle settings export/import/reset via CLI

  bool doExport = args.contains("--export");
  bool doImport = args.contains("--import");
  bool doReset = args.contains("--defaults");

  if (doExport)
    Helpers::exportAppSettings();

  if (doReset) {
    char ok;
    printf("%s [Y/N] ", QCoreApplication::translate("Companion", "Reset ALL application settings to default values and remove radio profiles, are you sure?").toUtf8().constData());
    std::cin >> ok;
    if (ok != 'Y' && ok != 'y')
      exit(0);
    if (!doExport) {
      printf("%s [Y/N] ", QCoreApplication::translate("Companion", "Would you like to perform a backup first?").toUtf8().constData());
      std::cin >> ok;
      if (ok == 'Y' || ok == 'y')
        Helpers::exportAppSettings();
    }
    g.resetAllSettings();
    g.storeAllSettings();
    std::cout << QCoreApplication::translate("Companion", "Application settings were reset and saved.").toUtf8().constData() << std::endl;
  }

  if (doImport)
    checkSettingsImport(true);

  // give option to quit before settings init
  if (args.contains("--quit"))
    exit(0);

  // end CLI handling

  if (!doImport && g.isFirstUse())
    checkSettingsImport(false);

  g.init();

  QFile dbgLog;
  if (AppDebugMessageHandler::instance() && g.appDebugLog() && !g.appLogsDir().isEmpty() && QDir().mkpath(g.appLogsDir())) {
    QString fn = g.appLogsDir() % "/CompanionDebug_" % QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss") % ".log";
    dbgLog.setFileName(fn);
    if (dbgLog.open(QIODevice::WriteOnly | QIODevice::Text)) {
      AppDebugMessageHandler::instance()->addOutputDevice(&dbgLog);
    }
  }

#ifdef __APPLE__
  app.setStyle(new MyProxyStyle);
#endif

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
    fprintf(stderr, "ERROR: couldn't initialize SDL: %s\n", SDL_GetError());
  }
#endif

  Q_INIT_RESOURCE(hwdefs);
  gBoardFactories = new BoardFactories();
  registerStorageFactories();
  registerOpenTxFirmwares();
  SimulatorLoader::registerSimulators();

  Profile & profile = g.currentProfile();
  if (profile.fwType().isEmpty()){
    profile.fwType(Firmware::getDefaultVariant()->getId());
    profile.fwName("");
  }

  Firmware::setCurrentVariant(Firmware::getFirmwareForId(g.profile[g.id()].fwType()));

  MainWindow *mainWin = new MainWindow();
  mainWin->show();

  if (g.showSplash()) {
    QSplashScreen *splash = new QSplashScreen(QPixmap(":/images/splash.png"));
    QTimer::singleShot(1, [=] {
      splash->show();
    });
    QTimer::singleShot(1000*SPLASH_TIME, [=] {
      splash->close();
      delete splash;
    });
  }

  int result = app.exec();

  delete mainWin;

  SimulatorLoader::unregisterSimulators();
  unregisterOpenTxFirmwares();
  unregisterStorageFactories();
  gBoardFactories->unregisterBoardFactories();

#if defined(JOYSTICKS) || defined(SIMU_AUDIO)
  SDL_Quit();
#endif

  qDebug() << "COMPANION EXIT" << result;

  if (dbgLog.isOpen()) {
    AppDebugMessageHandler::instance()->removeOutputDevice(&dbgLog);
    dbgLog.close();
  }

  return result;
}
