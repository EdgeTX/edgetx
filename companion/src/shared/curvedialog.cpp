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

#include "curvedialog.h"
#include "ui_curvedialog.h"
#include "../modeledit/node.h"
#include "../modeledit/edge.h"
#include "helpers.h"
#include "filtereditemmodels.h"
#include "eeprominterface.h"
#include "namevalidator.h"

float curveLinear(float x, float coeff, float yMin, float yMid, float yMax)
{
  float a = (yMax - yMin) / 200.0;
  return yMin + a * (x + 100.0);
}

float curveexpou(float point, float coeff)
{
  float x = point * 1024.0 / 100.0;
  float k = coeff * 256.0 / 100.0;
  return ((k * x * x * x / (1024 * 1024) + x * (256 - k) + 128) / 256) / 1024.0 * 100;
}

float curveExpo(float x, float coeff, float yMin, float yMid, float yMax)
{
  float a = (yMax - yMin) / 100.0;

  x += 100.0;
  x /= 2.0;

  if (coeff >= 0) {
    return round(curveexpou(x, coeff)*a + yMin);
  }
  else {
    coeff = -coeff;
    x = 100 - x;
    return round((100.0 - curveexpou(x, coeff)) * a + yMin);
  }
}

float curveSymmetricalY(float x, float coeff, float yMin, float yMid, float yMax)
{
  bool invert;
  if (x < 0) {
    x = -x;
    invert = 1;
  }
  else {
    invert = 0;
  }

  float y;
  if (coeff >= 0) {
    y = round(curveexpou(x, coeff) * (yMax / 100.0));
  }
  else {
    coeff = -coeff;
    x = 100.0 - x;
    y = round((100.0 - curveexpou(x, coeff)) * (yMax / 100.0));
  }

  if (invert) {
    y = -y;
  }

  return y;
}

float curveSymmetricalX(float x, float coeff, float yMin, float yMid, float yMax)
{
  float a = (yMax - yMid) / 100.0;

  if (x < 0)
    x = -x;

  float y;
  if (coeff >= 0) {
    y = round(curveexpou(x, coeff) * a + yMid);
  }
  else {
    coeff = -coeff;
    x = 100 - x;
    y = round((100.0 - curveexpou(x, coeff)) * a + yMid);
  }

  return y;
}

#define GFX_MARGIN 16

#define CURVE_COEFF_ENABLE   1
#define CURVE_YMID_ENABLE    2
#define CURVE_YMIN_ENABLE    4

/*
    CurveDialog
*/

