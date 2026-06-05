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

#include "prefs_simu.h"
#include "ui_settings_simu.h"
#include "appdata.h"

#if defined(USE_SDL)
#include "joystick.h"
#include "joystickdialog.h"
#endif

#include <QPalette>
#include <QColorDialog>

PrefsSimuPanel::PrefsSimuPanel(QWidget * parent):
  PrefsPanel(parent),
  ui(new Ui::Simu)
{
  ui->setupUi(this);
  lock = true;

#if !defined(USE_SDL)
  ui->joystickCB->hide();
  ui->joystickCB->setDisabled(true);
  ui->joystickcalButton->hide();
  ui->joystickChkB->hide();
  ui->label_11->hide();
#endif


  update();
  lock = false;
}

PrefsSimuPanel::~PrefsSimuPanel()
{
  delete ui;
}

bool PrefsSimuPanel::save()
{
  g.simuSW(ui->simuSW->isChecked());
  g.backLight(ui->backLightColor->currentIndex());
  g.simuGenericKeysPos((AppData::SimuGenericKeysPos)ui->cboSimuGenericKeysPos->currentIndex());
  g.simuScrollButtons(ui->chkSimuScrollButtons->isChecked());

  g.disableJoystickWarning(ui->joystickWarningCB->isChecked());

  if (ui->joystickChkB ->isChecked()) {
    g.jsSupport(ui->joystickChkB ->isChecked());
    // Don't overwrite selected joystick if not connected. Avoid surprising the user.
    if (ui->joystickCB->isEnabled()) {
      profile.jsName(ui->joystickCB->currentText());
      g.loadNamedJS();
    }
  }
  else {
    g.jsSupport(false);
  }

}

void PrefsSimuPanel::update()
{
  ui->snapshotPath->setText(g.snapshotDir());
  ui->snapshotPath->setReadOnly(true);
  ui->snapshotClipboardCKB->setChecked(g.snapToClpbrd());
  if (ui->snapshotClipboardCKB->isChecked()) {
    ui->snapshotPath->setDisabled(true);
    ui->snapshotPathButton->setDisabled(true);
  }
  ui->simuSW->setChecked(g.simuSW());
  ui->backLightColor->setCurrentIndex(g.backLight());
  if (!Boards::getCapability(getCurrentBoard(), Board::HasBacklightColor))
    ui->backLightColor->setEnabled(false);
  ui->volumeGain->setValue(profile.volumeGain() / 10.0);
  ui->cboSimuGenericKeysPos->addItems(AppData::simuGenericKeysPosList());
  ui->cboSimuGenericKeysPos->setCurrentIndex(g.simuGenericKeysPos());
  ui->chkSimuScrollButtons->setChecked(g.simuScrollButtons());
  ui->joystickWarningCB->setChecked(g.disableJoystickWarning());
  ui->chkUseSavedPrefsApp->setChecked(g.useSavedPrefs());

#if defined(USE_SDL)
  ui->joystickChkB->setChecked(g.jsSupport());
  if (ui->joystickChkB->isChecked()) {
    QStringList joystickNames;
    joystickNames << tr("No joysticks found");
    joystick = new Joystick(0,0,false,0);
    ui->joystickcalButton->setDisabled(true);
    ui->joystickCB->setDisabled(true);

    if ( joystick ) {
      if ( joystick->joystickNames.count() > 0 ) {
        joystickNames = joystick->joystickNames;
        ui->joystickCB->setEnabled(true);
        ui->joystickcalButton->setEnabled(true);
      }
      joystick->close();
    }
    ui->joystickCB->clear();
    ui->joystickCB->insertItems(0, joystickNames);
    int stick = joystick->findCurrent(g.currentProfile().jsName());
    ui->joystickCB->setCurrentIndex(stick);
  }
  else {
    ui->joystickCB->clear();
    ui->joystickCB->setDisabled(true);
    ui->joystickcalButton->setDisabled(true);
  }
#endif

}

void PrefsSimuPanel::on_snapshotClipboardCKB_clicked()
{
  if (ui->snapshotClipboardCKB->isChecked()) {
    ui->snapshotPath->setDisabled(true);
    ui->snapshotPathButton->setDisabled(true);
    g.snapToClpbrd(true);
  }
  else {
    ui->snapshotPath->setEnabled(true);
    ui->snapshotPath->setReadOnly(true);
    ui->snapshotPathButton->setEnabled(true);
    g.snapToClpbrd(false);
  }
}

void PrefsSimuPanel::on_btnClearPos_clicked()
{
  SimulatorOptions opts = g.profile[g.sessionId()].simulatorOptions();
  opts.controlsState.clear();
  g.profile[g.sessionId()].simulatorOptions(opts);
}

#if defined(USE_SDL)
void PrefsSimuPanel::on_joystickChkB_clicked() {
  if (ui->joystickChkB->isChecked()) {
    QStringList joystickNames;
    joystickNames << tr("No joysticks found");
    joystick = new Joystick(0,0,false,0);
    ui->joystickcalButton->setDisabled(true);
    ui->joystickCB->setDisabled(true);

    if ( joystick ) {
      if ( joystick->joystickNames.count() > 0 ) {
        joystickNames = joystick->joystickNames;
        ui->joystickCB->setEnabled(true);
        ui->joystickcalButton->setEnabled(true);
      }
      joystick->close();
    }
    ui->joystickCB->clear();
    ui->joystickCB->insertItems(0, joystickNames);
  }
  else {
    ui->joystickCB->clear();
    ui->joystickCB->setDisabled(true);
    ui->joystickcalButton->setDisabled(true);
  }
}

void PrefsSimuPanel::on_joystickcalButton_clicked() {
  g.currentProfile().jsName(ui->joystickCB->currentText());
  joystickDialog * jd = new joystickDialog(this);
  jd->exec();
}
#endif

void PrefsSimuPanel::on_chkSimBtnClickedUseOSTheme_stateChanged()
{
  if (ui->chkSimBtnClickedUseOSTheme->isChecked()) {
    ui->lblSimBtnClickedColorSample->setVisible(false);
    ui->btnSimBtnClickedColor->setEnabled(false);
  } else {
    ui->lblSimBtnClickedColorSample->setVisible(true);
    ui->btnSimBtnClickedColor->setEnabled(true);
  }
}

void PrefsSimuPanel::on_btnSimBtnClickedColor_clicked()
{
  QColorDialog *dlg = new QColorDialog(this);
  QColor color = dlg->getColor(g.currentProfile().simBtnClickedColor(), this);
  ui->lblSimBtnClickedColorSample->setPalette(QPalette(color));
  ui->lblSimBtnClickedColorSample->repaint();
}


