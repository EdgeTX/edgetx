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

#include "genericpanel.h"
#include "autowidget.h"

#include <TimerEdit>
#include <QComboBox>
#include <QEvent>
#include <QLabel>
#include <QGridLayout>
#include <QSpacerItem>

GenericPanel::GenericPanel(QWidget * parent, ModelData * model, GeneralSettings & generalSettings, Firmware * firmware):
  QWidget(parent),
  model(model),
  generalSettings(generalSettings),
  firmware(firmware),
  //board(firmware->getBoard()), TODO: as part of capabilities refactor
  lock(false)
{
}

GenericPanel::~GenericPanel()
{
}

void GenericPanel::update()
{
}

void GenericPanel::addLabel(QGridLayout *gridLayout, const QString &text, int col, bool minimize)
{
  QLabel *label = new QLabel(this);
  label->setFrameShape(QFrame::Panel);
  label->setFrameShadow(QFrame::Raised);
  label->setMidLineWidth(0);
  label->setAlignment(Qt::AlignCenter);
  label->setMargin(5);
  label->setText(text);
  if (!minimize)
    label->setMinimumWidth(100);
  gridLayout->addWidget(label, 0, col, 1, 1);
}

void GenericPanel::addEmptyLabel(QGridLayout * gridLayout, int col)
{
  QLabel *label = new QLabel(this);
  label->setText("");
  gridLayout->addWidget(label, 0, col, 1, 1);
}

void GenericPanel::addHSpring(QGridLayout * gridLayout, int col, int row)
{
  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  gridLayout->addItem(spacer, row, col);
}

void GenericPanel::addVSpring(QGridLayout * gridLayout, int col, int row)
{
  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
  gridLayout->addItem(spacer, row, col);
}

void GenericPanel::addDoubleSpring(QGridLayout * gridLayout, int col, int row)
{
  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );
  gridLayout->addItem(spacer, row, col);
}

bool GenericPanel::eventFilter(QObject *object, QEvent * event)
{
  QWidget * widget = qobject_cast<QWidget*>(object);
  if (widget) {
    if (event->type() == QEvent::Wheel) {
      if (widget->focusPolicy() == Qt::WheelFocus) {
        event->accept();
        return false;
      }
      else {
        event->ignore();
        return true;
      }
    }
    else if (event->type() == QEvent::FocusIn) {
      widget->setFocusPolicy(Qt::WheelFocus);
    }
    else if (event->type() == QEvent::FocusOut) {
      widget->setFocusPolicy(Qt::StrongFocus);
    }
  }
  return QWidget::eventFilter(object, event);
}

void GenericPanel::setFocusFilter(QWidget * w)
{
  w->installEventFilter(this);
  w->setFocusPolicy(Qt::StrongFocus);
}

void GenericPanel::disableMouseScrolling()
{
  foreach(QWidget * cb, findChildren<QComboBox*>())
    setFocusFilter(cb);

  foreach(QWidget * sb, findChildren<QAbstractSpinBox*>())
    setFocusFilter(sb);

  foreach(QWidget * slider, findChildren<QSlider*>())
    setFocusFilter(slider);

  foreach(QWidget * te, findChildren<TimerEdit*>())
    setFocusFilter(te);
}

void GenericPanel::updateAutoWidgets()
{
  foreach(QWidget *wdgt, findChildren<QWidget *>()) {
    AutoWidget *autowdgt = dynamic_cast<AutoWidget *>(wdgt);
    if (autowdgt) {
      autowdgt->updateValue();
    }
  }
}
