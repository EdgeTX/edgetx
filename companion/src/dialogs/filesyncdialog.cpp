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

#include "filesyncdialog.h"

#include "autobitsetcheckbox.h"
#include "autocheckbox.h"
#include "autocombobox.h"
#include "autolineedit.h"
#include "constants.h"
#include "helpers.h"
#include "progresswidget.h"

#include <QApplication>
#include <QComboBox>
#include <QCompleter>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QLayout>
#include <QLineEdit>
#include <QMenu>
#include <QSpinBox>
#include <QMessageBox>
#include <QPushButton>

FileSyncDialog::FileSyncDialog(QWidget * parent, const SyncProcess::SyncOptions & syncOptions) :
  QDialog(parent),
  m_syncOptions(syncOptions),
  m_running(false)
{
  setWindowTitle(tr("Synchronize Files"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setSizeGripEnabled(true);

  setupUi();
  toggleExtraOptions(false);
  validate();
  updateRunningState();
}

void FileSyncDialog::reject()
{
  // override close event via window system close or esc. key
  if (m_running) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Are you sure you wish to abort the sync?")) == QMessageBox::No)
      return;
    emit stopSync();
    // Force termination if the sync hasn't finished within 5s
    QElapsedTimer tim;
    tim.start();
    while (!tim.hasExpired(5000) && m_running)
      QApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 10);
    if (m_running)
      qWarning() << "Terminating thread while Sync process was still running.";
  }
  QDialog::reject();
}

// Create a widget with a line edit and folder select button and handles all interactions. Features autosuggest
//   path hints while typing, invalid paths shown in red.
// This should probably be moved some place more reusable, esp. the QFileSystemModel.
static QWidget * folderSelectorWidget(QLineEdit * le, QWidget * parent)
{
  static QFileSystemModel fileModel;
  static bool init = false;
  if (!init) {
    init = true;
    fileModel.setFilter(QDir::Dirs);
    fileModel.setRootPath("/");
  }

  QWidget * fsw = new QWidget(parent);
  //le = new QLineEdit(parent);
  QCompleter * fsc = new QCompleter(fsw);
  fsc->setModel(&fileModel);
  //fsc->setCompletionMode(QCompleter::InlineCompletion);
  le->setCompleter(fsc);

  QToolButton * btn = new QToolButton(fsw);
  btn->setIcon(CompanionIcon("open.png"));
  QHBoxLayout * l = new QHBoxLayout(fsw);
  l->setContentsMargins(0,0,0,0);
  l->setSpacing(3);
  l->addWidget(le);
  l->addWidget(btn);

  QObject::connect(btn, &QToolButton::clicked, [=]() {
    QString dir = QFileDialog::getExistingDirectory(parent, fsw->property("fileDialogTitle").toString(), le->text());
    if (!dir.isEmpty()) {
      le->setText(QDir::toNativeSeparators(dir));
      le->setFocus();
    }
  });

  QObject::connect(le, &QLineEdit::textChanged, [=](const QString & text) {
    if (QFile::exists(text))
      le->setStyleSheet("");
    else
      le->setStyleSheet("QLineEdit {color: red;}");
  });

  return fsw;
}

