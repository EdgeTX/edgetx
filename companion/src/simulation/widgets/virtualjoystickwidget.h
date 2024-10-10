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

#include <QWidget>
#include <QResizeEvent>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QToolButton>
#include <QTimer>
#include <QLabel>

class CustomGraphicsScene;
class Node;
class RadioTrimWidget;

class VirtualJoystickWidget : public QWidget
{
  Q_OBJECT

  public:
    enum ConstraintTypes {
      HOLD_X = 0x01,
      HOLD_Y = 0x02,
      FIX_X  = 0x04,
      FIX_Y  = 0x08
    };

    explicit VirtualJoystickWidget(QWidget * parent = NULL, QChar side = 'L', bool showTrims = true, bool showBtns = true, bool showValues = true, QSize size = QSize(125, 125));

    qreal getStickX();
    qreal getStickY();
    QPointF getStickPos();
    int getTrimValue(int which);
    virtual QSize sizeHint() const;
    virtual void resizeEvent(QResizeEvent *event);

    int getStickScale() const;
    void setStickScale(int stickScale);

    const RadioTrimWidget *horizontalTrim() { return hTrimWidget; }
    const RadioTrimWidget *verticalTrim() { return vTrimWidget; }

  public slots:
    void setStickX(qreal x);
    void setStickY(qreal y);
    void setStickPos(QPointF xy);
    void setStickAxisValue(int index, int value);
    void centerStick();
    void setTrimsRange(int min, int max);
    void setTrimsRange(int rng);
    void setTrimRange(int index, int min, int max);
    void setTrimValue(int index, int value);
    void setWidgetValue(const RadioWidget::RadioWidgetType type, const int index, const int value);

    void setStickIndices(int xIdx = -1, int yIdx = -1);
    void setStickConstraint(quint8 index, bool active);
    void setStickColor(const QColor & color);
    void loadDefaultsForMode(const unsigned mode);

  signals:
    void valueChange(const RadioWidget::RadioWidgetType type, const int index, int value);

  protected slots:
    void onNodeXChanged();
    void onNodeYChanged();
    void onButtonChange(bool checked);
    void updateNodeValueLabels();
    void onGsMouseEvent(QGraphicsSceneMouseEvent * event);

  protected:
    void setSize(const QSize & size, const QSize &);
    RadioTrimWidget * createTrimWidget(QChar type);
    QToolButton * createButtonWidget(int type);
    QLayout * createNodeValueLayout(QChar type, QLabel *& valLabel);
    int getStickIndex(QChar type);
    int getTrimSliderType(QChar type);
    int getTrimButtonType(QChar type, int pos);
    RadioTrimWidget * getTrimWidget(int which);

    QChar stickSide;
    QSize prefSize;
    QGridLayout * layout;
    QGraphicsView * gv;
    CustomGraphicsScene * scene;
    Node * node;
    RadioTrimWidget * hTrimWidget;
    RadioTrimWidget * vTrimWidget;
    QToolButton * btnHoldX, * btnHoldY;
    QToolButton * btnFixX, * btnFixY;
    QLabel * nodeLabelX, * nodeLabelY;
    QSize extraSize;
    QTimer centerStickTimer;
    float ar;  // aspect ratio
    int m_xIndex;
    int m_yIndex;
    int m_stickScale;
    bool m_stickPressed;
    bool isBoardSurface;
};


/*
 * Custom GraphicsScene for mouse handling
*/
class CustomGraphicsScene : public QGraphicsScene
{
  Q_OBJECT

  public:
    CustomGraphicsScene(QObject *parent = Q_NULLPTR) :
      QGraphicsScene(parent)
    {}

  signals:
    void mouseEvent(QGraphicsSceneMouseEvent * event);

  protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
};
