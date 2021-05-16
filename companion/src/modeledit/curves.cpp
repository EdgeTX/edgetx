/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "curves.h"
#include "ui_curves.h"
#include "node.h"
#include "edge.h"
#include "helpers.h"
#include "filtereditemmodels.h"

#define GFX_MARGIN 16

#define CURVE_COEFF_ENABLE   1
#define CURVE_YMID_ENABLE    2
#define CURVE_YMIN_ENABLE    4

float curveLinear(float x, float coeff, float yMin, float yMid, float yMax)
{
  float a = (yMax - yMin) / 200.0;
  return yMin + a * (x + 100.0);
}

float c9xexpou(float point, float coeff)
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
    return round(c9xexpou(x, coeff)*a + yMin);
  }
  else {
    coeff = -coeff;
    x = 100 - x;
    return round((100.0 - c9xexpou(x, coeff)) * a + yMin);
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
    y = round(c9xexpou(x, coeff) * (yMax / 100.0));
  }
  else {
    coeff = -coeff;
    x = 100.0 - x;
    y = round((100.0 - c9xexpou(x, coeff)) * (yMax / 100.0));
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
    y = round(c9xexpou(x, coeff) * a + yMid);
  }
  else {
    coeff = -coeff;
    x = 100 - x;
    y = round((100.0 - c9xexpou(x, coeff)) * a + yMid);
  }

  return y;
}

