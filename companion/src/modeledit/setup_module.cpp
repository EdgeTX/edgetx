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

#include "setup_module.h"
#include "ui_setup_module.h"
#include "namevalidator.h"
#include "autolineedit.h"
#include "multiprotocols.h"
#include "helpers.h"

#define FAILSAFE_CHANNEL_HOLD    2000
#define FAILSAFE_CHANNEL_NOPULSE 2001

#define MASK_PROTOCOL              (1<<0)
#define MASK_CHANNELS_COUNT        (1<<1)
#define MASK_RX_NUMBER             (1<<2)
#define MASK_CHANNELS_RANGE        (1<<3)
#define MASK_PPM_FIELDS            (1<<4)
#define MASK_FAILSAFES             (1<<5)
#define MASK_OPEN_DRAIN            (1<<6)
#define MASK_MULTIMODULE           (1<<7)
#define MASK_ANTENNA               (1<<8)
#define MASK_MULTIOPTION           (1<<9)
#define MASK_R9M                   (1<<10)
#define MASK_SBUSPPM_FIELDS        (1<<11)
#define MASK_SUBTYPES              (1<<12)
#define MASK_ACCESS                (1<<13)
#define MASK_RX_FREQ               (1<<14)
#define MASK_RF_POWER              (1<<15)
#define MASK_RF_RACING_MODE        (1<<16)
#define MASK_GHOST                 (1<<17)
#define MASK_BAUDRATE              (1<<18)
#define MASK_MULTI_DSM_OPT         (1<<19)
#define MASK_CHANNELMAP            (1<<20)
#define MASK_MULTI_BAYANG_OPT      (1<<21)
#define MASK_AFHDS                 (1<<22)
#define MASK_CSRF_ARMING_MODE      (1<<23)
#define MASK_CSRF_ARMING_TRIGGER   (1<<24)

quint8 ModulePanel::failsafesValueDisplayType = ModulePanel::FAILSAFE_DISPLAY_PERCENT;

