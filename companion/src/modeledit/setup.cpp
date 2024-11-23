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

#include "setup.h"
#include "ui_setup.h"
#include "appdata.h"
#include "checklistdialog.h"
#include "helpers.h"
#include "namevalidator.h"
#include "autolineedit.h"

#include <QDir>

constexpr char FIM_THRSOURCE[]   {"Throttle Source"};
// constexpr char FIM_TRAINERMODE[] {"Trainer Mode"};
constexpr char FIM_HATSMODE[]    {"Hats Mode"};

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

  panelFilteredModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSwitch),
                                                               RawSwitch::SpecialFunctionsContext),
                                         FIM_CRSFARMSWITCH);
  connectItemModelEvents(panelFilteredModels->getItemModel(FIM_CRSFARMSWITCH));

  panelFilteredModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_ThrSource)),
                                         FIM_THRSOURCE);
  connectItemModelEvents(panelFilteredModels->getItemModel(FIM_THRSOURCE));

  panelFilteredModels->registerItemModel(new FilteredItemModel(GeneralSettings::hatsModeItemModel(false)), FIM_HATSMODE);

  panelItemModels = new CompoundItemModelFactory(&generalSettings, &model);
  panelItemModels->registerItemModel(TimerData::countdownBeepItemModel());
  panelItemModels->registerItemModel(TimerData::countdownStartItemModel());
  panelItemModels->registerItemModel(TimerData::persistentItemModel());
  panelItemModels->registerItemModel(TimerData::modeItemModel());
  panelItemModels->registerItemModel(TimerData::showElapsedItemModel());
  panelItemModels->registerItemModel(ModuleData::crsfArmingModeItemModel());

  Board::Type board = firmware->getBoard();

  memset(modules, 0, sizeof(modules));

  ui->name->setValidator(new NameValidator(board, this));
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
      connect(timers[i], &TimerPanel::nameChanged, this, &SetupPanel::onTimerChanged);
      connect(timers[i], &TimerPanel::modeChanged, this, &SetupPanel::onTimerChanged);
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

  const int ttlSticks = Boards::getBoardCapability(board, Board::Sticks);
  const int ttlFlexInputs = Boards::getBoardCapability(board, Board::FlexInputs);
  const int ttlInputs = ttlSticks + ttlFlexInputs;

  for (int i = 0; i < ttlInputs + firmware->getCapability(RotaryEncoders); i++) {
    RawSource src((i < ttlInputs) ? SOURCE_TYPE_INPUT : SOURCE_TYPE_ROTARY_ENCODER, (i < ttlInputs) ? i + 1 : i - ttlInputs);
    QCheckBox * checkbox = new QCheckBox(this);
    checkbox->setProperty("index", i);
    checkbox->setText(src.toString(&model, &generalSettings));
    ui->centerBeepLayout->addWidget(checkbox, 0, i + 1);
    connect(checkbox, SIGNAL(toggled(bool)), this, SLOT(onBeepCenterToggled(bool)));
    centerBeepCheckboxes << checkbox;
    if (IS_HORUS_OR_TARANIS(board)) {
      if (!(generalSettings.isInputAvailable(i) &&
            (generalSettings.isInputStick(i) || (generalSettings.isInputPot(i) && !generalSettings.isInputMultiPosPot(i)) ||
             generalSettings.isInputSlider(i))))
        checkbox->hide();
    }
    QWidget::setTabOrder(prevFocus, checkbox);
    prevFocus = checkbox;
  }

  // Startup switches warnings
  for (int i = 0; i < Boards::getBoardCapability(board, Board::Switches); i++) {
    GeneralSettings::SwitchConfig &swcfg = generalSettings.switchConfig[i];

    if (Boards::isSwitchFunc(i, board) || !generalSettings.isSwitchAvailable(i) || swcfg.type == Board::SWITCH_TOGGLE) {
      model.switchWarningEnable |= (1 << i);
      continue;
    }

    RawSource src(RawSourceType::SOURCE_TYPE_SWITCH, i + 1);
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
    slider->setMaximum(swcfg.type == Board::SWITCH_3POS ? 2 : 1);
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

  if (IS_HORUS_OR_TARANIS(board) && ttlInputs > 0) {
    for (int i = ttlSticks; i < ttlInputs; i++) {
      RawSource src(SOURCE_TYPE_INPUT, i + 1);
      QCheckBox * cb = new QCheckBox(this);
      cb->setProperty("index", i - ttlSticks);
      cb->setText(src.toString(&model, &generalSettings));
      ui->potWarningLayout->addWidget(cb, 0, i - ttlSticks + 1);
      connect(cb, SIGNAL(toggled(bool)), this, SLOT(potWarningToggled(bool)));
      potWarningCheckboxes << cb;
      if (!(generalSettings.isInputAvailable(i) && (generalSettings.isInputPot(i) || generalSettings.isInputSlider(i))))
        cb->hide();
      QWidget::setTabOrder(prevFocus, cb);
      prevFocus = cb;
    }
  }
  else {
    ui->label_potWarning->hide();
    ui->potWarningMode->hide();
  }

  ui->trimsDisplay->setField(model.trimsDisplay, this);

  if (IS_FLYSKY_EL18(board) || IS_FLYSKY_NV14(board) || IS_FLYSKY_PL18(board)) {
    ui->cboHatsMode->setModel(panelFilteredModels->getItemModel(FIM_HATSMODE));
    ui->cboHatsMode->setField(model.hatsMode, this);
  }
  else {
    ui->lblHatsMode->hide();
    ui->cboHatsMode->hide();
  }

  if (Boards::getCapability(firmware->getBoard(), Board::FunctionSwitches) > 0) {
    funcswitches = new FunctionSwitchesPanel(this, model, generalSettings, firmware);
    ui->functionSwitchesLayout->addWidget(funcswitches);
    connect(funcswitches, &FunctionSwitchesPanel::modified, this, &SetupPanel::modified);
    connect(funcswitches, &FunctionSwitchesPanel::updateDataModels, this, &SetupPanel::onFunctionSwitchesUpdateItemModels);
  }

  for (int i = firmware->getCapability(NumFirstUsableModule); i < firmware->getCapability(NumModules); i++) {
    modules[i] = new ModulePanel(this, model, model.moduleData[i], generalSettings, firmware, i, panelFilteredModels, panelItemModels);
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
    connect(modules[CPN_MAX_MODULES], &ModulePanel::updateItemModels, this, &SetupPanel::onModuleUpdateItemModels);
    for (int i = firmware->getCapability(NumFirstUsableModule); i < firmware->getCapability(NumModules); i++) {
      connect(modules[i], &ModulePanel::protocolChanged, modules[CPN_MAX_MODULES], &ModulePanel::updateTrainerModeItemModel);
    }
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
  emit throttleReverseChanged();
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
  bool isBoardSurface = Boards::isSurface(board);
  lock = true;
  ui->throttleTrimSwitch->clear();
  int idx = 0;
  QString trim;
  for (int i = 0; i < getBoardCapability(board, Board::NumTrims); i++, idx++) {
    if (isBoardSurface) {
      if (i == 0)
        trim = RawSource(SOURCE_TYPE_TRIM, 1 + 1).toString(model, &generalSettings);
      else if (i == 1)
        trim = RawSource(SOURCE_TYPE_TRIM, 0 + 1).toString(model, &generalSettings);
      else
        trim = RawSource(SOURCE_TYPE_TRIM, i + 1).toString(model, &generalSettings);
    }
    else {
      // here order is TERA instead of RETA
      if (i == 0)
        trim = RawSource(SOURCE_TYPE_TRIM, 2 + 1).toString(model, &generalSettings);
      else if (i == 2)
        trim = RawSource(SOURCE_TYPE_TRIM, 0 + 1).toString(model, &generalSettings);
      else
        trim = RawSource(SOURCE_TYPE_TRIM, i + 1).toString(model, &generalSettings);
    }
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
  ui->checklistInteractive->setChecked(model->checklistInteractive);
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
      if (generalSettings.switchConfig[index].type != Board::SWITCH_3POS && value == 2) {
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

    if (IS_HORUS_OR_TARANIS(firmware->getBoard()) && generalSettings.switchConfig[index].type != Board::SWITCH_3POS) {
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

void SetupPanel::on_checklistInteractive_toggled(bool checked)
{
  model->checklistInteractive = checked;
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

void SetupPanel::onTimerChanged()
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
