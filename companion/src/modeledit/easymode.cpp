/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "easymode.h"
#include "easymode_data.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

EasyModePanel::EasyModePanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware)
{
  buildUI();
  update();
}

EasyModePanel::~EasyModePanel()
{
}

static QSpinBox* createChannelSpin(QWidget* parent)
{
  auto spin = new QSpinBox(parent);
  spin->setRange(0, 16);
  spin->setSpecialValueText("---");
  return spin;
}

static QSpinBox* createPercentSpin(QWidget* parent, int min = 0, int max = 100)
{
  auto spin = new QSpinBox(parent);
  spin->setRange(min, max);
  spin->setSuffix("%");
  return spin;
}

void EasyModePanel::buildUI()
{
  auto mainLayout = new QVBoxLayout(this);

  // Model Type
  auto typeLayout = new QFormLayout();
  modelTypeCombo = new QComboBox(this);
  modelTypeCombo->addItem(tr("---"), EasyModeData::EASYMODE_NONE);
  modelTypeCombo->addItem(tr("Airplane"), EasyModeData::EASYMODE_AIRPLANE);
  modelTypeCombo->addItem(tr("Helicopter"), EasyModeData::EASYMODE_HELICOPTER);
  modelTypeCombo->addItem(tr("Glider"), EasyModeData::EASYMODE_GLIDER);
  modelTypeCombo->addItem(tr("Multirotor"), EasyModeData::EASYMODE_MULTIROTOR);
  modelTypeCombo->addItem(tr("Car"), EasyModeData::EASYMODE_CAR);
  modelTypeCombo->addItem(tr("Boat"), EasyModeData::EASYMODE_BOAT);
  typeLayout->addRow(tr("Model Type:"), modelTypeCombo);
  mainLayout->addLayout(typeLayout);
  connect(modelTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EasyModePanel::edited);

  // Wing Type group
  wingGroup = new QGroupBox(tr("Wing"), this);
  auto wingLayout = new QFormLayout(wingGroup);
  wingTypeCombo = new QComboBox(this);
  wingTypeCombo->addItem(tr("Single Aileron"));
  wingTypeCombo->addItem(tr("Dual Aileron"));
  wingTypeCombo->addItem(tr("Flaperon"));
  wingTypeCombo->addItem(tr("1 Ail + 1 Flap"));
  wingTypeCombo->addItem(tr("2 Ail + 1 Flap"));
  wingTypeCombo->addItem(tr("2 Ail + 2 Flap"));
  wingTypeCombo->addItem(tr("2 Ail + 4 Flap"));
  wingTypeCombo->addItem(tr("Elevon"));
  wingTypeCombo->addItem(tr("Delta"));
  wingLayout->addRow(tr("Wing Type:"), wingTypeCombo);
  mainLayout->addWidget(wingGroup);
  connect(wingTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EasyModePanel::edited);

  // Tail Type group
  tailGroup = new QGroupBox(tr("Tail"), this);
  auto tailLayout = new QFormLayout(tailGroup);
  tailTypeCombo = new QComboBox(this);
  tailTypeCombo->addItem(tr("Normal"));
  tailTypeCombo->addItem(tr("V-Tail"));
  tailTypeCombo->addItem(tr("Tailless"));
  tailTypeCombo->addItem(tr("Dual Elevator"));
  tailTypeCombo->addItem(tr("Ailevator"));
  tailLayout->addRow(tr("Tail Type:"), tailTypeCombo);
  mainLayout->addWidget(tailGroup);
  connect(tailTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EasyModePanel::edited);

  // Motor Type group
  motorGroup = new QGroupBox(tr("Motor"), this);
  auto motorLayout = new QFormLayout(motorGroup);
  motorTypeCombo = new QComboBox(this);
  motorTypeCombo->addItem(tr("None"));
  motorTypeCombo->addItem(tr("Electric"));
  motorTypeCombo->addItem(tr("Nitro/Gas"));
  motorLayout->addRow(tr("Motor Type:"), motorTypeCombo);
  mainLayout->addWidget(motorGroup);
  connect(motorTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EasyModePanel::edited);

  // Channel Map group
  channelGroup = new QGroupBox(tr("Channel Map"), this);
  auto chLayout = new QFormLayout(channelGroup);

  chAileron = createChannelSpin(this);
  chLayout->addRow(tr("Aileron:"), chAileron);
  chAileron2 = createChannelSpin(this);
  chLayout->addRow(tr("Aileron 2:"), chAileron2);
  chElevator = createChannelSpin(this);
  chLayout->addRow(tr("Elevator:"), chElevator);
  chThrottle = createChannelSpin(this);
  chLayout->addRow(tr("Throttle:"), chThrottle);
  chRudder = createChannelSpin(this);
  chLayout->addRow(tr("Rudder:"), chRudder);
  chSteering = createChannelSpin(this);
  chLayout->addRow(tr("Steering:"), chSteering);
  chFlap = createChannelSpin(this);
  chLayout->addRow(tr("Flap:"), chFlap);
  chFlap2 = createChannelSpin(this);
  chLayout->addRow(tr("Flap 2:"), chFlap2);
  chFlap3 = createChannelSpin(this);
  chLayout->addRow(tr("Brake L:"), chFlap3);
  chFlap4 = createChannelSpin(this);
  chLayout->addRow(tr("Brake R:"), chFlap4);
  chAux1 = createChannelSpin(this);
  chLayout->addRow(tr("Aux 1:"), chAux1);
  chAux2 = createChannelSpin(this);
  chLayout->addRow(tr("Aux 2:"), chAux2);

  mainLayout->addWidget(channelGroup);

  for (auto spin : {chAileron, chAileron2, chElevator, chThrottle, chRudder,
                     chSteering, chFlap, chFlap2, chFlap3, chFlap4, chAux1, chAux2}) {
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EasyModePanel::edited);
  }

  // Options group
  optionsGroup = new QGroupBox(tr("Options"), this);
  auto optLayout = new QFormLayout(optionsGroup);

  expoAil = createPercentSpin(this);
  optLayout->addRow(tr("Aileron Expo:"), expoAil);
  expoEle = createPercentSpin(this);
  optLayout->addRow(tr("Elevator Expo:"), expoEle);
  expoRud = createPercentSpin(this);
  optLayout->addRow(tr("Rudder Expo:"), expoRud);
  dualRateLow = createPercentSpin(this);
  optLayout->addRow(tr("Dual Rate Low:"), dualRateLow);
  ailDiff = createPercentSpin(this);
  optLayout->addRow(tr("Ail. Differential:"), ailDiff);
  ail2Rud = createPercentSpin(this);
  optLayout->addRow(tr("Ail. to Rudder:"), ail2Rud);

  chOrderCombo = new QComboBox(this);
  chOrderCombo->addItem("AETR");
  chOrderCombo->addItem("TAER");
  chOrderCombo->addItem("RETA");
  optLayout->addRow(tr("Channel Order:"), chOrderCombo);

  mainLayout->addWidget(optionsGroup);

  for (auto spin : {expoAil, expoEle, expoRud, dualRateLow, ailDiff, ail2Rud}) {
    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, &EasyModePanel::edited);
  }
  connect(chOrderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EasyModePanel::edited);

  // Convert to Expert button
  convertBtn = new QPushButton(tr("Convert to Expert Mode"), this);
  mainLayout->addWidget(convertBtn);
  connect(convertBtn, &QPushButton::clicked, this, &EasyModePanel::onConvertToExpert);

  mainLayout->addStretch();

  disableMouseScrolling();
}

