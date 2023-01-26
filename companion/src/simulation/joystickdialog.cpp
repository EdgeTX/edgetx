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

#include "joystickdialog.h"
#include "ui_joystickdialog.h"
#include "eeprominterface.h"

#include "boards.h"
#include "constants.h"

joystickDialog::joystickDialog(QWidget *parent, int stick) :
  QDialog(parent),
  ui(new Ui::joystickDialog),
  step(0),
  numAxes(0),
  started(false)
{
  ui->setupUi(this);

  int jscaltmp[8][3] = {
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767},
    {32767, 0, -32767}
  };
  memcpy(jscal, jscaltmp, sizeof (jscal));

  foreach(QComboBox *cb, findChildren<QComboBox *>(QRegularExpression("jsmapCB_[0-9]+"))) {
    populateSourceCombo(cb);
    sticks[cb->property("channel").toUInt()] = cb;
  }
  foreach(QComboBox *cb, findChildren<QComboBox *>(QRegularExpression("jsbtnmapCB_[0-9]+"))) {
    populateButtonCombo(cb);
    sticks[cb->property("channel").toUInt()] = cb;
  }
  foreach(QCheckBox *ib, findChildren<QCheckBox *>(QRegularExpression("ChInv_[0-9]+"))) {
    invert[ib->property("channel").toUInt()] = ib;
  }
  foreach(QSlider *sl, findChildren<QSlider *>(QRegularExpression("Ch_[0-9]+"))) {
    sliders[sl->property("channel").toUInt()] = sl;
  }

  for (int i = 0; i < MAX_JOYSTICKS; ++i) {
    if (!g.joystick[i].existsOnDisk())
      continue;

    jscal[i][0] = g.joystick[i].stick_min();
    jscal[i][1] = g.joystick[i].stick_med();
    jscal[i][2] = g.joystick[i].stick_max();
    sliders[i]->setMinimum(jscal[i][0]);
    sliders[i]->setMaximum(jscal[i][2]);
    invert[i]->setChecked(g.joystick[i].stick_inv());
    sticks[i]->setCurrentIndex(sticks[i]->findData(g.joystick[i].stick_axe()));
  }

  for (int i = 0; i < MAX_JSBUTTONS; ++i) {
    if (!g.jsButton[i].existsOnDisk())
      continue;

    invert[i+MAX_JOYSTICKS]->setChecked(g.jsButton[i].button_inv());
    sticks[i+MAX_JOYSTICKS]->setCurrentIndex(sticks[i+MAX_JOYSTICKS]->findData(g.jsButton[i].button_idx()));
  }

  ui->backButton->setEnabled(false);

  ui->joystickChkB->setChecked(g.jsSupport() || stick > -1);

  if (stick < 0)
    stick = g.jsCtrl();

  if (loadJoysticks(stick)) {
    joystickSetEnabled(ui->joystickChkB->isChecked());
    joystickOpen(ui->joystickCB->currentIndex());
  }
  else {
    joystickSetEnabled(false);
  }
  loadStep();

  connect(joystick, SIGNAL(axisValueChanged(int, int)), this, SLOT(onjoystickAxisValueChanged(int, int)));
  connect(joystick, SIGNAL(buttonValueChanged(int, bool)), this, SLOT(onjoystickButtonValueChanged(int, bool)));
  connect(ui->joystickCB, SIGNAL(currentIndexChanged(int)), this, SLOT(joystickOpen(int)));
  connect(ui->joystickChkB, SIGNAL(toggled(bool)), this, SLOT(joystickSetEnabled(bool)));
}

joystickDialog::~joystickDialog()
{
  delete ui;
}

void joystickDialog::populateSourceCombo(QComboBox * cb)
{
  int i;
  QString wname;

  Board::Type m_board = getCurrentBoard();
  GeneralSettings radioSettings = GeneralSettings();

  int ttlSticks = Boards::getCapability(m_board, Board::Sticks);
  int ttlKnobs = Boards::getCapability(m_board, Board::Pots);
  int ttlFaders = Boards::getCapability(m_board, Board::Sliders);

  cb->clear();
  cb->addItem(tr("Not Assigned"), -1);

  for (i=0; i < ttlSticks; ++i) {
    wname = RawSource(RawSourceType::SOURCE_TYPE_STICK, i).toString(nullptr, &radioSettings);
    cb->addItem(wname, i);
  }

  for (i = 0; i < ttlKnobs; ++i) {
    if (radioSettings.isPotAvailable(i)) {
      wname = RawSource(RawSourceType::SOURCE_TYPE_STICK, ttlSticks + i).toString(nullptr, &radioSettings);
      cb->addItem(wname, i + ttlSticks);
    }
  }

  for (i = 0; i < ttlFaders; ++i) {
    if (radioSettings.isSliderAvailable(i)) {
      wname = RawSource(RawSourceType::SOURCE_TYPE_STICK, ttlSticks + ttlKnobs + i).toString(nullptr, &radioSettings);
      cb->addItem(wname, i + ttlSticks + ttlKnobs);
    }
  }
}

