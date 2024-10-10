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

#include "simulateduiwidget.h"
#include "eeprominterface.h"
#include "lcdwidget.h"
#include "radiouiaction.h"
#include "radiokeywidget.h"
#include "simulatorinterface.h"
#include "buttonswidget.h"

#include <QPushButton>
#include <QVBoxLayout>

//#define FLASH_DURATION 10
//#define CBEEP_ON      "QLabel { background-color: #FF364E }"
//#define CBEEP_OFF     "QLabel { }"

SimulatedUIWidget::SimulatedUIWidget(SimulatorInterface * simulator, QWidget * parent) :
  QWidget(parent),
  m_simulator(simulator),
  m_parent(parent),
  m_lcd(nullptr),
  m_scrollUpAction(nullptr),
  m_scrollDnAction(nullptr),
  m_mouseMidClickAction(nullptr),
  m_screenshotAction(nullptr),
  m_board(getCurrentBoard()),
  m_backLight(0),
  m_beepShow(0),
  m_beepVal(0)
{
  m_screenshotAction = new RadioUiAction(-1, Qt::Key_Print);
  connect(m_screenshotAction, static_cast<void (RadioUiAction::*)(void)>(&RadioUiAction::pushed), this, &SimulatedUIWidget::captureScreenshot);

  connect(m_simulator, &SimulatorInterface::lcdChange, this, &SimulatedUIWidget::onLcdChange);
  connect(this, &SimulatedUIWidget::simulatorWheelEvent, m_simulator, &SimulatorInterface::rotaryEncoderEvent);
}

SimulatedUIWidget::~SimulatedUIWidget()
{
  foreach (RadioUiAction * act, m_actions) {
    if (act)
      delete act;
  }
  foreach (RadioWidget * w, m_widgets) {
    if (w)
      delete w;
  }
}

RadioWidget * SimulatedUIWidget::addRadioWidget(RadioWidget * widget)
{
  if (widget && !m_widgets.contains(widget)) {
    m_widgets.append(widget);
    // TODO : connect to actions instead
    connect(widget, &RadioWidget::valueChange, this, &SimulatedUIWidget::controlValueChange);
    if (widget->getAction())
      addRadioAction(widget->getAction());
  }
  return widget;
}

RadioUiAction * SimulatedUIWidget::addRadioAction(RadioUiAction * act)
{
  if (act && !m_actions.contains(act)) {
    act->setParent(m_parent);
    m_actions.append(act);
  }
  return act;
}

QVector<Simulator::keymapHelp_t> SimulatedUIWidget::getKeymapHelp() const
{
  QVector<Simulator::keymapHelp_t> keymapHelp;
  foreach (RadioUiAction * act, m_actions) {
    if (act && !act->getText().isEmpty())
      keymapHelp.append(Simulator::keymapHelp_t(act->getText(), act->getDescription()));
  }
  return keymapHelp;
}

// static
QPolygon SimulatedUIWidget::polyArc(int ctrX, int ctrY, int radius, int startAngle, int endAngle, int step)
{
  QPolygon polygon;
  float st = qDegreesToRadians((float)startAngle - 90.0f);
  float en = qDegreesToRadians((float)endAngle - 90.0f);
  float sp = qDegreesToRadians((float)step);
  for ( ; st < en; st += sp) {
    polygon << QPoint((int)(ctrX + radius * cosf(st)), (int)(ctrY + radius * sinf(st)));
  }
  return polygon;
}

/*  TODO : beep indicator
void SimulatedUIWidget::updateUi()
{
  //static quint32 loop = 0;
      if (!(loop % 5)) {
        TxOutputs outputs;
        simulator->getValues(outputs);
        if (outputs.beep) {
          beepVal = outputs.beep;
        }
        if (beepVal) {
          beepShow = 20;
          beepVal = 0;
          QApplication::beep();
        } else if (beepShow) {
          beepShow--;
        }
        ui->label_beep->setStyleSheet(beepShow ? CBEEP_ON : CBEEP_OFF);
      }
} */

void SimulatedUIWidget::onLcdChange(bool backlightEnable)
{
  if (!m_lcd || !m_lcd->isVisible())
    return;

  uint8_t* lcdBuf = m_simulator->getLcd();
  m_lcd->onLcdChanged(lcdBuf, backlightEnable);
  m_simulator->lcdFlushed();

  setLightOn(backlightEnable);
}

