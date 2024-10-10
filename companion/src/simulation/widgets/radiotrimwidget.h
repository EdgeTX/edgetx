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
#include "sliderwidget.h"

#include <QToolButton>

class RadioTrimWidget : public RadioWidget
{
  Q_OBJECT

  public:

    explicit RadioTrimWidget(Qt::Orientation orientation = Qt::Vertical, QWidget * parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags()) :
      RadioWidget(parent, f),
      m_slider(NULL)
    {
      init(orientation);
    }

    void setIndices(int sliderIdx = -1, int decrBtnIdx = -1, int incrBtnIdx = -1)
    {
      setIndex(sliderIdx);
      m_btnDecIndex = decrBtnIdx;
      m_btnIncIndex = incrBtnIdx;
    }

    void setTrimRange(int min, int max)
    {
      if (m_slider)
        m_slider->setRange(min, max);
    }

    void setTrimRangeQual(const int index, const int min, const int max)
    {
      if (index == m_index || index == Board::TRIM_AXIS_COUNT)
        setTrimRange(min, max);
    }

    void setTrimValue(const int index, const int value)
    {
      if (index == m_index || index == Board::TRIM_AXIS_COUNT)
        setValue(value);
    }

    void setValue(const int & value) override
    {
      if (sender() && qobject_cast<SliderWidget *>(sender())) {
        RadioWidget::setValue(value);
      }
      else if (m_slider) {
        m_slider->setValue(value);
      }
    }

    void chgValueQual(const RadioWidgetType & type, const int & index, const int offset, const bool state) override {
      if (type == m_type && index == m_index) {
        emit valueChange(m_type, (offset < 0) ? m_btnDecIndex : m_btnIncIndex, state ? RADIO_TRIM_BTN_ON : RADIO_TRIM_BTN_OFF);
      }
    }

  protected slots:

    void init(Qt::Orientation orientation)
    {
      m_type = RADIO_WIDGET_TRIM;
      m_slider = new SliderWidget(this);
      m_slider->setOrientation(orientation);

      setTrimRange(-125, 125);
      setIndices();

      const QSize btnSz(18, 18);
      QWidget * trimWidget = new QWidget(this);
      QBoxLayout * trimLayout = new QVBoxLayout(trimWidget);
      trimLayout->setSpacing(4);
      QToolButton * trimBtnInc = new QToolButton(trimWidget);
      trimBtnInc->setMaximumSize(btnSz);
      QToolButton * trimBtnDec = new QToolButton(trimWidget);
      trimBtnDec->setMaximumSize(btnSz);

      Qt::Alignment algn;
      if (orientation == Qt::Horizontal) {
        trimWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        trimLayout->setDirection(QBoxLayout::RightToLeft);
        trimLayout->setContentsMargins(0, 8, 0, 8);
        trimBtnInc->setArrowType(Qt::RightArrow);
        trimBtnDec->setArrowType(Qt::LeftArrow);
        algn = Qt::AlignVCenter;
      }
      else {
        trimWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        trimLayout->setDirection(QBoxLayout::TopToBottom);
        trimLayout->setContentsMargins(8, 0, 8, 0);
        trimBtnInc->setArrowType(Qt::UpArrow);
        trimBtnDec->setArrowType(Qt::DownArrow);
        algn = Qt::AlignHCenter;
      }

      trimLayout->addWidget(trimBtnInc, 0, algn);
      trimLayout->addWidget(m_slider, 1, algn);
      trimLayout->addWidget(trimBtnDec, 0, algn);

      setWidget(trimWidget, algn);

      connect(m_slider, &SliderWidget::valueChanged, this, &RadioTrimWidget::setValue);
      connect(trimBtnInc, &QToolButton::pressed,  [this]() { emit valueChange(m_type, m_btnIncIndex, RADIO_TRIM_BTN_ON); });
      connect(trimBtnInc, &QToolButton::released, [this]() { emit valueChange(m_type, m_btnIncIndex, RADIO_TRIM_BTN_OFF); });
      connect(trimBtnDec, &QToolButton::pressed,  [this]() { emit valueChange(m_type, m_btnDecIndex, RADIO_TRIM_BTN_ON); });
      connect(trimBtnDec, &QToolButton::released, [this]() { emit valueChange(m_type, m_btnDecIndex, RADIO_TRIM_BTN_OFF); });
    }

  protected:
    SliderWidget * m_slider;
    int m_btnDecIndex;
    int m_btnIncIndex;
};
