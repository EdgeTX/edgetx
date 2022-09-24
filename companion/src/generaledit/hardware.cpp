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
constexpr char FIM_INTERNALMODULES[] {"Internal Modules"};

class ExclusiveComboGroup: public QObject
{
  static constexpr auto _role = Qt::UserRole + 500;

  QList<QComboBox*> combos;
  std::function<bool(const QVariant&)> filter;

public:
 ExclusiveComboGroup(QObject *parent, std::function<bool(const QVariant&)> filter) :
   QObject(parent), filter(std::move(filter))
 {
 }

 void addCombo(QComboBox *comboBox)
 {
   connect(comboBox, QOverload<int>::of(&QComboBox::activated),
           [=](int index) { this->handleActivated(comboBox, index); });
   combos.append(comboBox);
  }

  void handleActivated(QComboBox* target, int index) {
    auto data = target->itemData(index);
    auto targetidx = combos.indexOf(target);
    for (auto combo : combos) {
      if (target == combo) continue;
      auto view = dynamic_cast<QListView*>(combo->view());
      Q_ASSERT(view);

      auto previous = combo->findData(targetidx, _role);
      if (previous >= 0) {
        view->setRowHidden(previous, false);
        combo->setItemData(previous, QVariant(), _role);
      }
      if (!filter(data)) {
        auto idx = combo->findData(data);
        if (idx >= 0) {
          view->setRowHidden(idx, true);
          combo->setItemData(idx, targetidx, _role);
        }
      }
    }
  }
};

