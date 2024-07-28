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

#ifndef _NODE_H_
#define _NODE_H_

#include <QtWidgets>

#define DEFAULT_BALL_SIZE 10
#define DEFAULT_BALL_HEIGHT 2

class Edge;
QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class Node : public QGraphicsObject
{
  Q_OBJECT

  public:
    Node();

    void addEdge(Edge *edge);
    QList<Edge *> edges() const;

    enum { Type = UserType + 1 };
    int type() const { return Type; }
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setBallSize(int size);
    void stepToCenter(qreal step=10);
    int  getBallSize() { return ballSize; }
    qreal getX();
    qreal getY();
    void setX(qreal newX);
    void setY(qreal newX);

    void setCenteringX(bool val) { centerX = val; }
    void setCenteringY(bool val) { centerY = val; }
    void setFixedX(bool val) { fixedX = val; }
    void setFixedY(bool val) { fixedY = val; }
    bool getFixedX() { return fixedX; }
    bool getFixedY() { return fixedY; }
    void setMinX(int val) { minX = val; }
    void setMaxX(int val) { maxX = val; }
    void setPressed(bool pressed) { bPressed = pressed; }
    bool isPressed() const { return bPressed; }
    void setColor(const QColor & color);
    void setBallHeight(int height);
    int geBallHeight() { return ballHeight; }

  signals:
    void moved(int x, int y);
    void focus();
    void unfocus();
    void deleteMe();

  protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

  private:
    bool bPressed;
    bool centerX;
    bool centerY;
    bool fixedX;
    bool fixedY;
    int  ballSize;
    int minX;
    int maxX;
    QList<Edge *> edgeList;
    QPointF newPos;
    QColor nodecolor;
    int ballHeight;
};

#endif // _NODE_H_
