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

#include "curves.h"
#include "curveimagewidget.h"
#include "compounditemmodels.h"
#include "datahelpers.h"
#include "helpers.h"

CurvesPanel::CurvesPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels)
{
  lock = true;
  maxCurves = firmware->getCapability(NumCurves);

  QStringList headerLabels;
  headerLabels << "#" << "" << tr("Note: to create a curve right click on the curve row label");

  TableLayout *tableLayout = new TableLayout(this, maxCurves, headerLabels);

  QFontMetrics *f = new QFontMetrics(QFont());
  QSize szpnts;
  szpnts = f->size(Qt::TextSingleLine, QString(75, 'X'));
  delete f;

  for (int i = 0; i < maxCurves; i++) {
    int col = 0;

    QLabel *label = new QLabel(this);
    label->setText(tr("CV%1").arg(i+1));
    label->setProperty("index", i);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    label->setToolTip(tr("Popup menu available"));
    label->setMouseTracking(true);
    connect(label, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
    tableLayout->addWidget(i, col++, label);

    image[i] = new CurveImageWidget(this);
    image[i]->set(&model, firmware, sharedItemModels, i + 1, colors[i], 3);
    image[i]->setGrid(Qt::gray, 2);
    image[i]->setProperty("index", i);
    image[i]->setFixedSize(QSize(100, 100));
    image[i]->setContextMenuPolicy(Qt::CustomContextMenu);
    image[i]->setToolTip(tr("Popup menu available"));
    connect(image[i], SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
    connect(image[i], &CurveImageWidget::doubleClicked, this, &CurvesPanel::on_curveImageDoubleClicked);

    tableLayout->addWidget(i, col++, image[i]);
    tableLayout->setColumnStretch(col, 1);

    grp[i] = new QWidget(); // layouts are not hideable so use widget as parent for grid

    grid[i] = new QGridLayout(grp[i]);
    int row = 0;

    grid[i]->addWidget(new QLabel(tr("Name:")), row, 0, Qt::AlignLeft);
    name[i] = new QLabel();
    grid[i]->addWidget(name[i], row++, 1, Qt::AlignLeft);

    grid[i]->addWidget(new QLabel(tr("Type:")), row, 0, Qt::AlignLeft);
    type[i] = new QLabel();
    grid[i]->addWidget(type[i], row++, 1, Qt::AlignLeft);

    grid[i]->addWidget(new QLabel(tr("Points:")), row, 0, Qt::AlignLeft);
    numpoints[i] = new QLabel();
    grid[i]->addWidget(numpoints[i], row, 1);
    points[i] = new QLabel();
    points[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //  at at Qt 5.12
    //  setWordWrap(true) does not work well in this instance due to the nested widgets and
    //  the display results varying between OSes so as a compromise
    //  stop widget expanding uncontrolled and consequentally expanding edit dialog potentially off screen
    //  contents will be truncated if exceed maximum size
    points[i]->setMaximumWidth(szpnts.width());
    grid[i]->addWidget(points[i], row++, 2, Qt::AlignLeft);

    grid[i]->addWidget(new QLabel(tr("Smooth:")), row, 0, Qt::AlignLeft);
    smooth[i] = new QLabel();
    grid[i]->addWidget(smooth[i], row++, 1, Qt::AlignLeft);

    tableLayout->addWidget(i, col++, grp[i], Qt::AlignLeft);
  }

  tableLayout->resizeColumnsToContents();
  tableLayout->pushColumnsLeft(headerLabels.count());
  tableLayout->pushRowsUp(maxCurves + 1);

  for (int i = 0; i < CPN_MAX_CURVES; i++) {
    update(i);
  }

  lock = false;
}

CurvesPanel::~CurvesPanel()
{
  // compiler warning if delete[]
  for (int i = 0; i < CPN_MAX_CURVES; i++) {
    delete image[i];
    delete name[i];
    delete type[i];
    delete numpoints[i];
    delete points[i];
    delete smooth[i];
    delete grid[i];
    delete grp[i];
  }
}

void CurvesPanel::update()
{
  for (int i = 0; i < maxCurves; i++) {
    update(i);
  }
}

void CurvesPanel::update(int index)
{
  if (model->curves[index].isEmpty()) {
    image[index]->setVisible(false);
    grp[index]->setVisible(false);
  }
  else {
    image[index]->setVisible(true);
    grp[index]->setVisible(true);

    image[index]->draw();
    name[index]->setText(QString(model->curves[index].name));
    type[index]->setText(model->curves[index].typeToString());
    numpoints[index]->setText(QString::number(model->curves[index].count));
    points[index]->setText(model->curves[index].pointsToString());
    smooth[index]->setText(DataHelpers::boolToString(model->curves[index].smooth, DataHelpers::BOOL_FMT_YESNO));
  }
}

void CurvesPanel::editCurve(int index)
{
  if (image[index]->edit()) {
    update(index);
    emit modified();
  }
}

void CurvesPanel::on_curveImageDoubleClicked()
{
  bool ok = false;
  int index = sender()->property("index").toInt(&ok);

  if (ok)
    editCurve(index);
}

void CurvesPanel::onCustomContextMenuRequested(QPoint pos)
{
  QWidget *wgt = (QWidget *)sender();
  selectedIndex = wgt->property("index").toInt();
  QPoint globalPos = wgt->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("new.png"), tr("Add"), this, SLOT(cmEdit()))->setEnabled(!curveExists());
  contextMenu.addAction(CompanionIcon("edit.png"), tr("Edit"), this, SLOT(cmEdit()))->setEnabled(curveExists());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this, SLOT(cmCopy()))->setEnabled(curveExists());
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this, SLOT(cmCut()))->setEnabled(curveExists());
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this, SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this, SLOT(cmClear()))->setEnabled(curveExists());
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

