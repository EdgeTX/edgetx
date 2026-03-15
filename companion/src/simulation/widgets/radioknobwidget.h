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

#include "radiowidget.h"
#include "boards.h"

#include <QDial>
#include <QMouseEvent>
#include <QToolTip>
#ifdef __APPLE__
#include <QStyleFactory>
#endif
#include <math.h>

class RadioKnobWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioKnobWidget(Board::FlexType type = Board::FLEX_POT_CENTER, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(parent, f)
    {
      init(type);
    }

    explicit RadioKnobWidget(Board::FlexType type, const QString & labelText, int value = 0, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(labelText, value, parent, f)
    {
      init(type);
    }

    void init(Board::FlexType potType)
    {
      m_type = RADIO_WIDGET_KNOB;

      m_stepSize = (potType == Board::FLEX_MULTIPOS) ? 2048 / 5 : 1;

      m_toolTip = tr("<p>Value (input): <b>%1</b></p>");
      if (m_stepSize == 1)
        m_toolTip.append(tr("Right-double-click to reset to center."));

      m_dial = new QDial(this);

#ifdef __APPLE__
      // Set style for dial to show dot instead of arrow pointer for selected position
      m_dial->setStyle(QStyleFactory::create("fusion"));
#endif

      m_dial->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      m_dial->setFixedSize(QSize(42, 42));
      m_dial->setNotchesVisible(true);

      m_dial->setMinimum(-1024);
      m_dial->setMaximum(1024);

      if (m_stepSize > 1) {
        // notch markers for multipos — actual values are constrained in setValueFromDial()
        m_dial->setSingleStep(m_stepSize / 10);
        m_dial->setPageStep(m_stepSize);
        m_dial->setNotchTarget(5.7);
      }
      else {
        m_dial->setPageStep(128);
        m_dial->setNotchTarget(64);
      }

      m_dial->setValue(m_value);

      connect(m_dial, &QDial::valueChanged, this, &RadioKnobWidget::setValueFromDial);
      connect(this, &RadioWidget::valueChanged, m_dial, &QDial::setValue);

      setWidget(m_dial);
    }

    void setValueFromDial(const int & value)
    {
      int v = value;
      if (m_stepSize > 1) {
        // Round to nearest step position relative to minimum
        int offset = v - m_dial->minimum();
        offset = ((offset + m_stepSize / 2) / m_stepSize) * m_stepSize;
        v = m_dial->minimum() + offset;
        // Fix values to account for lack of precision from integer step size
        // (2048 / 5 = 409 leaves a remainder of 3). Add correction to the
        // upper half so positions are symmetric around center.
        if (v > 0) v += 3;
        if (v != value) {
          m_dial->setValue(v);
          return;
        }
      }
      setValue(value);
    }

    void mousePressEvent(QMouseEvent * event)
    {
      if (m_stepSize == 1 && event->button() == Qt::RightButton && event->type() == QEvent::MouseButtonDblClick) {
        m_dial->setValue(0);
        event->accept();
        return;
      }
      RadioWidget::mousePressEvent(event);
    }

    void wheelEvent(QWheelEvent * event)
    {
      if (event->angleDelta().isNull())
        return;

      if (m_stepSize > 1) {
        int numSteps = event->angleDelta().y() / 8 / 15 * m_stepSize;  // one step per 15deg
        m_dial->setValue(m_dial->value() + numSteps);
        event->accept();
        return;
      }
      RadioWidget::wheelEvent(event);
    }

    bool event(QEvent *event)
    {
      if (event->type() == QEvent::ToolTip) {
        QHelpEvent * helpEvent = static_cast<QHelpEvent *>(event);
        if (helpEvent) {
          QToolTip::showText(helpEvent->globalPos(), m_toolTip.arg(m_dial->value()));
          return true;
        }
      }
      return RadioWidget::event(event);
    }

  private:

    QDial * m_dial;
    quint16 m_stepSize;
    QString m_toolTip;

};
