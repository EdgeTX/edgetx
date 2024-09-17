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

#include "hardware.h"
#include "compounditemmodels.h"
#include "filtereditemmodels.h"
#include "autolineedit.h"
#include "autocombobox.h"
#include "autocheckbox.h"
#include "autospinbox.h"
#include "autodoublespinbox.h"
#include "autobitmappedcombobox.h"
#include "autobitmappedcheckbox.h"
#include "namevalidator.h"

#include <QLabel>
#include <QGridLayout>
#include <QFrame>

constexpr char FIM_SWITCHTYPENONE[]    {"Switch Type None"};
constexpr char FIM_SWITCHTYPE2POS[]    {"Switch Type 2 Pos"};
constexpr char FIM_SWITCHTYPE3POS[]    {"Switch Type 3 Pos"};
constexpr char FIM_INTERNALMODULES[]   {"Internal Modules"};
constexpr char FIM_AUX1SERIALMODES[]   {"AUX1 Modes"};
constexpr char FIM_AUX2SERIALMODES[]   {"AUX2 Modes"};
constexpr char FIM_VCPSERIALMODES[]    {"VCP Modes"};
constexpr char FIM_FLEXSWITCHES[]      {"Flex Switches"};
constexpr char FIM_FLEXTYPE_SWITCH[]   {"Flex Type Switch"};
constexpr char FIM_FLEXTYPE_NOSWITCH[] {"Flex Type No Switch"};

class ExclusiveComboGroup: public QObject
{
  public:
    ExclusiveComboGroup(QObject *parent, std::function<bool(const QVariant&)> filter) :
      QObject(parent), filter(std::move(filter))
    {
    }

    typedef QList<QComboBox*> ComboBoxes;

