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

#define GBALL_SIZE       25
#define GBALL_SIZE_MN    20
#define GBALL_SIZE_MX    35
#define CENTER_INTERVAL  50  // ms

#include "virtualjoystickwidget.h"

#include "boards.h"
#include "modeledit/node.h"
#include "helpers.h"
#include "radiotrimwidget.h"
#include "simulator.h"

VirtualJoystickWidget::VirtualJoystickWidget(QWidget *parent, QChar side, bool showTrims, bool showBtns, bool showValues, QSize size) :
  QWidget(parent),
  stickSide(side),
  prefSize(size),
  hTrimWidget(nullptr),
  vTrimWidget(nullptr),
  btnHoldX(nullptr),
  btnHoldY(nullptr),
  btnFixX(nullptr),
  btnFixY(nullptr),
  nodeLabelX(nullptr),
  nodeLabelY(nullptr),
  m_stickScale(1024),
  m_stickPressed(false)
{
  isBoardSurface = Boards::isSurface(getCurrentBoard());
  ar = (float)size.width() / size.height();
  extraSize = QSize(0, 0);

  // 5 col x 4 rows
  layout = new QGridLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setSizePolicy(sizePolicy);

  gv = new QGraphicsView(this);
  gv->setSizePolicy(sizePolicy);
  gv->setMinimumSize(size);
  gv->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  gv->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  gv->setRenderHints(QPainter::Antialiasing);

  scene = new CustomGraphicsScene(gv);
  scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  gv->setScene(scene);

  node = new Node();
  node->setPos(-GBALL_SIZE / 2, -GBALL_SIZE / 2);
  node->setBallSize(GBALL_SIZE);
  scene->addItem(node);

  int colvt, colbb, colvx, colvy;
  if (stickSide == 'L') {
    colvt = 3;
    colbb = 1;
    colvx = 1;
    colvy = 3;
  }
  else {
    colvt = 1;
    colbb = 3;
    colvx = 3;
    colvy = 1;
  }

  if (showTrims) {
    hTrimWidget = createTrimWidget('H');
    vTrimWidget = createTrimWidget('V');

    if (isBoardSurface) {
      if (stickSide == 'L')
        vTrimWidget->hide();
      else
        vTrimWidget->hide();
    }

    layout->addWidget(vTrimWidget, 1, colvt, 1, 1);
    layout->addWidget(hTrimWidget, 2, 2, 1, 1);

    extraSize += QSize(vTrimWidget->sizeHint().width(), hTrimWidget->sizeHint().height());
  }
  else {
    colvx = colvy = colvt;
  }

  if (showBtns) {
    QVBoxLayout * btnbox = new QVBoxLayout();
    btnbox->setContentsMargins(9, 9, 9, 9);
    btnbox->setSpacing(2);
    btnbox->addWidget(btnHoldY = createButtonWidget(HOLD_Y));
    btnbox->addWidget(btnFixY = createButtonWidget(FIX_Y));
    btnbox->addWidget(btnFixX = createButtonWidget(FIX_X));
    btnbox->addWidget(btnHoldX = createButtonWidget(HOLD_X));
    if (isBoardSurface) {
      if (stickSide == 'L') {
        btnHoldX->hide();
        btnFixY->hide();
        btnFixX->hide();
      }
      else {
        btnHoldY->hide();
        btnFixY->hide();
        btnFixX->hide();
      }
    }

    layout->addLayout(btnbox, 1, colbb, 1, 1);

    extraSize.setWidth(extraSize.width() + btnbox->sizeHint().width());
  }

  if (showValues) {
    QLayout * valX = createNodeValueLayout('X', nodeLabelX);
    QLayout * valY = createNodeValueLayout('Y', nodeLabelY);
    if (!showTrims) {
      QVBoxLayout * vertXY = new QVBoxLayout();
      vertXY->addLayout(valX);
      vertXY->addLayout(valY);
      layout->addLayout(vertXY, 1, colvx, 1, 1);
      extraSize += QSize(nodeLabelX->sizeHint().width(), 0);
    }
    else {
      layout->addLayout(valX, 2, colvx, 1, 1);
      layout->addLayout(valY, 2, colvy, 1, 1);
    }
  }

  layout->addItem(new QSpacerItem(0, 0), 0, 0, 1, 5);  // r0 c0-4: top v spacer
  layout->addItem(new QSpacerItem(0, 0), 1, 0, 2, 1);  // r1-2 c0: left h spacer
  layout->addWidget(gv, 1, 2, 1, 1);                   // r1 c2: stick widget
  layout->addItem(new QSpacerItem(0, 0), 1, 4, 2, 1);  // r1-2 c4: right h spacer
  layout->addItem(new QSpacerItem(0, 0), 3, 0, 1, 5);  // r4 c0-4: bot v spacer

  setStickIndices(getStickIndex('H'), getStickIndex('V'));

  connect(node, &Node::xChanged, this, &VirtualJoystickWidget::onNodeXChanged);
  connect(node, &Node::yChanged, this, &VirtualJoystickWidget::onNodeYChanged);

  connect(scene, &CustomGraphicsScene::mouseEvent, this, &VirtualJoystickWidget::onGsMouseEvent);

  setSize(prefSize, frameSize());

  centerStickTimer.setInterval(CENTER_INTERVAL);
  connect(&centerStickTimer, &QTimer::timeout, this, &VirtualJoystickWidget::centerStick);
  centerStickTimer.start();
}

