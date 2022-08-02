/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "setup.h"
#include "ui_setup.h"
#include "ui_setup_timer.h"
#include "ui_setup_module.h"
#include "ui_setup_function_switches.h"
#include "appdata.h"
#include "modelprinter.h"
#include "multiprotocols.h"
#include "checklistdialog.h"
#include "helpers.h"
#include "moduledata.h"

#include <QDir>

constexpr char FIM_TIMERSWITCH[] {"Timer Switch"};
constexpr char FIM_THRSOURCE[]   {"Throttle Source"};
constexpr char FIM_TRAINERMODE[] {"Trainer Mode"};

TimerPanel::TimerPanel(QWidget * parent, ModelData & model, TimerData & timer, GeneralSettings & generalSettings, Firmware * firmware,
                       QWidget * prevFocus, FilteredItemModelFactory * panelFilteredModels, CompoundItemModelFactory * panelItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  timer(timer),
  ui(new Ui::Timer),
  modelsUpdateCnt(0)
{
  ui->setupUi(this);
  connectItemModelEvents(panelFilteredModels->getItemModel(FIM_TIMERSWITCH));

  lock = true;

  // Name
  int length = firmware->getCapability(TimersName);
  if (length == 0)
    ui->name->hide();
  else {
    ui->name->setField(timer.name, length, this);
    connect(ui->name, SIGNAL(currentDataChanged()), this, SLOT(onNameChanged()));
  }

  ui->value->setField(timer.val, this);
  ui->value->setMaximumTime(firmware->getMaxTimerStart());
  connect(ui->value, &AutoTimeEdit::currentDataChanged, [=](const int val) {
    update();
  });

  ui->mode->setModel(panelItemModels->getItemModel(AIM_TIMER_MODE));
  ui->mode->setField(timer.mode, this);
  connect(ui->mode, SIGNAL(currentDataChanged(int)), this, SLOT(onModeChanged(int)));

  ui->swtch->setModel(panelFilteredModels->getItemModel(FIM_TIMERSWITCH));
  ui->swtch->setField(timer.swtch, this);

  ui->countdownBeep->setModel(panelItemModels->getItemModel(AIM_TIMER_COUNTDOWNBEEP));
  ui->countdownBeep->setField(timer.countdownBeep, this);
  connect(ui->countdownBeep, SIGNAL(currentDataChanged(int)), this, SLOT(onCountdownBeepChanged(int)));

  ui->minuteBeep->setField(timer.minuteBeep, this);

  if (firmware->getCapability(PermTimers)) {
    ui->persistent->setModel(panelItemModels->getItemModel(AIM_TIMER_PERSISTENT));
    ui->persistent->setField(timer.persistent, this);
  }
  else {
    ui->persistent->hide();
    ui->persistentValue->hide();
  }

  ui->countdownStart->setModel(panelItemModels->getItemModel(AIM_TIMER_COUNTDOWNSTART));
  ui->countdownStart->setField(timer.countdownStart, this);


  ui->showElapsed->setModel(panelItemModels->getItemModel(AIM_TIMER_SHOWELAPSED));
  ui->showElapsed->setField(timer.showElapsed, this);

  disableMouseScrolling();
  QWidget::setTabOrder(prevFocus, ui->name);
  QWidget::setTabOrder(ui->name, ui->value);
  QWidget::setTabOrder(ui->value, ui->mode);
  QWidget::setTabOrder(ui->mode, ui->countdownBeep);
  QWidget::setTabOrder(ui->countdownBeep, ui->countdownStart);
  QWidget::setTabOrder(ui->countdownStart, ui->minuteBeep);
  QWidget::setTabOrder(ui->minuteBeep, ui->persistent);
  QWidget::setTabOrder(ui->persistent, ui->showElapsed);

  update();
  lock = false;
}

TimerPanel::~TimerPanel()
{
  delete ui;
}

void TimerPanel::update()
{
  lock = true;

  ui->name->updateValue();
  ui->mode->updateValue();
  ui->swtch->updateValue();
  ui->value->updateValue();
  ui->countdownBeep->updateValue();
  ui->minuteBeep->updateValue();
  ui->countdownStart->updateValue();
  ui->showElapsed->updateValue();

  if (timer.mode != TimerData::TIMERMODE_OFF)
    ui->swtch->setEnabled(true);
  else
    ui->swtch->setEnabled(false);

  if (timer.countdownBeep == TimerData::COUNTDOWNBEEP_SILENT) {
    ui->countdownStartLabel->setEnabled(false);
    ui->countdownStart->setEnabled(false);
  }
  else {
    ui->countdownStartLabel->setEnabled(true);
    ui->countdownStart->setEnabled(true);
  }

  if (firmware->getCapability(PermTimers)) {
    ui->persistent->updateValue();
    ui->persistentValue->setText(timer.pvalueToString());
  }

  if (timer.val) {
    ui->showElapsed->setEnabled(true);
  }
  else {
    ui->showElapsed->setEnabled(false);
  }

  lock = false;
}

QWidget * TimerPanel::getLastFocus()
{
  return ui->persistent;
}

void TimerPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &TimerPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &TimerPanel::onItemModelUpdateComplete);
}

void TimerPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void TimerPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}

void TimerPanel::onNameChanged()
{
  emit nameChanged();
}

void TimerPanel::onCountdownBeepChanged(int index)
{
  timer.countdownBeepChanged();
  update();
}

void TimerPanel::onModeChanged(int index)
{
  timer.modeChanged();
  update();
}

/******************************************************************************/

#define FAILSAFE_CHANNEL_HOLD    2000
#define FAILSAFE_CHANNEL_NOPULSE 2001

#define MASK_PROTOCOL       (1<<0)
#define MASK_CHANNELS_COUNT (1<<1)
#define MASK_RX_NUMBER      (1<<2)
#define MASK_CHANNELS_RANGE (1<<3)
#define MASK_PPM_FIELDS     (1<<4)
#define MASK_FAILSAFES      (1<<5)
#define MASK_OPEN_DRAIN     (1<<6)
#define MASK_MULTIMODULE    (1<<7)
#define MASK_ANTENNA        (1<<8)
#define MASK_MULTIOPTION    (1<<9)
#define MASK_R9M            (1<<10)
#define MASK_SBUSPPM_FIELDS (1<<11)
#define MASK_SUBTYPES       (1<<12)
#define MASK_ACCESS         (1<<13)
#define MASK_RX_FREQ        (1<<14)
#define MASK_RF_POWER       (1<<15)
#define MASK_RF_RACING_MODE (1<<16)
#define MASK_GHOST          (1<<17)
#define MASK_BAUDRATE       (1<<18)

quint8 ModulePanel::failsafesValueDisplayType = ModulePanel::FAILSAFE_DISPLAY_PERCENT;

ModulePanel::ModulePanel(QWidget * parent, ModelData & model, ModuleData & module, GeneralSettings & generalSettings, Firmware * firmware, int moduleIdx,
                         FilteredItemModelFactory * panelFilteredItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  module(module),
  moduleIdx(moduleIdx),
  ui(new Ui::Module)
{
  lock = true;

  ui->setupUi(this);

  ui->label_module->setText(ModuleData::indexToString(moduleIdx, firmware));
  if (moduleIdx < 0) {
    ui->formLayout_col1->setSpacing(0);
    if (!IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      ui->label_trainerMode->hide();
      ui->trainerMode->hide();
    }
    else {
      if (panelFilteredItemModels)
        ui->trainerMode->setModel(panelFilteredItemModels->getItemModel(FIM_TRAINERMODE));
      ui->trainerMode->setField(model.trainerMode);
      connect(ui->trainerMode, &AutoComboBox::currentDataChanged, this, [=] () {
              update();
              emit modified();
      });
    }
  }
  else {
    ui->label_trainerMode->hide();
    ui->trainerMode->hide();
  }

  if (panelFilteredItemModels && moduleIdx >= 0) {
    int id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(ModuleData::protocolItemModel(generalSettings), moduleIdx + 1/*flag cannot be 0*/), QString("Module Protocol %1").arg(moduleIdx));
    ui->protocol->setModel(panelFilteredItemModels->getItemModel(id));

    if (ui->protocol->findData(module.protocol) < 0) {
      QString msg = tr("Warning: The internal module protocol <b>%1</b> is incompatible with the hardware internal module <b>%2</b> and has been set to <b>OFF</b>!");
      msg = msg.arg(module.protocolToString(module.protocol)).arg(ModuleData::typeToString(generalSettings.internalModule));

      QMessageBox *msgBox = new QMessageBox(this);
      msgBox->setIcon( QMessageBox::Warning );
      msgBox->setText(msg);
      msgBox->addButton( "Ok", QMessageBox::AcceptRole );
      msgBox->setWindowFlag(Qt::WindowStaysOnTopHint);
      msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
      msgBox->setModal(false);
      msgBox->show();

      module.clear();
    }

    ui->protocol->setField(module.protocol, this);
  }

  for (int i = 0; i <= MODULE_SUBTYPE_MULTI_LAST; i++) {
    if (i == MODULE_SUBTYPE_MULTI_SCANNER || i == MODULE_SUBTYPE_MULTI_CONFIG)
      continue;
    ui->multiProtocol->addItem(Multiprotocols::protocolToString(i), i);
  }
  for (int i = MODULE_SUBTYPE_MULTI_LAST + 1; i <= 124; i++) {
    ui->multiProtocol->addItem(QString::number(i + 3), i);
  }

  ui->btnGrpValueType->setId(ui->optPercent, FAILSAFE_DISPLAY_PERCENT);
  ui->btnGrpValueType->setId(ui->optUs, FAILSAFE_DISPLAY_USEC);
  ui->btnGrpValueType->button(failsafesValueDisplayType)->setChecked(true);

  ui->registrationId->setText(model.registrationId);

  setupFailsafes();

  disableMouseScrolling();

  update();

  connect(ui->protocol, &AutoComboBox::currentDataChanged, this, &ModulePanel::onProtocolChanged);
  connect(ui->multiSubType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ModulePanel::onSubTypeChanged);
  connect(ui->multiProtocol, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ModulePanel::onMultiProtocolChanged);
  connect(this, &ModulePanel::channelsRangeChanged, this, &ModulePanel::setupFailsafes);
  connect(ui->btnGrpValueType, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ModulePanel::onFailsafesDisplayValueTypeChanged);
  connect(ui->rxFreq, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ModulePanel::onRfFreqChanged);
  connect(ui->clearRx1, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));
  connect(ui->clearRx2, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));
  connect(ui->clearRx3, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));

  lock = false;

}

