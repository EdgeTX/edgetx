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

#include "simulatormainwindow.h"
#include "ui_simulatormainwindow.h"

#include "appdata.h"
#include "debugoutput.h"
#include "radiooutputswidget.h"
#include "simulatorwidget.h"
#include "simulatorinterface.h"
#include "telemetrysimu.h"
#include "trainersimu.h"
#ifdef USE_SDL
#include "joystickdialog.h"
#endif
#include "serialportsdialog.h"

#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QMessageBox>

extern AppData g;  // ensure what "g" means

const quint16 SimulatorMainWindow::m_savedUiStateVersion = 2;

SimulatorMainWindow::SimulatorMainWindow(QWidget *parent, const QString & simulatorId, quint8 flags, Qt::WindowFlags wflags) :
  QMainWindow(parent, wflags),
  ui(new Ui::SimulatorMainWindow),
  m_simulatorWidget(NULL),
  m_consoleWidget(NULL),
  m_outputsWidget(NULL),
  m_simulatorDockWidget(NULL),
  m_consoleDockWidget(NULL),
  m_telemetryDockWidget(NULL),
  m_trainerDockWidget(NULL),
  m_outputsDockWidget(NULL),
  m_simulatorId(simulatorId),
  m_exitStatusCode(0),
  m_radioProfileId(g.sessionId()),
  m_radioSizeConstraint(Qt::Horizontal | Qt::Vertical),
  m_firstShow(true),
  m_showRadioDocked(true),
  m_showMenubar(true),
  m_batMin(0),
  m_batMax(0),
  m_batWarn(0),
  m_batVoltage(0)
{
  if (m_simulatorId.isEmpty()) {
    m_simulatorId = SimulatorLoader::findSimulatorByName(getCurrentFirmware()->getSimulatorId());
  }
  m_simulator = SimulatorLoader::loadSimulator(m_simulatorId);
  if (!m_simulator) {
    m_exitStatusMsg = tr("ERROR: Failed to create simulator interface, possibly missing or bad library.");
    m_exitStatusCode = -1;
    return;
  }

  if (g.fwTraceLog() && !g.appLogsDir().isEmpty() && QDir().mkpath(g.appLogsDir())) {
    // send firmware TRACE events to log file
    QString fn = g.appLogsDir() % "/FirmwareDebug_" % QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss") % ".log";
    m_simuLogFile.setFileName(fn);
    if (m_simuLogFile.open(QIODevice::WriteOnly | QIODevice::Text))
      m_simulator->addTracebackDevice(&m_simuLogFile);
  }

  m_simulator->moveToThread(&simuThread);
  simuThread.start();

  hostSerialConnector = new HostSerialConnector(this, m_simulator);

  ui->setupUi(this);

  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  m_simulatorWidget = new SimulatorWidget(this, m_simulator, flags);
  setWindowTitle(m_simulatorWidget->windowTitle());

  toggleRadioDocked(true);
  createDockWidgets();

  ui->actionReloadLua->setIcon(SimulatorIcon("reload_script"));
  ui->actionReloadRadioData->setIcon(SimulatorIcon("restart"));
  ui->actionJoystickSettings->setIcon(SimulatorIcon("joystick_settings"));
  ui->actionScreenshot->setIcon(SimulatorIcon("camera"));
  ui->actionShowKeymap->setIcon(SimulatorIcon("info"));
  ui->actionToggleMenuBar->setIcon(ui->toolBar->toggleViewAction()->icon());
  ui->actionFixedRadioWidth->setIcon(ui->toolBar->toggleViewAction()->icon());
  ui->actionFixedRadioHeight->setIcon(ui->toolBar->toggleViewAction()->icon());
  ui->actionDockRadio->setIcon(ui->toolBar->toggleViewAction()->icon());

  ui->toolBar->toggleViewAction()->setShortcut(tr("Alt+T"));
  ui->toolBar->setIconSize(SimulatorIcon::toolbarIconSize(g.iconSize()));
  ui->toolBar->insertSeparator(ui->actionReloadLua);

  // add these to this window directly to maintain shorcuts when menubar is hidden
  addAction(ui->toolBar->toggleViewAction());
  addAction(ui->actionToggleMenuBar);

  ui->menuView->insertSeparator(ui->actionToggleMenuBar);
  ui->menuView->insertAction(ui->actionToggleMenuBar, ui->toolBar->toggleViewAction());

  // Hide some actions based on simulator capabilities.
  if(!m_simulator->getCapability(SimulatorInterface::CAP_LUA))
    ui->actionReloadLua->setDisabled(true);
  if(!m_simulator->getCapability(SimulatorInterface::CAP_TELEM_FRSKY_SPORT))
    m_telemetryDockWidget->toggleViewAction()->setDisabled(true);
#ifndef USE_SDL
  ui->actionJoystickSettings->setDisabled(true);
#endif

  restoreUiState();

  setStyleSheet(SimulatorStyle::styleSheet());

  connect(ui->actionShowKeymap, &QAction::triggered, this, &SimulatorMainWindow::showHelp);
  connect(ui->actionAbout, &QAction::triggered, this, &SimulatorMainWindow::showAbout);
  connect(ui->actionJoystickSettings, &QAction::triggered, this, &SimulatorMainWindow::openJoystickDialog);
  connect(ui->actionSerialPorts, &QAction::triggered, this, &SimulatorMainWindow::openSerialPortsDialog);
  connect(ui->actionToggleMenuBar, &QAction::toggled, this, &SimulatorMainWindow::showMenuBar);
  connect(ui->actionFixedRadioWidth, &QAction::toggled, this, &SimulatorMainWindow::showRadioFixedWidth);
  connect(ui->actionFixedRadioHeight, &QAction::toggled, this, &SimulatorMainWindow::showRadioFixedHeight);
  connect(ui->actionDockRadio, &QAction::toggled, this, &SimulatorMainWindow::showRadioDocked);
  connect(ui->actionReloadRadioData, &QAction::triggered, this, &SimulatorMainWindow::simulatorRestart);

  connect(ui->actionReloadLua, &QAction::triggered, m_simulator, &SimulatorInterface::setLuaStateReloadPermanentScripts);
  connect(ui->actionSetTxBatteryVoltage, &QAction::triggered, this, &SimulatorMainWindow::openTxBatteryVoltageDialog);

  if (m_outputsWidget) {
    connect(this, &SimulatorMainWindow::simulatorStart, m_outputsWidget, &RadioOutputsWidget::start);
    connect(this, &SimulatorMainWindow::simulatorRestart, m_outputsWidget, &RadioOutputsWidget::restart);
  }

  if (m_simulatorWidget) {
    connect(this, &SimulatorMainWindow::simulatorStart, m_simulatorWidget, &SimulatorWidget::start);
    connect(this, &SimulatorMainWindow::simulatorRestart, m_simulatorWidget, &SimulatorWidget::restart);
    connect(ui->actionScreenshot, &QAction::triggered, m_simulatorWidget, &SimulatorWidget::captureScreenshot);
    connect(m_simulatorWidget, &SimulatorWidget::windowTitleChanged, this, &SimulatorMainWindow::setWindowTitle);
    connect(m_simulatorWidget, &SimulatorWidget::settingsBatteryChanged, this, &SimulatorMainWindow::onSettingsBatteryChanged);
  }

  connect(m_simulator, &SimulatorInterface::auxSerialSendData, hostSerialConnector, &HostSerialConnector::sendSerialData);
  connect(m_simulator, &SimulatorInterface::auxSerialSetEncoding, hostSerialConnector, &HostSerialConnector::setSerialEncoding);
  connect(m_simulator, &SimulatorInterface::auxSerialSetBaudrate, hostSerialConnector, &HostSerialConnector::setSerialBaudRate);
  connect(m_simulator, &SimulatorInterface::auxSerialStart, hostSerialConnector, &HostSerialConnector::serialStart);
  connect(m_simulator, &SimulatorInterface::auxSerialStop, hostSerialConnector, &HostSerialConnector::serialStop);
  connect(m_simulator, &SimulatorInterface::txBatteryVoltageChanged, this, &SimulatorMainWindow::onTxBatteryVoltageChanged);
}

