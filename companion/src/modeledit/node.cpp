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

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "edge.h"
#include "node.h"

Node::Node():
  minX(-100),
  maxX(+100)
{
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  nodecolor = QColor(Qt::yellow);
  bPressed  = false;
  centerX   = true;
  centerY   = true;
  fixedX    = false;
  fixedY    = false;
  ballSize = DEFAULT_BALL_SIZE;
  ballHeight = DEFAULT_BALL_HEIGHT;
}

void Node::addEdge(Edge *edge)
{
  edgeList << edge;
  edge->adjust();
}

void Node::stepToCenter(qreal step)
{
    if(scene() && !bPressed)
    {
        QRectF qr = scene()->sceneRect();
        qreal xStep = qr.width()/step;
        qreal yStep = qr.height()/step;

        QPointF p = pos();
        if(centerX)
        {
            qreal x = qr.right() - qr.width()/2;
            if(fabs(x-p.x())>xStep)
                p.rx() += p.x()>x ? -xStep : xStep;
            else
                p.rx() = x;
        }

        if(centerY)
        {
            qreal y = qr.bottom() - qr.height()/2;
            if(fabs(y-p.y())>yStep)
                p.ry() += p.y()>y ? -yStep : yStep;
            else
                p.ry() = y;
        }
        setPos(p);
    }

}

void Node::setColor(const QColor & color)
{
    nodecolor=color;
}


void Node::setBallSize(int size)
{
    if(size>2) ballSize = size;
}

qreal Node::getX()
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        return 1+((x()-rect.right())*2/rect.width());
    }
    return 0;
}

void Node::setX(qreal newX)
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        QGraphicsObject::setX(rect.left() + (newX+1) * rect.width()/2);
    }
}

qreal Node::getY()
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        return ((rect.bottom()-y())*2/rect.height())-1;
    }
    return 0;
}

void Node::setY(qreal newY)
{
    if(scene())
    {
        QRectF rect = scene()->sceneRect();
        QGraphicsObject::setY(rect.top() + (1-newY) * rect.height()/2);
    }
}

QList<Edge *> Node::edges() const
{
    return edgeList;
}

QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF(-ballSize/2 - adjust, -ballSize/2 - adjust,
                  ballSize+ballHeight + adjust, ballSize+ballHeight + adjust);
}


QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-ballSize/2, -ballSize/2, ballSize, ballSize);
    return path;
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-ballSize/2 + ballHeight, -ballSize/2 + ballHeight, ballSize, ballSize);

    QRadialGradient gradient(-ballHeight, -ballHeight, ballSize/2);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(ballHeight, ballHeight);
        gradient.setFocalPoint(ballHeight, ballHeight);
        gradient.setColorAt(1, nodecolor.lighter(180));
        gradient.setColorAt(0, nodecolor.lighter(120));
    } else {
        gradient.setColorAt(0, nodecolor);
        gradient.setColorAt(1, nodecolor.darker(120));
    }
    painter->setBrush(gradient);
    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-ballSize/2, -ballSize/2, ballSize, ballSize);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionChange:
        if (scene())
        {
             // value is the new position.
             QPointF newPos = value.toPointF();
             QRectF rect = scene()->sceneRect();
             if(fixedX) newPos.setX(x());//make sure x doesn't change
             if(fixedY) newPos.setY(y());//make sure x doesn't change
             newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));// bound X
             newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));// bound Y

             int newX = -100 + ( (newPos.x()-rect.left()) * 200) / rect.width();
             int newY = 100+(rect.top()-y())*200/rect.height();
             if (newX < minX) newX = minX;
             if (newX > maxX) newX = maxX;

             if (!getFixedX()) {
               newPos.setX(((newX+100)*rect.width()/200+rect.left()));
             }

             emit moved(newX, newY);
             return newPos;
         }
        break;
    case ItemPositionHasChanged:
        foreach (Edge *edge, edgeList)
          edge->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    bPressed = true;
    QGraphicsItem::mousePressEvent(event);
    emit focus();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
    bPressed = false;

    if (scene()) {
      QGraphicsItem::mouseReleaseEvent(event);
      if (event->button() == Qt::LeftButton && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        emit deleteMe();
      }
      else {
        emit unfocus();
      }
    }
    else {
      QGraphicsItem::mouseReleaseEvent(event);
    }
}

void Node::setBallHeight(int height)
{
    ballHeight = height;
}

