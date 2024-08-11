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

#include "radiouiaction.h"
#include "radiokeywidget.h"

#include <QWidget>
#include <QtGui>
#include <QStyleOption>

class ButtonsWidget : public QWidget
{
  Q_OBJECT

  public:

    explicit ButtonsWidget(QWidget * parent):
      QWidget(parent)
    {
    }

    virtual void setStyleSheet(const QString & sheet)
    {
      defaultStyleSheet = sheet;
      QWidget::setStyleSheet(sheet);
    }

    RadioKeyWidget * addArea(const QRect & rect, const char * image, RadioUiAction * action = nullptr)
    {
      return addArea(QPolygon(rect), image, action);
    }

    RadioKeyWidget * addArea(const QPolygon & polygon, const char * image, RadioUiAction * action = nullptr)
    {
      RadioKeyWidget * rkw = new RadioKeyWidget(polygon, image, action, this);
      m_buttons.append(rkw);
      connect(rkw, &RadioKeyWidget::imageChanged, this, &ButtonsWidget::setBitmap);
      return rkw;
    }

    RadioKeyWidget * addPushButton(QPushButton * pushbtn, RadioUiAction * action = nullptr)
    {
      RadioKeyWidget * rkw = new RadioKeyWidget(pushbtn, action, this);
      m_buttons.append(rkw);
      connect(pushbtn, &QPushButton::pressed, rkw, &RadioKeyWidget::press);
      connect(pushbtn, &QPushButton::released, rkw, &RadioKeyWidget::release);

      if (action) {
        //  blink push button on click or matching key(s) press
        connect(action, static_cast<void (RadioUiAction::*)(void)>(&RadioUiAction::pushed), [this, pushbtn] (void) {
                //  TODO: use a palette colors
                //        set to default -> blink -> default
                QString csssave = pushbtn->styleSheet();
                QString blnkcol = "background-color: rgb(239, 41, 41)";
                // pressing the same key in rapid seccession can affect the order of the events see TODO
                if (csssave != blnkcol) {
                  pushbtn->setFocus();
                  pushbtn->setStyleSheet(blnkcol);
                  QTimer * tim = new QTimer(this);
                  tim->setSingleShot(true);
                  connect(tim, &QTimer::timeout, [pushbtn, csssave]() { pushbtn->setStyleSheet(csssave); });
                  tim->start(300);
                }
        });
      }
      pushbtn->setFocusPolicy(Qt::ClickFocus);
      return rkw;
    }

  protected:

    void setBitmap(QString bitmap)
    {
      QString css = defaultStyleSheet;
      if (!bitmap.isEmpty())
        css = QString("background:url(:/images/simulator/%1);").arg(bitmap);

      QWidget::setStyleSheet(css);
      setFocus();
    }

    void onMouseButtonEvent(bool press, QMouseEvent * event)
    {
      if (!(event->button() & (Qt::LeftButton | Qt::MiddleButton))) {
        event->ignore();
        return;
      }

      foreach(RadioKeyWidget * key, m_buttons) {
        if (key->contains(event->pos())) {
          key->toggle(press);
          event->accept();
          return;
        }
        else if (key->getValue()) {
          key->toggle(false);
        }
      }
      event->ignore();
    }

    virtual void mousePressEvent(QMouseEvent * event)
    {
      onMouseButtonEvent(true, event);
    }
    virtual void mouseReleaseEvent(QMouseEvent * event)
    {
      onMouseButtonEvent(false, event);
    }

    void paintEvent(QPaintEvent *)
    {
      QStyleOption opt;
      opt.init(this);
      QPainter p(this);
      style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    }

    QList<RadioKeyWidget *> m_buttons;
    QString defaultStyleSheet;
};