void EasyModePanel::update()
{
  lock = true;

  auto& em = model->easyModeData;
  modelTypeCombo->setCurrentIndex(em.modelType);
  wingTypeCombo->setCurrentIndex(em.wingType);
  tailTypeCombo->setCurrentIndex(em.tailType);
  motorTypeCombo->setCurrentIndex(em.motorType);

  // Channels: internal -1 = none, display as 0 (special text "---"), 0-based -> 1-based display
  auto setChSpin = [](QSpinBox* spin, int val) { spin->setValue(val + 1); };
  setChSpin(chAileron, em.aileron);
  setChSpin(chAileron2, em.aileron2);
  setChSpin(chElevator, em.elevator);
  setChSpin(chThrottle, em.throttle);
  setChSpin(chRudder, em.rudder);
  setChSpin(chSteering, em.steering);
  setChSpin(chFlap, em.flap);
  setChSpin(chFlap2, em.flap2);
  setChSpin(chFlap3, em.flap3);
  setChSpin(chFlap4, em.flap4);
  setChSpin(chAux1, em.aux1);
  setChSpin(chAux2, em.aux2);

  expoAil->setValue(em.expoAileron);
  expoEle->setValue(em.expoElevator);
  expoRud->setValue(em.expoRudder);
  dualRateLow->setValue(em.dualRateLow);
  ailDiff->setValue(em.aileronDifferential);
  ail2Rud->setValue(em.aileronToRudderMix);
  chOrderCombo->setCurrentIndex(em.multiChannelOrder);

  updateVisibility();

  lock = false;
}

