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

#include "prefs_edit.h"
#include "prefs_profile.h"
#include "prefs_app.h"
#include "prefs_simu.h"
#include "prefs_update.h"
#include "ui_prefs_edit.h"
#include "helpers.h"

#include <QMessageBox>

PrefsEditDialog::PrefsEditDialog(QWidget * parent, UpdateFactories * factories) :
  QDialog(parent),
  ui(new Ui::PrefsEdit),
  mainWinHasDirtyChild(false),
  firmware(getCurrentFirmware()),
  board(getCurrentBoard()),
  profile(g.currentProfile()),
  dirty(false)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("apppreferences.png"));
  setAttribute(Qt::WA_DeleteOnClose);

  PrefsProfilePanel *prefsProfPanel = new PrefsProfilePanel(this, firmware, board, profile);
  addTab(prefsProfPanel, tr("Radio Profile"));
  connect(prefsProfPanel, &PrefsProfilePanel::radioChanged, this, &PrefsEditDialog::onRadioChanged);

  addTab(new PrefsAppPanel(this, firmware, board, profile), tr("Application"));
  addTab(new PrefsSimuPanel(this, firmware, board, profile), tr("Simulator"));

  PrefsUpdatePanel *prefsUpdatePanel = new PrefsUpdatePanel(this, firmware, board, profile, factories);
  addTab(prefsUpdatePanel, tr("Update"));

  connect(prefsProfPanel, &PrefsProfilePanel::sdPathChanged, prefsUpdatePanel, &PrefsUpdatePanel::onSDPathChanged);

  bool hasSavedGeo = restoreGeometry(g.prefsEditGeo());

  if (!hasSavedGeo)
    shrink();

  ui->tabWidget->setCurrentIndex(0);
}

PrefsEditDialog::~PrefsEditDialog()
{
  delete ui;
}

void PrefsEditDialog::accept()
{
  if (!save())
    return;

  QDialog::accept();
}

void PrefsEditDialog::closeEvent(QCloseEvent *event)
{
  if (!maybeSave())
    return;

  QDialog::closeEvent(event);
}

void PrefsEditDialog::reject()
{
  if (!maybeSave())
    return;

  QDialog::reject();
}

PrefsPanel * PrefsEditDialog::addTab(PrefsPanel * panel, QString text)
{
  panels << panel;
  PrefsScrollArea *scrollArea = new PrefsScrollArea(ui->tabWidget, panel);
  ui->tabWidget->addTab(scrollArea, text);
  connect(panel, &PrefsPanel::modified, this, [this] { this->dirty = true; });
  return panel;
}

void PrefsEditDialog::done(int r)
{
  if (!isMaximized())
    g.prefsEditGeo(saveGeometry());

  QDialog::done(r);
}

bool PrefsEditDialog::maybeSave()
{
  if (dirty) {
    int ret = QMessageBox::question(this, tr("Edit Preferences"),
                tr("Preferences have been modified.\nDo you want to save your changes?"),
                (QMessageBox::Save | QMessageBox::Discard), QMessageBox::Save);

    if (ret == QMessageBox::Save)
      return save();
  }

  return true;
}

bool PrefsEditDialog::save()
{
  if (dirty) {
    bool fwchange = false;
    // If a new fw type has been choosen, several things need to reset
    if (getCurrentFirmware()->getFirmwareBase()->getId() != firmware->getFirmwareBase()->getId()) {
      // check if we're going to be converting to a new radio type and there are unsaved files in the main window
      if (mainWinHasDirtyChild && !Boards::isBoardCompatible(getCurrentBoard(), firmware->getBoard())) {
        QString q = tr("<p><b>You cannot change Radio Types while there are unsaved model file changes. What do you wish to do?</b></p> <ul>" \
                      "<li><i>Save All</i> - save all open model file(s) before saving preferences.<li>" \
                      "<li><i>Reset</i> - revert Radio Type and Build Options before saving all other preferences.</li>" \
                      "<li><i>Cancel</i> - return to the editing preferences.</li></ul>");
        int resp = QMessageBox::question(this, windowTitle(), q, (QMessageBox::SaveAll | QMessageBox::Reset | QMessageBox::Cancel), QMessageBox::Cancel);
        if (resp == QMessageBox::SaveAll) {
          // signal main window to save files
          // need to do this before the current firmware actually changes
          emit profileFirmwareAboutToChange();
          fwchange = true;
        } else if (resp == QMessageBox::Reset) {
          // reset firmware to that at time of dialog ctor
          onRadioChanged(getCurrentFirmware());
        } else {
          // we do not accept the dialog close
          return false;
        }
      } else {
        fwchange = true;
      }
    }

    // save preferences for every tab
    for (const auto panel : panels)
      panel->save();

    // prevent re-prompting
    dirty = false;

    hide();

    if (fwchange) {
      // do not keep incompatible backup settings
      profile.generalSettings(QByteArray());
      profile.timeStamp(QString());
      // used by flash firmware so no longer applicable
      profile.fwName(QString());
      emit profileFirmwareChanged();
    }
  }

  return true;
}

void PrefsEditDialog::setMainWinHasDirtyChild(bool value)
{
  mainWinHasDirtyChild = value;
}

void PrefsEditDialog::shrink()
{
// adjustSize() only accounts for the currently visible tab, since
// QStackedWidget doesn't lay out hidden pages. Walk every tab once so
// each panel reports its real size hint, and size the dialog to fit
// the largest one so switching tabs later doesn't resize the window.
  QSize maxHint;
  const int current = ui->tabWidget->currentIndex();

  for (int i = 0; i < ui->tabWidget->count(); i++) {
    ui->tabWidget->setCurrentIndex(i);
    ui->tabWidget->currentWidget()->adjustSize();
    maxHint = maxHint.expandedTo(ui->tabWidget->currentWidget()->sizeHint());
  }

  ui->tabWidget->setCurrentIndex(current);
  adjustSize();
  resize(maxHint.expandedTo(size()));
}

void PrefsEditDialog::onRadioChanged(Firmware * newFirmware)
{
  firmware = newFirmware;

  for (const auto panel : panels)
    panel->onRadioChanged(firmware);
}