void FileSyncDialog::setupUi()
{
  m_folderNameA = tr("Folder A");
  m_folderNameB = tr("Folder B");
  int row = 0;

  CompanionIcon playIcon("play.png");
  playIcon.addImage("stop.png", QIcon::Normal, QIcon::On);

  QLabel * lblFolderA = new QLabel(m_folderNameA % ":", this);
  AutoLineEdit * leFolderA =  new AutoLineEdit(this, true);
  leFolderA->setField(m_syncOptions.folderA);
  leFolderA->setText(QDir::toNativeSeparators(m_syncOptions.folderA));
  QWidget * wdgFolderA = folderSelectorWidget(leFolderA, this);

  QLabel * lblFolderB = new QLabel(m_folderNameB % ":", this);
  AutoLineEdit * leFolderB =  new AutoLineEdit(this, true);
  leFolderB->setField(m_syncOptions.folderB);
  leFolderB->setText(QDir::toNativeSeparators(m_syncOptions.folderB));
  QWidget * wdgFolderB = folderSelectorWidget(leFolderB, this);

  ui_syncDir = new AutoComboBox(this);
  setSyncDirList();
  ui_syncDir->setField(m_syncOptions.direction);

  AutoComboBox * copyMode = new AutoComboBox(this);
  copyMode->setToolTip(tr("How to handle overwriting files which already exist in the destination folder."));
  copyMode->addItem(tr("Copy only if newer and different (compare contents)"), SyncProcess::OVERWR_NEWER_IF_DIFF);
  copyMode->addItem(tr("Copy only if newer (do not compare contents)"), SyncProcess::OVERWR_NEWER_ALWAYS);
  copyMode->addItem(tr("Copy only if different (ignore file time stamps)"), SyncProcess::OVERWR_IF_DIFF);
  copyMode->addItem(tr("Always copy (force overwite existing files)"), SyncProcess::OVERWR_ALWAYS);
  copyMode->setField(m_syncOptions.compareType);

  QSpinBox * maxSize = new QSpinBox(this);
  maxSize->setRange(0, 100 * 1024);
  maxSize->setAccelerated(true);
  maxSize->setSpecialValueText(tr("Any size"));
  maxSize->setToolTip(tr("Skip files larger than this size. Enter zero for unlimited."));
  maxSize->setGroupSeparatorShown(true);
  maxSize->setValue(0);  // we set the real default value later

  AutoComboBox * logLevel = new AutoComboBox(this);
  logLevel->setToolTip(tr("Minimum reporting level. Events of this type and of higher importance are shown.\n" \
                          "WARNING: High log rates may make the user interface temporarily unresponsive."));
  logLevel->addItem(tr("Skipped"), QtDebugMsg);
  logLevel->addItem(tr("Created"), QtInfoMsg);
  logLevel->addItem(tr("Updated"), QtWarningMsg);
  //logLevel->addItem(tr("Item Deleted"), QtCriticalMsg);  // unused
  logLevel->addItem(tr("Errors Only"), QtFatalMsg);
  logLevel->setField(m_syncOptions.logLevel);

  AutoBitsetCheckBox * testRun = new AutoBitsetCheckBox(m_syncOptions.flags, SyncProcess::OPT_DRY_RUN, tr("Test-run only"), this);
  testRun->setToolTip(tr("Run as normal but do not actually copy anything. Useful for verifying results before real sync."));

  // layout to hold size spinbox and checkbox option(s)
  QHBoxLayout * hlay1 = new QHBoxLayout();
  hlay1->addWidget(maxSize, 1);
  hlay1->addWidget(testRun);
  hlay1->addWidget(new QLabel(tr("Log Level:"), this));
  hlay1->addWidget(logLevel);

  // "extra" options

  QLabel * lbFilter = new QLabel(tr("Filters:"), this);
  lbFilter->setToolTip(tr("The \"Include\" filter will only copy files which match the pattern(s).\n" \
                          "The \"Exclude\" filter will skip files matching the filter pattern(s).\n" \
                          "The Include filter is evaluated first."));

  AutoLineEdit * leFiltExc = new AutoLineEdit(this, true);
  leFiltExc->setField(m_syncOptions.excludeFilter);
  leFiltExc->setToolTip(tr("One or more file pattern(s) to exclude, separated by commas.\n" "Blank means exclude none. ?, *, and [...] wildcards accepted."));

  AutoLineEdit * leFiltInc = new AutoLineEdit(this, true);
  leFiltInc->setField(m_syncOptions.includeFilter);
  leFiltInc->setToolTip(tr("One or more file pattern(s) to include, separated by commas.\n" "Blank means include all. ?, *, and [...] wildcards accepted."));

  // layout to hold filter options
  QHBoxLayout * filtLo = new QHBoxLayout();
  filtLo->addWidget(new QLabel(tr("Include:"), this));
  filtLo->addWidget(leFiltInc, 1);
  filtLo->addWidget(new QLabel(tr("Exclude:"), this));
  filtLo->addWidget(leFiltExc, 1);

  // file options
  QHBoxLayout * filOptsLo = new QHBoxLayout();
  filOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.dirFilterFlags, QDir::CaseSensitive, tr("Case sensitive"), this));
  filOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.dirFilterFlags, QDir::NoSymLinks, true, tr("Follow links"), this));
  filOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.dirFilterFlags, QDir::Hidden, tr("Include hidden"), this));

  // folder options
  QHBoxLayout * dirOptsLo = new QHBoxLayout();
  dirOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.flags, SyncProcess::OPT_RECURSIVE, tr("Recursive"), this));
  dirOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.flags, SyncProcess::OPT_SKIP_DIR_LINKS, true, tr("Follow links"), this));
  dirOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.dirFilterFlags, QDir::Dirs, true, tr("Skip empty"), this));
  dirOptsLo->addWidget(new AutoBitsetCheckBox(m_syncOptions.dirFilterFlags, QDir::AllDirs, true, tr("Apply filters"), this));

  // extra options container widget and layout
  ui_extraOptions = new QWidget(this);
  ui_extraOptions->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

  QGridLayout * xOptsLay = new QGridLayout(ui_extraOptions);
  xOptsLay->setContentsMargins(0, 0, 0, 0);
  xOptsLay->addWidget(lbFilter, row, 0);
  xOptsLay->addLayout(filtLo, row++, 1);
  xOptsLay->addWidget(new QLabel(tr("Filter Options:"), this), row, 0);
  xOptsLay->addLayout(filOptsLo, row++, 1);
  xOptsLay->addWidget(new QLabel(tr("Folder Options:"), this), row, 0);
  xOptsLay->addLayout(dirOptsLo, row++, 1);

  // status label
  ui_statusLabel = new QLabel(this);
  // progress widget
  ui_progress = new ProgressWidget(this);
  ui_progress->setMinimumWidth(0);
  ui_progress->setVisible(false);

  // dialog buttons & menu

  QMenu * optionsMenu = new QMenu(tr("Options"));
  QAction * xtrasAct = optionsMenu->addAction(CompanionIcon("fuses.png"), tr("Show extra options"), this, &FileSyncDialog::toggleExtraOptions);
  xtrasAct->setCheckable(true);
  connect(this, &FileSyncDialog::extraOptionsToggled, xtrasAct, &QAction::setChecked);
  optionsMenu->addAction(CompanionIcon("paintbrush.png"), tr("Reset to defaults"), this, &FileSyncDialog::resetOptions);

  QToolButton * btnOpts = new QToolButton(this);
  btnOpts->setDefaultAction(xtrasAct);
  btnOpts->setMenu(optionsMenu);
  btnOpts->setToolButtonStyle(Qt::ToolButtonIconOnly);
  btnOpts->setPopupMode(QToolButton::MenuButtonPopup);
  btnOpts->setToolTip(xtrasAct->text());

  ui_btnStartStop = new QToolButton(this);
  ui_btnStartStop->setCheckable(true);
  ui_btnStartStop->setIcon(playIcon);
  ui_btnStartStop->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  ui_btnClose = new QToolButton(this);
  ui_btnClose->setText(tr("Close"));
  ui_btnClose->setToolButtonStyle(Qt::ToolButtonTextOnly);
  ui_btnClose->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);

  ui_buttonBox = new QDialogButtonBox(this);
  ui_buttonBox->addButton(btnOpts, QDialogButtonBox::ActionRole);
  ui_buttonBox->addButton(ui_btnStartStop, QDialogButtonBox::ActionRole);
  ui_buttonBox->addButton(ui_btnClose, QDialogButtonBox::RejectRole);

  // The options group box.
  ui_optionsPanel = new QGroupBox(tr("Options"), this);
  QGridLayout * optsLay = new QGridLayout(ui_optionsPanel);
  row = 0;
  optsLay->addWidget(lblFolderA, row, 0);
  optsLay->addWidget(wdgFolderA, row++, 1);
  optsLay->addWidget(lblFolderB, row, 0);
  optsLay->addWidget(wdgFolderB, row++, 1);
  optsLay->addWidget(new QLabel(tr("Sync. Direction:"), this), row, 0);
  optsLay->addWidget(ui_syncDir, row++, 1);
  optsLay->addWidget(new QLabel(tr("Existing Files:"), this), row, 0);
  optsLay->addWidget(copyMode, row++, 1);
  optsLay->addWidget(new QLabel(tr("Max. File Size:"), this), row, 0);
  optsLay->addLayout(hlay1, row++, 1);
  optsLay->addWidget(ui_extraOptions, row, 0, 1, 2);

  // layout to hold warning label and button box
  QHBoxLayout * hlay2 = new QHBoxLayout();
  hlay2->addWidget(ui_statusLabel, 1);
  hlay2->addWidget(ui_buttonBox);

  // Create main layout and add options, progress, buttons.
  QVBoxLayout * dlgL = new QVBoxLayout(this);
  dlgL->addWidget(ui_optionsPanel, 0, Qt::AlignTop);
  dlgL->addLayout(hlay2);
  dlgL->addWidget(ui_progress, 1);

  // Connect all signal handlers

  // folder A path
  connect(leFolderA, &AutoLineEdit::textChanged, this, &FileSyncDialog::validate);
  // folder B path
  connect(leFolderB, &AutoLineEdit::textChanged, this, &FileSyncDialog::validate);

  // function to dis/enable the OVERWR_ALWAYS option depending on sync direction
  connect(ui_syncDir, &AutoComboBox::currentDataChanged, [=](int dir) {
    int idx = copyMode->findData(SyncProcess::OVERWR_ALWAYS);
    int flg = (dir == SyncProcess::SYNC_A2B || dir == SyncProcess::SYNC_B2A) ? 33 : 0;
    if (!flg && idx == copyMode->currentIndex())
      copyMode->setCurrentIndex(copyMode->findData(SyncProcess::OVERWR_NEWER_IF_DIFF));
    copyMode->setItemData(idx, flg, Qt::UserRole - 1);
  });

  // function to set magnitude of file size spinbox, KB or MB
  connect(maxSize, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value) {
    const QSignalBlocker blocker(maxSize);
    int multi = maxSize->property("multi").isValid() ? maxSize->property("multi").toInt() : 0;
    if (value >= 10 * 1024 && multi != 1024 * 1024) {
      // KB -> MB
      multi = 1024 * 1024;
      maxSize->setValue(value / 1024);
      maxSize->setMaximum(100);
      maxSize->setSingleStep(1);
      maxSize->setSuffix(tr(" MB"));
    }
    else if ((value < 10 && multi != 1024) || !multi) {
      // MB -> KB
      if (multi)
        value *= 1024;
      multi = 1024;
      if (value == 9 * 1024)
        value += 1024 - 32;  // avoid large jump when stepping from 10MB to 10,208KB
      maxSize->setMaximum(100 * 1024);
      maxSize->setValue(value);
      maxSize->setSingleStep(32);
      maxSize->setSuffix(tr(" KB"));
    }
    maxSize->setProperty("multi", multi);
    m_syncOptions.maxFileSize = value * multi;
  });

  // we set these after connections are made because we want the signal processors to run
  ui_syncDir->setField(m_syncOptions.direction);
  maxSize->setValue(m_syncOptions.maxFileSize / 1024);

  // connect Reset action to all fields
  for (AutoLineEdit * a : ui_optionsPanel->findChildren<AutoLineEdit *>())
    connect(this, &FileSyncDialog::optionsWereReset, a, &AutoLineEdit::updateValue);
  for (AutoComboBox * a : ui_optionsPanel->findChildren<AutoComboBox *>())
    connect(this, &FileSyncDialog::optionsWereReset, a, &AutoComboBox::updateValue);
  for (AutoBitsetCheckBox * a : ui_optionsPanel->findChildren<AutoBitsetCheckBox *>())
    connect(this, &FileSyncDialog::optionsWereReset, a, &AutoBitsetCheckBox::updateValue);
  connect(this, &FileSyncDialog::optionsWereReset, [=]() { maxSize->setValue(m_syncOptions.maxFileSize / 1024); });

  // React to name changes for folder path labels
  connect(this, &FileSyncDialog::folderNameAChanged, [=](const QString &text) {
    lblFolderA->setText(text % ":");
    wdgFolderA->setProperty("fileDialogTitle", text);
    setSyncDirList();
  });
  connect(this, &FileSyncDialog::folderNameBChanged, [=](const QString &text) {
    lblFolderB->setText(text % ":");
    wdgFolderB->setProperty("fileDialogTitle", text);
    setSyncDirList();
  });

  // button actions
  connect(ui_btnStartStop, &QPushButton::clicked,           this, &FileSyncDialog::toggleSync);
  connect(ui_buttonBox,    &QDialogButtonBox::rejected,     this, &QDialog::reject);
  connect(ui_buttonBox,    &QDialogButtonBox::accepted,     this, &QDialog::accept);
  connect(ui_progress,     &ProgressWidget::detailsToggled, this, &FileSyncDialog::adjustSizeDelayed);
}