void SimulatedUIWidget::captureScreenshot()
{
  QString fileName;
  if (!g.snapToClpbrd()) {
    QString path = g.snapshotDir();
    if (path.isEmpty())
      path = "./";
    QDir dir(path);
    if (dir.exists() && dir.isReadable()) {
      QStringList fwid = getCurrentFirmware()->getId().split("-", Qt::SkipEmptyParts);
      QString flavor = fwid.at(qMin(1, fwid.size()));
      QString fnpfx = tr("screenshot", "Simulator LCD screenshot file name prefix");
      fileName = "%1/%2_%3_%4.png";
      fileName = fileName.arg(dir.absolutePath(), fnpfx, flavor, QDateTime::currentDateTime().toString("yy-MM-dd_HH-mm-ss"));
    }
    else {
      qWarning() << "SIMULATOR ERROR - Cannot open screenshot folder, check your settings.";
    }
  }
  m_lcd->makeScreenshot(fileName);
}

void SimulatedUIWidget::wheelEvent(QWheelEvent * event)
{
  if (event->angleDelta().isNull())
    return;
  // steps can be negative or positive to determine direction (negative is UP/LEFT scroll)
  QPoint numSteps = event->angleDelta() / 8 / 15 * -1;  // one step per 15deg
  emit simulatorWheelEvent(numSteps.y());
  event->accept();
}

void SimulatedUIWidget::mousePressEvent(QMouseEvent * event)
{
  if (event->button() == Qt::MiddleButton && m_mouseMidClickAction)
    m_mouseMidClickAction->trigger(true);
  else
    event->ignore();
}

void SimulatedUIWidget::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->button() == Qt::MiddleButton && m_mouseMidClickAction)
    m_mouseMidClickAction->trigger(false);
  else
    event->ignore();
}

void SimulatedUIWidget::setLcd(LcdWidget * lcd)
{
  m_lcd = lcd;
  Board::Type board = getCurrentBoard();

  auto width = Boards::getCapability(board, Board::LcdWidth);
  auto height = Boards::getCapability(board, Board::LcdHeight);
  auto depth = Boards::getCapability(board, Board::LcdDepth);
  m_lcd->setData(width, height, depth);

  if (!m_backlightColors.size())
    return;

  m_backLight = g.backLight();
  if ((int)m_backLight >= m_backlightColors.size())
    m_backLight = 0;

  m_lcd->setBackgroundColor(m_backlightColors.at(m_backLight));
  connect(m_lcd, &LcdWidget::touchEvent, m_simulator, &SimulatorInterface::touchEvent);
}

void SimulatedUIWidget::connectScrollActions()
{
  if (m_scrollUpAction) {
    addRadioAction(m_scrollUpAction);
    connect(m_scrollUpAction, static_cast<void (RadioUiAction::*)(void)>(&RadioUiAction::pushed), [this](void) {
      emit simulatorWheelEvent(-1);
      m_scrollUpAction->toggle(false);
    });
  }

  if (m_scrollDnAction) {
    addRadioAction(m_scrollDnAction);
    connect(m_scrollDnAction, static_cast<void (RadioUiAction::*)(void)>(&RadioUiAction::pushed), [this](void) {
      emit simulatorWheelEvent(1);
      m_scrollDnAction->toggle(false);
    });
  }
}

// static
int SimulatedUIWidget::strKeyToInt(std::string key)
{
  QStringList keys = {
    "KEY_MENU",
    "KEY_EXIT",
    "KEY_ENTER",
    "KEY_PAGEUP",
    "KEY_PAGEDN",
    "KEY_UP",
    "KEY_DOWN",
    "KEY_LEFT",
    "KEY_RIGHT",
    "KEY_PLUS",
    "KEY_MINUS",
    "KEY_MODEL",
    "KEY_TELE",
    "KEY_SYS",
    "KEY_SHIFT",
    "KEY_BIND"
  };

  return keys.indexOf(key.c_str());
}

void SimulatedUIWidget::addScrollActions()
{
  //  Note: the keys cannot duplicate those used for radio buttons
  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus,
                                       SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal,
                                       SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();
}

void SimulatedUIWidget::addMouseActions()
{
  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return,
                                            SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
}

