/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

//#define FLASH_DURATION 10
//#define CBEEP_ON      "QLabel { background-color: #FF364E }"
//#define CBEEP_OFF     "QLabel { }"

SimulatedUIWidget::SimulatedUIWidget(SimulatorInterface * simulator, QWidget * parent) :
  QWidget(parent),
  m_simulator(simulator),
  m_parent(parent),
  m_lcd(NULL),
  m_scrollUpAction(NULL),
  m_scrollDnAction(NULL),
  m_mouseMidClickAction(NULL),
  m_screenshotAction(NULL),
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
      QStringList fwid = getCurrentFirmware()->getId().split("-", QString::SkipEmptyParts);
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
}

void SimulatedUIWidget::mousePressEvent(QMouseEvent * event)
{
  if (event->button() == Qt::MidButton && m_mouseMidClickAction)
    m_mouseMidClickAction->trigger(true);
  else
    event->ignore();
}

void SimulatedUIWidget::mouseReleaseEvent(QMouseEvent * event)
{
  if (event->button() == Qt::MidButton && m_mouseMidClickAction)
    m_mouseMidClickAction->trigger(false);
  else
    event->ignore();
}

void SimulatedUIWidget::setLcd(LcdWidget * lcd)
{
  m_lcd = lcd;
  Firmware * firmware = getCurrentFirmware();

  auto width = firmware->getCapability(LcdWidth);
  auto height = firmware->getCapability(LcdHeight);
  auto depth = firmware->getCapability(LcdDepth);
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