SimulatorMainWindow::~SimulatorMainWindow()
{
  delete m_telemetryDockWidget;
  delete m_trainerDockWidget;
  delete m_outputsDockWidget;
  delete m_simulatorDockWidget;
  delete m_simulatorWidget;
  delete m_consoleDockWidget;
  delete ui;

  if (m_simulator) {
    simuThread.quit();
    simuThread.wait();
    if (m_simuLogFile.isOpen()) {
      m_simulator->removeTracebackDevice(&m_simuLogFile);
      m_simuLogFile.close();
    }
    delete m_simulator;
  }

  delete hostSerialConnector;
  SimulatorLoader::unloadSimulator(m_simulatorId);
}

void SimulatorMainWindow::closeEvent(QCloseEvent *)
{
  saveUiState();
}

void SimulatorMainWindow::show()
{
  QMainWindow::show();
  if (m_firstShow) {
    m_firstShow = false;
    #ifdef Q_OS_LINUX
      // for whatever reason, w/out this workaround any floating docks may appear and get "stuck" behind other windows, eg. Terminal or Companion.
      restoreUiState();
    #endif
    start();
  }
}

void SimulatorMainWindow::changeEvent(QEvent *e)
{
  QMainWindow::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      ui->retranslateUi(this);
      break;
    default:
      break;
  }
}

