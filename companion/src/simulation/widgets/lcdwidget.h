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

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMouseEvent>
#include <AppDebugMessageHandler>

#include "appdata.h"

#define LCD_WIDGET_REFRESH_PERIOD    16  // [ms] 16 = 62.5fps

class LcdWidget : public QWidget
{
  Q_OBJECT

 public:
  enum TouchEvent { TouchUp = 0, TouchDown, TouchSlide };

  LcdWidget(QWidget *parent = 0) :
      QWidget(parent),
      localBuf(NULL),
      lightEnable(false),
      bgDefaultColor(QColor(198, 208, 199)),
      fgDefaultColor(QColor(0, 0, 0))
  {
  }

  ~LcdWidget()
  {
    if (localBuf) {
      free(localBuf);
    }
  }

  void setData(int width, int height, int depth = 1);
  void setBgDefaultColor(const QColor &color);
  void setFgDefaultColor(const QColor &color);
  void setBackgroundColor(const QColor &color);

  void makeScreenshot(const QString &fileName);

  void onLcdChanged(uint8_t* lcdBuf, bool light);

 signals:
  void touchEvent(int type, int x, int y);

 protected:
  int lcdWidth;
  int lcdHeight;
  int lcdDepth;
  int lcdSize;

  unsigned char *localBuf;

  bool lightEnable;
  QColor bgColor;
  QColor bgDefaultColor;
  QColor fgDefaultColor;
  QMutex lcdMtx;
  QElapsedTimer redrawTimer;

  void doPaint(QPainter &p);

  void paintEvent(QPaintEvent *) override;

  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
};
