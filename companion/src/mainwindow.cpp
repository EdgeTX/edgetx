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

#include "mainwindow.h"
#include "mdichild.h"
#include "burnconfigdialog.h"
#include "comparedialog.h"
#include "logsdialog.h"
#include "apppreferencesdialog.h"
#include "firmwareinterface.h"
#include "printdialog.h"
#include "version.h"
#include "customizesplashdialog.h"
#include "flashfirmwaredialog.h"
#include "hexinterface.h"
#include "warnings.h"
#include "helpers.h"
#include "appdata.h"
#include "radionotfound.h"
#include "process_sync.h"
#include "radiointerface.h"
#include "progressdialog.h"
#include "progresswidget.h"
#include "storage.h"
#include "translations.h"

#include "dialogs/filesyncdialog.h"
#include "profilechooser.h"
#include "constants.h"
#include "updates/updates.h"
#include "updates/updatefactories.h"

#include <QtGui>
#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>

MainWindow::MainWindow():
  updateFactories(nullptr),
  windowsListActions(new QActionGroup(this))
{
  // setUnifiedTitleAndToolBarOnMac(true);
  this->setWindowIcon(QIcon(":/icon.png"));
  setAcceptDrops(true);

  mdiArea = new QMdiArea(this);
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setTabsClosable(true);
  mdiArea->setTabsMovable(true);
  mdiArea->setDocumentMode(true);
  connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenus);

  setCentralWidget(mdiArea);

  createActions();
  createMenus();
  createToolBars();
  retranslateUi();

  initWindowOptions();

  connect(windowsListActions, &QActionGroup::triggered, this, &MainWindow::onChangeWindowAction);
  connect(&g, &AppData::currentProfileChanged, this, &MainWindow::onCurrentProfileChanged);

  // give time to the splash to disappear and main window to open before starting updates
  int updateDelay = 1000;
  bool showSplash = g.showSplash();
  if (showSplash) {
    updateDelay += (SPLASH_TIME*1000);
  }

  updateFactories = new UpdateFactories();

  if (g.isFirstUse()) {
    g.warningId(g.warningId() | AppMessages::MSG_WELCOME);
    QTimer::singleShot(updateDelay-500, this, SLOT(editAppSettings()));  // must be shown before warnings dialog but after splash
  }
  else {
    if (!g.previousVersion().isEmpty())
      g.warningId(g.warningId() | AppMessages::MSG_UPGRADED);

    if (g.promptProfile()) {
      chooseProfile();
    }
  }
  QTimer::singleShot(updateDelay, this, SLOT(displayWarnings()));

  QStringList strl = QApplication::arguments();
  QString str;
  QString printfilename;
  int printing = strl.contains("--print");
  int model = -1;
  int count = 0;
  foreach(QString arg, strl) {
    count++;
    if (arg.contains("--model")) {
      model = strl[count].toInt() - 1;
    }
    if (arg.contains("--filename")) {
      printfilename = strl[count];
    }
  }
  if (strl.count() > 1)
    str = strl[1];
  if (!str.isEmpty()) {
    int fileType = getStorageType(str);

    if (fileType == STORAGE_TYPE_ETX) {
      MdiChild * child = createMdiChild();
      if (child->loadFile(str)) {
        if (!(printing && model >= 0 && (getCurrentFirmware()->getCapability(Models) == 0 || model<getCurrentFirmware()->getCapability(Models)) && !printfilename.isEmpty())) {
          statusBar()->showMessage(tr("File loaded"), 2000);
          child->show();
        }
        else {
          child->show();
          child->print(model, printfilename);
          child->close();
        }
      }
      else {
        child->closeFile(true);
      }
    }
  }
  if (printing) {
    QTimer::singleShot(0, this, SLOT(autoClose()));
  }

  if (checkProfileRadioExists(g.sessionId()))
    QTimer::singleShot(updateDelay, this, &MainWindow::autoCheckForUpdates);
  else
    g.warningId(g.warningId() | AppMessages::MSG_NO_RADIO_TYPE);
}

MainWindow::~MainWindow()
{
  if (windowsListActions) {
    delete windowsListActions;
    windowsListActions = nullptr;
  }

  if (updateFactories) {
    delete updateFactories;
    updateFactories = nullptr;
  }
}

void MainWindow::initWindowOptions()
{
  updateMenus();
  setIconThemeSize(g.iconSize());
  restoreGeometry(g.mainWinGeo());
  restoreState(g.mainWinState());
  setTabbedWindows(g.tabbedMdi());
}

void MainWindow::displayWarnings()
{
  static int shownMsgs = 0;
  const int showMsgs = g.warningId();
  int msgId = 0;
  for (int i = 1; i < AppMessages::MSG_ENUM_END; i <<= 1) {
    if ((showMsgs & i) && !(shownMsgs & i)) {
      msgId = i;
      break;
    }
  }
  if (!msgId)
    return;
  AppMessages::displayMessage(msgId, this);
  shownMsgs |= msgId;
  if (shownMsgs != showMsgs)
    displayWarnings();  // in case more warnings need showing
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#ifdef __APPLE__
  // If simulator is running ignore this Quit event (simulator will still be closed)
  // - prevents app crash on exit
  if (isSimulatorRunning()) {
    event->ignore();
    return;
  }
#endif
  g.mainWinGeo(saveGeometry());
  g.mainWinState(saveState());
  g.tabbedMdi(tabbedWindowsAct->isChecked());
  QApplication::closeAllWindows();
  mdiArea->closeAllSubWindows();
  if (mdiArea->currentSubWindow()) {
    event->ignore();
  }
  else {
    event->accept();
  }
}

void MainWindow::changeEvent(QEvent * e)
{
  QMainWindow::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      retranslateUi(true);
      break;
    default:
      break;
  }
}

void MainWindow::setLanguage(const QString & langString)
{
  g.locale(langString);
  Translations::installTranslators();
}

void MainWindow::onLanguageChanged(QAction * act)
{
  QString lang = act->property("locale").toString();
  if (!lang.isNull())
    setLanguage(lang);
}

void MainWindow::autoCheckForUpdates()
{
  statusBar()->showMessage(tr("Checking for updates..."));
  doUpdates(true, false);
  statusBar()->clearMessage();
}

void MainWindow::manualCheckForUpdates()
{
  statusBar()->showMessage(tr("Checking for updates..."));
  doUpdates(true);
  statusBar()->clearMessage();
}

void MainWindow::updates()
{
  doUpdates(false);
}

void MainWindow::doUpdates(bool check, bool interactive)
{
  Updates *upd = new Updates(this, updateFactories);

  connect(upd, &Updates::runSDSync, [=] () {
    sdsync(true);
  });

  if (check)
    upd->autoUpdates(interactive);
  else
    upd->manualUpdates();

  delete upd;
}

void  MainWindow::setTheme(int index)
{
  g.theme(index);
  QMessageBox::information(this, CPN_STR_APP_NAME, tr("The new theme will be loaded the next time you start Companion."));
}

void MainWindow::onThemeChanged(QAction * act)
{
  bool ok;
  int id = act->property("themeId").toInt(&ok);
  if (ok && id >= 0 && id < 5)
    setTheme(id);
}

void  MainWindow::setIconThemeSize(int index)
{
  if (index != g.iconSize())
    g.iconSize(index);

  QSize size;
  switch(g.iconSize()) {
    case 0:
      size=QSize(16,16);
      break;
    case 2:
      size=QSize(32,32);
      break;
    case 3:
      size=QSize(48,48);
      break;
    case 1:
    default:
      size=QSize(24,24);
      break;
  }
  this->setIconSize(size);
}

