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

#include <QObject>
#include <QMap>
#include <QElapsedTimer>
#include <QTimer>
#include <QStringList>
#include <SDL.h>

#define SDL_JOYSTICK_DEFAULT_EVENT_TIMEOUT         25
#define SDL_JOYSTICK_DEFAULT_AUTOREPEAT_DELAY      250

class Joystick : public QObject
{
  Q_OBJECT

  public:
    QStringList joystickNames;
    SDL_Joystick *joystick;
    int numAxes;
    int numButtons;
    int numHats;
    int numTrackballs;
    int eventTimeout;
    int autoRepeatDelay;
    bool autoRepeat;
    QTimer joystickTimer;
    QMap<int, int> deadzones;
    QMap<int, int> sensitivities;

    explicit Joystick(QObject * parent = nullptr,
                      int joystickEventTimeout = SDL_JOYSTICK_DEFAULT_EVENT_TIMEOUT,
                      bool doAutoRepeat = true,
                      int autoRepeatDelay = SDL_JOYSTICK_DEFAULT_AUTOREPEAT_DELAY);
    ~Joystick() override;
    bool open(int);
    void close();
    bool isOpen()
    {
      return joystick != nullptr;
    }
    int getAxisValue(int);

    int findCurrent(QString jsName);

  private:
    QMap<int, Sint16> axes;
    QMap<int, Uint8> buttons;
    QMap<int, Uint8> hats;
    QMap<int, QElapsedTimer> axisRepeatTimers;
    QMap<int, QElapsedTimer> buttonRepeatTimers;
    QMap<int, QElapsedTimer> hatRepeatTimers;

  signals:
    void axisValueChanged(int axis, int value);
    void buttonValueChanged(int button, bool value);
    void hatValueChanged(int hat, int value);
    void trackballValueChanged(int trackball, int deltaX, int deltaY);

  public slots:
    void processEvents();
};
