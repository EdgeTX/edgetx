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

#include "simulatorwidget.h"
#include "ui_simulatorwidget.h"

#include "appdata.h"
#include "radiofaderwidget.h"
#include "radiokeywidget.h"
#include "radioknobwidget.h"
#include "radioswitchwidget.h"
#include "radiotrimwidget.h"
#include "radiouiaction.h"
#include "sdcard.h"
#include "simulateduiwidget.h"
#include "storage.h"
#include "virtualjoystickwidget.h"
#ifdef JOYSTICKS
#include "joystick.h"
#include "joystickdialog.h"
#endif

#include <AppDebugMessageHandler>
#include <QFile>
#include <QMessageBox>
#include <iostream>

using namespace Simulator;

SimulatorWidget::SimulatorWidget(QWidget * parent, SimulatorInterface * simulator, quint8 flags):
  QWidget(parent),
  ui(new Ui::SimulatorWidget),
  simulator(simulator),
  firmware(getCurrentFirmware()),
  radioSettings(GeneralSettings()),
  m_board(getCurrentBoard()),
  flags(flags)
{
  ui->setupUi(this);

  windowName = tr("Radio Simulator (%1)").arg(firmware->getName());
  setWindowTitle(windowName);

  switch(m_board) {
    case Board::BOARD_TARANIS_X9LITE:
    case Board::BOARD_TARANIS_X9LITES:
      radioUiWidget = new SimulatedUIWidgetX9LITE(simulator, this);
      break;
    case Board::BOARD_TARANIS_X7:
    case Board::BOARD_TARANIS_X7_ACCESS:
      radioUiWidget = new SimulatedUIWidgetX7(simulator, this);
      break;
    case Board::BOARD_TARANIS_X9D:
    case Board::BOARD_TARANIS_X9DP:
      radioUiWidget = new SimulatedUIWidgetX9(simulator, this);
      break;
    case Board::BOARD_TARANIS_X9DP_2019:
      radioUiWidget = new SimulatedUIWidgetX9D2019(simulator, this);
      break;
    case Board::BOARD_TARANIS_XLITE:
    case Board::BOARD_TARANIS_XLITES:
      radioUiWidget = new SimulatedUIWidgetXLITE(simulator, this);
      break;
    case Board::BOARD_TARANIS_X9E:
      radioUiWidget = new SimulatedUIWidgetX9E(simulator, this);
      break;
    case Board::BOARD_HORUS_X12S:
      radioUiWidget = new SimulatedUIWidgetX12(simulator, this);
      break;
    case Board::BOARD_X10:
    case Board::BOARD_X10_EXPRESS:
      radioUiWidget = new SimulatedUIWidgetX10(simulator, this);
      break;
    case Board::BOARD_BETAFPV_LR3PRO:
      radioUiWidget = new SimulatedUIWidgetLR3PRO(simulator, this);
      break;
    case Board::BOARD_IFLIGHT_COMMANDO8:
      radioUiWidget = new SimulatedUIWidgetCommando8(simulator, this);
      break;
    case Board::BOARD_JUMPER_T12:
      radioUiWidget = new SimulatedUIWidgetJumperT12(simulator, this);
      break;
    case Board::BOARD_JUMPER_TLITE:
    case Board::BOARD_JUMPER_TLITE_F4:
      radioUiWidget = new SimulatedUIWidgetJumperTLITE(simulator, this);
      break;
    case Board::BOARD_JUMPER_TPRO:
    case Board::BOARD_JUMPER_TPROV2:
      radioUiWidget = new SimulatedUIWidgetJumperTPRO(simulator, this);
      break;
    case Board::BOARD_JUMPER_TPROS:
      radioUiWidget = new SimulatedUIWidgetJumperTPROS(simulator, this);
      break;
    case Board::BOARD_JUMPER_BUMBLEBEE:
      radioUiWidget = new SimulatedUIWidgetJumperBumblebee(simulator, this);
      break;
    case Board::BOARD_JUMPER_T12MAX:
      radioUiWidget = new SimulatedUIWidgetJumperT12max(simulator, this);
      break;
    case Board::BOARD_JUMPER_T14:
      radioUiWidget = new SimulatedUIWidgetJumperT14(simulator, this);
      break;
    case Board::BOARD_JUMPER_T15:
      radioUiWidget = new SimulatedUIWidgetJumperT15(simulator, this);
      break;
    case Board::BOARD_JUMPER_T16:
      radioUiWidget = new SimulatedUIWidgetJumperT16(simulator, this);
      break;
    case Board::BOARD_JUMPER_T18:
      radioUiWidget = new SimulatedUIWidgetJumperT18(simulator, this);
      break;
    case Board::BOARD_JUMPER_T20:
    case Board::BOARD_JUMPER_T20V2:
      radioUiWidget = new SimulatedUIWidgetJumperT20(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_TX12:
    case Board::BOARD_RADIOMASTER_TX12_MK2:
      radioUiWidget = new SimulatedUIWidgetTX12(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_ZORRO:
      radioUiWidget = new SimulatedUIWidgetZorro(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_BOXER:
      radioUiWidget = new SimulatedUIWidgetBoxer(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_MT12:
      radioUiWidget = new SimulatedUIWidgetMT12(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_GX12:
      radioUiWidget = new SimulatedUIWidgetGeneric(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_T8:
      radioUiWidget = new SimulatedUIWidgetT8(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_TX16S:
      radioUiWidget = new SimulatedUIWidgetTX16S(simulator, this);
      break;
    case Board::BOARD_RADIOMASTER_POCKET:
      radioUiWidget = new SimulatedUIWidgetPocket(simulator, this);
      break;
    case Board::BOARD_FATFISH_F16:
      radioUiWidget = new SimulatedUIWidgetFatfishF16(simulator, this);
      break;
    case Board::BOARD_FLYSKY_NV14:
      radioUiWidget = new SimulatedUIWidgetNV14(simulator, this);
      break;
    case Board::BOARD_FLYSKY_EL18:
      radioUiWidget = new SimulatedUIWidgetEL18(simulator, this);
      break;
    case Board::BOARD_FLYSKY_PL18:
      radioUiWidget = new SimulatedUIWidgetPL18(simulator, this);
      break;
    case Board::BOARD_HELLORADIOSKY_V16:
      radioUiWidget = new SimulatedUIWidgetV16(simulator, this);
      break;
    default:
      radioUiWidget = new SimulatedUIWidget9X(simulator, this);
      break;
  }

  foreach (keymapHelp_t item, radioUiWidget->getKeymapHelp())
    keymapHelp.append(item);

  ui->radioUiWidget->layout()->removeItem(ui->radioUiTempSpacer);
  delete ui->radioUiTempSpacer;
  ui->radioUiWidget->layout()->addWidget(radioUiWidget);
  radioUiWidget->setFocusPolicy(Qt::WheelFocus);
  radioUiWidget->setFocus();

  connect(radioUiWidget, &SimulatedUIWidget::controlValueChange, this, &SimulatorWidget::onRadioWidgetValueChange);
  connect(radioUiWidget, &SimulatedUIWidget::customStyleRequest, this, &SimulatorWidget::setUiAreaStyle);

  vJoyLeft = new VirtualJoystickWidget(this, 'L');
  ui->leftStickLayout->addWidget(vJoyLeft);

  vJoyRight = new VirtualJoystickWidget(this, 'R', (m_board == Board::BOARD_TARANIS_XLITE || m_board == Board::BOARD_TARANIS_XLITES ? false : true));  // TODO: maybe remove trims for both joysticks and add a cross in the middle?
  ui->rightStickLayout->addWidget(vJoyRight);

  connect(vJoyLeft, &VirtualJoystickWidget::valueChange, this, &SimulatorWidget::onRadioWidgetValueChange);
  connect(vJoyRight, &VirtualJoystickWidget::valueChange, this, &SimulatorWidget::onRadioWidgetValueChange);
  connect(this, &SimulatorWidget::stickModeChange, vJoyLeft, &VirtualJoystickWidget::loadDefaultsForMode);
  connect(this, &SimulatorWidget::stickModeChange, vJoyRight, &VirtualJoystickWidget::loadDefaultsForMode);
  connect(simulator, &SimulatorInterface::trimValueChange, vJoyLeft, &VirtualJoystickWidget::setTrimValue);
  connect(simulator, &SimulatorInterface::trimValueChange, vJoyRight, &VirtualJoystickWidget::setTrimValue);
  connect(simulator, &SimulatorInterface::trimRangeChange, vJoyLeft, &VirtualJoystickWidget::setTrimRange);
  connect(simulator, &SimulatorInterface::trimRangeChange, vJoyRight, &VirtualJoystickWidget::setTrimRange);

  connect(this, &SimulatorWidget::simulatorInit, simulator, &SimulatorInterface::init);
  connect(this, &SimulatorWidget::simulatorStart, simulator, &SimulatorInterface::start);
  connect(this, &SimulatorWidget::simulatorStop, simulator, &SimulatorInterface::stop);
  connect(this, &SimulatorWidget::inputValueChange, simulator, &SimulatorInterface::setInputValue);
  connect(this, &SimulatorWidget::simulatorSdPathChange, simulator, &SimulatorInterface::setSdPath);
  connect(this, &SimulatorWidget::simulatorVolumeGainChange, simulator, &SimulatorInterface::setVolumeGain);

  connect(simulator, &SimulatorInterface::started, this, &SimulatorWidget::onSimulatorStarted);
  connect(simulator, &SimulatorInterface::heartbeat, this, &SimulatorWidget::onSimulatorHeartbeat);
  connect(simulator, &SimulatorInterface::runtimeError, this, &SimulatorWidget::onSimulatorError);
  connect(simulator, &SimulatorInterface::phaseChanged, this, &SimulatorWidget::onPhaseChanged);

  m_timer.setInterval(SIMULATOR_INTERFACE_HEARTBEAT_PERIOD * 6);
  connect(&m_timer, &QTimer::timeout, this, &SimulatorWidget::onTimerEvent);

  setupJoysticks();

  // defaults
  setRadioProfileId(g.sessionId());
  setSdPath(g.profile[radioProfileId].sdPath());
}

SimulatorWidget::~SimulatorWidget()
{
  shutdown();

  delete radioUiWidget;
  delete vJoyLeft;
  delete vJoyRight;
#ifdef JOYSTICKS
  delete joystick;
#endif
  firmware = nullptr;
  delete ui;
}

/*
 * Public slots/setters
 */

void SimulatorWidget::setSdPath(const QString & sdPath)
{
  setPaths(sdPath, radioDataPath);
}

void SimulatorWidget::setDataPath(const QString & dataPath)
{
  setPaths(sdCardPath, dataPath);
}

void SimulatorWidget::setPaths(const QString & sdPath, const QString & dataPath)
{
  sdCardPath = sdPath;
  radioDataPath = dataPath;
  emit simulatorSdPathChange(sdPath, dataPath);
}

void SimulatorWidget::setRadioSettings(const GeneralSettings settings)
{
  radioSettings = settings;
}

/*
 * This function can accept no parameters, a file name (QString is a QBA), or a data array. It will attempt to load radio settings data from one of
 *   several sources into a RadioData object, parse the data, and then pass it on as appropriate to the SimulatorInterface in start().
 * If given no/blank <dataSource>, and setDataPath() was already called, then it will check that directory for "Horus-style" data files.
 * If given a file name, set the <fromFile> parameter to 'true'. This will attempt to load radio settings from said file
 *   and later start the simulator interface in start() using the same data.
 * If <dataSource> is a byte array of data, attempts to load radio settings from there and will also start the simulator interface
 *   with the same data when start() is called.
 * If you already have a valid RadioData structure, call setRadioData() instead.
 */
bool SimulatorWidget::setStartupData(const QByteArray & dataSource, bool fromFile)
{
  RadioData simuData;
  quint16 ret = 1;
  QString error;
  QString fileName(dataSource);

  // If <dataSource> is blank but we have a data path, use that for individual radio/model files.
  if (dataSource.isEmpty() && !radioDataPath.isEmpty()) {
    // If directory structure already exists, try to load data from there.
    // FIXME : need Storage class to return formal error code, not just a boolean, because it would be better
    //   to avoid hard-coding paths like "RADIO" here. E.g. did it fail due to no data at all, or corrupt data, or...?
    if (QDir(QString(radioDataPath).append("/RADIO")).exists()) {
      SdcardFormat sdcard(radioDataPath);
      if (!(ret = sdcard.load(simuData))) {
        error = sdcard.error();
      }
    }
  }
  // Supposedly we're being given a file name to use, try that out.
  else if (fromFile && !fileName.isEmpty()) {
    Storage store = Storage(fileName);
    ret = store.load(simuData);
    if (!ret && QFile(fileName).exists()) {
      error = store.error();
    }
    else {
      if (fileName.endsWith(".etx", Qt::CaseInsensitive)) {
        // no radios can work with .etx files directly, so we load contents into either
        //   a temporary folder (Horus) or local data array (other radios) which we'll save back to .etx upon exit
        if ((ret = setRadioData(&simuData))) {
          startupFromFile = false;
          return true;
        }
      }
      else {
        // the binary file will be read/written directly by the fw interface, save the file name for simulator->start()
        startupData = dataSource;
        ret = 1;
      }
    }
  }
  // Assume a byte array of radio data was passed, load it.
  else if (!dataSource.isEmpty()) {
    //ret = firmware->getEEpromInterface()->load(simuData, (uint8_t *)dataSource.constData(), Boards::getEEpromSize(m_board));
    //startupData = dataSource;  // save the data for start()
    error = tr("Unable to handle byte array.");
    ret = 0;
  }
  // we're :-(
  else {
    ret = 0;
    error = tr("Could not determine startup data source.");
  }

  if (!ret) {
    if (error.isEmpty())
      error = tr("Could not load data, possibly wrong format.");
    QMessageBox::critical(this, tr("Data Load Error"), error);
    return false;
  }

  radioSettings = simuData.generalSettings;
  startupFromFile = fromFile;

  return true;
}

bool SimulatorWidget::setRadioData(RadioData * radioData)
{
  bool ret = true;

  saveTempRadioData = (flags & SIMULATOR_FLAGS_STANDALONE);

  // All radios use SD card data path from 2.6.0 on
  bool hasSdCard = Boards::getCapability(m_board, Board::HasSDCard);
  if (hasSdCard)
    ret = useTempDataPath(true);

  if (ret) {
    if (!hasSdCard) {
      startupData.fill(0, Boards::getEEpromSize(m_board));
      //if (firmware->getEEpromInterface()->save(
      //        (uint8_t *)startupData.data(), *radioData, 0,
      //        firmware->getCapability(SimulatorVariant)) <= 0) {
        ret = false;
      //}
    } else {
      ret = saveRadioData(radioData, radioDataPath);
    }
  }

  if (ret)
    radioSettings = radioData->generalSettings;

  return ret;
}

bool SimulatorWidget::setOptions(SimulatorOptions & options, bool withSave)
{
  bool ret = false;

  setSdPath(options.sdPath);

  if (options.startupDataType == SimulatorOptions::START_WITH_FOLDER && !options.dataFolder.isEmpty()) {
    setDataPath(options.dataFolder);
    ret = setStartupData();
  }
  else if (options.startupDataType == SimulatorOptions::START_WITH_SDPATH && !options.sdPath.isEmpty()) {
    setDataPath(options.sdPath);
    ret = setStartupData();
  }
  else if (options.startupDataType == SimulatorOptions::START_WITH_FILE && !options.dataFile.isEmpty()) {
    ret = setStartupData(options.dataFile.toLocal8Bit(), true);
  }
  else {
    QString error = tr("Invalid startup data provided. Plese specify a proper file/path.");
    QMessageBox::critical(this, tr("Simulator Startup Error"), error);
  }

  if (ret && withSave)
    g.profile[radioProfileId].simulatorOptions(options);

  return ret;
}

bool SimulatorWidget::saveRadioData(RadioData * radioData, const QString & path, QString * error)
{
  QString dir = path;
  if (dir.isEmpty())
    dir = radioDataPath;

  if (radioData && !dir.isEmpty()) {
    SdcardFormat sdcard(dir);
    bool ret = sdcard.write(*radioData);
    if (!ret && error)
      *error = sdcard.error();
    return ret;
  }

  return false;
}

bool SimulatorWidget::useTempDataPath(bool deleteOnClose)
{
  if (deleteTempRadioData)
    deleteTempData();

  QTemporaryDir tmpDir(QDir::tempPath() + "/etx-XXXXXX");
  if (tmpDir.isValid()) {
    setDataPath(tmpDir.path());
    tmpDir.setAutoRemove(false);
    deleteTempRadioData = deleteOnClose;
    qDebug() << "Created temporary settings directory" << radioDataPath << "with delteOnClose:" << deleteOnClose;
    return true;
  }
  else {
    qCritical() << "ERROR : Failed to create temporary settings directory" << radioDataPath;
    return false;
  }
}

// This will save radio data from temporary folder structure back into an .etx file
bool SimulatorWidget::saveTempData()
{
  bool ret = false;
  QString error;
  QString file = g.profile[radioProfileId].simulatorOptions().dataFile;

  if (!file.isEmpty()) {
    RadioData radioData;

    if (radioDataPath.isEmpty()) {
      if (!startupData.isEmpty()) {
        if (!QFile(file).exists()) {
          QFile fh(file);
          if (!fh.open(QIODevice::WriteOnly))
            error = tr("Error saving data: could open file for writing: '%1'").arg(file);
          else
            fh.close();
        }

        qDebug() << "ERROR: WE DO NOT WANT TO BE HERE";
//        if (!firmware->getEEpromInterface()->load(radioData, (uint8_t *)startupData.constData(), Boards::getEEpromSize(m_board))) {
//          error = tr("Error saving data: could not get data from simulator interface.");
//        }
//        else {
//          radioData.fixModelFilenames();
//          ret = true;
//        }
      }
    }
    else {
      SdcardFormat sdcard(radioDataPath);
      if (!(ret = sdcard.load(radioData))) {
        error = sdcard.error();
      }
    }
    if (ret) {
      Storage store(file);
      if (!(ret = store.write(radioData)))
        error = store.error();
      else
        qInfo() << "Saved radio data to file" << file;
    }
  }

  if (!ret) {
    if (error.isEmpty())
      error = tr("An unexpected error occurred while attempting to save radio data to file '%1'.").arg(file);
    QMessageBox::critical(this, tr("Data Save Error"), error);
  }

  return ret;
}

void SimulatorWidget::deleteTempData()
{
  if (!radioDataPath.isEmpty()) {
    QDir tpath(radioDataPath);
    qDebug() << "Deleting temporary settings directory" << tpath.absolutePath();
    tpath.removeRecursively();
    tpath.rmdir(radioDataPath);  // for some reason this is necessary to remove the base folder
  }
}

void SimulatorWidget::saveState()
{
  SimulatorOptions opts = g.profile[radioProfileId].simulatorOptions();
  saveRadioWidgetsState(opts.controlsState);
  g.profile[radioProfileId].simulatorOptions(opts);
}

void SimulatorWidget::setUiAreaStyle(const QString & style)
{
  setStyleSheet(style);
}

void SimulatorWidget::captureScreenshot(bool)
{
  if (radioUiWidget)
    radioUiWidget->captureScreenshot();
}

/*
 * Startup
 */

void SimulatorWidget::start()
{
  emit simulatorInit();  // init simulator default I/O values

  setupRadioWidgets();
  restoreRadioWidgetsState();

  bool tests = !(flags & SIMULATOR_FLAGS_NOTX);
  if (!startupData.isEmpty()) {
    if (startupFromFile) {
      emit simulatorStart(startupData.constData(), tests);
    }
    else {
      simulator->setRadioData(startupData);
      emit simulatorStart((const char *)0, tests);
    }
  }
  else {
    emit simulatorStart((const char *)0, tests);
  }
}

void SimulatorWidget::stop()
{
  emit simulatorStop();
  QElapsedTimer tmout;
  tmout.start();
  // block until simulator stops or times out
  while (simulator->isRunning()) {
    if (tmout.hasExpired(2000)) {
      onSimulatorError("Timeout while trying to stop simulation!");
      break;
    }
    QApplication::processEvents();
  }
  onSimulatorStopped();
}

void SimulatorWidget::onSimulatorStarted()
{
  m_heartbeatTimer.start();
  m_timer.start();
}

void SimulatorWidget::onSimulatorStopped()
{
  m_timer.stop();
  m_heartbeatTimer.invalidate();

  if (simulator && !simulator->isRunning() && saveTempRadioData) {
    startupData.fill(0, Boards::getEEpromSize(m_board));
    simulator->readRadioData(startupData);
  }
}

void SimulatorWidget::restart()
{
  stop();
  saveState();
  setStartupData(startupData, startupFromFile);
  start();
}

void SimulatorWidget::shutdown()
{
  stop();
  saveState();
  if (saveTempRadioData)
    saveTempData();
  if (deleteTempRadioData)
    deleteTempData();
}

/*
 * Setup
 */

void SimulatorWidget::setRadioProfileId(int value)
{
  Q_ASSERT(value >= 0);
  radioProfileId = value;
  emit simulatorVolumeGainChange(g.profile[radioProfileId].volumeGain());
}

void SimulatorWidget::setupRadioWidgets()
{
  QString wname;
  int i, midpos;
  const int ttlSticks = Boards::getCapability(m_board, Board::Sticks);
  const int ttlSwitches = Boards::getCapability(m_board, Board::Switches);
  const int ttlInputs = Boards::getCapability(m_board, Board::Inputs);
  const int extraTrims = Boards::getCapability(m_board, Board::NumTrims) - ttlSticks;

  // First clear out any existing widgets.
  foreach (RadioWidget * rw, m_radioWidgets) {
    switch(rw->getType()) {
      case RadioWidget::RADIO_WIDGET_SWITCH :
      case RadioWidget::RADIO_WIDGET_KNOB   :
        ui->radioWidgetsHTLayout->removeWidget(rw);
        break;
      case RadioWidget::RADIO_WIDGET_FADER :
      case RadioWidget::RADIO_WIDGET_TRIM  :
        ui->VCGridLayout->removeWidget(rw);
        break;
      default :
        break;
    }
    disconnect(rw, 0, this, 0);
    disconnect(this, 0, rw, 0);
    rw->deleteLater();
  }
  m_radioWidgets.clear();

  // Now set up new widgets.

  // switches
  Board::SwitchType swcfg;
  for (i = 0; i < ttlSwitches; ++i) {
    if (!radioSettings.isSwitchAvailable(i) || Boards::isSwitchFunc(i))
      continue;

    swcfg = Board::SwitchType(radioSettings.switchConfig[i].type);
    wname = RawSource(RawSourceType::SOURCE_TYPE_SWITCH, i + 1).toString(nullptr, &radioSettings, Board::BOARD_UNKNOWN, false);
    RadioSwitchWidget * sw = new RadioSwitchWidget(swcfg, wname, -1, ui->radioWidgetsHT);
    sw->setIndex(i);
    ui->radioWidgetsHTLayout->addWidget(sw);

    m_radioWidgets.append(sw);
  }

  midpos = (int)floorf(m_radioWidgets.size() / 2.0f);

  // pots in middle of switches
  for (i = 0; i < ttlInputs; ++i) {
    if (!(radioSettings.isInputAvailable(i) && radioSettings.isInputPot(i)))
      continue;

    wname = RawSource(RawSourceType::SOURCE_TYPE_INPUT, i + 1).toString(nullptr, &radioSettings, Board::BOARD_UNKNOWN, false);
    RadioKnobWidget * pot = new RadioKnobWidget(radioSettings.inputConfig[i].flexType, wname, 0, ui->radioWidgetsHT);
    pot->setIndex(i);
    ui->radioWidgetsHTLayout->insertWidget(midpos++, pot);
    m_radioWidgets.append(pot);
  }

  // faders between sticks
  int fc = extraTrims / 2;  // leave space for any extra trims

  for (i = 0; i < ttlInputs; ++i) {
    if (!(radioSettings.isInputAvailable(i) && radioSettings.isInputSlider(i)))
      continue;

    wname = RawSource(RawSourceType::SOURCE_TYPE_INPUT, i + 1).toString(nullptr, &radioSettings, Board::BOARD_UNKNOWN, false);
    RadioFaderWidget * sl = new RadioFaderWidget(wname, 0, ui->radioWidgetsVC);
    sl->setIndex(i);
    ui->VCGridLayout->addWidget(sl, 0, fc++, 1, 1);
    m_radioWidgets.append(sl);
  }

  // extra trims around faders
  int tc = 0;
  int tridx = ttlSticks;
  for (i = 0; i < extraTrims; i += 1, tridx += 1) {
    wname = RawSource(RawSourceType::SOURCE_TYPE_TRIM, tridx + 1).toString(nullptr, &radioSettings, Board::BOARD_UNKNOWN, false);
    wname = wname.left(1) % wname.right(1);
    RadioTrimWidget * tw = new RadioTrimWidget(Qt::Vertical, ui->radioWidgetsVC);
    tw->setIndices(tridx, tridx * 2, tridx * 2 + 1);
    tw->setLabelText(wname);
    if (i == extraTrims / 2)
      tc += (fc + extraTrims / 2 - 1);
    ui->VCGridLayout->addWidget(tw, 0, tc, 1, 1);
    tc += (i >= extraTrims / 2) ? -1 : 1;

    connect(simulator, &SimulatorInterface::trimValueChange, tw, &RadioTrimWidget::setTrimValue);
    connect(simulator, &SimulatorInterface::trimRangeChange, tw, &RadioTrimWidget::setTrimRangeQual);
    m_radioWidgets.append(tw);
  }

  // connect all the widgets
  foreach (RadioWidget * rw, m_radioWidgets) {
    connect(rw, &RadioWidget::valueChange, this, &SimulatorWidget::onRadioWidgetValueChange);
    connect(this, &SimulatorWidget::widgetValueChange, rw, &RadioWidget::setValueQual);
    connect(this, &SimulatorWidget::widgetValueAdjust, rw, &RadioWidget::chgValueQual);
    connect(this, &SimulatorWidget::widgetStateChange, rw, &RadioWidget::setStateData);
  }

}

void SimulatorWidget::setupJoysticks()
{
#ifdef JOYSTICKS
  bool joysticksEnabled = false;

  if (g.jsSupport()) {
    if (!joystick)
      joystick = new Joystick(this, SDL_JOYSTICK_DEFAULT_EVENT_TIMEOUT, false, SDL_JOYSTICK_DEFAULT_AUTOREPEAT_DELAY);
    else
      joystick->close();

    int stick = joystick->findCurrent(g.currentProfile().jsName());

    if (joystick && joystick->open(stick)) {
      int numAxes = std::min(joystick->numAxes, MAX_JS_AXES);
      for (int j=0; j<numAxes; j++) {
        joystick->sensitivities[j] = 0;
        joystick->deadzones[j] = 0;
      }
      connect(joystick, &Joystick::axisValueChanged, this, &SimulatorWidget::onjoystickAxisValueChanged);
      connect(joystick, &Joystick::buttonValueChanged, this, &SimulatorWidget::onjoystickButtonValueChanged);
      if (vJoyLeft) {
        connect(this, &SimulatorWidget::stickValueChange, vJoyLeft, &VirtualJoystickWidget::setStickAxisValue);
        connect(this, &SimulatorWidget::widgetValueAdjust, vJoyLeft->horizontalTrim(), &RadioWidget::chgValueQual);
        connect(this, &SimulatorWidget::widgetValueAdjust, vJoyLeft->verticalTrim(), &RadioWidget::chgValueQual);
      }
      if (vJoyRight) {
        connect(this, &SimulatorWidget::stickValueChange, vJoyRight, &VirtualJoystickWidget::setStickAxisValue);
        connect(this, &SimulatorWidget::widgetValueAdjust, vJoyRight->horizontalTrim(), &RadioWidget::chgValueQual);
        connect(this, &SimulatorWidget::widgetValueAdjust, vJoyRight->verticalTrim(), &RadioWidget::chgValueQual);
      }
      joysticksEnabled = true;
    }
    else {
      if (!g.disableJoystickWarning())
        QMessageBox::critical(this, CPN_STR_TTL_WARNING, tr("Cannot open joystick, joystick disabled"));
    }
  }
  else if (joystick) {
    joystick->close();
    disconnect(joystick, 0, this, 0);
    if (vJoyLeft)
      disconnect(this, 0, vJoyLeft, 0);
    if (vJoyRight)
      disconnect(this, 0, vJoyRight, 0);
    joystick->deleteLater();
    joystick = nullptr;
  }
  if (vJoyRight)
    vJoyRight->setStickConstraint((VirtualJoystickWidget::HOLD_X | VirtualJoystickWidget::HOLD_Y), joysticksEnabled);
  if (vJoyLeft)
    vJoyLeft->setStickConstraint((VirtualJoystickWidget::HOLD_X | VirtualJoystickWidget::HOLD_Y), joysticksEnabled);
#endif
}

void SimulatorWidget::restoreRadioWidgetsState()
{
  // All RadioWidgets
  RadioWidget::RadioWidgetState state;
  QList<QByteArray> states = g.profile[radioProfileId].simulatorOptions().controlsState;
  foreach (QByteArray ba, states) {
    QDataStream stream(ba);
    stream >> state;
    emit widgetStateChange(state);
  }

  // Set throttle stick down and locked, side depends on mode
  emit stickModeChange(radioSettings.stickMode);

  // TODO : custom voltages
  qint16 volts = radioSettings.vBatWarn + 20; // 1V above min
  emit inputValueChange(SimulatorInterface::INPUT_SRC_TXVIN, 0, volts);
}

void SimulatorWidget::saveRadioWidgetsState(QList<QByteArray> & state)
{
  if (m_radioWidgets.size()) {
    if (g.simuSW()) {
      state.clear();
      foreach (RadioWidget * rw, m_radioWidgets)
        state.append(rw->getStateData());
    }
  }
}


/*
 * Event handlers/private slots
 */

void SimulatorWidget::mousePressEvent(QMouseEvent *event)
{
  if (radioUiWidget)
    radioUiWidget->mousePressEvent(event);
}

void SimulatorWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if (radioUiWidget)
    radioUiWidget->mouseReleaseEvent(event);
}

void SimulatorWidget::wheelEvent(QWheelEvent *event)
{
  if (radioUiWidget)
    radioUiWidget->wheelEvent(event);
}

void SimulatorWidget::onTimerEvent()
{
  if (m_heartbeatTimer.isValid() && m_heartbeatTimer.hasExpired(m_timer.interval())) {
    onSimulatorError("Heartbeat timeout!");
    onSimulatorStopped();
  }
}

void SimulatorWidget::onSimulatorHeartbeat(qint32 loops, qint64 timestamp)
{
  Q_UNUSED(loops)
  Q_UNUSED(timestamp)
  m_heartbeatTimer.start();

#if 0
  static qint64 lastTs = 0;
  if (!(loops % 1000)) {
    qDebug() << "loops:" << loops << "ts:" << timestamp << "ts-delta:" << timestamp - lastTs << "This:" << QThread::currentThread() << "Simu:" << simulator->thread();
    lastTs = timestamp;
  }
#endif
}

void SimulatorWidget::onSimulatorError(const QString & error)
{
  QMessageBox::critical(this, windowName, tr("Radio firmware error: %1").arg(error.isEmpty() ? "Unknown reason" : error));
}

void SimulatorWidget::onPhaseChanged(qint32 phase, const QString & name)
{
  setWindowTitle(windowName + QString(" - %1 %2 (#%3)").arg(Boards::getCapability(m_board, Board::Air) ? tr("Flight Mode") : tr("Drive Mode")).arg(name).arg(phase));
}

void SimulatorWidget::onRadioWidgetValueChange(const RadioWidget::RadioWidgetType type, int index, int value)
{
  //qDebug() << type << index << value;
  if (!simulator || index < 0)
    return;

  SimulatorInterface::InputSourceType inpType = SimulatorInterface::INPUT_SRC_NONE;
  GeneralSettings::SwitchConfig *cfg = nullptr;

  switch (type) {
    case RadioWidget::RADIO_WIDGET_SWITCH :
      cfg = &radioSettings.switchConfig[index];
      if (cfg->inputIdx != SWITCH_INPUTINDEX_NONE) {
        inpType = SimulatorInterface::INPUT_SRC_STICK;
        index = cfg->inputIdx;
        value = value * 1024;
      }
      else
        inpType = SimulatorInterface::INPUT_SRC_SWITCH;
      break;

    case RadioWidget::RADIO_WIDGET_KNOB :
      inpType = SimulatorInterface::INPUT_SRC_KNOB;
      break;

    case RadioWidget::RADIO_WIDGET_FADER :
      inpType = SimulatorInterface::INPUT_SRC_SLIDER;
      break;

    case RadioWidget::RADIO_WIDGET_TRIM :
      switch (value) {
        case RadioWidget::RADIO_TRIM_BTN_ON :
          inpType = SimulatorInterface::INPUT_SRC_TRIM_SW;
          value = 1;
          break;
        case RadioWidget::RADIO_TRIM_BTN_OFF :
          inpType = SimulatorInterface::INPUT_SRC_TRIM_SW;
          value = 0;
          break;
        default :
          inpType = SimulatorInterface::INPUT_SRC_TRIM;
          break;
      }
      break;

    case RadioWidget::RADIO_WIDGET_STICK :
      inpType = SimulatorInterface::INPUT_SRC_STICK;
      break;

    case RadioWidget::RADIO_WIDGET_KEY :
      inpType = SimulatorInterface::INPUT_SRC_KEY;
      break;

    default :
      return;
  }

  emit inputValueChange(inpType, index, value);
}

void SimulatorWidget::onjoystickAxisValueChanged(int axis, int value)
{
#ifdef JOYSTICKS
  static const int ttlSticks = 4;
  const int ttlKnobs = Boards::getCapability(m_board, Board::Pots);
  const int ttlFaders = Boards::getCapability(m_board, Board::Sliders);
  static const int valueRange = 1024;

  if (!joystick || axis >= MAX_JS_AXES)
    return;

  int dlta;
  int stick = g.joystick[axis].stick_axe();

  if (stick < 0 || stick >= ttlSticks + ttlKnobs + ttlFaders)
    return;

  int stickval = valueRange * (value - g.joystick[axis].stick_med());

  if (value > g.joystick[axis].stick_med()) {
    if ((dlta = g.joystick[axis].stick_max() - g.joystick[axis].stick_med()))
      stickval /= dlta;
  }
  else if ((dlta = g.joystick[axis].stick_med() - g.joystick[axis].stick_min())) {
    stickval /= dlta;
  }

  if (g.joystick[axis].stick_inv())
    stickval *= -1;

  if (stick < ttlSticks) {
    emit stickValueChange(stick, stickval);
  }
  else {
    GeneralSettings radioSettings = GeneralSettings();
    if (radioSettings.isInputAvailable(stick)) {
      if (radioSettings.isInputPot(stick)) {
        if (radioSettings.inputConfig[stick].flexType == Board::FlexType::FLEX_MULTIPOS)
          stickval += 1024;
        emit widgetValueChange(RadioWidget::RADIO_WIDGET_KNOB, stick, stickval);
      } else if (radioSettings.isInputSlider(stick)) {
        emit widgetValueChange(RadioWidget::RADIO_WIDGET_FADER, stick, stickval);
      }
    }
  }

#endif
}

void SimulatorWidget::onjoystickButtonValueChanged(int button, bool state)
{
#ifdef JOYSTICKS

  if (!joystick || button >= MAX_JS_BUTTONS)
    return;

  int ttlSwitches = Boards::getCapability(m_board, Board::Switches);

  int btn = g.jsButton[button].button_idx();

  int swtch = btn & JS_BUTTON_SWITCH_MASK;

  if (swtch < ttlSwitches) {
    if (btn & JS_BUTTON_3POS_DN) {
      // 3POS Down
      if (state || (switchDirection[swtch] == 0) || (switchDirection[swtch] == (btn & JS_BUTTON_TYPE_MASK))) {
        emit widgetValueChange(RadioWidget::RADIO_WIDGET_SWITCH, swtch, state ? 1 : 0);
        switchDirection[swtch] = (btn & JS_BUTTON_TYPE_MASK);
      }
    } else if (btn & JS_BUTTON_3POS_UP) {
      // 3POS Up
      if (state || (switchDirection[swtch] == 0) || (switchDirection[swtch] == (btn & JS_BUTTON_TYPE_MASK))) {
        emit widgetValueChange(RadioWidget::RADIO_WIDGET_SWITCH, swtch, state ? -1 : 0);
        switchDirection[swtch] = (btn & JS_BUTTON_TYPE_MASK);
      }
    } else if (btn & JS_BUTTON_TOGGLE) {
      // Toggle or momentary
      emit widgetValueChange(RadioWidget::RADIO_WIDGET_SWITCH, swtch, state ? 1 : - 1);
    }
  } else {
    // Trim
    swtch -= ttlSwitches;
    int offset = (btn & JS_BUTTON_3POS_DN) ? -1 : 1;
    emit widgetValueAdjust(RadioWidget::RADIO_WIDGET_TRIM, swtch, offset, state);
  }
#endif
}