ModulePanel::~ModulePanel()
{
  delete ui;
}

void ModulePanel::setupFailsafes()
{
  ChannelFailsafeWidgetsGroup grp;
  const int start = module.channelsStart;
  const int end = start + module.channelsCount;
  const bool hasFailsafe = module.hasFailsafes(firmware);

  lock = true;

  QMutableMapIterator<int, ChannelFailsafeWidgetsGroup> i(failsafeGroupsMap);
  while (i.hasNext()) {
    i.next();
    grp = i.value();
    ui->failsafesLayout->removeWidget(grp.label);
    ui->failsafesLayout->removeWidget(grp.combo);
    ui->failsafesLayout->removeWidget(grp.sbPercent);
    ui->failsafesLayout->removeWidget(grp.sbUsec);
    if (i.key() < start || i.key() >= end || !hasFailsafe) {
      grp.label->deleteLater();
      grp.combo->deleteLater();
      grp.sbPercent->deleteLater();
      grp.sbUsec->deleteLater();
      i.remove();
    }
  }

  if (!hasFailsafe) {
    lock = false;
    return;
  }

  int row = 0;
  int col = 0;
  int channelMax = model->getChannelsMax();
  int channelMaxUs = 512 * channelMax / 100 * 2;

  for (int i = start; i < end; ++i) {
    if (failsafeGroupsMap.contains(i)) {
      grp = failsafeGroupsMap.value(i);
    }
    else {
      QLabel * label = new QLabel(this);
      label->setProperty("index", i);
      label->setText(QString::number(i + 1));

      QComboBox * combo = new QComboBox(this);
      combo->setProperty("index", i);
      combo->addItem(tr("Value"), 0);
      combo->addItem(tr("Hold"), FAILSAFE_CHANNEL_HOLD);
      combo->addItem(tr("No Pulse"), FAILSAFE_CHANNEL_NOPULSE);

      QDoubleSpinBox * sbDbl = new QDoubleSpinBox(this);
      sbDbl->setProperty("index", i);
      sbDbl->setMinimumSize(QSize(20, 0));
      sbDbl->setRange(-channelMax, channelMax);
      sbDbl->setSingleStep(0.1);
      sbDbl->setDecimals(1);

      QSpinBox * sbInt = new QSpinBox(this);
      sbInt->setProperty("index", i);
      sbInt->setMinimumSize(QSize(20, 0));
      sbInt->setRange(-channelMaxUs, channelMaxUs);
      sbInt->setSingleStep(1);

      grp = ChannelFailsafeWidgetsGroup();
      grp.combo = combo;
      grp.sbPercent = sbDbl;
      grp.sbUsec = sbInt;
      grp.label = label;
      failsafeGroupsMap.insert(i, grp);

      connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(onFailsafeComboIndexChanged(int)));
      connect(sbInt, SIGNAL(valueChanged(int)), this, SLOT(onFailsafeUsecChanged(int)));
      connect(sbDbl, SIGNAL(valueChanged(double)), this, SLOT(onFailsafePercentChanged(double)));
    }

    ui->failsafesLayout->addWidget(grp.label, row, col, Qt::AlignHCenter);
    ui->failsafesLayout->addWidget(grp.combo, row + 1, col, Qt::AlignHCenter);
    ui->failsafesLayout->addWidget(grp.sbPercent, row + 2, col, Qt::AlignHCenter);
    ui->failsafesLayout->addWidget(grp.sbUsec, row + 3, col, Qt::AlignHCenter);
    grp.sbPercent->setVisible(failsafesValueDisplayType == FAILSAFE_DISPLAY_PERCENT);
    grp.sbUsec->setVisible(failsafesValueDisplayType == FAILSAFE_DISPLAY_USEC);

    updateFailsafe(i);

    if (++col > 7) {
      row += 4;
      col = 0;
    }
  }

  lock = false;
}