    ComboBoxes* getComboBoxes()
    {
     return &combos;
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

  private:
    static constexpr auto _role = Qt::UserRole + 500;

    ComboBoxes combos;
    std::function<bool(const QVariant&)> filter;

};

HardwarePanel::HardwarePanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  editorItemModels(sharedItemModels),
  serialPortUSBVCP(nullptr),
  params(new QList<QWidget *>),
  row(0),
  exclFlexSwitchesGroup(nullptr)
{
  tabFilteredModels = new FilteredItemModelFactory();

  int id = editorItemModels->registerItemModel(Boards::flexTypeItemModel());
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::FlexTypeContextSwitch), FIM_FLEXTYPE_SWITCH);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::FlexTypeContextNoSwitch), FIM_FLEXTYPE_NOSWITCH);

  id = editorItemModels->registerItemModel(Boards::switchTypeItemModel());
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContextNone), FIM_SWITCHTYPENONE);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext2Pos), FIM_SWITCHTYPE2POS);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), Board::SwitchTypeContext3Pos), FIM_SWITCHTYPE3POS);

  int antmodelid = editorItemModels->registerItemModel(GeneralSettings::antennaModeItemModel());
  int btmodelid = editorItemModels->registerItemModel(GeneralSettings::bluetoothModeItemModel());
  id = editorItemModels->registerItemModel(GeneralSettings::serialModeItemModel());
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), GeneralSettings::AUX1Context), FIM_AUX1SERIALMODES);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), GeneralSettings::AUX2Context), FIM_AUX2SERIALMODES);
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id), GeneralSettings::VCPContext), FIM_VCPSERIALMODES);
  int baudmodelid = editorItemModels->registerItemModel(GeneralSettings::internalModuleBaudrateItemModel());
  int uartmodelid = editorItemModels->registerItemModel(GeneralSettings::uartSampleModeItemModel());

  id = editorItemModels->registerItemModel(ModuleData::internalModuleItemModel());
  tabFilteredModels->registerItemModel(new FilteredItemModel(editorItemModels->getItemModel(id)), FIM_INTERNALMODULES);

  editorItemModels->addItemModel(AbstractItemModel::IMID_FlexSwitches);

  grid = new QGridLayout(this);
  int count;

  addSection(tr("Axis"));

  count = Boards::getCapability(board, Board::Sticks);
  if (count > 0) {
    for (int i = 0; i < count; i++) {
      addStick(i);
    }
  }

  if (IS_FLYSKY_NV14(board)) {
    addLabel(tr("Dead zone"));
    AutoComboBox *spnStickDeadZone = new AutoComboBox(this);
    spnStickDeadZone->setModel(GeneralSettings::stickDeadZoneItemModel());
    spnStickDeadZone->setField(generalSettings.stickDeadZone, this);
    params->append(spnStickDeadZone);
    addParams();
  }

  count = Boards::getCapability(board, Board::Inputs);

  if (count > 0) {
    addSection(tr("Pots"));
    for (int i = Boards::getCapability(board, Board::Sticks); i < count; i++) {
      if (Boards::isInputConfigurable(i, board))
        addFlex(i);
    }
  }

  count = Boards::getCapability(board, Board::Switches);

  if (count) {
    // All values except -1 (None) are mutually exclusive
    exclFlexSwitchesGroup = new ExclusiveComboGroup(
        this, [=](const QVariant &value) { return value == -1; });

    addSection(tr("Switches"));
    for (int i = 0; i < count && i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, board))
        addSwitch(i);
    }

    // if multiple combo boxes force update to lists
    if (exclFlexSwitchesGroup->getComboBoxes()->count() > 1) {
      QComboBox *cb = exclFlexSwitchesGroup->getComboBoxes()->at(0);
      exclFlexSwitchesGroup->handleActivated(cb, cb->currentIndex());
    }
  }

  addLine();

  if (Boards::getCapability(board, Board::HasRTC)) {
    addLabel(tr("RTC Battery Check"));
    AutoCheckBox *rtcCheckDisable = new AutoCheckBox(this);
    rtcCheckDisable->setField(generalSettings.rtcCheckDisable, this, true);
    params->append(rtcCheckDisable);
    addParams();
  }

  if (firmware->getCapability(HasADCJitterFilter)) {
    addLabel(tr("ADC Filter"));
    AutoCheckBox *filterEnable = new AutoCheckBox(this);
    filterEnable->setField(generalSettings.noJitterFilter, this, true);
    params->append(filterEnable);
    addParams();
  }

  if (Boards::getCapability(board, Board::HasAudioMuteGPIO)) {
    addLabel(tr("Mute if no sound"));
    AutoCheckBox *muteIfNoSound = new AutoCheckBox(this);
    muteIfNoSound->setField(generalSettings.muteIfNoSound, this, false);
    params->append(muteIfNoSound);
    addParams();
  }

  if (firmware->getCapability(HasBluetooth)) {
    addLabel(tr("Bluetooth"));

    AutoComboBox *bluetoothMode = new AutoComboBox(this);
    bluetoothMode->setValidator(new NameValidator(board, this));
    bluetoothMode->setModel(editorItemModels->getItemModel(btmodelid));
    bluetoothMode->setField(generalSettings.bluetoothMode, this);
    params->append(bluetoothMode);

    QLabel *btnamelabel = new QLabel(this);
    btnamelabel->setText(tr("Device Name:"));
    params->append(btnamelabel);

    AutoLineEdit *bluetoothName = new AutoLineEdit(this);
    bluetoothName->setField(generalSettings.bluetoothName, BLUETOOTH_NAME_LEN, this);
    params->append(bluetoothName);

    addParams();
  }

  if (Boards::getCapability(board, Board::HasInternalModuleSupport)) {
    m_internalModule = generalSettings.internalModule; // to permit undo
    addSection(tr("Internal RF"));
    addLabel(tr("Type"));
    internalModule = new AutoComboBox(this);
    internalModule->setModel(tabFilteredModels->getItemModel(FIM_INTERNALMODULES));
    internalModule->setField(generalSettings.internalModule, this);
    params->append(internalModule);

    connect(internalModule, &AutoComboBox::currentDataChanged, this,
            &HardwarePanel::on_internalModuleChanged);

    internalModuleBaudRateLabel = new QLabel(tr("Baudrate:"));
    params->append(internalModuleBaudRateLabel);

    internalModuleBaudRate = new AutoComboBox(this);
    internalModuleBaudRate->setModel(editorItemModels->getItemModel(baudmodelid));
    internalModuleBaudRate->setField(generalSettings.internalModuleBaudrate, this);
    params->append(internalModuleBaudRate);

    if (m_internalModule != MODULE_TYPE_GHOST && m_internalModule != MODULE_TYPE_CROSSFIRE) {
      generalSettings.internalModuleBaudrate = 0;
      internalModuleBaudRateLabel->setVisible(false);
      internalModuleBaudRate->setVisible(false);
    }

    antennaLabel = new QLabel(tr("Antenna:"));
    params->append(antennaLabel);

    antennaMode = new AutoComboBox(this);
    antennaMode->setModel(editorItemModels->getItemModel(antmodelid));
    antennaMode->setField(generalSettings.antennaMode, this);
    params->append(antennaMode);

    if (!(m_internalModule == MODULE_TYPE_XJT_PXX1 && HAS_EXTERNAL_ANTENNA(board))) {
      antennaLabel->setVisible(false);
      antennaMode->setVisible(false);
    }

    addParams();
  }

  if (Boards::getCapability(board, Board::HasExternalModuleSupport)) {
    addSection(tr("External RF"));
    addLabel(tr("Sample Mode"));
    AutoComboBox *uartSampleMode = new AutoComboBox(this);
    uartSampleMode->setModel(editorItemModels->getItemModel(uartmodelid));
    uartSampleMode->setField(generalSettings.uartSampleMode);
    params->append(uartSampleMode);
    addParams();
  }

  // All values except 0 are mutually exclusive
  ExclusiveComboGroup *exclGroup = new ExclusiveComboGroup(
      this, [=](const QVariant &value) { return value == 0; });

  if (firmware->getCapability(HasAuxSerialMode) || firmware->getCapability(HasAux2SerialMode) || firmware->getCapability(HasVCPSerialMode))
    addSection(tr("Serial ports"));

  if (firmware->getCapability(HasAuxSerialMode)) {
    addLabel(tr("AUX1"));
    AutoComboBox *serialPortMode = new AutoComboBox(this);
    serialPortMode->setModel(tabFilteredModels->getItemModel(FIM_AUX1SERIALMODES));
    serialPortMode->setField(generalSettings.serialPort[GeneralSettings::SP_AUX1], this);
    exclGroup->addCombo(serialPortMode);
    params->append(serialPortMode);

    AutoCheckBox *serialPortPower = new AutoCheckBox(this);
    serialPortPower->setField(generalSettings.serialPower[GeneralSettings::SP_AUX1], this);
    serialPortPower->setText(tr("Power"));
    params->append(serialPortPower);

    addParams();

    if (!firmware->getCapability(HasSoftwareSerialPower))
      serialPortPower->setVisible(false);
  }

  if (firmware->getCapability(HasAux2SerialMode)) {
    addLabel(tr("AUX2"));
    AutoComboBox *serialPortMode = new AutoComboBox(this);
    serialPortMode->setModel(tabFilteredModels->getItemModel(FIM_AUX2SERIALMODES));
    serialPortMode->setField(generalSettings.serialPort[GeneralSettings::SP_AUX2], this);
    exclGroup->addCombo(serialPortMode);
    params->append(serialPortMode);

    AutoCheckBox *serialPortPower = new AutoCheckBox(this);
    serialPortPower->setField(generalSettings.serialPower[GeneralSettings::SP_AUX2], this);
    serialPortPower->setText(tr("Power"));
    params->append(serialPortPower);

    addParams();

    if (!firmware->getCapability(HasSoftwareSerialPower))
      serialPortPower->setVisible(false);
  }

  if (firmware->getCapability(HasVCPSerialMode)) {
    addLabel(tr("USB-VCP"));
    serialPortUSBVCP = new AutoComboBox(this);
    serialPortUSBVCP->setModel(tabFilteredModels->getItemModel(FIM_VCPSERIALMODES));
    serialPortUSBVCP->setField(generalSettings.serialPort[GeneralSettings::SP_VCP], this);
    exclGroup->addCombo(serialPortUSBVCP);
    params->append(serialPortUSBVCP);
    addParams();
    connect(this, &HardwarePanel::internalModuleChanged, this, &HardwarePanel::updateSerialPortUSBVCP);
    updateSerialPortUSBVCP();
  }

  if (firmware->getCapability(HasSportConnector)) {
    addLabel(tr("S.Port Power"));
    AutoCheckBox *sportPower = new AutoCheckBox(this);
    sportPower->setField(generalSettings.sportPower, this);
    params->append(sportPower);
    addParams();
  }

  if (firmware->getCapability(HastxCurrentCalibration)) {
    addLabel(tr("Current Offset"));
    AutoSpinBox *txCurrentCalibration = new AutoSpinBox(this);
    FieldRange txCCRng = GeneralSettings::getTxCurrentCalibration();
    txCurrentCalibration->setSuffix(txCCRng.unit);
    txCurrentCalibration->setField(generalSettings.txCurrentCalibration);
    params->append(txCurrentCalibration);
    addParams();
  }

  if (Boards::getCapability(board, Board::LcdWidth) == 128) {
    addSection(tr("Screen"));

    addLabel(tr("Invert"));
    AutoCheckBox *screenInvert = new AutoCheckBox(this);
    screenInvert->setField(generalSettings.invertLCD, this);
    params->append(screenInvert);
    addParams();
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

      internalModuleBaudRateLabel->setVisible(true);
      internalModuleBaudRate->setVisible(true);
    } else {
      generalSettings.internalModuleBaudrate = 0;
      internalModuleBaudRateLabel->setVisible(false);
      internalModuleBaudRate->setVisible(false);
    }

    if (m_internalModule == MODULE_TYPE_XJT_PXX1 && HAS_EXTERNAL_ANTENNA(board)) {
        antennaLabel->setVisible(true);
        antennaMode->setVisible(true);
    }
    else {
      antennaLabel->setVisible(false);
      antennaMode->setVisible(false);
    }

    emit internalModuleChanged();
  }
}