CurvesPanel::CurvesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                         CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::Curves),
  currentCurve(0),
  sharedItemModels(sharedItemModels)
{
  ui->setupUi(this);

  lock = true;

  maxCurves = firmware->getCapability(NumCurves);
  hasNames = firmware->getCapability(HasCvNames);
  hasEnhanced = firmware->getCapability(EnhancedCurves);
  maxPoints = firmware->getCapability(NumCurvePoints);

  if (!hasNames) {
    ui->curveName->hide();
    ui->curveNameLabel->hide();
  }

  scene = new CustomScene(ui->curvePreview);
  scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  connect(scene, SIGNAL(newPoint(int, int)), this, SLOT(onSceneNewPoint(int, int)));

  ui->curvePreview->setScene(scene);

  int limit;
  if (maxCurves > 16) {
      limit = maxCurves / 2;
  } else {
      limit = maxCurves;
  }
  for (int i = 0; i < maxCurves; i++) {
    visibleCurves[i] = false;

    // The edit curve button
    QPushButton * edit = new QPushButton(this);
    edit->setProperty("index", i);

#ifdef __GNUC__
    //  creates more compact and likely consistent buttons across OS flavors
    edit->setStyleSheet(QString("background-color: %1; color: white; padding: 2px 3px; border-style: outset; border-width: 1px; border-radius: 2px; border-color: inherit;").arg(colors[i].name()));
#else
    edit->setStyleSheet(QString("background-color: %1; color: white;").arg(colors[i].name()));
#endif

    edit->setText(tr("Curve %1").arg(i + 1));
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    edit->setToolTip(tr("Popup menu available"));
    connect(edit, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));
    connect(edit, SIGNAL(clicked()), this, SLOT(editCurve()));
    if (i < limit) {
      ui->curvesLayout->addWidget(edit, i, 1, 1, 1);
    } else {
      ui->curvesLayout2->addWidget(edit, i - limit, 2, 1, 1);
    }

    // The curve plot checkbox
    QCheckBox * plot = new QCheckBox(this);
    plot->setProperty("index", i);
    connect(plot, SIGNAL(toggled(bool)), this, SLOT(plotCurve(bool)));
    if (i < limit) {
      ui->curvesLayout->addWidget(plot, i, 2, 1, 1);
    } else {
      ui->curvesLayout2->addWidget(plot, i-limit, 1, 1, 1);
    }
  }
  QSpacerItem * item = new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);

  ui->curvesLayout->addItem(item,limit + 1, 1, 1, 1, 0);
  if (limit != maxCurves) {
    QSpacerItem * item2 = new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding);
    ui->curvesLayout2->addItem(item2,limit + 1, 1, 1, 1, 0);
  }

  for (int i = 0; i < CPN_MAX_POINTS; i++) {
    spnx[i] = new QSpinBox(this);
    spnx[i]->setProperty("index", i);
    spnx[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    spnx[i]->setAccelerated(true);
    connect(spnx[i], SIGNAL(valueChanged(int)), this, SLOT(onPointEdited()));
    ui->pointsLayout->addWidget(spnx[i], i, 0, 1, 1);

    spny[i] = new QSpinBox(this);
    spny[i]->setProperty("index", i);
    spny[i]->setMinimum(-100);
    spny[i]->setMaximum(+100);
    spny[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    spny[i]->setAccelerated(true);
    connect(spny[i], SIGNAL(valueChanged(int)), this, SLOT(onPointEdited()));
    ui->pointsLayout->addWidget(spny[i], i, 1, 1, 1);

    bool insert;
    if (hasEnhanced) {
      insert = (i >= 1);
    }
    else {
      insert = (i == 2 || i == 4 || i ==8 || i == 16);
    }
    if (insert) {
      ui->curvePoints->addItem(tr("%1 points").arg(i + 1), i + 1);
    }
  }

  addTemplate(tr("Linear"), CURVE_YMIN_ENABLE, curveLinear);
  addTemplate(tr("Single Expo"), CURVE_COEFF_ENABLE | CURVE_YMIN_ENABLE, curveExpo);
  addTemplate(tr("Symmetrical f(x)=-f(-x)"), CURVE_COEFF_ENABLE, curveSymmetricalY);
  addTemplate(tr("Symmetrical f(x)=f(-x)"), CURVE_COEFF_ENABLE | CURVE_YMID_ENABLE, curveSymmetricalX);

  ui->pointSize->setValue(10);
  ui->pointSize->setMinimum(3);
  ui->pointSize->setMaximum(20);
  connect(ui->pointSize, SIGNAL(valueChanged(int)), this, SLOT(onPointSizeEdited()));

  disableMouseScrolling();

  lock = false;
}

CurvesPanel::~CurvesPanel()
{
  delete ui;
}

void CurvesPanel::editCurve()
{
  QPushButton *button = (QPushButton *)sender();
  int index = button->property("index").toInt();
  setCurrentCurve(index);
  update();
}

void CurvesPanel::plotCurve(bool checked)
{
  QCheckBox *chk = (QCheckBox *)sender();
  int index = chk->property("index").toInt();
  visibleCurves[index] = checked;
  updateCurve();
}

void CurvesPanel::update()
{
  lock = true;

  if (hasNames) {
    ui->curveName->setText(model->curves[currentCurve].name);
  }

  updateCurveType();
  updateCurve();
  updateCurvePoints();

  lock = false;
}

void CurvesPanel::setCurrentCurve(int index)
{
  currentCurve = index;
}

void CurvesPanel::updateCurveType()
{
  lock = true;

  int index = 0;

  if (hasEnhanced) {
    index = model->curves[currentCurve].count - 2;
  }
  else {
    ui->curveSmooth->hide();
    if (model->curves[currentCurve].count == 5)
      index = 1;
    else if (model->curves[currentCurve].count == 9)
      index = 2;
    else if (model->curves[currentCurve].count == 17)
      index = 3;
  }

  ui->curvePoints->setCurrentIndex(index);
  ui->curveCustom->setCurrentIndex(model->curves[currentCurve].type);
  ui->curveSmooth->setCurrentIndex(model->curves[currentCurve].smooth);

  lock = false;
}

void CurvesPanel::updateCurve()
{
  lock = true;

  Node * nodel = 0;
  Node * nodex = 0;
  QColor color;

  scene->clear();

  qreal width  = scene->sceneRect().width();
  qreal height = scene->sceneRect().height();

  qreal centerX = scene->sceneRect().left() + width / 2;
  qreal centerY = scene->sceneRect().top() + height / 2;

  QGraphicsSimpleTextItem *ti = scene->addSimpleText(tr("Editing curve %1").arg(currentCurve + 1));
  ti->setPos(3, 3);

  scene->addLine(centerX, GFX_MARGIN, centerX, height + GFX_MARGIN);
  scene->addLine(GFX_MARGIN, centerY, width + GFX_MARGIN, centerY);

  QPen pen;
  pen.setWidth(1);
  pen.setStyle(Qt::SolidLine);

  for (int k = 0; k < maxCurves; k++) {
    pen.setColor(colors[k]);
    if (currentCurve != k && visibleCurves[k]) {
      int numpoints = model->curves[k].count;
      for (int i = 0; i < numpoints - 1; i++) {
        if (model->curves[k].type == CurveData::CURVE_TYPE_CUSTOM)
          scene->addLine(centerX + (qreal)model->curves[k].points[i].x * width / 200,
                         centerY - (qreal)model->curves[k].points[i].y * height / 200,
                         centerX + (qreal)model->curves[k].points[i + 1].x * width / 200,
                         centerY - (qreal)model->curves[k].points[i + 1].y * height / 200, pen);
        else
          scene->addLine(GFX_MARGIN + i * width / (numpoints - 1),
                         centerY - (qreal)model->curves[k].points[i].y * height / 200,
                         GFX_MARGIN + (i + 1) * width / (numpoints - 1),
                         centerY - (qreal)model->curves[k].points[i + 1].y * height / 200, pen);
      }
    }
  }

  int numpoints = model->curves[currentCurve].count;
  for (int i = 0; i < numpoints; i++) {
    nodel = nodex;
    nodex = new Node();
    nodex->setProperty("index", i);
    nodex->setColor(colors[currentCurve]);
    nodex->setBallSize(ui->pointSize->value());
    nodex->setBallHeight(0);
    if (model->curves[currentCurve].type == CurveData::CURVE_TYPE_CUSTOM) {
      if (i > 0 && i < numpoints - 1) {
        nodex->setFixedX(false);
        nodex->setMinX(model->curves[currentCurve].points[i - 1].x);
        nodex->setMaxX(model->curves[currentCurve].points[i + 1].x);
      }
      else {
        nodex->setFixedX(true);
      }
      nodex->setPos(centerX + (qreal)model->curves[currentCurve].points[i].x * width / 200,
                    centerY - (qreal)model->curves[currentCurve].points[i].y * height / 200);
    }
    else {
      nodex->setFixedX(true);
      nodex->setPos(GFX_MARGIN + i * width / (numpoints - 1),
                    centerY - (qreal)model->curves[currentCurve].points[i].y * height / 200);
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

void CurvesPanel::updateCurvePoints()
{
  lock = true;

  int count = model->curves[currentCurve].count;
  for (int i = 0; i < count; i++) {
    spny[i]->show();
    spny[i]->setValue(model->curves[currentCurve].points[i].y);
    if (model->curves[currentCurve].type == CurveData::CURVE_TYPE_CUSTOM) {
      spnx[i]->show();
      if (i == 0 || i == model->curves[currentCurve].count - 1) {
        spnx[i]->setDisabled(true);
        spnx[i]->setMaximum(+100);
        spnx[i]->setMinimum(-100);
      }
      else {
        spnx[i]->setDisabled(false);
        spnx[i]->setMaximum(model->curves[currentCurve].points[i + 1].x);
        spnx[i]->setMinimum(model->curves[currentCurve].points[i - 1].x);
      }
      spnx[i]->setValue(model->curves[currentCurve].points[i].x);
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

void CurvesPanel::onPointEdited()
{
  if (!lock) {
    int index = sender()->property("index").toInt();
    model->curves[currentCurve].points[index].x = spnx[index]->value();
    model->curves[currentCurve].points[index].y = spny[index]->value();
    updateCurve();
    updateCurvePoints();
    emit modified();
  }
}

void CurvesPanel::onNodeMoved(int x, int y)
{
  if (!lock) {
    lock = true;
    int index = sender()->property("index").toInt();
    model->curves[currentCurve].points[index].x = x;
    model->curves[currentCurve].points[index].y = y;
    spnx[index]->setValue(x);
    spny[index]->setValue(y);
    if (index > 0)
      spnx[index - 1]->setMaximum(x);
    if (index < model->curves[currentCurve].count - 1)
      spnx[index + 1]->setMinimum(x);
    emit modified();
    lock = false;
  }
}

void CurvesPanel::onNodeFocus()
{
  int index = sender()->property("index").toInt();
  spny[index]->setFocus();
}

void CurvesPanel::onNodeUnfocus()
{
  int index = sender()->property("index").toInt();
  spny[index]->clearFocus();
  updateCurve();
}

bool CurvesPanel::allowCurveType(int points, CurveData::CurveType type)
{
  int totalpoints = 0;
  for (int i = 0; i < maxCurves; i++) {
    int cvPoints = (i == currentCurve ? points : model->curves[i].count);
    CurveData::CurveType cvType = (i == currentCurve ? type : model->curves[i].type);
    totalpoints += cvPoints + (cvType == CurveData::CURVE_TYPE_CUSTOM ? cvPoints - 2 : 0);
  }

  if (totalpoints > maxPoints) {
    QMessageBox::warning(this, "companion", tr("Not enough free points in EEPROM to store the curve."));
    return false;
  }
  else {
    return true;
  }
}

void CurvesPanel::on_curvePoints_currentIndexChanged(int index)
{
  if (!lock) {
    int numpoints = ((QComboBox *)sender())->itemData(index).toInt();

    if (allowCurveType(numpoints, model->curves[currentCurve].type)) {
      model->curves[currentCurve].count = numpoints;

      // TODO something better + reuse!
      for (int i = 0; i < CPN_MAX_POINTS; i++) {
        model->curves[currentCurve].points[i].x = (i >= numpoints - 1 ? +100 : -100 + (200 * i) / (numpoints - 1));
        model->curves[currentCurve].points[i].y = 0;
      }

      update();
      emit modified();
    }
    else {
      updateCurveType();
    }
  }
}

void CurvesPanel::on_curveCustom_currentIndexChanged(int index)
{
  if (!lock) {
    CurveData::CurveType type = (CurveData::CurveType)index;
    int numpoints = ui->curvePoints->itemData(ui->curvePoints->currentIndex()).toInt();

    if (allowCurveType(model->curves[currentCurve].count, type)) {
      model->curves[currentCurve].type = type;

      // TODO something better + reuse!
      for (int i = 0; i < CPN_MAX_POINTS; i++) {
        model->curves[currentCurve].points[i].x = (i >= numpoints - 1 ? +100 : -100 + (200 * i) / (numpoints - 1));
        model->curves[currentCurve].points[i].y = 0;
      }

      update();
      emit modified();
    }
    else {
      updateCurveType();
    }
  }
}

void CurvesPanel::on_curveSmooth_currentIndexChanged(int index)
{
  model->curves[currentCurve].smooth = index;
  update();
}

void CurvesPanel::on_curveName_editingFinished()
{
  if (ui->curveName->text() != model->curves[currentCurve].name) {
    memset(model->curves[currentCurve].name, 0, sizeof(model->curves[currentCurve].name));
    strcpy(model->curves[currentCurve].name, ui->curveName->text().toLatin1());
    updateItemModels();
    emit modified();
  }
}

void CurvesPanel::resizeEvent(QResizeEvent *event)
{
  QRect qr = ui->curvePreview->contentsRect();
  ui->curvePreview->scene()->setSceneRect(GFX_MARGIN, GFX_MARGIN, qr.width() - GFX_MARGIN * 2, qr.height() - GFX_MARGIN * 2);
  updateCurve();
  ModelPanel::resizeEvent(event);
}

void CurvesPanel::on_curveType_currentIndexChanged(int index)
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

void CurvesPanel::addTemplate(QString name, unsigned int flags, curveFunction function)
{
  CurveCreatorTemplate tmpl;
  tmpl.name = name;
  tmpl.flags = flags;
  tmpl.function = function;
  templates.append(tmpl);
  ui->curveType->addItem(name);
}

void CurvesPanel::on_curveApply_clicked()
{
  int index = ui->curveType->currentIndex();
  int numpoints = model->curves[currentCurve].count;

  for (int i = 0; i < numpoints; i++) {
    float x;
    if (model->curves[currentCurve].type == CurveData::CURVE_TYPE_CUSTOM)
      x = model->curves[currentCurve].points[i].x;
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
      model->curves[currentCurve].points[i].y = templates[index].function(x, ui->curveCoeff->value(), ui->yMin->value(), ui->yMid->value(),
                                                                          ui->yMax->value());
    }
  }

  updateCurve();
  updateCurvePoints();
  emit modified();
}

void CurvesPanel::onPointSizeEdited()
{
  if (!lock) {
    update();
  }
}

void CurvesPanel::onNodeDelete()
{
  int index = sender()->property("index").toInt();
  int numpoints = model->curves[currentCurve].count;
  if ((model->curves[currentCurve].type == CurveData::CURVE_TYPE_CUSTOM) && (index > 0) && (index < numpoints-1)) {
    spny[index]->clearFocus();
    for (int i=index+1; i<numpoints; i++) {
      model->curves[currentCurve].points[i-1] = model->curves[currentCurve].points[i];
    }
    numpoints--;
    model->curves[currentCurve].points[numpoints].x = 0;
    model->curves[currentCurve].points[numpoints].y = 0;
    model->curves[currentCurve].count = numpoints;
    update();
    emit modified();
  }
}

void CurvesPanel::onSceneNewPoint(int x, int y)
{
  if ((model->curves[currentCurve].type == CurveData::CURVE_TYPE_CUSTOM) && (model->curves[currentCurve].count < CPN_MAX_POINTS)) {
    int newidx = 0;
    int numpoints = model->curves[currentCurve].count;
    if (x < model->curves[currentCurve].points[0].x) {
      newidx = 0;
    }
    else if (x > model->curves[currentCurve].points[numpoints - 1].x) {
      newidx = numpoints;
    }
    else {
      for (int i = 0; i < numpoints; i++) {
        if (x < model->curves[currentCurve].points[i].x) {
          newidx = i;
          break;
        }
      }
    }
    numpoints++;
    model->curves[currentCurve].count = numpoints;
    for (int idx = (numpoints - 1); idx > newidx; idx--) {
      model->curves[currentCurve].points[idx] = model->curves[currentCurve].points[idx - 1];
    }
    model->curves[currentCurve].points[newidx].x = x;
    model->curves[currentCurve].points[newidx].y = y;
    update();
    emit modified();
  }
}

void CurvesPanel::onCustomContextMenuRequested(QPoint pos)
{
  QPushButton *button = (QPushButton *)sender();
  selectedIndex = button->property("index").toInt();
  QPoint globalPos = button->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this, SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this, SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this, SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this, SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this, SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this, SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this, SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this, SLOT(cmClearAll()));

  contextMenu.exec(globalPos);
}

bool CurvesPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_CURVE)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_CURVE));
    return true;
  }
  return false;
}

