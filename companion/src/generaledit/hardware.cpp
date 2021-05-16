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

#include "hardware.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"
#include "autolineedit.h"
#include "autocombobox.h"
#include "autocheckbox.h"
#include "autospinbox.h"
#include "autodoublespinbox.h"

#include <QLabel>
#include <QGridLayout>
#include <QFrame>

constexpr char FIM_SWITCHTYPE2POS[]  {"Switch Type 2 Pos"};
constexpr char FIM_SWITCHTYPE3POS[]  {"Switch Type 3 Pos"};

HardwarePanel::HardwarePanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  editorItemModels(sharedItemModels)
{
  editorItemModels->registerItemModel(Boards::potTypeItemModel());
  editorItemModels->registerItemModel(Boards::sliderTypeItemModel());
  int id = editorItemModels->registerItemModel(Boards::switchTypeItemModel());

  tabFilteredModels = new FilteredItemModelFactory();
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext2Pos), FIM_SWITCHTYPE2POS);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext3Pos), FIM_SWITCHTYPE3POS);

  int antmodelid = editorItemModels->registerItemModel(GeneralSettings::antennaModeItemModel());
  int btmodelid = editorItemModels->registerItemModel(GeneralSettings::bluetoothModeItemModel());
  int auxmodelid = editorItemModels->registerItemModel(GeneralSettings::auxSerialModeItemModel());
  int baudmodelid = editorItemModels->registerItemModel(GeneralSettings::telemetryBaudrateItemModel());

  grid = new QGridLayout(this);
  int count;
  int row = 0;

  count = Boards::getCapability(board, Board::Sticks);
  if (count) {
    for (int i = 0; i < count; i++) {
      addStick(i, row);
    }
  }

  count = Boards::getCapability(board, Board::Pots);
  if (count) {
    for (int i = 0; i < count; i++) {
      addPot(i, row);
    }
  }

  count = Boards::getCapability(board, Board::Sliders);
  if (count) {
    for (int i = 0; i < count; i++) {
      addSlider(i, row);
    }
    addLine(row);
  }

  count = Boards::getCapability(board, Board::Switches);
  if (count) {
    for (int i = 0; i < count; i++) {
      addSwitch(i, row);
    }
    addLine(row);
  }

  addLabel(tr("Battery Offset"), row, 0);
  AutoDoubleSpinBox *txVoltageCalibration = new AutoDoubleSpinBox(this);
  FieldRange txVCRng = GeneralSettings::getTxVoltageCalibrationRange();
  txVoltageCalibration->setDecimals(txVCRng.decimals);
  txVoltageCalibration->setSingleStep(txVCRng.step);
  txVoltageCalibration->setSuffix(txVCRng.unit);
  txVoltageCalibration->setField(generalSettings.txVoltageCalibration);
  addParams(row, txVoltageCalibration);

  if (Boards::getCapability(board, Board::HasRTC)) {
    addLabel(tr("RTC Battery Check"), row, 0);
    AutoCheckBox *rtcCheckDisable = new AutoCheckBox(this);
    rtcCheckDisable->setField(generalSettings.rtcCheckDisable, this, true);
    addParams(row, rtcCheckDisable);
  }

  if (firmware->getCapability(HasBluetooth)) {
    addLabel(tr("Bluetooth"), row, 0);

    QGridLayout *btlayout = new QGridLayout();

    AutoComboBox *bluetoothMode = new AutoComboBox(this);
    bluetoothMode->setModel(editorItemModels->getItemModel(btmodelid));
    bluetoothMode->setField(generalSettings.bluetoothMode, this);
    btlayout->addWidget(bluetoothMode, 0, 0);

    QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    btlayout->addItem(spacer, 0, 1);

    QLabel *btnamelabel = new QLabel(this);
    btnamelabel->setText(tr("Device Name:"));
    btlayout->addWidget(btnamelabel, 0, 2);

    AutoLineEdit *bluetoothName = new AutoLineEdit(this);
    bluetoothName->setField(generalSettings.bluetoothName, BLUETOOTH_NAME_LEN, this);
    btlayout->addWidget(bluetoothName, 0, 3);

    grid->addLayout(btlayout, row, 1);
    row++;
  }

  if (firmware->getCapability(HasAntennaChoice)) {
    addLabel(tr("Antenna"), row, 0);
    AutoComboBox *antennaMode = new AutoComboBox(this);
    antennaMode->setModel(editorItemModels->getItemModel(antmodelid));
    antennaMode->setField(generalSettings.antennaMode, this);
    addParams(row, antennaMode);
  }

  if (firmware->getCapability(HasAuxSerialMode)) {
    QString lbl = "Serial Port";
    if (IS_RADIOMASTER_TX16S(board))
      lbl.append(" (TTL)");
    addLabel(tr("%1").arg(lbl), row, 0);
    AutoComboBox *serialPortMode = new AutoComboBox(this);
    serialPortMode->setModel(editorItemModels->getItemModel(auxmodelid));
    serialPortMode->setField(generalSettings.auxSerialMode);
    addParams(row, serialPortMode);
  }

  if (firmware->getCapability(HasAux2SerialMode)) {
    QString lbl = "Serial Port 2";
    if (IS_RADIOMASTER_TX16S(board))
      lbl.append(" (TTL)");
    addLabel(tr("%1").arg(lbl), row, 0);
    AutoComboBox *serialPort2Mode = new AutoComboBox(this);
    serialPort2Mode->setModel(editorItemModels->getItemModel(auxmodelid));
    serialPort2Mode->setField(generalSettings.aux2SerialMode);
    addParams(row, serialPort2Mode);
  }

  if (firmware->getCapability(HasADCJitterFilter)) {
    addLabel(tr("ADC Filter"), row, 0);
    AutoCheckBox *filterEnable = new AutoCheckBox(this);
    filterEnable->setField(generalSettings.jitterFilter, this, true);
    addParams(row, filterEnable);
  }

  if (firmware->getCapability(HasSportConnector)) {
    addLabel(tr("S.Port Power"), row, 0);
    AutoCheckBox *sportPower = new AutoCheckBox(this);
    sportPower->setField(generalSettings.sportPower, this);
    addParams(row, sportPower);
  }

  if (firmware->getCapability(HasTelemetryBaudrate)) {
    addLabel(tr("Maximum Baud"), row, 0);
    AutoComboBox *maxBaudRate = new AutoComboBox(this);
    maxBaudRate->setModel(editorItemModels->getItemModel(baudmodelid));
    maxBaudRate->setField(generalSettings.telemetryBaudrate, this);
    addParams(row, maxBaudRate);
  }

  if (firmware->getCapability(HastxCurrentCalibration)) {
    addLabel(tr("Current Offset"), row, 0);
    AutoSpinBox *txCurrentCalibration = new AutoSpinBox(this);
    FieldRange txCCRng = GeneralSettings::getTxCurrentCalibration();
    txCurrentCalibration->setSuffix(txVCRng.unit);
    txCurrentCalibration->setField(generalSettings.txCurrentCalibration);
    addParams(row, txCurrentCalibration);
  }

  addVSpring(grid, 0, grid->rowCount());
  addHSpring(grid, grid->columnCount(), 0);
  disableMouseScrolling();
}