void MainWindow::onIconSizeChanged(QAction * act)
{
  bool ok;
  int id = act->property("sizeId").toInt(&ok);
  if (ok && id >= 0 && id < 4)
    setIconThemeSize(id);
}

void MainWindow::setTabbedWindows(bool on)
{
  mdiArea->setViewMode(on ? QMdiArea::TabbedView : QMdiArea::SubWindowView);
  if (tileWindowsAct)
    tileWindowsAct->setDisabled(on);
  if (cascadeWindowsAct)
    cascadeWindowsAct->setDisabled(on);

  if (tabbedWindowsAct->isChecked() != on)
    tabbedWindowsAct->setChecked(on);
}

void MainWindow::newFile()
{
  MdiChild * child = createMdiChild();
  child->newFile();
  child->show();
}

void MainWindow::openDocURL()
{
  QString link = "https://edgetx.org/";
  QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::openFile(const QString & fileName)
{
  if (!fileName.isEmpty()) {
    QMdiSubWindow *existing = findMdiChild(fileName);
    if (existing) {
      mdiArea->setActiveSubWindow(existing);
      return;
    }

    MdiChild *child = createMdiChild();
    if (child->loadFile(fileName)) {
      statusBar()->showMessage(tr("File loaded"), 2000);
      child->show();
    }
    else {
      child->closeFile(true);
    }
  }
}

void MainWindow::openFile()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Models and Settings file"), g.eepromDir(), EEPROM_FILES_FILTER);
  openFile(fileName);
}

void MainWindow::save()
{
  if (activeMdiChild() && activeMdiChild()->save()) {
    statusBar()->showMessage(tr("File saved"), 2000);
  }
}

void MainWindow::saveAs()
{
  if (activeMdiChild() && activeMdiChild()->saveAs()) {
    statusBar()->showMessage(tr("File saved"), 2000);
  }
}

void MainWindow::saveAll()
{
  foreach (QMdiSubWindow * window, mdiArea->subWindowList()) {
    MdiChild * child;
    if ((child = qobject_cast<MdiChild *>(window->widget())) && child->isWindowModified())
      child->save();
  }
}

void MainWindow::closeFile()
{
  if (mdiArea->activeSubWindow())
    mdiArea->activeSubWindow()->close();
}

void MainWindow::openRecentFile()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (action) {
    QString fileName = action->data().toString();
    openFile(fileName);
  }
}

bool MainWindow::checkProfileRadioExists(int profId)
{
  const QString profType = g.getProfile(profId).fwType();
  return (Firmware::getFirmwareForId(profType)->getFirmwareBase()->getId() == profType.section('-', 0, 1));
}

bool MainWindow::loadProfileId(const unsigned pid)  // TODO Load all variables - Also HW!
{
  if (pid >= MAX_PROFILES)
    return false;

  Firmware * newFw = Firmware::getFirmwareForId(g.getProfile(pid).fwType());
  // warn if the selected profile doesn't exist
  if (!checkProfileRadioExists(pid))
    AppMessages::displayMessage(AppMessages::MSG_NO_RADIO_TYPE, this);
  // warn if we're switching between incompatible board types and any files have been modified
  if (!Boards::isBoardCompatible(Firmware::getCurrentVariant()->getBoard(), newFw->getBoard()) && anyChildrenDirty()) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME,
                              tr("There are unsaved file changes which you may lose when switching radio types.\n\nDo you wish to continue?"),
                              (QMessageBox::Yes | QMessageBox::No), QMessageBox::No) != QMessageBox::Yes) {
      updateProfilesActions();
      return false;
    }
  }

  // Set the new profile number
  g.id(pid);
  return true;
}

void MainWindow::loadProfile()
{
  QAction * action = qobject_cast<QAction *>(sender());
  if (action) {
    bool ok = false;
    unsigned profnum = action->data().toUInt(&ok);
    if (ok)
      loadProfileId(profnum);
  }
}

void MainWindow::editAppSettings()
{
  AppPreferencesDialog * dialog = new AppPreferencesDialog(this, updateFactories);
  dialog->setMainWinHasDirtyChild(anyChildrenDirty());
  connect(dialog, &AppPreferencesDialog::firmwareProfileAboutToChange, this, &MainWindow::saveAll);
  connect(dialog, &AppPreferencesDialog::firmwareProfileChanged, this, &MainWindow::onCurrentProfileChanged);
  dialog->exec();
  dialog->deleteLater();
  updateMenus();
}

void MainWindow::sdsync(bool postUpdate)
{
  // remember user-selectable options for duration of session  TODO: save to settings
  static SyncProcess::SyncOptions syncOpts;
  static bool showExtraOptions = false;
  QStringList errorMsgs;

  if (syncOpts.sessionId != g.sessionId()) {
    syncOpts.reset();
    syncOpts.folderA = QString();
    syncOpts.folderB = QString();
    syncOpts.sessionId = g.sessionId();
  }

  if (postUpdate)
    syncOpts.folderA = g.lastUpdateDir();

  if (syncOpts.folderA.isEmpty())
    syncOpts.folderA = g.profile[g.id()].sdPath();
  if (syncOpts.folderB.isEmpty())
    syncOpts.folderB = findMassstoragePath("SOUNDS", true);

  if (syncOpts.folderA.isEmpty())
    errorMsgs << tr("No local SD structure path configured!");
  if (syncOpts.folderB.isEmpty())
    errorMsgs << tr("No Radio or SD card detected!");

  QPointer<FileSyncDialog> dlg = new FileSyncDialog(this, syncOpts);
  dlg->setAttribute(Qt::WA_DeleteOnClose, true);
  dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);
  dlg->setWindowTitle(tr("Synchronize SD"));
  dlg->setWindowIcon(CompanionIcon("sdsync.png"));
  dlg->setFolderNameA(tr("Local Folder"));
  dlg->setFolderNameB(tr("Radio Folder"));
  dlg->toggleExtraOptions(showExtraOptions);
  if (errorMsgs.size())
    dlg->setStatusText(errorMsgs.join('\n'), QtWarningMsg);
  dlg->show();

  connect(dlg.data(), &FileSyncDialog::finished, [=](int) {
    if (!dlg.isNull()) {
      syncOpts = dlg->syncOptions();
      showExtraOptions = dlg->extraOptionsVisible();
    }
  });
}

