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
#include "firmwarepicker.h"
#include "firmwarepicker.h"

#include <QAbstractItemView>
#include <QFileInfo>
#include <QImageReader>
#include <QStandardItemModel>

constexpr char FIM_TEMPLATESETUP[]    {"Template Setup"};

PrefsProfilePanel::PrefsProfilePanel(QWidget * parent, Firmware * fw, Board::Type & bd, Profile & prof) :
  PrefsPanel(parent, fw, bd, prof),
  ui(new Ui::PrefsProfile)
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
  ui->leName->setEditSignal(true);
  ui->leName->setBindSave([this] {
    profile.name(ui->leName->text());

    if (profile.name().isEmpty())
      profile.name(tr("My Radio"));
  });

  // radio
  // this widget is for data management
  fwTypeData = new AutoLineEdit(this, true);
  fwTypeData->setHidden(true);
  fwTypeData->setValue(profile.fwType());
  fwTypeData->setBindSave([this] {
    this->profile.fwType(this->fwTypeData->text());
  });
  fwTypeData->setBindPostChanged([this] {
    // appending "-xxx" forces the associated Board definition to be loaded if not already loaded
    // TODO fix as part of refactoring Firmware and Boards
    this->firmware = Firmware::getFirmwareForId(this->fwTypeData->text() % "-xxx");
    this->board = this->firmware->getBoard();
    this->populateFirmwareOptions();
    // clear backup settings as they are specific to the firmware
    this->profile.generalSettings(QByteArray());
    this->profile.timeStamp(QString());
    this->update();
    emit radioChanged(this->firmware);
  });

  // this widget displays the firmware full name
  ui->leFirmwareType->setReadOnly(true);
  ui->leFirmwareType->setBindText([this] {
    return Firmware::getFirmwareForId(this->fwTypeData->text())->getFullName();
  });
  ui->leFirmwareType->setMinimumWidth(PATH_MIN_WIDTH);
  ui->leFirmwareType->setSizePolicy(PATH_SIZE_POLICY);

  connect(ui->btnFirmwareType, &QPushButton::pressed, this, &PrefsProfilePanel::onFirmwareTypePressed);

  sectionNewFile();
  sectionFolders();
  sectionFirmwareOpts();
  sectionSplash();

  update();
  shrink();
  lock = false;
}

PrefsProfilePanel::~PrefsProfilePanel()
{
  delete ui;
}

void PrefsProfilePanel::onFirmwareTypePressed()
{
  FirmwarePicker * picker = new FirmwarePicker(this, fwTypeData->text());
  connect(picker, &FirmwarePicker::firmwareTypeChanged, this, [this] (const QString newType) {
    this->fwTypeData->setText(newType);
    this->update();
  });
  picker->exec();
  picker->deleteLater();
}

QString PrefsProfilePanel::getLanguage()
{
  return !profile.fwLanguage().isEmpty() ?
    profile.fwLanguage() :
    QLocale::languageToString(QLocale().language()).split("_").first();
}

QStringList PrefsProfilePanel::getSelectedOptions()
{
  QStringList opts;

  if (chkFirmwareBuildOpts.size()) {
    QMutableMapIterator<QString, AutoCheckBox *> it(chkFirmwareBuildOpts);

    while (it.hasNext()) {
      it.next();
      AutoCheckBox * chk = it.value();

      if (chk->isChecked())
        opts.append(it.key());
    }
  }

  return opts;
}

QString PrefsProfilePanel::getSplashFileFilter()
{
  QString fmts;

  for (int idx = 0; idx < QImageReader::supportedImageFormats().count(); idx++) {
    fmts += QLatin1String(" *.") + QImageReader::supportedImageFormats()[idx];
  }

  return fmts;
}

QAbstractItemModel * PrefsProfilePanel::languageModel()
{
  QStandardItemModel * mdl = new QStandardItemModel(this);

  for (const char *lang : firmware->getFirmwareBase()->languageList()) {
    QStandardItem * item =  new QStandardItem();
    item->setText(lang);
    item->setData(lang, Qt::UserRole);
    mdl->appendRow(item);
  }

  QSortFilterProxyModel *smdl = new QSortFilterProxyModel(this);
  smdl->setSourceModel(mdl);
  smdl->setSortCaseSensitivity(Qt::CaseInsensitive);
  smdl->sort(0);
  return smdl;
}

