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

#ifndef _RADIOKNOBWIDGET_H_
#define _RADIOKNOBWIDGET_H_

#include "radiowidget.h"
#include "boards.h"

#include <QDial>
#include <QMouseEvent>
#include <QToolTip>
#include <math.h>

class RadioKnobWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioKnobWidget(Board::PotType type = Board::POT_WITH_DETENT, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(parent, f),
      m_potType(type)
    {
      init();
    }
    explicit RadioKnobWidget(Board::PotType type, const QString & labelText, int value = 0, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(labelText, value, parent, f),
      m_potType(type)
    {
      init();
    }

    void init()
    {
      m_type = RADIO_WIDGET_KNOB;

      KnobWidget * pot = new KnobWidget(m_potType, this);
      pot->setValue(m_value);

      connect(pot, &KnobWidget::valueChanged, this, &RadioWidget::setValue);
      connect(this, &RadioWidget::valueChanged, pot, &KnobWidget::setValue);

      setWidget(pot);
    }

  private:

    Board::PotType m_potType;

    class KnobWidget : public QDial
    {
      public:

        explicit KnobWidget(Board::PotType type, QWidget * parent = 0):
          QDial(parent)
        {
          m_toolTip = tr("<p>Value (input): <b>%1</b></p>");

          setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
          setFixedSize(QSize(42, 42));
          setNotchesVisible(true);
          if (type == Board::POT_MULTIPOS_SWITCH) {
            m_stepSize = 2048 / 5;
            setMinimum(0);
            setMaximum(2048);
            // this is a bit of a hack to get the notch markers to display correctly
            // the actual notches/value are constrained in setValue()
            setSingleStep(m_stepSize / 10);
            setPageStep(m_stepSize);
            setNotchTarget(5.7);
          }
          else {
            m_stepSize = 1;
            m_toolTip.append(tr("Right-double-click to reset to center."));
            setMinimum(-1024);
            setMaximum(1024);
            setPageStep(128);
            setNotchTarget(64);
          }
        }

        void setValue(int value)
        {
          if (m_stepSize > 1) {
            value = value / m_stepSize * m_stepSize;
          }
          QDial::setValue(value);
        }


        bool event(QEvent *event)
        {
          if (event->type() == QEvent::ToolTip) {
            QHelpEvent * helpEvent = static_cast<QHelpEvent *>(event);
            if (helpEvent) {
              QToolTip::showText(helpEvent->globalPos(), m_toolTip.arg(this->value()));
              return true;
            }
          }
          return QWidget::event(event);
        }

        void mousePressEvent(QMouseEvent * event)
        {
          if (m_stepSize == 1 && event->button() == Qt::RightButton && event->type() == QEvent::MouseButtonDblClick) {
            setValue(0);
            event->accept();
            return;
          }
          QDial::mousePressEvent(event);
        }

        void wheelEvent(QWheelEvent * event)
        {
          if (event->angleDelta().isNull())
            return;

          if (m_stepSize > 1) {
            int numSteps = event->angleDelta().y() / 8 / 15 * m_stepSize;  // one step per 15deg
            setValue(value() + numSteps);
            event->accept();
            return;
          }
          QDial::wheelEvent(event);
        }

        quint16 m_stepSize;
        QString m_toolTip;
    };

};


#endif // _RADIOKNOBWIDGET_H_
