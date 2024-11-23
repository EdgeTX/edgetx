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

#include "setup_timer.h"
#include "ui_setup_timer.h"
#include "namevalidator.h"

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
    ui->name->setValidator(new NameValidator(firmware->getBoard(), this));
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
  emit modeChanged();
}