void HardwarePanel::addStick(int index)
{
  GeneralSettings::InputConfig &config = generalSettings.inputConfig[index];

  addLabel(Boards::getInputName(index, board));

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setValidator(new NameValidator(board, this));
  name->setField(config.name, HARDWARE_NAME_LEN, this);
  params->append(name);
  addParams();
}

void HardwarePanel::addFlex(int index)
{
  GeneralSettings::InputConfig &config = generalSettings.inputConfig[index];

  addLabel(Boards::getInputName(index, board));

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setValidator(new NameValidator(board, this));
  name->setField(config.name, HARDWARE_NAME_LEN, this);
  params->append(name);

  AutoComboBox *type = new AutoComboBox(this);
  setFlexTypeModel(type, index);
  int & flexType = (int &)config.flexType;
  type->setField(flexType, this);

  connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
          AbstractItemModel *mdl = editorItemModels->getItemModel(AbstractItemModel::IMID_FlexSwitches);
          if (mdl)
            mdl->update(AbstractItemModel::IMUE_FunctionSwitches);
          if (generalSettings.isInputMultiPosPot(index)) {
            invertToggles[index - Boards::getCapability(board, Board::Sticks)]->hide();
            if (generalSettings.inputConfig[index].inverted) {
              generalSettings.inputConfig[index].inverted = false;
              invertToggles[index - Boards::getCapability(board, Board::Sticks)]->updateValue();
              emit modified();
            }
          } else {
            invertToggles[index - Boards::getCapability(board, Board::Sticks)]->show();
          }
          emit InputFlexTypeChanged();
  });

  connect(this, &HardwarePanel::InputFlexTypeChanged, [=]() { setFlexTypeModel(type, index); });

  params->append(type);

  AutoCheckBox *inverted = new AutoCheckBox(this);
  inverted->setField(config.inverted, this);
  params->append(inverted);
  if (generalSettings.isInputMultiPosPot(index)) {
    inverted->hide();
    if (config.inverted) {
      config.inverted = false;
      inverted->updateValue();
      emit modified();
    }
  }
  invertToggles.push_back(inverted);

  addParams();
}