//  Notes: unused rows will be hidden and squashed
//         keys cannot duplicate those used for scrolling tefer m_mouseMidClickAction
static const QList<GenericKeyDefinition> genericKeyDefinitions = {
  { KEY_SYS,    'R', 0, 0, QList<int>() << Qt::Key_Left,      SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS },

  { KEY_MODEL,  'R', 0, 1, QList<int>() << Qt::Key_Up,        SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL },

  { KEY_PAGEUP, 'R', 1, 0, QList<int>() << Qt::Key_PageUp,    SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP },

  { KEY_PAGEDN, 'R', 1, 1, QList<int>() << Qt::Key_PageDown,  SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN },

  { KEY_UP,     'R', 2, 0, QList<int>() << Qt::Key_Up
                                        << Qt::Key_PageUp,    SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_UP },

  { KEY_DOWN,   'R', 2, 1, QList<int>() << Qt::Key_Down
                                        << Qt::Key_PageDown,  SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_DN },

  { KEY_LEFT,   'R', 3, 0, QList<int>() << Qt::Key_Left
                                        << Qt::Key_Minus,     SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_MIN, SIMU_STR_HLP_ACT_MIN },

  { KEY_RIGHT,  'R', 3, 1, QList<int>() << Qt::Key_Right
                                        << Qt::Key_Plus,      SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_PLS, SIMU_STR_HLP_ACT_PLS },

  { KEY_MINUS,  'R', 4, 0, QList<int>() << Qt::Key_Minus
                                        << Qt::Key_Right,     SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_MIN },

  { KEY_PLUS,   'R', 4, 1, QList<int>() << Qt::Key_Plus
                                        << Qt::Key_Equal
                                        << Qt::Key_Left,      SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_PLS },

  { KEY_TELE,   'R', 5, 1, QList<int>() << Qt::Key_Right,     SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_TELE },

  { KEY_MENU,   'R', 6, 0, QList<int>() << Qt::Key_Up
                                        << Qt::Key_PageUp,    SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU },

  { KEY_SHIFT,  'R', 6, 1, QList<int>() << Qt::Key_S,         SIMU_STR_HLP_KEY_SHIFT, SIMU_STR_HLP_ACT_SHIFT },

  { KEY_EXIT,   'R', 7, 0, QList<int>() << Qt::Key_Down
                                        << Qt::Key_Delete
                                        << Qt::Key_Escape
                                        << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT },

  { KEY_ENTER,  'R', 7, 1, QList<int>() << Qt::Key_Enter
                                        << Qt::Key_Return,    SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN },
};

void SimulatedUIWidget::addGenericPushButtons(ButtonsWidget * leftButtons, ButtonsWidget * rightButtons)
{
  QGridLayout * leftButtonsGrid = new QGridLayout();
  QGridLayout * rightButtonsGrid = new QGridLayout();

  for (int i = 0; i < Boards::getCapability(m_board, Board::Keys); i++) {
    Board::KeyInfo info = Boards::getKeyInfo(i, m_board);
    int idx = strKeyToInt(info.key);
    //qDebug() << "key:" << info.key.c_str() << info.name.c_str() << info.label.c_str() << idx;
    if (idx >= 0)
      addGenericPushButton(idx, info.label.c_str(), leftButtons, leftButtonsGrid, rightButtons, rightButtonsGrid);
    else
      qDebug() << "Unknown key:" << info.key.c_str() << info.name.c_str() << info.label.c_str();
  }

  QGridLayout * gridLeft = new QGridLayout((QWidget *)leftButtons);
  gridLeft->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0);
  gridLeft->addLayout(leftButtonsGrid, 0, 1);
  //gridLeft->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);

  QGridLayout * gridRight = new QGridLayout((QWidget *)rightButtons);
  //gridRight->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0);
  gridRight->addLayout(rightButtonsGrid, 0, 1);
  gridRight->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2);
}

void SimulatedUIWidget::addGenericPushButton(int index, QString label, ButtonsWidget * leftButtons, QGridLayout * leftButtonsGrid,
                                             ButtonsWidget * rightButtons, QGridLayout * rightButtonsGrid)
{
  if (index >= genericKeyDefinitions.size())
    return;

  for (int i = 0; i < genericKeyDefinitions.size(); i++) {
    const GenericKeyDefinition defn = genericKeyDefinitions.at(i);

    if (defn.index == index) {
      QPushButton * b = new QPushButton(label);
      b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      b->setMinimumSize(60, 30);
      b->setMaximumSize(60, 30);
      RadioUiAction * act = new RadioUiAction();
      ButtonsWidget * btns = defn.side == 'L' ? leftButtons : rightButtons;
      QGridLayout * grid = defn.side == 'L' ? leftButtonsGrid : rightButtonsGrid;
      grid->addWidget(b, defn.gridRow, defn.gridCol);
      act = new RadioUiAction(defn.index, defn.keys, defn.helpKeys, defn.helpActions);
      addRadioWidget(btns->addPushButton(b, act));
      break;
    }
  }
}