HardwarePanel::HardwarePanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  editorItemModels(sharedItemModels),
  serialPortUSBVCP(nullptr)
{
  editorItemModels->registerItemModel(Boards::potTypeItemModel());
  editorItemModels->registerItemModel(Boards::sliderTypeItemModel());
  int id = editorItemModels->registerItemModel(Boards::switchTypeItemModel());

  tabFilteredModels = new FilteredItemModelFactory();
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext2Pos), FIM_SWITCHTYPE2POS);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext3Pos), FIM_SWITCHTYPE3POS);

  int antmodelid = editorItemModels->registerItemModel(GeneralSettings::antennaModeItemModel());
  int btmodelid = editorItemModels->registerItemModel(GeneralSettings::bluetoothModeItemModel());
  int auxmodelid = editorItemModels->registerItemModel(GeneralSettings::serialModeItemModel(GeneralSettings::SP_AUX1));
  int vcpmodelid = editorItemModels->registerItemModel(GeneralSettings::serialModeItemModel(GeneralSettings::SP_VCP));
  int baudmodelid = editorItemModels->registerItemModel(GeneralSettings::internalModuleBaudrateItemModel());
  int uartmodelid = editorItemModels->registerItemModel(GeneralSettings::uartSampleModeItemModel());

  id = editorItemModels->registerItemModel(ModuleData::internalModuleItemModel());
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id)), FIM_INTERNALMODULES);

  grid = new QGridLayout(this);
  int count;
  int row = 0;

  addSection(tr("Sticks"), row);

  count = Boards::getCapability(board, Board::Sticks);
  if (count) {
    for (int i = 0; i < count; i++) {
      addStick(i, row);
    }
  }

  if (IS_FLYSKY_NV14(board)) {
    addLabel(tr("Dead zone"), row, 0);
    AutoComboBox *spnStickDeadZone = new AutoComboBox(this);
    spnStickDeadZone->setModel(GeneralSettings::stickDeadZoneItemModel());
    spnStickDeadZone->setField(generalSettings.stickDeadZone, this);
    addParams(row, spnStickDeadZone);
  }

  count = Boards::getCapability(board, Board::Pots);
  count -= firmware->getCapability(HasFlySkyGimbals) ? 2 : 0;
  if (count > 0) {
    addSection(tr("Pots"), row);
    for (int i = 0; i < count; i++) {
      addPot(i, row);
    }
  }

  count = Boards::getCapability(board, Board::Sliders);
  if (count) {
    addSection(tr("Sliders"), row);
    for (int i = 0; i < count; i++) {
      addSlider(i, row);
    }
  }

  count = Boards::getCapability(board, Board::Switches);
  if (count) {
    addSection(tr("Switches"), row);
    for (int i = 0; i < count; i++) {
      addSwitch(i, row);
    }
  }

  addLine(row);

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

  if (Boards::getCapability(board, Board::HasInternalModuleSupport)) {
    m_internalModule = generalSettings.internalModule; // to permit undo
    addLabel(tr("Internal module"), row, 0);
    internalModule = new AutoComboBox(this);
    internalModule->setModel(tabFilteredModels->getItemModel(FIM_INTERNALMODULES));
    internalModule->setField(generalSettings.internalModule, this);

    connect(internalModule, &AutoComboBox::currentDataChanged, this,
            &HardwarePanel::on_internalModuleChanged);

    internalModuleBaudRate = new AutoComboBox(this);
    internalModuleBaudRate->setModel(editorItemModels->getItemModel(baudmodelid));
    internalModuleBaudRate->setField(generalSettings.internalModuleBaudrate, this);

    if (m_internalModule != MODULE_TYPE_GHOST && m_internalModule != MODULE_TYPE_CROSSFIRE) {
      generalSettings.internalModuleBaudrate = 0;
      internalModuleBaudRate->setVisible(false);
    }

    addParams(row, internalModule, internalModuleBaudRate);
    row++;
  }

  if (Boards::getCapability(board, Board::HasExternalModuleSupport)) {
    addLabel(tr("Sample Mode"), row, 0);
    AutoComboBox *uartSampleMode = new AutoComboBox(this);
    uartSampleMode->setModel(editorItemModels->getItemModel(uartmodelid));
    uartSampleMode->setField(generalSettings.uartSampleMode);
    addParams(row, uartSampleMode);
  }

  // All values except 0 are mutually exclusive
  ExclusiveComboGroup *exclGroup = new ExclusiveComboGroup(
      this, [=](const QVariant &value) { return value == 0; });

  if (firmware->getCapability(HasAuxSerialMode) || firmware->getCapability(HasAux2SerialMode) || firmware->getCapability(HasVCPSerialMode))
    addSection(tr("Serial ports"), row);

  if (firmware->getCapability(HasAuxSerialMode)) {
    QString lbl = "AUX1";
    addLabel(tr("%1").arg(lbl), row, 0);
    AutoComboBox *serialPortMode = new AutoComboBox(this);
    serialPortMode->setModel(editorItemModels->getItemModel(auxmodelid));
    serialPortMode->setField(generalSettings.serialPort[GeneralSettings::SP_AUX1], this);
    exclGroup->addCombo(serialPortMode);

    AutoCheckBox *serialPortPower = new AutoCheckBox(this);
    serialPortPower->setField(generalSettings.serialPower[GeneralSettings::SP_AUX1], this);
    serialPortPower->setText(tr("Power"));

    addParams(row, serialPortMode, serialPortPower);

    if (!firmware->getCapability(HasSoftwareSerialPower))
      serialPortPower->setVisible(false);
  }

  if (firmware->getCapability(HasAux2SerialMode)) {
    QString lbl = "AUX2";
    addLabel(tr("%1").arg(lbl), row, 0);
    AutoComboBox *serialPortMode = new AutoComboBox(this);
    serialPortMode->setModel(editorItemModels->getItemModel(auxmodelid));
    serialPortMode->setField(generalSettings.serialPort[GeneralSettings::SP_AUX2], this);
    exclGroup->addCombo(serialPortMode);

    AutoCheckBox *serialPortPower = new AutoCheckBox(this);
    serialPortPower->setField(generalSettings.serialPower[GeneralSettings::SP_AUX2], this);
    serialPortPower->setText(tr("Power"));

    addParams(row, serialPortMode, serialPortPower);

    if (!firmware->getCapability(HasSoftwareSerialPower))
      serialPortPower->setVisible(false);
  }

  if (firmware->getCapability(HasVCPSerialMode)) {
    addLabel(tr("USB-VCP"), row, 0);
    serialPortUSBVCP = new AutoComboBox(this);
    serialPortUSBVCP->setModel(editorItemModels->getItemModel(vcpmodelid));
    serialPortUSBVCP->setField(generalSettings.serialPort[GeneralSettings::SP_VCP], this);
    exclGroup->addCombo(serialPortUSBVCP);
    addParams(row, serialPortUSBVCP);
    connect(this, &HardwarePanel::internalModuleChanged, this, &HardwarePanel::updateSerialPortUSBVCP);
    updateSerialPortUSBVCP();
  }

  if (firmware->getCapability(HasADCJitterFilter)) {
    addLabel(tr("ADC Filter"), row, 0);
    AutoCheckBox *filterEnable = new AutoCheckBox(this);
    filterEnable->setField(generalSettings.noJitterFilter, this, true);
    addParams(row, filterEnable);
  }

  if (firmware->getCapability(HasSportConnector)) {
    addLabel(tr("S.Port Power"), row, 0);
    AutoCheckBox *sportPower = new AutoCheckBox(this);
    sportPower->setField(generalSettings.sportPower, this);
    addParams(row, sportPower);
  }

  if (firmware->getCapability(HastxCurrentCalibration)) {
    addLabel(tr("Current Offset"), row, 0);
    AutoSpinBox *txCurrentCalibration = new AutoSpinBox(this);
    FieldRange txCCRng = GeneralSettings::getTxCurrentCalibration();
    txCurrentCalibration->setSuffix(txCCRng.unit);
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

void HardwarePanel::on_internalModuleChanged()
{
  if (QMessageBox::warning(this, CPN_STR_APP_NAME,
                       tr("Warning: Changing the Internal module may invalidate the internal module protocol of the models!"),
                       QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok) {

    generalSettings.internalModule = m_internalModule;
    internalModule->updateValue();
  }
  else {
    m_internalModule = generalSettings.internalModule;
    if (m_internalModule == MODULE_TYPE_GHOST || m_internalModule == MODULE_TYPE_CROSSFIRE) {

      if (Boards::getCapability(getCurrentFirmware()->getBoard(),
                                Board::SportMaxBaudRate) < 400000) {
        // default to 115k
        internalModuleBaudRate->setCurrentIndex(0);
      } else {
        // default to 400k
        internalModuleBaudRate->setCurrentIndex(1);
      }

      internalModuleBaudRate->setVisible(true);
    } else {
      generalSettings.internalModuleBaudrate = 0;
      internalModuleBaudRate->setVisible(false);
    }

    emit internalModuleChanged();
  }
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

void HardwarePanel::addSection(QString text, int & row)
{
  addLabel(QString("<b>%1</b>").arg(text), row, 0);
  row++;
}

void HardwarePanel::updateSerialPortUSBVCP()
{
  if (!serialPortUSBVCP)
    return;

  if (m_internalModule == MODULE_TYPE_CROSSFIRE &&
      generalSettings.serialPort[GeneralSettings::SP_VCP] == GeneralSettings::AUX_SERIAL_OFF) {
    generalSettings.serialPort[GeneralSettings::SP_VCP] = GeneralSettings::AUX_SERIAL_CLI;
    serialPortUSBVCP->updateValue();
  }

  auto view = dynamic_cast<QListView*>(serialPortUSBVCP->view());
  Q_ASSERT(view);

  for (int i = 0; i < serialPortUSBVCP->count(); i++) {
    if (m_internalModule == MODULE_TYPE_CROSSFIRE && i == GeneralSettings::AUX_SERIAL_OFF)
      view->setRowHidden(i, true);
    else
      view->setRowHidden(i, false);
  }
}
