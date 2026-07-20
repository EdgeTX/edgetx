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
#include "moduledata.h"

constexpr char FIM_TEMPLATESETUP[]    {"Template Setup"};

PrefsProfilePanel::PrefsProfilePanel(QWidget * parent):
  PrefsPanel(parent),
  ui(new Ui::PrefsProfile)
{
  ui->setupUi(this);
  lock = true;
  int row = 0;
  int col = 0;

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

  // custom options
  ui->csectCustomOpts->setTitle(tr("Custom Options"));
  ui->csectCustomOpts->setAnimationDuration(300);
  QHBoxLayout *layCustomOpts = new QHBoxLayout();
  ui->csectCustomOpts->setContentLayout(*layCustomOpts);


  // new file
  ui->csectNewFile->setTitle(tr("New File"));
  ui->csectNewFile->setAnimationDuration(300);
  QGridLayout *layNewFile = new QGridLayout();
  row = col = 0;
  AutoCheckBox *chkUseSettingsBackup = new AutoCheckBox(this);
  layNewFile->addWidget(chkUseSettingsBackup, row, col++);
  ++row;
  col = 0;
  AutoLabel *lblSettingsBackup = new AutoLabel(this);
  layNewFile->addWidget(lblSettingsBackup, row, col++);
  ++row;
  col = 0;
  AutoLabel *lblStickMode = new AutoLabel(this);
  layNewFile->addWidget(lblStickMode, row, col++);
  AutoComboBox *cboStickMode = new AutoComboBox(this);
  layNewFile->addWidget(cboStickMode, row, col++);
  ++row;
  col = 0;
  AutoLabel *lblChannelOrder = new AutoLabel(this);
  layNewFile->addWidget(lblChannelOrder, row, col++);
  AutoComboBox *cboChannelOrder = new AutoComboBox(this);
  layNewFile->addWidget(cboChannelOrder, row, col++);
  ++row;
  col = 0;
  AutoLabel *lblModuleInternal = new AutoLabel(this);
  layNewFile->addWidget(lblModuleInternal, row, col++);
  AutoComboBox *cboModuleInternal = new AutoComboBox(this);
  layNewFile->addWidget(cboModuleInternal, row, col++);
  AutoLabel *lblModuleExternal = new AutoLabel(this);
  layNewFile->addWidget(lblModuleExternal, row, col++);
  AutoComboBox *cboModuleExternal = new AutoComboBox(this);
  layNewFile->addWidget(cboModuleExternal, row, col++);
  ui->csectNewFile->setContentLayout(*layNewFile);

  // folders
  ui->csectFolders->setTitle(tr("Folders"));
  ui->csectFolders->setAnimationDuration(300);
  QGridLayout *layFolders = new QGridLayout();
  row = col = 0;
  QLabel *lblSDPath = new QLabel(tr("SD Path"), this);
  layFolders->addWidget(lblSDPath, row, col++);
  AutoLineEdit *leSDPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leSDPath, row, col++);
  AutoFolderSelectButton *btnSDPath = new AutoFolderSelectButton(this);
  layFolders->addWidget(btnSDPath, row, col++);
  ++row;
  col = 0;
  QLabel *lblModelsPath = new QLabel(tr("Models"), this);
  layFolders->addWidget(lblModelsPath, row, col++);
  AutoLineEdit *leModelsPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leModelsPath, row, col++);
  AutoFolderSelectButton *btnModelsPath = new AutoFolderSelectButton(this);
  layFolders->addWidget(btnModelsPath, row, col++);
  ++row;
  col = 0;
  QLabel *lblBackupsPath = new QLabel(tr("Backups"), this);
  layFolders->addWidget(lblBackupsPath, row, col++);
  AutoLineEdit *leBackupsPath = new AutoLineEdit(this, true);
  layFolders->addWidget(leBackupsPath, row, col++);
  AutoFolderSelectButton *btnBackupsPath = new AutoFolderSelectButton(this);
  layFolders->addWidget(btnBackupsPath, row, col++);
  ui->csectFolders->setContentLayout(*layFolders);

  // cloud build options
  ui->csectCloudBuildOpts->setTitle(tr("Cloud Build Options"));
  ui->csectCloudBuildOpts->setAnimationDuration(300);
  QGridLayout *layCloudBuildOpts = new QGridLayout();
  ui->csectCloudBuildOpts->setContentLayout(*layCloudBuildOpts);

  // B&W firmware splash image
  //if (!Boards::getCapability(baseFw->getBoard(), Board::HasColorLcd)) {
    row = col = 0;
    ui->csectSplash->setTitle(tr("Splash Screen"));
    ui->csectSplash->setAnimationDuration(300);
    QGridLayout *laySplash = new QGridLayout();
    AutoLineEdit *leSplashPath = new AutoLineEdit(this, true);
    laySplash->addWidget(leSplashPath, row, col++);
    AutoFolderSelectButton *btnSplashSelect = new AutoFolderSelectButton(this);
    laySplash->addWidget(btnSplashSelect, row, col++);
    ++row;
    col = 0;
    QLabel *lblSplashImage = new QLabel(this);
    laySplash->addWidget(lblSplashImage, row, col++);
    AutoPushButton *btnSplashClear = new AutoPushButton(this);
    laySplash->addWidget(btnSplashClear, row, col++);
    ui->csectSplash->setContentLayout(*laySplash);
  //}









//  connect(ui->profileBackupPath, &QLineEdit::editingFinished, this, &PrefsProfilePanel::onProfileBackupPathEditingFinished);
//  connect(ui->profileBackupPathButton, &QPushButton::clicked, this, &PrefsProfilePanel::onProfileBackupPathButtonClicked);

  update();
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