CurveDialog::CurveDialog(QWidget * parent, ModelData & model, const int curveIdx, Firmware * firmware,
                         CompoundItemModelFactory * sharedItemModels) :
  QDialog(parent),
  ui(new Ui::CurveDialog),
  model(model),
  curve(model.curves[curveIdx]),
  curveIdx(curveIdx),
  sharedItemModels(sharedItemModels)
{
  ui->setupUi(this);

  if (curve.isEmpty())
    curve.init();

  lock = true;

  maxCurves = firmware->getCapability(NumCurves);
  maxPoints = firmware->getCapability(NumCurvePoints);

  ui->curveName->setValidator(new NameValidator(firmware->getBoard(), this));
  ui->curveName->setField(curve.name, CURVEDATA_NAME_LEN);
  connect(ui->curveName, &AutoLineEdit::currentDataChanged, this, &CurveDialog::on_curveNameChanged);
  on_curveNameChanged();

  dialogFilterFactory = new FilteredItemModelFactory();
  int id = dialogFilterFactory->registerItemModel(new FilteredItemModel(CurveData::typeItemModel()),
                                                         "Curve Type");
  ui->curveType->setModel(dialogFilterFactory->getItemModel(id));
  ui->curveType->setField((int &)curve.type);
  connect(ui->curveType, &AutoComboBox::currentDataChanged, this, [&] () { on_curveTypeChanged(ui->curveType->currentIndex()) ;});

  id = dialogFilterFactory->registerItemModel(new FilteredItemModel(CurveData::pointsItemModel()),
                                                         "Curve Points");

  ui->curvePoints->setModel(dialogFilterFactory->getItemModel(id));
  ui->curvePoints->setField(curve.count);
  connect(ui->curvePoints, &AutoComboBox::currentDataChanged, this, [&] () { on_curvePointsChanged(ui->curvePoints->currentIndex()) ;});

  ui->curveSmooth->setField(curve.smooth);
  connect(ui->curveSmooth, &AutoCheckBox::currentDataChanged, this, [&] () { update() ;});

  scene = new CustomScene(ui->curvePreview);
  scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  connect(scene, &CustomScene::newPoint, this, &CurveDialog::onSceneNewPoint);

  ui->curvePreview->setScene(scene);

  for (int i = 0; i < CPN_MAX_POINTS; i++) {
    spnx[i] = new QSpinBox(this);
    spnx[i]->setProperty("index", i);
    spnx[i]->setMinimum(-100);
    spnx[i]->setMaximum(+100);
    spnx[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    spnx[i]->setAccelerated(true);
    connect(spnx[i], QOverload<int>::of(&QSpinBox::valueChanged), this, &CurveDialog::onPointEdited);
    ui->pointsLayout->addWidget(spnx[i], i, 0, 1, 1);

    spny[i] = new QSpinBox(this);
    spny[i]->setProperty("index", i);
    spny[i]->setMinimum(-100);
    spny[i]->setMaximum(+100);
    spny[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    spny[i]->setAccelerated(true);
    connect(spny[i], QOverload<int>::of(&QSpinBox::valueChanged), this, &CurveDialog::onPointEdited);
    ui->pointsLayout->addWidget(spny[i], i, 1, 1, 1);
  }

  addTemplate(tr("Linear"), CURVE_YMIN_ENABLE, curveLinear);
  addTemplate(tr("Single Expo"), CURVE_COEFF_ENABLE | CURVE_YMIN_ENABLE, curveExpo);
  addTemplate(tr("Symmetrical f(x)=-f(-x)"), CURVE_COEFF_ENABLE, curveSymmetricalY);
  addTemplate(tr("Symmetrical f(x)=f(-x)"), CURVE_COEFF_ENABLE | CURVE_YMID_ENABLE, curveSymmetricalX);
  on_curveTemplateChanged(0);

  ui->pointSize->setValue(10);
  ui->pointSize->setMinimum(3);
  ui->pointSize->setMaximum(20);
  connect(ui->pointSize, QOverload<int>::of(&QSpinBox::valueChanged), this, [&] () { if (!lock) update(); });

  connect(ui->curveTemplate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CurveDialog::on_curveTemplateChanged);
  connect(ui->curveApply, &QPushButton::clicked, this, &CurveDialog::on_curveApplyClicked);
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(this, &QDialog::accepted, this, &CurveDialog::onDialogAccepted);

  update();
  shrink();

  lock = false;
}

CurveDialog::~CurveDialog()
{
  delete ui;
  delete dialogFilterFactory;
}

void CurveDialog::onDialogAccepted()
{
  model.curves[curveIdx] = curve;
  if (sharedItemModels)
    sharedItemModels->update(AbstractItemModel::IMUE_Curves);
}

void CurveDialog::update()
{
  lock = true;

  ui->curveName->update();
  updateCurveType();
  updateCurve();
  updateCurvePoints();
  shrink();

  lock = false;
}

void CurveDialog::updateCurveType()
{
  lock = true;

  ui->curveType->update();
  ui->curvePoints->update();
  ui->curveSmooth->update();

  lock = false;
}

void CurveDialog::updateCurve()
{
  lock = true;

  Node * nodel = 0;
  Node * nodex = 0;

  scene->clear();

  qreal width  = scene->sceneRect().width();
  qreal height = scene->sceneRect().height();

  qreal leftX = scene->sceneRect().left() + width / 4 * 1;
  qreal centerX = scene->sceneRect().left() + width / 2;
  qreal rightX = scene->sceneRect().left() + width / 4 * 3;
  qreal topY = scene->sceneRect().top() + height / 4 * 1;
  qreal centerY = scene->sceneRect().top() + height / 2;
  qreal bottomY = scene->sceneRect().top() + height / 4 * 3;

  scene->addLine(leftX, GFX_MARGIN, leftX, height + GFX_MARGIN);
  scene->addLine(centerX, GFX_MARGIN, centerX, height + GFX_MARGIN);
  scene->addLine(rightX, GFX_MARGIN, rightX, height + GFX_MARGIN);
  scene->addLine(GFX_MARGIN, topY, width + GFX_MARGIN, topY);
  scene->addLine(GFX_MARGIN, centerY, width + GFX_MARGIN, centerY);
  scene->addLine(GFX_MARGIN, bottomY, width + GFX_MARGIN, bottomY);

  QPen pen;
  pen.setWidth(1);
  pen.setStyle(Qt::SolidLine);

  pen.setColor(colors[0]);    //  needed????

  int numpoints = curve.count;

  for (int i = 0; i < numpoints - 1; i++) {
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM)
      scene->addLine(centerX + (qreal)curve.points[i].x * width / 200,
                     centerY - (qreal)curve.points[i].y * height / 200,
                     centerX + (qreal)curve.points[i + 1].x * width / 200,
                     centerY - (qreal)curve.points[i + 1].y * height / 200, pen);
    else
      scene->addLine(GFX_MARGIN + i * width / (numpoints - 1),
                     centerY - (qreal)curve.points[i].y * height / 200,
                     GFX_MARGIN + (i + 1) * width / (numpoints - 1),
                     centerY - (qreal)curve.points[i + 1].y * height / 200, pen);
  }

  for (int i = 0; i < numpoints; i++) {
    nodel = nodex;
    nodex = new Node();
    nodex->setProperty("index", i);
    nodex->setColor(colors[0]);
    nodex->setBallSize(ui->pointSize->value());
    nodex->setBallHeight(0);
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM) {
      if (i > 0 && i < numpoints - 1) {
        nodex->setFixedX(false);
        nodex->setMinX(curve.points[i - 1].x);
        nodex->setMaxX(curve.points[i + 1].x);
      }
      else {
        nodex->setFixedX(true);
      }
      nodex->setPos(centerX + (qreal)curve.points[i].x * width / 200,
                    centerY - (qreal)curve.points[i].y * height / 200);
    }
    else {
      nodex->setFixedX(true);
      nodex->setPos(GFX_MARGIN + i * width / (numpoints - 1),
                    centerY - (qreal)curve.points[i].y * height / 200);
    }
    connect(nodex, SIGNAL(moved(int, int)), this, SLOT(onNodeMoved(int, int)));
    connect(nodex, SIGNAL(focus()), this, SLOT(onNodeFocus()));
    connect(nodex, SIGNAL(unfocus()), this, SLOT(onNodeUnfocus()));
    connect(nodex, SIGNAL(deleteMe()), this, SLOT(onNodeDelete()));
    scene->addItem(nodex);
    if (i > 0)
      scene->addItem(new Edge(nodel, nodex));
  }

  lock = false;
}