void PrefsProfilePanel::onOptionChanged(QString name)
{
  AutoCheckBox *chk = chkFirmwareBuildOpts.value(name, nullptr);

  if (!(chk && chk->isChecked())) return;

  const Firmware::OptionsList & fwOpts = firmware->getFirmwareBase()->optionGroups();

  // This de-selects any mutually exlusive options (that is, members of the same QList<Option> list).
  for (const Firmware::OptionsGroup & optGrp : fwOpts) {
    for (const Firmware::Option & opt : optGrp) {
      if (name == opt.name) {
        AutoCheckBox *ochk = nullptr;

        foreach(const Firmware::Option & other, optGrp) {
          if (other.name != opt.name && (ochk = chkFirmwareBuildOpts.value(other.name, nullptr)))
            ochk->setValue(false);
        }

        return;
      }
    }
  }
}

void PrefsProfilePanel::populateFirmwareOptions(QStringList opts)
{
  QStringList currOpts = opts;

  if (!opts.size() && chkFirmwareBuildOpts.size()) {
    currOpts.clear();
    QMutableMapIterator<QString, AutoCheckBox *> it(chkFirmwareBuildOpts);
    while (it.hasNext()) {
      it.next();
      QCheckBox * chk = it.value();

      if (chk->isChecked())
        currOpts.append(it.key());    // keep previous selections

      layFirmwareBuildOpts->removeWidget(chk);
      chk->deleteLater();
      it.remove();
    }
  }

  int index = 0;
  QWidget * prevFocus = cboFirmwareLanguage;

  for (const Firmware::OptionsGroup &optGrp : firmware->getFirmwareBase()->optionGroups()) {
    for (const Firmware::Option &opt : optGrp) {
      AutoCheckBox * chk = new AutoCheckBox(this, opt.name);
      chk->setValue(currOpts.contains(opt.name));
      chk->setToolTip(opt.tooltip);
      // connect to duplicates check handler if this option is part of a group
      if (optGrp.size() > 1)
        chk->setBindPostChanged([=] { this->onOptionChanged(opt.name); });

      layFirmwareBuildOpts->addWidget(chk, index / 4, index % 4);
      chkFirmwareBuildOpts.insert(opt.name, chk);
      QWidget::setTabOrder(prevFocus, chk);
      prevFocus = chk;
      ++index;
    }
  }

  shrink();
}

void PrefsProfilePanel::save()
{
  profile.fwOptions(getSelectedOptions().join("-"));
  AbstractPanel::save();
}

// options  TODO split into those supported by Cloud Build and others
void PrefsProfilePanel::sectionFirmwareOpts()
{
  QGridLayout *layFirmwareOpts = ui->csectFirmwareOpts->start(tr("Firmware Options"));
  row = col = 0;

  // language
  QLabel *lblFirmwareLanguage = new QLabel(tr("Language"), this);
  layFirmwareOpts->addWidget(lblFirmwareLanguage, row, col++);

  QHBoxLayout *layLanguage = new QHBoxLayout();
  cboFirmwareLanguage = new AutoComboBox(this);
  cboFirmwareLanguage->setModel(languageModel());
  cboFirmwareLanguage->setValue(profile.fwLanguage());
  cboFirmwareLanguage->setBindSave([this] {
    this->profile.fwLanguage(this->cboFirmwareLanguage->currentData().toString());
  });
  layLanguage->addWidget(cboFirmwareLanguage);
  QLabel *spacerLanguage = new QLabel();
  spacerLanguage->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  layLanguage->addWidget(spacerLanguage);
  layFirmwareOpts->addLayout(layLanguage, row, col++);

  // other options
  ++row; col = 0;
  AutoLabel *lblFirmwareOptions = new AutoLabel(this, tr("Build"));
  layFirmwareOpts->addWidget(lblFirmwareOptions, row, col++, Qt::AlignTop);

  layFirmwareBuildOpts = new QGridLayout();
  layFirmwareOpts->addLayout(layFirmwareBuildOpts, row, col++);
  populateFirmwareOptions(profile.fwOptions().split("-", Qt::SkipEmptyParts));
  // flashing
  ++row; col = 0;
  AutoLabel *lblBackupBeforeFlash = new AutoLabel(this, tr("Flashing"));
  layFirmwareOpts->addWidget(lblBackupBeforeFlash, row, col++, Qt::AlignTop);

  chkBackupBeforeFlash = new AutoCheckBox(this, tr("prompt to backup current firmware before flashing"));
  chkBackupBeforeFlash->setValue(profile.penableBackup());
  chkBackupBeforeFlash->setBindSave([this] {
    this->profile.penableBackup(this->chkBackupBeforeFlash->isChecked());
  });
  layFirmwareOpts->addWidget(chkBackupBeforeFlash, row, col++);

  ui->csectFirmwareOpts->finish(row, col, [this] { this->shrink(); });
}