QMenu * SimulatorMainWindow::createPopupMenu()
{
  QMenu * menu = QMainWindow::createPopupMenu();
  menu->clear();
  menu->addActions(ui->menuView->actions());
  return menu;
}

void SimulatorMainWindow::saveUiState()
{
  QByteArray state;
  QDataStream stream(&state, QIODevice::WriteOnly);
  stream << m_savedUiStateVersion << saveState(m_savedUiStateVersion)
         << m_showMenubar << m_showRadioDocked << m_radioSizeConstraint;

  SimulatorOptions opts = g.profile[m_radioProfileId].simulatorOptions();
  opts.windowState = state;
  opts.windowGeometry = saveGeometry();
  g.profile[m_radioProfileId].simulatorOptions(opts);
}

void SimulatorMainWindow::restoreUiState()
{
  quint16 ver = 0;
  QByteArray windowState;
  QByteArray state = g.profile[m_radioProfileId].simulatorOptions().windowState;
  QDataStream stream(state);

  stream >> ver;
  if (ver && ver <= m_savedUiStateVersion) {
    stream >> windowState >> m_showMenubar >> m_showRadioDocked;
    if (ver >= 2)
      stream >> m_radioSizeConstraint;
  }

  toggleRadioDocked(m_showRadioDocked);
  setRadioSizePolicy(m_radioSizeConstraint);
  toggleMenuBar(m_showMenubar);
  restoreGeometry(g.profile[m_radioProfileId].simulatorOptions().windowGeometry);
  restoreState(windowState, m_savedUiStateVersion);
}

int SimulatorMainWindow::getExitStatus(QString * msg)
{
  if (msg)
    *msg = m_exitStatusMsg;
  return m_exitStatusCode;
}

bool SimulatorMainWindow::setRadioData(RadioData * radioData)
{
  return m_simulatorWidget->setRadioData(radioData);
}

bool SimulatorMainWindow::useTempDataPath(bool deleteOnClose)
{
  return m_simulatorWidget->useTempDataPath(deleteOnClose);
}

bool SimulatorMainWindow::setOptions(SimulatorOptions & options, bool withSave)
{
  return m_simulatorWidget->setOptions(options, withSave);
}

void SimulatorMainWindow::start()
{
  emit simulatorStart();
}