HardwarePanel::~HardwarePanel()
{
  delete tabFilteredModels;
}

void HardwarePanel::addStick(int index, int & row)
{
  int col = 0;
  addLabel(Boards::getAnalogInputName(board, index), row, col++);

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setField(generalSettings.stickName[index], HARDWARE_NAME_LEN, this);

  addParams(row, name);
}

void HardwarePanel::addPot(int index, int & row)
{
  addLabel(Boards::getAnalogInputName(board, Boards::getCapability(board, Board::Sticks) + index), row, 0);

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setField(generalSettings.potName[index], HARDWARE_NAME_LEN, this);

  AutoComboBox *type = new AutoComboBox(this);
  type->setModel(editorItemModels->getItemModel(AIM_BOARDS_POT_TYPE));
  type->setField(generalSettings.potConfig[index], this);

  addParams(row, name, type);
}

void HardwarePanel::addSlider(int index, int & row)
{
  addLabel(Boards::getAnalogInputName(board, Boards::getCapability(board, Board::Sticks) +
                                             Boards::getCapability(board, Board::Pots) + index), row, 0);

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setField(generalSettings.sliderName[index], HARDWARE_NAME_LEN, this);

  AutoComboBox *type = new AutoComboBox(this);
  type->setModel(editorItemModels->getItemModel(AIM_BOARDS_SLIDER_TYPE));
  type->setField(generalSettings.sliderConfig[index], this);

  addParams(row, name, type);
}

void HardwarePanel::addSwitch(int index, int & row)
{
  addLabel(Boards::getSwitchInfo(board, index).name, row, 0);

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setField(generalSettings.switchName[index], HARDWARE_NAME_LEN, this);

  AutoComboBox *type = new AutoComboBox(this);
  Board::SwitchInfo switchInfo = Boards::getSwitchInfo(board, index);
  type->setModel(switchInfo.config < Board::SWITCH_3POS ? tabFilteredModels->getItemModel(FIM_SWITCHTYPE2POS) :
                                                          tabFilteredModels->getItemModel(FIM_SWITCHTYPE3POS));
  type->setField(generalSettings.switchConfig[index], this);

  addParams(row, name, type);
}

void HardwarePanel::addLabel(QString text, int row, int col)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void HardwarePanel::addLine(int & row)
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row, 0, 1, grid->columnCount());
  row++;
}

void HardwarePanel::addParams(int & row, QWidget * widget1, QWidget * widget2)
{
  QGridLayout *subgrid = new QGridLayout();
  subgrid->addWidget(widget1, 0, 0);
  if (widget2)
    subgrid->addWidget(widget2, 0, 1);
  else
    addHSpring(subgrid, 1, 0);
  addHSpring(subgrid, 2, 0);
  grid->addLayout(subgrid, row, 1);
  row++;
}