ModulePanel::ModulePanel(QWidget * parent, ModelData & model, ModuleData & module, GeneralSettings & generalSettings, Firmware * firmware, int moduleIdx,
                         FilteredItemModelFactory * panelFilteredItemModels, CompoundItemModelFactory * panelItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  module(module),
  moduleIdx(moduleIdx),
  ui(new Ui::Module),
  trainerModeItemModel(nullptr)
{
  lock = true;

  ui->setupUi(this);

  ui->label_module->setText(ModuleData::indexToString(moduleIdx, firmware));
  if (isTrainerModule(moduleIdx)) {
    ui->formLayout_col1->setSpacing(0);
    if (!IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      ui->label_trainerMode->hide();
      ui->trainerMode->hide();
    }
    else {
      updateTrainerModeItemModel();
      ui->trainerMode->setField(model.trainerMode);
      connect(ui->trainerMode, &AutoComboBox::currentDataChanged, this, [=] () {
        update();
        emit updateItemModels();
        emit modified();
      });
    }
  }
  else {
    ui->label_trainerMode->hide();
    ui->trainerMode->hide();
  }

  if (panelFilteredItemModels) {
    if (!isTrainerModule(moduleIdx)) {
      int id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(ModuleData::protocolItemModel(generalSettings), moduleIdx + 1/*flag cannot be 0*/), QString("Module Protocol %1").arg(moduleIdx));
      panelFilteredItemModels->getItemModel(id)->setSortCaseSensitivity(Qt::CaseInsensitive);
      panelFilteredItemModels->getItemModel(id)->sort(0);
      ui->protocol->setModel(panelFilteredItemModels->getItemModel(id));

      if (ui->protocol->findData(module.protocol) < 0) {
        const QString moduleIdxDesc = isInternalModule(moduleIdx) ? tr("internal") : tr("external");
        const QString compareDesc = isInternalModule(moduleIdx) ? tr("hardware") : tr("profile");
        const QString intModuleDesc = isInternalModule(moduleIdx) ? ModuleData::typeToString(generalSettings.internalModule) : "";
        QString msg = tr("Warning: The %1 module protocol <b>%2</b> is incompatible with the <b>%3 %1 module %4</b> and has been set to <b>OFF</b>!");
        msg = msg.arg(moduleIdxDesc).arg(module.protocolToString(module.protocol)).arg(compareDesc).arg(intModuleDesc);

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

    if (isInternalModule(moduleIdx)) {
      int id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(GeneralSettings::antennaModeItemModel(true)), FIM_ANTENNAMODE);
      ui->antennaMode->setModel(panelFilteredItemModels->getItemModel(id));
    }
  }

  ui->multiProtocol->setModel(Multiprotocols::protocolItemModel());

  ui->btnGrpValueType->setId(ui->optPercent, FAILSAFE_DISPLAY_PERCENT);
  ui->btnGrpValueType->setId(ui->optUs, FAILSAFE_DISPLAY_USEC);
  ui->btnGrpValueType->button(failsafesValueDisplayType)->setChecked(true);

  ui->registrationId->setText(model.registrationId);

  if (panelItemModels) {
    ui->crsfArmingMode->setModel(panelItemModels->getItemModel(AIM_MODULE_CRSFARMINGMODE));
    ui->crsfArmingTrigger->setModel(panelFilteredItemModels->getItemModel(FIM_CRSFARMSWITCH));
  }

  setupFailsafes();

  disableMouseScrolling();

  update();

  connect(ui->protocol, &AutoComboBox::currentDataChanged, this, &ModulePanel::onProtocolChanged);
  connect(ui->multiSubType, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ModulePanel::onSubTypeChanged);
  connect(ui->multiProtocol, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ModulePanel::onMultiProtocolChanged);
  connect(this, &ModulePanel::channelsRangeChanged, this, &ModulePanel::setupFailsafes);
  connect(ui->btnGrpValueType, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::idClicked), this, &ModulePanel::onFailsafesDisplayValueTypeChanged);
  connect(ui->rxFreq, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &ModulePanel::onRfFreqChanged);
  connect(ui->clearRx1, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));
  connect(ui->clearRx2, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));
  connect(ui->clearRx3, SIGNAL(clicked()), this, SLOT(onClearAccessRxClicked()));
  connect(ui->cboAfhdsOpt1, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] (int index)
    {
      if (lock)
        return;

      if (this->module.protocol == PULSES_FLYSKY_AFHDS2A)
        Helpers::setBitmappedValue(this->module.flysky.mode, ui->cboAfhdsOpt1->currentData().toInt(), 1);
      else
        this->module.afhds3.phyMode = ui->cboAfhdsOpt1->currentData().toInt();

      emit modified();
    });
  connect(ui->cboAfhdsOpt2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] (int index)
    {
      if (lock)
        return;

      if (this->module.protocol == PULSES_FLYSKY_AFHDS2A)
        Helpers::setBitmappedValue(this->module.flysky.mode, ui->cboAfhdsOpt2->currentData().toInt(), 0);
      else
        this->module.afhds3.emi = ui->cboAfhdsOpt2->currentData().toInt();

      emit modified();
    });

  connect(ui->crsfArmingMode, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] (int index)
    {
      this->module.crsf.crsfArmingMode = ui->crsfArmingMode->currentData().toInt();
      if (this->module.crsf.crsfArmingMode != ModuleData::CRSF_ARMING_MODE_SWITCH)
        this->module.crsf.crsfArmingTrigger = RawSwitch(SWITCH_TYPE_NONE);
      update();
      emit modified();
    });

  connect(ui->crsfArmingTrigger, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] (int index)
    {
      this->module.crsf.crsfArmingTrigger = RawSwitch(ui->crsfArmingTrigger->currentData().toInt());
      emit modified();
    });

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

  if (!isTrainerModule(moduleIdx)) {
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
        if (isInternalModule(moduleIdx) &&
            (protocol==PULSES_PXX_XJT_X16 ||
             protocol==PULSES_PXX_XJT_D8 || protocol==PULSES_PXX_XJT_LR12) &&
            HAS_EXTERNAL_ANTENNA(board) && generalSettings.antennaMode == GeneralSettings::ANTENNA_MODE_PER_MODEL)
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
        mask |= MASK_CHANNELS_RANGE | MASK_RX_NUMBER | MASK_BAUDRATE | MASK_CSRF_ARMING_MODE;
        module.channelsCount = 16;
        ui->telemetryBaudrate->setModel(ModuleData::telemetryBaudrateItemModel(protocol));
        ui->telemetryBaudrate->setField(module.crsf.telemetryBaudrate);
        ui->crsfArmingMode->setCurrentIndex(module.crsf.crsfArmingMode);
        if (module.crsf.crsfArmingMode == ModuleData::CRSF_ARMING_MODE_SWITCH) {
          mask |= MASK_CSRF_ARMING_TRIGGER;
          ui->crsfArmingTrigger->setCurrentIndex(ui->crsfArmingTrigger->findData(RawSwitch(module.crsf.crsfArmingTrigger).toValue()));
        }
        break;
      case PULSES_GHOST:
        mask |= MASK_CHANNELS_RANGE | MASK_GHOST | MASK_BAUDRATE;
        module.channelsCount = 16;
        ui->telemetryBaudrate->setModel(ModuleData::telemetryBaudrateItemModel(protocol));
        ui->telemetryBaudrate->setField(module.ghost.telemetryBaudrate);
        break;
      case PULSES_PPM:
        mask |= MASK_SUBTYPES | MASK_PPM_FIELDS | MASK_SBUSPPM_FIELDS| MASK_CHANNELS_RANGE| MASK_CHANNELS_COUNT;
        if (IS_9XRPRO(board)) {
          mask |= MASK_OPEN_DRAIN;
        }
        break;
      case PULSES_SBUS:
        module.channelsCount = 16;
        mask |=  MASK_SUBTYPES | MASK_SBUSPPM_FIELDS| MASK_CHANNELS_RANGE;
        break;
      case PULSES_MULTIMODULE:
        mask |= MASK_CHANNELS_RANGE | MASK_RX_NUMBER | MASK_MULTIMODULE | MASK_SUBTYPES;

        switch (module.multi.rfProtocol) {
          case MODULE_SUBTYPE_MULTI_OLRS:
            max_rx_num = MODULE_SUBTYPE_MULTI_OLRS_RXNUM;
          case MODULE_SUBTYPE_MULTI_BUGS:
            max_rx_num = MODULE_SUBTYPE_MULTI_BUGS_RXNUM;
          case MODULE_SUBTYPE_MULTI_BUGS_MINI:
            max_rx_num = MODULE_SUBTYPE_MULTI_BUGS_MINI_RXNUM;
          default:
             max_rx_num = 63;
        }

        if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
          mask |= MASK_CHANNELS_COUNT;
        else
          module.channelsCount = 16;
        if (pdef.optionsstr != nullptr) {
          if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2)
            mask |= MASK_MULTI_DSM_OPT;
          else if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_BAYANG)
            mask |= MASK_MULTI_BAYANG_OPT;
          else
            mask |= MASK_MULTIOPTION;
        }
        if (pdef.hasFailsafe || (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_FRSKY && (module.subType == 0 || module.subType == 2 || module.subType > 3 )))
          mask |= MASK_FAILSAFES;
        if (pdef.disableChannelMap)
          mask |= MASK_CHANNELMAP;
        break;
      case PULSES_FLYSKY_AFHDS3:
        mask |= MASK_RX_NUMBER;
      case PULSES_FLYSKY_AFHDS2A:
        mask |= MASK_CHANNELS_RANGE| MASK_CHANNELS_COUNT | MASK_FAILSAFES | MASK_AFHDS;
        break;
      case PULSES_LEMON_DSMP:
        mask |= MASK_CHANNELS_RANGE;
        break;
      default:
        break;
    }

    if (protocol != PULSES_MULTIMODULE && module.hasFailsafes(firmware))
      mask |= MASK_FAILSAFES;
  }
  else if (IS_HORUS_OR_TARANIS(board)) {
    if (model->trainerMode == TRAINER_MODE_SLAVE_JACK) {
      mask |= MASK_PPM_FIELDS | MASK_SBUSPPM_FIELDS | MASK_CHANNELS_RANGE | MASK_CHANNELS_COUNT;
    }
  }
  else if (model->trainerMode != TRAINER_MODE_MASTER_JACK) {
    mask |= MASK_PPM_FIELDS | MASK_CHANNELS_RANGE | MASK_CHANNELS_COUNT;
  }

  if (isExternalModule(moduleIdx))
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
  ui->label_crsfArmingMode->setVisible(mask & MASK_CSRF_ARMING_MODE);
  ui->crsfArmingMode->setVisible(mask & MASK_CSRF_ARMING_MODE);
  ui->crsfArmingTrigger->setVisible(mask & MASK_CSRF_ARMING_TRIGGER);

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

  if (mask & MASK_ANTENNA) {
    if (module.pxx.antennaMode == GeneralSettings::ANTENNA_MODE_PER_MODEL)
      module.pxx.antennaMode = GeneralSettings::ANTENNA_MODE_INTERNAL;
    ui->antennaMode->setField(module.pxx.antennaMode, this);
    ui->antennaLabel->show();
    ui->antennaMode->show();
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
    case PULSES_PPM:
        numEntries = PPM_NUM_SUBTYPES;
        break;
    case PULSES_SBUS:
        numEntries = SBUS_NUM_SUBTYPES;
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
  ui->lblChkOption->setVisible(mask & MASK_MULTI_DSM_OPT);
  ui->chkOption->setVisible(mask & MASK_MULTI_DSM_OPT);
  ui->lblCboOption->setVisible(mask & MASK_MULTI_DSM_OPT || mask & MASK_MULTI_BAYANG_OPT);
  ui->cboOption->setVisible(mask & MASK_MULTI_DSM_OPT || mask & MASK_MULTI_BAYANG_OPT);
  ui->disableTelem->setVisible(mask & MASK_MULTIMODULE);
  ui->disableChMap->setVisible(mask & MASK_CHANNELMAP);
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
    ui->label_option->setText(qApp->translate("Multiprotocols", qPrintable(pdef.optionsstr)));
    int8_t min, max;
    getMultiOptionValues(module.multi.rfProtocol, min, max);

    auto lineEdit = ui->optionValue->findChild<QLineEdit*>();

    if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
      ui->optionValue->setMinimum(50 + 5 * min);
      ui->optionValue->setMaximum(50 + 5 * max);
      ui->optionValue->setSingleStep(5);
      ui->optionValue->setValue(50 + 5 * module.multi.optionValue);
      if (lineEdit) {
        lineEdit->setReadOnly(true);
        lineEdit->setFocusPolicy(Qt::NoFocus);
      }
    }
    else {
      ui->optionValue->setMinimum(min);
      ui->optionValue->setMaximum(max);
      ui->optionValue->setSingleStep(1);
      ui->optionValue->setValue(module.multi.optionValue);
      if (lineEdit) {
        lineEdit->setReadOnly(false);
        lineEdit->setFocusPolicy(Qt::WheelFocus);
      }
    }
  }

  if (mask & MASK_MULTI_DSM_OPT) {
    ui->lblChkOption->setText(qApp->translate("Multiprotocols", qPrintable(pdef.optionsstr)));
    ui->chkOption->setChecked(Helpers::getBitmappedValue(module.multi.optionValue, 0));
    ui->lblCboOption->setText(qApp->translate("Multiprotocols", "Servo update rate"));
    ui->cboOption->clear();
    ui->cboOption->addItems({ DSM2_OPTION_SERVOFREQ_NAMES });
    ui->cboOption->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui->cboOption->setCurrentIndex(Helpers::getBitmappedValue(module.multi.optionValue, 1));
  }

  if (mask & MASK_MULTI_BAYANG_OPT) {
    ui->lblCboOption->setText(qApp->translate("Multiprotocols", qPrintable(pdef.optionsstr)));
    ui->cboOption->clear();
    ui->cboOption->addItems({ BAYANG_OPTION_TELEMETRY_NAMES });
    ui->cboOption->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui->cboOption->setCurrentIndex(Helpers::getBitmappedValue(module.multi.optionValue, 1));
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

  // AFHFS
  if (mask & MASK_AFHDS) {
    if (protocol == PULSES_FLYSKY_AFHDS2A) {
      ui->label_afhds->setText(tr("Options"));
      ui->cboAfhdsOpt1->setModel(ModuleData::afhds2aMode1ItemModel());
      ui->cboAfhdsOpt1->setCurrentIndex(Helpers::getBitmappedValue(module.flysky.mode, 1));

      ui->cboAfhdsOpt2->setModel(ModuleData::afhds2aMode2ItemModel());
      ui->cboAfhdsOpt2->setCurrentIndex(Helpers::getBitmappedValue(module.flysky.mode, 0));
    }
    else {
      ui->label_afhds->setText(tr("Type"));
      ui->cboAfhdsOpt1->setModel(ModuleData::afhds3PhyModeItemModel());
      ui->cboAfhdsOpt1->setCurrentIndex(ui->cboAfhdsOpt1->findData(module.afhds3.phyMode));

      ui->cboAfhdsOpt2->setModel(ModuleData::afhds3EmiItemModel());
      ui->cboAfhdsOpt2->setCurrentIndex(ui->cboAfhdsOpt2->findData(module.afhds3.emi));
    }
  }

  ui->label_afhds->setVisible(mask & MASK_AFHDS);
  ui->cboAfhdsOpt1->setVisible(mask & MASK_AFHDS);
  ui->cboAfhdsOpt2->setVisible(mask & MASK_AFHDS);

  // Failsafes
  ui->label_failsafeMode->setVisible(mask & MASK_FAILSAFES);
  ui->failsafeMode->setVisible(mask & MASK_FAILSAFES);

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
    else if (module.protocol == PULSES_FLYSKY_AFHDS2A) {
      module.flysky.setDefault();
    }
    else if (module.protocol == PULSES_FLYSKY_AFHDS3) {
      module.afhds3.setDefault();
    }

    emit protocolChanged();
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

    if (module.protocol == PULSES_FLYSKY_AFHDS2A && module.flysky.rfPower != (unsigned int)index) {
      module.flysky.rfPower = index;
      emit modified();
    }
    else if (module.protocol == PULSES_FLYSKY_AFHDS3 && module.afhds3.rfPower != (unsigned int)index) {
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
    module.multi.optionValue = 0;
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

void ModulePanel::on_optionValue_valueChanged(int value)
{
  if (!lock) {
    if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_FS_AFHDS2A) {
      module.multi.optionValue = (value - 50) / 5;
    }
    else {
      module.multi.optionValue = value;
    }
    emit modified();
  }
}

void ModulePanel::on_chkOption_stateChanged(int state)
{
  if (!lock) {
    if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2) {
      unsigned int opt = (unsigned int)module.multi.optionValue;
      if (Helpers::getBitmappedValue(opt, 0) != (state == Qt::Checked)) {
        Helpers::setBitmappedValue(opt, (state == Qt::Checked), 0);
        module.multi.optionValue = opt;
        emit modified();
      }
    }
    else if (module.multi.optionValue != (state == Qt::Checked)) {
      module.multi.optionValue = (state == Qt::Checked);
      emit modified();
    }
  }
}

