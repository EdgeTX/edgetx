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

#include "channels.h"
#include "helpers.h"
#include "filtereditemmodels.h"
#include "curveimagewidget.h"
#include "namevalidator.h"
#include "curvereferencewidget.h"

// LimitsGroup::LimitsGroup(Firmware * firmware, TableLayout * tableLayout, int row,
//                 int col, int & value, const ModelData & model, GeneralSettings & generalSettings,
//                 int min, int max, int deflt, FilteredItemModel * gvarModel, ModelPanel * panel):
//   firmware(firmware),
//   spinbox(new QDoubleSpinBox()),
//   value(value)
// {
//   spinbox->setProperty("index", row);
//   spinbox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
//   spinbox->setAccelerated(true);
//   spinbox->setDecimals(1);

//   if (generalSettings.ppmunit == GeneralSettings::PPM_US) {
//     displayStep = 0.512;
//     spinbox->setSuffix("us");
//   }
//   else {
//     displayStep = 0.1;
//     spinbox->setSuffix("%");
//   }

//   spinbox->setSingleStep(displayStep);
//   spinbox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

//   QHBoxLayout *horizontalLayout = new QHBoxLayout();
//   gv = new QCheckBox(tr("GV"));
//   gv->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//   horizontalLayout->addWidget(gv);
//   QComboBox *cb = new QComboBox();
//   cb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
//   horizontalLayout->addWidget(cb);
//   horizontalLayout->addWidget(spinbox);
//   tableLayout->addLayout(row, col, horizontalLayout);
//   gvarGroup = new GVarGroup(gv, spinbox, cb, value, model, deflt, min, max, displayStep, gvarModel);
//   QObject::connect(gvarGroup, &GVarGroup::valueChanged, panel, &ModelPanel::modified);
// }

// LimitsGroup::~LimitsGroup()
// {
//   delete gvarGroup;
// }

// void LimitsGroup::setValue(int val)
// {
//   gvarGroup->setWeight(val);
// }

// void LimitsGroup::updateMinMax(int max)
// {
//   if (spinbox->maximum() == 0) {
//     spinbox->setMinimum(-max * displayStep);
//     gvarGroup->setMinimum(-max);
//     if (!gv->isChecked() && value < -max) {
//       value = -max;
//     }
//   }
//   if (spinbox->minimum() == 0) {
//     spinbox->setMaximum(max * displayStep);
//     gvarGroup->setMaximum(max);
//     if (!gv->isChecked() && value > max) {
//       value = max;
//     }
//   }
// }