void CurveDialog::updateCurvePoints()
{
  lock = true;

  int count = curve.count;
  for (int i = 0; i < count; i++) {
    spny[i]->show();
    spny[i]->setValue(curve.points[i].y);

    if (curve.type == CurveData::CURVE_TYPE_CUSTOM) {
      spnx[i]->show();
      if (i == 0 || i == count - 1) {
        spnx[i]->setDisabled(false);
        spnx[i]->setMaximum(+100);
        spnx[i]->setMinimum(-100);
        spnx[i]->setDisabled(true);
      }
      else {
        spnx[i]->setDisabled(false);
        spnx[i]->setMaximum(+100);
        spnx[i]->setMinimum(-100);
        spnx[i]->setMaximum(curve.points[i + 1].x);
        spnx[i]->setMinimum(curve.points[i - 1].x);
      }
      spnx[i]->setValue(curve.points[i].x);
    }
    else {
      spnx[i]->hide();
    }
  }

  for (int i = count; i < CPN_MAX_POINTS; i++) {
    spny[i]->hide();
    spnx[i]->hide();
  }

  lock = false;
}

void CurveDialog::onPointEdited(int i)
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    curve.points[index].x = spnx[index]->value();
    curve.points[index].y = spny[index]->value();
    updateCurve();
    updateCurvePoints();
  }
}

