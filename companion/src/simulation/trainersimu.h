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

#pragma once

#include <QShowEvent>
#include <QCloseEvent>
#include <QWidget>
#include <QTimer>
#include "radiowidget.h"
#include "simulatorinterface.h"

namespace Ui {
  class TrainerSimulator;
}

class VirtualJoystickWidget;

class TrainerSimulator : public QWidget
{
  Q_OBJECT

  public:
    explicit TrainerSimulator(QWidget * parent, SimulatorInterface * simulator);
    virtual ~TrainerSimulator();

  signals:
    void trainerHeartbeat(quint16 ms);
    void trainerChannelChange(quint8 index, qint16 value);

  protected slots:
    void start();
    void stop();
    virtual void showEvent(QShowEvent *event);
    virtual void hideEvent(QHideEvent *event);
    void onSimulatorStarted();
    void onSimulatorStopped();
    void onRadioWidgetValueChange(RadioWidget::RadioWidgetType type, int index, int value);
    void emitHeartbeat();

  protected:
    Ui::TrainerSimulator * ui;
    SimulatorInterface *simulator;
    VirtualJoystickWidget * vJoyLeft;
    VirtualJoystickWidget * vJoyRight;
    QTimer timer;
    bool m_simuStarted;

};