void HardwarePanel::setFlexTypeModel(AutoComboBox * cb, int index)
{
  cb->setModel((generalSettings.inputConfig[index].flexType == Board::FLEX_SWITCH || generalSettings.unassignedInputFlexSwitches()) ?
                 tabFilteredModels->getItemModel(FIM_FLEXTYPE_SWITCH) :
                 tabFilteredModels->getItemModel(FIM_FLEXTYPE_NOSWITCH));
}

void HardwarePanel::addSwitch(int index)
{
  GeneralSettings::SwitchConfig &config = generalSettings.switchConfig[index];
  Board::SwitchInfo info = Boards::getSwitchInfo(index);

  addLabel(Boards::getSwitchName(index));

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setValidator(new NameValidator(board, this));
  name->setField(config.name, HARDWARE_NAME_LEN, this);
  params->append(name);

  AutoComboBox *input = nullptr;

  if (generalSettings.isSwitchFlex(index)) {
    int id = tabFilteredModels->registerItemModel(
               new FilteredItemModel(editorItemModels->getItemModel(AbstractItemModel::IMID_FlexSwitches)),
                                                  QString("%1 %2").arg(FIM_FLEXSWITCHES).arg(index));
    input = new AutoComboBox(this);
    input->setModel(tabFilteredModels->getItemModel(id));
    input->setField(config.inputIdx, this);
    exclFlexSwitchesGroup->addCombo(input);
    params->append(input);
  }

  AutoComboBox *type = new AutoComboBox(this);
  type->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  if (generalSettings.isSwitchFlex(index))
    if (config.type == Board::SWITCH_NOT_AVAILABLE)
      type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPENONE));
    else
      type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPE3POS));
  else if (info.type < Board::SWITCH_3POS)
    type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPE2POS));
  else
    type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPE3POS));

  int & swtype = (int &)config.type;
  type->setField(swtype, this);
  params->append(type);

  if (generalSettings.isSwitchFlex(index)) {
    connect(input, &AutoComboBox::currentDataChanged, [=] (int val) {
            if (val < 0) {
              generalSettings.switchConfig[index].type = Board::SWITCH_NOT_AVAILABLE;
              type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPENONE));
              type->updateValue();
            }
            else
              type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPE3POS));
    });
  }

  addParams();
}

void HardwarePanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, 0);
}

void HardwarePanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void HardwarePanel::addParams()
{
  int col = 0;
  QGridLayout *subgrid = new QGridLayout();

  for (int i = 0; i < params->size(); i++) {
    subgrid->addWidget(params->at(i), 0, col++);
  }

  addHSpring(subgrid, col, 0);
  grid->addLayout(subgrid, row++, 1);
  params->clear();
}

void HardwarePanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
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