void CurveDialog::onNodeMoved(int x, int y)
{
  if (!lock) {
    lock = true;
    int index = sender()->property("index").toInt();
    curve.points[index].x = x;
    curve.points[index].y = y;
    spnx[index]->setValue(x);
    spny[index]->setValue(y);
    if (index > 0)
      spnx[index - 1]->setMaximum(x);
    if (index < curve.count - 1)
      spnx[index + 1]->setMinimum(x);
    lock = false;
  }
}

void CurveDialog::onNodeFocus()
{
  int index = sender()->property("index").toInt();
  spny[index]->setFocus();
}

void CurveDialog::onNodeUnfocus()
{
  int index = sender()->property("index").toInt();
  spny[index]->clearFocus();
  updateCurve();
}

bool CurveDialog::allowCurveType(int points, CurveData::CurveType type)
{
  int totalpoints = 0;
  for (int i = 0; i < maxCurves; i++) {
    int cvPoints = (i == curveIdx ? points : model.curves[i].count);
    CurveData::CurveType cvType = (i == curveIdx ? type : model.curves[i].type);
    totalpoints += cvPoints + (cvType == CurveData::CURVE_TYPE_CUSTOM ? cvPoints - 2 : 0);
  }

  if (totalpoints > maxPoints) {
    QMessageBox::warning(this, CPN_STR_APP_NAME, tr("Not enough free points in global points array to store the curve."));
    return false;
  }
  else {
    return true;
  }
}

void CurveDialog::on_curveNameChanged()
{
  setWindowTitle(tr("Editing curve %1: %2").arg(curveIdx + 1).arg(QString(curve.name)));
}

void CurveDialog::on_curvePointsChanged(int index)
{
  if (!lock) {
    int numpoints = ((QComboBox *)sender())->itemData(index).toInt();

    if (allowCurveType(numpoints, curve.type)) {
      curve.count = numpoints;
      curve.init();
      updateCurve();
      updateCurvePoints();
    }
    else {
      updateCurveType();
    }
  }
}

void CurveDialog::on_curveTypeChanged(int index)
{
  if (!lock) {
    CurveData::CurveType type = (CurveData::CurveType)index;
    int numpoints = ui->curvePoints->itemData(ui->curvePoints->currentIndex()).toInt();

    if (allowCurveType(numpoints, type)) {
      curve.type = type;
      curve.count = numpoints;
      curve.init();
      update();
    }
    else {
      updateCurveType();
    }
  }
}

void CurveDialog::resizeEvent(QResizeEvent *event)
{
  QRect qr = ui->curvePreview->contentsRect();
  ui->curvePreview->scene()->setSceneRect(GFX_MARGIN, GFX_MARGIN, qr.width() - GFX_MARGIN * 2, qr.height() - GFX_MARGIN * 2);
  updateCurve();
  updateCurvePoints();
}

