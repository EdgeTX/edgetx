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

#include "convmapdialog.h"
#include "boards.h"
#include "filtereditemmodels.h"
#include "generalsettings.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QSpacerItem>

constexpr char FIM_STICKS[]   {"Sticks"};
constexpr char FIM_POTS[]     {"Pots"};
constexpr char FIM_SWITCHES[] {"Switches"};

ConvMapDialog::ConvMapDialog(QWidget * parent, RadioDataConversionState & cstate):
  QDialog(parent),
  cstate(cstate),
  params(new QList<QWidget *>),
  row(0)
{
  setWindowTitle(tr("Radio Conversion"));
  setSizeGripEnabled(true);

  QVBoxLayout *baseLayout = new QVBoxLayout(this);
  QWidget *wgt = new QWidget();
  wgt->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  grid = new QGridLayout(wgt);

  QScrollArea *sa = new QScrollArea();
  sa->setWidget(wgt);
  baseLayout->addWidget(sa);
  sa->setWidgetResizable(true);
  sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  buildToSourcesItemModel();

  toSourcesFilteredModels = new FilteredItemModelFactory();
  toSourcesFilteredModels->registerItemModel(new FilteredItemModel(toSourcesItemModel, SticksGroup),   FIM_STICKS);
  toSourcesFilteredModels->registerItemModel(new FilteredItemModel(toSourcesItemModel, PotsGroup),     FIM_POTS);
  toSourcesFilteredModels->registerItemModel(new FilteredItemModel(toSourcesItemModel, SwitchesGroup), FIM_SWITCHES);

  addHeading();

  int count = Boards::getCapability(cstate.fromType, Board::Sticks);

  if (count > 0) {
    addSection(tr("Axis"));
    addLabel(tr("Name"));
    addParams();

    for (int i = 0; i < count; i++) {
      addStick(i);
    }
  }

  count = Boards::getCapability(cstate.fromType, Board::Inputs);

  if (count > 0) {
    addSection(tr("Pots"));
    addLabel(tr("Name"));
    addLabel(tr("Type"));
    addParams();

    for (int i = Boards::getCapability(cstate.fromType, Board::Sticks); i < count; i++) {
      if (Boards::isInputConfigurable(i, cstate.fromType))
        addFlex(i);
    }
  }

  if (Boards::getCapability(cstate.fromType, Board::Switches)) {
    addSection(tr("Switches"));
    addLabel(tr("Name"));
    addLabel("");
    addLabel(tr("Type"));
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, cstate.fromType) &&
          !cstate.fromGS()->isSwitchFunc(i) && !cstate.fromGS()->isSwitchFlex(i))
        addSwitch(i);
    }
  }

  if (Boards::getCapability(cstate.fromType, Board::FlexSwitches)) {
    addSection(tr("Flex Switches"));
    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Source"));
    addLabel(tr("Type"));
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, cstate.fromType) && cstate.fromGS()->isSwitchFlex(i))
        addSwitch(i);
    }
  }

  if (Boards::getCapability(cstate.fromType, Board::FunctionSwitches)) {
    addSection(tr("Customisable Switches"));

    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Type"));
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, cstate.fromType) && cstate.fromGS()->isSwitchFunc(i))
        addSwitch(i);
    }
  }

  QSpacerItem * hspacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  grid->addItem(hspacer, 0, grid->columnCount(), grid->rowCount());
  QSpacerItem * vspacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
  grid->addItem(vspacer, grid->rowCount(), 0, 1, grid->columnCount());

  QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Ok & QDialogButtonBox::Cancel);
  baseLayout->addWidget(btns);

}

ConvMapDialog::~ConvMapDialog()
{
  delete toSourcesFilteredModels;
  delete toSourcesItemModel;
}

void ConvMapDialog::addHeading()
{
  QLabel *fromBd = new QLabel(QString("<b>%1</b>").arg(Boards::getBoardName(cstate.fromType)));
  grid->addWidget(fromBd, row, 0, 1, 3);
  QLabel *toBd = new QLabel(QString("<b>%1</b>").arg(Boards::getBoardName(cstate.toType)));
  grid->addWidget(toBd, row++, 3, 1, 2);
}

void ConvMapDialog::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  params->append(label);
}

