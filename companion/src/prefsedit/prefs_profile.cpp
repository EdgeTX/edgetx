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

#include "prefs_profile.h"
#include "ui_prefs_profile.h"
#include "eeprominterface.h"
#include "moduledata.h"

constexpr char FIM_TEMPLATESETUP[]    {"Template Setup"};

PrefsProfilePanel::PrefsProfilePanel(QWidget * parent):
  PrefsPanel(parent),
  ui(new Ui::PrefsProfile),
  row(0),
  col(0)
{
  ui->setupUi(this);
  lock = true;

  panelItemModels->registerItemModel(new FilteredItemModel(GeneralSettings::templateSetupItemModel()), FIM_TEMPLATESETUP);

    // radio
/*   QString currType = QStringList(profile.fwType().split('-').mid(0, 2)).join('-');
  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    ui->boardCB->addItem(firmware->getName(), firmware->getId());
    if (currType == firmware->getId()) {
      ui->boardCB->setCurrentIndex(ui->boardCB->count() - 1);
    }
  }
 */
  // name
  ui->leName->setValue(profile.fwName());
  ui->leName->setBindSave([this] { profile.fwName(ui->leName->text()); });

  // language
  ui->cboLanguage->addItems(languageList());
  ui->cboLanguage->setValue(getLanguage());

  // new file
  row = col = 0;
  ui->csectNewFile->setTitle(tr("New Models and Settings Files"));
  ui->csectNewFile->setAnimationDuration(300);
  QGridLayout *layNewFile = new QGridLayout();
  AutoCheckBox *chkUseSettingsBackup = new AutoCheckBox(this, tr("Use backup settings"));
  layNewFile->addWidget(chkUseSettingsBackup, row, 1);
  newRow();
  AutoLabel *lblSettingsBackup = new AutoLabel(this, tr("Radio Settings Label"));
  layNewFile->addWidget(lblSettingsBackup, row, 1);
  newRow();
  AutoLabel *lblStickMode = new AutoLabel(this, tr("Stick Mode"));
  layNewFile->addWidget(lblStickMode, row, col++);
  AutoComboBox *cboStickMode = new AutoComboBox(this);
  layNewFile->addWidget(cboStickMode, row, col++);
  newRow();
  AutoLabel *lblChannelOrder = new AutoLabel(this, tr("Channel Order"));
  layNewFile->addWidget(lblChannelOrder, row, col++);
  AutoComboBox *cboChannelOrder = new AutoComboBox(this);
  layNewFile->addWidget(cboChannelOrder, row, col++);
  newRow();
  AutoLabel *lblModuleInternal = new AutoLabel(this, tr("Internal Module"));
  layNewFile->addWidget(lblModuleInternal, row, col++);
  AutoComboBox *cboModuleInternal = new AutoComboBox(this);
  layNewFile->addWidget(cboModuleInternal, row, col++);
  newRow();
  AutoLabel *lblModuleExternal = new AutoLabel(this, tr("External Module"));
  layNewFile->addWidget(lblModuleExternal, row, col++);
  AutoComboBox *cboModuleExternal = new AutoComboBox(this);
  layNewFile->addWidget(cboModuleExternal, row, col++);
  addHSpring(layNewFile, row, col);
  ui->csectNewFile->setContentLayout(*layNewFile);
  ui->csectNewFile->setBindResize([this] { this->shrink(); });

  // folders
  row = col = 0;
  ui->csectFolders->setTitle(tr("Folders"));
  ui->csectFolders->setAnimationDuration(300);
  QGridLayout *layFolders = new QGridLayout();
  QLabel *lblSDPath = new QLabel(tr("SD Path"), this);
  layFolders->addWidget(lblSDPath, row, col++);
  AutoLineEdit *leSDPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leSDPath, row, col++);
  AutoFolderSelectButton *btnSDPath = new AutoFolderSelectButton(this);
  btnSDPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  layFolders->addWidget(btnSDPath, row, col++);
  newRow();
  QLabel *lblModelsPath = new QLabel(tr("Models"), this);
  layFolders->addWidget(lblModelsPath, row, col++);
  AutoLineEdit *leModelsPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leModelsPath, row, col++);
  AutoFolderSelectButton *btnModelsPath = new AutoFolderSelectButton(this);
  btnModelsPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  layFolders->addWidget(btnModelsPath, row, col++);
  newRow();
  QLabel *lblBackupsPath = new QLabel(tr("Backups"), this);
  layFolders->addWidget(lblBackupsPath, row, col++);
  AutoLineEdit *leBackupsPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leBackupsPath, row, col++);
  AutoFolderSelectButton *btnBackupsPath = new AutoFolderSelectButton(this);
  btnBackupsPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  layFolders->addWidget(btnBackupsPath, row, col++);
  addHSpring(layFolders, row, col);
  ui->csectFolders->setContentLayout(*layFolders);
  ui->csectFolders->setBindResize([this] { this->shrink(); });

  // options  TODO split into custom and those supported by Cloud Build
  ui->csectFirmwareOpts->setTitle(tr("Firmware Options"));
  ui->csectFirmwareOpts->setAnimationDuration(300);
  QGridLayout *layFirmwareOpts = new QGridLayout();
  addHSpring(layFirmwareOpts, row, col);
  ui->csectFirmwareOpts->setContentLayout(*layFirmwareOpts);
  ui->csectFirmwareOpts->setBindResize([this] { this->shrink(); });

  // B&W firmware splash image
  row = col = 0;
  ui->csectSplash->setTitle(tr("Splash Screen"));
  ui->csectSplash->setAnimationDuration(300);
  QGridLayout *laySplash = new QGridLayout();
  AutoLineEdit *leSplashPath = new AutoLineEdit(this, true);
  laySplash->addWidget(leSplashPath, row, col++);
  AutoFolderSelectButton *btnSplashSelect = new AutoFolderSelectButton(this);
  laySplash->addWidget(btnSplashSelect, row, col++);
  newRow();
  QLabel *lblSplashImage = new QLabel(this);
  laySplash->addWidget(lblSplashImage, row, col++);
  AutoPushButton *btnSplashClear = new AutoPushButton(this);
  laySplash->addWidget(btnSplashClear, row, col++);
  addHSpring(laySplash, row, col);
  ui->csectSplash->setContentLayout(*laySplash);
  ui->csectSplash->setBindResize([this] { this->shrink(); });
  ui->csectSplash->setBindVisible([this] {
    return !Boards::getCapability(getBaseFirmware()->getBoard(), Board::HasColorLcd);
  });

  update();
  shrink();
  lock = false;
}

