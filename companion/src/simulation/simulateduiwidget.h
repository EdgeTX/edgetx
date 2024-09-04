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

  MAX_KEYS
};

struct GenericKeyDefinition {
  int index = 0;
  QChar side = 'L';
  int gridRow = 0;
  int gridCol = 0;
  QList<int> keys = QList<int>();
  QString helpKeys = "";
  QString helpActions = "";

  GenericKeyDefinition(int index, QChar side, int gridRow, int gridCol,
                       QList<int> keys, QString helpKeys, QString helpActions) :
                       index(index), side(side), gridRow(gridRow), gridCol(gridCol),
                       keys(keys), helpKeys(helpKeys), helpActions(helpActions) {}

  GenericKeyDefinition() = default;
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

  signals:

    void controlValueChange(RadioWidget::RadioWidgetType type, int index, int value);
    void customStyleRequest(const QString & style);
    void simulatorWheelEvent(qint8 steps);

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

    void addGenericPushButton(int index, QString label, ButtonsWidget * leftButtons, QGridLayout * leftButtonsGrid,
                              ButtonsWidget * rightButtons, QGridLayout * rightButtonsGrid);
    void addGenericPushButtons(ButtonsWidget * leftButtons, ButtonsWidget * rightButtons);
    void addScrollActions();
    void addMouseActions();
};


// Each subclass is responsible for its own Ui
namespace Ui {
  class SimulatedUIWidgetGeneric;
  class SimulatedUIWidget9X;
  class SimulatedUIWidgetX9LITE;
  class SimulatedUIWidgetX7;
  class SimulatedUIWidgetX9;
  class SimulatedUIWidgetXLITE;
  class SimulatedUIWidgetX9E;
  class SimulatedUIWidgetX9D2019;
  class SimulatedUIWidgetX10;
  class SimulatedUIWidgetX12;
  class SimulatedUIWidgetLR3PRO;
  class SimulatedUIWidgetCommando8;
  class SimulatedUIWidgetJumperT12;
  class SimulatedUIWidgetJumperTLITE;
  class SimulatedUIWidgetJumperTPRO;
  class SimulatedUIWidgetJumperTPROS;
  class SimulatedUIWidgetJumperBumblebee;
  class SimulatedUIWidgetJumperT12max;
  class SimulatedUIWidgetJumperT14;
  class SimulatedUIWidgetJumperT15;
  class SimulatedUIWidgetJumperT16;
  class SimulatedUIWidgetJumperT18;
  class SimulatedUIWidgetJumperT20;
  class SimulatedUIWidgetTX16S;
  class SimulatedUIWidgetTX12;
  class SimulatedUIWidgetZorro;
  class SimulatedUIWidgetBoxer;
  class SimulatedUIWidgetMT12;
  class SimulatedUIWidgetPocket;
  class SimulatedUIWidgetT8;
  class SimulatedUIWidgetFatfishF16;
  class SimulatedUIWidgetNV14;
  class SimulatedUIWidgetEL18;
  class SimulatedUIWidgetPL18;
  class SimulatedUIWidgetV16;
}

class SimulatedUIWidgetGeneric: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetGeneric(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetGeneric();

  private:
    Ui::SimulatedUIWidgetGeneric * ui;

};

class SimulatedUIWidget9X: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidget9X(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidget9X();

  protected:
    void setLightOn(bool enable);

  private:
    Ui::SimulatedUIWidget9X * ui;

};

class SimulatedUIWidgetX9LITE: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9LITE(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX9LITE();

  private:
    Ui::SimulatedUIWidgetX9LITE * ui;
};

class SimulatedUIWidgetX7: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX7(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX7();

  private:
    Ui::SimulatedUIWidgetX7 * ui;
};

class SimulatedUIWidgetX9: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX9();

  private:
    Ui::SimulatedUIWidgetX9 * ui;
};

class SimulatedUIWidgetXLITE: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetXLITE(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetXLITE();

  private:
    Ui::SimulatedUIWidgetXLITE * ui;
};

class SimulatedUIWidgetX9E: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9E(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX9E();

  private:
    Ui::SimulatedUIWidgetX9E * ui;
};

class SimulatedUIWidgetX9D2019: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX9D2019(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX9D2019();

  private:
    Ui::SimulatedUIWidgetX9D2019 * ui;
};

class SimulatedUIWidgetX10: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX10(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX10();

  private:
    Ui::SimulatedUIWidgetX10 * ui;
};

class SimulatedUIWidgetX12: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetX12(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetX12();

  private:
    Ui::SimulatedUIWidgetX12 * ui;
};

class SimulatedUIWidgetLR3PRO: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetLR3PRO(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetLR3PRO();

  private:
    Ui::SimulatedUIWidgetLR3PRO * ui;
};

