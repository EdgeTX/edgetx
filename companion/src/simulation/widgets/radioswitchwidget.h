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
#include "simulator.h"

#include <QSlider>
#include <QTimer>
#include <QToolButton>

class RadioSwitchWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioSwitchWidget(Board::SwitchType type = Board::SWITCH_3POS, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(parent, f)
    {
      init(type);
    }

    explicit RadioSwitchWidget(Board::SwitchType type, const QString & labelText, int value = -1, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(labelText, value, parent, f)
    {
      init(type);
    }

    void init(Board::SwitchType swType)
    {
      m_type = RADIO_WIDGET_SWITCH;

      m_stepSize = (swType == Board::SWITCH_3POS ? 1 : 2);

      m_slider = new QSlider();
      m_slider->setOrientation(Qt::Vertical);
      m_slider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
      m_slider->setInvertedAppearance(true);
      m_slider->setInvertedControls(true);
      m_slider->setTickPosition(QSlider::TicksBothSides);
      m_slider->setMinimum(-1);
      m_slider->setMaximum(1);
      m_slider->setTickInterval(m_stepSize);
      m_slider->setSingleStep(m_stepSize);
      m_slider->setPageStep(m_stepSize);
      m_slider->setValue(m_value);

      if (swType == Board::SWITCH_TOGGLE) {
        QToolButton * lockBtn = new QToolButton(this);
        lockBtn->setIcon(Simulator::SimulatorIcon("toggle_lock"));
        lockBtn->setIconSize(QSize(8, 8));
        lockBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        lockBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        lockBtn->setAutoRaise(true);
        lockBtn->setCheckable(true);
        lockBtn->setToolTip(tr("Latch/unlatch the momentary switch."));

        QWidget * container = new QWidget(this);
        container->setFixedHeight(56);
        container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QVBoxLayout * cl = new QVBoxLayout(container);
        cl->setContentsMargins(0, 0, 0, 0);
        cl->setSpacing(1);
        cl->addWidget(m_slider, 1, Qt::AlignHCenter);
        cl->addWidget(lockBtn, 0, Qt::AlignHCenter);

        setWidget(container);

        connect(lockBtn, &QToolButton::toggled, this, &RadioSwitchWidget::setToggleLocked);
        connect(m_slider, &QSlider::sliderReleased, this, &RadioSwitchWidget::onMomentaryReleased);
        connect(m_slider, &QSlider::valueChanged, this, &RadioSwitchWidget::onMomentaryReleased);
        connect(this, &RadioWidget::flagsChanged, lockBtn, &QToolButton::setChecked);
      }
      else {
        m_slider->setFixedHeight(56);
        setWidget(m_slider);
      }

      connect(m_slider, &QSlider::valueChanged, this, &RadioSwitchWidget::setValueFromSlider);
      connect(this, &RadioWidget::valueChanged, m_slider, &QSlider::setValue);
    }

    void setValueFromSlider(const int & value)
    {
      // 2POS and TOGGLE switches do not have a center point - force slider value to 1 if dragged to center
      if ((m_stepSize > 1) && (value == 0)) {
        // Note: this will trigger another value changed event call back into this function, at which time setValue will be called below
        m_slider->setValue(1);
        return;
      }
      setValue(value);
    }

    void setToggleLocked(bool lock)
    {
      if (m_flags != (quint16)lock) {
        setFlags((quint16)lock);
        resetToggle();
      }
    }

    void resetToggle()
    {
      setValue(-1);
    }

  private slots:

    void onMomentaryReleased()
    {
      if (!m_flags && !m_slider->isSliderDown())
        QTimer::singleShot(500, this, SLOT(onMomentaryTimeout()));
    }

    void onMomentaryTimeout()
    {
      if (!m_flags)
        resetToggle();
    }

  private:

    QSlider * m_slider;
    quint16 m_stepSize;

};
