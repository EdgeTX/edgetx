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

#include "curveimagewidget.h"
#include "curveimage.h"
#include "curvedialog.h"
#include "compounditemmodels.h"
#include "modeldata.h"
#include "eeprominterface.h"

CurveImageWidget::CurveImageWidget(QWidget * parent) :
  QLabel(parent)
{
  setToolTip(tr("Double click to edit"));
}

void CurveImageWidget::set(ModelData * model, Firmware * firmware, CompoundItemModelFactory * sharedItemModels, int index, QColor penColor, qreal penWidth)
{
  this->model = model;
  this->firmware = firmware;
  this->sharedItemModels = sharedItemModels;
  this->penWidth = penWidth;
  this->penColor = penColor;
  setIndex(index);
  setGrid();
}

void CurveImageWidget::setIndex(int index)
{
  this->index = index;
}

void CurveImageWidget::setGrid(QColor color, qreal width)
{
  gridColor = color;
  gridWidth = width;
}

void CurveImageWidget::setPen(QColor color, qreal width)
{
  penColor = color;
  penWidth = width;
}

int CurveImageWidget::edit()
{
  int ret = 0;

  if (abs(index) > 0 && abs(index) <= CPN_MAX_CURVES) {
    CurveDialog *dlg = new CurveDialog(this, *model, abs(index) - 1, firmware, sharedItemModels);
    ret = dlg->exec();
    delete dlg;
  }

  return ret;
}

void CurveImageWidget::draw()
{
  if (abs(index) > 0 && abs(index) <= CPN_MAX_CURVES) {
    CurveImage *curveImage = new CurveImage(gridColor, gridWidth);
    curveImage->drawCurve(model->curves[abs(index) - 1], penColor, penWidth);

    QImage image = curveImage->get();
    if (index < 0)
      image = image.mirrored(true, false);
    setPixmap(QPixmap::fromImage(image.scaled(height(), width())));

    delete curveImage;
  }
  else {
    CurveImage *curveImage = new CurveImage(gridColor, gridWidth);
    QImage image = curveImage->get();
    setPixmap(QPixmap::fromImage(image.scaled(height(), width())));
  }
}

void CurveImageWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
  QLabel::mouseDoubleClickEvent(event);
  emit doubleClicked();
}