PrefsProfilePanel::~PrefsProfilePanel()
{
  delete ui;
}


void PrefsProfilePanel::save()
{
  AbstractPanel::save();
}

void PrefsProfilePanel::update()
{
  AbstractPanel::update();
}

Firmware *PrefsProfilePanel::getBaseFirmware() const
{
  return Firmware::getFirmwareForId(ui->cboRadio->currentData().toString());
}

Firmware * PrefsProfilePanel::getFirmwareVariant() const
{
  QString id = ui->cboRadio->currentData().toString();

/*   foreach(QCheckBox *cb, firmwareOptions.values()) {
    if (cb->isChecked())
      id += "-" + cb->text();
  }
 */
  if (ui->cboLanguage->count())
    id += "-" + ui->cboLanguage->currentText();

  return Firmware::getFirmwareForId(id);
}

QString PrefsProfilePanel::getLanguage()
{
  QString fwLang = Firmware::getCurrentVariant()->getLanguage();

  if (fwLang.isEmpty()) // try to detect os language
    fwLang = QLocale::languageToString(QLocale().language()).split("_").first();

  return fwLang;
}

QStringList PrefsProfilePanel::languageList()
{
  QStringList strl;
  QString fwLang(getLanguage());

  //for (const char *lang : firmware->getFirmwareBase()->languageList())
  //  strl.append(lang);

  if (!strl.contains(fwLang))
    strl.append(fwLang);

  strl.sort();
  return strl;
}