class SimulatedUIWidgetCommando8: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetCommando8(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetCommando8();

  private:
    Ui::SimulatedUIWidgetCommando8 * ui;
};

class SimulatedUIWidgetJumperT12: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperT12(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetJumperT12();

  private:
    Ui::SimulatedUIWidgetJumperT12 * ui;
};

class SimulatedUIWidgetJumperTLITE: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperTLITE(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetJumperTLITE();

  private:
    Ui::SimulatedUIWidgetJumperTLITE * ui;
};

class SimulatedUIWidgetJumperTPRO: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperTPRO(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetJumperTPRO();

  private:
    Ui::SimulatedUIWidgetJumperTPRO * ui;
};

class SimulatedUIWidgetJumperTPROS: public SimulatedUIWidget
{
    Q_OBJECT

public:
    explicit SimulatedUIWidgetJumperTPROS(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetJumperTPROS();

private:
    Ui::SimulatedUIWidgetJumperTPROS * ui;
};

class SimulatedUIWidgetJumperBumblebee: public SimulatedUIWidget
{
    Q_OBJECT

public:
    explicit SimulatedUIWidgetJumperBumblebee(SimulatorInterface * simulator, QWidget * parent = NULL);
    virtual ~SimulatedUIWidgetJumperBumblebee();

private:
    Ui::SimulatedUIWidgetJumperBumblebee * ui;
};

class SimulatedUIWidgetJumperT15: public SimulatedUIWidget
{
    Q_OBJECT

public:
    explicit SimulatedUIWidgetJumperT15(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT15();

private:
    Ui::SimulatedUIWidgetJumperT15 * ui;
};

class SimulatedUIWidgetJumperT16: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperT16(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT16();

  private:
    Ui::SimulatedUIWidgetJumperT16 * ui;
};

class SimulatedUIWidgetJumperT12max: public SimulatedUIWidget
{
    Q_OBJECT

public:
    explicit SimulatedUIWidgetJumperT12max(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT12max();

private:
    Ui::SimulatedUIWidgetJumperT12max * ui;
};

class SimulatedUIWidgetJumperT14: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperT14(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT14();

  private:
    Ui::SimulatedUIWidgetJumperT14 * ui;
};

class SimulatedUIWidgetJumperT18: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperT18(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT18();

  private:
    Ui::SimulatedUIWidgetJumperT18 * ui;
};

class SimulatedUIWidgetJumperT20: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetJumperT20(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetJumperT20();

  private:
    Ui::SimulatedUIWidgetJumperT20 * ui;
};

class SimulatedUIWidgetTX12: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetTX12(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetTX12();

  private:
    Ui::SimulatedUIWidgetTX12 * ui;
};

class SimulatedUIWidgetTX16S: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetTX16S(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetTX16S();

  private:
    Ui::SimulatedUIWidgetTX16S * ui;
};

class SimulatedUIWidgetZorro: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetZorro(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetZorro();

  private:
    Ui::SimulatedUIWidgetZorro * ui;
};

class SimulatedUIWidgetBoxer: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetBoxer(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetBoxer();

  private:
    Ui::SimulatedUIWidgetBoxer * ui;
};

class SimulatedUIWidgetMT12: public SimulatedUIWidgetGeneric
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetMT12(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetMT12();
};

class SimulatedUIWidgetPocket: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetPocket(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetPocket();

  private:
    Ui::SimulatedUIWidgetPocket * ui;
};

class SimulatedUIWidgetT8: public SimulatedUIWidget
{
Q_OBJECT

public:
  explicit SimulatedUIWidgetT8(SimulatorInterface * simulator, QWidget * parent = nullptr);
  virtual ~SimulatedUIWidgetT8();

private:
  Ui::SimulatedUIWidgetT8 * ui;
};

class SimulatedUIWidgetFatfishF16: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetFatfishF16(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetFatfishF16();

  private:
    Ui::SimulatedUIWidgetFatfishF16 * ui;
};

class SimulatedUIWidgetNV14: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetNV14(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetNV14();

  private:
    Ui::SimulatedUIWidgetNV14 * ui;
};

class SimulatedUIWidgetEL18: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetEL18(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetEL18();

  private:
    Ui::SimulatedUIWidgetEL18 * ui;
};

class SimulatedUIWidgetPL18: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetPL18(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetPL18();

  private:
    Ui::SimulatedUIWidgetPL18 * ui;
};

class SimulatedUIWidgetV16: public SimulatedUIWidget
{
  Q_OBJECT

  public:
    explicit SimulatedUIWidgetV16(SimulatorInterface * simulator, QWidget * parent = nullptr);
    virtual ~SimulatedUIWidgetV16();

  private:
    Ui::SimulatedUIWidgetV16 * ui;
};