void ModulePanel::update()
{
  lock = true;

  const auto protocol = (PulsesProtocol)module.protocol;
  const auto board = firmware->getBoard();
  const auto & pdef = multiProtocols.getProtocol(module.multi.rfProtocol);
  unsigned int mask = 0;
  unsigned int max_rx_num = 63;

  if (moduleIdx >= 0) {
    mask |= MASK_PROTOCOL;
    switch (protocol) {
      case PULSES_PXX_R9M:
        mask |= MASK_R9M | MASK_RF_POWER | MASK_SUBTYPES;
      case PULSES_ACCESS_R9M:
      case PULSES_ACCESS_R9M_LITE:
      case PULSES_ACCESS_R9M_LITE_PRO:
      case PULSES_ACCESS_ISRM:
      case PULSES_ACCST_ISRM_D16:
      case PULSES_XJT_LITE_X16:
      case PULSES_XJT_LITE_D8:
      case PULSES_XJT_LITE_LR12:
      case PULSES_PXX_XJT_X16:
      case PULSES_PXX_XJT_D8:
      case PULSES_PXX_XJT_LR12:
      case PULSES_PXX_DJT:
        mask |= MASK_CHANNELS_RANGE | MASK_CHANNELS_COUNT;
        // ACCST Rx ID
        if (protocol==PULSES_PXX_XJT_X16 || protocol==PULSES_PXX_XJT_LR12 ||
            protocol==PULSES_PXX_R9M || protocol==PULSES_ACCST_ISRM_D16 ||
            protocol==PULSES_XJT_LITE_X16 || protocol==PULSES_XJT_LITE_LR12)
          mask |= MASK_RX_NUMBER;
        // ACCESS
        else if (protocol==PULSES_ACCESS_ISRM || protocol==PULSES_ACCESS_R9M ||
                 protocol==PULSES_ACCESS_R9M_LITE || protocol==PULSES_ACCESS_R9M_LITE_PRO)
          mask |= MASK_RX_NUMBER | MASK_ACCESS;
        if (moduleIdx == 0 && HAS_EXTERNAL_ANTENNA(board) && generalSettings.antennaMode == GeneralSettings::ANTENNA_MODE_PER_MODEL)
          mask |= MASK_ANTENNA;
        if (protocol == PULSES_ACCESS_ISRM && module.channelsCount == 8)
          mask |= MASK_RF_RACING_MODE;
        break;
      case PULSES_LP45:
      case PULSES_DSM2:
      case PULSES_DSMX:
        mask |= MASK_CHANNELS_RANGE | MASK_RX_NUMBER;
        module.channelsCount = 6;
        max_rx_num = 20;
        break;
      case PULSES_CROSSFIRE:
        mask |= MASK_CHANNELS_RANGE | MASK_RX_NUMBER | MASK_BAUDRATE;
        module.channelsCount = 16;
        ui->telemetryBaudrate->setModel(ModuleData::telemetryBaudrateItemModel(protocol));
        ui->telemetryBaudrate->setField(module.crsf.telemetryBaudrate);
        break;
      case PULSES_GHOST:
        mask |= MASK_CHANNELS_RANGE | MASK_GHOST | MASK_BAUDRATE;
        module.channelsCount = 16;
        ui->telemetryBaudrate->setModel(ModuleData::telemetryBaudrateItemModel(protocol));
        ui->telemetryBaudrate->setField(module.ghost.telemetryBaudrate);
        break;
      case PULSES_PPM:
        mask |= MASK_PPM_FIELDS | MASK_SBUSPPM_FIELDS| MASK_CHANNELS_RANGE| MASK_CHANNELS_COUNT;
        if (IS_9XRPRO(board)) {
          mask |= MASK_OPEN_DRAIN;
        }
        break;
      case PULSES_SBUS:
        module.channelsCount = 16;
        mask |=  MASK_SBUSPPM_FIELDS| MASK_CHANNELS_RANGE;
        break;
      case PULSES_MULTIMODULE:
        mask |= MASK_CHANNELS_RANGE | MASK_RX_NUMBER | MASK_MULTIMODULE | MASK_SUBTYPES;
        max_rx_num = 63;
        if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
          mask |= MASK_CHANNELS_COUNT;
        else
          module.channelsCount = 16;
        if (pdef.optionsstr != nullptr)
          mask |= MASK_MULTIOPTION;
        if (pdef.hasFailsafe || (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_FRSKY && (module.subType == 0 || module.subType == 2 || module.subType > 3 )))
          mask |= MASK_FAILSAFES;
        break;
      case PULSES_AFHDS3:
        module.channelsCount = 18;
        mask |= MASK_CHANNELS_RANGE| MASK_CHANNELS_COUNT | MASK_FAILSAFES;
        mask |= MASK_SUBTYPES | MASK_RX_FREQ | MASK_RF_POWER;
        break;
      case PULSES_LEMON_DSMP:
        mask |= MASK_CHANNELS_RANGE;
        break;
      default:
        break;
    }
  }
  else if (IS_HORUS_OR_TARANIS(board)) {
    if (model->trainerMode == TRAINER_MODE_SLAVE_JACK) {
      mask |= MASK_PPM_FIELDS | MASK_SBUSPPM_FIELDS | MASK_CHANNELS_RANGE | MASK_CHANNELS_COUNT;
    }
  }
  else if (model->trainerMode != TRAINER_MODE_MASTER_JACK) {
    mask |= MASK_PPM_FIELDS | MASK_CHANNELS_RANGE | MASK_CHANNELS_COUNT;
  }

  if (module.hasFailsafes(firmware)) {
    mask |= MASK_FAILSAFES;
  }

  if (moduleIdx > 0)
    ui->telemetryBaudrate->setVisible(mask & MASK_BAUDRATE);
  else
    ui->telemetryBaudrate->setVisible(false);

  ui->label_protocol->setVisible(mask & MASK_PROTOCOL);
  ui->protocol->setVisible(mask & MASK_PROTOCOL);
  ui->label_rxNumber->setVisible(mask & MASK_RX_NUMBER);
  ui->rxNumber->setVisible(mask & MASK_RX_NUMBER);
  ui->rxNumber->setMaximum(max_rx_num);
  ui->rxNumber->setValue(module.modelId);
  ui->label_channelsStart->setVisible(mask & MASK_CHANNELS_RANGE);
  ui->channelsStart->setVisible(mask & MASK_CHANNELS_RANGE);
  ui->channelsStart->setMaximum(33 - module.channelsCount);
  ui->channelsStart->setValue(module.channelsStart+1);
  ui->label_channelsCount->setVisible(mask & MASK_CHANNELS_RANGE);
  ui->channelsCount->setVisible(mask & MASK_CHANNELS_RANGE);
  ui->channelsCount->setEnabled(mask & MASK_CHANNELS_COUNT);
  ui->channelsCount->setMaximum(module.getMaxChannelCount());
  ui->channelsCount->setValue(module.channelsCount);
  ui->channelsCount->setSingleStep(firmware->getCapability(HasPPMStart) ? 1 : 2);

  // PPM settings fields
  ui->label_ppmPolarity->setVisible(mask & MASK_SBUSPPM_FIELDS);
  ui->ppmPolarity->setVisible(mask & MASK_SBUSPPM_FIELDS);
  ui->ppmPolarity->setCurrentIndex(module.ppm.pulsePol);
  ui->label_ppmOutputType->setVisible(mask & MASK_OPEN_DRAIN);
  ui->ppmOutputType->setVisible(mask & MASK_OPEN_DRAIN);
  ui->ppmOutputType->setCurrentIndex(module.ppm.outputType);
  ui->label_ppmDelay->setVisible(mask & MASK_PPM_FIELDS);
  ui->ppmDelay->setVisible(mask & MASK_PPM_FIELDS);
  ui->ppmDelay->setValue(module.ppm.delay);
  ui->label_ppmFrameLength->setVisible(mask & MASK_SBUSPPM_FIELDS);
  ui->ppmFrameLength->setVisible(mask & MASK_SBUSPPM_FIELDS);
  ui->ppmFrameLength->setMinimum(module.channelsCount * (model->extendedLimits ? 2.250 : 2)+3.5);
  ui->ppmFrameLength->setMaximum(firmware->getCapability(PPMFrameLength));
  ui->ppmFrameLength->setValue(22.5 + ((double)module.ppm.frameLength) * 0.5);

  // Antenna mode on Horus and XLite
  if (mask & MASK_ANTENNA) {
    ui->antennaLabel->show();
    ui->antennaMode->show();
    ui->antennaMode->clear();
    ui->antennaMode->addItem(tr("Ask"), -1);
    ui->antennaMode->addItem(tr("Internal"), 0);
    ui->antennaMode->addItem(IS_HORUS_X12S(board) ? tr("Internal + External") : tr("External"), 1);
    ui->antennaMode->setField(module.pxx.antennaMode, this);
  }
  else {
    ui->antennaLabel->hide();
    ui->antennaMode->hide();
  }

  if (mask & MASK_RF_RACING_MODE) {
    ui->racingMode->show();
    ui->racingMode->setChecked(module.access.racingMode);
  }
  else {
    ui->racingMode->hide();
  }

  // R9M options
  ui->r9mPower->setVisible(mask & MASK_RF_POWER);
  ui->label_r9mPower->setVisible(mask & MASK_RF_POWER);
  ui->warning_r9mPower->setVisible((mask & MASK_R9M) && module.subType == MODULE_SUBTYPE_R9M_EU);
  ui->warning_r9mFlex->setVisible((mask & MASK_R9M) && module.subType > MODULE_SUBTYPE_R9M_EU);

  if (mask & MASK_RF_POWER) {
    const QSignalBlocker blocker(ui->r9mPower);
    ui->r9mPower->clear();
    ui->r9mPower->addItems(ModuleData::powerValueStrings(protocol, module.subType, firmware));
    ui->r9mPower->setCurrentIndex(mask & MASK_R9M ? module.pxx.power : module.afhds3.rfPower);
  }

  // module subtype
  ui->label_multiSubType->setVisible(mask & MASK_SUBTYPES);
  ui->multiSubType->setVisible(mask & MASK_SUBTYPES);
  if (mask & MASK_SUBTYPES) {
    unsigned numEntries = 2;  // R9M FCC/EU
    unsigned i = 0;
    switch(protocol){
    case PULSES_MULTIMODULE:
      numEntries = (module.multi.rfProtocol > MODULE_SUBTYPE_MULTI_LAST ? 8 : pdef.numSubTypes());
      break;
    case PULSES_PXX_R9M:
      if (firmware->getCapability(HasModuleR9MFlex))
        i = 2;
      break;
    case PULSES_AFHDS3:
        numEntries = 4;
        break;
    default:
      break;
    }
    numEntries += i;
    const QSignalBlocker blocker(ui->multiSubType);
    ui->multiSubType->clear();
    for ( ; i < numEntries; i++)
      ui->multiSubType->addItem(module.subTypeToString(i), i);
    ui->multiSubType->setCurrentIndex(ui->multiSubType->findData(module.subType));
  }

  // Multi settings fields
  ui->label_multiProtocol->setVisible(mask & MASK_MULTIMODULE);
  ui->multiProtocol->setVisible(mask & MASK_MULTIMODULE);
  ui->label_option->setVisible(mask & MASK_MULTIOPTION);
  ui->optionValue->setVisible(mask & MASK_MULTIOPTION);
  ui->disableTelem->setVisible(mask & MASK_MULTIMODULE);
  ui->disableChMap->setVisible(mask & MASK_MULTIMODULE);
  ui->lowPower->setVisible(mask & MASK_MULTIMODULE);
  ui->autoBind->setVisible(mask & MASK_MULTIMODULE);
  if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
    ui->autoBind->setVisible(false);
  else
    ui->autoBind->setText(tr("Bind on channel"));

  if (mask & MASK_MULTIMODULE) {
    ui->multiProtocol->setCurrentIndex(ui->multiProtocol->findData(module.multi.rfProtocol));
    ui->disableTelem->setChecked(module.multi.disableTelemetry);
    ui->disableChMap->setChecked(module.multi.disableMapping);
    ui->autoBind->setChecked(module.multi.autoBindMode);
    ui->lowPower->setChecked(module.multi.lowPowerMode);
  }

  if (mask & MASK_MULTIOPTION) {
    ui->optionValue->setMinimum(pdef.getOptionMin());
    ui->optionValue->setMaximum(pdef.getOptionMax());
    ui->optionValue->setValue(module.multi.optionValue);
    ui->label_option->setText(qApp->translate("Multiprotocols", qPrintable(pdef.optionsstr)));
  }

  // Ghost settings fields
  ui->raw12bits->setVisible(mask & MASK_GHOST);
  if (mask & MASK_GHOST) {
    ui->raw12bits->setChecked(module.ghost.raw12bits);
  }

  if (mask & MASK_ACCESS) {
    ui->rx1->setText(module.access.receiverName[0]);
    ui->rx2->setText(module.access.receiverName[1]);
    ui->rx3->setText(module.access.receiverName[2]);
  }

  ui->registrationIdLabel->setVisible(mask & MASK_ACCESS);
  ui->registrationId->setVisible(mask & MASK_ACCESS);

  ui->rx1Label->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 0)));
  ui->clearRx1->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 0)));
  ui->rx1->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 0)));

  ui->rx2Label->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 1)));
  ui->clearRx2->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 1)));
  ui->rx2->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 1)));

  ui->rx3Label->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 2)));
  ui->clearRx3->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 2)));
  ui->rx3->setVisible((mask & MASK_ACCESS) && (module.access.receivers & (1 << 2)));

  // Failsafes
  ui->label_failsafeMode->setVisible(mask & MASK_FAILSAFES);
  ui->failsafeMode->setVisible(mask & MASK_FAILSAFES);
  //hide reciever mode for afhds3
  qobject_cast<QListView *>(ui->failsafeMode->view())->setRowHidden(FAILSAFE_RECEIVER, protocol == PULSES_AFHDS3);

  if ((mask & MASK_FAILSAFES) && module.failsafeMode == FAILSAFE_CUSTOM) {
    if (ui->failsafesGroupBox->isHidden()) {
      setupFailsafes();
      ui->failsafesGroupBox->setVisible(true);
    }
  }
  else {
    ui->failsafesGroupBox->setVisible(false);
  }

  if (mask & MASK_FAILSAFES) {
    ui->failsafeMode->setCurrentIndex(module.failsafeMode);

    if (firmware->getCapability(ChannelsName)) {
      int chan;
      QString name;
      QMapIterator<int, ChannelFailsafeWidgetsGroup> i(failsafeGroupsMap);
      while (i.hasNext()) {
        i.next();
        chan = i.key();
        name = QString(model->limitData[chan + module.channelsStart].name).trimmed();
        if (name.isEmpty())
          i.value().label->setText(QString::number(chan + 1));
        else
          i.value().label->setText(name);
      }
    }
  }

  ui->label_rxFreq->setVisible((mask & MASK_RX_FREQ));
  ui->rxFreq->setVisible((mask & MASK_RX_FREQ));

  lock = false;
}

