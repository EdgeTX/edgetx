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

#include "constants.h"
#include "modeldata.h"

#include <QtWidgets>
#include <QGraphicsScene>
#include <QGraphicsView>

class Firmware;
class FilteredItemModelFactory;
class CompoundItemModelFactory;

namespace Ui {
  class CurveDialog;
}

typedef float (*curveFunction) (float x, float coeff, float yMin, float yMid, float yMax);

struct CurveCreatorTemplate {
  QString name;
  unsigned int flags;
  curveFunction function;
};

class CustomScene : public QGraphicsScene
{
  Q_OBJECT

  public:
    CustomScene(QGraphicsView * view);

  signals:
    void newPoint(int, int);

  protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;
};

class CurveDialog : public QDialog
{
    Q_OBJECT

  public:
    CurveDialog(QWidget *parent, ModelData & model, const int curveIdx, Firmware * firmware, CompoundItemModelFactory * sharedItemModels);
    virtual ~CurveDialog();

  private slots:
    void shrink();
    void on_curveNameChanged();
    void on_curvePointsChanged(int index);
    void on_curveTypeChanged(int index);
    void onPointEdited(int i);
    void onNodeMoved(int x, int y);
    void onNodeFocus();
    void onNodeUnfocus();
    void on_curveTemplateChanged(int index);
    void on_curveApplyClicked();
    void onSceneNewPoint(int x, int y);
    void onNodeDelete();
    void onDialogAccepted();

  protected:
    virtual void resizeEvent(QResizeEvent *event);
    void addTemplate(QString name, unsigned int flags, curveFunction function);

  private:
    Ui::CurveDialog *ui;
    ModelData &model;
    CurveData curve;  // copy initialised in constructor and copied back to model if dialog is accepted
    const int curveIdx;
    CompoundItemModelFactory *sharedItemModels;

    QSpinBox *spnx[CPN_MAX_POINTS];
    QSpinBox *spny[CPN_MAX_POINTS];
    QVector<CurveCreatorTemplate> templates;
    CustomScene *scene;
    int maxCurves;
    int maxPoints;
    bool lock;
    FilteredItemModelFactory * dialogFilterFactory;

    void update();
    void updateCurve();
    void updateCurveType();
    void updateCurvePoints();
    bool allowCurveType(int points, CurveData::CurveType type);
    void setPointY(int i, int x, int y);
};
