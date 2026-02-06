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
#include "appdata.h"

#include <QPushButton>
#include <QVBoxLayout>

constexpr int BUTTON_WIDTH  {65};
constexpr int BUTTON_HEIGHT {30};

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

  auto width = Boards::getCapability(m_board, Board::LcdWidth);
  auto height = Boards::getCapability(m_board, Board::LcdHeight);
  auto depth = Boards::getCapability(m_board, Board::LcdDepth);
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
//  MUST be kept in sync with EnumKeys
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

//  Notes: unused rows will be hidden and squashed
static const QList<RadioKeyDefinition> radioKeyDefinitions = {
  { KEY_SYS,    'L', 0, QList<int>() << Qt::Key_S,        SIMU_STR_HLP_KEY_S,     SIMU_STR_HLP_ACT_SYS },

  { KEY_MODEL,  'R', 0, QList<int>() << Qt::Key_M,        SIMU_STR_HLP_KEY_M,     SIMU_STR_HLP_ACT_MDL },

  { KEY_PAGEUP, 'L', 1, QList<int>() << Qt::Key_PageUp,   SIMU_STR_HLP_KEY_PGUP,  SIMU_STR_HLP_ACT_PGUP },

  { KEY_PAGEDN, 'R', 1, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN,  SIMU_STR_HLP_ACT_PGDN },

  { KEY_UP,     'L', 2, QList<int>() << Qt::Key_Up,       SIMU_STR_HLP_KEY_UP,    SIMU_STR_HLP_ACT_UP },

  { KEY_DOWN,   'R', 2, QList<int>() << Qt::Key_Down,     SIMU_STR_HLP_KEY_DN,    SIMU_STR_HLP_ACT_DOWN },

  { KEY_LEFT,   'L', 3, QList<int>() << Qt::Key_Left,     SIMU_STR_HLP_KEY_LFT,   SIMU_STR_HLP_ACT_LFT },

  { KEY_RIGHT,  'R', 3, QList<int>() << Qt::Key_Right,    SIMU_STR_HLP_KEY_RGT,   SIMU_STR_HLP_ACT_RGT },

  { KEY_MINUS,  'L', 4, QList<int>() << Qt::Key_Minus,    SIMU_STR_HLP_KEY_MIN,   SIMU_STR_HLP_ACT_MIN },

  { KEY_PLUS,   'R', 4, QList<int>() << Qt::Key_Plus,     SIMU_STR_HLP_KEY_PLS,   SIMU_STR_HLP_ACT_PLS },

  { KEY_TELE,   'R', 5, QList<int>() << Qt::Key_T,        SIMU_STR_HLP_KEY_T,     SIMU_STR_HLP_ACT_TELE },

  { KEY_MENU,   'L', 6, QList<int>() << Qt::Key_Equal,    SIMU_STR_HLP_KEY_EQL,   SIMU_STR_HLP_ACT_MENU },

  { KEY_SHIFT,  'R', 6, QList<int>() << Qt::Key_Insert
                                     << Qt::Key_Shift,    SIMU_STR_HLP_KEY_INS,   SIMU_STR_HLP_ACT_SHIFT },

  { KEY_EXIT,   'L', 7, QList<int>() << Qt::Key_Escape
                                     << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT },

  { KEY_ENTER,  'R', 7, QList<int>() << Qt::Key_Enter
                                     << Qt::Key_Return,   SIMU_STR_HLP_KEYS_ENTER, SIMU_STR_HLP_ACT_ENTER },

  //  keep these on the last row
  { KEY_SCRLUP, 'L', 8, QList<int>() << Qt::Key_Comma,    SIMU_STR_HLP_KEYS_SCRLUP, SIMU_STR_HLP_ACT_ROT_LFT },

  { KEY_SCRLDN, 'R', 8, QList<int>() << Qt::Key_Period,   SIMU_STR_HLP_KEYS_SCRLDN, SIMU_STR_HLP_ACT_ROT_RGT },
};

void SimulatedUIWidget::addScrollActions()
{
  if (g.simuScrollButtons() || !Boards::getCapability(m_board, Board::RotaryEncoderNavigation))
    return;

  const RadioKeyDefinition *updefn = getRadioKeyDefinition(KEY_SCRLUP);
  if (updefn)
    m_scrollUpAction = new RadioUiAction(-2, updefn->keys, updefn->helpKeys, updefn->helpActions);

  const RadioKeyDefinition *downdefn = getRadioKeyDefinition(KEY_SCRLDN);
  if (downdefn)
    m_scrollDnAction = new RadioUiAction(-3, downdefn->keys, downdefn->helpKeys, downdefn->helpActions);

  connectScrollActions();
}

void SimulatedUIWidget::addMouseActions()
{
  const RadioKeyDefinition *defn = getRadioKeyDefinition(KEY_ENTER);
  if (defn)
    m_mouseMidClickAction = new RadioUiAction(defn->index, defn->keys, defn->helpKeys, defn->helpActions);
}

