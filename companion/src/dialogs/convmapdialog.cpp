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
#include "compounditemmodels.h"
#include "generalsettings.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

ConvMapDialog::ConvMapDialog(QWidget * parent, RadioDataConversionState & cstate):
  QDialog(parent),
  cstate(cstate),
  params(new QList<QWidget *>),
  row(0)
{
  buildToSticksItemModel();
  buildToInputsItemModel();
  buildToSwitchesItemModel();

  // maybe inherit genericpanel ???????
  setWindowTitle(tr("Radio Conversion"));
  // set other attributes

  // add scrollable area

  // set initial size

  grid = new QGridLayout(this);
  int count = 0;

  count = Boards::getCapability(cstate.fromType, Board::Sticks);

  if (count > 0) {
    addSection(tr("Axis"));
    addLabel("");
    addLabel(tr("Name"));
    addParams();

    for (int i = 0; i < count; i++) {
      addStick(i);
    }
  }

  count = Boards::getCapability(cstate.fromType, Board::Inputs);

  if (count > 0) {
    addSection(tr("Pots"));
    addLabel("");
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
    addLabel("");
    addLabel(tr("Name"));
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

  QDialogButtonBox *btns = new QDialogButtonBox(this);
  //addVSpring(grid, 0, grid->rowCount());
  //addHSpring(grid, grid->columnCount(), 0);
  //disableMouseScrolling();
  //delayed resize
}

void ConvMapDialog::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
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
  addLabel(QString("<b>%1</b>").arg(text));
  addParams();
}

void ConvMapDialog::addStick(int index)
{
  const GeneralSettings::InputConfig &config = cstate.fromGS()->inputConfig[index];
  addLabel(Boards::getInputName(index, cstate.fromType));
  addLabel(config.name);
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toSticksItemModel);
  cbo->setCurrentIndex(cbo->findText(config.name));
  params->append(cbo);
  addParams();
}

void ConvMapDialog::addFlex(int index)
{
  const GeneralSettings::InputConfig &config = cstate.fromGS()->inputConfig[index];
  addLabel(Boards::getInputName(index, cstate.fromType));
  addLabel(config.name);
  addLabel(Boards::flexTypeToString(config.flexType));
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toInputsItemModel);
  cbo->setCurrentIndex(cbo->findText(config.name));
  params->append(cbo);
  addParams();
}

void ConvMapDialog::addSwitch(int index)
{
  const GeneralSettings::SwitchConfig &config = cstate.fromGS()->switchConfig[index];
  Board::SwitchInfo info = Boards::getSwitchInfo(index);

  addLabel(Boards::getSwitchName(index));
  addLabel(config.name);

  if (cstate.fromGS()->isSwitchFlex(index)) {
    addLabel(QString::number(config.inputIdx));
  }

  addLabel(Boards::switchTypeToString(config.type));
  QComboBox *cbo = new QComboBox();
  cbo->setModel(toSwitchesItemModel);
  cbo->setCurrentIndex(cbo->findText(config.name));
  params->append(cbo);
  addParams();
}

void ConvMapDialog::buildToSticksItemModel()
{
  toSticksItemModel = new AbstractStaticItemModel();

  toSticksItemModel->appendToItemList(CPN_STR_NONE_ITEM, -1);

  for (int i = 0; i < Boards::getCapability(cstate.toType, Board::Sticks); i++) {
    toSticksItemModel->appendToItemList(Boards::getInputName(i, cstate.toType), i);
  }

  toSticksItemModel->loadItemList();
}

void ConvMapDialog::buildToInputsItemModel()
{
  toInputsItemModel = new AbstractStaticItemModel();

  toInputsItemModel->appendToItemList(CPN_STR_NONE_ITEM, -1);

  for (int i = Boards::getCapability(cstate.toType, Board::Sticks);
       i < Boards::getCapability(cstate.toType, Board::Inputs); i++) {
    if (Boards::isInputConfigurable(i, cstate.toType))
      toInputsItemModel->appendToItemList(Boards::getInputName(i, cstate.toType), i);
  }

  toInputsItemModel->loadItemList();
}

void ConvMapDialog::buildToSwitchesItemModel()
{
  toSwitchesItemModel = new AbstractStaticItemModel();

  toSwitchesItemModel->appendToItemList(CPN_STR_NONE_ITEM, -1);

  for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
    if (Boards::isSwitchConfigurable(i, cstate.toType))
      toSwitchesItemModel->appendToItemList(Boards::getSwitchName(i, cstate.toType), i);
  }

  toSwitchesItemModel->loadItemList();
}
