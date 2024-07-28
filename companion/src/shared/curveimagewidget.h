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

#include <QLabel>

class ModelData;
class Firmware;
class CompoundItemModelFactory;

class CurveImageWidget : public QLabel
{
    Q_OBJECT

  public:
    CurveImageWidget(QWidget * parent);
    virtual ~CurveImageWidget() {}

    void set(ModelData * model, Firmware * firmware, CompoundItemModelFactory * sharedItemModels, int index, QColor penColor = Qt::black,
             qreal penWidth = 2);
    void setIndex(int index);
    void setGrid(QColor color = Qt::darkGray, qreal width = 1);
    void setPen(QColor color = Qt::black, qreal width = 2);
    int edit();
    void draw();

  signals:
    void doubleClicked();

  protected:
    virtual void mouseDoubleClickEvent(QMouseEvent * event);

  private:
    ModelData *model = nullptr;
    Firmware *firmware = nullptr;
    CompoundItemModelFactory *sharedItemModels;
    int index;
    qreal penWidth;
    QColor penColor;
    qreal gridWidth;
    QColor gridColor;
};
