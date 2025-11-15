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

GeneralEdit::GeneralEdit(QWidget * parent, RadioData & radioData, Firmware * firmware) :
  QDialog(parent),
  ui(new Ui::GeneralEdit),
  generalSettings(radioData.generalSettings),
  firmware(firmware)
{
  ui->setupUi(this);
  this->setWindowIcon(CompanionIcon("open.png"));
  ui->btnLoadSettings->setDisabled(g.currentProfile().generalSettings().isEmpty());
  ui->btnClearSettings->setDisabled(g.currentProfile().generalSettings().isEmpty());

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

  connect(hwpnl, &HardwarePanel::internalModuleChanged, this, [&] { intModChanged = true; });

  ui->tabWidget->setCurrentIndex( g.generalEditTab() );
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

void GeneralEdit::on_btnLoadSettings_clicked()
{
  if (QMessageBox::question(this, tr("Profile Radio Settings"),
        tr("WARNING: Loading settings from profile is irreversable.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  QByteArray data = g.currentProfile().generalSettings();

  try {
    if (!loadRadioSettingsFromYaml(generalSettings, data)) {
      QMessageBox::critical(this, tr("Profile Radio Settings"), tr("Unable to load settings from profile!"));
      return;
    }
  } catch(const std::runtime_error& e) {
    QMessageBox::critical(this, tr("Profile Radio Settings"),
      tr("Unable to load settings from profile!") + ":\n" + QString(e.what()));
    return;
  }

  on_tabWidget_currentChanged(ui->tabWidget->currentIndex());
  QMessageBox::information(this, tr("Profile Radio Settings"),
    tr("Settings successfully loaded.") + "\n\n" +
    tr("The Radio Settings window will now be closed for the settings to take effect"));

  emit modified();
  accept();
}

void GeneralEdit::on_btnSaveSettings_clicked()
{
  if (QMessageBox::question(this, tr("Profile Radio Settings"),
        tr("WARNING: Saving settings to the profile is irreversable.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  QByteArray data;
  writeRadioSettingsToYaml(generalSettings, data);
  g.currentProfile().generalSettings(data);
  QDateTime dateTime = QDateTime::currentDateTime();
  g.currentProfile().timeStamp(dateTime.toString("yyyy-MM-dd hh:mm"));
  ui->btnLoadSettings->setEnabled(true);
  ui->btnClearSettings->setEnabled(true);
  QMessageBox::information(this, tr("Save Radio Settings to Profile"), tr("Settings saved to profile."));
}

void GeneralEdit::on_btnClearSettings_clicked()
{
  if (QMessageBox::question(this, tr("Profile Radio Settings"),
        tr("WARNING: Clearing settings from the profile is irreversable.\n\nAre you sure?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return;

  g.currentProfile().generalSettings(QByteArray());
  g.currentProfile().timeStamp(QString());
  ui->btnLoadSettings->setEnabled(false);
  ui->btnClearSettings->setEnabled(false);
  QMessageBox::information(this, tr("Profile Radio Settings"), tr("Settings cleared from profile."));
}