void FileSyncDialog::toggleSync(bool)
{
  if (m_running) {
    setCursor(Qt::WaitCursor);
    emit stopSync();
    return;
  }

  ui_progress->clearDetails();
  ui_progress->setInfo("");
  SyncProcess * syncProcess = new SyncProcess(m_syncOptions);

  // move sync process to separate thread, we only use signals/slots from here on...
  QThread * syncThread = new QThread(this);
  syncProcess->moveToThread(syncThread);

  // ...and quite a few of them!
  connect(this,        &FileSyncDialog::startSync,     syncProcess, &SyncProcess::run);
  connect(this,        &FileSyncDialog::stopSync,      syncProcess, &SyncProcess::stop);
  connect(syncProcess, &SyncProcess::started,          this,        &FileSyncDialog::onSyncStarted);
  connect(syncProcess, &SyncProcess::finished,         this,        &FileSyncDialog::onSyncFinished);
  connect(syncProcess, &SyncProcess::statusMessage,    this,        &FileSyncDialog::setStatusText);
  connect(syncProcess, &SyncProcess::statusUpdate,     this,        &FileSyncDialog::onStatusUpdate);
  connect(syncProcess, &SyncProcess::fileCountChanged, ui_progress, &ProgressWidget::setMaximum);
  connect(syncProcess, &SyncProcess::progressMessage,  ui_progress, &ProgressWidget::addMessage);
  connect(syncProcess, &SyncProcess::finished,         syncThread,  &QThread::quit);
  connect(syncThread,  &QThread::finished,             syncThread,  &QThread::deleteLater);
  connect(syncThread,  &QThread::destroyed,            syncProcess, &SyncProcess::deleteLater);

  // go
  syncThread->start(QThread::LowPriority);
  emit startSync();
}

