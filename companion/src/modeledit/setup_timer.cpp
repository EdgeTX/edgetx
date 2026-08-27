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

TimerPanel::TimerPanel(QWidget * parent, ModelData & model, TimerData & timer,
                       GeneralSettings & generalSettings, Firmware * firmware,
                       QWidget * prevFocus,
                       FilteredItemModelFactory * panelFilteredModels,
                       CompoundItemModelFactory * panelItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  timer(timer),
  ui(new Ui::Timer)
{
  ui->setupUi(this);
  lock = true;
  imEventHandler = new ItemModelEventHandler(
    panelFilteredModels->getItemModel(FIM_TIMERSWITCH),
    lock,
    [this] { this->update(); }
  );

  // Name
  ui->name->setValidator(new NameValidator(firmware->getBoard(), this));
  ui->name->setField(timer.name, firmware->getCapability(TimersName), this);
  ui->name->setBindPostChanged([this] {
    emit this->nameChanged();
  });

  // Start
  ui->value->setField(timer.val, this);
  ui->value->setMaximumTime(firmware->getMaxTimerStart());
  ui->value->setBindPostChanged([this]{
    this->update();
  });

  // Mode
  ui->mode->setModel(panelItemModels->getItemModel(AIM_TIMER_MODE));
  ui->mode->setField(timer.mode, this);
  ui->mode->setBindPostChanged([this] {
    this->timer.modeChanged();
    this->update();
    emit this->modeChanged();
  });

  // Switch
  ui->swtch->setModel(panelFilteredModels->getItemModel(FIM_TIMERSWITCH));
  ui->swtch->setField(timer.swtch, this);
  ui->swtch->setBindEnabled([this] {
    return this->timer.mode != TimerData::TIMERMODE_OFF;
  });

  // Countdown
  ui->countdownBeep->setModel(panelItemModels->getItemModel(AIM_TIMER_COUNTDOWNBEEP));
  ui->countdownBeep->setField(timer.countdownBeep, this);
  ui->countdownBeep->setBindPostChanged([this] {
    this->timer.countdownBeepChanged();
    this->update();
  });
  ui->countdownStart->setModel(panelItemModels->getItemModel(AIM_TIMER_COUNTDOWNSTART));
  ui->countdownStart->setField(timer.countdownStart, this);
  ui->countdownStart->setBindEnabled([this] {
    return this->timer.countdownBeep != TimerData::COUNTDOWNBEEP_SILENT;
  });
  ui->countdownStart->addBuddyWidget(ui->countdownStartLabel);

  // Minute call
  ui->minuteBeep->setField(timer.minuteBeep, this);

  // Persistence
  ui->persistent->setModel(panelItemModels->getItemModel(AIM_TIMER_PERSISTENT));
  ui->persistent->setField(timer.persistent, this);

  // Time type
  ui->showElapsed->setModel(panelItemModels->getItemModel(AIM_TIMER_SHOWELAPSED));
  ui->showElapsed->setField(timer.showElapsed, this);
  ui->showElapsed->setBindEnabled([this] { return this->timer.val; });

  // Time
  // do not use setField as the raw value is meaningless
  ui->persistentValue->setBindText([this] { return this->timer.pvalueToString(); });

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
}

TimerPanel::~TimerPanel()
{
  delete ui;
  delete imEventHandler;
}

void TimerPanel::update()
{
  lock = true;
  updateAutoWidgets();
  lock = false;
}

QWidget * TimerPanel::getLastFocus()
{
  return ui->persistent;
}
