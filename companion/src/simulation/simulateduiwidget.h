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

#include "boards.h"
#include "constants.h"
#include "radiowidget.h"
#include "simulator.h"
#include "simulator_strings.h"

#include <QWidget>
#include <QMouseEvent>

class SimulatorInterface;
class LcdWidget;
class RadioKeyWidget;
class RadioUiAction;
class ButtonsWidget;
class QPushButton;

// Match with /radio/src/hal/key_driver.h
// key in sync with strKeyToInt
enum EnumKeys {
  KEY_MENU,
  KEY_EXIT,
  KEY_ENTER,

  KEY_PAGEUP,
  KEY_PAGEDN,

  KEY_UP,
  KEY_DOWN,

  KEY_LEFT,
  KEY_RIGHT,

  KEY_PLUS,
  KEY_MINUS,

  KEY_MODEL,
  KEY_TELE,
  KEY_SYS,

  KEY_SHIFT,
  KEY_BIND,

  //  these are only used by the simulator
  KEY_SCRLUP,
  KEY_SCRLDN
};

struct RadioKeyDefinition {
  int index = 0;
  QChar side = 'L';
  int gridRow = 0;
  QList<int> keys = QList<int>();
  QString helpKeys = "";
  QString helpActions = "";

  RadioKeyDefinition(int index, QChar side, int gridRow,
                     QList<int> keys, QString helpKeys, QString helpActions) :
                     index(index), side(side), gridRow(gridRow),
                     keys(keys), helpKeys(helpKeys), helpActions(helpActions) {}

  RadioKeyDefinition() = default;
};

/*
 * This is a base class for the main hardware-specific radio user interface, including LCD screen and navigation buttons/widgets.
 * It is responsible for hanlding all interactions with this part of the simulation (vs. common radio widgets like sticks/switches/knobs).
 * Sub-classes are responsible for building the actual UI form they need for presentation.
 * This base class should not be instantiated directly.
 */
class SimulatedUIWidget : public QWidget
{
  Q_OBJECT

  protected:

    explicit SimulatedUIWidget(SimulatorInterface * simulator, QWidget * parent = nullptr);

  public:

    ~SimulatedUIWidget();

    RadioWidget * addRadioWidget(RadioWidget * keyWidget);
    RadioUiAction * addRadioAction(RadioUiAction * act);

    QVector<Simulator::keymapHelp_t> getKeymapHelp() const;

    static QPolygon polyArc(int ctrX, int ctrY, int radius, int startAngle = 0, int endAngle = 360, int step = 10);

  public slots:

    void captureScreenshot();

    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    virtual void shrink() { };

  signals:

    void controlValueChange(RadioWidget::RadioWidgetType type, int index, int value);
    void customStyleRequest(const QString & style);
    void simulatorWheelEvent(qint8 steps);
    void resizeRequest();

  protected slots:

    void setLcd(LcdWidget * lcd);
    void connectScrollActions();
    void onLcdChange(bool backlightEnable);
    virtual void setLightOn(bool enable) { }

  protected:

    SimulatorInterface * m_simulator;
    QWidget * m_parent;
    LcdWidget * m_lcd;
    QVector<QColor> m_backlightColors;
    QList<RadioUiAction *> m_actions;
    QList<RadioWidget *> m_widgets;
    RadioUiAction * m_scrollUpAction;
    RadioUiAction * m_scrollDnAction;
    RadioUiAction * m_mouseMidClickAction;
    RadioUiAction * m_screenshotAction;
    Board::Type m_board;
    unsigned int m_backLight;
    int m_beepShow;
    int m_beepVal;

    static int strKeyToInt(std::string key);

    void addPushButton(int index, QString label, ButtonsWidget * leftButtons, QGridLayout * leftButtonsGrid,
                       ButtonsWidget * rightButtons, QGridLayout * rightButtonsGrid);
    void addPushButtons(ButtonsWidget * leftButtons, ButtonsWidget * rightButtons);
    void addScrollActions();
    void addMouseActions();
    const RadioKeyDefinition * getRadioKeyDefinition(const int key) const;
};