void PrefsProfilePanel::sectionFolders()
{
  QGridLayout *layFolders = ui->csectFolders->start(tr("Folders"));
  row = col = 0;

  AutoLabel *lblSDPath = new AutoLabel(this, tr("SD Path"));
  layFolders->addWidget(lblSDPath, row, col++);

  leSDPath = newPathWidget(this);
  leSDPath->setValue(profile.sdPath());
  leSDPath->setBindSave([this] {
    this->profile.sdPath(this->leSDPath->text());
  });
  leSDPath->setBindPostChanged([this] {
    // alert other tabs eg Updates
    emit this->sdPathChanged(this->leSDPath->text());
  });
  layFolders->addWidget(leSDPath, row, col++);

  AutoDirectorySelectButton *btnSDPath = new AutoDirectorySelectButton(this);
  btnSDPath->setup(tr("Select SD path folder"), profile.sdPath(), leSDPath);;
  layFolders->addWidget(btnSDPath, row, col++);
  // Backups path
  ++row; col = 0;
  AutoLabel *lblBackupsPath = new AutoLabel(this, tr("Backups"));
  layFolders->addWidget(lblBackupsPath, row, col++);

  leBackupsPath = newPathWidget(this);
  leBackupsPath->setValue(profile.pBackupDir());
  leBackupsPath->setBindSave([this] {
    this->profile.pBackupDir(this->leBackupsPath->text());
  });
  layFolders->addWidget(leBackupsPath, row, col++);

  AutoDirectorySelectButton *btnBackupsPath = new AutoDirectorySelectButton(this);
  btnBackupsPath->setup(tr("Select backups folder"), profile.pBackupDir(), leBackupsPath);;
  layFolders->addWidget(btnBackupsPath, row, col++);

  /*  TODO implement
  ++row; col = 0;
  AutoLabel *lblModelsPath = new AutoLabel(this, tr("Models"));
  layFolders->addWidget(lblModelsPath, row, col++);

  leModelsPath = newPathWidget(this);
  leModelsPath->setValue(profile.modelsDir());
  leModelsPath->setBindSave([this] {
    this->profile.modelsDir(this->leModelsPath->text());
  });
  layFolders->addWidget(leModelsPath, row, col++);

  AutoDirectorySelectButton *btnModelsPath = new AutoDirectorySelectButton(this);
  btnModelsPath->setup(tr("Select models folder"), profile.modelsDir(), leModelsPath);;
  layFolders->addWidget(btnModelsPath, row, col++);
 */

  ui->csectFolders->finish(-1, -1, [this] { this->shrink(); });
}