void ModulePanel::onProtocolChanged(int index)
{
  if (!lock) {
    module.channelsCount = module.getMaxChannelCount();
    update();

    if (module.protocol == PULSES_GHOST ||
        module.protocol == PULSES_CROSSFIRE) {
      if (Boards::getCapability(getCurrentFirmware()->getBoard(),
                                Board::SportMaxBaudRate) < 400000) {
        // default to 115k
        ui->telemetryBaudrate->setCurrentIndex(0);
      } else {
        // default to 400k
        ui->telemetryBaudrate->setCurrentIndex(1);
      }
    }

    emit updateItemModels();
    emit modified();
  }
}

void ModulePanel::on_ppmPolarity_currentIndexChanged(int index)
{
  if (!lock && module.ppm.pulsePol != (bool)index) {
    module.ppm.pulsePol = index;
    emit modified();
  }
}

void ModulePanel::on_r9mPower_currentIndexChanged(int index)
{
  if (!lock) {

    if (module.protocol == PULSES_AFHDS3 && module.afhds3.rfPower != (unsigned int)index) {
      module.afhds3.rfPower = index;
      emit modified();
    }
    else if(module.pxx.power != (unsigned int)index)
      module.pxx.power = index;
  }
}

void ModulePanel::on_ppmOutputType_currentIndexChanged(int index)
{
  if (!lock && module.ppm.outputType != (bool)index) {
    module.ppm.outputType = index;
    emit modified();
  }
}

void ModulePanel::on_channelsCount_editingFinished()
{
  if (!lock && module.channelsCount != ui->channelsCount->value()) {
    module.channelsCount = ui->channelsCount->value();
    emit channelsRangeChanged();
    update();
    emit modified();
  }
}

void ModulePanel::on_channelsStart_editingFinished()
{
  if (!lock && module.channelsStart != (unsigned)ui->channelsStart->value() - 1) {
    module.channelsStart = (unsigned)ui->channelsStart->value() - 1;
    emit channelsRangeChanged();
    update();
    emit modified();
  }
}

void ModulePanel::on_ppmDelay_editingFinished()
{
  if (!lock && module.ppm.delay != ui->ppmDelay->value()) {
    // TODO only accept valid values
    module.ppm.delay = ui->ppmDelay->value();
    emit modified();
  }
}

void ModulePanel::on_rxNumber_editingFinished()
{
  if (module.modelId != (unsigned)ui->rxNumber->value()) {
    module.modelId = (unsigned)ui->rxNumber->value();
    emit modified();
  }
}

void ModulePanel::on_ppmFrameLength_editingFinished()
{
  int val = (ui->ppmFrameLength->value() - 22.5) / 0.5;
  if (module.ppm.frameLength != val) {
    module.ppm.frameLength = val;
    emit modified();
  }
}

void ModulePanel::on_failsafeMode_currentIndexChanged(int value)
{
  if (!lock && module.failsafeMode != (unsigned)value) {
    module.failsafeMode = value;
    update();
    emit modified();
  }
}

void ModulePanel::onMultiProtocolChanged(int index)
{
  int rfProtocol = ui->multiProtocol->itemData(index).toInt();
  if (!lock && module.multi.rfProtocol != (unsigned)rfProtocol) {
    lock=true;
    module.multi.rfProtocol = (unsigned int)rfProtocol;
    unsigned int maxSubTypes = multiProtocols.getProtocol(index).numSubTypes();
    if (rfProtocol > MODULE_SUBTYPE_MULTI_LAST)
      maxSubTypes = 8;
    module.subType = std::min(module.subType, maxSubTypes - 1);
    module.channelsCount = module.getMaxChannelCount();
    update();
    emit updateItemModels();
    emit modified();
    lock = false;
  }
}

void ModulePanel::on_optionValue_editingFinished()
{
  if (module.multi.optionValue != ui->optionValue->value()) {
    module.multi.optionValue = ui->optionValue->value();
    emit modified();
  }
}

void ModulePanel::onSubTypeChanged()
{
  const unsigned type = ui->multiSubType->currentData().toUInt();
  if (!lock && module.subType != type) {
    lock=true;
    module.subType = type;
    if (module.protocol != PULSES_AFHDS3) {
      update();
    }
    emit modified();
    lock =  false;
  }
}

void ModulePanel::onRfFreqChanged(int freq) {
  if (module.afhds3.rxFreq != (unsigned int)freq) {
    module.afhds3.rxFreq = (unsigned int)freq;
    emit modified();
  }
}

void ModulePanel::on_disableTelem_stateChanged(int state)
{
  module.multi.disableTelemetry = (state == Qt::Checked);
}

void ModulePanel::on_disableChMap_stateChanged(int state)
{
  module.multi.disableMapping = (state == Qt::Checked);
}

void ModulePanel::on_raw12bits_stateChanged(int state)
{
  module.ghost.raw12bits = (state == Qt::Checked);
}

void ModulePanel::on_racingMode_stateChanged(int state)
{
  module.access.racingMode = (state == Qt::Checked);
}

void ModulePanel::on_autoBind_stateChanged(int state)
{
  module.multi.autoBindMode = (state == Qt::Checked);
}

void ModulePanel::on_lowPower_stateChanged(int state)
{
  module.multi.lowPowerMode = (state == Qt::Checked);
}

void ModulePanel::onFailsafeModified(unsigned channel)
{
  updateFailsafeUI(channel, FAILSAFE_DISPLAY_PERCENT | FAILSAFE_DISPLAY_USEC);
}

void ModulePanel::updateFailsafeUI(unsigned channel, quint8 updtSb)
{
  int value = model->limitData[channel].failsafe;
  double pctVal = divRoundClosest(value * 1000, 1024) / 10.0;

  if (failsafeGroupsMap.contains(channel)) {
    const ChannelFailsafeWidgetsGroup & grp = failsafeGroupsMap.value(channel);
    bool disable = (value == FAILSAFE_CHANNEL_HOLD || value == FAILSAFE_CHANNEL_NOPULSE);
    if (grp.combo) {
      grp.combo->setCurrentIndex(grp.combo->findData(disable ? value : 0));
    }
    if ((updtSb & FAILSAFE_DISPLAY_PERCENT) && grp.sbPercent) {
      grp.sbPercent->blockSignals(true);
      grp.sbPercent->setValue(pctVal);
      grp.sbPercent->blockSignals(false);
    }
    if ((updtSb & FAILSAFE_DISPLAY_USEC) && grp.sbUsec) {
      grp.sbUsec->blockSignals(true);
      grp.sbUsec->setValue(value);
      grp.sbUsec->blockSignals(false);
    }
  }
}

// updtSb (update spin box(es)): 0=none or bitmask of FailsafeValueDisplayTypes
void ModulePanel::setChannelFailsafeValue(const int channel, const int value, quint8 updtSb)
{
  if (channel < 0 || channel >= CPN_MAX_CHNOUT)
    return;

  model->limitData[channel].failsafe = value;
  updateFailsafeUI(channel, updtSb);

  if (!lock) {
    emit failsafeModified(channel);
    emit modified();
  }
}