void ModulePanel::on_cboOption_currentIndexChanged(int value)
{
  if (!lock && value >= 0) {
    if (module.multi.rfProtocol == MODULE_SUBTYPE_MULTI_DSM2) {
      unsigned int opt = (unsigned int)module.multi.optionValue;
      if (Helpers::getBitmappedValue(opt, 1) != (unsigned int)value) {
        Helpers::setBitmappedValue(opt, value, 1);
        module.multi.optionValue = opt;
        emit modified();
      }
    }
    else if (module.multi.optionValue != value) {
      module.multi.optionValue = value;
      emit modified();
    }
  }
}

void ModulePanel::onSubTypeChanged()
{
  const unsigned type = ui->multiSubType->currentData().toUInt();
  if (!lock && module.subType != type) {
    lock=true;
    module.subType = type;
    emit modified();
    lock =  false;
  }
}

void ModulePanel::onRfFreqChanged(int freq) {
  //  TODO fix for AFHDS2A
  //if (module.afhds3.rxFreq != (unsigned int)freq) {
  //  module.afhds3.rxFreq = (unsigned int)freq;
  //  emit modified();
  //}
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

void ModulePanel::updateTrainerModeItemModel()
{
  if (isTrainerModule(moduleIdx)) {
    if (trainerModeItemModel)
      delete trainerModeItemModel;

    trainerModeItemModel = new FilteredItemModel(model->trainerModeItemModel(generalSettings, firmware));
    ui->trainerMode->setModel(trainerModeItemModel);
    ui->trainerMode->updateValue();
  }
}
