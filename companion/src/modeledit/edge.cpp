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

#include <QPainter>

#include "edge.h"
#include "node.h"

#include <math.h>

Edge::Edge(Node *sourceNode, Node *destNode)
    : arrowSize(10)
{
  setAcceptedMouseButtons(Qt::NoButton);
  source = sourceNode;
  dest = destNode;
  source->addEdge(this);
  dest->addEdge(this);
  adjust();
}

Edge::~Edge()
{
}

Node *Edge::sourceNode() const
{
  return source;
}

void Edge::setSourceNode(Node *node)
{
  source = node;
  adjust();
}

Node *Edge::destNode() const
{
  return dest;
}

void Edge::setDestNode(Node *node)
{
  dest = node;
  adjust();
}

void Edge::adjust()
{
  if (!source || !dest)
    return;

  QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
  qreal length = line.length();
  prepareGeometryChange();
  int ballSize = source->getBallSize();
  if (length > qreal(ballSize)) {
    QPointF edgeOffset((line.dx() * ballSize / 2) / length, (line.dy() * ballSize / 2) / length);
    sourcePoint = line.p1() + edgeOffset;
    destPoint = line.p2() - edgeOffset;
  } else {
    sourcePoint = destPoint = line.p1();
  }
}

QRectF Edge::boundingRect() const
{
  if (!source || !dest)
    return QRectF();

  qreal penWidth = 1;
  qreal extra = (penWidth + arrowSize) / 2.0;

  return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
          destPoint.y() - sourcePoint.y()))
          .normalized()
          .adjusted(-extra, -extra, extra, extra);
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
  if (!source || !dest)
    return;

  QLineF line(sourcePoint, destPoint);
  if (qFuzzyCompare(line.length(), qreal(0.)))
    return;

  // Draw the line itself
  painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  painter->drawLine(line);
}