ChannelsPanel::ChannelsPanel(QWidget * parent,
                             ModelData & model,
                             GeneralSettings & generalSettings,
                             Firmware * firmware,
                             CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels)
{
  Board::Type board = firmware->getBoard();
  chnCapability = firmware->getCapability(Outputs);

  dialogFilteredItemModels = new FilteredItemModelFactory();

  int crvid = dialogFilteredItemModels->registerItemModel(
    new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                          RawSource::CurvesGroup | RawSource::NoneGroup), "Curves");
  connectItemModelEvents(dialogFilteredItemModels->getItemModel(crvid));

  int gvid = dialogFilteredItemModels->registerItemModel(
    new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource),
                          RawSource::GVarsGroup), "Global Variables");

  QStringList headerLabels;
  headerLabels << "#" << tr("Name") << tr("Subtrim") << tr("Min") << tr("Max")
               << tr("Direction") << tr("Curve") << tr("Plot") << tr("PPM Center")
               << tr("Linear Subtrim");

  TableLayout *tableLayout = new TableLayout(this, chnCapability, headerLabels);

  for (int i = 0; i < chnCapability; i++) {
    int col = 0;
    // Channel label
    QLabel *label = new QLabel(this);
    label->setText(tr("CH%1").arg(i+1));
    label->setProperty("index", i);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    label->setContextMenuPolicy(Qt::CustomContextMenu);
    label->setToolTip(tr("Popup menu available"));
    label->setMouseTracking(true);
    connect(label, &QMenu::customContextMenuRequested, this,
        &ChannelsPanel::onCustomContextMenuRequested);
    tableLayout->addWidget(i, col++, label);

    // Channel name
    name[i] = new QLineEdit(this);
    name[i]->setProperty("index", i);
    name[i]->setMaxLength(firmware->getCapability(ChannelsName));
    name[i]->setValidator(new NameValidator(board, this));
    connect(name[i], &QLineEdit::editingFinished, this, &ChannelsPanel::nameEdited);
    tableLayout->addWidget(i, col++, name[i]);

    // Channel offset
    // chnOffset[i] = new LimitsGroup(firmware, tableLayout, i, col++,
    //   model.limitData[i].offset, model, generalSettings, -1000, 1000, 0,
    //   dialogFilteredItemModels->getItemModel(gvid), this);

    double displayStep;
    QString suffix;

    if (generalSettings.ppmunit == GeneralSettings::PPM_US) {
      displayStep = 0.512;
      suffix = tr("us");
    } else {
      displayStep = 0.1;
      suffix = tr("us");
    }

    chnOffset[i] = new RawSourceExtWidget(this, &model, sharedItemModels,
      &model.limitData[i].offset,RawSource(SOURCE_TYPE_NUMBER),
      RawSource::GVarsGroup | RawSource::NoneGroup,
      UI_FLAG_USE | UI_FLAG_DBLSPINBOX | UI_FLAG_SOURCE, tr("GV"),
      -1000, 1000, displayStep, 1, suffix);
    // Channel min
    // chnMin[i] = new LimitsGroup(firmware, tableLayout, i, col++,
    //   model.limitData[i].min, model, generalSettings, -model.getChannelsMax() * 10, 0, -1000,
    //   dialogFilteredItemModels->getItemModel(gvid), this);
    chnMin[i] = new RawSourceExtWidget(this, &model, sharedItemModels,
      &model.limitData[i].offset,RawSource(SOURCE_TYPE_NUMBER),
      RawSource::GVarsGroup | RawSource::NoneGroup,
      UI_FLAG_USE | UI_FLAG_DBLSPINBOX | UI_FLAG_SOURCE, tr("GV"),
      -1000, 1000, displayStep, 1, suffix);
    // Channel max
    // chnMax[i] = new LimitsGroup(firmware, tableLayout, i, col++,
    //   model.limitData[i].max, model, generalSettings, 0, model.getChannelsMax() * 10, 1000,
    //   dialogFilteredItemModels->getItemModel(gvid), this);
    chnMax[i] = new RawSourceExtWidget(this, &model, sharedItemModels,
      &model.limitData[i].offset,RawSource(SOURCE_TYPE_NUMBER),
      RawSource::GVarsGroup | RawSource::NoneGroup,
      UI_FLAG_USE | UI_FLAG_DBLSPINBOX | UI_FLAG_SOURCE, tr("GV"),
      -1000, 1000, displayStep, 1, suffix);
    // Channel inversion
    invCB[i] = new QComboBox(this);
    invCB[i]->insertItems(0, QStringList() << tr("---") << tr("INV"));
    invCB[i]->setProperty("index", i);
    connect(invCB[i], &QComboBox::currentIndexChanged, this, &ChannelsPanel::invEdited);
    tableLayout->addWidget(i, col++, invCB[i]);

    // curveCB[i] = new QComboBox(this);
    // curveCB[i]->setProperty("index", i);
    // tableLayout->addWidget(i, col++, curveCB[i]);

    // curveImage[i] = new CurveImageWidget(this);
    // curveImage[i]->setProperty("index", i);
    // curveImage[i]->setFixedSize(QSize(100, 100));
    // tableLayout->addWidget(i, col++, curveImage[i]);

    // curveGroup[i] = new CurveReferenceUIManager(curveCB[i], curveImage[i],
    //                       model.limitData[i].curve, model, sharedItemModels, this);
    curve[i] = new CurveReferenceWidget(this, &model, sharedItemModels,
      &model.limitData[i].offset,RawSource(SOURCE_TYPE_NUMBER),
      RawSource::CurvesGroup | RawSource::NoneGroup,
      UI_FLAG_SOURCE | UI_FLAG_CURVE_IMAGE);
    // PPM center
    int ppmCenterMax = firmware->getCapability(PPMCenter);
    centerSB[i] = new QSpinBox(this);
    centerSB[i]->setProperty("index", i);
    centerSB[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    centerSB[i]->setSuffix("us");
    centerSB[i]->setMinimum(1500 - ppmCenterMax);
    centerSB[i]->setMaximum(1500 + ppmCenterMax);
    centerSB[i]->setValue(1500);
    connect(centerSB[i], &QSpinBox::editingFinished, this, &ChannelsPanel::ppmcenterEdited);
    tableLayout->addWidget(i, col++, centerSB[i]);

    // Symetrical limits
    symlimitsChk[i] = new QCheckBox(this);
    symlimitsChk[i]->setProperty("index", i);
    symlimitsChk[i]->setChecked(model.limitData[i].symetrical);
    connect(symlimitsChk[i], &QCheckBox::toggled, this, &ChannelsPanel::symlimitsEdited);
    tableLayout->addWidget(i, col++, symlimitsChk[i]);
  }

  update();

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->pushRowsUp(chnCapability + 1);

  adjustSize();
}

ChannelsPanel::~ChannelsPanel()
{
  // compiler warning if delete[]
  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    delete name[i];
    delete chnOffset[i];
    delete chnMin[i];
    delete chnMax[i];
    delete invCB[i];
    delete curveCB[i];
    delete centerSB[i];
    delete symlimitsChk[i];
    delete curveGroup[i];
  }
  delete dialogFilteredItemModels;
}