void ModulePanel::onFailsafeUsecChanged(int value)
{
  if (!sender())
    return;

  bool ok = false;
  int channel = sender()->property("index").toInt(&ok);
  if (ok)
    setChannelFailsafeValue(channel, value, FAILSAFE_DISPLAY_PERCENT);
}

void ModulePanel::onFailsafePercentChanged(double value)
{
  if (!sender())
    return;

  bool ok = false;
  int channel = sender()->property("index").toInt(&ok);
  if (ok)
    setChannelFailsafeValue(channel, divRoundClosest(int(value * 1024), 100), FAILSAFE_DISPLAY_USEC);
}

void ModulePanel::onFailsafeComboIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    int channel = sender()->property("index").toInt(&ok);
    if (ok) {
      model->limitData[channel].failsafe = cb->itemData(index).toInt();
      updateFailsafe(channel);
      emit failsafeModified(channel);
      emit modified();
    }
    lock = false;
  }
}

void ModulePanel::onFailsafesDisplayValueTypeChanged(int type)
{
  if (failsafesValueDisplayType != type) {
    failsafesValueDisplayType = type;
    foreach (ChannelFailsafeWidgetsGroup grp, failsafeGroupsMap) {
      if (grp.sbPercent)
        grp.sbPercent->setVisible(type == FAILSAFE_DISPLAY_PERCENT);
      if (grp.sbUsec)
        grp.sbUsec->setVisible(type == FAILSAFE_DISPLAY_USEC);
    }
  }
}

void ModulePanel::onExtendedLimitsToggled()
{
  double channelMaxPct = double(model->getChannelsMax());
  int channelMaxUs = 512 * channelMaxPct / 100 * 2;
  foreach (ChannelFailsafeWidgetsGroup grp, failsafeGroupsMap) {
    if (grp.sbPercent)
      grp.sbPercent->setRange(-channelMaxPct, channelMaxPct);
    if (grp.sbUsec)
      grp.sbUsec->setRange(-channelMaxUs, channelMaxUs);
  }
}

void ModulePanel::updateFailsafe(unsigned channel)
{
  if (channel >= CPN_MAX_CHNOUT || !failsafeGroupsMap.contains(channel))
    return;

  const int failsafeValue = model->limitData[channel].failsafe;
  const ChannelFailsafeWidgetsGroup & grp = failsafeGroupsMap.value(channel);
  const bool valDisable = (failsafeValue == FAILSAFE_CHANNEL_HOLD || failsafeValue == FAILSAFE_CHANNEL_NOPULSE);

  if (grp.combo)
    grp.combo->setCurrentIndex(grp.combo->findData(valDisable ? failsafeValue : 0));
  if (grp.sbPercent)
    grp.sbPercent->setDisabled(valDisable);
  if (grp.sbUsec)
    grp.sbUsec->setDisabled(valDisable);

  if (!valDisable)
    setChannelFailsafeValue(channel, failsafeValue, FAILSAFE_DISPLAY_PERCENT | FAILSAFE_DISPLAY_USEC);
}

void ModulePanel::onClearAccessRxClicked()
{
  QPushButton *button = qobject_cast<QPushButton *>(sender());

  if (button == ui->clearRx1) {
    module.access.receivers &= ~(1 << 0);
    ui->rx1->clear();
    update();
    emit modified();
  }
  else if (button == ui->clearRx2) {
    module.access.receivers &= ~(1 << 1);
    ui->rx2->clear();
    update();
    emit modified();
  }
  else if (button == ui->clearRx3) {
    module.access.receivers &= ~(1 << 2);
    ui->rx3->clear();
    update();
    emit modified();
  }
}

/******************************************************************************/
FunctionSwitchesPanel::FunctionSwitchesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::FunctionSwitches)
{
  ui->setupUi(this);

  AbstractStaticItemModel *fsConfig = ModelData::funcSwitchConfigItemModel();
  AbstractStaticItemModel *fsStart = ModelData::funcSwitchStartItemModel();

  lock = true;

  QRegExp rx(CHAR_FOR_NAMES_REGEX);

  switchcnt = Boards::getCapability(firmware->getBoard(), Board::FunctionSwitches);

  for (int i = 0; i < switchcnt; i++) {
    QLabel * lblSwitchId = new QLabel(this);
    lblSwitchId->setText(tr("SW%1").arg(i + 1));

    AutoLineEdit * aleName = new AutoLineEdit(this);
    aleName->setProperty("index", i);
    aleName->setValidator(new QRegExpValidator(rx, this));
    aleName->setField((char *)model.functionSwitchNames[i], 3);

    QComboBox * cboConfig = new QComboBox(this);
    cboConfig->setProperty("index", i);
    cboConfig->setModel(fsConfig);

    QComboBox * cboStartPosn = new QComboBox(this);
    cboStartPosn->setProperty("index", i);
    cboStartPosn->setModel(fsStart);

    QSpinBox * sbGroup = new QSpinBox(this);
    sbGroup->setProperty("index", i);
    sbGroup->setMaximum(3);
    sbGroup->setSpecialValueText("-");

    QCheckBox * cbAlwaysOnGroup = new QCheckBox(this);
    cbAlwaysOnGroup->setProperty("index", i);

    int row = 0;
    int coloffset = 1;
    ui->gridSwitches->addWidget(lblSwitchId, row++, i + coloffset);
    ui->gridSwitches->addWidget(aleName, row++, i + coloffset);
    ui->gridSwitches->addWidget(cboConfig, row++, i + coloffset);
    ui->gridSwitches->addWidget(cboStartPosn, row++, i + coloffset);
    ui->gridSwitches->addWidget(sbGroup, row++, i + coloffset);
    ui->gridSwitches->addWidget(cbAlwaysOnGroup, row++, i + coloffset);

    connect(aleName, &AutoLineEdit::currentDataChanged, this, &FunctionSwitchesPanel::on_nameEditingFinished);
    connect(cboConfig, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_configCurrentIndexChanged);
    connect(cboStartPosn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FunctionSwitchesPanel::on_startPosnCurrentIndexChanged);
    connect(sbGroup, QOverload<int>::of(&QSpinBox::valueChanged), this, &FunctionSwitchesPanel::on_groupChanged);
    connect(cbAlwaysOnGroup, &QCheckBox::toggled, this, &FunctionSwitchesPanel::on_alwaysOnGroupChanged);

    aleNames << aleName;
    cboConfigs << cboConfig;
    cboStartupPosns << cboStartPosn;
    sbGroups << sbGroup;
    cbAlwaysOnGroups << cbAlwaysOnGroup;
  }

  update();

  lock = false;
}

FunctionSwitchesPanel::~FunctionSwitchesPanel()
{
  delete ui;
}

void FunctionSwitchesPanel::update()
{
  for (int i = 0; i < switchcnt; i++) {
    update(i);
  }
}

void FunctionSwitchesPanel::update(int index)
{
  lock = true;

  for (int i = 0; i < switchcnt; i++) {
    aleNames[i]->update();
    cboConfigs[i]->setCurrentIndex(model->getFuncSwitchConfig(i));
    cboStartupPosns[i]->setCurrentIndex(model->getFuncSwitchStart(i));
    unsigned int grp = model->getFuncSwitchGroup(i);
    sbGroups[i]->setValue(grp);
    cbAlwaysOnGroups[i]->setChecked(model->getFuncSwitchAlwaysOnGroup(i));

    if (cboConfigs[i]->currentIndex() < 2)
      cboStartupPosns[i]->setEnabled(false);
    else
      cboStartupPosns[i]->setEnabled(true);

    if (cboConfigs[i]->currentIndex() < 1)
      sbGroups[i]->setEnabled(false);
    else
      sbGroups[i]->setEnabled(true);

    if (!(sbGroups[i]->isEnabled()) || grp < 1)
      cbAlwaysOnGroups[i]->setEnabled(false);
    else
      cbAlwaysOnGroups[i]->setEnabled(true);
  }

  lock = false;
}

void FunctionSwitchesPanel::on_nameEditingFinished()
 {
   emit updateDataModels();
 }