void SimulatorMainWindow::createDockWidgets()
{
  if (!m_outputsDockWidget) {
    SimulatorIcon icon("radio_outputs");
    m_outputsDockWidget = new QDockWidget(tr("Radio Outputs"), this);
    m_outputsWidget = new RadioOutputsWidget(m_simulator, getCurrentFirmware(), this);
    m_outputsDockWidget->setWidget(m_outputsWidget);
    m_outputsDockWidget->setObjectName("OUTPUTS");
    addTool(m_outputsDockWidget, Qt::RightDockWidgetArea, icon, QKeySequence(tr("F2")));
  }

  if (!m_telemetryDockWidget) {
    SimulatorIcon icon("telemetry");
    m_telemetryDockWidget = new QDockWidget(tr("Telemetry Simulator"), this);
    auto * telem = new TelemetrySimulator(this, m_simulator);
    m_telemetryDockWidget->setWidget(telem);
    m_telemetryDockWidget->setObjectName("TELEMETRY_SIMULATOR");
    addTool(m_telemetryDockWidget, Qt::LeftDockWidgetArea, icon, QKeySequence(tr("F4")));
  }

  if (!m_trainerDockWidget) {
    SimulatorIcon icon("trainer");
    m_trainerDockWidget = new QDockWidget(tr("Trainer Simulator"), this);
    auto * trainer = new TrainerSimulator(this, m_simulator);
    m_trainerDockWidget->setWidget(trainer);
    m_trainerDockWidget->setObjectName("TRAINER_SIMULATOR");
    addTool(m_trainerDockWidget, Qt::TopDockWidgetArea, icon, QKeySequence(tr("F5")));
  }

  if (!m_consoleDockWidget) {
    SimulatorIcon icon("console");
    m_consoleDockWidget = new QDockWidget(tr("Debug Output"), this);
    m_consoleWidget = new DebugOutput(this, m_simulator);
    m_consoleDockWidget->setWidget(m_consoleWidget);
    m_consoleDockWidget->setObjectName("CONSOLE");
    addTool(m_consoleDockWidget, Qt::RightDockWidgetArea, icon, QKeySequence(tr("F6")));
  }
}

void SimulatorMainWindow::addTool(QDockWidget * widget, Qt::DockWidgetArea area, QIcon icon, QKeySequence shortcut)
{
  QAction* tempAction = widget->toggleViewAction();
  tempAction->setIcon(icon);
  tempAction->setShortcut(shortcut);
  ui->menuView->insertAction(ui->actionToggleMenuBar, tempAction);
  ui->toolBar->insertAction(ui->actionReloadLua, tempAction);
  widget->setWindowIcon(icon);
  widget->widget()->setWindowIcon(icon);
  addDockWidget(area, widget);
  widget->hide();
  widget->setFloating(true);

  // Upon subsequent launches of application, any previously un-shown floating widgets get
  //   positioned at screen location (0,0 - frameGeometry.topLeft) which is awkward at best.
  // This ensures newly shown floating widgets don't get stuck in top left corner.
  connect(widget, &QDockWidget::visibilityChanged, [this, widget](bool visible) {
    if (visible && widget->isFloating() && widget->geometry().topLeft() == QPoint(0,0)) {
      // position top left corner in middle of this parent window.
      QPoint newPos(pos() + (geometry().bottomRight() - geometry().topLeft()) / 2);
      widget->move(newPos);
    }
  });
}

void SimulatorMainWindow::showMenuBar(bool show)
{
  if (m_showMenubar != show)
    toggleMenuBar(show);
}

void SimulatorMainWindow::toggleMenuBar(bool show)
{
  ui->menubar->setVisible(show);
  m_showMenubar = show;
  if (ui->actionToggleMenuBar->isChecked() != show)
    ui->actionToggleMenuBar->setChecked(show);
}

void SimulatorMainWindow::showRadioFixedSize(Qt::Orientation orientation, bool fixed)
{
  int fix = m_radioSizeConstraint;
  if (fixed)
    fix |= orientation;
  else
    fix &= ~(orientation);

  if (m_radioSizeConstraint != fix)
    setRadioSizePolicy(fix);
}

void SimulatorMainWindow::showRadioFixedWidth(bool fixed)
{
  showRadioFixedSize(Qt::Horizontal, fixed);
}

void SimulatorMainWindow::showRadioFixedHeight(bool fixed)
{
  showRadioFixedSize(Qt::Vertical, fixed);
}