void EasyModePanel::updateVisibility()
{
  unsigned int type = model->easyModeData.modelType;
  bool isAir = (type == EasyModeData::EASYMODE_AIRPLANE || type == EasyModeData::EASYMODE_GLIDER ||
                type == EasyModeData::EASYMODE_HELICOPTER || type == EasyModeData::EASYMODE_MULTIROTOR);
  bool isSurface = (type == EasyModeData::EASYMODE_CAR || type == EasyModeData::EASYMODE_BOAT);
  bool hasWing = (type == EasyModeData::EASYMODE_AIRPLANE || type == EasyModeData::EASYMODE_GLIDER);
  bool hasTail = (type == EasyModeData::EASYMODE_AIRPLANE || type == EasyModeData::EASYMODE_GLIDER ||
                  type == EasyModeData::EASYMODE_HELICOPTER);
  bool active = (type != EasyModeData::EASYMODE_NONE);

  wingGroup->setVisible(hasWing);
  tailGroup->setVisible(hasTail);
  motorGroup->setVisible(isAir);
  channelGroup->setVisible(active);
  optionsGroup->setVisible(active);
  convertBtn->setVisible(active);

  chOrderCombo->parentWidget()->setVisible(type == EasyModeData::EASYMODE_MULTIROTOR);
  chSteering->parentWidget()->setVisible(isSurface);
}

void EasyModePanel::edited()
{
  if (!lock) {
    auto& em = model->easyModeData;
    em.modelType = modelTypeCombo->currentIndex();
    em.wingType = wingTypeCombo->currentIndex();
    em.tailType = tailTypeCombo->currentIndex();
    em.motorType = motorTypeCombo->currentIndex();

    auto getChSpin = [](QSpinBox* spin) { return spin->value() - 1; };
    em.aileron = getChSpin(chAileron);
    em.aileron2 = getChSpin(chAileron2);
    em.elevator = getChSpin(chElevator);
    em.throttle = getChSpin(chThrottle);
    em.rudder = getChSpin(chRudder);
    em.steering = getChSpin(chSteering);
    em.flap = getChSpin(chFlap);
    em.flap2 = getChSpin(chFlap2);
    em.flap3 = getChSpin(chFlap3);
    em.flap4 = getChSpin(chFlap4);
    em.aux1 = getChSpin(chAux1);
    em.aux2 = getChSpin(chAux2);

    em.expoAileron = expoAil->value();
    em.expoElevator = expoEle->value();
    em.expoRudder = expoRud->value();
    em.dualRateLow = dualRateLow->value();
    em.aileronDifferential = ailDiff->value();
    em.aileronToRudderMix = ail2Rud->value();
    em.multiChannelOrder = chOrderCombo->currentIndex();

    updateVisibility();
    emit modified();
  }
}

void EasyModePanel::onConvertToExpert()
{
  int ret = QMessageBox::question(this, tr("Convert to Expert Mode"),
    tr("This will convert the model to expert mode. The Easy Mode settings will be removed "
       "and the generated mixes will become manual. Continue?"),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (ret == QMessageBox::Yes) {
    model->easyModeData.clear();
    update();
    emit modified();
  }
}
