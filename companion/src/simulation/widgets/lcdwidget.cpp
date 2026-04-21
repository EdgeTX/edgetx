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

#include "lcdwidget.h"

void LcdWidget::setData(int width, int height, int depth)
{
  lcdWidth = width;
  lcdHeight = height;
  lcdDepth = depth;
  if (depth >= 8)
    lcdSize = (width * height) * ((depth + 7) / 8);
  else
    lcdSize = (width * ((height + 7) / 8)) * depth;

  localBuf = (unsigned char *)malloc(lcdSize);
  memset(localBuf, 0, lcdSize);
}

void LcdWidget::setBgDefaultColor(const QColor &color)
{
  bgDefaultColor = color;
}

void LcdWidget::setFgDefaultColor(const QColor &color)
{
  fgDefaultColor = color;
}

void LcdWidget::setBackgroundColor(const QColor &color) { bgColor = color; }

void LcdWidget::makeScreenshot(const QString &fileName)
{
  int width, height;
  if (lcdDepth < 12) {
    width = 2 * lcdWidth;
    height = 2 * lcdHeight;
  } else {
    width = lcdWidth;
    height = lcdHeight;
  }
  QPixmap buffer(width, height);
  QPainter p(&buffer);
  doPaint(p);
  if (fileName.isEmpty()) {
    QApplication::clipboard()->setPixmap(buffer);
    qInfo() << "Screenshot saved to clipboard";
  } else {
    buffer.toImage().save(fileName);
    qInfo() << "Screenshot saved to:" << fileName;
  }
}

void LcdWidget::onLcdChanged(uint8_t* lcdBuf, bool light)
{
  QMutexLocker locker(&lcdMtx);
  lightEnable = light;
  if (lcdBuf) memcpy(localBuf, lcdBuf, lcdSize);
  if (!redrawTimer.isValid() ||
      redrawTimer.hasExpired(LCD_WIDGET_REFRESH_PERIOD)) {
    update();
    redrawTimer.start();
  }
}

void LcdWidget::doPaint(QPainter &p)
{
  if (!localBuf) return;

  if (lcdDepth == 16) {
    QImage img((const uchar*)localBuf, lcdWidth, lcdHeight,
               lcdWidth * 2, QImage::Format_RGB16);
    p.drawImage(0, 0, img);
    return;
  }

  if (lcdDepth == 12) {
    QImage img(lcdWidth, lcdHeight, QImage::Format_RGB32);
    for (int y = 0; y < lcdHeight; y++) {
      QRgb* line = (QRgb*)img.scanLine(y);
      for (int x = 0; x < lcdWidth; x++) {
        uint16_t z = ((uint16_t *)localBuf)[y * lcdWidth + x];
        line[x] = qRgb(255 * ((z & 0xF00) >> 8) / 0x0F,
                        255 * ((z & 0x0F0) >> 4) / 0x0F,
                        255 * (z & 0x00F) / 0x0F);
      }
    }
    p.drawImage(0, 0, img);
    return;
  }

  QColor bg = lightEnable ? bgColor : bgDefaultColor;
  QRgb bgRgb = bg.rgb();

  if (lcdDepth == 1) {
    QRgb fgRgb = fgDefaultColor.rgb();
    QImage img(lcdWidth, lcdHeight, QImage::Format_RGB32);
    for (int y = 0; y < lcdHeight; y++) {
      QRgb* line = (QRgb*)img.scanLine(y);
      uint16_t idx = (y / 8) * lcdWidth;
      uint8_t mask = 1 << (y % 8);
      for (int x = 0; x < lcdWidth; x++, idx++) {
        line[x] = (localBuf[idx] & mask) ? fgRgb : bgRgb;
      }
    }
    p.drawImage(QRect(0, 0, 2 * lcdWidth, 2 * lcdHeight), img);
    return;
  }

  // lcdDepth == 4
  int bgR = bg.red(), bgG = bg.green(), bgB = bg.blue();
  QImage img(lcdWidth, lcdHeight, QImage::Format_RGB32);
  for (int y = 0; y < lcdHeight; y++) {
    QRgb* line = (QRgb*)img.scanLine(y);
    uint16_t idx = (y / 2) * lcdWidth;
    for (int x = 0; x < lcdWidth; x++, idx++) {
      uint8_t z = (y & 1) ? (localBuf[idx] >> 4) : (localBuf[idx] & 0x0F);
      line[x] = qRgb(bgR - (z * bgR) / 15,
                      bgG - (z * bgG) / 15,
                      bgB - (z * bgB) / 15);
    }
  }
  p.drawImage(QRect(0, 0, 2 * lcdWidth, 2 * lcdHeight), img);
}

void LcdWidget::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  doPaint(p);
}

void LcdWidget::mouseMoveEvent(QMouseEvent *event)
{
  emit touchEvent(TouchSlide, event->pos().x(), event->pos().y());
}

void LcdWidget::mousePressEvent(QMouseEvent *event)
{
  emit touchEvent(TouchDown, event->pos().x(), event->pos().y());
}

void LcdWidget::mouseReleaseEvent(QMouseEvent *event)
{
  emit touchEvent(TouchUp, event->pos().x(), event->pos().y());
}