void SimulatorMainWindow::setRadioSizePolicy(int fixType)
{
  QSizePolicy sp;
  sp.setHorizontalPolicy((fixType & Qt::Horizontal) ? QSizePolicy::Maximum : QSizePolicy::Preferred);
  sp.setVerticalPolicy((fixType & Qt::Vertical) ? QSizePolicy::Maximum : QSizePolicy::Preferred);
  m_simulatorWidget->setSizePolicy(sp);

  m_radioSizeConstraint = fixType;

  if (ui->actionFixedRadioWidth->isChecked() != bool(fixType & Qt::Horizontal))
    ui->actionFixedRadioWidth->setChecked((fixType & Qt::Horizontal));
  if (ui->actionFixedRadioHeight->isChecked() != bool(fixType & Qt::Vertical))
    ui->actionFixedRadioHeight->setChecked((fixType & Qt::Vertical));
}

void SimulatorMainWindow::showRadioDocked(bool dock)
{
  if (m_showRadioDocked != dock)
    toggleRadioDocked(dock);
}

void SimulatorMainWindow::toggleRadioDocked(bool dock)
{
  if (!m_simulatorWidget)
    return;

  if (dock) {
    if (m_simulatorDockWidget) {
      m_simulatorDockWidget->setWidget(0);
      m_simulatorDockWidget->deleteLater();
      m_simulatorDockWidget = NULL;
    }

    QWidget * w = takeCentralWidget();
    if (w && w != m_simulatorWidget)
      w->deleteLater();
    setCentralWidget(m_simulatorWidget);
    setRadioSizePolicy(m_radioSizeConstraint);
    ui->actionFixedRadioWidth->setEnabled(true);
    ui->actionFixedRadioHeight->setEnabled(true);
    m_simulatorWidget->show();
  }
  else {

    if (m_simulatorDockWidget) {
      m_simulatorDockWidget->deleteLater();
      m_simulatorDockWidget = NULL;
    }

    takeCentralWidget();
    QLabel * dummy = new QLabel("");
    dummy->setFixedSize(0, 0);
    dummy->setEnabled(false);
    setCentralWidget(dummy);

    m_simulatorDockWidget = new QDockWidget(m_simulatorWidget->windowTitle(), this);
    m_simulatorDockWidget->setObjectName("RADIO_SIMULATOR");
    m_simulatorDockWidget->setWidget(m_simulatorWidget);
    m_simulatorDockWidget->setFeatures(QDockWidget::DockWidgetFloatable);
    m_simulatorDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_simulatorDockWidget);
    m_simulatorDockWidget->setFloating(true);
    m_simulatorWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    ui->actionFixedRadioWidth->setDisabled(true);
    ui->actionFixedRadioHeight->setDisabled(true);
    restoreDockWidget(m_simulatorDockWidget);
    if (!m_simulatorDockWidget->isVisible())
      m_simulatorDockWidget->show();
    if (m_simulatorDockWidget->geometry().topLeft() == QPoint(0,0) && this->isVisible()) {
      // default position top left corner in middle of this parent window.
      QPoint newPos(pos() + (geometry().bottomRight() - geometry().topLeft()) / 2);
      m_simulatorDockWidget->move(newPos);
    }

    connect(m_simulatorWidget, &SimulatorWidget::windowTitleChanged, m_simulatorDockWidget, &QDockWidget::setWindowTitle);
    connect(m_simulatorDockWidget, &QDockWidget::topLevelChanged, [this](bool top) {
      showRadioDocked(!top);
    });
  }

  m_showRadioDocked = dock;

  if (ui->actionDockRadio->isChecked() != dock)
    ui->actionDockRadio->setChecked(dock);

}

void SimulatorMainWindow::openJoystickDialog(bool)
{
#ifdef USE_SDL
  joystickDialog * jd = new joystickDialog(this);
  if (jd->exec() == QDialog::Accepted && m_simulatorWidget)
    m_simulatorWidget->setupJoysticks();
  jd->deleteLater();
#endif
}