void ConvMapDialog::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void ConvMapDialog::addParams()
{
  for (int i = 0; i < params->size(); i++) {
    grid->addWidget(params->at(i), row, i);
  }

  row += 1;
  params->clear();
}

void ConvMapDialog::addSection(QString text)
{
  QLabel *lbl = new QLabel(QString("<b>%1</b>").arg(text));
  grid->addWidget(lbl, row++, 0, 1, 3);
}

void ConvMapDialog::addStick(int index)
{
  const GeneralSettings::InputConfig &config = cstate.fromGS()->inputConfig[index];
  const QString dfltName(Boards::getInputName(index, cstate.fromType));
  addLabel(DataHelpers::getCompositeName(dfltName, config.name, true));
  addLabel(QString());
  addLabel(QString());
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toSourcesFilteredModels->getItemModel(FIM_STICKS));
  int idx = cbo->findText(dfltName);
  cbo->setCurrentIndex(idx >= 0 ? idx : 0);
  cbo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  params->append(cbo);
  addParams();
}

void ConvMapDialog::addFlex(int index)
{
  const GeneralSettings::InputConfig &config = cstate.fromGS()->inputConfig[index];
  const QString dfltName(Boards::getInputName(index, cstate.fromType));
  addLabel(DataHelpers::getCompositeName(dfltName, config.name, true));
  addLabel(Boards::flexTypeToString(config.flexType));
  addLabel(QString());
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toSourcesFilteredModels->getItemModel(FIM_POTS));
  int idx = cbo->findText(dfltName);
  cbo->setCurrentIndex(idx >= 0 ? idx : 0);
  cbo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  params->append(cbo);
  addParams();
}

void ConvMapDialog::addSwitch(int index)
{
  const GeneralSettings::SwitchConfig &config = cstate.fromGS()->switchConfig[index];
  Board::SwitchInfo info = Boards::getSwitchInfo(index);
  const QString dfltName(Boards::getSwitchName(index, cstate.fromType));
  addLabel(DataHelpers::getCompositeName(dfltName, config.name, true));

  if (cstate.fromGS()->isSwitchFlex(index))
    addLabel(QString::number(config.inputIdx));
  else
    addLabel(QString());

  addLabel(Boards::switchTypeToString(config.type));
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toSourcesFilteredModels->getItemModel(FIM_SWITCHES));
  int idx = cbo->findText(dfltName);
  cbo->setCurrentIndex(idx >= 0 ? idx : 0);
  cbo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  params->append(cbo);
  addParams();
}

void ConvMapDialog::buildToSourcesItemModel()
{
  toSourcesItemModel = new AbstractStaticItemModel();

  unsigned int numSticks = Boards::getCapability(cstate.toType, Board::Sticks);
  addSourceItems(SOURCE_TYPE_NONE,   NoneGroup,     1);
  addSourceItems(SOURCE_TYPE_INPUT,  SticksGroup,   numSticks);
  addSourceItems(SOURCE_TYPE_INPUT,  PotsGroup,     Boards::getCapability(cstate.toType, Board::Inputs) - numSticks, numSticks);
  addSourceItems(SOURCE_TYPE_SWITCH, SwitchesGroup, Boards::getCapability(cstate.toType, Board::Switches));

  toSourcesItemModel->loadItemList();
}

void ConvMapDialog::addSourceItems(const RawSourceType & type, const int group, unsigned int count, unsigned int start)
{
  const unsigned int idxAdj = (type == SOURCE_TYPE_NONE ? -1 : 0);

  unsigned int first = start + 1;
  unsigned int last = start + count + 1;

  for (unsigned int i = first; i < last; ++i) {
    const RawSource src = RawSource(type, i + idxAdj);
    QStandardItem * modelItem = new QStandardItem();
    modelItem->setData(src.toValue(), AbstractItemModel::IMDR_Id);
    modelItem->setData(type, AbstractItemModel::IMDR_Type);
    modelItem->setData(group, AbstractItemModel::IMDR_Flags);
    modelItem->setText(src.toString(nullptr, cstate.toGS(), cstate.toType));
    modelItem->setData(src.isAvailable(nullptr, cstate.toGS(), cstate.toType), AbstractItemModel::IMDR_Available);
    toSourcesItemModel->appendRow(modelItem);
  }
}