void CurveDialog::on_curveTemplateChanged(int index)
{
  unsigned int flags = templates[index].flags;
  ui->curveCoeffLabel->setVisible(flags & CURVE_COEFF_ENABLE);
  ui->curveCoeff->setVisible(flags & CURVE_COEFF_ENABLE);
  ui->yMax->setValue(100);
  ui->yMidLabel->setVisible(flags & CURVE_YMID_ENABLE);
  ui->yMid->setVisible(flags & CURVE_YMID_ENABLE);
  ui->yMid->setValue(0);
  ui->yMinLabel->setVisible(flags & CURVE_YMIN_ENABLE);
  ui->yMin->setVisible(flags & CURVE_YMIN_ENABLE);
  ui->yMin->setValue(-100);
}

void CurveDialog::addTemplate(QString name, unsigned int flags, curveFunction function)
{
  CurveCreatorTemplate tmpl;
  tmpl.name = name;
  tmpl.flags = flags;
  tmpl.function = function;
  templates.append(tmpl);
  ui->curveTemplate->addItem(name);
}

void CurveDialog::on_curveApplyClicked()
{
  int index = ui->curveTemplate->currentIndex();
  int numpoints = curve.count;

  for (int i = 0; i < numpoints; i++) {
    float x;
    if (curve.type == CurveData::CURVE_TYPE_CUSTOM)
      x = curve.points[i].x;
    else
      x = -100.0 + (200.0 / (numpoints - 1)) * i;

    bool apply = false;
    switch (ui->curveSide->currentIndex()) {
      case 0:
        apply = true;
        break;
      case 1:
        if (x >= 0)
          apply = true;
        break;
      case 2:
        if (x < 0)
          apply = true;
        break;
    }

    if (apply) {
      curve.points[i].y = templates[index].function(x, ui->curveCoeff->value(), ui->yMin->value(), ui->yMid->value(),
                                                                          ui->yMax->value());
    }
  }

  updateCurve();
  updateCurvePoints();
}

void CurveDialog::onNodeDelete()
{
  int index = sender()->property("index").toInt();
  int numpoints = curve.count;
  if ((curve.type == CurveData::CURVE_TYPE_CUSTOM) && (index > 0) && (index < numpoints - 1)) {
    spny[index]->clearFocus();
    for (int i = index + 1; i < numpoints; i++) {
      curve.points[i - 1] = curve.points[i];
    }
    numpoints--;
    curve.points[numpoints].x = 0;
    curve.points[numpoints].y = 0;
    curve.count = numpoints;
    updateCurve();
    updateCurvePoints();
  }
}

void CurveDialog::onSceneNewPoint(int x, int y)
{
  if (!lock) {
    if ((curve.type == CurveData::CURVE_TYPE_CUSTOM) && (curve.count < CPN_MAX_POINTS)) {
      int newidx = 0;
      int numpoints = curve.count;
      if (x < curve.points[0].x) {
        newidx = 0;
      }
      else if (x > curve.points[numpoints - 1].x) {
        newidx = numpoints;
      }
      else {
        for (int i = 0; i < numpoints; i++) {
          if (x < curve.points[i].x) {
            newidx = i;
            break;
          }
        }
      }
      numpoints++;
      curve.count = numpoints;
      for (int idx = (numpoints - 1); idx > newidx; idx--) {
        curve.points[idx] = curve.points[idx - 1];
      }
      curve.points[newidx].x = x;
      curve.points[newidx].y = y;
      updateCurve();
      updateCurvePoints();
    }
  }
}

void CurveDialog::shrink()
{
  resize(0, 0);
}

CustomScene::CustomScene(QGraphicsView * view) :
  QGraphicsScene(view)
{
}

void CustomScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  if (event->button() == Qt::LeftButton && event->modifiers() == Qt::ControlModifier) {
    QRectF rect = sceneRect();
    QPointF pos = event->scenePos();
    QPointF p;
    p.setX(-100 + ((pos.x() - rect.left()) * 200) / rect.width());
    p.setY(100 + (rect.top() - pos.y()) * 200 / rect.height());
    QGraphicsScene::mouseReleaseEvent(event);
    emit newPoint((int)p.x(), (int)p.y());
  }
  else
    QGraphicsScene::mouseReleaseEvent(event);
}