void SimulatorMainWindow::openSerialPortsDialog(bool)
{
  SerialPortsDialog * dialog = new SerialPortsDialog(this, m_simulator, hostSerialConnector);
  if (dialog->exec() == QDialog::Accepted && m_simulator) {
    hostSerialConnector->connectSerialPort(0, dialog->aux1);
    hostSerialConnector->connectSerialPort(1, dialog->aux2);
  }
  dialog->deleteLater();
}

void SimulatorMainWindow::showHelp(bool show)
{
  QString helpText = ""
      "<style>"
      "  td { text-align: center; vertical-align: middle; font-size: large; padding: 0 1em; white-space: nowrap; }"
      "  th { background-color: palette(alternate-base); }"
      "  img { vertical-align: text-top; }"
      "</style>";
  helpText += tr("<b>Simulator Controls:</b>");
  helpText += "<table cellspacing=4 cellpadding=0>";
  helpText += tr("<tr><th>Key/Mouse</th><th>Action</th></tr>", "note: must match html layout of each table row (keyTemplate).");

  QString keyTemplate = tr("<tr><td><kbd>%1</kbd></td><td>%2</td></tr>", "note: must match html layout of help text table header.");
  keymapHelp_t pair;
  // Add our own help text (if any)
  foreach (pair, m_keymapHelp)
    helpText += keyTemplate.arg(pair.first, pair.second);
  // Add any radio-specific help text from simulator widget
  foreach (pair, m_simulatorWidget->getKeymapHelp())
    helpText += keyTemplate.arg(pair.first, pair.second);

  helpText += "</table>";

  QMessageBox * msgBox = new QMessageBox(this);
  msgBox->setObjectName("SimulatorHelpText");
  msgBox->setAttribute(Qt::WA_DeleteOnClose);
  msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);
  msgBox->setStandardButtons( QMessageBox::Ok );
  msgBox->setWindowTitle(tr("Simulator Help"));
  msgBox->setTextFormat(Qt::RichText);
  msgBox->setText(helpText);
  msgBox->setModal(false);
  msgBox->show();
}

void SimulatorMainWindow::showAbout(bool show)
{
  QString aboutStr = "<center><img src=\":/images/simulator-title.png\"></center><br/>";
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
  msgBox.setWindowTitle(tr("About EdgeTX Simulator"));
  msgBox.setText(aboutStr);
  msgBox.exec();
}

void SimulatorMainWindow::onSettingsBatteryChanged(const int batMin, const int batMax, const unsigned int batWarn)
{
  m_batMin = batMin;
  m_batMax = batMax;
  m_batWarn = batWarn;
  m_batVoltage = batWarn + 5; // abitary +0.5V
}

void SimulatorMainWindow::openTxBatteryVoltageDialog()
{
  QDialog *dlg = new QDialog(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint);

  QLabel *lbl = new QLabel(tr("Set voltage"));
  QDoubleSpinBox *sb = new QDoubleSpinBox();
  sb->setDecimals(1);
  // vBatWarn is voltage in 100mV, vBatMin is in 100mV but with -9V offset, vBatMax has a -12V offset
  sb->setMinimum((float)((m_batMin + 90) / 10.0f));
  sb->setMaximum((float)((m_batMax + 120) / 10.0f));
  sb->setSingleStep(0.1);
  sb->setSuffix(tr("V"));
  sb->setValue((float)m_batVoltage / 10.0f);

  auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btnBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
  connect(btnBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

  auto * lo = new QGridLayout(dlg);
  lo->addWidget(lbl, 0, 0);
  lo->addWidget(sb, 0, 1);
  lo->addWidget(btnBox, 1, 0, 1, 2);

  dlg->setWindowTitle(tr("Battery"));
  dlg->deleteLater();

  if(dlg->exec()) {
    unsigned int volts = (unsigned int)((float)sb->value() * 10.0f);
    m_batVoltage = volts;
    emit txBatteryVoltageChanged(volts);
  }
}

void SimulatorMainWindow::onTxBatteryVoltageChanged(const unsigned int voltage)
{
  m_batVoltage = voltage;
}
