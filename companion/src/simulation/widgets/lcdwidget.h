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

#ifndef _LCDWIDGET_H_
#define _LCDWIDGET_H_

#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMouseEvent>

#include "appdata.h"
#include "appdebugmessagehandler.h"

#define LCD_WIDGET_REFRESH_PERIOD    16  // [ms] 16 = 62.5fps

class LcdWidget : public QWidget
{
  Q_OBJECT

  public:

    enum TouchEvent {
      TouchUp=0,
      TouchDown,
      TouchSlide
    };
  
    LcdWidget(QWidget * parent = 0):
      QWidget(parent),
      lcdBuf(NULL),
      localBuf(NULL),
      lightEnable(false),
      bgDefaultColor(QColor(198, 208, 199))
    {
    }

    ~LcdWidget()
    {
      if (localBuf) {
        free(localBuf);
      }
    }

    void setData(unsigned char *buf, int width, int height, int depth=1)
    {
      lcdBuf = buf;
      lcdWidth = width;
      lcdHeight = height;
      lcdDepth = depth;
      if (depth >= 8)
        lcdSize = (width * height) * ((depth+7) / 8);
      else
        lcdSize = (width * ((height+7)/8)) * depth;

      localBuf = (unsigned char *)malloc(lcdSize);
      memset(localBuf, 0, lcdSize);
    }

    void setBgDefaultColor(const QColor & color)
    {
      bgDefaultColor = color;
    }

    void setBackgroundColor(const QColor & color)
    {
      bgColor = color;
    }

    void makeScreenshot(const QString & fileName)
    {
      int width, height;
      if (lcdDepth < 12) {
        width = 2 * lcdWidth;
        height = 2 * lcdHeight;
      }
      else {
        width = lcdWidth;
        height = lcdHeight;
      }
      QPixmap buffer(width, height);
      QPainter p(&buffer);
      doPaint(p);
      if (fileName.isEmpty()) {
        QApplication::clipboard()->setPixmap( buffer );
        qInfo() << "Screenshot saved to clipboard";
      }
      else {
        buffer.toImage().save(fileName);
        qInfo() << "Screenshot saved to:" << fileName;
      }
    }

    void onLcdChanged(bool light)
    {
      QMutexLocker locker(&lcdMtx);
      lightEnable = light;
      memcpy(localBuf, lcdBuf, lcdSize);
      if (!redrawTimer.isValid() || redrawTimer.hasExpired(LCD_WIDGET_REFRESH_PERIOD)) {
        update();
        redrawTimer.start();
      }
    }

  signals:
    void touchEvent(int type, int x, int y);
  
  protected:

    int lcdWidth;
    int lcdHeight;
    int lcdDepth;
    int lcdSize;

    unsigned char *lcdBuf;
    unsigned char *localBuf;

    bool lightEnable;
    QColor bgColor;
    QColor bgDefaultColor;
    QMutex lcdMtx;
    QElapsedTimer redrawTimer;

    inline void doPaint(QPainter & p)
    {
      QRgb rgb;
      uint16_t z;

      if (!localBuf)
        return;

      if (lcdDepth == 16) {
        for (int x = 0; x < lcdWidth; x++) {
          for (int y = 0; y < lcdHeight; y++) {
            z = ((uint16_t *)localBuf)[y * lcdWidth + x];
            rgb = qRgb(255 * ((z & 0xF800) >> 11) / 0x1F,
                       255 * ((z & 0x07E0) >> 5)  / 0x3F,
                       255 *  (z & 0x001F)        / 0x1F);
            p.setPen(rgb);
            p.drawPoint(x, y);
          }
        }
        return;
      }
      if (lcdDepth == 12) {
        for (int x = 0; x < lcdWidth; x++) {
          for (int y = 0; y < lcdHeight; y++) {
            z = ((uint16_t *)localBuf)[y * lcdWidth + x];
            rgb = qRgb(255 * ((z & 0xF00) >> 8) / 0x0F,
                       255 * ((z & 0x0F0) >> 4) / 0x0F,
                       255 *  (z & 0x00F)       / 0x0F);
            p.setPen(rgb);
            p.drawPoint(x, y);
          }
        }
        return;
      }

      QColor bg;
      if (lightEnable)
        bg = bgColor;
      else
        bg = bgDefaultColor;

      p.setBackground(QBrush(bg));
      p.eraseRect(0, 0, 2*lcdWidth, 2*lcdHeight);

      if (lcdDepth == 1) {
        rgb = qRgb(0, 0, 0);
        p.setPen(rgb);
        p.setBrush(QBrush(rgb));
      }

      uint16_t idx, mask;
      uint16_t previousDepth = 0xFF;

      for (int y = 0; y < lcdHeight; y++) {
        idx = (y * lcdDepth / 8) * lcdWidth;
        mask = (1 << (y % 8));
        for (int x = 0; x < lcdWidth; x++, idx++) {
          if (lcdDepth == 1) {
            if (localBuf[idx] & mask)
              p.drawRect(2 * x, 2 * y, 1, 1);
            continue;
          }
          // lcdDepth == 4
          z = (y & 1) ? (localBuf[idx] >> 4) : (localBuf[idx] & 0x0F);
          if (!z)
            continue;
          if (z != previousDepth) {
            previousDepth = z;
            rgb = qRgb(bg.red()   - (z * bg.red()) / 15,
                       bg.green() - (z * bg.green()) / 15,
                       bg.blue()  - (z * bg.blue()) / 15);
            p.setPen(rgb);
            p.setBrush(QBrush(rgb));
          }
          p.drawRect(2*x, 2*y, 1, 1);
        }
      }

    }

    void paintEvent(QPaintEvent*) override
    {
      QPainter p(this);
      doPaint(p);
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
      emit touchEvent(TouchSlide, event->pos().x(), event->pos().y());
    }

    void mousePressEvent(QMouseEvent *event) override
    {
      emit touchEvent(TouchDown, event->pos().x(), event->pos().y());
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
      emit touchEvent(TouchUp, event->pos().x(), event->pos().y());
    }
};

#endif // _LCDWIDGET_H_