void PrefsProfilePanel::sectionNewFile()
{
  QGridLayout *layNewFile = ui->csectNewFile->start(tr("New Models and Settings Files"));
  row = col = 0;

  AutoLabel *lblUseSettingsBackup = new AutoLabel(this, tr("Use backup settings"));
  layNewFile->addWidget(lblUseSettingsBackup, row, col++);
  chkUseSettingsBackup = new AutoCheckBox(this, " ");
  chkUseSettingsBackup->setValue(profile.useSavedSettings());
  chkUseSettingsBackup->setBindSave([this] {
    this->profile.useSavedSettings(this->chkUseSettingsBackup->isChecked());
  });
  chkUseSettingsBackup->setBindPostChanged([this] { this->update(); });
  layNewFile->addWidget(chkUseSettingsBackup, row, col++);

  ++row; col = 0;
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
  ++row; col = 0;
  AutoLabel *lblStickMode = new AutoLabel(this, tr("Default Stick Mode"));
  lblStickMode->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() &&
             this->profile.generalSettings().isEmpty()));
  });
  lblStickMode->setBindVisible([this] { return Boards::isAir(); });
  layNewFile->addWidget(lblStickMode, row, col++);

  cboStickMode = new AutoComboBox(this);
  cboStickMode->setModel(GeneralSettings::stickModeItemModel());
  cboStickMode->setValue(profile.defaultMode());
  cboStickMode->setBindSave([this] {
    this->profile.defaultMode(this->cboStickMode->currentData().toInt());
  });
  cboStickMode->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() &&
             this->profile.generalSettings().isEmpty()));
  });
  cboStickMode->setBindVisible([this] { return Boards::isAir(); });
  layNewFile->addWidget(cboStickMode, row, col++);
  // Channel Order
  ++row; col = 0;
  AutoLabel *lblChannelOrder = new AutoLabel(this, tr("Default Channel Order"));
  lblChannelOrder->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() &&
             this->profile.generalSettings().isEmpty()));
  });
  layNewFile->addWidget(lblChannelOrder, row, col++);

  cboChannelOrder = new AutoComboBox(this);
  cboChannelOrder->setModel(panelItemModels->getItemModel(FIM_TEMPLATESETUP));
  cboChannelOrder->setValue(profile.channelOrder());
  cboChannelOrder->setBindSave([this] {
    this->profile.channelOrder(this->cboChannelOrder->currentData().toInt());
  });
  cboChannelOrder->setBindEnabled([this] {
    return (!this->chkUseSettingsBackup->isChecked() ||
            (this->chkUseSettingsBackup->isChecked() &&
             this->profile.generalSettings().isEmpty()));
  });
  layNewFile->addWidget(cboChannelOrder, row, col++);
  // Internal Module
  ++row; col = 0;
  AutoLabel *lblModuleInternal = new AutoLabel(this, tr("Default Internal Module"));
  layNewFile->addWidget(lblModuleInternal, row, col++);
  cboModuleInternal = new AutoComboBox(this);
  cboModuleInternal->setModel(ModuleData::internalModuleItemModel());
  cboModuleInternal->setValue(profile.defaultInternalModule());
  cboModuleInternal->setBindSave([this] {
    profile.defaultInternalModule(this->cboModuleInternal->currentData().toInt());
  });
  layNewFile->addWidget(cboModuleInternal, row, col++);
  // External Module
  ++row; col = 0;
  AutoLabel *lblModuleExternal = new AutoLabel(this, tr("External Module Size"));
  layNewFile->addWidget(lblModuleExternal, row, col++);

  cboModuleExternal = new AutoComboBox(this);
  cboModuleExternal->setModel(Boards::externalModuleSizeItemModel());
  cboModuleExternal->setValue(profile.externalModuleSize());
  cboModuleExternal->setBindSave([this] {
    this->profile.externalModuleSize(this->cboModuleExternal->currentData().toInt());
  });
  layNewFile->addWidget(cboModuleExternal, row, col++);

  ui->csectNewFile->finish(row, col, [this] { this->shrink(); });
}

void PrefsProfilePanel::sectionSplash()
{
  QGridLayout *laySplash = ui->csectSplash->start(tr("Splash Screen"));
  ui->csectSplash->setBindVisible([this] {
    return !Boards::getCapability(this->board, Board::HasColorLcd);
  });
  row = col = 0;

  leSplashPath = newPathWidget(this);
  leSplashPath->setValue(profile.splashFile());
  leSplashPath->setBindSave([this] {
    this->profile.splashFile(this->leSplashPath->text());
  });
  laySplash->addWidget(leSplashPath, row, col++);
  // Splash folder select
  AutoFileSelectButton *btnSplashSelect = new AutoFileSelectButton(this);
  btnSplashSelect->setup(tr("Open Image to load"), g.imagesDir(),
                         tr("Images (%1)").arg(getSplashFileFilter()), leSplashPath);
  btnSplashSelect->setBindPostChanged([this] {
    if (!this->leSplashPath->text().isEmpty()){
      g.imagesDir(QFileInfo(this->leSplashPath->text()).dir().absolutePath());
    }
  });
  laySplash->addWidget(btnSplashSelect, row, col++);
  // Splash image
  ++row; col = 0;
  imgSplash = new AutoImage(this, leSplashPath->text());
  // change of firmware and thus board can effect the image
  imgSplash->setBindPreUpdate([this] {
    imgSplash->setDimensions(Boards::getCapability(this->board, Board::LcdWidth),
                             Boards::getCapability(this->board, Board::LcdHeight),
                             Boards::getCapability(this->board, Board::LcdDepth));
  });
  laySplash->addWidget(imgSplash, row, col++);
  // Splash clear
  AutoPushButton *btnSplashClear = new AutoPushButton(this, tr("Clear"));
  btnSplashClear->setBindClicked([this] {
    this->imgSplash->clear();
    this->leSplashPath->clear();
  });
  laySplash->addWidget(btnSplashClear, row, col++);

  ui->csectSplash->finish(row, col, [this] { this->shrink(); });
}

void PrefsProfilePanel::undoFirmwareChange()
{
  firmware = getCurrentFirmware();
  board = firmware->getBoard();
  fwTypeData->setText(firmware->getFirmwareBase()->getId());
  populateFirmwareOptions(profile.fwOptions().split("-"));
  update();
}

void PrefsProfilePanel::update()
{
  AbstractPanel::update();
}
