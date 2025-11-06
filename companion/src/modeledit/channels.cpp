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
#include "namevalidator.h"
#include "rawsourcewidget.h"

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

  QStringList headerLabels;
  headerLabels << "#" << tr("Name") << tr("Subtrim") << tr("Min") << tr("Max")
               << tr("Invert") << tr("Curve") << tr("PPM Center")
               << tr("Subtrim Mode");

  TableLayout *tableLayout = new TableLayout(this, chnCapability, headerLabels);
  int col = 0;
  const int leNameWidth = Helpers::calcQLineEditWidth(firmware->getCapability(ChannelsName) + 4/*abitary*/);

  for (int i = 0; i < chnCapability; i++) {
    col = 0;
    // Label
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

    // Name
    leName[i] = new QLineEdit(this);
    leName[i]->setProperty("index", i);
    leName[i]->setMaxLength(firmware->getCapability(ChannelsName));
    leName[i]->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    leName[i]->setValidator(new NameValidator(board, this));
    leName[i]->setFixedWidth(leNameWidth);
    connect(leName[i], &QLineEdit::editingFinished, this, &ChannelsPanel::nameEdited);
    tableLayout->addWidget(i, col++, leName[i]);

    double step;
    QString suffix;

    if (generalSettings.ppmunit == GeneralSettings::PPM_US) {
      step = 0.512;
      suffix = tr("us");
    } else {
      step = 0.1;
      suffix = tr("%");
    }

    // Subtrim
    rswSubTrim[i] = new RawSourceWidget(this, &model, sharedItemModels,
                    &model.limitData[i].offset, RawSource::GVarsGroup,
                    UI_FLAG_LIST | UI_FLAG_VALUE, RawSource(SOURCE_TYPE_NUMBER),
                    tr("GV"), -1000, 1000, 10, 0.1, "", suffix);
    connect(rswSubTrim[i], &RawSourceWidget::dataChanged, this, &ModelPanel::modified);
    connect(rswSubTrim[i], &RawSourceWidget::resized, [=] () { adjustSize(); });
    tableLayout->addWidget(i, col++, rswSubTrim[i]);

    // Minimum
    rswMin[i] = new RawSourceWidget(this, &model, sharedItemModels,
                &model.limitData[i].min, RawSource::GVarsGroup,
                UI_FLAG_LIST | UI_FLAG_VALUE, RawSource(SOURCE_TYPE_NUMBER, -model.getChannelsMax() * 10),
                tr("GV"), -model.getChannelsMax() * 10, 0, 10, step, "",
                suffix);
    connect(rswMin[i], &RawSourceWidget::dataChanged, this, &ModelPanel::modified);
    connect(rswMin[i], &RawSourceWidget::resized, [=] () { adjustSize(); });
    tableLayout->addWidget(i, col++, rswMin[i]);

    // Maximum
    rswMax[i] = new RawSourceWidget(this, &model, sharedItemModels,
                &model.limitData[i].max, RawSource::GVarsGroup,
                UI_FLAG_LIST | UI_FLAG_VALUE, RawSource(SOURCE_TYPE_NUMBER, model.getChannelsMax() * 10),
                tr("GV"), 0, model.getChannelsMax() * 10, 10, step, "",
                suffix);
    connect(rswMax[i], &RawSourceWidget::dataChanged, this, &ModelPanel::modified);
    connect(rswMax[i], &RawSourceWidget::resized, [=] () { adjustSize(); });
    tableLayout->addWidget(i, col++, rswMax[i]);

    // Direction invert
    chkInverted[i] = new QCheckBox(" ", this);  // Qt 6.9 Windows fix for checkbox display
    chkInverted[i]->setProperty("index", i);
    connect(chkInverted[i], &QCheckBox::stateChanged, this, &ChannelsPanel::invertedEdited);
    tableLayout->addWidget(i, col++, chkInverted[i], Qt::AlignCenter);

    // Curve
    crwCurve[i] = new RawSourceWidget(this, &model, sharedItemModels,
                  &model.limitData[i].curve,
                  RawSource::CurvesGroup | RawSource::NoneGroup,
                  UI_FLAG_LIST | UI_FLAG_CURVE_IMAGE,
                  RawSource(), "", 0, 0, 1, 1.0, "", "", false);
    connect(crwCurve[i], &RawSourceWidget::dataChanged, this, &ModelPanel::modified);
    connect(crwCurve[i], &RawSourceWidget::resized, [=] () { adjustSize(); });
    tableLayout->addWidget(i, col++, crwCurve[i]);

    // PPM center
    int ppmCenterMax = firmware->getCapability(PPMCenter);
    sbxPPMCenter[i] = new QSpinBox(this);
    sbxPPMCenter[i]->setProperty("index", i);
    sbxPPMCenter[i]->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    sbxPPMCenter[i]->setSuffix("us");
    sbxPPMCenter[i]->setMinimum(1500 -ppmCenterMax);
    sbxPPMCenter[i]->setMaximum(1500 + ppmCenterMax);
    connect(sbxPPMCenter[i], &QSpinBox::editingFinished,
            this, &ChannelsPanel::ppmCenterEdited);
    tableLayout->addWidget(i, col++, sbxPPMCenter[i]);

    // Subtrim mode
    cboSubTrimMode[i] = new QComboBox(this);
    cboSubTrimMode[i]->setProperty("index", i);
    cboSubTrimMode[i]->setModel(LimitData::symetricalModel());
    connect(cboSubTrimMode[i], QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChannelsPanel::subTrimModeEdited);
    tableLayout->addWidget(i, col++, cboSubTrimMode[i]);
  }

  update();

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->pushRowsUp(chnCapability + 1);
  tableLayout->pushColumnsLeft(col);

  adjustSize();
}

ChannelsPanel::~ChannelsPanel()
{
  // compiler warning if delete[]
  for (int i = 0; i < CPN_MAX_CHNOUT; i++) {
    delete leName[i];
    delete rswSubTrim[i];
    delete rswMin[i];
    delete rswMax[i];
    delete chkInverted[i];
    delete crwCurve[i];
    delete sbxPPMCenter[i];
    delete cboSubTrimMode[i];
  }
}

void ChannelsPanel::subTrimModeEdited()
{
  if (!lock) {
    QComboBox *cbo = qobject_cast<QComboBox*>(sender());
    int index = cbo->property("index").toInt();
    model->limitData[index].symetrical = (bool)(cbo->currentIndex());
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
    rswMin[i]->updateMinMax(-channelMax, channelMax);
    rswMax[i]->updateMinMax(-channelMax, channelMax);
  }

  emit modified();
}

void ChannelsPanel::invertedEdited()
{
  if (!lock) {
    QCheckBox *cb = qobject_cast<QCheckBox*>(sender());
    int index = cb->property("index").toInt();
    model->limitData[index].revert = cb->isChecked();
    emit modified();
  }
}

void ChannelsPanel::ppmCenterEdited()
{
  if (!lock) {
    QSpinBox *sb = qobject_cast<QSpinBox*>(sender());
    int index = sb->property("index").toInt();
    model->limitData[index].ppmCenter = sb->value();
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
  leName[i]->setText(chn.name);
  rswSubTrim[i]->update();
  rswMin[i]->update();
  rswMax[i]->update();
  chkInverted[i]->setChecked(chn.revert);
  crwCurve[i]->update();
  sbxPPMCenter[i]->setValue(chn.ppmCenter);
  cboSubTrimMode[i]->setCurrentIndex((int)chn.symetrical);
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