void joystickDialog::populateButtonCombo(QComboBox * cb)
{
  int i;
  QString wname;

  Board::Type m_board = getCurrentBoard();
  GeneralSettings radioSettings = GeneralSettings();

  int ttlSwitches = Boards::getCapability(m_board, Board::Switches);

  cb->clear();
  cb->addItem(tr("Not Assigned"), -1);

  Board::SwitchType swcfg;
  for (i = 0; i < ttlSwitches; ++i) {
    if (radioSettings.switchConfig[i] != Board::SWITCH_NOT_AVAILABLE) {
      swcfg = Board::SwitchType(radioSettings.switchConfig[i]);
      wname = RawSource(RawSourceType::SOURCE_TYPE_SWITCH, i).toString(nullptr, &radioSettings);
      if (swcfg == Board::SWITCH_3POS) {
        cb->addItem(wname + " Up", i | JS_BUTTON_3POS_UP);
        cb->addItem(wname + " Down", i | JS_BUTTON_3POS_DN);
      } else {
        cb->addItem(wname, i | JS_BUTTON_TOGGLE);
      }
    }
  }

  int ttlKnobs = Boards::getCapability(m_board, Board::Pots);
  
  for (i = 0; i < ttlKnobs; i += 1) {
    if (Board::PotType(radioSettings.potConfig[i]) == Board::POT_MULTIPOS_SWITCH) {
      cb->addItem("6P1", JS_BUTTON_6POS + i);
      cb->addItem("6P2", JS_BUTTON_6POS + i + (1 << JS_BUTTON_6POS_IDX_SHFT));
      cb->addItem("6P3", JS_BUTTON_6POS + i + (2 << JS_BUTTON_6POS_IDX_SHFT));
      cb->addItem("6P4", JS_BUTTON_6POS + i + (3 << JS_BUTTON_6POS_IDX_SHFT));
      cb->addItem("6P5", JS_BUTTON_6POS + i + (4 << JS_BUTTON_6POS_IDX_SHFT));
      cb->addItem("6P6", JS_BUTTON_6POS + i + (5 << JS_BUTTON_6POS_IDX_SHFT));
      break;
    }
  }
}

bool joystickDialog::loadJoysticks(int stick)
{
  QStringList joystickNames;
  bool found = false;

  ui->joystickCB->clear();
  joystickNames << tr("No joysticks found");
  joystick = new Joystick(0, false, 0, 0);
  if ( joystick ) {
    if ( joystick->joystickNames.count() > 0 ) {
      joystickNames = joystick->joystickNames;
      found = true;
    }
    joystick->close();
  }
  ui->joystickCB->insertItems(0, joystickNames);
  if (found && stick < joystickNames.size()) {
    ui->joystickCB->setCurrentIndex(stick);
  }
  else if (!found) {
    joystickSetEnabled(false);
  }
  return found;
}

void joystickDialog::joystickOpen(int stick)
{
  if (stick < 0)
    return;

  joystick = new Joystick(this, 1, false, 0);
  if (joystick && joystick->open(stick)) {
    numAxes = std::min(joystick->numAxes, MAX_JOYSTICKS);
    for (int j=0; j<numAxes; j++) {
      joystick->sensitivities[j] = 0;
      joystick->deadzones[j]=20;
    }
  }
  else {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Cannot open joystick."));
  }
}

void joystickDialog::joystickSetEnabled(bool enable)
{
  ui->joystickCB->setEnabled(enable);
  ui->calibrationWidget->setEnabled(enable);
  ui->nextButton->setEnabled(enable && step < 4);
  ui->backButton->setEnabled(enable && step);
}