void FunctionSwitchesPanel::on_configCurrentIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    unsigned int i = sender()->property("index").toInt(&ok);
      if (ok && model->getFuncSwitchConfig(i) != (unsigned int)index) {
        model->setFuncSwitchConfig(i, index);
      if (index < 2)
          model->setFuncSwitchStart(i, ModelData::FUNC_SWITCH_START_INACTIVE);
      if (index < 1)
          model->setFuncSwitchGroup(i, 0);
      update(i);
      emit modified();
      emit updateDataModels();
     }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_startPosnCurrentIndexChanged(int index)
{
  if (!sender())
    return;

  QComboBox * cb = qobject_cast<QComboBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    unsigned int i = sender()->property("index").toInt(&ok);
    if (ok && model->getFuncSwitchStart(i) != (unsigned int)index) {
      model->setFuncSwitchStart(i, index);
      emit modified();
    }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_groupChanged(int value)
{
  if (!sender())
    return;

  QSpinBox * sb = qobject_cast<QSpinBox *>(sender());

  if (sb && !lock) {
    lock = true;
    bool ok = false;
    int i = sender()->property("index").toInt(&ok);
    if (ok && model->getFuncSwitchGroup(i) != (unsigned int)value) {
      model->setFuncSwitchGroup(i, (unsigned int)value);
      update(i);
      emit modified();
    }
    lock = false;
  }
}

void FunctionSwitchesPanel::on_alwaysOnGroupChanged(int value)
{
  if (!sender())
    return;

  QCheckBox * cb = qobject_cast<QCheckBox *>(sender());

  if (cb && !lock) {
    lock = true;
    bool ok = false;
    int i = sender()->property("index").toInt(&ok);

    if (ok) {
      model->setFuncSwitchAlwaysOnGroup(i, (unsigned int)value);
      update();
      emit modified();
    }

    lock = false;
  }
}

/******************************************************************************/

SetupPanel::SetupPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                       CompoundItemModelFactory * sharedItemModels) :
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::Setup),
  sharedItemModels(sharedItemModels)
{
  ui->setupUi(this);

  lock = true;

  panelFilteredModels = new FilteredItemModelFactory();

  panelFilteredModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                               RawSwitch::MixesContext),
                                         FIM_TIMERSWITCH);
  connectItemModelEvents(panelFilteredModels->getItemModel(FIM_TIMERSWITCH));

  panelFilteredModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_ThrSource)),
                                         FIM_THRSOURCE);
  connectItemModelEvents(panelFilteredModels->getItemModel(FIM_THRSOURCE));

  panelItemModels = new CompoundItemModelFactory(&generalSettings, &model);
  panelItemModels->registerItemModel(TimerData::countdownBeepItemModel());
  panelItemModels->registerItemModel(TimerData::countdownStartItemModel());
  panelItemModels->registerItemModel(TimerData::persistentItemModel());
  panelItemModels->registerItemModel(TimerData::modeItemModel());
  panelFilteredModels->registerItemModel(new FilteredItemModel(ModelData::trainerModeItemModel(generalSettings, firmware)), FIM_TRAINERMODE);
  panelItemModels->registerItemModel(TimerData::showElapsedItemModel());
  Board::Type board = firmware->getBoard();

  memset(modules, 0, sizeof(modules));

  QRegExp rx(CHAR_FOR_NAMES_REGEX);
  ui->name->setValidator(new QRegExpValidator(rx, this));
  ui->name->setMaxLength(firmware->getCapability(ModelName));

  if (firmware->getCapability(HasModelImage)) {
    if (Boards::getCapability(board, Board::HasColorLcd)) {
      ui->imagePreview->setFixedSize(QSize(192, 114));
    }
    else {
      ui->imagePreview->setFixedSize(QSize(64, 32));
    }
    QStringList items;
    items.append("");
    QString path = g.profile[g.id()].sdPath();
    path.append("/IMAGES/");
    QDir qd(path);
    if (qd.exists()) {
      QStringList filters = firmware->getCapabilityStr(ModelImageFilters).split("|");
      foreach ( QString file, qd.entryList(filters, QDir::Files) ) {
        QFileInfo fi(file);
        QString temp;
        if (firmware->getCapability(ModelImageKeepExtn))
          temp = fi.fileName();
        else
          temp = fi.completeBaseName();
        if (!items.contains(temp) && temp.length() <= firmware->getCapability(ModelImageNameLen))
          items.append(temp);
      }
    }
    if (!items.contains(model.bitmap)) {
      items.append(model.bitmap);
    }
    items.sort(Qt::CaseInsensitive);
    foreach (QString file, items) {
      ui->image->addItem(file);
      if (file == model.bitmap) {
        ui->image->setCurrentIndex(ui->image->count() - 1);
        if (!file.isEmpty()) {
          QString fileName = path;
          fileName.append(model.bitmap);
          if (!firmware->getCapability(ModelImageKeepExtn)) {
            QString extn = firmware->getCapabilityStr(ModelImageFilters);
            if (extn.size() > 0)
              extn.remove(0, 1);  //  remove *
            fileName.append(extn);
          }
          QImage image(fileName);
          if (!image.isNull()) {
            ui->imagePreview->setPixmap(QPixmap::fromImage(image.scaled(ui->imagePreview->size())));
          }
        }
      }
    }
  }
  else {
    ui->image->hide();
    ui->modelImage_label->hide();
    ui->imagePreview->hide();
  }

  QWidget * prevFocus = ui->image;

  timersCount = firmware->getCapability(Timers);

  for (int i = 0; i < CPN_MAX_TIMERS; i++) {
    if (i < timersCount) {
      timers[i] = new TimerPanel(this, model, model.timers[i], generalSettings, firmware, prevFocus, panelFilteredModels, panelItemModels);
      ui->gridLayout->addWidget(timers[i], 1+i, 1);
      connect(timers[i], &TimerPanel::modified, this, &SetupPanel::modified);
      connect(timers[i], &TimerPanel::nameChanged, this, &SetupPanel::onTimerNameChanged);
      connect(this, &SetupPanel::updated, timers[i], &TimerPanel::update);
      prevFocus = timers[i]->getLastFocus();
      //  TODO more reliable method required
      QLabel *label = findChild<QLabel *>(QString("label_timer%1").arg(i + 1));
      if (label) {  //  to stop crashing if not found
        label->setProperty("index", i);
        label->setContextMenuPolicy(Qt::CustomContextMenu);
        label->setToolTip(tr("Popup menu available"));
        label->setMouseTracking(true);
        connect(label, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onTimerCustomContextMenuRequested(QPoint)));
      }
    }
    else {
      foreach(QLabel *label, findChildren<QLabel *>(QRegularExpression(QString("label_timer%1").arg(i + 1 )))) { //TODO more reliable method required
        label->hide();
      }
    }
  }

  if (firmware->getCapability(HasTopLcd)) {
    ui->toplcdTimer->setField(model.toplcdTimer, this);
    for (int i = 0; i < CPN_MAX_TIMERS; i++) {
      if (i < timersCount) {
        ui->toplcdTimer->addItem(tr("Timer %1").arg(i + 1), i);
      }
    }
  }
  else {
    ui->toplcdTimerLabel->hide();
    ui->toplcdTimer->hide();
  }

  ui->throttleSource->setModel(panelFilteredModels->getItemModel(FIM_THRSOURCE));
  ui->throttleSource->setField(model.thrTraceSrc, this);

  if (!firmware->getCapability(HasDisplayText)) {
    ui->displayText->hide();
    ui->editText->hide();
  }

  if (!firmware->getCapability(GlobalFunctions)) {
    ui->gfEnabled->hide();
  }

  if (!firmware->getCapability(HasADCJitterFilter))
  {
    ui->jitterFilter->hide();
  }

  // Beep Center checkboxes
  prevFocus = ui->trimsDisplay;
  int analogs = CPN_MAX_STICKS + getBoardCapability(board, Board::Pots) + getBoardCapability(board, Board::Sliders);
  int genAryIdx = 0;
  for (int i = 0; i < analogs + firmware->getCapability(RotaryEncoders); i++) {
    RawSource src((i < analogs) ? SOURCE_TYPE_STICK : SOURCE_TYPE_ROTARY_ENCODER, (i < analogs) ? i : analogs - i);
    QCheckBox * checkbox = new QCheckBox(this);
    checkbox->setProperty("index", i);
    checkbox->setText(src.toString(&model, &generalSettings));
    ui->centerBeepLayout->addWidget(checkbox, 0, i + 1);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onBeepCenterToggled(bool)));
    centerBeepCheckboxes << checkbox;
    if (IS_HORUS_OR_TARANIS(board)) {
      if (src.isPot(&genAryIdx) && !generalSettings.isPotAvailable(genAryIdx)) {
        checkbox->hide();
      }
      else if (src.isSlider(&genAryIdx) && !generalSettings.isSliderAvailable(genAryIdx)) {
        checkbox->hide();
      }
    }
    QWidget::setTabOrder(prevFocus, checkbox);
    prevFocus = checkbox;
  }

  // Startup switches warnings
  for (int i = 0; i < getBoardCapability(board, Board::Switches); i++) {
    Board::SwitchInfo switchInfo = Boards::getSwitchInfo(board, i);
    switchInfo.config = Board::SwitchType(generalSettings.switchConfig[i]);
    if (switchInfo.config == Board::SWITCH_NOT_AVAILABLE || switchInfo.config == Board::SWITCH_TOGGLE) {
      model.switchWarningEnable |= (1 << i);
      continue;
    }
    RawSource src(RawSourceType::SOURCE_TYPE_SWITCH, i);
    QLabel * label = new QLabel(this);
    QSlider * slider = new QSlider(this);
    QCheckBox * cb = new QCheckBox(this);
    slider->setProperty("index", i);
    slider->setOrientation(Qt::Vertical);
    slider->setMinimum(0);
    slider->setInvertedAppearance(true);
    slider->setInvertedControls(true);
    slider->setTickPosition(QSlider::TicksBothSides);
    slider->setMinimumSize(QSize(30, 50));
    slider->setMaximumSize(QSize(50, 50));
    slider->setSingleStep(1);
    slider->setPageStep(1);
    slider->setTickInterval(1);
    label->setText(src.toString(&model, &generalSettings));
    slider->setMaximum(switchInfo.config == Board::SWITCH_3POS ? 2 : 1);
    cb->setProperty("index", i);
    ui->switchesStartupLayout->addWidget(label, 0, i + 1);
    ui->switchesStartupLayout->setAlignment(label, Qt::AlignCenter);
    ui->switchesStartupLayout->addWidget(slider, 1, i + 1);
    ui->switchesStartupLayout->setAlignment(slider, Qt::AlignCenter);
    ui->switchesStartupLayout->addWidget(cb, 2, i + 1);
    ui->switchesStartupLayout->setAlignment(cb, Qt::AlignCenter);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(startupSwitchEdited(int)));
    connect(cb, SIGNAL(toggled(bool)), this, SLOT(startupSwitchToggled(bool)));
    startupSwitchesSliders << slider;
    startupSwitchesCheckboxes << cb;
    QWidget::setTabOrder(prevFocus, slider);
    QWidget::setTabOrder(slider, cb);
    prevFocus = cb;
  }

  // Pot warnings
  prevFocus = ui->potWarningMode;
  int count = getBoardCapability(board, Board::Pots) + getBoardCapability(board, Board::Sliders);

  if (IS_HORUS_OR_TARANIS(board) && count > 0) {
    for (int i = 0; i < count; i++) {
      RawSource src(SOURCE_TYPE_STICK, CPN_MAX_STICKS + i);
      QCheckBox * cb = new QCheckBox(this);
      cb->setProperty("index", i);
      cb->setText(src.toString(&model, &generalSettings));
      ui->potWarningLayout->addWidget(cb, 0, i + 1);
      connect(cb, SIGNAL(toggled(bool)), this, SLOT(potWarningToggled(bool)));
      potWarningCheckboxes << cb;
      if (src.isPot(&genAryIdx) && !generalSettings.isPotAvailable(genAryIdx)) {
        cb->hide();
      }
      else if (src.isSlider(&genAryIdx) && !generalSettings.isSliderAvailable(genAryIdx)) {
        cb->hide();
      }
      QWidget::setTabOrder(prevFocus, cb);
      prevFocus = cb;
    }
  }
  else {
    ui->label_potWarning->hide();
    ui->potWarningMode->hide();
  }

  ui->trimsDisplay->setField(model.trimsDisplay, this);

  if (Boards::getCapability(firmware->getBoard(), Board::FunctionSwitches) > 0) {
    funcswitches = new FunctionSwitchesPanel(this, model, generalSettings, firmware);
    ui->functionSwitchesLayout->addWidget(funcswitches);
    connect(funcswitches, &FunctionSwitchesPanel::modified, this, &SetupPanel::modified);
    connect(funcswitches, &FunctionSwitchesPanel::updateDataModels, this, &SetupPanel::onFunctionSwitchesUpdateItemModels);
  }

  for (int i = firmware->getCapability(NumFirstUsableModule); i < firmware->getCapability(NumModules); i++) {
    modules[i] = new ModulePanel(this, model, model.moduleData[i], generalSettings, firmware, i, panelFilteredModels);
    ui->modulesLayout->addWidget(modules[i]);
    connect(modules[i], &ModulePanel::modified, this, &SetupPanel::modified);
    connect(modules[i], &ModulePanel::updateItemModels, this, &SetupPanel::onModuleUpdateItemModels);
    connect(this, &SetupPanel::extendedLimitsToggled, modules[i], &ModulePanel::onExtendedLimitsToggled);
  }

  for (int i = firmware->getCapability(NumFirstUsableModule); i < firmware->getCapability(NumModules); i++) {
    for (int j = firmware->getCapability(NumFirstUsableModule); j < firmware->getCapability(NumModules); j++) {
      if (i != j) {
        connect(modules[i], SIGNAL(failsafeModified(unsigned)), modules[j], SLOT(onFailsafeModified(unsigned)));
      }
    }
  }

  if (firmware->getCapability(ModelTrainerEnable)) {
    modules[CPN_MAX_MODULES] = new ModulePanel(this, model, model.moduleData[CPN_MAX_MODULES], generalSettings, firmware, -1, panelFilteredModels);
    ui->modulesLayout->addWidget(modules[CPN_MAX_MODULES]);
    connect(modules[CPN_MAX_MODULES], &ModulePanel::modified, this, &SetupPanel::modified);
  }

  disableMouseScrolling();

  lock = false;
}