void MainWindow::changelog()
{
  QString link = "https://github.com/EdgeTX/edgetx/releases";
  QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::customizeSplash()
{
  auto * dialog = new CustomizeSplashDialog(this);
  dialog->exec();
  dialog->deleteLater();
}

void MainWindow::writeSettings()
{
  StatusDialog *status = new StatusDialog(this, tr("Writing models and settings to radio"), tr("In progress..."), 400);

  if (activeMdiChild())
    activeMdiChild()->writeSettings(status);

  delete status;
}

void MainWindow::readSettings()
{
  Board::Type board = getCurrentBoard();
  QString tempFile;
  if (Boards::getCapability(board, Board::HasSDCard))
    tempFile = generateProcessUniqueTempFileName("temp.etx");
  else
    tempFile = generateProcessUniqueTempFileName("temp.bin");

  qDebug() << "Reading models and settings into temp file: " << tempFile;

  if (readSettingsFromRadio(tempFile)) {
    MdiChild * child = createMdiChild();
    child->newFile(false);
    child->loadFile(tempFile, false);
    child->show();
    qunlink(tempFile);
  }
}

bool MainWindow::readFirmwareFromRadio(const QString & filename)
{
  ProgressDialog progressDialog(this, tr("Read Firmware from Radio"), CompanionIcon("read_flash.png"));
  bool result = readFirmware(filename, progressDialog.progress());
  if (!result && !progressDialog.isEmpty()) {
    progressDialog.exec();
  }
  return result;
}

bool MainWindow::readSettingsFromRadio(const QString & filename)
{
  ProgressDialog progressDialog(this, tr("Read Models and Settings from Radio"), CompanionIcon("read_eeprom.png"));
  bool result = ::readSettings(filename, progressDialog.progress());
  if (!result) {
    if (!progressDialog.isEmpty()) {
      progressDialog.exec();
    }
  }
  else {
    statusBar()->showMessage(tr("Models and Settings read"), 2000);
  }
  return result;
}

void MainWindow::writeBackup()
{
  QMessageBox::information(this, CPN_STR_APP_NAME, tr("This function is not yet implemented"));
}

void MainWindow::writeFlash(QString fileToFlash)
{
  FlashFirmwareDialog * cd = new FlashFirmwareDialog(this);
  cd->exec();
}

void MainWindow::readBackup()
{
  if (IS_FAMILY_HORUS_OR_T16(getCurrentBoard())) {
    QMessageBox::information(this, CPN_STR_APP_NAME, tr("This function is not yet implemented"));
    return;
    // TODO implementation
  }
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Radio Backup to File"), g.eepromDir(), EXTERNAL_EEPROM_FILES_FILTER);
  if (!fileName.isEmpty()) {
    if (!readSettingsFromRadio(fileName))
      return;
  }
}

void MainWindow::readFlash()
{
  QString fileName = QFileDialog::getSaveFileName(this,tr("Read Radio Firmware to File"), g.flashDir(), FLASH_FILES_FILTER);
  if (!fileName.isEmpty()) {
    readFirmwareFromRadio(fileName);
  }
}

void MainWindow::burnConfig()
{
  burnConfigDialog *bcd = new burnConfigDialog(this);
  bcd->exec();
  delete bcd;
}

void MainWindow::burnList()
{
  burnConfigDialog bcd(this);
}

void MainWindow::compare()
{
  CompareDialog *fd = new CompareDialog(this,getCurrentFirmware());
  fd->setAttribute(Qt::WA_DeleteOnClose, true);
  fd->show();
}

void MainWindow::logFile()
{
  LogsDialog *fd = new LogsDialog(this);
  fd->setWindowFlags(Qt::Window);   //to show minimize an maximize buttons
  fd->setAttribute(Qt::WA_DeleteOnClose, true);
  fd->show();
}

void MainWindow::about()
{
  QString aboutStr = "<center><img src=\":/images/companion-title.png\"></center><br/>";
  aboutStr.append(tr("EdgeTX Home Page: <a href='%1'>%1</a>").arg(EDGETX_HOME_PAGE_URL));
  aboutStr.append("<br/><br/>");
  aboutStr.append(tr("The EdgeTX project was originally forked from <a href='%1'>OpenTX</a>").arg("https://github.com/opentx/opentx"));
  aboutStr.append("<br/><br/>");
  aboutStr.append(tr("If you've found this program useful, please support by <a href='%1'>donating</a>").arg(EDGETX_DONATE_URL));
  aboutStr.append("<br/><br/>");
#if defined(VERSION_TAG)
  aboutStr.append(QString("Version %1 \"%2\", %3").arg(VERSION_TAG).arg(CODENAME).arg(__DATE__));
#else
  aboutStr.append(QString("Version %1-%2, %3").arg(VERSION).arg(VERSION_SUFFIX).arg(__DATE__));
  aboutStr.append("<br/>");
  aboutStr.append(QString("Commit <a href='%1'>%2</a>").arg(EDGETX_COMMIT_URL % GIT_STR).arg(GIT_STR));
#endif
  aboutStr.append("<br/><br/>");
  aboutStr.append(tr("File new <a href='%1'>Issue or Request</a>").arg(EDGETX_ISSUES_URL));
  aboutStr.append("<br/><br/>");
  aboutStr.append(tr("Copyright") + QString(" &copy; 2021-%1 EdgeTX<br/>").arg(BUILD_YEAR));

  QMessageBox msgBox(this);
  msgBox.setWindowIcon(CompanionIcon("information.png"));
  msgBox.setWindowTitle(tr("About EdgeTX Companion"));
  msgBox.setText(aboutStr);
  msgBox.exec();
}

void MainWindow::updateMenus()
{
  QMdiSubWindow * activeChild = mdiArea->activeSubWindow();

  newAct->setEnabled(true);
  openAct->setEnabled(true);
  saveAct->setEnabled(activeChild);
  saveAsAct->setEnabled(activeChild);
  closeAct->setEnabled(activeChild);
  compareAct->setEnabled(activeChild);
  writeSettingsAct->setEnabled(activeChild && !activeMdiChild()->invalidModels());
  readSettingsAct->setEnabled(true);
  writeSettingsSDPathAct->setEnabled(activeChild && isSDPathValid() && !activeMdiChild()->invalidModels());
  readSettingsSDPathAct->setEnabled(isSDPathValid());
  writeBUToRadioAct->setEnabled(false);
  readBUToFileAct->setEnabled(false);
  editSplashAct->setDisabled(Boards::getBoardCapability(getCurrentBoard(), Board::HasColorLcd));

  foreach (QAction * act, fileWindowActions) {
    if (!act)
      continue;
    if (fileMenu && fileMenu->actions().contains(act))
      fileMenu->removeAction(act);
    if (fileToolBar && fileToolBar->actions().contains(act)) {
      fileToolBar->removeAction(act);
    }
    if (act->isSeparator() && act->parent() == this)
      delete act;
  }

  fileWindowActions.clear();

  if (activeChild) {
    modelsMenu->clear();
    modelsMenu->addActions(activeMdiChild()->getEditActions());
    modelsMenu->addSeparator();
    modelsMenu->addActions(activeMdiChild()->getModelActions());  // maybe separate menu/toolbar?
    modelsMenu->addAction(compareAct);
    modelsMenu->setEnabled(true);

    modelsToolBar->clear();
    modelsToolBar->addActions(activeMdiChild()->getEditActions());
    modelsToolBar->addAction(compareAct);
    modelsToolBar->setEnabled(true);

    if (activeMdiChild()->getAction(MdiChild::ACT_MDL_MOV)) {
      // workaround for default split button appearance of action with menu  :-/
      QToolButton * btn;
      if ((btn = qobject_cast<QToolButton *>(modelsToolBar->widgetForAction(activeMdiChild()->getAction(MdiChild::ACT_MDL_MOV)))))
        btn->setPopupMode(QToolButton::InstantPopup);
    }

    fileWindowActions = activeMdiChild()->getGeneralActions();
    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    fileWindowActions.insert(0, sep);
    radioMenu->addActions(fileWindowActions);
    modelsToolBar->addActions(fileWindowActions); // do not put these in radio toolbar
  }
  else {
    modelsToolBar->setDisabled(true);
    modelsMenu->setDisabled(true);
  }

  foreach (QAction * act, windowsListActions->actions()) {
    if (act->property("window_ptr").canConvert<QMdiSubWindow *>() &&
        act->property("window_ptr").value<QMdiSubWindow *>() == activeChild) {
      act->setChecked(true);
      break;
    }
  }

  updateRecentFileActions();
  updateProfilesActions();
  setWindowTitle(tr("%1 %2 - Radio: %3 - Profile: %4").arg(CPN_STR_APP_NAME).arg(VERSION).arg(getCurrentFirmware()->getName()).arg(g.profile[g.id()].name()));
}

MdiChild * MainWindow::createMdiChild()
{
  QMdiSubWindow * win = new QMdiSubWindow();
  MdiChild * child = new MdiChild(this, win);
  win->setAttribute(Qt::WA_DeleteOnClose);
  win->setWidget(child);
  mdiArea->addSubWindow(win);
  if (g.mdiWinGeo().size() < 10 && g.mdiWinGeo() == "maximized")
    win->showMaximized();

  connect(this, &MainWindow::firmwareChanged, child, &MdiChild::onFirmwareChanged);
  connect(child, &MdiChild::windowTitleChanged, this, &MainWindow::onSubwindowTitleChanged);
  connect(child, &MdiChild::modified, this, &MainWindow::onSubwindowModified);
  connect(child, &MdiChild::newStatusMessage, statusBar(), &QStatusBar::showMessage);
  connect(child, &MdiChild::destroyed, win, &QMdiSubWindow::close);
  connect(child, &MdiChild::navigationUpdated, this, &MainWindow::updateMenus);
  connect(win, &QMdiSubWindow::destroyed, this, &MainWindow::updateWindowActions);

  updateWindowActions();
  return child;
}

QAction * MainWindow::addAct(const QString & icon, const char *slot, const QKeySequence & shortcut, QObject *slotObj, const char * signal)
{
  QAction * newAction = new QAction( this );
  newAction->setMenuRole(QAction::NoRole);
  if (!icon.isEmpty())
    newAction->setIcon(CompanionIcon(icon));
  if (!shortcut.isEmpty())
    newAction->setShortcut(shortcut);
  if (slot) {
    if (!slotObj)
      slotObj = this;
    if (!signal)
      connect(newAction, SIGNAL(triggered()), slotObj, slot);
    else
      connect(newAction, signal, slotObj, slot);
  }
  return newAction;
}

QAction * MainWindow::addActToGroup(QActionGroup * aGroup, const QString & sName, const QString & lName, const char * propName, const QVariant & propValue, const QVariant & dfltValue, const QKeySequence & shortcut)
{
  QAction * act = aGroup->addAction(sName);
  act->setMenuRole(QAction::NoRole);
  act->setStatusTip(lName);
  act->setCheckable(true);
  if (!shortcut.isEmpty())
    act->setShortcut(shortcut);
  if (propName) {
    act->setProperty(propName, propValue);
    if (propValue == dfltValue)
      act->setChecked(true);
  }
  return act;
}

void MainWindow::trAct(QAction * act, const QString & text, const QString & descript)
{
  if (!text.isEmpty())
    act->setText(text);
  if (!descript.isEmpty())
    act->setStatusTip(descript);
}

void MainWindow::retranslateUi(bool showMsg)
{
  trAct(newAct,                 tr("New"),                                   tr("Create a new Models and Settings file"));
  trAct(openAct,                tr("Open..."),                               tr("Open an existing Models and Settings file"));
  trAct(saveAct,                tr("Save"),                                  tr("Save to Models and Settings file"));
  trAct(saveAsAct,              tr("Save As..."),                            tr("Save Models and Settings to another file name"));
  trAct(closeAct,               tr("Close"),                                 tr("Close Models and Settings file"));
  trAct(recentFilesAct,         tr("Recent Files"),                          tr("List of recently used files"));
  trAct(writeSettingsSDPathAct, tr("Write Models and Settings to SD Path"),  tr("Write Models and Settings to SD Path"));
  trAct(readSettingsSDPathAct,  tr("Read Models and Settings from SD Path"), tr("Read Models and Settings from SD Path"));
  trAct(exitAct,                tr("Exit"),                                  tr("Exit the application"));

  trAct(editAppSettingsAct,   tr("Edit Settings..."),  tr("Edit %1 and Simulator settings (including radio profiles) settings").arg(CPN_STR_APP_NAME));
  trAct(exportAppSettingsAct, tr("Export Settings..."), tr("Save all the current %1 and Simulator settings (including radio profiles) to a file.").arg(CPN_STR_APP_NAME));
  trAct(importAppSettingsAct, tr("Import Settings..."), tr("Load %1 and Simulator settings from a prevously exported settings file.").arg(CPN_STR_APP_NAME));

  trAct(burnConfigAct,      tr("Configure Radio Communications..."),   tr("Configure Companion for communicating with the Radio"));
  trAct(editSplashAct,      tr("Edit Radio Splash Image..."),          tr("Edit the splash image of your Radio"));
  trAct(readFlashAct,       tr("Read Firmware from Radio"),            tr("Read firmware from Radio"));
  trAct(writeFlashAct,      tr("Write Firmware to Radio"),             tr("Write firmware to Radio"));
  trAct(writeSettingsAct,   tr("Write Models and Settings to Radio"),  tr("Write Models and Settings to Radio"));
  trAct(readSettingsAct,    tr("Read Models and Settings from Radio"), tr("Read Models and Settings from Radio"));
  trAct(writeBUToRadioAct,  tr("Write Backup to Radio"),               tr("Write Backup from file to Radio"));
  trAct(readBUToFileAct,    tr("Backup Radio to File"),                tr("Save a complete backup file of all settings and model data in the Radio"));

  trAct(compareAct,         tr("Compare Models"),         tr("Compare models"));
  trAct(updatesAct,         tr("Update components..."),   tr("Download and update EdgeTX components and supporting resources"));
  trAct(sdsyncAct,          tr("Synchronize SD card..."), tr("SD card synchronization"));
  trAct(logsAct,            tr("View Log File..."),       tr("Open and view log file"));

  trAct(profilesMenuAct,    tr("Radio Profiles"),                  tr("Create or Select Radio Profiles"));
  trAct(createProfileAct,   tr("Add Radio Profile"),               tr("Create a new Radio Settings Profile"));
  trAct(copyProfileAct,     tr("Copy Current Radio Profile"),      tr("Duplicate current Radio Settings Profile"));
  trAct(deleteProfileAct,   tr("Delete Current Radio Profile..."), tr("Delete the current Radio Settings Profile"));

  trAct(viewFileToolbarAct,     tr("File Toolbar"),     tr("Configure File toolbar visibility"));
  trAct(viewModelsToolbarAct,   tr("Models Toolbar"),   tr("Configure Models toolbar visibility"));
  trAct(viewRadioToolbarAct,    tr("Radio Toolbar"),    tr("Configure Radio toolbar visibility"));
  trAct(viewSettingsToolbarAct, tr("Settings Toolbar"), tr("Configure Settings toolbar visibility"));
  trAct(viewToolsToolbarAct,    tr("Tools Toolbar"),    tr("Configure Tools toolbar visibility"));

  trAct(tabbedWindowsAct,   tr("Tabbed Windows"),    tr("Use tabs to arrange open windows."));
  trAct(tileWindowsAct,     tr("Tile Windows"),      tr("Arrange open windows across all the available space."));
  trAct(cascadeWindowsAct,  tr("Cascade Windows"),   tr("Arrange all open windows in a stack."));
  trAct(closeAllWindowsAct, tr("Close All Windows"), tr("Closes all open files (prompts to save if necessary."));

  trAct(manualChkForUpdAct, tr("Check for updates..."), tr("Check for updates to EdgeTX and supporting resources"));
  trAct(changelogAct,       tr("Release notes..."),     tr("Show release notes"));
  trAct(aboutAct,           tr("About"),                tr("Show the application's About box"));

  fileMenu->setTitle(tr("File"));
  viewMenu->setTitle(tr("View"));
  modelsMenu->setTitle(tr("Models"));
  settingsMenu->setTitle(tr("Settings"));
  themeMenu->setTitle(tr("Set Icon Theme"));
  iconThemeSizeMenu->setTitle(tr("Set Icon Size"));
  radioMenu->setTitle(tr("Radio"));
  toolsMenu->setTitle(tr("Tools"));
  windowMenu->setTitle(tr("Window"));
  helpMenu->setTitle(tr("Help"));

  fileToolBar->setWindowTitle(tr("File"));
  modelsToolBar->setWindowTitle(tr("Models"));
  radioToolBar->setWindowTitle(tr("Radio"));
  settingsToolBar->setWindowTitle(tr("Settings"));
  toolsToolBar->setWindowTitle(tr("Tools"));

  showReadyStatus();

  if (showMsg)
    QMessageBox::information(this, CPN_STR_APP_NAME, tr("Some text will not be translated until the next time you start Companion. Please note that some translations may not be complete."));
}

void MainWindow::createActions()
{
  newAct =                 addAct("new.png",                SLOT(newFile()),          QKeySequence::New);
  openAct =                addAct("open.png",               SLOT(openFile()),         QKeySequence::Open);
  saveAct =                addAct("save.png",               SLOT(save()),             QKeySequence::Save);
  saveAsAct =              addAct("saveas.png",             SLOT(saveAs()),           tr("Ctrl+Shift+S"));       // Windows doesn't have "native" save-as key, Lin/OSX both use this one anyway
  // assigned menus in createMenus()
  recentFilesAct =         addAct("recentdocument.png");
  closeAct =               addAct("clear.png",              SLOT(closeFile())        /*, QKeySequence::Close*/); // setting/showing this shortcut interferes with the system one (Ctrl+W/Ctrl-F4)
  writeSettingsSDPathAct = addAct("folder-tree-write.png",  SLOT(writeSettingsSDPath()));
  readSettingsSDPathAct =  addAct("folder-tree-read.png",   SLOT(readSettingsSDPath()));
  exitAct =                addAct("exit.png",               SLOT(closeAllWindows()),  QKeySequence::Quit, qApp);

  editAppSettingsAct =     addAct("apppreferences.png",     SLOT(editAppSettings()),         QKeySequence::Preferences);
  exportAppSettingsAct =   addAct("saveas.png",             SLOT(exportAppSettings()));
  importAppSettingsAct =   addAct("open.png",               SLOT(importAppSettings()));
  burnConfigAct =          addAct("configure.png",          SLOT(burnConfig()));

  compareAct =             addAct("compare.png",            SLOT(compare()),          tr("Ctrl+Alt+R"));
  updatesAct =             addAct("download.png",           SLOT(updates()),          tr("Ctrl+Alt+D"));
  editSplashAct =          addAct("paintbrush.png",         SLOT(customizeSplash()));
  // assigned menus in createMenus()
  profilesMenuAct =        addAct("profiles.png");
  createProfileAct =       addAct("new.png",                SLOT(createProfile()));
  copyProfileAct   =       addAct("copy.png",               SLOT(copyProfile()));
  deleteProfileAct =       addAct("clear.png",              SLOT(deleteCurrentProfile()));
  sdsyncAct =              addAct("sdsync.png",             SLOT(sdsync()));
  logsAct =                addAct("logs.png",               SLOT(logFile()),          tr("Ctrl+Alt+L"));

  burnListAct =            addAct("list.png",               SLOT(burnList()));
  readFlashAct =           addAct("read_flash.png",         SLOT(readFlash()));
  writeFlashAct =          addAct("write_flash.png",        SLOT(writeFlash()));
  writeSettingsAct =       addAct("write_eeprom.png",       SLOT(writeSettings()));
  readSettingsAct =        addAct("read_eeprom.png",        SLOT(readSettings()));
  writeBUToRadioAct =      addAct("write_eeprom_file.png",  SLOT(writeBackup()));
  readBUToFileAct =        addAct("read_eeprom_file.png",   SLOT(readBackup()));

  viewFileToolbarAct =     addAct("",                       SLOT(viewFileToolbar()));
  viewModelsToolbarAct =   addAct("",                       SLOT(viewModelsToolbar()));
  viewRadioToolbarAct =    addAct("",                       SLOT(viewRadioToolbar()));
  viewSettingsToolbarAct = addAct("",                       SLOT(viewSettingsToolbar()));
  viewToolsToolbarAct =    addAct("",                       SLOT(viewToolsToolbar()));

  tabbedWindowsAct =       addAct("",                       SLOT(setTabbedWindows(bool)), 0, this, SIGNAL(triggered(bool)));
  tileWindowsAct =         addAct("",                       SLOT(tileSubWindows()),       0, mdiArea);
  cascadeWindowsAct =      addAct("",                       SLOT(cascadeSubWindows()),    0, mdiArea);
  closeAllWindowsAct =     addAct("",                       SLOT(closeAllSubWindows()),   0, mdiArea);

  manualChkForUpdAct =     addAct("update.png",             SLOT(manualCheckForUpdates()));
  changelogAct =           addAct("changelog.png",          SLOT(changelog()));
  aboutAct =               addAct("information.png",        SLOT(about()));

  exitAct->setMenuRole(QAction::QuitRole);
  aboutAct->setMenuRole(QAction::AboutRole);
  editAppSettingsAct->setMenuRole(QAction::PreferencesRole);
  changelogAct->setMenuRole(QAction::ApplicationSpecificRole);

  compareAct->setEnabled(false);

  setActCheckability(tabbedWindowsAct, false);
  setActCheckability(viewFileToolbarAct, g.fileToolbarVisible());
  setActCheckability(viewModelsToolbarAct, g.modelsToolbarVisible());
  setActCheckability(viewRadioToolbarAct, g.radioToolbarVisible());
  setActCheckability(viewSettingsToolbarAct, g.settingsToolbarVisible());
  setActCheckability(viewToolsToolbarAct, g.toolsToolbarVisible());
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu("");
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addAction(closeAct);
  fileMenu->addAction(recentFilesAct);
  fileMenu->addSeparator();
  fileMenu->addAction(readSettingsSDPathAct);
  fileMenu->addAction(writeSettingsSDPathAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  viewMenu = menuBar()->addMenu("");
  viewMenu->addAction(viewFileToolbarAct);
  viewMenu->addAction(viewModelsToolbarAct);
  viewMenu->addAction(viewRadioToolbarAct);
  viewMenu->addAction(viewSettingsToolbarAct);
  viewMenu->addAction(viewToolsToolbarAct);
  viewMenu->addSeparator();
  viewMenu->addMenu(createLanguageMenu(viewMenu));

  modelsMenu = menuBar()->addMenu("");

  themeMenu = viewMenu->addMenu("");
  QActionGroup * themeGroup = new QActionGroup(themeMenu);
  addActToGroup(themeGroup, tr("Classical"),  tr("The classic companion9x icon theme"),  "themeId", 0, g.theme());
  addActToGroup(themeGroup, tr("Yerico"),     tr("Yellow round honey sweet icon theme"), "themeId", 1, g.theme());
  addActToGroup(themeGroup, tr("Monochrome"), tr("A monochrome black icon theme"),       "themeId", 3, g.theme());
  addActToGroup(themeGroup, tr("MonoBlue"),   tr("A monochrome blue icon theme"),        "themeId", 4, g.theme());
  addActToGroup(themeGroup, tr("MonoWhite"),  tr("A monochrome white icon theme"),       "themeId", 2, g.theme());
  connect(themeGroup, &QActionGroup::triggered, this, &MainWindow::onThemeChanged);
  themeMenu->addActions(themeGroup->actions());

  iconThemeSizeMenu = viewMenu->addMenu("");
  QActionGroup * szGroup = new QActionGroup(iconThemeSizeMenu);
  addActToGroup(szGroup, tr("Small"),  tr("Use small toolbar icons"),       "sizeId", 0, g.iconSize());
  addActToGroup(szGroup, tr("Normal"), tr("Use normal size toolbar icons"), "sizeId", 1, g.iconSize());
  addActToGroup(szGroup, tr("Big"),    tr("Use big toolbar icons"),         "sizeId", 2, g.iconSize());
  addActToGroup(szGroup, tr("Huge"),   tr("Use huge toolbar icons"),        "sizeId", 3, g.iconSize());
  connect(szGroup, &QActionGroup::triggered, this, &MainWindow::onIconSizeChanged);
  iconThemeSizeMenu->addActions(szGroup->actions());

  radioMenu = menuBar()->addMenu("");
  radioMenu->addAction(writeSettingsAct);
  radioMenu->addAction(readSettingsAct);
  radioMenu->addSeparator();
  radioMenu->addAction(writeBUToRadioAct);
  radioMenu->addAction(readBUToFileAct);
  radioMenu->addSeparator();
  radioMenu->addAction(writeFlashAct);
  radioMenu->addAction(readFlashAct);
  radioMenu->addSeparator();
  radioMenu->addAction(burnConfigAct);
  radioMenu->addAction(editSplashAct);

  settingsMenu = menuBar()->addMenu("");
  settingsMenu->addAction(editAppSettingsAct);
  settingsMenu->addAction(exportAppSettingsAct);
  settingsMenu->addAction(importAppSettingsAct);
  settingsMenu->addSeparator();
  settingsMenu->addAction(profilesMenuAct);

  toolsMenu = menuBar()->addMenu("");
  toolsMenu->addAction(updatesAct);
  toolsMenu->addAction(sdsyncAct);
  toolsMenu->addAction(logsAct);
  toolsMenu->addSeparator();

  windowMenu = menuBar()->addMenu("");
  windowMenu->addAction(tabbedWindowsAct);
  windowMenu->addAction(tileWindowsAct);
  windowMenu->addAction(cascadeWindowsAct);
  windowMenu->addAction(closeAllWindowsAct);
  windowMenu->addSeparator();

  helpMenu = menuBar()->addMenu("");
  helpMenu->addAction(manualChkForUpdAct);
  helpMenu->addAction(changelogAct);
  helpMenu->addAction(aboutAct);

  recentFilesMenu = new QMenu(this);
  recentFilesMenu->setToolTipsVisible(true);
  for ( int i = 0; i < g.historySize(); ++i) {
    recentFileActs.append(recentFilesMenu->addAction(""));
    recentFileActs[i]->setVisible(false);
    connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
  }
  recentFilesAct->setMenu(recentFilesMenu);

  profilesMenu = new QMenu(this);
  QActionGroup *profilesGroup = new QActionGroup(this);
  for (int i = 0; i < MAX_PROFILES; i++) {
    profileActs.append(profilesMenu->addAction(""));
    profileActs[i]->setVisible(false);
    profileActs[i]->setCheckable(true);
    connect(profileActs[i], SIGNAL(triggered()), this, SLOT(loadProfile()));
    profilesGroup->addAction(profileActs[i]);
  }
  profilesMenu->addSeparator();
  profilesMenu->addAction(createProfileAct);
  profilesMenu->addAction(copyProfileAct);
  profilesMenu->addAction(deleteProfileAct);
  profilesMenuAct->setMenu(profilesMenu);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar("");
  fileToolBar->setObjectName("File");
  fileToolBar->addAction(newAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(recentFilesAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(closeAct);
  fileToolBar->addSeparator();
  fileToolBar->addAction(readSettingsSDPathAct);
  fileToolBar->addAction(writeSettingsSDPathAct);

  // workaround for default split button appearance of action with menu  :-/
  QToolButton * btn;
  if ((btn = qobject_cast<QToolButton *>(fileToolBar->widgetForAction(recentFilesAct))))
    btn->setPopupMode(QToolButton::InstantPopup);

  // gets populated later
  modelsToolBar = addToolBar("");
  modelsToolBar->setObjectName("Models");

  radioToolBar = new QToolBar(this);
  addToolBar(Qt::LeftToolBarArea, radioToolBar);
  radioToolBar->setObjectName("Radio");
  radioToolBar->addAction(writeSettingsAct);
  radioToolBar->addAction(readSettingsAct);
  radioToolBar->addSeparator();
  radioToolBar->addAction(writeBUToRadioAct);
  radioToolBar->addAction(readBUToFileAct);
  radioToolBar->addSeparator();
  radioToolBar->addAction(writeFlashAct);
  radioToolBar->addAction(readFlashAct);
  radioToolBar->addSeparator();
  radioToolBar->addAction(burnConfigAct);

  settingsToolBar = addToolBar("");
  settingsToolBar->setObjectName("Settings");
  settingsToolBar->addAction(editAppSettingsAct);
  settingsToolBar->addAction(profilesMenuAct);

  if ((btn = qobject_cast<QToolButton *>(settingsToolBar->widgetForAction(profilesMenuAct))))
    btn->setPopupMode(QToolButton::InstantPopup);

  toolsToolBar = addToolBar("");
  toolsToolBar->setObjectName("Tools");
  toolsToolBar->addAction(updatesAct);
  toolsToolBar->addAction(sdsyncAct);
  toolsToolBar->addAction(logsAct);

  connect(fileToolBar, &QToolBar::visibilityChanged, [=](bool visible)
    {
      g.fileToolbarVisible(visible);
      const QSignalBlocker blocker(viewFileToolbarAct);
      viewFileToolbarAct->setChecked(g.fileToolbarVisible());
    });

  connect(modelsToolBar, &QToolBar::visibilityChanged, [=](bool visible)
    {
      g.modelsToolbarVisible(visible);
      const QSignalBlocker blocker(viewModelsToolbarAct);
      viewModelsToolbarAct->setChecked(g.modelsToolbarVisible());
    });

  connect(radioToolBar, &QToolBar::visibilityChanged, [=](bool visible)
    {
      g.radioToolbarVisible(visible);
      const QSignalBlocker blocker(viewRadioToolbarAct);
      viewRadioToolbarAct->setChecked(g.radioToolbarVisible());
    });

  connect(settingsToolBar, &QToolBar::visibilityChanged, [=](bool visible)
    {
      g.settingsToolbarVisible(visible);
      const QSignalBlocker blocker(viewSettingsToolbarAct);
      viewSettingsToolbarAct->setChecked(g.settingsToolbarVisible());
    });

  connect(toolsToolBar, &QToolBar::visibilityChanged, [=](bool visible)
    {
      g.toolsToolbarVisible(visible);
      const QSignalBlocker blocker(viewToolsToolbarAct);
      viewToolsToolbarAct->setChecked(g.toolsToolbarVisible());
    });

}

QMenu * MainWindow::createLanguageMenu(QWidget * parent)
{
  QMenu * menu = new QMenu(tr("Set Menu Language"), parent);
  QActionGroup * actGroup = new QActionGroup(menu);
  QString lName;

  addActToGroup(actGroup, tr("System language"), tr("Use default system language."), "locale", QString(""), g.locale());
  foreach (const QString & lang, Translations::getAvailableTranslations()) {
    QLocale locale(lang);
    lName = locale.nativeLanguageName();
    addActToGroup(actGroup, lName.left(1).toUpper() % lName.mid(1), tr("Use %1 language (some translations may not be complete).").arg(lName), "locale", lang, g.locale());
  }
  if (!actGroup->checkedAction())
    actGroup->actions().first()->setChecked(true);

  connect(actGroup, &QActionGroup::triggered, this, &MainWindow::onLanguageChanged);
  menu->addActions(actGroup->actions());
  return menu;
}

void MainWindow::showReadyStatus()
{
  statusBar()->showMessage(tr("Ready"));
}

MdiChild *MainWindow::activeMdiChild()
{
  if (QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
    return qobject_cast<MdiChild *>(activeSubWindow->widget());
  return 0;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
  QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

  foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
    MdiChild *mdiChild = qobject_cast<MdiChild *>(window->widget());
    if (mdiChild->currentFile() == canonicalFilePath)
      return window;
  }
  return 0;
}

bool MainWindow::anyChildrenDirty()
{
  foreach (QMdiSubWindow * window, mdiArea->subWindowList()) {
    MdiChild * child;
    if ((child = qobject_cast<MdiChild *>(window->widget())) && child->isWindowModified())
      return true;
  }
  return false;
}

void MainWindow::updateRecentFileActions()
{
  QStringList files = g.recentFiles();
  for (int i=0; i < recentFileActs.size(); i++) {
    if (i < files.size() && !files.at(i).isEmpty()) {
      recentFileActs[i]->setText(QFileInfo(files.at(i)).fileName());
      recentFileActs[i]->setData(files.at(i));
      recentFileActs[i]->setStatusTip(QDir::toNativeSeparators(files.at(i)));
      recentFileActs[i]->setToolTip(recentFileActs[i]->statusTip());
      recentFileActs[i]->setVisible(true);
    }
    else {
      recentFileActs[i]->setVisible(false);
    }
  }
}

void MainWindow::updateProfilesActions()
{
  for (int i=0; i < qMin(profileActs.size(), MAX_PROFILES); i++) {
    if (g.profile[i].existsOnDisk()) {
      QString text = tr("%2").arg(g.profile[i].name());
      profileActs[i]->setText(text);
      profileActs[i]->setData(i);
      profileActs[i]->setVisible(true);
      if (i == g.id())
        profileActs[i]->setChecked(true);
    }
    else {
      profileActs[i]->setVisible(false);
    }
  }
}

void MainWindow::updateWindowActions()
{
  if (!windowsListActions)
    return;

  foreach (QAction * act, windowsListActions->actions()) {
    windowsListActions->removeAction(act);
    if (windowMenu->actions().contains(act))
      windowMenu->removeAction(act);
    delete act;
  }
  int count = 0;
  foreach (QMdiSubWindow * win, mdiArea->subWindowList()) {
    QString scut;
    if (++count < 10)
      scut = tr("Alt+%1").arg(count);
    QAction * act = addActToGroup(windowsListActions, "", "", "window_ptr", QVariant::fromValue(win), QVariant(), scut);
    act->setChecked(win == mdiArea->activeSubWindow());
    updateWindowActionTitle(win, act);
  }
  windowMenu->addActions(windowsListActions->actions());
}

void MainWindow::updateWindowActionTitle(const QMdiSubWindow * win, QAction * act)
{
  MdiChild * child = qobject_cast<MdiChild *>(win->widget());
  if (!child)
    return;

  if (!act) {
    foreach (QAction * a, windowsListActions->actions()) {
      if (a->property("window_ptr").canConvert<QMdiSubWindow *>() &&
          a->property("window_ptr").value<QMdiSubWindow *>() == win) {
        act = a;
        break;
      }
    }
  }
  if (!act)
    return;

  QString ttl = child->userFriendlyCurrentFile();
  if (child->isWindowModified())
    ttl.prepend("* ");
  act->setText(ttl);
}

void MainWindow::onSubwindowTitleChanged()
{
  QMdiSubWindow * win = nullptr;
  if ((win = qobject_cast<QMdiSubWindow *>(sender()->parent())))
    updateWindowActionTitle(win);
}

void MainWindow::onSubwindowModified()
{
  onSubwindowTitleChanged();
}

void MainWindow::onChangeWindowAction(QAction * act)
{
  if (!act->isChecked())
    return;

  QMdiSubWindow * win = nullptr;
  if (act->property("window_ptr").canConvert<QMdiSubWindow *>())
    win = act->property("window_ptr").value<QMdiSubWindow *>();
  if (win)
    mdiArea->setActiveSubWindow(win);
}

void MainWindow::onCurrentProfileChanged()
{
  g.moveCurrentProfileToTop();
  Firmware::setCurrentVariant(Firmware::getFirmwareForId(g.currentProfile().fwType()));
  emit firmwareChanged();
  updateFactories->radioProfileChanged();
  QApplication::clipboard()->clear();
  updateMenus();
}

int MainWindow::newProfile(bool loadProfile)
{
  int i;
  for (i=0; i < MAX_PROFILES && g.profile[i].existsOnDisk(); i++)
    ;
  if (i == MAX_PROFILES) {  //Failed to find free slot
    QMessageBox::warning(this, tr("Cannot add profile"), tr("There is no space left to add a new profile. Delete an exsting profile before adding a new one."));
    return -1;
  }

  Firmware *newfw = Firmware::getDefaultVariant();
  g.profile[i].init();
  g.profile[i].name("New Radio");
  g.profile[i].fwType(newfw->getId());
  g.profile[i].defaultInternalModule(Boards::getDefaultInternalModules(newfw->getBoard()));
  g.profile[i].externalModuleSize(Boards::getDefaultExternalModuleSize(newfw->getBoard()));

  if (loadProfile) {
    if (loadProfileId(i))
      editAppSettings();
  }

  return i;
}

void MainWindow::createProfile()
{
  newProfile(true);
}

void MainWindow::copyProfile()
{
  int newId = newProfile(false);

  if (newId > -1) {
    g.profile[newId] = g.profile[g.id()];
    g.profile[newId].name(g.profile[newId].name() + tr(" - Copy"));
    if (loadProfileId(newId))
      editAppSettings();
  }
}

void MainWindow::deleteProfile(const int pid)
{
  if (pid == g.id() && anyChildrenDirty()) {
    QMessageBox::warning(this, tr("Companion :: Open files warning"), tr("Please save or close modified file(s) before deleting the active profile."));
    return;
  }
  int newPid = 0;
  if (pid == 0) {
    // Find valid profile
    for (newPid = 1; newPid < MAX_PROFILES && !g.profile[newPid].existsOnDisk(); newPid += 1);
    if (newPid == MAX_PROFILES) {
      QMessageBox::warning(this, tr("Not possible to remove profile"), tr("The default profile can not be removed."));
      return;
    }
  }
  int ret = QMessageBox::question(this,
                                  tr("Confirm Delete Profile"),
                                  tr("Are you sure you wish to delete the \"%1\" radio profile? There is no way to undo this action!").arg(g.profile[pid].name()));
  if (ret != QMessageBox::Yes)
    return;

  g.getProfile(pid).resetAll();
  loadProfileId(newPid);
  g.moveCurrentProfileToTop();
}

void MainWindow::deleteCurrentProfile()
{
  deleteProfile(g.id());
}

void MainWindow::exportAppSettings()
{
  Helpers::exportAppSettings();
}

void MainWindow::importAppSettings()
{
  if (anyChildrenDirty()) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Please save or close all modified files before importing settings"));
    return;
  }
  QString resultMsg = tr("<html>" \
    "<p>%1 and Simulator settings can be imported (restored) from a previosly saved export (backup) file. " \
      "This will replace current settings with any settings found in the file.</p>" \
    "<p>An automatic backup of the current settings will be attempted. But if the current settings are useful then it is recommended that you make a manual backup first.</p>" \
    "<p>For best results when importing settings, <b>close any other %1 windows you may have open, and make sure the standalone Simulator application is not running.</p>" \
    "<p>Do you wish to continue?</p>" \
    "</html>").arg(CPN_STR_APP_NAME);

  int ret = QMessageBox::question(this, tr("Confirm Settings Import"), resultMsg);
  if (ret != QMessageBox::Yes)
    return;

  QString impFile = CPN_SETTINGS_BACKUP_DIR;
  impFile = QFileDialog::getOpenFileName(this, tr("Select %1:").arg(CPN_STR_APP_SETTINGS_FILES), impFile, CPN_STR_APP_SETTINGS_FILTER);
  if (impFile.isEmpty() || !QFileInfo(impFile).isReadable() || QFileInfo(impFile).isExecutable())
    return;

  // Try a backup first
  QString expFile = CPN_SETTINGS_INI_PATH.arg(tr("backup") % " " % QDateTime::currentDateTime().toString("dd-MMM-yy HH-mm"));
  if (!g.exportSettingsToFile(expFile, resultMsg)) {
    resultMsg.append("\n" % tr("Press the 'Ignore' button to continue anyway."));
    if (QMessageBox::warning(this, CPN_STR_APP_NAME, resultMsg, QMessageBox::Cancel, QMessageBox::Ignore) == QMessageBox::Cancel)
      return;
    expFile.clear();
  }
  const QString prevLoc = g.locale();

  // Do the import
  QSettings fromSettings(impFile, QSettings::IniFormat);
  if (!g.importSettings(&fromSettings)) {
    QMessageBox::critical(this, CPN_STR_APP_NAME, tr("The settings could not be imported."), QMessageBox::Ok);
    return;
  }
  resultMsg = tr("<html>" \
                 "<p>New settings have been imported from:<br> %1.</p>" \
                 "<p>%2 will now re-initialize.</p>" \
                 "<p>Note that you may need to close and restart %2 before some settings like language and icon theme take effect.</p>" \
                ).arg(impFile).arg(CPN_STR_APP_NAME);

  if (!expFile.isEmpty())
    resultMsg.append(tr("<p>The previous settings were backed up to:<br> %1</p>").arg(expFile));
  resultMsg.append("</html>");
  QMessageBox::information(this, CPN_STR_APP_NAME, resultMsg);

  g.init();
  initWindowOptions();
  if (prevLoc != g.locale())
    Translations::installTranslators();
  onCurrentProfileChanged();
}

QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/uri-list"))
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  if (urls.isEmpty()) return;
  QString fileName = urls.first().toLocalFile();
  openFile(fileName);
}

void MainWindow::autoClose()
{
  this->close();
}

void MainWindow::chooseProfile()
{
  QMap<int, QString> active;
  active = g.getActiveProfiles();
  if (active.size() > 1) {
    ProfileChooserDialog *pcd = new ProfileChooserDialog(this);
    connect(pcd, &ProfileChooserDialog::profileChanged, this, &MainWindow::loadProfileId);
    pcd->exec();
    delete pcd;

    if (!checkProfileRadioExists(g.sessionId()))
      g.warningId(g.warningId() | AppMessages::MSG_NO_RADIO_TYPE);
  }
}

void MainWindow::readSettingsSDPath()
{
  QString tempFile;
  tempFile = generateProcessUniqueTempFileName("temp.etx");
  qDebug() << "Reading models and settings from SD path into temp file: " << tempFile;

  if (readSettingsFromSDPath(tempFile)) {
    MdiChild * child = createMdiChild();
    child->newFile(false);
    child->loadFile(tempFile, false);
    child->show();
    qunlink(tempFile);
  }
}

bool MainWindow::readSettingsFromSDPath(const QString & filename)
{
  ProgressDialog progressDialog(this, tr("Read Models and Settings from SD path"), CompanionIcon("read_eeprom.png"));
  bool result = ::readSettingsSDCard(filename, progressDialog.progress(), false);
  if (!result) {
    if (!progressDialog.isEmpty()) {
      progressDialog.exec();
    }
  }
  else {
    statusBar()->showMessage(tr("Models and Settings read"), 2000);
  }
  return result;
}

void MainWindow::writeSettingsSDPath()
{
  StatusDialog *status = new StatusDialog(this, tr("Writing models and settings to SD path"), tr("In progress..."), 400);

  if (activeMdiChild())
    activeMdiChild()->writeSettings(status, false);

  delete status;
}

bool MainWindow::isSDPathValid()
{
  bool ret = false;
  const QString sdPath = g.currentProfile().sdPath();
  if (!sdPath.isEmpty()) {
    if (QFile::exists(sdPath))
      ret = true;
  }
  return ret;
}

void MainWindow::setActCheckability(QAction * act, bool checked)
{
  act->setCheckable(true);
  act->setChecked(checked);
}

void MainWindow::viewFileToolbar()
{
  g.fileToolbarVisible(viewFileToolbarAct->isChecked());
  const QSignalBlocker blocker(fileToolBar);
  fileToolBar->setVisible(viewFileToolbarAct->isChecked());
}

void MainWindow::viewModelsToolbar()
{
  g.modelsToolbarVisible(viewModelsToolbarAct->isChecked());
  const QSignalBlocker blocker(modelsToolBar);
  modelsToolBar->setVisible(viewModelsToolbarAct->isChecked());
}

void MainWindow::viewRadioToolbar()
{
  g.radioToolbarVisible(viewRadioToolbarAct->isChecked());
  const QSignalBlocker blocker(radioToolBar);
  radioToolBar->setVisible(viewRadioToolbarAct->isChecked());
}

void MainWindow::viewSettingsToolbar()
{
  g.settingsToolbarVisible(viewSettingsToolbarAct->isChecked());
  const QSignalBlocker blocker(settingsToolBar);
  settingsToolBar->setVisible(viewSettingsToolbarAct->isChecked());
}

void MainWindow::viewToolsToolbar()
{
  g.toolsToolbarVisible(viewToolsToolbarAct->isChecked());
  const QSignalBlocker blocker(toolsToolBar);
  toolsToolBar->setVisible(viewToolsToolbarAct->isChecked());
}