void FileSyncDialog::onSyncStarted()
{
  m_running = true;
  updateRunningState();
}

void FileSyncDialog::onSyncFinished()
{
  m_running = false;
  updateRunningState();
  if (parentWidget())
    QApplication::alert(parentWidget());
}

void FileSyncDialog::updateRunningState()
{
  ui_btnStartStop->setChecked(m_running);
  ui_optionsPanel->setEnabled(!m_running);
  ui_btnClose->setEnabled(!m_running);
  ui_btnStartStop->setText(m_running ? tr("Abort") : tr("Start"));
  if (m_running) {
    ui_progress->setVisible(true);  // don't hide after run
  }
  else {
    setCursor(Qt::ArrowCursor);     // in case we set a wait cursor upon abort request
  }
}

void FileSyncDialog::onStatusUpdate(const SyncProcess::SyncStatus & stat)
{
  static const QString reportTemplate = tr("Total: <b>%1</b>; Created: <b>%2</b>; Updated: <b>%3</b>; Skipped: <b>%4</b>; Errors: <font color=%6><b>%5</b></font>;");
  ui_progress->setInfo((stat.index < stat.count ? tr("Current: <b>%1</b> of ").arg(stat.index) : "") % reportTemplate.arg(stat.count).arg(stat.created).arg(stat.updated).arg(stat.skipped).arg(stat.errored).arg(stat.errored ? "red" : "black"));
  ui_progress->setValue(stat.index);
}

