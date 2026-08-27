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

#include "generaledit.h"
#include "ui_generaledit.h"
#include "helpers.h"
#include "appdata.h"
#include "generalsetup.h"
#include "trainer.h"
#include "hardware.h"
#include "../modeledit/customfunctions.h"
#include "generaloptions.h"
#include "scrollarea.h"
#include "compounditemmodels.h"
#include "firmwares/edgetx/edgetxinterface.h"
#include "generalfavorites.h"
#include "generalkeyshortcuts.h"

GeneralEdit::GeneralEdit(QWidget * parent, RadioData & radioData, Firmware * firmware) :
  QDialog(parent),
  ui(new Ui::GeneralEdit),
  generalSettings(radioData.generalSettings),
  firmware(firmware)
{
  ui->setupUi(this);
  setWindowIcon(CompanionIcon("open.png"));
  setAttribute(Qt::WA_DeleteOnClose);

  editorItemModels = new CompoundItemModelFactory(&generalSettings, nullptr);
  // tabs created below expect these item models to be pre-registered
  editorItemModels->addItemModel(AbstractItemModel::IMID_RawSource);
  editorItemModels->addItemModel(AbstractItemModel::IMID_RawSwitch);
  editorItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncAction);
  editorItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncResetParam);
  editorItemModels->addItemModel(AbstractItemModel::IMID_ControlSource);

  addTab(new GeneralSetupPanel(this, generalSettings, firmware, editorItemModels), tr("Setup"));
  addTab(new CustomFunctionsPanel(this, nullptr, generalSettings, firmware, editorItemModels), tr("Global Functions"));
  addTab(new TrainerPanel(this, generalSettings, firmware, editorItemModels), tr("Trainer"));
  auto hwpnl = new HardwarePanel(this, generalSettings, firmware, editorItemModels);
  addTab(hwpnl, tr("Hardware"));
  addTab(new GeneralOptionsPanel(this, generalSettings, firmware), tr("Enabled Features"));
  if (firmware->getCapability(QMFavourites))
    addTab(new GeneralFavsPanel(this, generalSettings, firmware), tr("Favourites"));
  if (firmware->getCapability(KeyShortcuts))
    addTab(new GeneralKeysPanel(this, generalSettings, firmware), tr("Key Shortcuts"));

  ui->btnSettingsRestore->setDisabled(g.currentProfile().generalSettings().isEmpty());
  ui->btnSettingsBackupDelete->setDisabled(g.currentProfile().generalSettings().isEmpty());

  connect(hwpnl, &HardwarePanel::internalModuleChanged, this, [&] { intModChanged = true; });
  connect(ui->btnSettingsBackup, &QPushButton::clicked, this, &GeneralEdit::on_btnSettingsBackupClicked);
  connect(ui->btnSettingsRestore, &QPushButton::clicked, this, &GeneralEdit::on_btnSettingsRestoreClicked);
  connect(ui->btnSettingsBackupDelete, &QPushButton::clicked, this, &GeneralEdit::on_btnSettingsBackupDeleteClicked);
  connect(ui->btnSettingsDefaults, &QPushButton::clicked, this, &GeneralEdit::on_btnSettingsDefaultsClicked);

  ui->tabWidget->setCurrentIndex(g.generalEditTab());
}

GeneralEdit::~GeneralEdit()
{
  delete ui;
  delete editorItemModels;  // cleans up item models registered by the tabs
}

void GeneralEdit::closeEvent(QCloseEvent *event)
{
  g.generalEditTab(ui->tabWidget->currentIndex());
  if (intModChanged)
    emit internalModuleChanged(); // for MidiChild to trap
}

void GeneralEdit::addTab(GenericPanel *panel, QString text)
{
  panels << panel;
  QWidget * widget = new QWidget(ui->tabWidget);
  QVBoxLayout *baseLayout = new QVBoxLayout(widget);
  ScrollArea * area = new ScrollArea(widget, panel);
  baseLayout->addWidget(area);
  ui->tabWidget->addTab(widget, text);
  connect(panel, &GenericPanel::modified, this, &GeneralEdit::onTabModified);
}

void GeneralEdit::onTabModified()
{
  emit modified();
}

void GeneralEdit::on_tabWidget_currentChanged(int index)
{
  panels[index]->update();
}

void GeneralEdit::on_btnSettingsRestoreClicked()
{
  if (QMessageBox::question(this, tr("Radio Settings"),
        tr("WARNING: Restore settings from profile.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  const unsigned int oldIntMod = generalSettings.internalModule;
  QByteArray data = g.currentProfile().generalSettings();

  try {
    if (!loadRadioSettingsFromYaml(generalSettings, data)) {
      QMessageBox::critical(this, tr("Radio Settings"), tr("Unable to restore settings from profile!"));
      return;
    }
  } catch(const std::runtime_error& e) {
    QMessageBox::critical(this, tr("Radio Settings"),
      tr("Unable to restore settings from profile!") + ":\n" + QString(e.what()));
    return;
  }

  postSettingsChangeMsg(oldIntMod);
  emit modified();
  closeEvent(new QCloseEvent());
  done(-1);
}

void GeneralEdit::on_btnSettingsBackupClicked()
{
  if (QMessageBox::question(this, tr("Radio Settings"),
        tr("WARNING: Backup settings to profile.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  QByteArray data;

  try {
    if (!writeRadioSettingsToYaml(generalSettings, data)) {
      QMessageBox::critical(this, tr("Radio Settings"), tr("Unable to backup settings to profile!"));
      return;
    }
  } catch(const std::runtime_error& e) {
    QMessageBox::critical(this, tr("Radio Settings"),
      tr("Unable to backup settings to profile!") + ":\n" + QString(e.what()));
    return;
  }

  g.currentProfile().generalSettings(data);
  QDateTime dateTime = QDateTime::currentDateTime();
  g.currentProfile().timeStamp(dateTime.toString("yyyy-MM-dd hh:mm"));
  ui->btnSettingsRestore->setEnabled(true);
  ui->btnSettingsBackupDelete->setEnabled(true);
  QMessageBox::information(this, tr("Radio Settings"), tr("Settings saved to profile."));
}

void GeneralEdit::on_btnSettingsBackupDeleteClicked()
{
  if (QMessageBox::question(this, tr("Radio Settings"),
        tr("WARNING: Delete settings backup from profile.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  g.currentProfile().generalSettings(QByteArray());
  g.currentProfile().timeStamp(QString());
  ui->btnSettingsRestore->setEnabled(false);
  ui->btnSettingsBackupDelete->setEnabled(false);
  QMessageBox::information(this, tr("Radio Settings"), tr("Settings backup deleted from profile."));
}

void GeneralEdit::on_btnSettingsDefaultsClicked()
{
  if (QMessageBox::question(this, tr("Radio Settings"),
        tr("WARNING: Reset settings to defaults.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  const unsigned int oldIntMod = generalSettings.internalModule;
  generalSettings.clear();
  postSettingsChangeMsg(oldIntMod);
  emit modified();
  closeEvent(new QCloseEvent());
  done(-1);
}

void GeneralEdit::postSettingsChangeMsg(const unsigned int oldIntMod)
{
  QString intmodmsg;

  if (oldIntMod != generalSettings.internalModule) {
    intModChanged = true;
    intmodmsg = tr("The internal module type has been changed and may trigger model updates.") + "\n";
  }

  QMessageBox::information(this, tr("Radio Settings"),
    tr("Default settings successfully applied.") + "\n" +
    intmodmsg +
    tr("The Radio Settings window will be closed and re-opened for the changes to take effect."));
}