bool CurvesPanel::curveExists() const
{
  return !model->curves[selectedIndex].isEmpty();
}

bool CurvesPanel::insertAllowed() const
{
  return ((selectedIndex < maxCurves - 1) && (model->curves[maxCurves - 1].isEmpty()));
}

bool CurvesPanel::moveDownAllowed() const
{
  return selectedIndex < maxCurves - 1 && curveExists();
}

bool CurvesPanel::moveUpAllowed() const
{
  return selectedIndex > 0 && curveExists();
}

void CurvesPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear curve %1:%2. Are you sure?").arg(QString::number(selectedIndex + 1)).arg(model->curves[selectedIndex].name), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->curves[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_CLEAR, selectedIndex);
  update(selectedIndex);
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all curves. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
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
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut curve %1:%2. Are you sure?").arg(QString::number(selectedIndex + 1)).arg(model->curves[selectedIndex].name), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;
  cmCopy();
  cmClear(false);
}

void CurvesPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete curve %1:%2. Are you sure?").arg(QString::number(selectedIndex + 1)).arg(model->curves[selectedIndex].name), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  memmove(&model->curves[selectedIndex], &model->curves[selectedIndex + 1], (CPN_MAX_CURVES - (selectedIndex + 1)) * sizeof(CurveData));
  model->curves[maxCurves - 1].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, -1);
  for (int i = selectedIndex; i < CPN_MAX_CURVES; i++) {
    update(i);
  }
  updateItemModels();
  emit modified();
}

void CurvesPanel::cmEdit()
{
  editCurve(selectedIndex);
}

void CurvesPanel::cmInsert()
{
  memmove(&model->curves[selectedIndex + 1], &model->curves[selectedIndex], (CPN_MAX_CURVES - (selectedIndex + 1)) * sizeof(CurveData));
  model->curves[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_CURVE, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, 1);
  for (int i = selectedIndex; i < CPN_MAX_CURVES; i++) {
    update(i);
  }
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
    update(selectedIndex);
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
    update(idx1);
    update(idx2);
    updateItemModels();
    emit modified();
  }
}

void CurvesPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Curves);
}