void FileSyncDialog::setSyncOptions(const SyncProcess::SyncOptions & syncOptions)
{
  m_syncOptions = syncOptions;
  emit optionsWereReset();
}

void FileSyncDialog::resetOptions()
{
  m_syncOptions.reset();
  emit optionsWereReset();
}

void FileSyncDialog::toggleExtraOptions(bool show)
{
  ui_extraOptions->setVisible(show);
  if (!ui_progress->detailsVisible())
    adjustSizeDelayed();
  m_extraOptionsVisible = show;
  emit extraOptionsToggled(show);
}

void FileSyncDialog::setFolderNameA(const QString & name)
{
  if (m_folderNameA != name) {
    m_folderNameA = name;
    emit folderNameAChanged(name);
  }
}

void FileSyncDialog::setFolderNameB(const QString & name)
{
  if (m_folderNameB != name) {
    m_folderNameB = name;
    emit folderNameBChanged(name);
  }
}

void FileSyncDialog::setStatusText(const QString & text, int type)
{
  ui_statusLabel->setText(text);
  ui_statusLabel->setStyleSheet(type != QtInfoMsg && type >= QtWarningMsg ?  "QLabel { color: red; }:" : "");
}

bool FileSyncDialog::validate()
{
  QString msg;
  if (m_syncOptions.folderA.isEmpty() || !QFileInfo(m_syncOptions.folderA).exists())
    msg = tr("%1 not found.").arg(m_folderNameA);
  else if (m_syncOptions.folderB.isEmpty() || !QFileInfo(m_syncOptions.folderB).exists())
    msg = tr("%1 not found.").arg(m_folderNameB);
  else if (QFileInfo(m_syncOptions.folderA).canonicalFilePath() == QFileInfo(m_syncOptions.folderB).canonicalFilePath())
    msg = tr("Folders are the same.");

  setStatusText(msg, QtCriticalMsg);
  ui_btnStartStop->setEnabled(msg.isEmpty());
  return msg.isEmpty();
}

void FileSyncDialog::adjustSizeDelayed()
{
  QTimer::singleShot(0, [=]() { adjustSize(); });
}

void FileSyncDialog::setSyncDirList()
{
  const QString upArw = CPN_STR_SW_INDICATOR_UP % " ";
  const QString downArw = CPN_STR_SW_INDICATOR_DN % " ";

  ui_syncDir->clear();
  ui_syncDir->addItem(tr("%1%2 Both directions, to %3 first").arg(downArw, upArw, m_folderNameB), SyncProcess::SYNC_A2B_B2A);
  ui_syncDir->addItem(tr("%1%2 Both directions, to %3 first").arg(upArw, downArw, m_folderNameA), SyncProcess::SYNC_B2A_A2B);
  ui_syncDir->addItem(tr(" %1  Only from %2 to %3").arg(downArw, m_folderNameA, m_folderNameB), SyncProcess::SYNC_A2B);
  ui_syncDir->addItem(tr(" %1  Only from %2 to %3").arg(upArw, m_folderNameB, m_folderNameA), SyncProcess::SYNC_B2A);
  ui_syncDir->updateValue();
}