void VirtualJoystickWidget::setStickX(qreal x)
{
  node->setX(x);
}

void VirtualJoystickWidget::setStickY(qreal y)
{
  node->setY(y);
}

void VirtualJoystickWidget::setStickPos(QPointF xy)
{
  node->setPos(xy);
}

void VirtualJoystickWidget::setStickAxisValue(int index, int value)
{
  using namespace Board;
  qreal rvalue = value / 1024.0f;

  if (isBoardSurface) {
    switch (index) {
      case STICK_AXIS_SURFACE_LV :
        if (stickSide == 'L')
          setStickY(rvalue);
        break;
      case STICK_AXIS_SURFACE_RH :
        if (stickSide == 'R')
          setStickX(rvalue);
        break;
    }
  }
  else {
    switch (index) {
      case STICK_AXIS_LH :
        if (stickSide == 'L')
          setStickX(rvalue);
        break;
      case STICK_AXIS_RH :
        if (stickSide == 'R')
          setStickX(rvalue);
        break;
      case STICK_AXIS_LV :
        if (stickSide == 'L')
          setStickY(rvalue);
        break;
      case STICK_AXIS_RV :
        if (stickSide == 'R')
          setStickY(rvalue);
        break;
    }
  }
}

void VirtualJoystickWidget::centerStick()
{
  node->stepToCenter();
}

qreal VirtualJoystickWidget::getStickX()
{
  return getStickPos().x();
}

qreal VirtualJoystickWidget::getStickY()
{
  return getStickPos().y();
}

QPointF VirtualJoystickWidget::getStickPos()
{
  return QPointF(node->getX(), node->getY());
}

void VirtualJoystickWidget::setTrimsRange(int min, int max)
{
  if (hTrimWidget)
    hTrimWidget->setTrimRange(min, max);
  if (vTrimWidget)
    vTrimWidget->setTrimRange(min, max);
}

void VirtualJoystickWidget::setTrimsRange(int rng)
{
  setTrimsRange(-rng, rng);
}

void VirtualJoystickWidget::setTrimRange(int index, int min, int max)
{
  if (index == Board::TRIM_AXIS_COUNT) {
    setTrimsRange(min, max);
    return;
  }
  RadioTrimWidget * trim = getTrimWidget(index);
  if (trim) {
    trim->setTrimRange(min, max);
  }
}

void VirtualJoystickWidget::setTrimValue(int index, int value)
{
  RadioTrimWidget * trim = getTrimWidget(index);
  if (trim) {
    trim->setValue(value);
  }
}

int VirtualJoystickWidget::getStickScale() const
{
  return m_stickScale;
}

void VirtualJoystickWidget::setStickScale(int stickScale)
{
  m_stickScale = stickScale;
}

void VirtualJoystickWidget::setWidgetValue(const RadioWidget::RadioWidgetType type, const int index, const int value)
{
  if (type == RadioWidget::RADIO_WIDGET_TRIM)
    setTrimValue(index, value);
  else if (type == RadioWidget::RADIO_WIDGET_STICK)
    setStickAxisValue(index, value);
}

int VirtualJoystickWidget::getTrimValue(int which)
{
  RadioTrimWidget * trim = getTrimWidget(which);
  if (trim) {
    return trim->getValue();
  }
  return 0;
}

void VirtualJoystickWidget::setStickIndices(int xIdx, int yIdx)
{
  m_xIndex = xIdx;
  m_yIndex = yIdx;
}

