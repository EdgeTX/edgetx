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

#include <stdint.h>
#include "trainersimu.h"
#include "ui_trainersimu.h"
#include "helpers.h"
#include "virtualjoystickwidget.h"

#define TRAINERSIMU_HEARTBEAT_PERIOD    500  // [ms]

TrainerSimulator::TrainerSimulator(QWidget * parent, SimulatorInterface * simulator):
  QWidget(parent),
  ui(new Ui::TrainerSimulator),
  simulator(simulator),
  m_simuStarted(false)
{
  ui->setupUi(this);

  vJoyLeft = new VirtualJoystickWidget(this, 'L', false);
  vJoyLeft->setStickColor(Qt::cyan);
  vJoyLeft->setStickScale(512);
  ui->leftStickLayout->addWidget(vJoyLeft);

  vJoyRight = new VirtualJoystickWidget(this, 'R', false);
  vJoyRight->setStickColor(Qt::cyan);
  vJoyRight->setStickScale(512);
  ui->rightStickLayout->addWidget(vJoyRight);

  connect(vJoyLeft, &VirtualJoystickWidget::valueChange, this, &TrainerSimulator::onRadioWidgetValueChange);
  connect(vJoyRight, &VirtualJoystickWidget::valueChange, this, &TrainerSimulator::onRadioWidgetValueChange);

  connect(this, &TrainerSimulator::trainerHeartbeat, simulator, &SimulatorInterface::setTrainerTimeout);
  connect(this, &TrainerSimulator::trainerChannelChange, simulator, &SimulatorInterface::setTrainerInput);
  connect(simulator, &SimulatorInterface::started, this, &TrainerSimulator::onSimulatorStarted);
  connect(simulator, &SimulatorInterface::stopped, this, &TrainerSimulator::onSimulatorStopped);

  timer.setInterval(TRAINERSIMU_HEARTBEAT_PERIOD - 10);
  connect(&timer, &QTimer::timeout, this, &TrainerSimulator::emitHeartbeat);
}

TrainerSimulator::~TrainerSimulator()
{
  if (vJoyLeft)
    delete vJoyLeft;
  if (vJoyRight)
    delete vJoyRight;

  delete ui;
}

void TrainerSimulator::showEvent(QShowEvent *event)
{
  start();
}

void TrainerSimulator::hideEvent(QHideEvent *event)
{
  stop();
}

void TrainerSimulator::start()
{
  timer.start();
}

void TrainerSimulator::stop()
{
  timer.stop();
}

void TrainerSimulator::onSimulatorStarted()
{
  m_simuStarted = true;
}

void TrainerSimulator::onSimulatorStopped()
{
  m_simuStarted = false;
  stop();
}

void TrainerSimulator::emitHeartbeat()
{
  emit trainerHeartbeat(TRAINERSIMU_HEARTBEAT_PERIOD);
}

void TrainerSimulator::onRadioWidgetValueChange(RadioWidget::RadioWidgetType type, int index, int value)
{
  if (type == RadioWidget::RADIO_WIDGET_STICK && m_simuStarted && timer.isActive()) {
    emit trainerChannelChange(index, value);
    emitHeartbeat();
  }
}
