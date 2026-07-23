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

#include <QAbstractItemView>
#include <QStandardItemModel>

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
  panelItemModels->getItemModel(FIM_TEMPLATESETUP)->setFilterFlags(Boards::isAir() ? GeneralSettings::RadioTypeContextAir :
                                                                                     GeneralSettings::RadioTypeContextSurface);

  // name
  // The profile name may NEVER be empty
  if (profile.name().isEmpty())
    profile.name(tr("My Radio"));

  ui->leName->setValue(profile.name(), this);
  ui->leName->setBindSave([this] {
    profile.name(ui->leName->text());

    if (profile.name().isEmpty())
      profile.name(tr("My Radio"));
  });

  // radio
  ui->cboRadio->setModel(firmwareModel());
  // QComboBox::sizeAdjustPolicy(QCombobox::AdjustToContents) does not resize as requested
  // due to using a model and nested layouts. Since the list view is correct, use it
  ui->cboRadio->setMaximumWidth(ui->cboRadio->view()->width());
  ui->cboRadio->setValue(profile.fwType(), this);
  ui->cboRadio->setBindSave([this] { profile.fwType(ui->cboRadio->currentData().toString());} );

  // new file
  row = col = 0;
  ui->csectNewFile->setTitle(tr("New Models and Settings Files"));
  QGridLayout *layNewFile = new QGridLayout();
  // Use backup settings
  AutoLabel *lblUseSettingsBackup = new AutoLabel(this, tr("Use backup settings"));
  layNewFile->addWidget(lblUseSettingsBackup, row, col++);
  chkUseSettingsBackup = new AutoCheckBox(this, " ");
  chkUseSettingsBackup->setValue(profile.useSavedSettings(), this);
  chkUseSettingsBackup->setBindSave([this] {
    profile.useSavedSettings(this->chkUseSettingsBackup->isChecked());
  });
  chkUseSettingsBackup->setBindPostChanged([this] { this->update(); });
  layNewFile->addWidget(chkUseSettingsBackup, row, col++);

  newRow();
  lblSettingsBackup = new AutoLabel(this);
  lblSettingsBackup->setBindText([this] (){
    if (profile.generalSettings().isEmpty()) {
      return tr("No backup available for this profile");
    } else {
      QString str = profile.timeStamp();
      if (str.isEmpty())
        return tr("Backup available of unknown age");
      else
        return tr("Backup available dated %1").arg(str);
    }
  });
  layNewFile->addWidget(lblSettingsBackup, row, 1);

  // Stick Mode
  newRow();
  AutoLabel *lblStickMode = new AutoLabel(this, tr("Default Stick Mode"));
  lblStickMode->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() && profile.generalSettings().isEmpty()));
  });
  lblStickMode->setBindVisible([this] { return Boards::isAir(); });
  layNewFile->addWidget(lblStickMode, row, col++);

  cboStickMode = new AutoComboBox(this);
  cboStickMode->setModel(GeneralSettings::stickModeItemModel());
  cboStickMode->setValue(profile.defaultMode(), this);
  cboStickMode->setBindSave([this] { profile.defaultMode(this->cboStickMode->currentData().toInt()); });
  cboStickMode->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() && profile.generalSettings().isEmpty()));
  });
  cboStickMode->setBindVisible([this] { return Boards::isAir(); });
  layNewFile->addWidget(cboStickMode, row, col++);
  // Channel Order
  newRow();
  AutoLabel *lblChannelOrder = new AutoLabel(this, tr("Default Channel Order"));
  lblChannelOrder->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() && profile.generalSettings().isEmpty()));
  });
  layNewFile->addWidget(lblChannelOrder, row, col++);

  cboChannelOrder = new AutoComboBox(this);
  cboChannelOrder->setModel(panelItemModels->getItemModel(FIM_TEMPLATESETUP));
  cboChannelOrder->setValue(profile.channelOrder(), this);
  cboChannelOrder->setBindSave([this] { profile.channelOrder(this->cboChannelOrder->currentData().toInt()); });
  cboChannelOrder->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() && profile.generalSettings().isEmpty()));
  });
  layNewFile->addWidget(cboChannelOrder, row, col++);
  // Internal Module
  newRow();
  AutoLabel *lblModuleInternal = new AutoLabel(this, tr("Default Internal Module"));
  layNewFile->addWidget(lblModuleInternal, row, col++);
  cboModuleInternal = new AutoComboBox(this);
  cboModuleInternal->setModel(ModuleData::internalModuleItemModel());
  cboModuleInternal->setValue(profile.defaultInternalModule(), this);
  cboModuleInternal->setBindSave([this] { profile.defaultInternalModule(this->cboModuleInternal->currentData().toInt()); });
  layNewFile->addWidget(cboModuleInternal, row, col++);
  // External Module
  newRow();
  AutoLabel *lblModuleExternal = new AutoLabel(this, tr("External Module Size"));
  layNewFile->addWidget(lblModuleExternal, row, col++);

  cboModuleExternal = new AutoComboBox(this);
  cboModuleExternal->setModel(Boards::externalModuleSizeItemModel());
  cboModuleExternal->setValue(profile.externalModuleSize(), this);
  cboModuleExternal->setBindSave([this] { profile.externalModuleSize(this->cboModuleExternal->currentData().toInt()); });
  layNewFile->addWidget(cboModuleExternal, row, col++);

  addHSpring(layNewFile, row, col);
  ui->csectNewFile->setContentLayout(*layNewFile);
  ui->csectNewFile->setBindResize([this] { this->shrink(); });

  // folders
  row = col = 0;
  ui->csectFolders->setTitle(tr("Folders"));
  QGridLayout *layFolders = new QGridLayout();
  // SD Path
  AutoLabel *lblSDPath = new AutoLabel(this, tr("SD Path"));
  layFolders->addWidget(lblSDPath, row, col++);
  leSDPath = new AutoLineEdit(this, true);
  leSDPath->setValue(profile.sdPath(), this);
  leSDPath->setEditSignal(true);
  leSDPath->setBindSave([this] { profile.sdPath(this->leSDPath->text()); });
  layFolders->addWidget(leSDPath, row, col++);

  AutoDirectorySelectButton *btnSDPath = new AutoDirectorySelectButton(this);
  btnSDPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  btnSDPath->setup(tr("Select SD path folder"), profile.sdPath(), leSDPath);;
  layFolders->addWidget(btnSDPath, row, col++);
  // Models path
  newRow();
  AutoLabel *lblModelsPath = new AutoLabel(this, tr("Models"));
  layFolders->addWidget(lblModelsPath, row, col++);
  leModelsPath = new AutoLineEdit(this, true);
  leModelsPath->setValue(profile.modelsDir(), this);
  leModelsPath->setEditSignal(true);
  leModelsPath->setBindSave([this] { profile.modelsDir(this->leModelsPath->text()); });
  layFolders->addWidget(leModelsPath, row, col++);

  AutoDirectorySelectButton *btnModelsPath = new AutoDirectorySelectButton(this);
  btnModelsPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  btnModelsPath->setup(tr("Select models folder"), profile.modelsDir(), leModelsPath);;
  layFolders->addWidget(btnModelsPath, row, col++);
  // Backups path
  newRow();
  AutoLabel *lblBackupsPath = new AutoLabel(this, tr("Backups"));
  layFolders->addWidget(lblBackupsPath, row, col++);
  leBackupsPath = new AutoLineEdit(this, true);
  leBackupsPath->setValue(profile.pBackupDir(), this);
  leBackupsPath->setEditSignal(true);
  leBackupsPath->setBindSave([this] { profile.pBackupDir(this->leBackupsPath->text());});
  layFolders->addWidget(leBackupsPath, row, col++);

  AutoDirectorySelectButton *btnBackupsPath = new AutoDirectorySelectButton(this);
  btnBackupsPath->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  btnBackupsPath->setup(tr("Select backups folder"), profile.pBackupDir(), leBackupsPath);;
  layFolders->addWidget(btnBackupsPath, row, col++);

  addHSpring(layFolders, row, col);
  ui->csectFolders->setContentLayout(*layFolders);
  ui->csectFolders->setBindResize([this] { this->shrink(); });

  // options  TODO split into custom and those supported by Cloud Build
  row = col = 0;
  ui->csectFirmwareOpts->setTitle(tr("Firmware Options"));
  QGridLayout *layFirmwareOpts = new QGridLayout();
  // language
  QLabel *lblLanguage = new QLabel(tr("Language"), this);
  layFirmwareOpts->addWidget(lblLanguage, row, col++);
  cboLanguage = new AutoComboBox(this);
  cboLanguage->addItems(languageList());
  cboLanguage->setValue(profile.fwLanguage(), this, false);
  cboLanguage->setBindSave([this] { profile.fwLanguage(this->cboLanguage->currentText()); });
  layFirmwareOpts->addWidget(cboLanguage, row, col++);

  addHSpring(layFirmwareOpts, row, col);
  ui->csectFirmwareOpts->setContentLayout(*layFirmwareOpts);
  ui->csectFirmwareOpts->setBindResize([this] { this->shrink(); });

  // B&W firmware splash image
  row = col = 0;
  ui->csectSplash->setTitle(tr("Splash Screen"));
  QGridLayout *laySplash = new QGridLayout();
  // Splash path
  leSplashPath = new AutoLineEdit(this, true);
  leSplashPath->setValue(profile.splashFile(), this);
  leSplashPath->setBindSave([this] { profile.splashFile(this->leSplashPath->text());});
  laySplash->addWidget(leSplashPath, row, col++);
  // Splash folder select
  AutoFileSelectButton *btnSplashSelect = new AutoFileSelectButton(this);
  btnSplashSelect->setup(tr("Select splash folder"), profile.splashFile(), getSplashFileFilter(), leSplashPath);;
  laySplash->addWidget(btnSplashSelect, row, col++);
  // Splash image
  newRow();
  lblSplashImage = new AutoLabel(this);
  laySplash->addWidget(lblSplashImage, row, col++);
  // Splash clear
  AutoPushButton *btnSplashClear = new AutoPushButton(this, tr("Clear"));
  connect(btnSplashClear, &QPushButton::released, this, [this] () {
    this->lblSplashImage->clear();
    this->leSplashPath->clear();
  });
  laySplash->addWidget(btnSplashClear, row, col++);
  addHSpring(laySplash, row, col);
  ui->csectSplash->setContentLayout(*laySplash);
  ui->csectSplash->setBindResize([this] { this->shrink(); });
  ui->csectSplash->setBindVisible([this] {
    return !(Boards::getCapability(this->getFirmwareVariant()->getBoard(), Board::HasColorLcd));
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

Firmware * PrefsProfilePanel::getFirmwareVariant() const
{
  // TODO check if need -xxx
  return Firmware::getFirmwareForId(ui->cboRadio->currentData().toString() % "-xxx");
}

QString PrefsProfilePanel::getLanguage()
{
  return !profile.fwLanguage().isEmpty() ?
    profile.fwLanguage() :
    QLocale::languageToString(QLocale().language()).split("_").first();
}

QStringList PrefsProfilePanel::languageList()
{
  QStringList strl;

  for (const char *lang : getFirmwareVariant()->getFirmwareBase()->languageList())
    strl.append(lang);

  strl.sort();
  return strl;
}

QAbstractItemModel * PrefsProfilePanel::firmwareModel()
{
  QStandardItemModel * mdl = new QStandardItemModel(this);

  foreach(Firmware * firmware, Firmware::getRegisteredFirmwares()) {
    QStandardItem * item =  new QStandardItem();
    item->setText(firmware->getName());
    item->setData(firmware->getId(), Qt::UserRole);
    mdl->appendRow(item);
  }

  QSortFilterProxyModel *smdl = new QSortFilterProxyModel(this);
  smdl->setSourceModel(mdl);
  smdl->setSortCaseSensitivity(Qt::CaseInsensitive);
  smdl->sort(0);
  return smdl;
}
