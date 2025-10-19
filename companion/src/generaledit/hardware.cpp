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
#include "exclusivecombogroup.h"

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

HardwarePanel::HardwarePanel(QWidget * parent, GeneralSettings & generalSettings,
                             Firmware * firmware, CompoundItemModelFactory * sharedItemModels):
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
    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Invert"));
    addParams();
    for (int i = 0; i < count; i++) {
      addStick(i);
    }
  }

  if (IS_FLYSKY_NV14(board) || IS_FLYSKY_EL18(board) || IS_FAMILY_PL18(board)) {
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
    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Type"));
    addLabel(tr("Invert"));
    addParams();
    for (int i = Boards::getCapability(board, Board::Sticks); i < count; i++) {
      if (Boards::isInputConfigurable(i, board))
        addFlex(i);
    }
  }

  if (Boards::getCapability(board, Board::Switches)) {
    addSection(tr("Switches"));

    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Type"));
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, board) && !generalSettings.isSwitchFunc(i) && !generalSettings.isSwitchFlex(i))
        addSwitch(i);
    }
  }

  if (Boards::getCapability(board, Board::FlexSwitches)) {
    // All values except -1 (None) are mutually exclusive
    exclFlexSwitchesGroup = new ExclusiveComboGroup(
        this, [=](const QVariant &value) { return value == -1; });

    addSection(tr("Flex Switches"));

    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Source"));
    addLabel(tr("Type"));
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, board) && generalSettings.isSwitchFlex(i))
        addSwitch(i);
    }

    // if multiple combo boxes force update to lists
    if (exclFlexSwitchesGroup->getComboBoxes()->count() > 1) {
      QComboBox *cb = exclFlexSwitchesGroup->getComboBoxes()->at(0);
      exclFlexSwitchesGroup->handleActivated(cb, cb->currentIndex());
    }
  }

  if (Boards::getCapability(board, Board::FunctionSwitches)) {
    addSection(tr("Customisable Switches"));

    addLabel("");
    addLabel(tr("Name"));
    addLabel(tr("Type"));
    addLabel(tr("Start"));
    if (Boards::getCapability(board, Board::FunctionSwitchColors)) {
      addLabel(tr("Off color"));
      addLabel(tr("Lua override"));
      addLabel(tr("On color"));
      addLabel(tr("Lua override"));
    }
    addParams();

    for (int i = 0; i < CPN_MAX_SWITCHES; i++) {
      if (Boards::isSwitchConfigurable(i, board) && generalSettings.isSwitchFunc(i))
        addSwitch(i);
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

  connect(this, &HardwarePanel::inputFlexTypeChanged, [=](AutoComboBox *cb, int index)
    { setFlexTypeModel(cb, index); }
  );
  connect(this, &HardwarePanel::refreshItemModels, [=]() { updateItemModels(); });
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

  AutoCheckBox *inverted = new AutoCheckBox(this);
  inverted->setField(config.inverted, this);
  params->append(inverted);

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
  connect(name, &AutoLineEdit::editingFinished, [=] () { emit refreshItemModels(); });

  AutoComboBox *type = new AutoComboBox(this);
  setFlexTypeModel(type, index);
  type->setField(config.flexType, this);

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
          emit inputFlexTypeChanged(type, index);
          emit refreshItemModels();
  });

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

  QLabel *label = new QLabel(this);
  label->setText(Boards::getSwitchName(index));
  params->append(label);

  AutoLineEdit *name = new AutoLineEdit(this);
  name->setValidator(new NameValidator(board, this));
  name->setField(config.name, HARDWARE_NAME_LEN, this);
  params->append(name);
  connect(name, &AutoLineEdit::editingFinished, [=] () { emit refreshItemModels(); });

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

  type->setField(config.type, this);
  params->append(type);

  if (!generalSettings.isSwitchFunc(index)) {
    connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
            emit refreshItemModels();
    });
  }

  if (generalSettings.isSwitchFlex(index)) {
    connect(input, &AutoComboBox::currentDataChanged, [=] (int val) {
            if (val < 0) {
              generalSettings.switchConfig[index].type = Board::SWITCH_NOT_AVAILABLE;
              type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPENONE));
              type->updateValue();
            }
            else
              type->setModel(tabFilteredModels->getItemModel(FIM_SWITCHTYPE3POS));

            emit refreshItemModels();
    });
  }

  if (generalSettings.isSwitchFunc(index)) {
    AutoComboBox *start = new AutoComboBox(this);
    start->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    AbstractStaticItemModel *fsStart = ModelData::funcSwitchStartItemModel();
    start->setModel(fsStart);
    int & swstart = (int &)config.start;
    start->setField(swstart, this);
    start->setEnabled(config.type == Board::SWITCH_2POS);
    params->append(start);

    connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
            start->setEnabled(val == Board::SWITCH_2POS);
            emit refreshItemModels();
    });

    if (Boards::getCapability(board, Board::FunctionSwitchColors)) {
      const QString qss = QString("border-style: outset; border-width: 2px; border-radius: 5px; border-color: darkgrey; padding: 2px; background-color: %1;");
      QPushButton * btnOffColor = new QPushButton();
      QColor off = generalSettings.switchConfig[index].offColor.getQColor();
      btnOffColor->setStyleSheet(QString(qss).arg(off.name()));
      connect(btnOffColor, &QPushButton::clicked, [=]() {
        QColorDialog *dlg = new QColorDialog();
        QColor color = dlg->getColor(generalSettings.switchConfig[index].offColor.getQColor());
        if (color.isValid()) {
          generalSettings.switchConfig[index].offColor.setColor(color.red(), color.green(), color.blue());
          btnOffColor->setStyleSheet(QString(qss).arg(color.name()));
          emit modified();
        }
      });
      params->append(btnOffColor);
      connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
              btnOffColor->setEnabled(val != Board::SWITCH_NOT_AVAILABLE); });

      QCheckBox * cbOffLuaOverride = new QCheckBox(this);
      cbOffLuaOverride->setChecked(generalSettings.switchConfig[index].offColorLuaOverride);
      connect(cbOffLuaOverride, &QCheckBox::toggled, [=](int value) {
        generalSettings.switchConfig[index].offColorLuaOverride = value;
        emit modified();
      });
      params->append(cbOffLuaOverride);
      connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
              cbOffLuaOverride->setEnabled(val != Board::SWITCH_NOT_AVAILABLE); });

      QPushButton * btnOnColor = new QPushButton(tr(""));
      QColor on = generalSettings.switchConfig[index].onColor.getQColor();
      btnOnColor->setStyleSheet(QString(qss).arg(on.name()));
      connect(btnOnColor, &QPushButton::clicked, [=]() {
        QColorDialog *dlg = new QColorDialog();
        QColor color = dlg->getColor(generalSettings.switchConfig[index].onColor.getQColor());
        if (color.isValid()) {
          generalSettings.switchConfig[index].onColor.setColor(color.red(), color.green(), color.blue());
          btnOnColor->setStyleSheet(QString(qss).arg(color.name()));
          emit modified();
        }
      });
      params->append(btnOnColor);
      connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
              btnOnColor->setEnabled(val != Board::SWITCH_NOT_AVAILABLE); });

      QCheckBox * cbOnLuaOverride = new QCheckBox(this);
      cbOnLuaOverride->setChecked(generalSettings.switchConfig[index].onColorLuaOverride);
      connect(cbOnLuaOverride, &QCheckBox::toggled, [=](int value) {
        generalSettings.switchConfig[index].onColorLuaOverride = value;
        emit modified();
      });
      params->append(cbOnLuaOverride);
      connect(type, &AutoComboBox::currentDataChanged, [=] (int val) {
              cbOnLuaOverride->setEnabled(val != Board::SWITCH_NOT_AVAILABLE); });
    }
  }

  addParams();
}

void HardwarePanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  params->append(label);
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
  for (int i = 0; i < params->size(); i++) {
    grid->addWidget(params->at(i), row, i);
  }

  row += 1;
  params->clear();
}

void HardwarePanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  addParams();
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

void HardwarePanel::updateItemModels()
{
  editorItemModels->update(AbstractItemModel::IMUE_Hardware);
}