void SimulatedUIWidget::addPushButtons(ButtonsWidget * leftButtons, ButtonsWidget * rightButtons)
{
  QGridLayout * gridLeft = new QGridLayout((QWidget *)leftButtons);
  QGridLayout * leftButtonsGrid = new QGridLayout();

  if (g.simuGenericKeysPos() != AppData::SIMU_GENERIC_KEYS_RIGHT) {
    leftButtons->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    gridLeft->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0);

    int width = 0;
    int col = 0;
    leftButtonsGrid->setColumnMinimumWidth(col++, BUTTON_WIDTH);
    width += BUTTON_WIDTH;

    if (g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_LEFT) {
      leftButtonsGrid->setColumnMinimumWidth(col++, 2);
      width += 2;
      leftButtonsGrid->setColumnMinimumWidth(col++, BUTTON_WIDTH);
      width += BUTTON_WIDTH;
    }

    leftButtonsGrid->setColumnMinimumWidth(col++, 2);
    width += 2;
    leftButtons->setMinimumWidth(width);
    gridLeft->addLayout(leftButtonsGrid, 0, 1);
  }
  else {
    leftButtons->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    leftButtons->setFixedSize(0, 0);
  }

  QGridLayout * gridRight = new QGridLayout((QWidget *)rightButtons);
  QGridLayout * rightButtonsGrid = new QGridLayout();

  if (g.simuGenericKeysPos() != AppData::SIMU_GENERIC_KEYS_LEFT) {
    rightButtons->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    int width = 0;
    int col = 0;
    rightButtonsGrid->setColumnMinimumWidth(col++, BUTTON_WIDTH);
    width += BUTTON_WIDTH;

    if (g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_RIGHT) {
      rightButtonsGrid->setColumnMinimumWidth(col++, 2);
      width += 2;
      rightButtonsGrid->setColumnMinimumWidth(col++, BUTTON_WIDTH);
      width += BUTTON_WIDTH;
    }

    rightButtonsGrid->setColumnMinimumWidth(col++, 2);
    width += 2;

    rightButtons->setMinimumWidth(width);
    gridRight->addLayout(rightButtonsGrid, 0, 0);
    gridRight->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1);
  }
  else {
    rightButtons->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rightButtons->setFixedSize(0, 0);
  }

  for (int i = 0; i < Boards::getCapability(m_board, Board::Keys); i++) {
    Board::KeyInfo info = Boards::getKeyInfo(i, m_board);
    int idx = strKeyToInt(info.key);
    //qDebug() << "key:" << info.key.c_str() << info.name.c_str() << info.label.c_str() << idx;
    if (idx >= 0)
      addPushButton(idx, info.label.c_str(), leftButtons, leftButtonsGrid, rightButtons, rightButtonsGrid);
    else
      qDebug() << "Unknown key:" << info.key.c_str() << info.name.c_str() << info.label.c_str();
  }

  if (g.simuScrollButtons() && Boards::getCapability(m_board, Board::RotaryEncoderNavigation)) {
      addPushButton(KEY_SCRLUP, tr("Scrl Up"), leftButtons, leftButtonsGrid, rightButtons, rightButtonsGrid);
      addPushButton(KEY_SCRLDN, tr("Scrl Dn"), leftButtons, leftButtonsGrid, rightButtons, rightButtonsGrid);
      connectScrollActions();
  }
}

void SimulatedUIWidget::addPushButton(int index, QString label, ButtonsWidget * leftButtons, QGridLayout * leftButtonsGrid,
                                      ButtonsWidget * rightButtons, QGridLayout * rightButtonsGrid)
{
  for (int i = 0; i < radioKeyDefinitions.size(); i++) {
    const RadioKeyDefinition defn = radioKeyDefinitions.at(i);

    if (defn.index == index) {
      QPushButton * b = new QPushButton(label);
      b->setToolTip(tr("Shortcut: %1").arg(defn.helpKeys));
      b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      b->setMinimumSize(BUTTON_WIDTH, BUTTON_HEIGHT);
      b->setMaximumSize(b->minimumSize());
      ButtonsWidget * btns = g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_DEFAULT ? (defn.side == 'L' ? leftButtons : rightButtons) :
                             (g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_LEFT ? leftButtons : rightButtons);
      QGridLayout * grid = g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_DEFAULT ? (defn.side == 'L' ? leftButtonsGrid : rightButtonsGrid) :
                           (g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_LEFT ? leftButtonsGrid : rightButtonsGrid);
      int col = g.simuGenericKeysPos() == AppData::SIMU_GENERIC_KEYS_DEFAULT ? 0 : (defn.side == 'L' ? 0 : 2);
      grid->setRowMinimumHeight(defn.gridRow, BUTTON_HEIGHT);
      grid->addWidget(b, defn.gridRow, col);
      int idx = -1;

      switch (defn.index) {
        case KEY_SCRLUP:
          idx = -2;
          break;
        case KEY_SCRLDN:
          idx = -3;
          break;
        default:
          idx = defn.index;
      }

      RadioUiAction * act = new RadioUiAction(idx, defn.keys, defn.helpKeys, defn.helpActions);

      switch (defn.index) {
        case KEY_SCRLUP:
          m_scrollUpAction = act;
          break;
        case KEY_SCRLDN:
          m_scrollDnAction = act;
          break;
      }

      addRadioWidget(btns->addPushButton(b, act));
      break;
    }
  }
}

const RadioKeyDefinition * SimulatedUIWidget::getRadioKeyDefinition(const int key) const
{
  for (int i = 0; i < radioKeyDefinitions.size(); i++) {
    const RadioKeyDefinition defn = radioKeyDefinitions.at(i);
    if (defn.index == key)
      return &radioKeyDefinitions.at(i);
  }

  qDebug() << "Unknown key:" << key;
  return nullptr;
}