bool CurvesPanel::insertAllowed() const
{
  return ((selectedIndex < maxCurves - 1) && (model->curves[maxCurves - 1].isEmpty()));
}

bool CurvesPanel::moveDownAllowed() const
{
  return selectedIndex < maxCurves - 1;
}

bool CurvesPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void CurvesPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Curve. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->curves[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_CLEAR, selectedIndex);
  update();
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Curves. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  for (int i = 0; i < maxCurves; i++) {
    model->curves[i].clear();
    model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_CLEAR, i);
  }
  update();
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmCopy()
{
  QByteArray data;
  data.append((char*)&model->curves[selectedIndex], sizeof(CurveData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_CURVE, data);
  QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void CurvesPanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Curve. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void CurvesPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Curve. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  memmove(&model->curves[selectedIndex], &model->curves[selectedIndex + 1], (CPN_MAX_CURVES - (selectedIndex + 1)) * sizeof(CurveData));
  model->curves[maxCurves - 1].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, -1);
  update();
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmInsert()
{
  memmove(&model->curves[selectedIndex + 1], &model->curves[selectedIndex], (CPN_MAX_CURVES - (selectedIndex + 1)) * sizeof(CurveData));
  model->curves[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, 1);
  update();
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmMoveDown()
{
  swapData(selectedIndex, selectedIndex + 1);
}

void CurvesPanel::cmMoveUp()
{
  swapData(selectedIndex, selectedIndex - 1);
}

void CurvesPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    CurveData *cd = &model->curves[selectedIndex];
    memcpy(cd, data.constData(), sizeof(CurveData));
    update();
    updateItemModels();
    emit modified();
  }
}

void CurvesPanel::swapData(int idx1, int idx2)
{
  if ((idx1 != idx2) && (!model->curves[idx1].isEmpty() || !model->curves[idx2].isEmpty())) {
    CurveData cdtmp = model->curves[idx2];
    CurveData *cd1 = &model->curves[idx1];
    CurveData *cd2 = &model->curves[idx2];
    memcpy(cd2, cd1, sizeof(CurveData));
    memcpy(cd1, &cdtmp, sizeof(CurveData));
    model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
    update();
    updateItemModels();
    emit modified();
  }
}

void CurvesPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Curves);
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
