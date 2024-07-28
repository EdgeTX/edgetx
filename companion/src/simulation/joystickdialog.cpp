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

#include "joystickdialog.h"
#include "ui_joystickdialog.h"
#include "eeprominterface.h"

#include "boards.h"
#include "constants.h"

joystickDialog::joystickDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::joystickDialog),
  step(0),
  numAxes(0),
  numButtons(0),
  started(false)
{
  ui->setupUi(this);

  int i;

  for (i = 0; i < MAX_JS_AXES; i += 1) {
    jscal[i][0] = 32767;
    jscal[i][1] = 0;
    jscal[i][2] = -32768;
  }

  ui->backButton->setEnabled(false);

  ui->joystickChkB->setChecked(g.jsSupport());

  if (loadJoysticks()) {
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

void joystickDialog::loadGrid()
{
  int i;
  char s[20];

  QGridLayout *grid = findChild<QGridLayout*>("gridLayout");

  QLayoutItem* item;
  while ((item = grid->takeAt(0)) != NULL)
  {
      delete item->widget();
      delete item;
  }

  memset(sliders, 0, sizeof(sliders));
  memset(sticks, 0, sizeof(sticks));
  memset(invert, 0, sizeof(invert));

  int row = 0;
  int col = 0;
  if (grid) {
    for (i = 0; i < numAxes; i += 1, row += 1) {
      col = (i & 1) * 4;
      sprintf(s, "Ch%d", i + 1);
      QLabel *l = new QLabel(s);
      grid->addWidget(l, row/2, col+0, 1, 1);
      QSlider *s = new QSlider(Qt::Horizontal);
      s->setMinimum(-32767);
      s->setMaximum(32767);
      sliders[i] = s;
      grid->addWidget(s, row/2, col+1, 1, 1);
      QCheckBox *c = new QCheckBox("");
      invert[i] = c;
      grid->addWidget(c, row/2, col+2, 1, 1);
      QComboBox *d = new QComboBox();
      populateSourceCombo(d);
      sticks[i] = d;
      grid->addWidget(d, row/2, col+3, 1, 1);
    }
    if (row & 1) row += 1;
    for (i = 0; i < numButtons; i += 1, row += 1) {
      col = (i & 1) * 4;
      sprintf(s, "Btn%d", i + 1);
      QLabel *l = new QLabel(s);
      grid->addWidget(l, row/2, col+0, 1, 1);
      QSlider *s = new QSlider(Qt::Horizontal);
      s->setMinimum(0);
      s->setMaximum(1);
      sliders[i+numAxes] = s;
      grid->addWidget(s, row/2, col+1, 1, 1);
      QComboBox *d = new QComboBox();
      populateButtonCombo(d);
      sticks[i+numAxes] = d;
      grid->addWidget(d, row/2, col+3, 1, 1);
    }
  }

  for (int i = 0; i < numAxes; ++i) {
    if (g.joystick[i].existsOnDisk()) {
      jscal[i][0] = g.joystick[i].stick_min();
      jscal[i][1] = g.joystick[i].stick_med();
      jscal[i][2] = g.joystick[i].stick_max();
      sliders[i]->setMinimum(jscal[i][0]);
      sliders[i]->setMaximum(jscal[i][2]);
      invert[i]->setChecked(g.joystick[i].stick_inv());
      sticks[i]->setCurrentIndex(sticks[i]->findData(g.joystick[i].stick_axe()));
    }
  }

  for (int i = 0; i < numButtons; ++i) {
    if (g.jsButton[i].existsOnDisk()) {
      sticks[i+numAxes]->setCurrentIndex(sticks[i+numAxes]->findData(g.jsButton[i].button_idx()));
    }
  }
}

void joystickDialog::populateSourceCombo(QComboBox * cb)
{
  int i;
  QString wname;

  Board::Type m_board = getCurrentBoard();
  GeneralSettings radioSettings = GeneralSettings();

  int ttlInputs = Boards::getCapability(m_board, Board::Inputs);

  cb->clear();
  cb->addItem(tr("Not Assigned"), -1);

  for (i = 0; i < ttlInputs; ++i) {
    if (radioSettings.isInputAvailable(i) &&
        (radioSettings.isInputStick(i) || radioSettings.isInputPot(i) || radioSettings.isInputSlider(i))) {
      if (radioSettings.isInputStick(i))
        wname = Boards::getAxisName(i);
      else
        wname = RawSource(RawSourceType::SOURCE_TYPE_INPUT, i + 1).toString(nullptr, &radioSettings);
      cb->addItem(wname, i);
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
  int ttlTrims = Boards::getCapability(m_board, Board::NumTrims);

  cb->clear();
  cb->addItem(tr("Not Assigned"), -1);

  Board::SwitchType swtype;
  for (i = 0; i < ttlSwitches; ++i) {
    if (radioSettings.switchConfig[i].type != Board::SWITCH_NOT_AVAILABLE) {
      swtype = Board::SwitchType(radioSettings.switchConfig[i].type);
      wname = RawSource(RawSourceType::SOURCE_TYPE_SWITCH, i + 1).toString(nullptr, &radioSettings);
      if (swtype == Board::SWITCH_3POS) {
        cb->addItem(wname + CPN_STR_SW_INDICATOR_UP, i | JS_BUTTON_3POS_UP);
        cb->addItem(wname + CPN_STR_SW_INDICATOR_DN, i | JS_BUTTON_3POS_DN);
      } else {
        cb->addItem(wname, i | JS_BUTTON_TOGGLE);
      }
    }
  }

  for (i = 0; i < ttlTrims; i += 1) {
    wname = RawSource(RawSourceType::SOURCE_TYPE_TRIM, i + 1).toString(nullptr, &radioSettings);
    if ((i == 0) || (i == 3)) {
      cb->addItem(wname + " Left", (i + ttlSwitches) | JS_BUTTON_3POS_DN);
      cb->addItem(wname + " Right", (i + ttlSwitches) | JS_BUTTON_3POS_UP);
    } else {
      cb->addItem(wname + " Down", (i + ttlSwitches) | JS_BUTTON_3POS_DN);
      cb->addItem(wname + " Up", (i + ttlSwitches) | JS_BUTTON_3POS_UP);
    }
  }
}

bool joystickDialog::loadJoysticks()
{
  QStringList joystickNames;
  bool found = false;

  ui->joystickCB->clear();
  joystickNames << tr("No joysticks found");
  joystick = new Joystick(0, 0, false, 0);
  if ( joystick ) {
    if ( joystick->joystickNames.count() > 0 ) {
      joystickNames = joystick->joystickNames;
      found = true;
    }
    joystick->close();
  }
  ui->joystickCB->insertItems(0, joystickNames);
  if (found) {
    int stick = joystick->findCurrent(g.currentProfile().jsName());
    ui->joystickCB->setCurrentIndex(stick);
  }
  else if (!found) {
    joystickSetEnabled(false);
  }
  return found;
}

void joystickDialog::joystickOpen(int stick)
{
  numAxes = 0;
  numButtons = 0;

  if (stick < 0)
    return;

  joystick = new Joystick(this, 1, false, 0);
  if (joystick && joystick->open(stick)) {
    numAxes = std::min(joystick->numAxes, MAX_JS_AXES);
    numButtons = std::min(joystick->numButtons, MAX_JS_BUTTONS);
    for (int j=0; j<numAxes; j++) {
      joystick->sensitivities[j] = 0;
      joystick->deadzones[j] = 20;
    }
  }
  else {
    QMessageBox::critical(this, CPN_STR_TTL_ERROR, tr("Cannot open joystick."));
  }
  g.currentProfile().jsName(ui->joystickCB->currentText());
  g.loadNamedJS();
  loadGrid();
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
  if (axis >= numAxes)
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
  if (button >= numButtons)
    return;

  sliders[button + numAxes]->setValue(state);
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
      for (int i=0; i < numAxes; i++) {
        jscal[i][0] = 0;
        jscal[i][2] = 0;
      }
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
  g.currentProfile().jsName(ui->joystickCB->currentText());

  if (joystick)
    joystick->close();

  if (!g.jsSupport()) {
    this->accept();
    return;
  }

  if (started && step < 4) {
    int resp = QMessageBox::warning(this, CPN_STR_TTL_WARNING, tr("Calibration not complete, save anyway?"),
                                    QDialogButtonBox::Ok | QMessageBox::Default, QDialogButtonBox::Cancel | QMessageBox::Escape, QMessageBox::NoButton);
    if (resp == QDialogButtonBox::Cancel)
      return;
  }

  g.clearJSData();

  for (int i = 0; i < numAxes; ++i) {
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
      qDebug() << "joystick mapping " << sticks[i]->currentText() << "stick:" << i << "axe:" << stick << jscal[i][0] << jscal[i][1] << jscal[i][2];
    }
  }

  for (int i = 0; i < numButtons; ++i) {
    auto btn = sticks[i+numAxes]->currentData().toInt();
    if (btn < 0) {
      g.jsButton[i].button_idx(-1);
    }
    else {
      g.jsButton[i].button_idx(btn);
      qDebug() << "joystick button mapping " << sticks[i+numAxes]->currentText() << "button:" << i << "idx:" << btn;
    }
  }

  g.saveNamedJS();

  this->accept();
}