void VirtualJoystickWidget::setStickConstraint(quint8 index, bool active)
{
  if (btnHoldX == NULL || !index)
    return;  // no buttons

  if (index & HOLD_X)
    btnHoldX->setChecked(active);
  if (index & HOLD_Y)
    btnHoldY->setChecked(active);
  if (index & FIX_X)
    btnFixX->setChecked(active);
  if (index & FIX_Y)
    btnFixY->setChecked(active);
}

void VirtualJoystickWidget::setStickColor(const QColor & color)
{
  node->setColor(color);
}

void VirtualJoystickWidget::loadDefaultsForMode(const unsigned mode)
{
  if (isBoardSurface) {
    if (stickSide == 'L') {
      setStickX(0.0);
      setStickConstraint(FIX_X, true);
      onNodeXChanged();
    }
    else {
      setStickY(0.0);
      setStickConstraint(FIX_Y, true);
      onNodeYChanged();
    }
  }
  else {
    if (((mode & 1) && stickSide == 'L') || (!(mode & 1) && stickSide == 'R')) {
      setStickConstraint(HOLD_Y, true);
      setStickY(-1.0);
      onNodeYChanged();
    }
  }
}

QSize VirtualJoystickWidget::sizeHint() const {
  return prefSize;
}

void VirtualJoystickWidget::resizeEvent(QResizeEvent * event)
{
  QWidget::resizeEvent(event);
  setSize(event->size(), event->oldSize());
}

void VirtualJoystickWidget::onNodeXChanged()
{
  emit valueChange(RadioWidget::RADIO_WIDGET_STICK, m_xIndex, int(m_stickScale * getStickX()));
  updateNodeValueLabels();
}

void VirtualJoystickWidget::onNodeYChanged()
{
  emit valueChange(RadioWidget::RADIO_WIDGET_STICK, m_yIndex, int(m_stickScale * getStickY()));
  updateNodeValueLabels();
}

void VirtualJoystickWidget::setSize(const QSize & size, const QSize &)
{
  float thisAspectRatio = (float)size.width() / size.height();
  float newGvSz, spacerSz;
  bool sized = false;

  if (thisAspectRatio > ar) {
    // constrain width
    newGvSz = (height() - extraSize.height()) * ar; // new width
    spacerSz = (width() - newGvSz - extraSize.width()) / 2; // + 0.5;
    if (spacerSz >= 0.0f) {
      layout->setRowStretch(0, 0);
      layout->setColumnStretch(0, spacerSz);
      layout->setColumnStretch(4, spacerSz);
      layout->setRowStretch(3, 0);
      sized = true;
    }
  }
  if (!sized) {
    // constrain height
    newGvSz = (width() - extraSize.width()) * ar; // new height
    spacerSz = (height() - newGvSz - extraSize.height()) / 2; // + 0.5;
    spacerSz = qMax(spacerSz, 0.0f);
    layout->setRowStretch(0, spacerSz);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(4, 0);
    layout->setRowStretch(3, spacerSz);
  }

  layout->setColumnStretch(2, newGvSz);
  layout->setRowStretch(1, newGvSz);

  gv->resize(newGvSz, newGvSz);
  gv->updateGeometry();

  int ballSize = (newGvSz * GBALL_SIZE * 0.005f);
  ballSize = qMin(GBALL_SIZE_MX, qMax(ballSize, GBALL_SIZE_MN));

  QRectF qr = (QRectF)gv->contentsRect();
  qreal w  = qr.width()  - ballSize;
  qreal h  = qr.height() - ballSize;
  qreal cx = qr.width() / 2;
  qreal cy = qr.height() / 2;
  qreal nodeX = node->getX();
  qreal nodeY = node->getY();

  scene->setSceneRect(-cx,-cy,w,h);

  node->setBallSize(ballSize);
  node->setX(nodeX);
  node->setY(nodeY);

  //qDebug() << thisAspectRatio << size << newGvSz << spacerSz << extraSize << gv->geometry() << gv->contentsRect() << gv->frameRect() << getStickPos();
}

RadioTrimWidget * VirtualJoystickWidget::createTrimWidget(QChar type)
{
  RadioTrimWidget * trimWidget = new RadioTrimWidget(type == 'H' ? Qt::Horizontal : Qt::Vertical);
  trimWidget->setIndices(getTrimSliderType(type), getTrimButtonType(type, 0), getTrimButtonType(type, 1));

  connect(trimWidget, &RadioTrimWidget::valueChange, this, &VirtualJoystickWidget::valueChange);

  return trimWidget;
}