SetupPanel::~SetupPanel()
{
  delete ui;
  delete panelFilteredModels;
  delete panelItemModels;
}

void SetupPanel::on_extendedLimits_toggled(bool checked)
{
  model->extendedLimits = checked;
  emit extendedLimitsToggled();
  emit modified();
}

void SetupPanel::on_throttleWarning_toggled(bool checked)
{
  model->disableThrottleWarning = !checked;
  emit modified();
}

void SetupPanel::on_enableCustomThrottleWarning_toggled(bool checked)
{
  model->enableCustomThrottleWarning = checked;
  emit modified();
}

void SetupPanel::on_customThrottleWarningPosition_valueChanged(int value)
{
  model->customThrottleWarningPosition = value;
  emit modified();
}

void SetupPanel::on_throttleReverse_toggled(bool checked)
{
  model->throttleReversed = checked;
  emit modified();
}

void SetupPanel::on_extendedTrims_toggled(bool checked)
{
  model->extendedTrims = checked;
  emit modified();
}

void SetupPanel::on_trimIncrement_currentIndexChanged(int index)
{
  model->trimInc = index - 2;
  emit modified();
}

void SetupPanel::on_throttleTrimSwitch_currentIndexChanged(int index)
{
  if (!lock) {
    model->thrTrimSwitch = ui->throttleTrimSwitch->currentData().toUInt();
    emit modified();
  }
}

void SetupPanel::on_name_editingFinished()
{
  if (QString(model->name) != ui->name->text()) {
    int length = ui->name->maxLength();
    strncpy(model->name, ui->name->text().toLatin1(), length);
    emit modified();
  }
}

void SetupPanel::on_image_currentIndexChanged(int index)
{
  if (!lock) {
    memset(model->bitmap, 0, CPN_MAX_BITMAP_LEN);
    strncpy(model->bitmap, ui->image->currentText().toLatin1(), CPN_MAX_BITMAP_LEN);
    if (model->bitmap[0] != '\0') {
      QString path = g.profile[g.id()].sdPath();
      path.append("/IMAGES/");
      path.append(model->bitmap);
      if (!firmware->getCapability(ModelImageKeepExtn)) {
        QString extn = firmware->getCapabilityStr(ModelImageFilters);
        if (extn.size() > 0)
          extn.remove(0, 1);  //  remove *
        path.append(extn);
      }
      QImage image(path);
      if (!image.isNull())
        ui->imagePreview->setPixmap(QPixmap::fromImage(image.scaled(ui->imagePreview->size())));
      else
        ui->imagePreview->clear();
    }
    else {
      ui->imagePreview->clear();
    }
    emit modified();
  }
}

void SetupPanel::populateThrottleTrimSwitchCB()
{
  Board::Type board = firmware->getBoard();
  lock = true;
  ui->throttleTrimSwitch->clear();
  int idx=0;
  QString trim;
  for (int i=0; i<getBoardCapability(board, Board::NumTrims); i++, idx++) {
    // here order is TERA instead of RETA
    if (i == 0)
      trim = RawSource(SOURCE_TYPE_TRIM, 2).toString(model, &generalSettings);
    else if (i == 2)
      trim = RawSource(SOURCE_TYPE_TRIM, 0).toString(model, &generalSettings);
    else
      trim = RawSource(SOURCE_TYPE_TRIM, i).toString(model, &generalSettings);
    ui->throttleTrimSwitch->addItem(trim, idx);
  }

  int thrTrimSwitchIdx = ui->throttleTrimSwitch->findData(model->thrTrimSwitch);
  ui->throttleTrimSwitch->setCurrentIndex(thrTrimSwitchIdx);
  lock = false;
}

void SetupPanel::update()
{
  ui->name->setText(model->name);
  ui->throttleReverse->setChecked(model->throttleReversed);
  ui->throttleSource->updateValue();
  populateThrottleTrimSwitchCB();
  ui->throttleWarning->setChecked(!model->disableThrottleWarning);
  ui->enableCustomThrottleWarning->setChecked(model->enableCustomThrottleWarning);
  ui->customThrottleWarningPosition->setValue(model->customThrottleWarningPosition);
  ui->trimIncrement->setCurrentIndex(model->trimInc+2);
  ui->throttleTrim->setChecked(model->thrTrim);
  ui->extendedLimits->setChecked(model->extendedLimits);
  ui->extendedTrims->setChecked(model->extendedTrims);
  ui->displayText->setChecked(model->displayChecklist);
  ui->gfEnabled->setChecked(!model->noGlobalFunctions);
  ui->jitterFilter->setCurrentIndex(model->jitterFilter);

  updateBeepCenter();
  updateStartupSwitches();

  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    updatePotWarnings();
  }

  for (int i = 0; i < CPN_MAX_MODULES + 1; i++) {
    if (modules[i]) {
      modules[i]->update();
    }
  }

  emit updated();
}