void ChannelsPanel::symlimitsEdited()
{
  if (!lock) {
    QCheckBox *ckb = qobject_cast<QCheckBox*>(sender());
    int index = ckb->property("index").toInt();
    model->limitData[index].symetrical = (ckb->checkState() ? 1 : 0);
    emit modified();
  }
}

void ChannelsPanel::nameEdited()
{
  if (!lock) {
    QLineEdit *le = qobject_cast<QLineEdit*>(sender());
    int index = le->property("index").toInt();
    if (model->limitData[index].name != le->text()) {
      strcpy(model->limitData[index].name, le->text().toLatin1());
      updateItemModels();
      emit modified();
    }
  }
}

void ChannelsPanel::refreshExtendedLimits()
{
  int channelMax = model->getChannelsMax() * 10;

  for (int i = 0 ; i < CPN_MAX_CHNOUT; i++) {
    chnMin[i]->updateMinMax(channelMax);
    chnMax[i]->updateMinMax(channelMax);
  }
  emit modified();
}

void ChannelsPanel::invEdited()
{
  if (!lock) {
    QComboBox *cb = qobject_cast<QComboBox*>(sender());
    int index = cb->property("index").toInt();
    model->limitData[index].revert = cb->currentIndex();
    emit modified();
  }
}

void ChannelsPanel::ppmcenterEdited()
{
  if (!lock) {
    QSpinBox *sb = qobject_cast<QSpinBox*>(sender());
    int index = sb->property("index").toInt();
    model->limitData[index].ppmCenter = sb->value() - 1500;
    emit modified();
  }
}

void ChannelsPanel::update()
{
  for (int i = 0; i < chnCapability; i++) {
    updateLine(i);
  }
}

void ChannelsPanel::updateLine(int i)
{
  lock = true;
  LimitData &chn = model->limitData[i];
  name[i]->setText(chn.name);
  chnOffset[i]->setValue(chn.offset);
  chnMin[i]->setValue(chn.min);
  chnMax[i]->setValue(chn.max);
  invCB[i]->setCurrentIndex((chn.revert) ? 1 : 0);
  centerSB[i]->setValue(chn.ppmCenter + 1500);
  symlimitsChk[i]->setChecked(chn.symetrical);
  lock = false;
}

void ChannelsPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    model->limitsSet(selectedIndex, data);
    updateLine(selectedIndex);
    updateItemModels();
  }
}

void ChannelsPanel::cmDelete()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Channel. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  model->limitsDelete(selectedIndex);
  for (int i = selectedIndex; i < chnCapability; i++) {
    updateLine(i);
  }

  updateItemModels();
}

void ChannelsPanel::cmCopy()
{
  QByteArray data;
  model->limitsGet(selectedIndex, data);
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_CHANNEL, data);
  QApplication::clipboard()->setMimeData(mimeData,QClipboard::Clipboard);
}

void ChannelsPanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Channel. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  cmCopy();
  cmClear(false);
}

void ChannelsPanel::onCustomContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);
  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"), this,
                        SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"), this,
                        SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"), this,
                        SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"), this,
                        SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"), this,
                        SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"), this,
                        SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"), this,
                        SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"), this,
                        SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"), this,
                      SLOT(cmClearAll()));
  contextMenu.exec(globalPos);
}

bool ChannelsPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_CHANNEL)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_CHANNEL));
    return true;
  }
  return false;
}

bool ChannelsPanel::insertAllowed() const
{
  return ((selectedIndex < chnCapability - 1) &&
          (model->limitData[chnCapability - 1].isEmpty()));
}

bool ChannelsPanel::moveDownAllowed() const
{
  return selectedIndex < chnCapability - 1;
}

bool ChannelsPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void ChannelsPanel::cmMoveUp()
{
  model->limitsMove(selectedIndex, -1);
  updateLine(selectedIndex - 1);
  updateLine(selectedIndex);
  updateItemModels();
}

void ChannelsPanel::cmMoveDown()
{
  model->limitsMove(selectedIndex, 1);
  updateLine(selectedIndex);
  updateLine(selectedIndex + 1);
  updateItemModels();
}

void ChannelsPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Channel. Are you sure?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  model->limitsClear(selectedIndex);
  updateLine(selectedIndex);
  updateItemModels();
}

void ChannelsPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Channels. Are you sure?"),
                            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  model->limitsClearAll();
  update();
  updateItemModels();
}

void ChannelsPanel::cmInsert()
{
  model->limitsInsert(selectedIndex);
  for (int i = selectedIndex; i < chnCapability; i++) {
    updateLine(i);
  }

  updateItemModels();
}

void ChannelsPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this,
    &ChannelsPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this,
    &ChannelsPanel::onItemModelUpdateComplete);
}

void ChannelsPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
}

void ChannelsPanel::onItemModelUpdateComplete()
{
  update();
  lock = false;
}

void ChannelsPanel::updateItemModels()
{
  sharedItemModels->update(AbstractItemModel::IMUE_Channels);
  emit modified();
}