QToolButton * VirtualJoystickWidget::createButtonWidget(int type)
{
  QString btnLabel, tooltip;
  QIcon icon;
  switch (type) {
    case HOLD_Y:
      btnLabel = tr("Hld Y");
      tooltip = tr("Hold Vertical position.");
      icon = Simulator::SimulatorIcon("hold_y");
      break;
    case FIX_Y:
      btnLabel = tr("Fix Y");
      tooltip = tr("Prevent Vertical movement.");
      icon = Simulator::SimulatorIcon("fixed_y");
      break;
    case FIX_X:
      btnLabel = tr("Fix X");
      tooltip = tr("Prevent Horizontal movement.");
      icon = Simulator::SimulatorIcon("fixed_x");
      break;
    case HOLD_X:
      btnLabel = tr("Hld X");
      tooltip = tr("Hold Horizontal position.");
      icon = Simulator::SimulatorIcon("hold_x");
      break;
    default:
      return NULL;
  }
  QToolButton * btn = new QToolButton(this);
  btn->setProperty("btnType", type);
  btn->setIcon(icon);
  btn->setIconSize(QSize(20, 20));
  btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
  btn->setText(btnLabel);
  btn->setToolTip(tooltip);
  btn->setCheckable(true);
  btn->setAutoRaise(true);

  connect(btn, &QToolButton::toggled, this, &VirtualJoystickWidget::onButtonChange);

  return btn;
}

QLayout *VirtualJoystickWidget::createNodeValueLayout(QChar type, QLabel *& valLabel)
{
  QLabel * lbl = new QLabel(QString("%1 %").arg(type), this);
  lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  lbl->setAlignment(Qt::AlignCenter);
  QLabel * val = new QLabel("0");
  val->setObjectName(QString("val_%1").arg(type));
  val->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  val->setAlignment(Qt::AlignCenter);
  val->setMinimumWidth(val->fontMetrics().horizontalAdvance("-100 "));
  QVBoxLayout * layout = new QVBoxLayout();
  layout->setContentsMargins(2, 2, 2, 2);
  layout->setSpacing(2);
  layout->addWidget(lbl);
  layout->addWidget(val);

  valLabel = val;
  return layout;
}

int VirtualJoystickWidget::getStickIndex(QChar type)
{
  using namespace Board;

  if (isBoardSurface) {
    if (stickSide == 'L') {
      if (type == 'H')
        return STICK_AXIS_SURFACE_LV;
      else
        return STICK_AXIS_SURFACE_LV;
    }
    else {
      if (type == 'H')
        return STICK_AXIS_SURFACE_RH;
      else
        return STICK_AXIS_SURFACE_RH;
    }
  }
  else {
    if (stickSide == 'L') {
      if (type == 'H')
        return STICK_AXIS_LH;
      else
        return STICK_AXIS_LV;
    }
    else {
      if (type == 'H')
        return STICK_AXIS_RH;
      else
        return STICK_AXIS_RV;
    }
  }
}

int VirtualJoystickWidget::getTrimSliderType(QChar type)
{
  using namespace Board;

  if (isBoardSurface) {
    if (stickSide == 'L') {
      if (type == 'H')
        return TRIM_AXIS_SURFACE_LH;
      else
        return TRIM_AXIS_SURFACE_LH;
    }
    else {
      if (type == 'H')
        return TRIM_AXIS_SURFACE_RH;
      else
        return TRIM_AXIS_SURFACE_RH;
    }
  }
  else {
    if (stickSide == 'L') {
      if (type == 'H')
        return TRIM_AXIS_LH;
      else
        return TRIM_AXIS_LV;
    }
    else {
      if (type == 'H')
        return TRIM_AXIS_RH;
      else
        return TRIM_AXIS_RV;
    }
  }
}