void SetupPanel::updateBeepCenter()
{
  for (int i = 0; i < centerBeepCheckboxes.size(); i++) {
    centerBeepCheckboxes[i]->setChecked(model->beepANACenter & (0x01 << i));
  }
}

void SetupPanel::updateStartupSwitches()
{
  lock = true;

  uint64_t switchStates = model->switchWarningStates;
  uint64_t value;

  for (int i = 0; i < startupSwitchesSliders.size(); i++) {
    QSlider * slider = startupSwitchesSliders[i];
    QCheckBox * cb = startupSwitchesCheckboxes[i];
    int index = slider->property("index").toInt();
    bool enabled = !(model->switchWarningEnable & (1 << index));
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      value = (switchStates >> (2 * index)) & 0x03;
      if (generalSettings.switchConfig[index] != Board::SWITCH_3POS && value == 2) {
        value = 1;
      }
    }
    else {
      value = (i == 0 ? switchStates & 0x3 : switchStates & 0x1);
      switchStates >>= (i == 0 ? 2 : 1);
    }
    slider->setValue(value);
    slider->setEnabled(enabled);
    cb->setChecked(enabled);
  }

  lock = false;
}

void SetupPanel::startupSwitchEdited(int value)
{
  if (!lock) {
    int shift = 0;
    uint64_t mask;
    int index = sender()->property("index").toInt();

    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      shift = index * 2;
      mask = 0x03ull << shift;
    }
    else {
      if (index == 0) {
        mask = 0x03;
      }
      else {
        shift = index + 1;
        mask = 0x01ull << shift;
      }
    }

    model->switchWarningStates &= ~mask;

    if (IS_HORUS_OR_TARANIS(firmware->getBoard()) && generalSettings.switchConfig[index] != Board::SWITCH_3POS) {
      if (value == 1) {
        value = 2;
      }
    }

    if (value) {
      model->switchWarningStates |= ((uint64_t)value << shift);
    }

    updateStartupSwitches();
    emit modified();
  }
}

void SetupPanel::startupSwitchToggled(bool checked)
{
  if (!lock) {
    int index = sender()->property("index").toInt();

    if (checked)
      model->switchWarningEnable &= ~(1 << index);
    else
      model->switchWarningEnable |= (1 << index);

    updateStartupSwitches();
    emit modified();
  }
}

void SetupPanel::updatePotWarnings()
{
  lock = true;
  ui->potWarningMode->setCurrentIndex(model->potsWarningMode);
  for (int i = 0; i < potWarningCheckboxes.size(); i++) {
    QCheckBox *checkbox = potWarningCheckboxes[i];
    int index = checkbox->property("index").toInt();
    checkbox->setChecked(model->potsWarnEnabled[index]);
    checkbox->setDisabled(model->potsWarningMode == 0);
  }
  lock = false;
}

void SetupPanel::potWarningToggled(bool checked)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    model->potsWarnEnabled[index] = checked;
    updatePotWarnings();
    emit modified();
  }
}

void SetupPanel::on_potWarningMode_currentIndexChanged(int index)
{
  if (!lock) {
    model->potsWarningMode = index;
    updatePotWarnings();
    emit modified();
  }
}

void SetupPanel::on_displayText_toggled(bool checked)
{
  model->displayChecklist = checked;
  emit modified();
}

void SetupPanel::on_gfEnabled_toggled(bool checked)
{
  model->noGlobalFunctions = !checked;
  emit modified();
}

void SetupPanel::on_jitterFilter_currentIndexChanged(int index)
{
  if (!lock) {
    model->jitterFilter = ui->jitterFilter->currentIndex();
    emit modified();
  }
}

void SetupPanel::on_throttleTrim_toggled(bool checked)
{
  model->thrTrim = checked;
  emit modified();
}

void SetupPanel::onBeepCenterToggled(bool checked)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    unsigned int mask = (0x01 << index);
    if (checked)
      model->beepANACenter |= mask;
    else
      model->beepANACenter &= ~mask;
    emit modified();
  }
}

void SetupPanel::on_editText_clicked()
{
  const QString path = Helpers::getChecklistsPath();
  QDir d(path);
  if (!d.exists()) {
    QMessageBox::critical(this, tr("Profile Settings"), tr("SD structure path not specified or invalid"));
  }
  else {
    ChecklistDialog *g = new ChecklistDialog(this, model);
    g->exec();
  }
}

void SetupPanel::onTimerCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedTimerIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this, SLOT(cmTimerCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this, SLOT(cmTimerCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this, SLOT(cmTimerPaste()))->setEnabled(hasTimerClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(cmTimerClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this, SLOT(cmTimerInsert()))->setEnabled(insertTimerAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this, SLOT(cmTimerDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(cmTimerMoveUp()))->setEnabled(moveTimerUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(cmTimerMoveDown()))->setEnabled(moveTimerDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(cmTimerClearAll()));

  contextMenu.exec(globalPos);
}

bool SetupPanel::hasTimerClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_TIMER)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_TIMER));
    return true;
  }
  return false;
}

bool SetupPanel::insertTimerAllowed() const
{
  return ((selectedTimerIndex < timersCount - 1) && (model->timers[timersCount - 1].isEmpty()));
}

bool SetupPanel::moveTimerDownAllowed() const
{
  return selectedTimerIndex < timersCount - 1;
}

bool SetupPanel::moveTimerUpAllowed() const
{
  return selectedTimerIndex > 0;
}

void SetupPanel::cmTimerClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Timer. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->timers[selectedTimerIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_TIMER, ModelData::REF_UPD_ACT_CLEAR, selectedTimerIndex);
  updateItemModels();
  emit modified();
}

void SetupPanel::cmTimerClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Timers. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < timersCount; i++) {
    model->timers[i].clear();
    model->updateAllReferences(ModelData::REF_UPD_TYPE_TIMER, ModelData::REF_UPD_ACT_CLEAR, i);
  }
  updateItemModels();
  emit modified();
}

void SetupPanel::cmTimerCopy()
{
  QByteArray data;
  data.append((char*)&model->timers[selectedTimerIndex], sizeof(TimerData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_TIMER, data);
  QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void SetupPanel::cmTimerCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Timer. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmTimerCopy();
  cmTimerClear(false);
}

void SetupPanel::cmTimerDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Timer. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  int maxidx = timersCount - 1;
  for (int i = selectedTimerIndex; i < maxidx; i++) {
    if (!model->timers[i].isEmpty() || !model->timers[i + 1].isEmpty()) {
      memcpy(&model->timers[i], &model->timers[i+1], sizeof(TimerData));
    }
  }
  model->timers[maxidx].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_TIMER, ModelData::REF_UPD_ACT_SHIFT, selectedTimerIndex, 0, -1);
  updateItemModels();
  emit modified();
}

void SetupPanel::cmTimerInsert()
{
  for (int i = (timersCount - 1); i > selectedTimerIndex; i--) {
    if (!model->timers[i].isEmpty() || !model->timers[i-1].isEmpty()) {
      memcpy(&model->timers[i], &model->timers[i-1], sizeof(TimerData));
    }
  }
  model->timers[selectedTimerIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_TIMER, ModelData::REF_UPD_ACT_SHIFT, selectedTimerIndex, 0, 1);
  updateItemModels();
  emit modified();
}

void SetupPanel::cmTimerMoveDown()
{
  swapTimerData(selectedTimerIndex, selectedTimerIndex + 1);
}

void SetupPanel::cmTimerMoveUp()
{
  swapTimerData(selectedTimerIndex, selectedTimerIndex - 1);
}

void SetupPanel::cmTimerPaste()
{
  QByteArray data;
  if (hasTimerClipboardData(&data)) {
    TimerData *td = &model->timers[selectedTimerIndex];
    memcpy(td, data.constData(), sizeof(TimerData));
    updateItemModels();
    emit modified();
  }
}

void SetupPanel::swapTimerData(int idx1, int idx2)
{
  if ((idx1 != idx2) && (!model->timers[idx1].isEmpty() || !model->timers[idx2].isEmpty())) {
    TimerData tdtmp = model->timers[idx2];
    TimerData *td1 = &model->timers[idx1];
    TimerData *td2 = &model->timers[idx2];
    memcpy(td2, td1, sizeof(TimerData));
    memcpy(td1, &tdtmp, sizeof(TimerData));
    model->updateAllReferences(ModelData::REF_UPD_TYPE_TIMER, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
    updateItemModels();
    emit modified();
  }
}

void SetupPanel::onTimerNameChanged()
{
  updateItemModels();
}

void SetupPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &SetupPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &SetupPanel::onItemModelUpdateComplete);
}

void SetupPanel::onItemModelAboutToBeUpdated()
{
}

void SetupPanel::onItemModelUpdateComplete()
{
}

void SetupPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Timers);
  emit updated();
}

void SetupPanel::onModuleUpdateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Modules);
}

void SetupPanel::onFunctionSwitchesUpdateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_FunctionSwitches);
}