void joystickDialog::onjoystickAxisValueChanged(int axis, int value)
{
  if (axis >= MAX_JOYSTICKS)
    return;

  if (started) {
    if (value > jscal[axis][2])
      jscal[axis][2] = value;
    else if (value < jscal[axis][0])
      jscal[axis][0] = value;

    sliders[axis]->setMinimum(jscal[axis][0]);
    sliders[axis]->setMaximum(jscal[axis][2]);
  }
  sliders[axis]->setInvertedAppearance(invert[axis]->isChecked());
  sliders[axis]->setValue(value);
}

void joystickDialog::onjoystickButtonValueChanged(int button, bool state)
{
  if (button >= MAX_JSBUTTONS)
    return;

  sliders[button + MAX_JOYSTICKS]->setValue(state);
  sliders[button + MAX_JOYSTICKS]->setInvertedAppearance(invert[button + MAX_JOYSTICKS]->isChecked());
}

void joystickDialog::loadStep()
{
  switch (step) {
    case 0:
      ui->howtoLabel->setText(tr("Press the Start button to start the stick range calibration procedure.\n" \
                                 "You may change the channel assignments or inversion at any time."));
      ui->backButton->setDisabled(true);
      break;
    case 1:
      started = true;
      ui->howtoLabel->setText(tr("Move sticks and pots in every direction making full movement\nPress Next when finished"));
      ui->nextButton->setText(tr("Next"));
      ui->backButton->setDisabled(true);
      break;
    case 2:
      ui->howtoLabel->setText(tr("Place sticks and pots in middle position.\nPress Next when done"));
      ui->backButton->setEnabled(true);
      ui->nextButton->setEnabled(true);
      break;
    case 3:
      for (int i=0; i < numAxes; i++) {
        jscal[i][1]=0;
        for (int j=0; j < 100; j++) {
          jscal[i][1] += joystick->getAxisValue(i);
        }
        jscal[i][1] /= 100;
      }
      ui->howtoLabel->setText(tr("Map joystick channels to controls using comboboxes.\nPress Next when done."));
      ui->nextButton->setEnabled(true);
      break;
    case 4:
      ui->howtoLabel->setText(tr("Check inversion checkbox to if necessary to reverse direction.\nPress Next when done."));
      ui->nextButton->setEnabled(true);
      break;
    case 5:
      ui->howtoLabel->setText(tr("Press OK to save configuration\nPress Cancel to abort joystick calibration without saving."));
      ui->nextButton->setDisabled(true);
  }
}

void joystickDialog::on_backButton_clicked()
{
  if (step) {
    --step;
    loadStep();
  }
}

void joystickDialog::on_nextButton_clicked()
{
  if (step < 5){
    ++step;
    loadStep();
  }
}

void joystickDialog::on_cancelButton_clicked()
{
  joystick->close();
  this->reject();
}

void joystickDialog::on_okButton_clicked()
{
  g.jsSupport(ui->joystickChkB->isChecked());
  g.jsCtrl(ui->joystickCB->currentIndex());

  if (joystick)
    joystick->close();

  if (!g.jsSupport() || g.jsCtrl() < 0) {
    this->accept();
    return;
  }

  if (started && step < 4) {
    int resp = QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("Calibration not complete, save anyway?"),
                                    QDialogButtonBox::Ok | QMessageBox::Default, QDialogButtonBox::Cancel | QMessageBox::Escape, QMessageBox::NoButton);
    if (resp == QDialogButtonBox::Cancel)
      return;
  }

  for (int i = 0; i < MAX_JOYSTICKS; ++i) {
    auto stick = sticks[i]->currentData().toInt();
    if (stick < 0) {
      g.joystick[i].stick_axe(-1);
    }
    else {
      g.joystick[i].stick_axe(stick);
      g.joystick[i].stick_max(jscal[i][2]);
      g.joystick[i].stick_med(jscal[i][1]);
      g.joystick[i].stick_min(jscal[i][0]);
      g.joystick[i].stick_inv(invert[i]->isChecked() );
      qDebug() << "joystick mapping " << sticks[i]->objectName() << "stick:" << i << "axe:" << stick;
    }
  }

  for (int i = 0; i < MAX_JSBUTTONS; ++i) {
    auto btn = sticks[i+MAX_JOYSTICKS]->currentData().toInt();
    if (btn < 0) {
      g.jsButton[i].button_idx(-1);
    }
    else {
      g.jsButton[i].button_idx(btn);
      g.jsButton[i].button_inv(invert[i+MAX_JOYSTICKS]->isChecked() );
      qDebug() << "joystick button mapping " << sticks[i+MAX_JOYSTICKS]->objectName() << "stick:" << i << "idx:" << btn;
    }
  }

  this->accept();
}