int VirtualJoystickWidget::getTrimButtonType(QChar type, int pos)
{
  using namespace Board;

  if (isBoardSurface) {
    if (stickSide == 'L') {
      if (type == 'H') {
        if (pos == 0)
          return TRIM_SW_SURFACE_LH_DEC;
        else
          return TRIM_SW_SURFACE_LH_INC;
      }
      else {
        if (pos == 0)
          return TRIM_SW_SURFACE_LH_DEC;
        else
          return TRIM_SW_SURFACE_LH_INC;
      }
    }
    // right side
    else {
      if (type == 'H') {
        if (pos == 0)
          return TRIM_SW_SURFACE_RH_DEC;
        else
          return TRIM_SW_SURFACE_RH_INC;
      }
      else {
        if (pos == 0)
          return TRIM_SW_SURFACE_RH_DEC;
        else
          return TRIM_SW_SURFACE_RH_INC;
      }
    }
  }
  else {
    if (stickSide == 'L') {
      if (type == 'H') {
        if (pos == 0)
          return TRIM_SW_LH_DEC;
        else
          return TRIM_SW_LH_INC;
      }
      else {
        if (pos == 0)
          return TRIM_SW_LV_DEC;
        else
          return TRIM_SW_LV_INC;
      }
    }
    // right side
    else {
      if (type == 'H') {
        if (pos == 0)
          return TRIM_SW_RH_DEC;
        else
          return TRIM_SW_RH_INC;
      }
      else {
        if (pos == 0)
          return TRIM_SW_RV_DEC;
        else
          return TRIM_SW_RV_INC;
      }
    }
  }
}

RadioTrimWidget * VirtualJoystickWidget::getTrimWidget(int which)
{
  if (isBoardSurface) {
    if ((stickSide == 'L' && which == Board::TRIM_AXIS_SURFACE_LH) || (stickSide == 'R' && which == Board::TRIM_AXIS_SURFACE_RH))
      return hTrimWidget;
    else
      return nullptr;
  }
  else {
    if ((stickSide == 'L' && which == Board::TRIM_AXIS_LH) || (stickSide == 'R' && which == Board::TRIM_AXIS_RH))
      return hTrimWidget;
    else if ((stickSide == 'L' && which == Board::TRIM_AXIS_LV) || (stickSide == 'R' && which == Board::TRIM_AXIS_RV))
      return vTrimWidget;
    else
      return nullptr;
  }
}

void VirtualJoystickWidget::onButtonChange(bool checked)
{
  if (!sender() || !sender()->property("btnType").isValid())
    return;

  switch (sender()->property("btnType").toInt()) {
    case HOLD_Y:
      node->setCenteringY(!checked);
      break;
    case FIX_Y:
      node->setFixedY(checked);
      break;
    case FIX_X:
      node->setFixedX(checked);
      break;
    case HOLD_X:
      node->setCenteringX(!checked);
      break;
  }
}

void VirtualJoystickWidget::updateNodeValueLabels()
{
  if (isBoardSurface) {
    if (nodeLabelX) {
      if (stickSide == 'L')
        nodeLabelX->setText(QString("%1").arg(0));  // workaround for resize event and no analog mapped to set
      else
        nodeLabelX->setText(QString("%1").arg(getStickX() * 100.0f, 2, 'f', 0));
    }
    if (nodeLabelY) {
      if (stickSide == 'L')
        nodeLabelY->setText(QString("%1").arg(getStickY() * 100.0f, 2, 'f', 0));
      else
        nodeLabelY->setText(QString("%1").arg(0));  // workaround for resize event and no analog mapped to set
    }
  }
  else {
    if (nodeLabelX)
      nodeLabelX->setText(QString("%1").arg(getStickX() * 100.0f, 2, 'f', 0));
    if (nodeLabelY)
      nodeLabelY->setText(QString("%1").arg(getStickY() * 100.0f, 2, 'f', 0));
  }
}

void VirtualJoystickWidget::onGsMouseEvent(QGraphicsSceneMouseEvent * event)
{
  if (!node)
    return;

  //qDebug() << event->type() << event->scenePos() << event->buttons() << event->isAccepted() << m_stickPressed;
  if (event->type() == QEvent::GraphicsSceneMouseRelease && m_stickPressed) {
    node->setPressed(false);
    m_stickPressed = false;
    return;
  }

  if (!(event->buttons() & Qt::LeftButton))
    return;

  if (event->type() == QEvent::GraphicsSceneMousePress) {
    node->setPressed(true);
    m_stickPressed = true;
  }
  else if (!m_stickPressed || event->type() != QEvent::GraphicsSceneMouseMove) {
    return;
  }
  node->setPos(event->scenePos());
}


/*
 *  CustomGraphicsScene
*/

void CustomGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsScene::mousePressEvent(event);
  if (!event->isAccepted()) {
    event->accept();
    emit mouseEvent(event);
  }
}

void CustomGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsScene::mouseReleaseEvent(event);
  emit mouseEvent(event);
}

void CustomGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsScene::mouseMoveEvent(event);
  if (!event->isAccepted() && (event->buttons() & Qt::LeftButton)) {
    event->accept();
    emit mouseEvent(event);
  }
}
